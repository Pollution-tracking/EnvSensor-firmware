#ifndef SLEEP_UTILS_HPP
#define SLEEP_UTILS_HPP

struct SleepUtils {
private:
    bool allowSleep    = false; // True when sleep mode should start
    bool sleepCooldown = false; // True when starting cooldown after sleep wake-up caused by buttons
    bool sleepFinished = false; // True when sleep mode is over
    
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
};

#endif // SLEEP_UTILS_HPP