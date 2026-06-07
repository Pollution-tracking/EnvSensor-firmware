/**
 * @file DecisionTreeEngine.h
 * @brief Serialized Decision Tree (XGBoost/LightGBM) inference engine for ESP32.
 * @details Implements a lightweight, flash-optimized traversal runtime for
 * tree-based machine learning calibration models, with support for SD card override.
 */

#ifndef DECISION_TREE_ENGINE_H
#define DECISION_TREE_ENGINE_H

#include <Arduino.h>
#include <Resources/Constants/compensation_constants.h>

/**
 * @struct TreeNode
 * @brief Represents a single decision or leaf node in a decision tree.
 * @note Struct is designed to compile to exactly 16 bytes with standard padding.
 */
struct TreeNode {
    int8_t feature;      ///< Feature index to split on (from MLFeature enum), or -1 if leaf node.
    uint8_t reserved;    ///< Reserved padding for 16-bit alignment.
    int16_t left;        ///< Absolute node index in nodes array for Left child branch.
    int16_t right;       ///< Absolute node index in nodes array for Right child branch.
    float threshold;     ///< Decision split threshold (only relevant if feature >= 0).
    float value;         ///< Leaf prediction output value (only relevant if feature == -1).
};

/**
 * @struct DecisionTree
 * @brief Describes the range of nodes in a flat array that belong to a single tree.
 */
struct DecisionTree {
    uint16_t nodeOffset; ///< The starting index in the global TreeNode pool.
    uint16_t nodeCount;  ///< Total number of nodes in this tree.
};

/**
 * @struct BoostedEnsemble
 * @brief An ensemble of decision trees forming a gradient boosted model.
 */
struct BoostedEnsemble {
    float baseScore;         ///< Base prediction bias (e.g. mean value from XGBoost / LightGBM).
    uint16_t treeCount;      ///< Total number of trees in this ensemble.
    uint16_t nodeCount;      ///< Total number of nodes in this ensemble.
    DecisionTree* trees;     ///< Array of DecisionTree structures.
    TreeNode* nodes;         ///< Global flat array of all TreeNodes.
    bool loadedFromSD;       ///< True if loaded dynamically from SD card, false if using flash fallback.
};

// Global ensemble pointers for calibration
extern BoostedEnsemble pm1Ensemble;
extern BoostedEnsemble pm25Ensemble;
extern BoostedEnsemble pm10Ensemble;
extern BoostedEnsemble coEnsemble;
extern BoostedEnsemble no2Ensemble;
extern BoostedEnsemble nh3Ensemble;
extern BoostedEnsemble co2Ensemble;


/**
 * @brief Initializes the ML calibration engine, checking for SD override files.
 * @details This function is executed at every boot stage (due to deep sleep) to
 * verify the presence of model binaries on the SD card. If present, it loads them into
 * RAM. Otherwise, it falls back to the default Flash calibration data.
 */
void initMLCalibration();

/**
 * @brief Evaluates a single decision tree recursively/iteratively.
 * @param nodes Pointer to the node pool.
 * @param tree Root tree descriptor.
 * @param features Pointer to input feature array.
 * @return The leaf prediction value.
 */
float evaluateTree(const TreeNode* nodes, const DecisionTree& tree, const float* features);

/**
 * @brief Evaluates a full boosted ensemble.
 * @param ensemble The BoostedEnsemble to run inference on.
 * @param features Pointer to input feature array.
 * @return The accumulated ensemble prediction score.
 */
float evaluateEnsemble(const BoostedEnsemble& ensemble, const float* features);

/**
 * @brief Releases memory allocated for a dynamically loaded ensemble.
 * @param ensemble The ensemble to deallocate.
 */
void freeEnsemble(BoostedEnsemble& ensemble);

#endif // DECISION_TREE_ENGINE_H
