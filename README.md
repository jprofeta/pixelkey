# PixelKey
[![Repository](https://img.shields.io/static/v1?style=social&logo=github&message=jprofeta/pixelkey&label=Repo)](https://github.com/jprofeta/pixelkey)
<br/>[![API Repository](https://img.shields.io/static/v1?style=social&logo=github&message=jprofeta/pixelkey-python&label=API%20Repo)](https://github.com/jprofeta/pixelkey-python)

[![License](https://img.shields.io/badge/license-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Documentation](https://img.shields.io/badge/docs-GitHub_Pages-informational)](https://jprofeta.github.io/pixelkey)

A simple NeoPixel USB Key powered by a Renesas RA4M1 and the Renesas FSP: [https://github.com/renesas/fsp](https://github.com/renesas/fsp).

## To do
 - [X] Implement frame transmission
 - [X] Implement task manager
 - [X] Create frame render task and link to GPT0
 - [X] Test keyframe processor
 - [X] USB enumeration
 - [X] Test USB serial echo
 - [X] Implement USB to command buffer
 - [X] Implement command parsing and handler
 - [X] Implement keyframe command handling
 - [X] Add NV memory for initial state and framerate
 - [X] Fix fade keyframe beizer curve following
 - [ ] Add timesync and RTC functionality
 - [ ] Implement scheduled keyframes
 - [ ] Implement FW upgrade (maybe)
 - [ ] Finalize MK2 hardware design
 - [ ] Add MK2 schematics and layout to repo
 - [ ] Create python API and support project

## How to use
PixelKey enumerates as a virtual COM device. Command strings terminated by a new-line can configure the device and change the NeoPixel states. A terminal emulator such as PuTTY can open the COM port to control the PixelKey. Please ensure you use the following settings:
 - Backspace is set to Control+H or `\b`. ASCII control sequences are ignored by the command parser.
 - New-line is set to `\n` or you use implicit `CRLF`.
 - The flow control mode is set to assert the RTS signal. The PixelKey will not echo otherwise unless configured to do so.

All commands are terminated by either a new-line or a semi-colon. Commands are processed upon receipt of a new-line character (either `\r` or `\n`).

The PixelKey will respond with `OK` for all successful commands. For errors, it will reply with `<error code> NAK` and halt executing any following commands in that sequence.

For available keyframes and keyframe modifiers see [docs/keyframes](./docs/keyframes.md).

For command descriptions see [docs/commands](./docs/commands.md).

For a list of available error codes see [docs/error_codes](./docs/error_codes.md).

## License
See [LICENSE.md](./LICENSE.md).

## Firmware design
The design philosiphy for the firmware is to be as modular as possible. All the core PixelKey code that resides in `src/src/pixelkey` should be agnostic to the system it is running under. All HAL accesses are made though simple API structures that are registered at startup. Theoretically, this will make it easy to port to another MCU.

All hardware specific code including the task manager, API layers, and some tasks themselves are stored at the top level of `src/src`. They provide the necessary abstractions to work on top of the RA4M1 and the Renesas FSP. 

### Firmware stackup
![firmware stackup](docs/firmware_stackup.drawio.svg)

The columns show the approximate relationship between each block element.
