#!/usr/bin/python

import os;
import sys;

options = [ "gconf",
            "xml",
            "gnome3",
            "indicator",
            "distribution",
            "gstreamer",
            "dbus",
            "exercises",
            "pulse",
            "debug",
            "x11-monitoring-fallback",
            "tracing"]

sys.stdout.write("Q=@\n\n")
sys.stdout.write("all:\n\n")

for i in range(0, 1024) :
    d = ""
    conf = "";
    for j in range(0, 10) :
        if i & (1 << j) :
            conf = conf + "--disable-" + options[j] + " "
            d = d + "1"
        else :
            conf = conf + "--enable-" + options[j] + " "
            d = d + "0"

    dir = "testconf/" + d
            
    sys.stdout.write("all: " + d + "\n");
    sys.stdout.write(".PHONY: " + d + "\n");
    sys.stdout.write(d + ":\n");
    sys.stdout.write("\t-$(Q)rm -rf " + dir + "*\n")
    sys.stdout.write("\t$(Q)mkdir -p " + dir + "\n")
    sys.stdout.write("\t$(Q)(cd " + dir + " && ../../configure " + conf + ") > " + dir + "-conf.log 2>&1\n");
    sys.stdout.write("\t$(Q)date +\"%y-%m-%d %H:%M:%S " + conf + "\"\n")
    sys.stdout.write("\t$(Q)$(MAKE) -C " + dir + " > " + dir + "-make.log 2>&1\n")
    sys.stdout.write("\t-$(Q)rm -rf " + dir + "*\n")
    sys.stdout.write("\n")
