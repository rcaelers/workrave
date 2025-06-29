#!/usr/bin/env python3

from dasbus.connection import SessionMessageBus
from collections.abc import Sequence
from collections import namedtuple

#########################################################################
#
# For those who want to edit this script's behavior (especially to add
# a new format) or to use it as a module ...
#
# The "fmt_timer_{$FORMAT}" function, where $FORMAT is a format type
# (plain, waybar, etc.), formats the string for an individual timer,
# i.e.  microbreak, rest break, or daily limit. This function takes
# the following inputs to describe the timer:
#
#  * enabled: a Boolean indicating if the timer is enabled
#
#  * time_left: in seconds, the time left until the next break, or for
#    the case of the daily limit, the time left for computer activity
#    for the day
#
#  * limit: in seconds, the time between breaks, or for the case of
#    the daily limit, the daily limit itself
#
#  * timer_type: a string with one of the following values:
#    "microbreak", "restbreak", or "dailylimit".
#
#  * timer_state: a string with one of the following values:
#    "default", "close to break", and "overdue".
#
#  * colors: a tuple of two strings, the first with a string
#    indicating the foreground (or text) color, and the second
#    indicating the background color. The colors indicate the value of
#    timer_state.
#
# The "fmt_all_timers_{$FORMAT}" function takes a list of strings,
# each of which has information on an individual timer, and produces a
# string that contains informations about all of the timers
# described in that list.
#
# Here's how these functions are used. For example, suppose that:
#
#   * The interval between microbreaks is 5 minutes and the time to
#     the next microbreak is 3 minutes.
#
#   * The interval between rest breaks is 50 minutes and the time to
#     the next rest break is 15 minutes.
#
#   * The daily limit is 4 hours, and time left for computer activity
#     is 2.5 hours.
#
# Then fmt_timer_plain will generate the string "M 3:00/5:00" for the
# microbreak, "R 15:00/50:00" for the rest break, and
# "D 4:00:00/2:30:00" for the daily limit. fmt_all_timers_plain will
# receive the list of strings ["M 3:00/5:00", "R 15:00/50:00",
# "D 4:00:00/2:30:00"] and output the following string:
#
# "M 3:00/5:00 R 15:00/50:00 D 4:00:00/2:30:00"
#
# The dictionary fmt_info_dict associates a string that labels the
# format with a tuple containing the "fmt_timer_{$FORMAT}"
# function and the "fmt_all_timers_{$FORMAT}" function.
# The first key in fmt_info_dict is the default format (i.e.,
# "plain").
#
# To add, say, format "foo", define the functions fmt_timer_foo and
# fmt_all_timers_foo, and then add the following entry to fmt_info_dict:
#
#   foo: (fmt_timer_foo, fmt_all_timers_foo)
#
# This script can also be used as a module, and in addition to the
# aforementioned functions, it also has the class WorkraveBreakInfo,
# whose constructor has the following signature:
#
# class WorkraveBreakInfo(color_dict = None,
#                         get_timer_state_func = None)
#
# The optional keyword argument color_dict is a dictionary with three
# keys, which correspond to the aforementioned three states of the
# timer: "default", "close to break", and "overdue", and the values
# corresponding to each of those keys is a two-element sequence whose
# first element indicates a foreground color and whose second element
# indicates a background color. Both elements are strings containing
# some color specification suitable for the chosen fmt_timer_func.
#
# The optional keyword argument get_timer_state_func is a function
# that returns one of the strings "default", "close to break", or
# "overdue", depending on its two arguments: "time_left" and
# "limit". The first is the time left until a break starts, and the
# second is the minimum time between breaks (i.e., a *limit* on the
# amount of time that one can be active before the break starts). By
# default, a timer is considered "close to break" if time_left/limit
# <= 0.1 and "overdue" if time_left is negative.
#
# The class WorkraveBreakInfo has the following public methods:
#
#     * get_timer_info(): Takes one argument, the break type, which
#       can be "microbreak", "restbreak", or "dailylimit". It returns
#       a named tuple of type TimerInfo whose fields are all the
#       arguments needed for a "fmt_timer_{$FORMAT}" function:
#       "enabled", "time_left", "limit", "timer_type", "timer_state",
#       and "colors". Give such a tuple named "foo" and a
#       "fmt_timer_{$FORMAT}" function named "fmt_timer_baz", one can
#       use this tuple with that function simply by unpacking it,
#       i.e., doing this: fmt_timer_baz(*foo).
#
#     * open_workrave(): This just opens a Workrave status window if
#       one is not already open. It basically does the same thing as
#       workrave-open.py.
#
# This class does *not* do any polling. That is either the job of the
# module when it is used as a script, or the job of whatever Python
# script wishes to use the class -- if it needs to do any polling at
# all.
#
# There is another free function in this module,
# seconds_to_time_str(), which takes a time interval in seconds as an
# argument, and returns a string in the format "%M:%S" (or "%H:%M:%S"
# for time intervals longer than a hour). It can be used in a custom
# "fmt_timer_{$FORMAT}" function.
#
#########################################################################

def fmt_timer_plain(enabled, time_left, limit, timer_type, timer_state, colors):

    if not enabled: return ""

    time_left_str = seconds_to_time_str(time_left)
    limit_str = seconds_to_time_str(limit)

    prefix = {"microbreak": "M",
              "restbreak": "R",
              "dailylimit": "D"}[timer_type]
    
    return f"{prefix}: {time_left_str}/{limit_str}"

def fmt_all_timers_plain(timer_strs):
    return " ".join(timer_str for timer_str in timer_strs if timer_str)

def fmt_timer_waybar(enabled, time_left, limit, timer_type, timer_state, colors):

    if not enabled: return ""
    
    timer_str = fmt_timer_plain(enabled, time_left, limit, timer_type, timer_state, colors)
    
    return f"<span color='{colors[0]}' bgcolor='{colors[1]}'>{timer_str}</span>"

def fmt_all_timers_waybar(timer_strs):
    all_timers_str = " ".join(timer_str for timer_str in timer_strs if timer_str)
    
    return  f"""{{"text": "{all_timers_str}", "tooltip": "Workrave"}}"""

def fmt_timer_yambar(enabled, time_left, limit, timer_type, timer_state, colors):

    timer_str = fmt_timer_plain(True, time_left, limit, timer_type, timer_state, colors)
    
    return "\n".join((f"{timer_type}|string|{timer_str.replace('|', '/')}",
                      f"{timer_type}_enabled|bool|{str(enabled).lower()}",
                      f'{timer_type}_state|string|{timer_state}'))

def fmt_all_timers_yambar(timer_strs):
    all_timers_str = "\n".join(timer_strs)
    return f"{all_timers_str}\n"

def fmt_timer_json(enabled, time_left, limit, timer_type, timer_state, colors):
    limit_str_key_prefix = (f"daily" if timer_type == "dailylimit"
                            else f"{timer_type}_")
    
    vars_str = ", ".join([f'"{timer_type}_enabled": {str(enabled).lower()}',
                          f'"{timer_type}_left_in_seconds": {time_left}',
                          f'"{limit_str_key_prefix}limit_in_seconds": {limit}',
                          f'"{timer_type}_left_str": "{seconds_to_time_str(time_left)}"',
                          f'"{limit_str_key_prefix}limit_str": "{seconds_to_time_str(limit)}"',
                          f'"{timer_type}_state": "{timer_state}"',
                          f'"{timer_type}_fgcol": "{colors[0]}"',
                          f'"{timer_type}_bgcol": "{colors[1]}"'])

    return f"{{{vars_str}}}"

def fmt_all_timers_json(timer_strs):
    all_timers_str = ", ".join(timer_str[1:-1] # [1:-1] removes
                                               # initial and final
                                               # braces.
                               for timer_str in timer_strs)
    
    return f"{{{all_timers_str}}}"

fmt_info_dict = {
    "plain": (fmt_timer_plain, fmt_all_timers_plain),
    "waybar": (fmt_timer_waybar, fmt_all_timers_waybar),
    "yambar": (fmt_timer_yambar, fmt_all_timers_yambar),
    "json": (fmt_timer_json, fmt_all_timers_json)
}

#### Shouldn't need to edit anything below here. ####

def seconds_to_time_str(tot_num_sec):
    abs_tot_num_sec = abs(tot_num_sec)
    sign = "" if tot_num_sec >= 0 else "-"

    if abs_tot_num_sec >= 3600:
        num_hr, rem_num_sec =  divmod(abs_tot_num_sec, 3600)
        num_min, num_sec = divmod(rem_num_sec, 60)

        return f"{sign}{num_hr}:{num_min:02d}:{num_sec:02d}"
    else:
        num_min, num_sec = divmod(abs_tot_num_sec, 60)
        return f"{sign}{num_min}:{num_sec:02d}"

_default_color_dict = {
    "default": ["black", "skyblue"],
    "close to break": ["black", "orange"],
    "overdue": ["white", "red"]
}

def _default_get_timer_state(time_left, limit):
    if time_left < 0:
        return "overdue"
    elif time_left/limit <= 0.1:
        return "close to break"
    else:
        return "default"

TimerInfo = namedtuple(
    "TimerInfo",
    "enabled, time_left, limit, timer_type, timer_state, colors"
)
    
class WorkraveBreakInfo:
    def __init__(self, *,
                 color_dict = None,
                 get_timer_state_func = None):
        
        self.session_bus = SessionMessageBus()

        self._wr_core = self.session_bus.get_proxy(
            'org.workrave.Workrave',
            '/org/workrave/Workrave/Core',
            interface_name = "org.workrave.CoreInterface"
        )

        self._wr_cfg = self.session_bus.get_proxy(
            'org.workrave.Workrave',
            '/org/workrave/Workrave/Core',
            interface_name = "org.workrave.ConfigInterface"
        )

        self._timer_enabled_paths = {
            "microbreak": "/breaks/micro-pause/enabled",
            "restbreak": "/breaks/rest-break/enabled",
            "dailylimit": "/breaks/daily-limit/enabled"
        }

        self._timer_limit_paths = {
            "microbreak": "/timers/micro_pause/limit",
            "restbreak": "/timers/rest_break/limit",
            "dailylimit": "/timers/daily_limit/limit"
        }

        self.color_dict = (_default_color_dict
                           if color_dict is None
                           else color_dict)

        self.get_timer_state = (_default_get_timer_state
                                if get_timer_state_func is None
                                else get_timer_state_func)

        self._check_color_dict()

    def _check_color_dict(self):

        for key in ("default", "close to break", "overdue"):
            if key not in self.color_dict:
                raise ValueError(f"'{key}' is required key in "
                                 "'color_dict' argument")

            if not isinstance(self.color_dict[key], Sequence):
                raise ValueError(f"Value associated with key '{key}' "
                                 "must be a sequence")

            if len(self.color_dict[key]) != 2:
                raise ValueError("Length of sequence associated with "
                                 f"key '{key}' is not 2. Sequence is "
                                 f"{self.color_dict[key]}")

    def get_timer_info(self, timer_type):

        timer_enabled_path = self._timer_enabled_paths[timer_type]

        timer_limit_path = self._timer_limit_paths[timer_type]

        enabled = self._wr_cfg.GetBool(timer_enabled_path)[0]

        limit = self._wr_cfg.GetInt(timer_limit_path)[0]
        time_left = limit - self._wr_core.GetTimerElapsed(timer_type)

        timer_state = self.get_timer_state(time_left, limit)
        colors = self.color_dict[timer_state]

        return TimerInfo(enabled, time_left, limit,
                         timer_type, timer_state, colors)

    def open_workrave(self):
        wr_ui = self.session_bus.get_proxy('org.workrave.Workrave',
                                           '/org/workrave/Workrave/UI')

        wr_ui.MenuAction('workrave.open')

def main():
    import argparse
    import time

    output_fmts = tuple(fmt_info_dict.keys())

    parser = argparse.ArgumentParser()

    parser.add_argument("-i", "--polling-interval", type = float, default = 1.0,
                        help = "Time interval for polling Workrave, in seconds "
                        "[default = %(default)s]")

    parser.add_argument("-f", "--format",
                        choices = output_fmts, default = output_fmts[0],
                        help = "Format for output [default = %(default)s]")

    parser.add_argument("--colors-default", nargs = 2,
                        default = _default_color_dict["default"],
                        help = "Default timer colors (if color is used in the output "
                        "format). First color is text color, second is background color. "
                        "[default = %(default)s]")

    parser.add_argument("--colors-close-to-break", nargs = 2,
                        default = _default_color_dict["close to break"],
                        help = "Timer color when close to break time (if color is used in "
                        "the output format).  First color is text color, second is "
                        "background color.  [default = %(default)s]")

    parser.add_argument("--colors-overdue", nargs = 2,
                        default = _default_color_dict["overdue"],
                        help = "Timer colors when interval between breaks exceeded (if "
                        "color is used in the output format. First color is text color, "
                        "second is background color. [default = %(default)s]")

    args = parser.parse_args()

    fmt_timer_func, fmt_all_timers_func = fmt_info_dict[args.format]

    wr_break_info = WorkraveBreakInfo(color_dict = {
        "default": args.colors_default,
        "close to break": args.colors_close_to_break,
        "overdue": args.colors_overdue
    })
    
    while True:
        print(
            fmt_all_timers_func(
                [fmt_timer_func(*wr_break_info.get_timer_info(timer_type))
                 for timer_type in ("microbreak", "restbreak", "dailylimit")]
            ), flush = True
        )

        time.sleep(args.polling_interval)

if __name__ == "__main__":
    main()
