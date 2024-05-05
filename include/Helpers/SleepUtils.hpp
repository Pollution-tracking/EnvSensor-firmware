#ifndef SLEEP_UTILS_HPP
#define SLEEP_UTILS_HPP

struct SleepUtils {
private:
    bool allowSleep    = false; // Enable sleep mode
    bool sleepCooldown = false; // Cooldown after sleep
    bool sleepOver     = false; // Sleep mode over
    
public:
    void allow_sleep() {
        allowSleep = true;
    }

    void disable_sleep(bool sleep_over = false) {
        allowSleep = false;
        sleepOver = sleep_over;
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
        return sleepOver;
    }

    void mark_sleep_treated() {
        sleepOver = false;
    }
};

#endif // SLEEP_UTILS_HPP