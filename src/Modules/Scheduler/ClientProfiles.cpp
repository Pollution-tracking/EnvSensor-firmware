#include <Modules/Scheduler/ClientProfiles.h>

#include <Logger/logger.h>
#include <Modules/Scheduler/ClientRegistry.h>

#define logg(message) loggWithObj(message, "CLIENT_PROFILES")

// This function is called once during setup after wakeup reason is known.
// It mutates the retained client registry with the default runtime profiles.
//
// fromSleep semantics:
// - fromSleep == false: cold boot / reset path
//   We allow clients to request a boot-time preheat when needed.
// - fromSleep == true: resumed from deep sleep
//   We skip boot-time preheat because sensors were just used in the sleep cycle
//   and we want to avoid an unnecessary immediate warmup event.
//
// The scheduler still performs normal periodic preheat/read events later based on
// each profile interval and preheat offset. This flag only affects startup behavior.
void applyDefaultSensorClientProfiles(bool fromSleep) {
	logg("Applying default client profiles");

	// Extension points:
	// - Add a "slow background" profile for battery-only updates
	// - Add a "diagnostic burst" profile for temporary high-frequency sampling
	// - Add a "ble-streaming" profile when phone is connected
	#if defined(PM_ENABLE) || defined(CO2_ENABLE) || defined(MICS_ENABLE)
	// Pollution profile:
	// - read every 15s
	// - request preheat 5s before read
	// - boot-time preheat only on cold boot (!fromSleep)
	const SensorClientDefinition pollution = {CLIENT_POLLUTION, 15, 5, !fromSleep};
	clientRegistryUpsert(&pollution);
	logg("Registered pollution client profile");
	#endif

	#if defined(BME_ENABLE) || defined(SHTC3_ENABLE)
	// Environmental profile:
	// - read every 60s
	// - request preheat 5s before read
	// - boot-time preheat only on cold boot (!fromSleep)
	const SensorClientDefinition environmental = {CLIENT_ENVIRONMENTAL, 60, 5, !fromSleep};
	clientRegistryUpsert(&environmental);
	logg("Registered environmental client profile");
	#endif
}