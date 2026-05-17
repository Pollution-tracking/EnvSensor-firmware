#ifndef SCHEDULER_CONSTANTS_H
#define SCHEDULER_CONSTANTS_H

#include <Arduino.h>

typedef enum {
	CLIENT_BOOT = 0,
	CLIENT_POLLUTION = 1,
	CLIENT_ENVIRONMENTAL = 2,
	CLIENT_NONE = 255,
} ClientId;

typedef enum {
	PHASE_INIT = 0,
	PHASE_PREHEAT = 1,
	PHASE_READ = 2,
} ClientPhase;

typedef struct {
	ClientId client;
	ClientPhase phase;
} SensorClientEvent;

typedef struct {
	ClientId client;
	uint32_t intervalSeconds;
	uint32_t preheatOffsetSeconds;
	bool primeOnStart;
} SensorClientDefinition;

#define CLIENT_REGISTRY_MAX_CLIENTS 4
#define CLIENT_REGISTRY_MAGIC 0x434C5253U

#define SENSOR_SCHEDULER_TASK_STACK 4096
#define SENSOR_SCHEDULER_TASK_CORE 1

#endif // SCHEDULER_CONSTANTS_H