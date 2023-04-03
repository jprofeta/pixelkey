# Commands

## Set configuration values
Saves a configuration value. See below for available configuration keys and values.
```
$set <key> <value>
```
Returns on error or key does not exist
```
<err code> NAK
```

## Get configuration values
Retreives a configured value. See below for available configuration keys.
```
$get <key>
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

## Stop
Stops keyframe processing, clears the keyframe buffer, and turns off (sends `#000000`) all attached NeoPixles.
```
$stop
```

## Resume
Resumes keyframe processing.
```
$resume
```

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

Symantic versioning is used so other values may be present.

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

## Firmware upgrade commands
> **⚠️ Warning:** There be dragons ahead. Only use these commands if you know what you're doing. Incorrect usage can break the device.

### Firmware upgrade init
Clears the upgrade region and initializes anything necessary for the FW upgrade.
```
$fwupg-init <num blocks> <image crc>
```
Arguments
- **num blocks**: Total number of expected blocks for this image.
- **image crc**: Expected CRC for the image.

### Firmware block
Transfers a block to the device. Blocks are always 64 bytes long.
```
$fwupg-block <crc> <offset> <bin data>
```
Arguments
- **crc**: The CRC-16 (polynomial TBD) of the offset and bin data fields.
- **offset**: Block offset of this data.
- **bin data**: Binary data for this block. Must transfer entire block.

### Firmware verify
Verifies a received firmware image.
```
$fwupg-verify
```

Returns
- `<image crc> OK` on full image receipt and CRC check pass. includes calculated CRC.
- `<block list> 1 NAK` on incomplete image, includes list of missing blocks.
- `<image crc> 2 NAK` on CRC mismatch, includes calculated CRC.

### Firmware upgrade commit
Reboots into the bootloader and copies the new image.
```
$fwupg-commit
```

## Available configuration keys

### framesize
The total number of NeoPixels connected to the PixelKey.

Default: 4

The maximum frame size is approximately `(1/refreshrate - 50us)/31.2us`. This is about 1065 NeoPixels for 30 fps.

### refreshrate
The number of refresh cycles per second. Controls the update rate of keyframes.

Default: 30

This is limited by the total number of attached NeoPixels.

Maximum refresh rate is approximately `1/(framesize * 31.2us + 50us)`.


