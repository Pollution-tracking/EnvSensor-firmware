#ifndef CLIENT_REGISTRY_H
#define CLIENT_REGISTRY_H

#include <Arduino.h>

#include <Resources/Constants/scheduler_constants.h>

// ClientRegistry provides a small retained registry of sensor client profiles.
// The storage is kept in RTC memory (RTC_DATA_ATTR) so profiles survive deep
// sleep cycles. The registry supports simple mutation semantics (Upsert/Remove)
// and exposes a generation counter which callers (the scheduler) can poll to
// detect changes and rebuild runtime state.
//
// Thread-safety: callers should perform mutations from a single context when
// possible. The scheduler detects registry changes via the generation value.

// Initialize or validate the retained registry (must be called once at boot).
void clientRegistryInit();

// Upsert a client definition. Returns true on success.
bool clientRegistryUpsert(const SensorClientDefinition *definition);

// Remove a client by id. Returns true on success.
bool clientRegistryRemove(ClientId client);

// Retrieve a client definition by id. Returns false if not found.
bool clientRegistryGet(ClientId client, SensorClientDefinition *definition);

// Enumeration helpers: number of active clients and indexed access.
size_t clientRegistryCount();
bool clientRegistryAt(size_t index, SensorClientDefinition *definition);

// Generation counter increments on each mutation and can be used to detect
// changes without reading the full registry content.
uint32_t clientRegistryGeneration();

#endif // CLIENT_REGISTRY_H