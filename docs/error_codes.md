# Error Codes

The following error codes may be returned by the PixelKey. For a full list of allocated error codes see the `pixelkey_error_t` enum in the [PixelKey documentation](https://jprofeta.github.io/pixelkey/group__pixelkey.html).

| Value | Description |
| :---: | :---------- |
|   | **General errors** |
| 1 | Invalid arguments |
| 2 | Keyframe buffer full |
| 3 | Communication error or timeout |
| 4 | Input buffer overflow |
| 5 | Index out of range |
| 6 | Keyframe processing stopped |
| 7 | RTC not set |
|    | **Configuration and command specific errors** |
| 16 | Configuration key not found |
| 17 | NV-mem error on configuration save |
| 18 | Configuration value out of range or wrong type |
|    | **Firmware upgrade errors** |
| 32 | Missing FW image blocks |
| 33 | CRC mismatch |
