
// SensorTask: dedicated worker that executes sensor preheat/read cycles.
//
// Rationale:
// - Sensor hardware typically requires serialized access and certain
//   warmup sequences. This task centralizes those interactions so callers
//   (scheduler, main) can simply enqueue events without managing timing
//   or hardware state.

#include <Modules/SensorTask.h>

#include <Logger/logger.h>
#include <Modules/Scheduler/SensorScheduler.h>

#include <Modules/DisplayTask.h>
#include <Modules/Sensors.h>
#include <Resources/Software/DataHandler.h>

#include "freertos/semphr.h"

#define SENSOR_TASK_STACK_SIZE 6144
#define SENSOR_QUEUE_LENGTH 10
#define SENSOR_TASK_CORE 1

#define logg(message) loggWithObj(message, "SENSOR_TASK")

// Internal task entry and handler.
static void sensorTask(void *pvParameters);
static void handleSensorEvent(const SensorClientEvent event);

// Queue and semaphore used to serialize sensor operations and allow callers
// to wait until the current sensor activity completes.
static QueueHandle_t sensorQueue;
static SemaphoreHandle_t sensorBusySemaphore;

TaskHandle_t sensorTaskHandle;

// Coalescing: when environmental and pollution reads happen nearly simultaneously
// (for example when a 15s pollution read aligns with a 60s environmental read),
// prefer the environmental display update and skip the pollution-triggered
// refresh to avoid double updates. Track the last environmental display time
// in milliseconds and skip pollution display commands if they occur within
// `ENV_DISPLAY_COALESCE_MS` of the environmental update.
static uint32_t lastEnvironmentalDisplayMs = 0;
#define ENV_DISPLAY_COALESCE_MS 2000

void createSensorTask() {
	logg("Creating sensor worker task");
	sensorQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorClientEvent));
	sensorBusySemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(sensorBusySemaphore);

	xTaskCreatePinnedToCore(
		sensorTask,
		"SensorTask",
		SENSOR_TASK_STACK_SIZE,
		NULL,
		1,
		&sensorTaskHandle,
		SENSOR_TASK_CORE
	);
}

void sendSensorCommand(const SensorClientEvent event) {
	// Scheduler and main loop both use this queue to serialize sensor work.
	xQueueSend(sensorQueue, &event, portMAX_DELAY);
}

void waitForSensorIdle() {
	// Take+Give a binary semaphore to wait until the worker returns to idle.
	xSemaphoreTake(sensorBusySemaphore, portMAX_DELAY);
	xSemaphoreGive(sensorBusySemaphore);
}

static void sensorTask(void *pvParameters) {
	SensorClientEvent receivedEvent;

	logg("Sensor worker task started");

	for (;;) {
		if (xQueueReceive(sensorQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
			// Keep at most one sensor operation active at a time.
			xSemaphoreTake(sensorBusySemaphore, portMAX_DELAY);
			handleSensorEvent(receivedEvent);
			xSemaphoreGive(sensorBusySemaphore);
		}
	}
}

// HandleSensorEvent: central dispatcher for client events. Each client
// uses the PHASE_PREHEAT and PHASE_READ phases; the handler executes the
// appropriate hardware sequence and triggers higher-level consumers (display,
// data handler) when readings complete.
static void handleSensorEvent(const SensorClientEvent event) {
	// Event model: each client emits PHASE_PREHEAT and PHASE_READ events.
	switch (event.client) {
		case CLIENT_BOOT:
			switch (event.phase) {
				case PHASE_INIT:
					logg("Handling boot init event");
					boardUtilities.enableBuck();
					initializeSensors();
					startSensorScheduler();
					break;
				default:
					break;
			}
			break;

		case CLIENT_POLLUTION:
			switch (event.phase) {
				case PHASE_PREHEAT:
					logg("Handling pollution preheat event");
					preparePollutionSensors();
					break;
				case PHASE_READ: {
					logg("Handling pollution read event");
					readPollutionSensors();
					sleepPollutionSensors();

					if (board_config.board_state == SLEEP_STATE::FROM_SLEEP) {
						board_config.board_state = SLEEP_STATE::TO_SLEEP;
						board_config.to_treat = true;
					}

					handleLiveData();

					// If an environmental read recently updated the display, skip the
					// pollution-triggered refresh to avoid duplicate updates.
					if ((millis() - lastEnvironmentalDisplayMs) > ENV_DISPLAY_COALESCE_MS) {
						DisplayCommand cmd_refresh = {CMD_REFRESH, {}};
						cmd_refresh.payload.screen_refresh = SCREEN_REFRESH::SENSORS;
						sendDisplayCommand(cmd_refresh);
					} else {
						// Coalesced: environmental read has recently refreshed the UI.
						logg("Skipping pollution display refresh (coalesced)");
					}
					break;
				}
				default:
					break;
			}
			break;

		case CLIENT_ENVIRONMENTAL:
			switch (event.phase) {
				case PHASE_PREHEAT:
					logg("Handling environmental preheat event");
					prepareEnvironmentalSensors();
					break;
				case PHASE_READ: {
					logg("Handling environmental read event");
					readEnvironmentalSensors();
					sleepEnvironmentalSensors();

					if (board_config.board_state == SLEEP_STATE::FROM_SLEEP) {
						board_config.board_state = SLEEP_STATE::TO_SLEEP;
						board_config.to_treat = true;
					}

					handleLiveData();

					// Environmental read is the authoritative full-update. Record the
					// display timestamp so closely following pollution reads can skip
					// their refresh to avoid double updates.
					lastEnvironmentalDisplayMs = millis();

					DisplayCommand cmd_set_mode = {CMD_SET_MODE, {}};
					cmd_set_mode.payload.screen_mode = SCREEN_MODE::SENSORS;
					sendDisplayCommand(cmd_set_mode);
					break;
				}
				default:
					break;
			}
			break;

		default:
			break;
	}
}