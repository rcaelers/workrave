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

        workrave = dbus.Interface(obj, "org.workrave.CoreInterface")

        workrave.connect_to_signal("MicrobreakChanged",
                                   self.microbreak_signal, sender_keyword='sender')
        workrave.connect_to_signal("RestbreakChanged",
                                   self.restbreak_signal, sender_keyword='sender')
        workrave.connect_to_signal("DailylimitChanged",
                                   self.dailylimit_signal, sender_keyword='sender')

    def microbreak_signal(self, progress, sender=None):
        self.break_signal("microbreak", progress)
        
    def restbreak_signal(self, progress, sender=None):
        self.break_signal("restbreak", progress)

    def dailylimit_signal(self, progress, sender=None):
        self.break_signal("dailylimit", progress)

    def break_signal(self, breakid, progress, sender=None):
        print "Break %s progress %s" % (breakid, progress)
        if progress == "break" and breakid == "microbreak":
            #slow down mouse with mouse-speed from https://github.com/rubo77/mouse-speed/blob/master/usr/bin/mouse-speed
            subprocess.call(["/usr/bin/mouse-speed", "-d", "30"])
        # elif progress == "break":
        #    print "Break %s started" % breakid
        elif progress == "none" and breakid == "microbreak":
            #reset mouse speed to 100%
            #TODO: only call this if the microbreak was long enough
            #use GetTimerElapsed/GetTimerIdle to figure this out
            subprocess.call(["/usr/bin/mouse-speed", "-r"])
        # else:
        #    print "Unknown progress for %s: %s" % (breakid, progress)

if __name__ == '__main__':

    d = WorkraveDBus()

    loop = gobject.MainLoop()
    loop.run()
