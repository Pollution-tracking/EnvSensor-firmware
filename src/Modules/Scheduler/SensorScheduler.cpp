// SensorScheduler: simple 1-second tick scheduler that converts retained
// SensorClientDefinition profiles into timed preheat/read events.
//
// Design notes:
// - The scheduler keeps a compact runtime view (SensorClientRuntime) derived
//   from the retained ClientRegistry. It monitors the registry's generation
//   counter and rebuilds its view when profiles change.
// - Each runtime entry tracks elapsedSeconds and whether the preheat event
//   has been issued for the current cycle so preheat/read ordering is correct.

#include <Modules/Scheduler/SensorScheduler.h>

#include <Logger/logger.h>
#include <Modules/SensorTask.h>

#define logg(message) loggWithObj(message, "SCHEDULER")

static void sensorSchedulerTask(void *pvParameters);
static void rebuildRuntimeClients();
static void dispatchBootWarmup();
static void scheduleClientTick(SensorClientRuntime &runtimeClient);

static bool schedulerEnabled = false;
static uint32_t registryGeneration = 0;
static SensorClientRuntime runtimeClients[CLIENT_REGISTRY_MAX_CLIENTS];
static size_t runtimeClientCount = 0;

TaskHandle_t sensorSchedulerTaskHandle;

void createSensorScheduler() {
	logg("Creating scheduler task");
	clientRegistryInit();
	registryGeneration = clientRegistryGeneration();
	rebuildRuntimeClients();

	xTaskCreatePinnedToCore(
		sensorSchedulerTask,
		"SensorScheduler",
		SENSOR_SCHEDULER_TASK_STACK,
		NULL,
		1,
		&sensorSchedulerTaskHandle,
		SENSOR_SCHEDULER_TASK_CORE
	);
}

void startSensorScheduler() {
	logg("Starting scheduler");
	schedulerEnabled = true;
	clientRegistryInit();
	rebuildRuntimeClients();
	dispatchBootWarmup();
}

void stopSensorScheduler() {
	logg("Stopping scheduler");
	schedulerEnabled = false;
}

// The scheduler runs an infinite loop with a 1 second tick. It increments
// elapsedSeconds for each runtime client and invokes scheduleClientTick to
// decide when to emit preheat/read events.
static void sensorSchedulerTask(void *pvParameters) {
	TickType_t lastWakeTime = xTaskGetTickCount();

	for (;;) {
		vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));

		if (!schedulerEnabled) {
			continue;
		}

		uint32_t currentGeneration = clientRegistryGeneration();
		if (currentGeneration != registryGeneration) {
			logg("Detected client registry update, rebuilding runtime schedule");
			registryGeneration = currentGeneration;
			rebuildRuntimeClients();
		}

		for (size_t i = 0; i < runtimeClientCount; i++) {
			runtimeClients[i].elapsedSeconds++;
			scheduleClientTick(runtimeClients[i]);
		}
	}
}

// Build a dense runtime view from retained registry entries.
static void rebuildRuntimeClients() {
	logg("Rebuilding runtime clients from retained registry");
	// Preserve previous runtime state when possible so transient registry
	// rebuilds (triggered by generation changes) don't cancel in-progress
	// preheat cycles. Copy the existing runtime table and attempt to reuse
	// elapsedSeconds/preheatIssued when the client id matches.
	SensorClientRuntime previousClients[CLIENT_REGISTRY_MAX_CLIENTS];
	size_t previousCount = runtimeClientCount;
	for (size_t i = 0; i < previousCount; i++) {
		previousClients[i] = runtimeClients[i];
	}

	runtimeClientCount = 0;

	for (size_t index = 0; index < clientRegistryCount(); index++) {
		SensorClientDefinition definition;
		if (!clientRegistryAt(index, &definition)) {
			continue;
		}

		if (runtimeClientCount >= CLIENT_REGISTRY_MAX_CLIENTS) {
			break;
		}

		// Default runtime state for a new entry.
		runtimeClients[runtimeClientCount].definition = definition;
		runtimeClients[runtimeClientCount].elapsedSeconds = 0;
		runtimeClients[runtimeClientCount].preheatIssued = false;

		// Try to find a matching previous client to preserve runtime fields.
		for (size_t p = 0; p < previousCount; p++) {
			if (previousClients[p].definition.client == definition.client) {
				runtimeClients[runtimeClientCount].elapsedSeconds = previousClients[p].elapsedSeconds;
				runtimeClients[runtimeClientCount].preheatIssued = previousClients[p].preheatIssued;
				break;
			}
		}

		runtimeClientCount++;
	}
}

// Optionally dispatch an immediate preheat for clients that requested a
// boot-time warmup (primeOnStart). Used to support cold-boot priming while
// avoiding unnecessary warmup after deep-sleep resumes.
static void dispatchBootWarmup() {
	for (size_t i = 0; i < runtimeClientCount; i++) {
		if (!runtimeClients[i].definition.primeOnStart) {
			continue;
		}

		logg("Dispatching startup preheat event");
		SensorClientEvent warmupEvent = {.client = runtimeClients[i].definition.client, .phase = PHASE_PREHEAT};
		sendSensorCommand(warmupEvent);
		runtimeClients[i].elapsedSeconds = 0;
		runtimeClients[i].preheatIssued = true;
	}
}

// Decide whether to emit preheat or read events for a runtime client based
// on its interval and preheat offset.
static void scheduleClientTick(SensorClientRuntime &runtimeClient) {
	// Guard against partially configured entries.
	if (runtimeClient.definition.client == CLIENT_NONE || runtimeClient.definition.intervalSeconds == 0) {
		return;
	}

	// Compute when the preheat event should happen in the current cycle.
	const uint32_t preheatMoment = (runtimeClient.definition.intervalSeconds > runtimeClient.definition.preheatOffsetSeconds)
		? (runtimeClient.definition.intervalSeconds - runtimeClient.definition.preheatOffsetSeconds)
		: runtimeClient.definition.intervalSeconds;

	if (!runtimeClient.preheatIssued && runtimeClient.elapsedSeconds == preheatMoment && runtimeClient.definition.preheatOffsetSeconds > 0) {
		logg("Dispatching scheduled preheat event");
		SensorClientEvent preheatEvent = {.client = runtimeClient.definition.client, .phase = PHASE_PREHEAT};
		sendSensorCommand(preheatEvent);
		runtimeClient.preheatIssued = true;
	}

	if (runtimeClient.elapsedSeconds >= runtimeClient.definition.intervalSeconds) {
		logg("Dispatching scheduled read event");
		SensorClientEvent readEvent = {.client = runtimeClient.definition.client, .phase = PHASE_READ};
		sendSensorCommand(readEvent);
		runtimeClient.elapsedSeconds = 0;
		runtimeClient.preheatIssued = false;
	}
}