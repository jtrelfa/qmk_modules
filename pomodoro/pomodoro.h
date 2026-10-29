// Copyright 2025 Jon Trelfa (@jtrelfa)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "quantum.h"
#include "color.h"

// Phases of a pomodoro cycle
enum pom_led_state {
    POM_LED_STATE_OFF,                   // Timer stopped, key shows its normal lighting
    POM_LED_STATE_PREPARE_FLASHING,      // Prepare to work (flashing)
    POM_LED_STATE_WORKING,               // Working phase (solid color)
    POM_LED_STATE_DONE_WORKING_FLASHING, // Almost done working (flashing)
    POM_LED_STATE_RESTING                // Resting phase (solid color)
};

// Default values if not configured
// Each work and rest period ends with a 10 second flashing countdown, which is included in its minutes
#ifndef POM_WORK_MINUTES
#    define POM_WORK_MINUTES 25
#endif
#ifndef POM_REST_MINUTES
#    define POM_REST_MINUTES 5
#endif
#ifndef POM_PREPARE_COLOR
#    define POM_PREPARE_COLOR RGB_YELLOW
#endif
#ifndef POM_WORKING_COLOR
#    define POM_WORKING_COLOR RGB_GREEN
#endif
#ifndef POM_DONE_WORKING_COLOR
#    define POM_DONE_WORKING_COLOR RGB_RED
#endif
#ifndef POM_RESTING_COLOR
#    define POM_RESTING_COLOR RGB_BLUE
#endif
// POM_KEY_INDEX is optional: leave it undefined to light whichever key started the timer

// Start (or restart) the timer on a specific LED. Returns false if the index is not a valid LED.
bool pomodoro_start(uint8_t led_index);
// Stop the timer and give the LED back to the current RGB effect
void pomodoro_stop(void);
bool pomodoro_is_running(void);
enum pom_led_state pomodoro_get_state(void);

// Toggle the timer from a key press, lighting the key that was pressed.
// Call this from process_record_user to drive the timer from your own keycode (e.g. for VIA).
bool process_keycode_pomodoro_toggle(keyrecord_t *record);
