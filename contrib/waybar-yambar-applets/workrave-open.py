#!/usr/bin/env python3

import pydbus

session_bus = pydbus.SessionBus()

wr_ui = session_bus.get('org.workrave.Workrave',
                        '/org/workrave/Workrave/UI')

wr_ui.MenuAction('workrave.open')
