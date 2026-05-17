#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include <Arduino.h>

#include <Resources/Constants/scheduler_constants.h>
// The sensor worker consumes SensorClientEvent messages produced by the
// SensorScheduler. It runs on a dedicated FreeRTOS task and serializes
// access to sensor hardware so only one sensor operation runs at a time.
//
// Public API:
// - createSensorTask(): create queue, semaphore and start the worker task.
// - sendSensorCommand(event): enqueue a SensorClientEvent (preheat/read).
// - waitForSensorIdle(): block until any in-progress sensor operation finishes.

#include <Modules/Scheduler/ClientRegistry.h>

void createSensorTask();
void sendSensorCommand(const SensorClientEvent event);
void waitForSensorIdle();

#endif // SENSOR_TASK_H