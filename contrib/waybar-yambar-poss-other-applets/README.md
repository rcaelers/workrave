These Python scripts can be used with Waybar and Yambar. They require the `pydbus` Python module.

To use with Waybar, place the files `workrave-break-info.py` and `workrave-open.py` somewhere in your `$PATH`, and add the following to your Waybar config file:
```json
    "custom/workrave": {
        "exec": "workrave-break-info.py -f waybar",
        "return-type": "json",
        "on-click": "workrave-open.py",
        "exec-on-event": false
    }
```
Of course, `"custom/workrave"` should be added to one of these arrays in the config file: `modules-left`, `modules-center`, `modules-right`.

Note that all `workrave-open.py` does is open Workrave's status window (which doesn't necessarily happen if one simply types in `workrave` at the command-line).

To use `workrave-break-info.py` with Yambar, the following can be added to the Yambar config file:
```yaml
    - script:
         path: ~/bin/workrave-break-info.py
         args:
           - "-f"
           - "yambar"
         anchors:
           - default_color: &default_color {foreground: 87ceebff} #skyblue
           - close_to_break_color: &close_to_break_color {foreground: ffa500ff} #orange
           - overdue_color: &overdue_color {foreground: ff6347ff}
         content:
           list:
             items:
               - map:
                   conditions:
                     microbreak_state == "default":
                       string:
                         text: "{microbreak} D"
                         <<: *default_color
                     microbreak_state == "close to break":
                       string:
                         text: "{microbreak} C"
                         <<: *close_to_break_color
                     microbreak_state == "overdue":
                       string:
                         text: "{microbreak} O"
                         <<: *overdue_color
               - map:
                   conditions:
                     restbreak_state == "default":
                       string:
                         text: "{restbreak} D"
                         <<: *default_color
                     restbreak_state == "close to break":
                       string:
                         text: "{restbreak} C"
                         <<: *close_to_break_color
                     restbreak_state == "overdue":
                       string:
                         text: "{restbreak} O"
                         <<: *overdue_color
               # - map:
               #     conditions:
               #       dailylimit_state == "default":
               #         string:
               #           text: "{dailylimit} D"
               #           <<: *default_color
               #       dailylimit_state == "close to break":
               #         string:
               #           text: "{dailylimit} C"
               #           <<: *close_to_break_color
               #       dailylimit_state == "overdue":
               #         string:
               #           text: "{dailylimit} O"
               #           <<: *overdue_color
```
The above configuration of course assumes that `workrave-break-info.py` is in `~/bin`. (Yambar does not use `$PATH` for its script modules.) It also assumes that Workrave has been configured to not use its daily limit. If it is used, the appropriate section should be uncommented. `default_color`, `close_to_break_color`, and `overdue_color` can be adjusted to one's taste, compatibility with Yambar's background, etc.

Here is the usage of `workrave-break-info.py`:
```
usage: workrave-break-info.py [-h] [-i POLLING_INTERVAL] [-f {plain,waybar,yambar}]
                              [--colors-default COLORS_DEFAULT COLORS_DEFAULT]
                              [--colors-close-to-break COLORS_CLOSE_TO_BREAK COLORS_CLOSE_TO_BREAK]
                              [--colors-overdue COLORS_OVERDUE COLORS_OVERDUE]

options:
  -h, --help            show this help message and exit
  -i POLLING_INTERVAL, --polling-interval POLLING_INTERVAL
                        Time interval for polling Workrave, in seconds [default = 1.0]
  -f {plain,waybar,yambar}, --format {plain,waybar,yambar}
                        Format for output [default = plain]
  --colors-default COLORS_DEFAULT COLORS_DEFAULT
                        Default timer colors (if color is used in the output format). First color is
                        text color, second is background color. [default = ['black', 'skyblue']]
  --colors-close-to-break COLORS_CLOSE_TO_BREAK COLORS_CLOSE_TO_BREAK
                        Timer color when close to break time (if color is used in the output
                        format). First color is text color, second is background color. [default =
                        ['black', 'orange']]
  --colors-overdue COLORS_OVERDUE COLORS_OVERDUE
                        Timer colors when interval between breaks exceeded (if color is used in the
                        output format. First color is text color, second is background color.
                        [default = ['white', 'red']]
```
