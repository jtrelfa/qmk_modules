# Pomodoro Timer

Pomodoro Timer is a QMK community module that provides a single key "timer" based on the <a href="https://en.wikipedia.org/wiki/Pomodoro_Technique" target="_blank" title="Pomodoro Technique on Wikipedia">Pomodoro Technique</a> for time management.

I wrote this originally to help me overcome my ADHD tendency to resist starting a task.

## How it works (default)

The goal is for a 30 minute cycle, 2 cycles in an hour

-   Press the key assigned to the `COMMUNITY_MODULE_POMODORO_TIMER` custom key code
-   The key will flash on and off yellow for 10 seconds (get ready to work!)
-   The key will become solid GREEN for 24 minutes and 50 seconds (get to work!)
-   The key will flash on and off red for 10 seconds (almost done)
-   The key will be solid blue for 4 minutes and 50 seconds (time to rest!)
-   Total time: 30 minutes
-   The cycle will repeat until stopped
-   Pressing the key again will stop the cycle and restore the key to whatever RGB state it was originally set to

_Colors and timing are configurable in your keymap_

## Use

Add the following to your `keymap.json`:

```json
{
    "modules": ["jtrelfa/pomodoro"]
}
```

Use the `COMMUNITY_MODULE_POMODORO_TIMER` keycode somewhere in your layout.

## Configuration

There are a number of configurable options. The following values below correspond to the defaults

```c
//in your header file (config.h)

//Times are in milliseconds
//***ALL TIMES SHOULD BE MULTIPLES OF 1000***

//How quickly the key flashes on and off (1 second intervals)
#define POM_FLASH_INTERVAL_MS 1000

//How long to "prepare to work" (10 seconds)
#define POM_PREPARE_FLASH_DURATION_MS 10000

//Total work time (24 mins, 50 seconds)
#define POM_WORKING_DURATION_MS 1490000

//How long to "prepare to rest" (10 seconds)
#define POM_DONE_WORKING_FLASH_DURATION_MS 10000

//Total rest time (4 minutes, 50 seconds)
#define POM_RESTING_DURATION_MS 290000

//These are named colors from color.h
//https://docs.qmk.fm/features/rgb_matrix#colors

//color to flash before the working phase
#define POM_PREPARE_COLOR RGB_YELLOW

//solid color during the working phase
#define POM_WORKING_COLOR RGB_GREEN

//color to flash at the end of the working phase
#define POM_DONE_WORKING_COLOR RGB_RED

//solid color during the resting phase
#define POM_RESTING_COLOR RGB_BLUE

/*
The LED index to be used in the rgb_matrix_set_color() function
https://docs.qmk.fm/features/rgb_matrix#api-rgb-matrix-set-color

I wrote this on a binepad bnk9 and 6 is the bottom left key

Check the QMK documentation to help with finding the index for the key you want to use
*/
#define POM_KEY_INDEX 6
```

There are several ways to make use of the module

1. Add the module alias `CM_POM` to your keymap.json file in the appropriate layer/position. Go to step 3
2. Add the module alias `CM_POM` to the `const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS]` section of your keymap.c file. Go to step 3
3. That's it! As long as you have a `process_record_user` function in your keymap.c file, it will trigger the timer on the POM_KEY_INDEX you've set
4. Compile and flash your keyboard

### Using VIA

If you want to use VIA, you need to take a couple of extra steps:

1. Add a `custom_keycodes` enum to your keymap.c (or config.h)
2. define the enum with a `QK_KB_<number>` value like `QK_KB_0` (\_0 through \_31 are available)
3. Add an item to your `process_record_user` function as below
4. Compile and flash your keyboard
5. Open VIA, highlight the key you want to use
6. Choose "Any" from the "Special" section
7. Type `CUSTOM(<number>)` (with the number you selected 0-31) and click save

```c
//keymap.c

//needed to call the toggle function
#include "pomodoro.h"

//define your code and assign it to a predefined value
enum custom_keycodes {
    CM_POMODORO_TIMER = QK_KB_0
};
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch(keycode) {
        case CM_POMODORO_TIMER: {
            return process_keycode_pomodoro_toggle(record);
        }
    }
}
```

## Notes/Limitations

There are some key items to note:

-   This has only been tested on a binepad bnk9.
-   The rgb_matrix_set_color() function is used for handling the lighting for the timer (https://docs.qmk.fm/features/rgb_matrix#api-rgb-matrix-set-color)
-   The lighting of the key is **_independent of layer_** since it directly addresses the index of the LED. (this could change in a future release)
-   Check the [QMK documentation](https://docs.qmk.fm/features/rgb_matrix#common-configuration) to help with finding the correct index when configuring your keymap
