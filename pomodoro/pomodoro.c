#include QMK_KEYBOARD_H
#include "pomodoro.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 1, 0);

// Default values if not configured
// timers are in milliseconds
#ifndef POM_FLASH_INTERVAL
#define POM_FLASH_INTERVAL 1000
#endif
#ifndef POM_PREPARE_FLASH_DURATION
#define POM_PREPARE_FLASH_DURATION 10000
#endif
#ifndef POM_WORKING_DURATION
#define POM_WORKING_DURATION 1490000
#endif
#ifndef POM_DONE_WORKING_FLASH_DURATION
#define POM_DONE_WORKING_FLASH_DURATION 10000
#endif
#ifndef POM_RESTING_DURATION
#define POM_RESTING_DURATION 290000
#endif
#define POM_TOTAL_CYCLE_DURATION                                               \
  POM_PREPARE_FLASH_DURATION + POM_WORKING_DURATION +                          \
      POM_DONE_WORKING_FLASH_DURATION + POM_RESTING_DURATION
#ifndef POM_PREPARE_COLOR
#define POM_PREPARE_COLOR RGB_YELLOW
#endif
#ifndef POM_WORKING_COLOR
#define POM_WORKING_COLOR RGB_GREEN
#endif
#ifndef POM_DONE_WORKING_COLOR
#define POM_DONE_WORKING_COLOR RGB_RED
#endif
#ifndef POM_RESTING_COLOR
#define POM_RESTING_COLOR RGB_BLUE
#endif
#ifndef POM_KEY_INDEX
#define POM_KEY_INDEX 6
#endif

// Store original RGB matrix mode and settings
static uint8_t pom_original_mode = 0;
static uint8_t pom_original_speed = 0;
static uint8_t pom_original_hue = 0;
static uint8_t pom_original_sat = 0;
static uint8_t pom_original_val = 0;
static bool pom_original_values_saved = false;

// Status variables
static bool pom_timer_running = false;
static uint32_t pom_timer_start = 0;
static uint32_t pom_last_flash_time = 0;
static enum pom_led_state pom_current_led_state = POM_LED_STATE_OFF;
static bool pom_led_on = false;
static bool pom_rgb_matrix_was_enabled = false;

// Save the original RGB matrix state
void pom_save_rgb_matrix_state(void) {
  if (!pom_pom_original_values_saved) {
    pom_rgb_matrix_was_enabled = rgb_matrix_is_enabled();

    if (pom_rgb_matrix_was_enabled) {
      // Save the current RGB matrix mode and settings
      pom_original_mode = rgb_matrix_get_mode();
      pom_original_speed = rgb_matrix_get_speed();
      pom_original_hue = rgb_matrix_get_hue();
      pom_original_sat = rgb_matrix_get_sat();
      pom_original_val = rgb_matrix_get_val();
    }

    pom_pom_original_values_saved = true;
  }
};

// Restore the original RGB matrix state
void pom_restore_rgb_matrix_state(void) {
  if (pom_pom_original_values_saved) {
    if (pom_rgb_matrix_was_enabled) {
      rgb_matrix_mode(pom_original_mode);
      rgb_matrix_set_speed(pom_original_speed);
      rgb_matrix_sethsv(pom_original_hue, pom_original_sat, pom_original_val);
      rgb_matrix_enable();
    } else {
      rgb_matrix_disable();
    }

    pom_pom_original_values_saved = false;
  }
};

// Get current state based on elapsed time
enum pom_led_state pom_get_current_state(uint32_t elapsed_time) {
  uint32_t cycle_position = elapsed_time % POM_TOTAL_CYCLE_DURATION;

  if (cycle_position < POM_PREPARE_FLASH_DURATION) {
    return POM_LED_STATE_PREPARE_FLASHING;
  } else if (cycle_position <
             (POM_PREPARE_FLASH_DURATION + POM_WORKING_DURATION)) {
    return POM_LED_STATE_WORKING;
  } else if (cycle_position <
             (POM_PREPARE_FLASH_DURATION + POM_WORKING_DURATION +
              POM_DONE_WORKING_FLASH_DURATION)) {
    return POM_LED_STATE_DONE_WORKING_FLASHING;
  } else {
    return POM_LED_STATE_RESTING;
  }
};

bool pom_toggle_led_timer(void) {
  if (pom_timer_running) {
    // Stop timer and restore original color
    pom_timer_running = false;
    pom_current_led_state = POM_LED_STATE_OFF;
    pom_restore_rgb_matrix_state();
    return false;
  } else {
    // Start timer and save original state
    pom_save_rgb_matrix_state();

    // gotta make sure it is enabled
    rgb_matrix_enable();

    pom_timer_running = true;
    pom_timer_start = timer_read32();
    pom_last_flash_time = pom_timer_start;
    pom_current_led_state = POM_LED_STATE_PREPARE_FLASHING;
    pom_led_on = true;
    return true;
  }
};

bool process_keycode_pomodoro_toggle(keyrecord_t *record) {
  if (record->event.pressed) {
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
    return process_keycode_pomodoro_toggle(record);
    break;
  }
  return true;
};

bool rgb_matrix_indicators_pomodoro(void) {
  // if (!rgb_matrix_indicators_kb()) {
  //   return false;
  // }
  if (!pom_timer_running) {
    return false;
  }
  uint32_t current_time = timer_read32();
  uint32_t elapsed_time = current_time - pom_timer_start;

  // Determine current state
  enum pom_led_state new_state = pom_get_current_state(elapsed_time);

  // Handle state transitions
  if (new_state != pom_current_led_state) {
    pom_current_led_state = new_state;
    pom_led_on = true; // Reset flash state on transitions
  }

  // Handle flashing for flashing states
  if (pom_current_led_state == POM_LED_STATE_PREPARE_FLASHING ||
      pom_current_led_state == POM_LED_STATE_DONE_WORKING_FLASHING) {
    if (current_time - pom_last_flash_time >= POM_FLASH_INTERVAL) {
      pom_led_on = !pom_led_on;
      pom_last_flash_time = current_time;
    }
  }

  // Set the color based on current state
  switch (pom_current_led_state) {
  case POM_LED_STATE_PREPARE_FLASHING:
    if (pom_led_on) {
      rgb_matrix_set_color(POM_KEY_INDEX, POM_PREPARE_COLOR);
    } else {
      rgb_matrix_set_color(POM_KEY_INDEX, RGB_OFF);
    }
    break;

  case POM_LED_STATE_WORKING:
    rgb_matrix_set_color(POM_KEY_INDEX, POM_WORKING_COLOR);
    break;

  case POM_LED_STATE_DONE_WORKING_FLASHING:
    if (pom_led_on) {
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