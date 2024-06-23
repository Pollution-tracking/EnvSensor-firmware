#ifndef SLEEP_UTILS_HPP
#define SLEEP_UTILS_HPP

struct SleepUtils {
private:
    bool allowSleep     = false; // True when sleep mode should start
    bool sleepCooldown  = false; // True when starting cooldown after sleep wake-up caused by buttons
    bool sleepFinished  = false; // True when sleep mode is over
    bool sleepAfterRead = false; // True when sleep mode should start after sensor readings
    
public:
    void allow_sleep() {
        allowSleep = true;
    }

    void disable_sleep(bool sleep_finished = false) {
        allowSleep = false;
        sleepFinished = sleep_finished;
    }

    bool is_sleep_allowed() {
        return allowSleep;
    }

    void enable_cooldown() {
        sleepCooldown = true;
    }

    void disable_cooldown() {
        sleepCooldown = false;
    }

    bool is_cooldown_enabled() {
        return sleepCooldown;
    }

    bool is_sleep_over() {
        return sleepFinished;
    }

    void mark_sleep_treated() {
        sleepFinished = false;
    }

    void allow_sleep_after_read() {
        sleepAfterRead = true;
    }

    bool should_sleep_after_read() {
        return sleepAfterRead;
    }
};

#endif // SLEEP_UTILS_HPP