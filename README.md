# LOKI - the Keyboard Indicator

Lightweight & Open source Keyboard Indicator for Windows 10

![Banner](./banner1.jpg)

## Description

This app shows icons in the Task tray indicating the states of three lock keys
(Num Lock, Caps Lock, Scroll Lock) and Insert key.

Written in C/C++ from scratch and directly calling Windows API,
LOKI runs on less memory and without heavy dependencies (such as .NET), and thus is resource-friendly.

|Targeted System    |Windows 10 |
|:---               |:---       |
|Supported Language |English    |
|Dev. Environment   |MinGW      |

## Download & Usage

You can get the zip of the app on Release page
(the link is usually in right colomn of this page).
Download, then extract it and just double-click the `loki.exe`.
Then the icons should appear in the Task tray!

The icon color changes when the key state is toggled.

LOKI supports the theme color of Dark/Light.
It dynamically adapts the icons to current theme!

On these icons:

- Left click to toggle key state.
- Right click to show menu.

## License & Disclaimer

This software is licensed under BSD 3-Clause License.
You can use, redistribute, and modify the software UNDER the several conditions.
See [LICENSE](./LICENSE) for details.

As written in LICENSE,
This software is provided WITHOUT ANY WARRANTIES,
and the author SHALL NOT be liable for any damages.

## Changelog

See [Changelog](./Changelog.md) for details.

---

    (c) 2021 inucat
