# PixelKey
[![Repository](https://img.shields.io/static/v1?style=social&logo=github&message=jprofeta/pixelkey&label=Repo)](https://github.com/jprofeta/pixelkey)
<br/>[![API Repository](https://img.shields.io/static/v1?style=social&logo=github&message=jprofeta/pixelkey-python&label=API%20Repo)](https://github.com/jprofeta/pixelkey-python)

[![License](https://img.shields.io/badge/license-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Documentation](https://img.shields.io/badge/docs-GitHub_Pages-informational)](https://jprofeta.github.io/pixelkey)

A simple NeoPixel USB Key powered by a Renesas RA4M1 and the Renesas FSP: [https://github.com/renesas/fsp](https://github.com/renesas/fsp).

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

### Frame transmission
Transmission of frame symbols to the attached NeoPixels occurs asynchronously using the DMAC and GPT peripherals. The flow diagram below outlines the two software components of transmitting the symbols.

![frame transmission](docs/npdata_transfer.drawio.svg)

First, upon the expiration of the frame timer, the frame initialization tasks is queued. The initialization task sets up the GPT and DMAC peripherals for transmission. The GPT is used in custom waveform mode to send the bit symbols. The DMAC 
pushes the next symbol timing to the GPT capture register. The DMAC is configured to repeat up to buffer size (N) transfers in symbols/N blocks (M).

After each repeat count is complete, the DMAC triggers and interrupt request. In this ISR, the DMAC source address is changed to the opposite buffer then the DMAC transfer is resumed. Then the just completed buffer is re-filled with the next batch of symbols.

Ping-pong-buffering was critical to reduce RAM overhead and maintain data integrity. With 4 attached NeoPixels, it requires 96 32-bit words for the frame symbols. This is increased by 24 words for every additional NeoPixel. Using two buffers of 24 symbols each (or smaller) and generating on the fly is a considerable RAM savings. At the HOCO frequency of 48&nbsp;MHz there are only 60 core clock cycles between NeoPixel bit symbols at the standard 800&nbsp;KHz operating frequency. ISR latency is nominally 29 cycles and the pipeline delay is 5 cycles, this provides a maximum of 25 instructions before the next GPT capture must be provided. SRAM accesses take approximately 2 cycles so this theoretically reduces available instructions to 12. This is very little opportunity to calculate the next symbol and restart the DMAC. 
