
#!/usr/bin/python
#
# Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# $Id$
#
"""
Workrave Activity Monitor based on face detection.

Based on OpenCV examples.

Original C example implementation by:  ?
Python example implementation by: Roman Stanchak
"""

import os
import sys
import pwd
import dbus
import dbus.decorators
import dbus.glib
import time
import gobject

import ConfigParser
from optparse import OptionParser

from opencv.cv import *
from opencv.highgui import *

class FaceActivityMonitor:

    def __init__(self):
        self.init_variables()
        self.init_options()
        self.init_dbus()
        self.init_face_detect()
        
    def cleanup(self):
        if self.show:
            cvDestroyWindow("result")

    def init_variables(self):
        self.cascade_name     = "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml"
        self.device           = 0
        self.show             = False
        self.verbose          = False
        self.face_threshold   = 2 
        self.noface_threshold = 5

        # Parameters for haar detection
        # From the API:
        # The default parameters (scale_factor=1.1, min_neighbors=3, flags=0) are tuned 
        # for accurate yet slow object detection. For a faster operation on real video 
        # images the settings are: 
        # scale_factor=1.2, min_neighbors=2, flags=CV_HAAR_DO_CANNY_PRUNING, 
        # min_size=<minimum possible face size
        self.min_size         = cvSize(20,20)
        self.image_scale      = 1.2
        self.haar_scale       = 1.2
        self.min_neighbors    = 2
        self.haar_flags       = CV_HAAR_DO_CANNY_PRUNING

        self.workrave = None
        self.cascase  = None
        self.capture  = None
        self.storage = None
        self.last_time = 0;
        self.frame_copy = None
        self.count_face = 0
        self.count_noface = 0
        self.ignore = False
        
    def init_options(self):
        usage = "usage: %prog [options]"    
        parser = OptionParser(usage=usage)
        parser.add_option("-d", "--devicenum",
                          dest="device", default=0,
                          help="/dev/video device number, default = 0",
                          )

        parser.add_option("-c", "--cascade",
                          dest="cascade_name", default=None,
                          help="Haar cascade to use",
                          )

        parser.add_option("-s", "--show",
                          action="store_true", dest="show", default=True,
                          help="Show detection",
                          )

        parser.add_option("-v", "--verbose",
                          action="store_true", dest="verbose", default=False,
                          help="Verbose message",
                          )
        (options, args) = parser.parse_args()

        self.device = options.device
        self.show = options.show
        self.verbose = options.verbose
        
        if options.cascade_name:
            self.cascade_name = options.cascade_name


    def init_dbus(self):
        bus = dbus.SessionBus()
        obj = bus.get_object("org.workrave.Workrave", "/org/workrave/Workrave/Core")

        self.workrave = dbus.Interface(obj, "org.workrave.CoreInterface")

        self.workrave.connect_to_signal("MicrobreakChanged",
                                        self.microbreak_signal, sender_keyword='sender')
        self.workrave.connect_to_signal("RestbreakChanged",
                                        self.restbreak_signal, sender_keyword='sender')
        self.workrave.connect_to_signal("DailylimitChanged",
                                        self.dailylimit_signal, sender_keyword='sender')
        
    def init_face_detect(self):
        # the OpenCV API says this function is obsolete, but we can't
        # cast the output of cvLoad to a HaarClassifierCascade, so use this anyways
        # the size parameter is ignored
        self.cascade = cvLoadHaarClassifierCascade(self.cascade_name, cvSize(1,1))
    
        if not self.cascade:
            print "ERROR: Could not load classifier cascade"
            sys.exit(-1)
            
        self.capture = cvCreateCameraCapture(int(self.device))
        self.storage = cvCreateMemStorage(0)

        if self.show:
            cvNamedWindow("result", 1)
        

    def detect_and_draw(self, image):
        gray = cvCreateImage(cvSize(image.width,image.height), 8, 1)
        small_image = cvCreateImage(cvSize(cvRound(image.width/self.image_scale),
                                           cvRound(image.height/self.image_scale)),
                                    8, 1)
    
        cvCvtColor(image, gray, CV_BGR2GRAY)
        cvResize(gray, small_image, CV_INTER_LINEAR)
        cvEqualizeHist(small_image, small_image)
     
        cvClearMemStorage(self.storage)

        faces = cvHaarDetectObjects(small_image,
                                    self.cascade,
                                    self.storage,
                                    self.haar_scale,
                                    self.min_neighbors,
                                    self.haar_flags,
                                    self.min_size)
        if self.show and faces:
            for r in faces:
                pt1 = cvPoint(int(r.x*self.image_scale), int(r.y*self.image_scale))
                pt2 = cvPoint(int((r.x+r.width)*self.image_scale), int((r.y+r.height)*self.image_scale))
                cvRectangle(image, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0)

            cvShowImage("result", image)

        return faces and faces.total > 0

    def iteration(self):
        frame = cvQueryFrame(self.capture)
        if not frame:
            return False
             
        if frame.origin == IPL_ORIGIN_TL:
            user_detected = self.detect_and_draw(frame)
        else:
            if not self.frame_copy:
                self.frame_copy = cvCreateImage(cvSize(frame.width,frame.height),
                                                IPL_DEPTH_8U, frame.nChannels)
                
            cvFlip(frame, self.frame_copy, 0)
            user_detected = self.detect_and_draw(self.frame_copy)

        self.inform_workrave(user_detected)

        cvWaitKey(5)
        

    def inform_workrave(self, user_detected):

        if self.ignore:
            return
        
        if user_detected:
            self.count_face += 1
            self.count_noface = 0
        else:
            self.count_noface += 1
            self.count_face = 0
        
        if self.count_face >= self.face_threshold:

            if self.verbose and self.count_face == self.face_threshold:
                print "Reporting user presence"
                
            now = time.time()
            if now > self.last_time + 5:
                self.workrave.ReportActivity("facedetect", True)
                self.last_time = now

        if self.count_noface == self.noface_threshold:
            if self.verbose:
                print "Reporting user absence"
            self.workrave.ReportActivity("facedetect", False)

    def microbreak_signal(self, progress, sender=None):
        self.break_signal("microbreak", progress)
        
    def restbreak_signal(self, progress, sender=None):
        self.break_signal("restbreak", progress)

    def dailylimit_signal(self, progress, sender=None):
        self.break_signal("dailylimit", progress)

    def break_signal(self, breakid, progress, sender=None):
        if progress != "none":
            self.count_face = 0
            self.count_noface = 0
            self.ignore = True;
            self.workrave.ReportActivity("facedetect", False)
        else:
            self.ignore = False;
        
        if self.verbose:
            if progress == "prelude":
                print "Ignoring user presence: %s warning" % breakid
            elif progress == "break":
                print "Ignoring user presence: %s started" % breakid
            elif progress == "none":
                print "Resuming presence monitoring: Break %s idle" % breakid
            else:
                print "Unknown progress for %s: %s" % (breakid, progress)


class Timeout(gobject.Timeout):
    def __init__(self, context, interval, func, *args):
        gobject.Timeout.__init__(self, interval, priority=gobject.PRIORITY_HIGH)
        self.set_callback(self.callback, func, *args)
        self.attach(context)

    def callback(self, func, *args):
        func(*args)
        return True

if __name__ == '__main__':

    am = FaceActivityMonitor()

    context = gobject.MainContext()
    loop = gobject.MainLoop(context=context)

    source = Timeout(context, 1000, am.iteration)

    loop.run()
