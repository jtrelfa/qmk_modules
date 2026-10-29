# Pomodoro Timer

Pomodoro Timer is a QMK community module that provides a single key "timer" based on the <a href="https://en.wikipedia.org/wiki/Pomodoro_Technique" target="_blank" title="Pomodoro Technique on Wikipedia">Pomodoro Technique</a> for time management.

I wrote this originally to help me overcome my ADHD tendency to resist starting a task.

> Updating from an older version? Check the [changelog](../CHANGELOG.md). The timing settings changed.

## How it works (default)

The goal is for a 30 minute cycle, 2 cycles in an hour: 25 minutes of work and 5 minutes of rest

-   Press the key assigned to the `COMMUNITY_MODULE_POMODORO_TIMER` keycode (alias `CM_POM`). The timer lights up **the key you pressed**
-   The key will flash on and off yellow for 10 seconds (get ready to work!)
-   The key will become solid GREEN for 24 minutes and 50 seconds (get to work!)
-   The key will flash on and off red for 10 seconds (almost done)
-   The key will be solid blue for 4 minutes and 50 seconds (time to rest!)
-   The key will flash yellow for 10 seconds again (get ready to work!), and the cycle repeats until stopped
-   Pressing the key again will stop the cycle and restore the key to whatever RGB state it was originally set to

The 10 second countdowns are part of the work and rest time, so 25/5 is exactly 30 minutes.

_Work/rest length and colors are configurable in your keymap_

## Requirements

-   **QMK Firmware 0.29.0 or later** (released 2025-05-25)
-   **A keyboard with per-key RGB, also called RGB Matrix.** Underglow-only boards (RGB Light) are not supported. To check, open your keyboard's `keyboard.json` (or `info.json`) in `qmk_firmware/keyboards/<your keyboard>/` and look for an `"rgb_matrix"` section, or look for `RGB_MATRIX_ENABLE = yes` in its `rules.mk`.

## Install

Download the module first by following the [install steps for this repo](../README.md#how-to-install).

## Add it to your keymap

**1. Enable the module.** In your keymap folder (for example `keyboards/<your keyboard>/keymaps/<your keymap>/`), add the module to `keymap.json`:

```json
{
    "modules": ["jtrelfa/pomodoro"]
}
```

If you already have a `keymap.json`, merge the `"modules"` line into it. If your keymap is a `keymap.c`, you still need this `keymap.json` next to it. It can contain nothing but the `"modules"` line.

**2. Put the key in your layout.** Use `CM_POM` (or the full name, `COMMUNITY_MODULE_POMODORO_TIMER`) in any position that has an LED:

-   In `keymap.json`, put `"CM_POM"` in the `"layers"` array, or
-   In `keymap.c`, put `CM_POM` in the `keymaps[][MATRIX_ROWS][MATRIX_COLS]` array

**3. Compile and flash.** No extra code is needed; the module handles the keycode itself.

> **Using VIA?** VIA stores your keymap on the keyboard, and that copy wins over the keymap you compiled. If the key does nothing after flashing, see [Using VIA](#using-via).

## Configuration (optional)

Everything works out of the box. To change a setting, add **only the lines you want to change** to the `config.h` in your keymap folder. If you don't have one, create it with `#pragma once` as the first line.

The values below are the defaults.

```c
// Length of each work and rest period, in whole minutes.
// The 10 second flashing countdown at the end of each period is included.
#define POM_WORK_MINUTES 25
#define POM_REST_MINUTES 5

// Colors must be the RGB_ names from https://docs.qmk.fm/features/rgb_matrix#colors
// (or three numbers, like 0xFF, 0x80, 0x00). The HSV_ names compile but show the wrong color.
#define POM_PREPARE_COLOR RGB_YELLOW    // flashing: get ready to work
#define POM_WORKING_COLOR RGB_GREEN     // solid: working
#define POM_DONE_WORKING_COLOR RGB_RED  // flashing: almost done working
#define POM_RESTING_COLOR RGB_BLUE      // solid: resting

// Not set by default. See "Always lighting the same LED" below.
// #define POM_KEY_INDEX 6
```

For example, a 50 minute work / 10 minute rest cycle:

```c
#pragma once

#define POM_WORK_MINUTES 50
#define POM_REST_MINUTES 10
```

The settings are checked when you compile. You get a build error if:

-   `POM_WORK_MINUTES` or `POM_REST_MINUTES` is less than 1 or more than 1440 (one day)
-   `POM_KEY_INDEX` is not a valid LED on your keyboard
-   you still have the timing settings from older versions of this module (`POM_WORKING_DURATION_MS`, `POM_FLASH_INTERVAL_MS`, etc.). Replace them with `POM_WORK_MINUTES` and `POM_REST_MINUTES`

### Always lighting the same LED

By default the timer lights whichever key you pressed to start it. Define `POM_KEY_INDEX` to always light one specific LED instead. This is how you start the timer from a key with no LED, like a rotary encoder button.

`POM_KEY_INDEX` is the LED's number. To find it, open your keyboard's `keyboard.json` (or `info.json`) and look at the `"rgb_matrix"` → `"layout"` list. The first entry is LED 0, the next is LED 1, and so on. Each entry's `"matrix"` value tells you which key it sits under. On a binepad bnk9, 6 is the bottom left key.

## Using VIA

VIA keeps its own copy of your keymap on the keyboard, and doesn't know about keycodes added by community modules. To use the timer with VIA, you create a custom keycode and point it at the timer.

**1. Pick a custom keycode number** from 0 to 31. Many keyboards already use some of these for their own features. Search your keyboard's folder for `QK_KB_` and pick a number that isn't used. For example, if you find `QK_KB_0` and `QK_KB_1`, use 2.

**2. Add the code below to your `keymap.c`** (not `config.h`). If you already have a `process_record_user` function, add the `case` to your existing `switch` instead of adding a second function.

```c
// keymap.c

#include "pomodoro.h"

enum custom_keycodes {
    CM_POMODORO_TIMER = QK_KB_2 // the number you picked in step 1
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case CM_POMODORO_TIMER:
            return process_keycode_pomodoro_toggle(record);
    }
    return true;
}
```

**3. Compile and flash** your keyboard.

**4. Assign the key in VIA.** Open VIA and select the key you want to use. In the "Special" section, choose "Any", type `CUSTOM(2)` (with the number you picked), and save.

> **Why didn't my compiled keymap change anything?** With VIA, the keymap saved on the keyboard is used instead of the one you compiled, until the keyboard's saved settings (EEPROM) are cleared. Either assign the key in VIA as above, or clear the EEPROM with the `EE_CLR` keycode or by holding the [Bootmagic](https://docs.qmk.fm/features/bootmagic) key while plugging the keyboard in. Clearing the EEPROM also resets your other VIA changes.

## Controlling the timer from your own code

Include `pomodoro.h` in your `keymap.c` to use these functions:

```c
bool               pomodoro_start(uint8_t led_index); // start (or restart) on a specific LED, false if the index is invalid
void               pomodoro_stop(void);
bool               pomodoro_is_running(void);
enum pom_led_state pomodoro_get_state(void);          // POM_LED_STATE_OFF, _PREPARE_FLASHING, _WORKING, _DONE_WORKING_FLASHING, _RESTING
```

For example, to toggle the timer on LED 6 from your own keycode (or a tap dance, combo, etc.):

```c
#include "pomodoro.h"

enum custom_keycodes {
    MY_POMODORO_KEY = SAFE_RANGE // put MY_POMODORO_KEY in your layout
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
#ifdef COMMUNITY_MODULE_POMODORO_ENABLE // keeps the keymap building if the module is removed
        case MY_POMODORO_KEY:
            if (record->event.pressed) {
                if (pomodoro_is_running()) {
                    pomodoro_stop();
                } else {
                    pomodoro_start(6);
                }
            }
            return false;
#endif
    }
    return true;
}
```

## Debugging

The module logs to the [QMK console](https://docs.qmk.fm/faq_debug) when it starts, stops, changes phase, or can't find an LED for the key you pressed. To see the messages:

1. Turn on the console, either with `CONSOLE_ENABLE = yes` in your keymap's `rules.mk`, or in `keymap.json`:

```json
{
    "config": {
        "features": {
            "console": true
        }
    }
}
```

2. Turn debugging on, either with the `DB_TOGG` keycode or in your keymap.c:

```c
void keyboard_post_init_user(void) {
    debug_enable = true;
}
```

3. Run `qmk console`

With the console disabled, the logging is compiled out and takes no space.

## Troubleshooting

**Build error: `CM_POM` (or `COMMUNITY_MODULE_POMODORO_TIMER`) undeclared**

-   Check that `"jtrelfa/pomodoro"` is in the `"modules"` list in your `keymap.json`
-   Check that the module is downloaded to `modules/jtrelfa/pomodoro` (see [Install](#install))
-   Run `qmk clean` and compile again

**Build error mentioning `POM_WORK_MINUTES`, `POM_REST_MINUTES` or `POM_KEY_INDEX`**

-   See the checks listed under [Configuration](#configuration-optional)

**The key doesn't light up**

-   If you use VIA, see [Using VIA](#using-via). The compiled keymap may not be the one on your keyboard
-   The key may not have an LED (rotary encoder buttons usually don't). Use another key, or set [`POM_KEY_INDEX`](#always-lighting-the-same-led)
-   Check the [console](#debugging): the module tells you when it can't find an LED for the key you pressed

**My timing or color settings are ignored**

-   Make sure they're in the `config.h` in your keymap folder, not the keyboard's folder
-   Colors must use the `RGB_` names, not `HSV_`

## Notes/Limitations

There are some key items to note:

-   This has only been tested on a binepad bnk9.
-   The rgb_matrix_set_color() function is used for handling the lighting for the timer (https://docs.qmk.fm/features/rgb_matrix#api-rgb-matrix-set-color)
-   The lit key is chosen when the timer starts and stays lit **_on every layer_**.
-   Keys without an LED (like the bnk9 knob) can't start the timer unless `POM_KEY_INDEX` is defined. Neither can combos or encoder turns, since they don't map to a physical key.
-   If RGB Matrix is off when the timer starts, it is turned on (without saving to EEPROM) and every other key is kept dark. Stopping the timer turns it back off.
-   Colors are scaled to your keyboard's `RGB_MATRIX_MAXIMUM_BRIGHTNESS`.
-   If you use `RGB_MATRIX_TIMEOUT`, the key goes dark with the rest of the board when the timeout kicks in. The timer itself keeps running.
