These Python scripts can be used with Waybar and Yambar, and possibly other clients as well. They require the `pydbus` Python module.

The script `workrave_break_info.py` can also be used as a Python module (which is why it has underscores in its name instead of hyphens) for those who don't find the script flexible enough. (For example, a user of the Python-configured window manager/compositor qtile may prefer to use it as a module. Documentation for its use as a module is in the comments near the top of the script.)

To use with Waybar, place the files `workrave_break_info.py` and `workrave-open.py` somewhere in your `$PATH`, and add the following to your Waybar config file:
```json
    "custom/workrave": {
        "exec": "workrave_break_info.py -f waybar",
        "return-type": "json",
        "on-click": "workrave-open.py",
        "exec-on-event": false
    }
```
Of course, `"custom/workrave"` should be added to one of these arrays in the Waybar config file: `modules-left`, `modules-center`, `modules-right`.

Note that all `workrave-open.py` does is open Workrave's status window if it isn't already open (which doesn't necessarily happen if one simply types in `workrave` at the command-line).

To use `workrave_break_info.py` with Yambar, the following can be added to the Yambar config file:
```yaml
    - script:
         path: ~/bin/workrave_break_info.py
         args:
           - "-f"
           - "yambar"
         anchors:
           - default_color: &default_color {foreground: 87ceebff} #skyblue
           - close_to_break_color: &close_to_break_color {foreground: ffa500ff} #orange
           - overdue_color: &overdue_color {foreground: ff6347ff}
           - disabled_color: &disabled_color {foreground: 808080ff} #grey
         content:
           list:
             items:
               - map:
                   conditions:
                     microbreak_enabled:
                       map:
                         conditions:
                           microbreak_state == "default":
                             string:
                               text: "{microbreak}"
                               <<: *default_color
                           microbreak_state == "close to break":
                             string:
                               text: "{microbreak}"
                               <<: *close_to_break_color
                           microbreak_state == "overdue":
                             string:
                               text: "{microbreak}"
                               <<: *overdue_color
                     ~microbreak_enabled:
                        string:
                          text: "M: --"
                          <<: *disabled_color
               - map:
                   conditions:
                     restbreak_enabled:
                       map:
                         conditions:
                           restbreak_state == "default":
                             string:
                               text: "{restbreak}"
                               <<: *default_color
                           restbreak_state == "close to break":
                             string:
                               text: "{restbreak}"
                               <<: *close_to_break_color
                           restbreak_state == "overdue":
                             string:
                               text: "{restbreak}"
                               <<: *overdue_color
                     ~restbreak_enabled:
                        string:
                          text: "R: --"
                          <<: *disabled_color
               - map:
                   conditions:
                     dailylimit_enabled:
                       map:
                         conditions:
                           dailylimit_state == "default":
                             string:
                               text: "{dailylimit}"
                               <<: *default_color
                           dailylimit_state == "close to break":
                             string:
                               text: "{dailylimit}"
                               <<: *close_to_break_color
                           dailylimit_state == "overdue":
                             string:
                               text: "{dailylimit}"
                               <<: *overdue_color
                     ~dailylimit_enabled:
                        string:
                          text: "D: --"
                          <<: *disabled_color
```
The above configuration of course assumes that `workrave_break_info.py` is in `~/bin`. (Yambar does not use `$PATH` for its script modules.) `default_color`, `close_to_break_color`, and `overdue_color` can be adjusted to one's taste, compatibility with Yambar's background, etc.

The `workrave_break_info.py` script also offers two other formats as well. One is "plain" format, which just has the script repeatedly prints the timer information from Workrave as brief plain text, e.g. `M: 4:53/5:00 R: 19:12/55:00`.

The other format is "json", where the script repeatedly outputs Workrave's timer information in a JSON format that can be used, for example, by [Elkowar's Wacky Widgets](https://elkowar.github.io/eww/) or [Sfwbar](https://github.com/LBCrion/sfwbar). Each line of output is the string representation of a JSON object with the following keys and values:

* `"microbreak_enabled"`: A Boolean that indicates if microbreaks are enabled
* `"microbreak_left_in_seconds"`: An integer indicating the number of seconds left until the microbreak starts
* `"microbreak_left_str"`: A string indicating the time left until the microbreak starts, expressed in a time format like `"%M:%S"` (or `"%H:%M:%S"` for time intervals longer than a hour)
* `"microbreak_limit_in_seconds"`: An integer indicating the interval of time between microbreaks as a number of seconds
* `"microbreak_limit_str"`: A string indicating the interval of time between microbreaks, expressed in a time format like `"%M:%S"` (or `"%H:%M:%S"` for time intervals longer than a hour)
* `"microbreak_state"`: A string indicating the state of the microbreak, either "default", "close to break" (for when the microbreak is about to start), or "overdue" (for when a microbreak is, well, overdue)
* `"microbreak_fgcol"`: The desired foreground color for the display of the microbreak
* `"microbreak_bgcol"`: The desired background color for the display of the microbreak
* `"restbreak_enabled"`: A Boolean that indicates if rest breaks are enabled
* `"restbreak_left_in_seconds"`: An integer indicating the number of seconds left until the rest break starts
* `"restbreak_left_str"`: A string indicating the time left until the rest break starts, expressed in a time format like `"%M:%S"` (or `"%H:%M:%S"` for time intervals longer than a hour)
* `"restbreak_limit_in_seconds"`: An integer indicating the interval of time between rest breaks as a number of seconds
* `"restbreak_limit_str"`: A string indicating the interval of time between rest breaks, expressed in a time format like `"%M:%S"` (or `"%H:%M:%S"` for time intervals longer than a hour)
* `"restbreak_state"`: A string indicating the state of the rest break, either "default", "close to break" (for when the rest break is about to start), or "overdue" (for when a rest break is, well, overdue)
* `"restbreak_fgcol"`: The desired foreground color for the display of the rest break
* `"restbreak_bgcol"`: The desired background color for the display of the rest break
* `"dailylimit_enabled"`: A Boolean that indicates if the daily limit is enabled
* `"dailylimit_left_in_seconds"`: An integer indicating the number of seconds in the daily limit
* `"dailylimit_left_str"`: A string indicating the length of the daily limit, expressed in a time format like `"%M:%S"` (or `"%H:%M:%S"` for time intervals longer than a hour)
* `"dailylimit_in_seconds"`: An integer indicating the time in seconds until the daily limit is reached
* `"dailylimit_str"`: A string indicating the time until the daily limit is reached, expressed in a time format like `"%M:%S"` (or `"%H:%M:%S"` for time intervals longer than a hour)
* `"dailylimit_state"`: A string indicating the state of the daily limit, either "default", "close to break" (for when the daily limit is close to being reached), or "overdue" (for when the daily limit has been exceeded)
* `"dailylimit_fgcol"`: The desired foreground color for the display of the daily limit
* `"dailylimit_bgcol"`: The desired background color for the display of the daily limit

A given client using this JSON output may, of course, ignore at least some of these keys. Example files showing JSON format being used with EWW and Sfwbar are in the directories named, of course, "eww" and "sfwbar", respectively.

Here is the usage of `workrave_break_info.py`:
```
usage: workrave_break_info.py [-h] [-i POLLING_INTERVAL] [-f {plain,waybar,yambar,json}]
                              [--colors-default COLORS_DEFAULT COLORS_DEFAULT]
                              [--colors-close-to-break COLORS_CLOSE_TO_BREAK COLORS_CLOSE_TO_BREAK]
                              [--colors-overdue COLORS_OVERDUE COLORS_OVERDUE]

options:
  -h, --help            show this help message and exit
  -i POLLING_INTERVAL, --polling-interval POLLING_INTERVAL
                        Time interval for polling Workrave, in seconds [default = 1.0]
  -f {plain,waybar,yambar,json}, --format {plain,waybar,yambar,json}
                        Format for output [default = plain]
  --colors-default COLORS_DEFAULT COLORS_DEFAULT
                        Default timer colors (if color is used in the output format). First color is
                        text color, second is background color. [default = ['black', 'skyblue']]
  --colors-close-to-break COLORS_CLOSE_TO_BREAK COLORS_CLOSE_TO_BREAK
                        Timer color when close to break time (if color is used in the output format).
                        First color is text color, second is background color. [default = ['black',
                        'orange']]
  --colors-overdue COLORS_OVERDUE COLORS_OVERDUE
                        Timer colors when interval between breaks exceeded (if color is used in the
                        output format. First color is text color, second is background color. [default
                        = ['white', 'red']]
```
