# PixelKey
[![License](https://img.shields.io/badge/license-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Documentation](https://img.shields.io/badge/docs-GitHub--Pages-informational)](https://jprofeta.github.io/pixelkey)

A simple USB NeoPixel driver.

## How to use
PixelKey enumerates as a virtual COM device. Simple strings terminated by a new-line can configure the device and change the NeoPixel states. Two formats of data are accepted: commands and keyframes. Commands configure the driver, while keyframes describe how the NeoPixels should be controlled.

For command descriptions see [docs/commands](./docs/commands.md). For available keyframes and keyframe modifiers see [docs/keyframes](./docs/keyframes.md)

## License
See [LICENSE.md](./LICENSE.md).
