import unittest

import os
import sys
import pwd
import dbus
import dbus.decorators
import dbus.glib
import time
import gobject

from workrave_test_base import WorkraveTestBase

loop = gobject.MainLoop()
bus = dbus.SessionBus()

class HistoryBaseTest(WorkraveTestBase):

    def get_num_autostart_workraves(self):
        return 2

    def check_history(self, hist, expected):
        for (ts, a) in hist:
            print ts, a
        
    def check_history(self, expected):
        for w in range(num_wr) :
            hist = self.debug[w].GetHistory()
            print hist
            self.check_history(self, hist, expected)
            
        
    def simulate(self, behaviour):
        num_wr = len(behaviour)
        timespan = len(behaviour[0])

        for w in range(num_wr) :
            self.debug[w].SetTickMode(1000)
            self.debug[w].Tick(1)

        for t in range(timespan) :
            for w in range(num_wr) :

                if behaviour[w][t] == '-' :
                    self.debug[w].SetFakeState(2)
                else:
                    self.debug[w].SetFakeState(4)

                self.debug[w].Tick(1)
                
        self.check_history("")
                
            
    def runTest(self):

        # Start listenin for incoming connections.
        self.network[0].Listen(2701)
        self.network[1].Listen(2702)

        self.config[0].SetBool("timers/micro_pause/enabled", True)
        self.config[0].SetInt("timers/micro_pause/auto_reset", 30)
        self.config[0].SetInt("timers/micro_pause/limit", 180)
        self.config[0].SetInt("timers/micro_pause/snooze", 180)
        self.config[0].SetBool("timers/rest_break/enabled", False)
        self.config[0].SetBool("timers/daily_limit/enabled", False)

        self.config[1].SetBool("timers/micro_pause/enabled", True)
        self.config[1].SetInt("timers/micro_pause/auto_reset", 30)
        self.config[1].SetInt("timers/micro_pause/limit", 180)
        self.config[1].SetInt("timers/micro_pause/snooze", 180)
        self.config[1].SetBool("timers/rest_break/enabled", False)
        self.config[1].SetBool("timers/daily_limit/enabled", False)

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++++++++++++------------------------------++++++++++----------+++++-----'
        b = '------------------------------++++++++++++++++++++++++++++++----------++++++++++--------------------'

        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        idle = self.core[0].GetTimerIdle(0)
        print idle
        self.failUnlessEqual(idle, 9, "Incorrect idle time")
        
        elapsed = self.core[0].GetTimerElapsed(0)
        print elapsed
        self.failUnlessEqual(elapsed, 60, "Incorrect elapsed time")

        idle = self.core[1].GetTimerIdle(0)
        print idle
        self.failUnlessEqual(idle, 9, "Incorrect idle time")
        
        elapsed = self.core[1].GetTimerElapsed(0)
        print elapsed
        self.failUnlessEqual(elapsed, 60, "Incorrect elapsed time")
        
        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)
        
class HistoryBaseTest(WorkraveTestBase):

    def get_num_autostart_workraves(self):
        return 2

    def simulate(self, behaviour):
        num_wr = len(behaviour)
        timespan = len(behaviour[0])

        for w in range(num_wr) :
            self.debug[w].SetTickMode(1000)
            self.debug[w].Tick(1)

        for t in range(timespan) :
            for w in range(num_wr) :

                if behaviour[w][t] == '-' :
                    self.debug[w].SetFakeState(2)
                else:
                    self.debug[w].SetFakeState(4)

                self.debug[w].Tick(1)
                
            
    def runTest(self):

        # Start listenin for incoming connections.
        self.network[0].Listen(2701)
        self.network[1].Listen(2702)

        self.config[0].SetBool("timers/micro_pause/enabled", True)
        self.config[0].SetInt("timers/micro_pause/auto_reset", 30)
        self.config[0].SetInt("timers/micro_pause/limit", 180)
        self.config[0].SetInt("timers/micro_pause/snooze", 180)
        self.config[0].SetBool("timers/rest_break/enabled", False)
        self.config[0].SetBool("timers/daily_limit/enabled", False)

        self.config[1].SetBool("timers/micro_pause/enabled", True)
        self.config[1].SetInt("timers/micro_pause/auto_reset", 30)
        self.config[1].SetInt("timers/micro_pause/limit", 180)
        self.config[1].SetInt("timers/micro_pause/snooze", 180)
        self.config[1].SetBool("timers/rest_break/enabled", False)
        self.config[1].SetBool("timers/daily_limit/enabled", False)

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++++++++++++------------------------------++++++++++----------+++++-----'
        b = '------------------------------++++++++++++++++++++++++++++++----------++++++++++--------------------'

        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        idle = self.core[0].GetTimerIdle(0)
        print idle
        self.failUnlessEqual(idle, 9, "Incorrect idle time")
        
        elapsed = self.core[0].GetTimerElapsed(0)
        print elapsed
        self.failUnlessEqual(elapsed, 60, "Incorrect elapsed time")

        idle = self.core[1].GetTimerIdle(0)
        print idle
        self.failUnlessEqual(idle, 9, "Incorrect idle time")
        
        elapsed = self.core[1].GetTimerElapsed(0)
        print elapsed
        self.failUnlessEqual(elapsed, 60, "Incorrect elapsed time")
        
        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryBaseTest(WorkraveTestBase):

    def get_num_autostart_workraves(self):
        return 2

    def simulate(self, behaviour):
        num_wr = len(behaviour)
        timespan = len(behaviour[0])

        for w in range(num_wr) :
            self.debug[w].SetTickMode(1000)
            self.debug[w].Tick(1)

        for t in range(timespan) :
            for w in range(num_wr) :

                if behaviour[w][t] == '-' :
                    self.debug[w].SetFakeState(2)
                else:
                    self.debug[w].SetFakeState(4)

                self.debug[w].Tick(1)
                
            
    def runTest(self):

        # Start listenin for incoming connections.
        self.network[0].Listen(2701)
        self.network[1].Listen(2702)

        self.config[0].SetBool("timers/micro_pause/enabled", True)
        self.config[0].SetInt("timers/micro_pause/auto_reset", 30)
        self.config[0].SetInt("timers/micro_pause/limit", 180)
        self.config[0].SetInt("timers/micro_pause/snooze", 180)
        self.config[0].SetBool("timers/rest_break/enabled", False)
        self.config[0].SetBool("timers/daily_limit/enabled", False)

        self.config[1].SetBool("timers/micro_pause/enabled", True)
        self.config[1].SetInt("timers/micro_pause/auto_reset", 30)
        self.config[1].SetInt("timers/micro_pause/limit", 180)
        self.config[1].SetInt("timers/micro_pause/snooze", 180)
        self.config[1].SetBool("timers/rest_break/enabled", False)
        self.config[1].SetBool("timers/daily_limit/enabled", False)

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++++++++++++------------------------------++++++++++----------+++++-----'
        b = '------------------------------++++++++++++++++++++++++++++++----------++++++++++--------------------'

        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        idle = self.core[0].GetTimerIdle(0)
        print idle
        self.failUnlessEqual(idle, 9, "Incorrect idle time")
        
        elapsed = self.core[0].GetTimerElapsed(0)
        print elapsed
        self.failUnlessEqual(elapsed, 60, "Incorrect elapsed time")

        idle = self.core[1].GetTimerIdle(0)
        print idle
        self.failUnlessEqual(idle, 9, "Incorrect idle time")
        
        elapsed = self.core[1].GetTimerElapsed(0)
        print elapsed
        self.failUnlessEqual(elapsed, 60, "Incorrect elapsed time")
        
        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)
        
if __name__ == '__main__':
    unittest.main()
