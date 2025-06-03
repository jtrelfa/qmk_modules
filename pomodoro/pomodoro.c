#include QMK_KEYBOARD_H
#include "pomodoro.h"
#ifdef CONSOLE_ENABLE
#    include "print.h"
#endif
ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 1, 0);

struct pom_settings {
    uint8_t            pom_original_mode;
    uint8_t            original_speed;
    uint8_t            original_hue;
    uint8_t            original_sat;
    uint8_t            original_val;
    bool               original_values_saved;
    bool               timer_running;
    uint32_t           timer_start;
    uint32_t           last_flash_time;
    enum pom_led_state current_led_state;
    bool               led_on;
    bool               rgb_matrix_was_enabled;
    int                total_cycle_duration;
};
struct pom_settings pom;
// Store original RGB matrix mode and settings
pom.original_mode         = 0;
pom.original_speed        = 0;
pom.original_hue          = 0;
pom.original_sat          = 0;
pom.original_val          = 0;
pom.original_values_saved = false;

// Status variables
pom.timer_running          = false;
pom.timer_start            = 0;
pom.last_flash_time        = 0;
pom.current_led_state      = POM_LED_STATE_OFF;
pom.led_on                 = false;
pom.rgb_matrix_was_enabled = false;
pom.total_cycle_duration   = POM_PREPARE_FLASH_DURATION_MS + POM_WORKING_DURATION_MS + POM_DONE_WORKING_FLASH_DURATION_MS + POM_RESTING_DURATION_MS;

// Save the original RGB matrix state
void pom_save_rgb_matrix_state(void) {
    if (!pom.original_values_saved) {
        pom.rgb_matrix_was_enabled = rgb_matrix_is_enabled();

        if (pom.rgb_matrix_was_enabled) {
            // Save the current RGB matrix mode and settings
            pom.original_mode  = rgb_matrix_get_mode();
            pom.original_speed = rgb_matrix_get_speed();
            pom.original_hue   = rgb_matrix_get_hue();
            pom.original_sat   = rgb_matrix_get_sat();
            pom.original_val   = rgb_matrix_get_val();
        }

        pom.original_values_saved = true;
    }
};

// Restore the original RGB matrix state
void pom_restore_rgb_matrix_state(void) {
    if (pom.original_values_saved) {
        if (pom.rgb_matrix_was_enabled) {
            rgb_matrix_mode(pom.original_mode);
            rgb_matrix_set_speed(pom.original_speed);
            rgb_matrix_sethsv(pom.original_hue, pom.original_sat, pom.original_val);
            rgb_matrix_enable();
        } else {
            rgb_matrix_disable();
        }

        pom.original_values_saved = false;
    }
};

// Get current state based on elapsed time
enum pom_led_state pom_get_current_state(uint32_t elapsed_time) {
    uint32_t cycle_position = elapsed_time % pom.total_cycle_duration;

    if (cycle_position < POM_PREPARE_FLASH_DURATION_MS) {
        return POM_LED_STATE_PREPARE_FLASHING;
    } else if (cycle_position < (POM_PREPARE_FLASH_DURATION_MS + POM_WORKING_DURATION_MS)) {
        return POM_LED_STATE_WORKING;
    } else if (cycle_position < (POM_PREPARE_FLASH_DURATION_MS + POM_WORKING_DURATION_MS + POM_DONE_WORKING_FLASH_DURATION_MS)) {
        return POM_LED_STATE_DONE_WORKING_FLASHING;
    } else {
        return POM_LED_STATE_RESTING;
    }
};

bool pom_toggle_led_timer(void) {
    if (pom.timer_running) {
        // Stop timer and restore original color
        pom.timer_running     = false;
        pom.current_led_state = POM_LED_STATE_OFF;
        pom_restore_rgb_matrix_state();
        return false;
    } else {
        // Start timer and save original state
        pom_save_rgb_matrix_state();

        // gotta make sure it is enabled
        rgb_matrix_enable();

        pom.timer_running     = true;
        pom.timer_start       = timer_read32();
        pom.last_flash_time   = pom.timer_start;
        pom.current_led_state = POM_LED_STATE_PREPARE_FLASHING;
        pom.led_on            = true;
        return true;
    }
};

bool process_keycode_pomodoro_toggle(keyrecord_t *record) {
    if (record->event.pressed) {
#ifdef CONSOLE_ENABLE
        print("^^ PROCESS KEYCODE POMODORO TIMER\n");
#endif
        pom_toggle_led_timer();
        return true;
    }
    return false;
}

bool process_record_pomodoro(uint16_t keycode, keyrecord_t *record) {
    // if (!process_record_pomodoro_kb(keycode, record)) {
    //   return false;
    // }
    switch (keycode) {
        case COMMUNITY_MODULE_POMODORO_TIMER:
#ifdef CONSOLE_ENABLE
            print("^^ COMMUNITY_MODULE_POMODORO_TIMER\n");
#endif
            return process_keycode_pomodoro_toggle(record);
            break;
    }
    return true;
};

bool rgb_matrix_indicators_pomodoro(void) {
    if (!pom.timer_running) {
        return false;
    }
    uint32_t current_time = timer_read32();
    uint32_t elapsed_time = current_time - pom.timer_start;

    // Determine current state
    enum pom_led_state new_state = pom_get_current_state(elapsed_time);

    // Handle state transitions
    if (new_state != pom.current_led_state) {
        pom.current_led_state = new_state;
        pom.led_on            = true; // Reset flash state on transitions
    }

    // Handle flashing for flashing states
    if (pom.current_led_state == POM_LED_STATE_PREPARE_FLASHING || pom.current_led_state == POM_LED_STATE_DONE_WORKING_FLASHING) {
        if (current_time - pom.last_flash_time >= POM_FLASH_INTERVAL_MS) {
            pom.led_on          = !pom.led_on;
            pom.last_flash_time = current_time;
        }
    }

    // Set the color based on current state
    switch (pom.current_led_state) {
        case POM_LED_STATE_PREPARE_FLASHING:
            if (pom.led_on) {
                rgb_matrix_set_color(POM_KEY_INDEX, POM_PREPARE_COLOR);
            } else {
                rgb_matrix_set_color(POM_KEY_INDEX, RGB_OFF);
            }
            break;

        case POM_LED_STATE_WORKING:
            rgb_matrix_set_color(POM_KEY_INDEX, POM_WORKING_COLOR);
            break;

        case POM_LED_STATE_DONE_WORKING_FLASHING:
            if (pom.led_on) {
                rgb_matrix_set_color(POM_KEY_INDEX, POM_DONE_WORKING_COLOR);
            } else {
                rgb_matrix_set_color(POM_KEY_INDEX, RGB_OFF);
            }
            break;

        case POM_LED_STATE_RESTING:
            rgb_matrix_set_color(POM_KEY_INDEX, POM_RESTING_COLOR);
            break;

        default:
            rgb_matrix_set_color(POM_KEY_INDEX, RGB_OFF);
            break;
    }
    return true;
};
