#include <Modules/Scheduler/ClientRegistry.h>

#include <Logger/logger.h>

#define logg(message) loggWithObj(message, "CLIENT_REGISTRY")

// ClientRegistry implementation: retained client profiles stored in RTC memory.
//
// Storage layout:
// - magic: sanity check value to determine whether the RTC storage contains
//   a valid registry from a previous boot.
// - generation: incremented on each mutation so consumers can detect changes
//   cheaply without reading full content.
// - count: current number of active entries.
// - clients: compacted array of SensorClientDefinition entries.

typedef struct {
    uint32_t magic;
    uint32_t generation;
    size_t count;
    SensorClientDefinition clients[CLIENT_REGISTRY_MAX_CLIENTS];
} ClientRegistryStorage;

// Place the registry in RTC memory so it survives deep sleep cycles.
static RTC_DATA_ATTR ClientRegistryStorage registryStorage = {
    .magic = 0,
    .generation = 0,
    .count = 0,
    .clients = {{CLIENT_NONE, 0, 0, false}, {CLIENT_NONE, 0, 0, false}, {CLIENT_NONE, 0, 0, false}, {CLIENT_NONE, 0, 0, false}},
};

static void compactRegistry();
static int findClientIndex(ClientId client);
static int findFreeSlot();

// Validate or initialize retained storage on first use.
void clientRegistryInit() {
    if (registryStorage.magic == CLIENT_REGISTRY_MAGIC) {
        // Already initialized in retained RTC memory; skip re-initialization.
        // Avoid noisy logging here because this function is called frequently
        // by runtime code paths (scheduler polling the generation counter).
        return;
    }

    logg("Initializing retained client registry");
    registryStorage.magic = CLIENT_REGISTRY_MAGIC;
    registryStorage.generation = 0;
    registryStorage.count = 0;
    for (size_t i = 0; i < CLIENT_REGISTRY_MAX_CLIENTS; i++) {
        registryStorage.clients[i] = {CLIENT_NONE, 0, 0, false};
    }
}

// Insert or update a client definition.
bool clientRegistryUpsert(const SensorClientDefinition *definition) {
    if (definition == NULL || definition->client == CLIENT_NONE) {
        return false;
    }

    clientRegistryInit();

    int index = findClientIndex(definition->client);
    if (index < 0) {
        index = findFreeSlot();
    }

    if (index < 0) {
        return false;
    }

    registryStorage.clients[index] = *definition;
    compactRegistry();
    registryStorage.generation++;
    logg("Client upserted in registry");
    return true;
}

// Mark a client slot free and compact the array.
bool clientRegistryRemove(ClientId client) {
    clientRegistryInit();

    int index = findClientIndex(client);
    if (index < 0) {
        return false;
    }

    registryStorage.clients[index].client = CLIENT_NONE;
    registryStorage.clients[index].intervalSeconds = 0;
    registryStorage.clients[index].preheatOffsetSeconds = 0;
    registryStorage.clients[index].primeOnStart = false;
    compactRegistry();
    registryStorage.generation++;
    logg("Client removed from registry");
    return true;
}

// Retrieve a specific client definition.
bool clientRegistryGet(ClientId client, SensorClientDefinition *definition) {
    if (definition == NULL) {
        return false;
    }

    clientRegistryInit();

    int index = findClientIndex(client);
    if (index < 0) {
        return false;
    }

    *definition = registryStorage.clients[index];
    return true;
}

// Return compacted element count.
size_t clientRegistryCount() {
    clientRegistryInit();
    return registryStorage.count;
}

// Indexed access into the compacted array.
bool clientRegistryAt(size_t index, SensorClientDefinition *definition) {
    if (definition == NULL) {
        return false;
    }

    clientRegistryInit();

    size_t seen = 0;
    for (size_t i = 0; i < CLIENT_REGISTRY_MAX_CLIENTS; i++) {
        if (registryStorage.clients[i].client == CLIENT_NONE) {
            continue;
        }

        if (seen == index) {
            *definition = registryStorage.clients[i];
            return true;
        }

        seen++;
    }

    return false;
}

uint32_t clientRegistryGeneration() {
    clientRegistryInit();
    return registryStorage.generation;
}

// Compact the registry into a dense array, update count.
static void compactRegistry() {
    SensorClientDefinition compacted[CLIENT_REGISTRY_MAX_CLIENTS] = {{CLIENT_NONE, 0, 0, false}, {CLIENT_NONE, 0, 0, false}, {CLIENT_NONE, 0, 0, false}, {CLIENT_NONE, 0, 0, false}};
    size_t compactedCount = 0;

    for (size_t i = 0; i < CLIENT_REGISTRY_MAX_CLIENTS; i++) {
        if (registryStorage.clients[i].client == CLIENT_NONE) {
            continue;
        }

        if (compactedCount < CLIENT_REGISTRY_MAX_CLIENTS) {
            compacted[compactedCount++] = registryStorage.clients[i];
        }
    }

    registryStorage.count = compactedCount;
    for (size_t i = 0; i < CLIENT_REGISTRY_MAX_CLIENTS; i++) {
        registryStorage.clients[i] = compacted[i];
    }
}

// Helpers to locate entries.
static int findClientIndex(ClientId client) {
    for (size_t i = 0; i < CLIENT_REGISTRY_MAX_CLIENTS; i++) {
        if (registryStorage.clients[i].client == client) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

static int findFreeSlot() {
    for (size_t i = 0; i < CLIENT_REGISTRY_MAX_CLIENTS; i++) {
        if (registryStorage.clients[i].client == CLIENT_NONE) {
            return static_cast<int>(i);
        }
    }

    return -1;
}
