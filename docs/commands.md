# Commands


## Configuration get values
Retreives a configured value. See below for available configuration keys.
```
$config-get <key>
```
Returns
```
<value>
OK
```
error or key does not exist
```
<err code> NAK
```

### Available configuration keys

#### **framesize**
The total number of NeoPixels connected to the PixelKey.

Default: 4

The maximum frame size is approximately `(1/refreshrate - 50us)/31.2us`. This is about 1065 NeoPixels for 30 fps.

#### **refreshrate**
The number of refresh cycles per second. Controls the update rate of keyframes.

Default: 30

This is limited by the total number of attached NeoPixels.

Maximum refresh rate is approximately `1/(framesize * 31.2us + 50us)`.



## Configuration set values
Saves a configuration value. See below for available configuration keys and values.
```
$config-set <key> <value>
```
Returns on error or key does not exist
```
<err code> NAK
```

## Resume
Resumes keyframe processing.
```
$resume
```

## Status
Prints the current device status.
```
$status
```
Returns
```
PixelKey vMM.mm.pp
Current state: active|idle|stopped
Keyframe pending? yes|no
Time to next keyframe: ttt seconds
Current keyframe: type
Next keyframe: type
OK
```

## Stop
Stops keyframe processing, clears the keyframe buffer, and turns off (sends `#000000`) all attached NeoPixles.
```
$stop
```

## Time get
Returns the current RTC time.
```
$time-get
```
Returns
```
YYYY-MM-DD HH:mm:ss.sss
OK
```
Returns "RTC not set" error if the RTC has not been programmed.

## Time set
Sets the current time for the RTC. Accepts an ISO-8601 timestamp, but the time specifier `T` may be replaced with a space or underscore.
```
$time-set YYYY-MM-DD HH:mm:ss
```
Returns `OK` on success or `<error> NAK` if RTC failed to set.

## Time sync*
Allows synchronization within several 10s of ms to an external clock source like the host computer. This command should not be used manually.
```
$time-sync
```
Once started the host and PixelKey exchange timestamps until the standard deviation is acceptable or a timeout occurs. The state machine flow is as follows:

1. Host sends `$time-sync`.
2. PixelKey pauses keyframe processing and responds `OK`.
3. PixelKey sends a 128-bit timestamp.
4. Host records timestamp at reception. Transmits RX timestamp and new TX timestamp.
5. PixelKey records timestamp at reception then calculates offset and round-trip delay.
6. If sample count < required or std. dev. > acceptable: repeat at 3 until timeout.
7. If timeout: send `<error> NAK` to host to end time-sync
8. If std.dev. is acceptable save offset to RTC/system-clock and send `OK` to host to end time-sync.
9. PixelKey resumes keyframe processing.

## Version
Prints version information.
```
$version
```
Returns
```
PixelKey version MM.mm.pp
OK
```
where 
- `MM` is the major version,
- `mm` is the minor version, and
- `pp` is the patch.

Semantic versioning is used so other values may be present.

## Firmware upgrade commands
> **⚠️ Warning:**
> There be dragons ahead. Only use these commands if you know what you're doing. Incorrect usage can break the device.

### Firmware upgrade init*
Clears the upgrade region and initializes anything necessary for the FW upgrade.
```
$fwupg-init <num blocks> <image crc>
```
Arguments
- **num blocks**: Total number of expected blocks for this image.
- **image crc**: Expected CRC for the image.

### Firmware block*
Transfers a block to the device. Blocks are always 64 bytes long.
```
$fwupg-block <crc> <offset> <bin data>
```
Arguments
- **crc**: The CRC-16 (polynomial TBD) of the offset and bin data fields.
- **offset**: Block offset of this data.
- **bin data**: Binary data for this block. Must transfer entire block.

### Firmware verify*
Verifies a received firmware image.
```
$fwupg-verify
```

Returns
- `<image crc> OK` on full image receipt and CRC check pass. includes calculated CRC.
- `<block list> 1 NAK` on incomplete image, includes list of missing blocks.
- `<image crc> 2 NAK` on CRC mismatch, includes calculated CRC.

### Firmware upgrade commit*
Reboots into the bootloader and copies the new image.
```
$fwupg-commit
```

> \* These commands may exist on a separate USB pipe and may not be exposed for the serial command set.

