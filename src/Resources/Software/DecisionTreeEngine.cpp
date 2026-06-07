/**
 * @file DecisionTreeEngine.cpp
 * @brief Traversal logic and dynamic SD loader for Decision Tree (XGBoost/LightGBM) ensembles.
 */

#include <Resources/Software/DecisionTreeEngine.h>
#include <Modules/SDcard.h>

#define logg(message) loggWithObj(message, "ML_ENGINE")
#define loggValue(message, value) loggWithCtx(message, "ML_ENGINE", value)

// Reuse the global SDcard instance declared in main.cpp
extern SDcard sdcard;

// Global ensemble definitions
BoostedEnsemble pm1Ensemble  = {0.0f, 0, 0, nullptr, nullptr, false};
BoostedEnsemble pm25Ensemble = {0.0f, 0, 0, nullptr, nullptr, false};
BoostedEnsemble pm10Ensemble = {0.0f, 0, 0, nullptr, nullptr, false};
BoostedEnsemble coEnsemble   = {0.0f, 0, 0, nullptr, nullptr, false};
BoostedEnsemble no2Ensemble  = {0.0f, 0, 0, nullptr, nullptr, false};
BoostedEnsemble nh3Ensemble  = {0.0f, 0, 0, nullptr, nullptr, false};
BoostedEnsemble co2Ensemble  = {0.0f, 0, 0, nullptr, nullptr, false};

// ---------------------------------------------------------------------------
// Static Default (Fallback) Model Definitions (Pass-through models)
// These models return 0.0f correction, adding no offset unless SD overrides them.
// ---------------------------------------------------------------------------

static DecisionTree defaultTrees[] = {
    {0, 1}
};

static TreeNode defaultNodes[] = {
    {-1, 0, 0, 0, 0.0f, 0.0f} // Leaf node returning 0.0f
};

// ---------------------------------------------------------------------------
// Private Helpers
// ---------------------------------------------------------------------------

/**
 * @brief Helper to load a binary model from the SD card.
 * @param filepath File path on the SD card.
 * @param ensemble Reference to the target BoostedEnsemble.
 * @return True if model loaded and verified successfully, false otherwise.
 */
static bool loadEnsembleFromSD(const char* filepath, BoostedEnsemble& ensemble) {
    if (!sdcard.isInitialised()) {
        logg("SD card not initialised – cannot load model.");
        return false;
    }

    File file = SD.open(filepath, FILE_READ);
    if (!file) {
        loggValue("Binary model file not found: ", filepath);
        return false;
    }

    // Verify magic header "MLCA" (Machine Learning Calibration Model)
    char magic[4];
    if (file.read((uint8_t*)magic, 4) != 4 || memcmp(magic, "MLCA", 4) != 0) {
        loggValue("Invalid model file header (magic mismatch): ", filepath);
        file.close();
        return false;
    }

    float baseScore = 0.0f;
    uint16_t treeCount = 0;
    uint16_t nodeCount = 0;

    if (file.read((uint8_t*)&baseScore, sizeof(float)) != sizeof(float) ||
        file.read((uint8_t*)&treeCount, sizeof(uint16_t)) != sizeof(uint16_t) ||
        file.read((uint8_t*)&nodeCount, sizeof(uint16_t)) != sizeof(uint16_t)) {
        loggValue("Failed to read header metadata from: ", filepath);
        file.close();
        return false;
    }

    if (treeCount == 0 || nodeCount == 0) {
        loggValue("Empty tree or node counts in model: ", filepath);
        file.close();
        return false;
    }

    // Allocate memory on the heap
    DecisionTree* trees = (DecisionTree*)malloc(treeCount * sizeof(DecisionTree));
    TreeNode* nodes = (TreeNode*)malloc(nodeCount * sizeof(TreeNode));

    if (!trees || !nodes) {
        logg("Out of memory allocating tree structures.");
        free(trees);
        free(nodes);
        file.close();
        return false;
    }

    // Read tree offsets/sizes table
    size_t treeTableBytes = treeCount * sizeof(DecisionTree);
    if (file.read((uint8_t*)trees, treeTableBytes) != treeTableBytes) {
        logg("Failed to read tree structure offsets.");
        free(trees);
        free(nodes);
        file.close();
        return false;
    }

    // Read flat node pool
    size_t nodePoolBytes = nodeCount * sizeof(TreeNode);
    if (file.read((uint8_t*)nodes, nodePoolBytes) != nodePoolBytes) {
        logg("Failed to read node split array.");
        free(trees);
        free(nodes);
        file.close();
        return false;
    }

    file.close();

    // Clean up any previously allocated dynamic memory to prevent leaks
    freeEnsemble(ensemble);

    ensemble.baseScore = baseScore;
    ensemble.treeCount = treeCount;
    ensemble.nodeCount = nodeCount;
    ensemble.trees = trees;
    ensemble.nodes = nodes;
    ensemble.loadedFromSD = true;

    loggValue("Successfully loaded model from SD: ", filepath);
    loggValue("Tree count: ", String(treeCount));
    loggValue("Node count: ", String(nodeCount));
    loggValue("Base score: ", String(baseScore, 4));

    return true;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

static void assignDefaultModel(BoostedEnsemble& ensemble, const char* name) {
    loggValue("Using default Flash fallback for calibration: ", name);
    ensemble.baseScore = 0.0f;
    ensemble.treeCount = 1;
    ensemble.nodeCount = 1;
    ensemble.trees = defaultTrees;
    ensemble.nodes = defaultNodes;
    ensemble.loadedFromSD = false;
}

void initMLCalibration() {
    logg("Initialising Machine Learning calibration stage...");

    if (!loadEnsembleFromSD("/models/pm1_calib.bin", pm1Ensemble)) {
        assignDefaultModel(pm1Ensemble, "PM1.0");
    }
    if (!loadEnsembleFromSD("/models/pm25_calib.bin", pm25Ensemble)) {
        assignDefaultModel(pm25Ensemble, "PM2.5");
    }
    if (!loadEnsembleFromSD("/models/pm10_calib.bin", pm10Ensemble)) {
        assignDefaultModel(pm10Ensemble, "PM10.0");
    }
    if (!loadEnsembleFromSD("/models/co_calib.bin", coEnsemble)) {
        assignDefaultModel(coEnsemble, "CO");
    }
    if (!loadEnsembleFromSD("/models/no2_calib.bin", no2Ensemble)) {
        assignDefaultModel(no2Ensemble, "NO2");
    }
    if (!loadEnsembleFromSD("/models/nh3_calib.bin", nh3Ensemble)) {
        assignDefaultModel(nh3Ensemble, "NH3");
    }
    if (!loadEnsembleFromSD("/models/co2_calib.bin", co2Ensemble)) {
        assignDefaultModel(co2Ensemble, "CO2");
    }
}


float evaluateTree(const TreeNode* nodes, const DecisionTree& tree, const float* features) {
    int16_t current = tree.nodeOffset;
    int16_t limit = tree.nodeOffset + tree.nodeCount;

    // Safety guard to avoid bounds overflow
    while (current >= tree.nodeOffset && current < limit) {
        const TreeNode& node = nodes[current];
        if (node.feature == -1) {
            return node.value;
        }

        // Feature indices are validated dynamically against mapping range
        if (node.feature < 0 || node.feature >= ML_FEATURE_COUNT) {
            return 0.0f;
        }

        float featValue = features[node.feature];
        if (featValue <= node.threshold) {
            current = node.left;
        } else {
            current = node.right;
        }
    }
    return 0.0f;
}

float evaluateEnsemble(const BoostedEnsemble& ensemble, const float* features) {
    if (ensemble.treeCount == 0 || !ensemble.trees || !ensemble.nodes) {
        return 0.0f;
    }

    float prediction = ensemble.baseScore;
    for (uint16_t i = 0; i < ensemble.treeCount; i++) {
        prediction += evaluateTree(ensemble.nodes, ensemble.trees[i], features);
    }
    return prediction;
}

void freeEnsemble(BoostedEnsemble& ensemble) {
    if (ensemble.loadedFromSD) {
        free(ensemble.trees);
        free(ensemble.nodes);
        ensemble.trees = nullptr;
        ensemble.nodes = nullptr;
        ensemble.treeCount = 0;
        ensemble.nodeCount = 0;
        ensemble.loadedFromSD = false;
    }
}
