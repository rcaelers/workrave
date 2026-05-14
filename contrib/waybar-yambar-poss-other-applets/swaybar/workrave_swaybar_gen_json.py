#!/usr/bin/env python3

polling_interval = 1.0
date_format = "%a %b %d, %I:%M %p"

import json
import sys
import time
from datetime import datetime

from workrave_break_info import fmt_timer_plain, WorkraveBreakInfo

wr_break_info = WorkraveBreakInfo(color_dict = {
    "default": ("#000000", "#87CEEB"),
    "close to break": ("#000000", "#FFA500"),
    "overdue": ("#FFFFFF", "#FF0000")
})

print('{ "version": 1, "click_events": true}\n[')

while True:
    print("[")

    for timer_type in ("microbreak", "restbreak", "dailylimit"):
        timer_info = wr_break_info.get_timer_info(timer_type)

        if timer_info.enabled:
            print(f'   {{"full_text": "{fmt_timer_plain(*timer_info)}", '
                  f'"name": "workrave", "instance": "wr_{timer_type}", '
                  f'"color": "{timer_info.colors[0]}", '
                  f'"background": "{timer_info.colors[1]}"}},') # Note comma at end

    print(f'   {{"full_text": "{datetime.now().strftime(date_format)}"}}')

    print("],", flush = True) 

    time.sleep(polling_interval)
