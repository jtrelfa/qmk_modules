#pragma once

#include "quantum.h"
#include "color.h"

// Define the key states for LED control
enum pom_led_state {
    POM_LED_STATE_OFF,                   // LED off (original state)
    POM_LED_STATE_PREPARE_FLASHING,      // prepare to work flashing phase
    POM_LED_STATE_WORKING,               // Working phase (solid color)
    POM_LED_STATE_DONE_WORKING_FLASHING, // almost done working flashing phase
    POM_LED_STATE_RESTING                // Resting phase (solid color)
};

// Default values if not configured
// timers are in milliseconds
#ifndef POM_FLASH_INTERVAL_MS
#    define POM_FLASH_INTERVAL_MS 1000
#endif
#ifndef POM_PREPARE_FLASH_DURATION_MS
#    define POM_PREPARE_FLASH_DURATION_MS 10000
#endif
#ifndef POM_WORKING_DURATION_MS
#    define POM_WORKING_DURATION_MS 1490000
#endif
#ifndef POM_DONE_WORKING_FLASH_DURATION_MS
#    define POM_DONE_WORKING_FLASH_DURATION_MS 10000
#endif
#ifndef POM_RESTING_DURATION_MS
#    define POM_RESTING_DURATION_MS 290000
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
#ifndef POM_KEY_INDEX
#    define POM_KEY_INDEX 6
#endif

enum pom_led_state pom_get_current_state(uint32_t elapsed_time);
void pom_save_rgb_matrix_state(void);
void pom_restore_rgb_matrix_state(void);
bool pom_toggle_led_timer(void);
bool process_keycode_pomodoro_toggle(keyrecord_t *record);
bool process_record_pomodoro(uint16_t keycode, keyrecord_t *record);
bool rgb_matrix_indicators_pomodoro(void);
