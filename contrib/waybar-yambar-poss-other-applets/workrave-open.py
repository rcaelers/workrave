#!/usr/bin/env python3

from dasbus.connection import SessionMessageBus

session_bus = SessionMessageBus()

wr_ui = session_bus.get_proxy('org.workrave.Workrave',
                              '/org/workrave/Workrave/UI',
                              interface_name = "org.workrave.AppletInterface")

wr_ui.MenuAction('workrave.open')
