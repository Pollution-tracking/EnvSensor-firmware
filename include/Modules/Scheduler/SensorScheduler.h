#ifndef SCHEDULER_SENSOR_SCHEDULER_H
#define SCHEDULER_SENSOR_SCHEDULER_H

#include <Arduino.h>

#include <Modules/Scheduler/ClientRegistry.h>

// SensorScheduler runs a simple 1-second tick to track elapsed time for each
// registered client. It translates client profiles (interval + preheat offset)
// into PHASE_PREHEAT and PHASE_READ events that are sent to the sensor worker.
//
// The scheduler rebuilds its runtime view when the ClientRegistry generation
// changes, allowing dynamic updates to profiles at runtime.
//
// SensorClientRuntime: runtime-only fields used by the scheduler to track
// elapsedSeconds and whether the preheat event was already issued for the
// current cycle.
typedef struct {
	SensorClientDefinition definition;
	uint32_t elapsedSeconds;
	bool preheatIssued;
} SensorClientRuntime;

void createSensorScheduler();
void startSensorScheduler();
void stopSensorScheduler();

#endif // SCHEDULER_SENSOR_SCHEDULER_H