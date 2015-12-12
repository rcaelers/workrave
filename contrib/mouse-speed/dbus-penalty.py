#!/usr/bin/python
import os
import sys
import pwd
import dbus
import dbus.decorators
import dbus.glib
import time
import gobject
import subprocess

class WorkraveDBus:

    def __init__(self):

        bus = dbus.SessionBus()
        obj = bus.get_object("org.workrave.Workrave", "/org/workrave/Workrave/Core")

        self.workrave = dbus.Interface(obj, "org.workrave.CoreInterface")
        self.config = dbus.Interface(obj, "org.workrave.ConfigInterface")

        self.workrave.connect_to_signal("MicrobreakChanged",
                                   self.on_microbreak_changed, sender_keyword='sender')
        self.workrave.connect_to_signal("RestbreakChanged",
                                   self.on_restbreak_changed, sender_keyword='sender')
        self.workrave.connect_to_signal("DailylimitChanged",
                                   self.on_dailylimit_signal, sender_keyword='sender')

    def on_microbreak_changed(self, progress, sender=None):
        self.on_break_changed("microbreak", progress)
        
    def on_restbreak_changed(self, progress, sender=None):
        self.on_break_changed("restbreak", progress)

    def on_dailylimit_signal(self, progress, sender=None):
        self.on_break_changed("dailylimit", progress)

    def on_break_changed(self, breakid, progress, sender=None):

        if progress == "prelude":
            print "Break warning %s" % breakid
        elif progress == "break":
            print "Break %s started" % breakid
            #slow down mouse with mouse-speed from https://github.com/rubo77/mouse-speed/blob/master/usr/bin/mouse-speed
            subprocess.call(["/usr/bin/mouse-speed", "-d", "30"])
        elif progress == "none":
            print "Break %s idle" % breakid
            self.on_break_idle(breakid)
        else:
            print "Unknown progress for %s: %s" % (breakid, progress)

    def on_break_idle(self, breakid):
        if breakid == "microbreak":
            configid = "micro_pause"
        elif breakid == "restbreak":
            configid = "rest_break"
        elif breakid == "dailylimit":
            configid = "daily_limit"
            
        limit = self.config.GetInt("/timers/%s/limit" % configid)[0]
        autoreset = self.config.GetInt("timers/%s/auto_reset" % configid)[0]

        if self.workrave.GetTimerIdle(breakid) >= autoreset:
            print "Break %s taken" % breakid
            #reset mouse speed to 100%
            #TODO: only call this if the microbreak was long enough
            #use GetTimerElapsed/GetTimerIdle to figure this out
            subprocess.call(["/usr/bin/mouse-speed", "-r"])
        elif self.workrave.GetTimerElapsed(breakid) < limit:
            print "Break %s skipped" % breakid
        else:
            print "Break %s postponed"

if __name__ == '__main__':

    d = WorkraveDBus()

    loop = gobject.MainLoop()
    loop.run()
