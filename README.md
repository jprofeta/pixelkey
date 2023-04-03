# PixelKey
A simple USB NeoPixel driver.

## How to use
PixelKey enumerates as a virtual COM device. Simple strings terminated by a new-line can configurate the device and change the neopixel states. Two formats of data are accepted: commands and keyframes. Commands configure the driver, while keyframes describe how the NeoPixels should be controlled.

### Command format
All commands start with a dollar sign, `$`, followed by the command name. Arguments may follow, separated by spaces.

`$<command> <arguments...>`

### Keyframe format
Keyframes are defined by specifying an optional NeoPixel index or range, a keyframe type, and any arguments.

`<index> <type> <arguments...>`


