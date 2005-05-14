#!/usr/bin/python

import os;

options = [ "app-gtk",
            "gconf",
            "xml",
            "gnome",
            "gnomemm",
            "kde",
            "distribution",
            "exercises"]

for i in range(0, 255) :
    conf = "./configure ";
    for j in range(0, 7) :
        if i & (1 << j) :
            conf = conf + "--enable-" + options[j] + " "
        else :
            conf = conf + "--disable-" + options[j] + " "

    print conf + "\n";
    os.system(conf);
    os.system("make");
    os.system("make distclean");
    

