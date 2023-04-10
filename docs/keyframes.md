# Keyframes
Keyframes are the instructions for how the NeoPixels are controlled and change state. Each keyframe is separated by either a newline or a semi-colon. Special keyframes modifiers can also be used and are separated the same as with keyframes. Keyframe modifiers add additional control variables to a keyframe.

## Available keyframes

### set
Changes the color of a NeoPixel to a constant value.

```
[index] set <color>
```

### blink
Blinks a NeoPixel with a specified period and duty-cycle between two values. The NeoPixel color will be set to color 2 after completion of the blink keyframe. This will repeat until the next keyframe is received unless otherwise specified with a repeat modifier.

```
[index] blink <period> [color 1[,<color 2>]] [duty cycle]
```
where
- **period**: Number of seconds to blink over.
- **color 1**: Color to blink during ON cycle. If not specified, it will blink using the last color set for this index.
- **color 2**: Color to blink during OFF cycle. If not specified, it is assumed to be "off/black".
- **duty cycle**: Percentage of time during "period" specified as ON. Ranges from 1-99.

### Fade
Transitions a NeoPixel between a list of colors over a specified time period. The NeoPixel color will be set to the last color after completion of the fade keyframe.

```
[index] fade <period> [&]<color 1>[,<color 2>,...] [type]
```
where
- **period**: Number of seconds over which the keyframe will fade between colors. Maximum of 60 seconds.
- **color 1,2,...**: List of colors to transition between.
  - If only one color is specified it will fade from the current color to the specified one.
  - Up to 15 colors allowed.
  - If the first color is prefixed with an ampersand, "`&`", then an additional transition from the current color to color 1 will occur first.
- **type**: Type of transition to use. ~~If precceded by a plus symbol, "`+`", the transition will apply between each color at intervals of `period/(len(colors)-1)`. Otherwise, the transition keyframe will use the specified function once across the entire transition.~~ Available transition types are:
    - *step*: No smooth transition occurs. Colors change immediately every `period/(len(colors)-1)` seconds.
    - *linear*: Transitions linearly between the specified colors [`cubic(0,0,1,1)`]. (default)
    - *ease*: Slow start, then fast middle, then slower end transition [`cubic(0.25,0.1,0.25,1)`].
    - *ease-in*: Slow transition at the start [`cubic(0.42,0,1,1)`].
    - *ease-out*: Slow transition at the end [`cubic(0,0,0.58,1)`].
    - *ease-in-out*: Slow transitions at the start and end [`cubic(0,42,0,0.58,1)`].
    - *cubic(x1,y1,x2,y2)*: Specify a custom cubic bezier transition formula.
        - For more information, see https://cubic-bezier.com/.

~~In cases where the transition occurs between all colors entirely, i.e. no `+` transition prefix, the colors are equally spaced across the transition curve. So if one uses `fade 10 red,green,blue ease-in`, it will quickly transition from red towards green, then slowly transition from green to blue all over 10 seconds. Whereas, if one uses `fade 10 red,green,blue +ease-in`, it will perform two transitions over 5 seconds each. This is equivalent to `fade 5 red,green ease-in; fade 5 green,blue ease-in`.~~

For non-pure color transitions, e.g. red to white, each color component will be transitioned separately in the HSV color space.

## Keyframe modifiers
Keyframe modifiers perform additional operations beyond that of specifying a NeoPixel state, e.g. grouping or repeating.

Please note, the order of keyframe modifiers matter. `^5; @PT10M;` is not the same as `@PT10M; ^5;`. The former meaning to repeat every 10 minutes for a total of 5 times, while the latter represents in 10 minutes immediately repeat the keyframe 5 times. When in doubt, use keyframe groups.

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

### Timestamped keyframes
Keyframes may be queued for a specific time, which can be used to synchronize multiple devices. Only one timestamped keyframe is supported per index. 

A timestamped modifier is an at symbol, "`@`", followed by a time string in ISO-8601 format. Date/time literals and durations are supported.
```
@<timestamp:iso8601>
```

The following ISO-8601 representations are allowed:
- Dates
  - `YYYYMMDD`, `YYYY-MM-DD`
  - Time assumes the time component is `00:00` or midnight in the morning.
- Times
  - `THHmmss.sss`, `THH:mm:ss.sss`
  - This will queue for the current day or the next day if that hour has already passed.
  - The leading "`T`" is optional (differs from ISO-8601).
- Timestamps
  - `YYYYMMDDTHHmmss.sss`, `YYYY-MM-DDTHH:mm:ss.sss`
  - The time indicator is allowed to be a space, "` `", or underscore, "`_`", instead (differs from ISO-8601).
- Durations/timespans
  - `PnYnMnDTnHnMnS`
  - The preceeding "`P`" and time separator, "`T`", are required.
  - This can be used to queue for x time in the future, e.g. 30 minutes, `PT30M`.
- Time intervals
  - `<start>/<end>`, `<start>/<duration>`, `<duration>/<end>`
  - These can be used to automatically stop a timestamped keyframe at a specific time.
  - Any supported time representation may be used on either side of the interval.
  - A double hyphen, "`--`", may be used instead of a forward-slash, "`/`".
- Repeating intervals
  - `Rn/<interval or duration>`
  - Can be used instead of a repeating keyframe modifier to repeat over a certain interval.
  - If `n` is not specified, it will repeat indefinitely until stopped.
- For more information, see https://en.wikipedia.org/wiki/ISO_8601.


For example, to start a keyframe at 1:42 p.m. on 22 Oct 2023:
```
@2023-10-22 13:42
blink 2 red
```
or in-line
```
@2023-10-22 13:42; blink 2 red
```

> **📝Note:**
> The repeating modifier may be combined with duration based timestamps. It is not valid with absolute timestamps and will be ignored.


### Grouping keyframes
Keyframes may be grouped together and optionally named. Please note, the name has no effect other than as information for the user.

Groups are started by using a pound symbol, "`#`", and groups are ended by a pound symbol followed by a forward-slash, "`#/`". A list of indexes may follow the pound symbol for group starts to overide the default index list. Groups may be nested.

> **📝Note:**
> The NeoPixel states will not update until all groups have been closed with a `#/`. At which point, the group keyframes will be executed.

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
fade 5 red,green,blue,red
#/
```
or in-line
```
#2,3; ^5; blink 2 blue; fade 5 red,green,blue,red; #/
```

> **📝Note:**
> Repeat and timestamp modifiers may be applied to groups.

## Specifying colors in keyframes
Colors can be specified in different formats:
- RGB (hex or decimal)
- HSL
- HSV
- named

RGB values can be specified as hexadecimal values, `#RRGGBB`, or as a tuple of percentages, `%rrr,ggg,bbb` where values can range from 0-100. For example, an orange can be specified as `#ED7651` or `%93,46,32`.

HSV, hue-saturation-value, can be specified by a tuple of values prefixed with an exclamation point, `!hhh,sss,vvv`. Hue ranges from 0-359 with pure red at 0, pure green at 120, and pure blue at 240. Saturation and value range from 0-100. For example, the same orange as above can be represented as `!14,66,93`. 

> **ℹ️**
> Two short-forms are allowed:
> - `!hhh,vvv`: represents a pure hue at a variable brightness
> - `!hhh`: represents a pure hue at full brightness

> **📝Note:**
> HSV is used for the internal representation for all operations as it most closely resembles the range and operation of an RBG light. It also provides easy methods to transition in a way most familiar to human viewers. Value controls how "on" the light is (0 is off, 100 is fully powered). Saturation controls how pure the color is (0 is white, 100 is pure hue). Lastly, hue controls which two LEDs are primarily mixed (0 is red, 120 is green, 240 is blue, with 360 also being red).

HSL, hue-saturation-lightness, values can be specified by a tuple of values prefixed with a double exclamation point, `!!hhh,sss,lll`. Hue ranges from 0-359 with pure red at 0, pure green at 120, and pure blue at 240. Saturation and lightness range from 0-100. For example, the same orange as above can be represented as `!!14,82,62`.

These named colors are available:
| Name | RGB | HSV |
| :--: | :-- | :-- |
| red | `#FF0000` | `!0,100,100` |
| orange | `#FF7F00` | `!30,100,100` |
| yellow | `#FFFF00` | `!60,100,100` |
| neon | `#7FFF00` | `!90,100,100` |
| green | `#00FF00` | `!120,100,100` |
| seafoam | `#00FF7F` | `!150,100,100` |
| cyan | `#00FFFF` | `!180,100,100` |
| lightblue | `#007FFF` | `!210,100,100` |
| blue | `#0000FF` | `!240,100,100` |
| purple | `#7F00FF` | `!270,100,100` |
| magenta | `#FF00FF` | `!300,100,100` |
| pink | `#FF007F` | `!330,100,100` |
| white | `#FFFFFF` | `!0,0,100` |
| black/off | `#000000` | `!0,0,0` |

> **📝Note:**
> Since NeoPixels are purely emissive sources of light, no blacks are possible. Therefore, many colors will not be able to be accurately represented. For instance, grays are not possible since it will just be a dim, pure white instead.

For all color specifications, leading zeros are permitted for decimal values and upper- or lower-case letters may be used for hexadecimal values.
