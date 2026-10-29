# Changelog

Notable changes to the modules in this repo, newest first. Changes before 2026-10-29 are not listed.

## 2026-10-29

### Pomodoro Timer

#### Breaking

-   Timing is now set in whole minutes with `POM_WORK_MINUTES` (default 25) and `POM_REST_MINUTES` (default 5). The 10 second countdowns are included in those minutes. The old `POM_FLASH_INTERVAL_MS`, `POM_PREPARE_FLASH_DURATION_MS`, `POM_WORKING_DURATION_MS`, `POM_DONE_WORKING_FLASH_DURATION_MS` and `POM_RESTING_DURATION_MS` settings are no longer used and cause a build error telling you what to change.
-   The internal `pom_*` functions (`pom_toggle_led_timer`, `pom_save_rgb_matrix_state`, `pom_restore_rgb_matrix_state`, `pom_get_current_state`) are no longer public. Use `process_keycode_pomodoro_toggle` or the new `pomodoro_*` functions instead.

#### Added

-   The timer lights whichever key you pressed to start it. `POM_KEY_INDEX` is now optional, for always lighting one specific LED (for example when starting the timer from a key with no LED).
-   `pomodoro_start()`, `pomodoro_stop()`, `pomodoro_is_running()` and `pomodoro_get_state()` for controlling the timer from your own code.
-   Console messages when the timer starts, stops, changes phase, or can't find an LED for the key you pressed.
-   Build errors for invalid settings, such as a work or rest time of 0 minutes or a `POM_KEY_INDEX` that isn't an LED on your keyboard.

#### Changed

-   If RGB Matrix is off when the timer starts, it is turned on without saving to EEPROM and the rest of the board stays dark. Stopping the timer turns it back off.
-   Colors are capped at the keyboard's `RGB_MATRIX_MAXIMUM_BRIGHTNESS`.
-   The flashing is calculated from the start time, so it no longer drifts.

#### Fixed

-   While the timer was stopped, it blocked the RGB indicators of any community module listed after it.
-   Stopping the timer no longer writes the RGB settings to EEPROM every time.
-   The timer key is no longer passed on to the rest of QMK after it's handled.

#### Docs

-   Rewrote the module README with requirements, setup steps, VIA instructions, debugging and troubleshooting.
-   Fixed the install links in the main README, which pointed to `qmk-modules` instead of `qmk_modules`.
