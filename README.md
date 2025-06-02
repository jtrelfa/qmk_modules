# @jtrelfa's QMK community module(s)

I was encouraged by @SenseiZé ッ on the Binepad discord to share my contribution(s) to QMK.

> :bangbang: **NB:** These work on QMK Firmware 0.29.0 (released 2025-05-25) or later

| Module                        | Description                                      |
| ----------------------------- | ------------------------------------------------ |
| [Pomodoro Timer](./pomodoro/) | A work timer based on the pomodoro timer concept |

## What is this?

This repo contains my community modules for [Quantum Mechanical Keyboard
(QMK)](https://docs.qmk.fm) firmware. QMK is used on custom keyboards and
macropads like the [BinePad BNK9](https://binepad.com/products/bnk9)

## How to install

This repo makes use of [Community
Modules](https://docs.qmk.fm/features/community_modules)
support added in QMK Firmware 0.28.0, released on 2025-02-27. [Update your QMK
set
up](https://docs.qmk.fm/newbs_git_using_your_master_branch#updating-your-master-branch)
to get the latest. If you have it, there will be a `modules` folder inside your
`qmk_firmware` folder.

**Step 1. Download modules.** Run these shell commands to download the
modules, replacing `/path/to/qmk_firmware` with the path of your
"`qmk_firmware`" folder:

```sh
cd /path/to/qmk_firmware
mkdir -p modules
git submodule add https://github.com/jtrelfa/qmk-modules.git modules/jtrelfa
git submodule update --init --recursive
```

Or if using [External
Userspace](https://docs.qmk.fm/newbs_external_userspace), replace the first
line with `cd /path/to/your/external/userspace`.

Or if you don't want to use git, [download a .zip of this
repo](https://github.com/jtrelfa/qmk-modules/archive/refs/heads/main.zip) into
the `modules` folder. Unzip it, then rename the resulting `qmk-modules-main`
folder to `jtrelfa`.

In any case, the installed directory structure is like this:

    <QMK_FIRMWARE or QMK_USERSPACE>
    └── modules
        └── jtrelfa
            └── pomodoro

**Step 2. Add modules to keymap.json.** Add a module to your keymap by writing a
file `keymap.json` in your keymap folder with the content

```json
{
  "modules": ["jtrelfa/pomodoro"]
}
```

Or if a `keymap.json` already exists, merge the `"modules"` line into it. Add
multiple modules into the JSON array:

```json
{
  "modules": ["existing/modulename", "jtrelfa/pomodoro"]
}
```

Follow the module documentation for any further specific set up.

**Step 3. Update the firmware.** Compile and flash the firmware as usual. If
there are build errors, try running `qmk clean` and compiling again for a clean
build.


To update to the latest commit (of all submodules), use:

```sh
git submodule update --remote --merge
```

## How to uninstall

Remove the modules from `keymap.json` and delete the `modules/jtrelfa` folder.
