#!/usr/bin/env python3

import argparse
import time
import sys

import pydbus

def wrap_timer_waybar(str_val, colors, timer_type, timer_state):
    return f"<span color='{colors[0]}' bgcolor='{colors[1]}'>{str_val}</span>"

def wrap_timer_plain(str_val, colors, timer_type, timer_state):
    return str_val

def wrap_timer_yambar(str_val, colors, timer_type, timer_state):    
    return "\n".join((f"{timer_type}|string|{str_val.replace('|', '/')}",
                      f'{timer_type}_state|string|{timer_state}'))

wrap_timer_func_dict = {
    "plain": wrap_timer_plain,
    "waybar": wrap_timer_waybar,
    "yambar": wrap_timer_yambar
}

wrap_str_func_dict = {
    "plain": lambda x: x,
    "waybar": lambda x: f"""{{"text": "{x}", "tooltip": "Workrave"}}""",
    "yambar": lambda x: f"{x}\n"
}

separator_dict = {
    "plain": " ",
    "waybar": " ",
    "yambar": "\n"
}

output_fmts = tuple(wrap_timer_func_dict.keys())

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

def choose_colors(time_left, limit):
    if time_left < 0:
        return "overdue"
    elif time_left/limit <= 0.1:
        return "close to break"
    else:
        return "default"

def timer_str(timer_type, wr_core, wr_cfg, wrap_timer_func, color_dict):

    timer_obj_path = {
        "microbreak": "/timers/micro_pause/limit",
        "restbreak": "/timers/rest_break/limit",
        "dailylimit": "/timers/daily_limit/limit"}[timer_type]
    
    limit = wr_cfg.GetInt(timer_obj_path)[0]
    time_left = limit - wr_core.GetTimerElapsed(timer_type)

    timer_state = choose_colors(time_left, limit)
    colors = color_dict[timer_state]

    time_left_str = seconds_to_time_str(time_left)
    limit_str = seconds_to_time_str(limit)

    prefix = {"microbreak": "M",
              "restbreak": "R",
              "dailylimit": "D"}[timer_type]

    basic_str = f"{prefix}: {time_left_str}/{limit_str}"

    return wrap_timer_func(basic_str, colors, timer_type, timer_state)

def microbreak_str(wr_core, wr_cfg, wrap_timer_func, color_dict):
    return timer_str("microbreak", wr_core, wr_cfg, wrap_timer_func, color_dict)

def restbreak_str(wr_core, wr_cfg, wrap_timer_func, color_dict):
    return timer_str("restbreak", wr_core, wr_cfg, wrap_timer_func, color_dict)

def dailylimit_str(wr_core, wr_cfg, wrap_timer_func, color_dict):
    return timer_str("dailylimit", wr_core, wr_cfg, wrap_timer_func, color_dict)

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

wrap_timer_func = wrap_timer_func_dict[args.format]
wrap_str_func = wrap_str_func_dict[args.format]
separator = separator_dict[args.format]

color_dict = {
    "default": args.colors_default,
    "close to break": args.colors_close_to_break,
    "overdue": args.colors_overdue
}
    
session_bus = pydbus.SessionBus()

wr_core = session_bus.get('org.workrave.Workrave',
                          '/org/workrave/Workrave/Core')

wr_cfg = wr_core["org.workrave.ConfigInterface"]

show_micro_break = wr_cfg.GetBool("/breaks/micro-pause/enabled")[0]
show_rest_break = wr_cfg.GetBool("/breaks/rest-break/enabled")[0]
show_daily_limit = wr_cfg.GetBool("/breaks/daily-limit/enabled")[0]

funcs_to_repeat = []
if show_micro_break:
    funcs_to_repeat.append(microbreak_str)

if show_rest_break:
    funcs_to_repeat.append(restbreak_str)

if show_daily_limit:
    funcs_to_repeat.append(dailylimit_str)

while True:
    print(wrap_str_func(
        separator.join(func(wr_core, wr_cfg, wrap_timer_func, color_dict)
                       for func in funcs_to_repeat)), flush = True)
    
    time.sleep(args.polling_interval)
