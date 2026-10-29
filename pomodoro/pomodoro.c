// Copyright 2025 Jon Trelfa (@jtrelfa)
// SPDX-License-Identifier: GPL-3.0-or-later

#include QMK_KEYBOARD_H
#include "pomodoro.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 1, 0);

#ifndef RGB_MATRIX_ENABLE
#    error "Pomodoro Timer requires RGB Matrix (RGB_MATRIX_ENABLE = yes)"
#endif

// Timing used to be set in milliseconds. Those settings are no longer read, so stop instead of silently ignoring them.
#if defined(POM_FLASH_INTERVAL_MS) || defined(POM_PREPARE_FLASH_DURATION_MS) || defined(POM_WORKING_DURATION_MS) || defined(POM_DONE_WORKING_FLASH_DURATION_MS) || defined(POM_RESTING_DURATION_MS) || \
    defined(POM_FLASH_INTERVAL) || defined(POM_PREPARE_FLASH_DURATION) || defined(POM_WORKING_DURATION) || defined(POM_DONE_WORKING_FLASH_DURATION) || defined(POM_RESTING_DURATION)
#    error "Pomodoro Timer: timing is now set in whole minutes with POM_WORK_MINUTES and POM_REST_MINUTES. Remove the old POM_*_DURATION / POM_FLASH_INTERVAL settings."
#endif

// The upper limit keeps the cycle length in milliseconds well inside a uint32_t
_Static_assert((POM_WORK_MINUTES) >= 1 && (POM_WORK_MINUTES) <= 1440, "Pomodoro Timer: POM_WORK_MINUTES must be between 1 and 1440");
_Static_assert((POM_REST_MINUTES) >= 1 && (POM_REST_MINUTES) <= 1440, "Pomodoro Timer: POM_REST_MINUTES must be between 1 and 1440");
#ifdef POM_KEY_INDEX
_Static_assert((POM_KEY_INDEX) >= 0 && (POM_KEY_INDEX) < RGB_MATRIX_LED_COUNT, "Pomodoro Timer: POM_KEY_INDEX must be less than RGB_MATRIX_LED_COUNT");
#endif

#define POM_FLASH_INTERVAL_MS 1000
#define POM_COUNTDOWN_MS 10000 // flashing countdown at the end of each work and rest period

// One cycle is: get ready countdown -> working -> almost done countdown -> resting.
// The countdowns come out of the solid phases so work and rest add up to exactly the minutes asked for.
// The get ready countdown belongs to the rest period, so the very first press starts with it as a warm up.
// Cast to uint32_t so the math can't overflow on platforms with a 16-bit int (AVR).
#define POM_WORKING_START ((uint32_t)POM_COUNTDOWN_MS)
#define POM_DONE_WORKING_START ((uint32_t)(POM_WORK_MINUTES) * 60000)
#define POM_RESTING_START (POM_DONE_WORKING_START + POM_COUNTDOWN_MS)
#define POM_TOTAL_CYCLE_DURATION (((uint32_t)(POM_WORK_MINUTES) + (POM_REST_MINUTES)) * 60000)

static struct {
    bool               timer_running;
    bool               rgb_enabled_by_us; // RGB was off at start, so we turned it on and keep the rest of the board dark
    bool               led_on;            // flash on/off phase, only used by the flashing states
    uint8_t            led_index;
    uint32_t           timer_start;
    enum pom_led_state current_led_state;
} pom = {0};

// NO_DEBUG is defined when the console is disabled; dprintf() compiles to nothing then, so this would be unused
#ifndef NO_DEBUG
static const char *const pom_state_names[] = {
    [POM_LED_STATE_OFF]                   = "off",
    [POM_LED_STATE_PREPARE_FLASHING]      = "prepare to work",
    [POM_LED_STATE_WORKING]               = "working",
    [POM_LED_STATE_DONE_WORKING_FLASHING] = "almost done working",
    [POM_LED_STATE_RESTING]               = "resting",
};
#endif

// Work out the phase and flash state from how far into the cycle we are.
// Deriving both from the start time (instead of toggling on a timer) keeps the flashing from drifting.
static void pom_update_state(void) {
    uint32_t           cycle_position = timer_elapsed32(pom.timer_start) % POM_TOTAL_CYCLE_DURATION;
    uint32_t           phase_start;
    enum pom_led_state new_state;

    if (cycle_position < POM_WORKING_START) {
        new_state   = POM_LED_STATE_PREPARE_FLASHING;
        phase_start = 0;
    } else if (cycle_position < POM_DONE_WORKING_START) {
        new_state   = POM_LED_STATE_WORKING;
        phase_start = POM_WORKING_START;
    } else if (cycle_position < POM_RESTING_START) {
        new_state   = POM_LED_STATE_DONE_WORKING_FLASHING;
        phase_start = POM_DONE_WORKING_START;
    } else {
        new_state   = POM_LED_STATE_RESTING;
        phase_start = POM_RESTING_START;
    }

    // Every phase starts with the LED on, then alternates each interval
    pom.led_on = ((cycle_position - phase_start) / (POM_FLASH_INTERVAL_MS)) % 2 == 0;

    if (new_state != pom.current_led_state) {
        pom.current_led_state = new_state;
        dprintf("Pomodoro: %s\n", pom_state_names[new_state]);
    }
}

// Find the LED under the key that sent this record, or NO_LED if there isn't one
static uint8_t pom_led_index_for_record(const keyrecord_t *record) {
#ifdef POM_KEY_INDEX
    (void)record;
    return POM_KEY_INDEX;
#else
    // Combos, encoder map turns, etc. don't come from a physical key, so their row/col mean nothing here
    if (!IS_KEYEVENT(record->event)) {
        return NO_LED;
    }
    keypos_t key = record->event.key;
    if (key.row >= MATRIX_ROWS || key.col >= MATRIX_COLS) {
        return NO_LED;
    }
    return g_led_config.matrix_co[key.row][key.col];
#endif
}

// Indicator colors bypass the effect brightness, so cap them at the keyboard's maximum
static void pom_set_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    rgb_matrix_set_color(index, (uint16_t)red * RGB_MATRIX_MAXIMUM_BRIGHTNESS / UINT8_MAX, (uint16_t)green * RGB_MATRIX_MAXIMUM_BRIGHTNESS / UINT8_MAX, (uint16_t)blue * RGB_MATRIX_MAXIMUM_BRIGHTNESS / UINT8_MAX);
}

bool pomodoro_start(uint8_t led_index) {
    if (led_index >= RGB_MATRIX_LED_COUNT) {
        dprintf("Pomodoro: LED index %u is out of range (0-%u), not starting\n", led_index, RGB_MATRIX_LED_COUNT - 1);
        return false;
    }

    // Indicators only render while RGB Matrix is on. Use noeeprom so the saved setting is left alone.
    if (!pom.timer_running && !rgb_matrix_is_enabled()) {
        rgb_matrix_enable_noeeprom();
        pom.rgb_enabled_by_us = true;
    }

    pom.timer_running     = true;
    pom.led_index         = led_index;
    pom.timer_start       = timer_read32();
    pom.current_led_state = POM_LED_STATE_OFF;
    dprintf("Pomodoro: started on LED %u\n", led_index);
    pom_update_state();
    return true;
}

void pomodoro_stop(void) {
    if (!pom.timer_running) {
        return;
    }

    pom.timer_running     = false;
    pom.current_led_state = POM_LED_STATE_OFF;
    if (pom.rgb_enabled_by_us) {
        rgb_matrix_disable_noeeprom();
        pom.rgb_enabled_by_us = false;
    }
    dprintf("Pomodoro: stopped\n");
}

bool pomodoro_is_running(void) {
    return pom.timer_running;
}

enum pom_led_state pomodoro_get_state(void) {
    return pom.current_led_state;
}

bool process_keycode_pomodoro_toggle(keyrecord_t *record) {
    if (record->event.pressed) {
        if (pom.timer_running) {
            pomodoro_stop();
        } else {
            uint8_t led_index = pom_led_index_for_record(record);
            if (led_index == NO_LED) {
                dprintf("Pomodoro: key at row %u, col %u has no RGB LED, not starting (define POM_KEY_INDEX to choose one)\n", record->event.key.row, record->event.key.col);
            } else {
                pomodoro_start(led_index);
            }
        }
    }
    // Handled here on both press and release; there is nothing to send to the host
    return false;
}

bool process_record_pomodoro(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_pomodoro_kb(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case COMMUNITY_MODULE_POMODORO_TIMER:
            return process_keycode_pomodoro_toggle(record);
    }
    return true;
}

// Timing runs here rather than in the indicator callback so it keeps going even when RGB is not rendering
void housekeeping_task_pomodoro(void) {
    housekeeping_task_pomodoro_kb();

    if (pom.timer_running) {
        pom_update_state();
    }
}

bool rgb_matrix_indicators_pomodoro(void) {
    if (!rgb_matrix_indicators_pomodoro_kb()) {
        return false;
    }
    // Always return true when we're done: returning false would skip the indicators of modules listed after this one
    if (!pom.timer_running) {
        return true;
    }

    if (pom.rgb_enabled_by_us) {
        rgb_matrix_set_color_all(RGB_OFF);
    }

    switch (pom.current_led_state) {
        case POM_LED_STATE_PREPARE_FLASHING:
            if (pom.led_on) {
                pom_set_color(pom.led_index, POM_PREPARE_COLOR);
            } else {
                pom_set_color(pom.led_index, RGB_OFF);
            }
            break;

        case POM_LED_STATE_WORKING:
            pom_set_color(pom.led_index, POM_WORKING_COLOR);
            break;

        case POM_LED_STATE_DONE_WORKING_FLASHING:
            if (pom.led_on) {
                pom_set_color(pom.led_index, POM_DONE_WORKING_COLOR);
            } else {
                pom_set_color(pom.led_index, RGB_OFF);
            }
            break;

        case POM_LED_STATE_RESTING:
            pom_set_color(pom.led_index, POM_RESTING_COLOR);
            break;

        default:
            break;
    }
    return true;
}
