import unittest

import os
import sys
import pwd
import dbus
import dbus.decorators
import dbus.glib
import time
import gobject

loop = gobject.MainLoop()
bus = dbus.SessionBus()

run_debugger = False

class WorkraveTestBase(unittest.TestCase):
        
    def get_num_autostart_workraves(self):
        return 3
    
    def start_workrave(self, instance, clean = True):

        name = "workrave" + str(instance)
        tmpdir = "/tmp/" + name + "/"
        pidfile = tmpdir + "pid"
        cwd = os.getcwd();

        env = os.environ
        env["WORKRAVE_TEST"]="1"
        env["WORKRAVE_FAKE"]="1"
        env["WORKRAVE_DBUS_NAME"] = "org.workrave.Workrave" + str(instance)
        env["WORKRAVE_HOME"] = tmpdir

        if clean:
            try:
                os.system("rm -rf " + tmpdir + ".workrave");
                os.mkdir(tmpdir)
            except:
                pass

        newpid = os.fork()
        if newpid == 0:
            out_log = file(name + ".out", 'w+')
            dev_null = file('/dev/null', 'r')

            os.dup2(out_log.fileno(), sys.stdout.fileno())
            os.dup2(out_log.fileno(), sys.stderr.fileno())
            os.dup2(dev_null.fileno(), sys.stdin.fileno())
            
            os.execlpe("/sbin/start-stop-daemon",
                       "/sbin/start-stop-daemon",               
                       "--quiet",
                       "--start",
                       "--pidfile", pidfile,
                       "--make-pidfile",
                       "--exec",
                       #"/usr/bin/valgrind",
                       #"--", "-v", "--trace-children=yes", "--leak-check=full",
                       #"--show-reachable=yes",
                       #"--log-file=" + cwd +"/val." + str(instance),  
                       cwd + "/ui/app/text/src/workrave",
                       env)
                  

    def stop_workrave(self, instance):
        name = "workrave" + str(instance)
        tmpdir = "/tmp/" + name + "/"
        pidfile = tmpdir + "pid"

        newpid = os.fork()
        if newpid == 0:
            os.execlp("/sbin/start-stop-daemon",
                      "/sbin/start-stop-daemon",
                      "--stop",
                      "--pidfile", pidfile)

    def launch(self, num, clean = True):
        for i in range(num):
            self.start_workrave(i + 1, clean)
        time.sleep(2)

        self.wr = []
        self.wrd = []
        self.core = []
        self.network = []
        self.config = []
        self.debug = []

        for i in range(num):
            self.wr.append(bus.get_object("org.workrave.Workrave" + str(i + 1), "/org/workrave/Workrave/Core"))
            self.wrd.append(bus.get_object("org.workrave.Workrave" + str(i + 1), "/org/workrave/Workrave/Debug"))

            self.core.append   (dbus.Interface(self.wr[i], "org.workrave.CoreInterface"))
            self.network.append(dbus.Interface(self.wr[i], "org.workrave.NetworkInterface"))
            self.config.append (dbus.Interface(self.wr[i], "org.workrave.ConfigInterface"))
            self.debug.append  (dbus.Interface(self.wrd[i], "org.workrave.DebugInterface"))

        if run_debugger:
            for i in range(1, num + 1):
                os.system("cat /tmp/workrave" + str(i) + "/pid")
            print "Attach debugger and press return."
            sys.stdin.readline();
            time.sleep(1)

        time.sleep(1)
        for i in range(num):
            # Start listening for incoming connections.
            self.network[i].Listen(2701 + i)

            self.config[i].SetBool("timers/micro_pause/enabled", True)
            self.config[i].SetInt("timers/micro_pause/auto_reset", 20)
            self.config[i].SetInt("timers/micro_pause/limit", 60)
            self.config[i].SetInt("timers/micro_pause/snooze", 180)
            self.config[i].SetBool("timers/rest_break/enabled", False)
            self.config[i].SetBool("timers/daily_limit/enabled", False)

        self.num_running = num;

    def kill(self):
        time.sleep(2)
        if run_debugger:
            print "Press return"
            sys.stdin.readline();

        for i in range(1, self.num_running + 1):
            self.stop_workrave(i)
        
    def setUp(self):

        self.num_running = 0
        self.wr = []
        self.wrd = []
        self.core = []
        self.network = []
        self.config = []
        self.debug = []
        self.connected = False
        
        num = self.get_num_autostart_workraves()
        self.launch(num);
        time.sleep(1)

    def tearDown(self): 
        self.kill()
        time.sleep(4)
