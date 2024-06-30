#!/usr/bin/env python3

import argparse
import time
import sys

import pydbus

#########################################################################
#
# For those who want to edit this script's behavior, especially to add
# a new format ...
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
    
    return ", ".join([f'"{timer_type}_enabled": {str(enabled).lower()}',
                      f'"{timer_type}_left_in_seconds": {time_left}',
                      f'"{limit_str_key_prefix}limit_in_seconds": {limit}',
                      f'"{timer_type}_left_str": "{seconds_to_time_str(time_left)}"',
                      f'"{limit_str_key_prefix}limit_str": "{seconds_to_time_str(limit)}"',
                      f'"{timer_type}_state": "{timer_state}"',
                      f'"{timer_type}_fgcol": "{colors[0]}"',
                      f'"{timer_type}_bgcol": "{colors[1]}"'])

def fmt_all_timers_json(timer_strs):
    all_timers_str = ", ".join(timer_strs)
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

def get_timer_state(time_left, limit):
    if time_left < 0:
        return "overdue"
    elif time_left/limit <= 0.1:
        return "close to break"
    else:
        return "default"

def timer_str(timer_type, wr_core, wr_cfg, fmt_timer_func, color_dict):

    timer_enabled_path = {
        "microbreak": "/breaks/micro-pause/enabled",
        "restbreak": "/breaks/rest-break/enabled",
        "dailylimit": "/breaks/daily-limit/enabled"}[timer_type]

    timer_limit_path = {
        "microbreak": "/timers/micro_pause/limit",
        "restbreak": "/timers/rest_break/limit",
        "dailylimit": "/timers/daily_limit/limit"}[timer_type]

    enabled = wr_cfg.GetBool(timer_enabled_path)[0]

    limit = wr_cfg.GetInt(timer_limit_path)[0]
    time_left = limit - wr_core.GetTimerElapsed(timer_type)

    timer_state = get_timer_state(time_left, limit)
    colors = color_dict[timer_state]

    return fmt_timer_func(enabled, time_left, limit, timer_type, timer_state, colors)

# def microbreak_str(wr_core, wr_cfg, fmt_timer_func, color_dict):
#     return timer_str("microbreak", wr_core, wr_cfg, fmt_timer_func, color_dict)

# def restbreak_str(wr_core, wr_cfg, fmt_timer_func, color_dict):
#     return timer_str("restbreak", wr_core, wr_cfg, fmt_timer_func, color_dict)

# def dailylimit_str(wr_core, wr_cfg, fmt_timer_func, color_dict):
#     return timer_str("dailylimit", wr_core, wr_cfg, fmt_timer_func, color_dict)

output_fmts = tuple(fmt_info_dict.keys())

parser = argparse.ArgumentParser()

parser.add_argument("-i", "--polling-interval", type = float, default = 1.0,
                    help = "Time interval for polling Workrave, in seconds "
                    "[default = %(default)s]")

parser.add_argument("-f", "--format",
                    choices = output_fmts, default = output_fmts[0],
                    help = "Format for output [default = %(default)s]")

parser.add_argument("--colors-default", nargs = 2, default = ["black", "skyblue"],
                    help = "Default timer colors (if color is used in the output "
                    "format). First color is text color, second is background color. "
                    "[default = %(default)s]")

parser.add_argument("--colors-close-to-break", nargs = 2, default = ["black", "orange"],
                    help = "Timer color when close to break time (if color is used in "
                    "the output format).  First color is text color, second is "
                    "background color.  [default = %(default)s]")

parser.add_argument("--colors-overdue", nargs = 2, default = ["white", "red"],
                    help = "Timer colors when interval between breaks exceeded (if "
                    "color is used in the output format. First color is text color, "
                    "second is background color. [default = %(default)s]")

args = parser.parse_args()

fmt_timer_func, fmt_all_timers_func = fmt_info_dict[args.format]

color_dict = {
    "default": args.colors_default,
    "close to break": args.colors_close_to_break,
    "overdue": args.colors_overdue
}
    
session_bus = pydbus.SessionBus()

wr_core = session_bus.get('org.workrave.Workrave',
                          '/org/workrave/Workrave/Core')

wr_cfg = wr_core["org.workrave.ConfigInterface"]

while True:
    
    print(
        fmt_all_timers_func(
            [timer_str(timer_type, wr_core, wr_cfg, fmt_timer_func, color_dict)
             for timer_type in ("microbreak", "restbreak", "dailylimit")]
        ), flush = True
    )
    
    time.sleep(args.polling_interval)
