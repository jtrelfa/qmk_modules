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
enum pom_led_state pom_get_current_state(uint32_t elapsed_time);
void pom_save_rgb_matrix_state(void);
void pom_restore_rgb_matrix_state(void);
bool pom_toggle_led_timer(void);
bool process_keycode_pomodoro_toggle(keyrecord_t *record);
bool process_record_pomodoro(uint16_t keycode, keyrecord_t *record);
