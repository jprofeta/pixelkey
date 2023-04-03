# Keyframes


## Available keyframes

### set
Changes the color of a NeoPixel to a constant value.

```
[index] set <color>
```

### blink
Blinks a NeoPixel with a specified period and duty-cycle between two values. The NeoPixel color will be set to color 2 after completion of the blink keyframe. This will repeat until the next keyframe is received unless otherwise specifed with a repeat modifier.

```
[index] blink <period> [color 1[,<color 2>]] [duty cycle]
```
where
- **period**: Number of seconds to blink over.
- **color 1**: Color to blink during ON cycle. If not specified, it will blink using the last color set for this index.
- **color 2**: Color to blink during OFF cycle. If not specified, it is assumed to be "off/black".
- **duty cycle**: Percentage of time during "period" specified as ON. Ranges from 1-99.

### fade
Fades a NeoPixel between two colors over a specified time period. The NeoPixel color will be set to the end color after completion of the fade keyframe.

```
[index] fade <period> [<start color>,]<end color> [type]
```
where
- **period**: Number of seconds to fade over.
- **start color**: Color of start of fade. If not specified, it will use the current set color for this index.
- **end color**: Color to end the fade on.
- **type**: Type of fade transition to use. Available values are:
    - *linear*: Transitions linearly betwen the start and end color.

### rotate
Cycles a NeoPixel between a list of colors over a specified time period. The NeoPixel color will be set to the last color after completion of the rotate keyframe.

```
[index] rotate <period> <color 1>,<color 2>[,...] [type]
```
where
- **period**: Number of seconds for the whole color rotation.
- **color 1,2,...**: List of colors to fade between.
- **type**: Type of fade transition to use. Available values are:
    - *linear*: Transitions linearly betwen the specified colors.
    - *step*: No smooth transition occurs. Colors change immediately every `period/num colors` seconds.

## Keyframe modifiers
Keyframe modifiers perform additional operations beyond that of specifying a NeoPixel state, e.g. grouping or repeating.

### Repeating keyframes
Keyframes may be repeated by sending a repeat modifier before the desired repeating keyframe. A repeat modifier is a carrot, "`^`", followed by a count. A count of "0" indicates repeat until next keyframe is received.

```
^<count>
```

For example, to repeat a blink keyframe 10 times:
```
^10
blink 2 red
```
or in-line
```
^10; blink 2 red
```

### Grouping keyframes
Keyframes may be grouped together and optionally named. Please note, the name has no effect other than as information for the user.

Groups are started by using a pound symbol, "`#`", and groups are ended by a pound symbol followed by a forward-slash, "`#/`". A list of indexes may follow the pound symbol for group starts to overide the default index list. Groups may be nested.

> **📝Note:** The NeoPixel states will not update until all groups have been closed with a `#/`. At which point, the group keyframes will be executed.

```
#[index] [group name]
keyframes...
#/
```

For example, to blink NeoPixels 2 and 3 blue 5 times then rotate through all colors over 5 seconds:
```
#2,3 demo_group
^5
blink 2 blue
rotate 5 red,green,blue,red
#/
```
or in-line
```
#2,3; ^5; blink 2 blue; rotate 5 red,green,blue,red; #/
```

> **📝Note:** Repeat modifiers may be applied to groups.

## Specifying colors in keyframes
Colors can be specified in different formats:
- RGB (hex or decimal)
- HSL
- HSV
- named

RGB values can be specifed as hexadecimal values, `#RRGGBB`, or as a tuple of percentages, `%rrr,ggg,bbb` where values can range from 0-100. For example, an orange can be specified as `#ED7651` or `%93,46,32`.

HSL, hue-saturation-lightness, values can be specifed by a tuple of values prefixed with an exclimation point, `!hhh,sss,lll`. Hue ranges from 0-359 with pure red at 0, pure green at 120, and pure blue at 240. Saturation and lightness range from 0-100. For example, the same orange as above can be represented as `!14,82,62`.

HSV, hue-saturation-value, can be specifed by a tuple of values prefixed with an exclimation point and a "v", `!vhhh,sss,vvv`. Hue ranges from 0-359 with pure red at 0, pure green at 120, and pure blue at 240. Saturation and value range from 0-100. For example, the same orange as above can be represented as `!v14,66,93`.

These named colors are available:
- red: `#FF0000`
- orange: `#FFA500`
- yellow: `#FFFF00`
- green: `#00FF00`
- cyan: `#00FFFF`
- blue: `#0000FF`
- magenta: `#FF00FF`
- white: `#FFFFFF`
- black/off: `#000000`

> **📝Note:** Since NeoPixels are purely emissive sources of light, no blacks are possible. Therefore, many colors will not be able to be accurately represented.

For all color specifications, leading zeros are permitted for decimal values and upper- or lower-case letters may be used.
