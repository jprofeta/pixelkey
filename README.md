# PixelKey
[![License](https://img.shields.io/badge/license-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Documentation](https://img.shields.io/badge/docs-GitHub_Pages-informational)](https://jprofeta.github.io/pixelkey)

A simple NeoPixel USB Key powered by an RA4M1.

## To do
 - [X] Implement frame transmission
 - [X] Implement task manager
 - [X] Create frame render task and link to GPT0
 - [X] Test keyframe processor
 - [X] USB enumeration
 - [X] Test USB serial echo
 - [ ] Implement USB to command buffer
 - [ ] Implement command parsing and handler
 - [ ] Add timesync and RTC functionality
 - [ ] Implement scheduled keyframes
 - [ ] Implement FW upgrade (maybe)
 - [ ] Finalize MK2 hardware design
 - [ ] Add MK2 schematics and layout to repo
 - [ ] Fix fade keyframe beizer curve following
 - [ ] Add NV memory for initial state and framerate

## How to use
PixelKey enumerates as a virtual COM device. Simple strings terminated by a new-line can configure the device and change the NeoPixel states. Two formats of data are accepted: commands and keyframes. Commands configure the driver, while keyframes describe how the NeoPixels should be controlled.

For command descriptions see [docs/commands](./docs/commands.md). For available keyframes and keyframe modifiers see [docs/keyframes](./docs/keyframes.md)

## License
See [LICENSE.md](./LICENSE.md).
