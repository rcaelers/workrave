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

    def history_to_string(self, hist, totlen, start):
        last_ts = start
        last_a  = 0
        ret = ""
        for (ts, a) in hist:
            if ts >= last_ts:
                if last_a & ( 1 << 1 ) == 0:
                    ret = ret + ( "-" * (ts - last_ts) )
                else:
                    ret = ret + ( "+" * (ts - last_ts) )

                last_ts = ts
                last_a = a

        if last_a & ( 1 << 1 ) == 0:
            ret = ret + ( "-" * (totlen - len(ret)) )
        else:
            ret = ret + ( "+" * (totlen - len(ret)) )

        return ret
        
    def check_history(self, expected, expected_idle, expected_active, expected_overdue):
        for w in range(self.get_num_autostart_workraves()) :
            hist = self.debug[w].GetActivityHistory()
            
            ret = self.history_to_string(hist, len(expected), 1001)
            self.failUnlessEqual(expected, ret, "Incorrect merged activity " + str(w) +
                                 "\nExpected:\n|" + expected + "|\ngot\n|" + ret + "|")

            idle = self.core[w].GetTimerIdle('microbreak')
            self.failUnlessEqual(idle, expected_idle, "Incorrect idle time. "+ str(w) +
                                 "\nExpected " + str(expected_idle) + " got " + str(idle))
        
            elapsed = self.core[w].GetTimerElapsed('microbreak')
            self.failUnlessEqual(elapsed, expected_active, "Incorrect elapsed time."+ str(w) +
                                 "\nExpected " + str(expected_active) + " got " + str(elapsed))

            overdue = self.core[w].GetTimerOverdue('microbreak')
            self.failUnlessEqual(overdue, expected_overdue, "Incorrect overdue time."+ str(w) +
                                 "\nExpected " + str(expected_overdue) + " got " + str(overdue))

    def check_settings(self, behaviour, expected):

        num_wr = len(behaviour)
        sim_time = 1001
        times = [ 1000, 1000 ]
        expected = [('reset', -1), ('limit', -1)] + expected

        for t in range(len(behaviour[0])) :
            for w in range(num_wr) :
                if behaviour[w][t] == 's' :
                    times.append(sim_time)
                    break
            sim_time = sim_time + 1


        for w in range(self.get_num_autostart_workraves()) :
            hist = self.debug[w].GetSettingsHistory()

            count = 0
            for (ts, setting, break_id, value) in hist:

                if break_id == 'microbreak':
                    self.failUnlessEqual(ts, times[count], "Incorrect time." + str(w) +
                                         "\nExpected " + str(times[count]) + " got " + str(ts) + " at " +str(count))
                    self.failUnlessEqual(setting, expected[count][0], "Incorrect setting" + str(w) +
                                         "\nExpected " + expected[count][0] + " got " + str(setting) + " at " +str(count))
                    if expected[count][1] != -1:
                        self.failUnlessEqual(value, expected[count][1], "Incorrect value." + str(w) +
                                             "\nExpected " + str(expected[count][1]) + " got " + str(value) + " at " +str(count))
                    count = count + 1

    def simulate(self, behaviour, settings = None):
        num_wr = len(behaviour)
        timespan = len(behaviour[0])

        active = [ False ] * num_wr;

        self.current_time = 1000
        
        for w in range(num_wr) :
            self.debug[w].InitTime(self.current_time)
            self.debug[w].Tick(1)
        
        for t in range(timespan) :

            self.current_time = self.current_time + 1

            for w in range(num_wr) :

                self.debug[w].SetTime(self.current_time)

                if behaviour[w][t] == '-' :
                    self.debug[w].SetFakeState(2)
                    active[w] = False
                elif behaviour[w][t] == '+' :
                    self.debug[w].SetFakeState(4)
                    active[w] = True
                elif behaviour[w][t] == 's' :
                    (key, value) = settings[w].pop(0)
                    if key == 'reset':
                        self.config[w].SetInt("timers/micro_pause/auto_reset", value)
                    elif key == 'limit':
                        self.config[w].SetInt("timers/micro_pause/limit", value)

            if self.connected:
                time.sleep(0.5)
            else:
                time.sleep(0.5)
                    
            for w in range(num_wr) :
                self.debug[w].Tick(1)

                if self.connected:
                    time.sleep(0.25)

            combined = self.connected and (True in active)

                
            for w in range(num_wr) :
                running = self.core[w].IsTimerRunning('microbreak')
                elapsed = self.core[w].GetTimerElapsed('microbreak')
                limit = self.config[w].GetInt("timers/micro_pause/limit")
                if elapsed == limit[0] and not running and active[w]:
                    running = True
                
                self.failUnlessEqual(running, active[w] or combined,
                                     "Timer Active NOK " + str(w) + " " + str(self.core[w].IsTimerRunning('microbreak')) + " " + str(running) + " " + str(active[w]) + " " + str(combined) + " " + str(self.current_time))
                
        if not self.connected:
            for w in range(num_wr) :
                hist = self.debug[w].GetActivityHistory()
                ret = self.history_to_string(hist, len(behaviour[w]), 1001)

                prev = '-'
                ref = ''
                for t in range(timespan) :
                    if behaviour[w][t] != 's':
                        prev = behaviour[w][t]
                    ref = ref + prev
                
                self.failUnlessEqual(ref, ret, "History NOK\n"
                                     + ref + "\n" +
                                     ret + "\nFrom " +  str(w))
            

class HistoryTest1(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++++++++++++------------------------------++++++++++----------+++++-----'
        b = '------------------------------++++++++++++++++++++++++++++++----------++++++++++--------------------'
        x = '----------++++++++++++++++++++++++++++++++++++++++++++++++++----------++++++++++----------+++++-----'

        # Simulate activity
        self.simulate( [a, b] )

        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 4, 65, 5)
        
        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)


class HistoryTest2(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----------++++++++++--------------------+++++++++++++------------------+++++++++++++-----------------'
        x = '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 100, 40)
        
        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryTest3(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '++++++++++----------++++++++++++++++++++-------------++++++++++++++++++-------------+++++++++++++++-'
        b = '----------++++++++++--------------------+++++++++++++------------------+++++++++++++----------------'
        x = '+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 99, 39)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)


class HistoryTest4(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++++++++++++------------------------------------------------------------'
        b = '--------------------++++++++++----------------------------------------------------------------------'
        x = '----------++++++++++++++++++++++++++++++------------------------------------------------------------'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 20, 0, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)


class HistoryTest5(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++----------'
        b = '-------------+++++++++++++++++++++------'
        x = '----------++++++++++++++++++++----------'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 9, 20, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryTest6(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++----------'
        b = '-------------++++++++++++++++-----------'
        x = '----------+++++++++++++++++++-----------'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 10, 19, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryTest7(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++----------'
        b = '-------------+++++++++++++++------------'
        x = '----------++++++++++++++++++++----------'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 9, 20, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

        
class HistoryTest8(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++----------'
        b = '----------++++++++++++++++++++++++------'
        x = '----------++++++++++++++++++++----------'

        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 9, 20, 0)
        
        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryTest9(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++----------'
        b = '----------+++++++++++++++++++++++++-----'
        x = '----------+++++++++++++++++++++++++-----'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)
        
        self.check_history(x, 4, 25, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryTest10(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------++++++++++++++++++++----------'
        b = '-------------+++++++++++++++++----------'
        x = '----------++++++++++++++++++++----------'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 9, 20, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryTest11(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------+++++++++++-----------------------------++++++++++++++++++++--------------------+++++++++-'
        b = '------------------------------+++++++++++--------------------+++++++++----------+++++++++++++++++++-'
        x = '----------+++++++++++---------+++++++++++---------++++++++++++++++++++----------+++++++++++++++++++-'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 61, 1)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)
        
class HistoryTest12(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------+++++++++++-----------------------------+++++++++++++++++-----------------------+++++++++-'
        b = '------------------------------+++++++++++--------------------++++++---------------------------+++++-'
        x = '----------+++++++++++---------+++++++++++---------+++++++++++++++++-----------------------+++++++++-'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0 , 9, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistoryTest13(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 0

    def runTest(self):
        
        self.launch(2, True)
        
        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------+++++++++++-----------------------------+++++++++++++++++-----------------------+++++++++-'
        b = '------------------------------+++++++++++--------------------++++++---------------------------+++++-'
        x = '----------+++++++++++---------+++++++++++---------+++++++++++++++++-----------------------+++++++++-'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0 , 9, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

        # Relaunch
        self.launch(2, False)

        # Check result
        self.check_history(x, 0 , 9,0 )

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

        self.kill()

class HistoryTest14(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 0

    def runTest(self):
        
        self.launch(2, True)
        
        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------+++++++++++-----------------------------+++++++++++++++++-----------------------+++++++++-'
        b = '------------------------------+++++++++++--------------------++++++---------------------------+++++-'
        x = '----------+++++++++++---------+++++++++++---------+++++++++++++++++-----------------------+++++++++-'
        y = '----------------------------------------------------------------------------------------------------'

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0 , 9, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

        # Relaunch
        self.launch(2, False)

        time.sleep(4)

        self.debug[0].SetTime(25*60*60 - 10)
        self.debug[1].SetTime(25*60*60 - 10)

        self.debug[0].Tick(100)
        self.debug[1].Tick(100)

        # Check result
        self.check_history(y, 0 , 0, 0)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

        self.kill()

class HistoryTest15(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 1
    
    def runTest(self):
        
        
        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----------+++++++++++-----------------------------+++++++++++++++++-----------------------++++++--+-'
        x = '----------+++++++++++-----------------------------+++++++++++++++++-----------------------++++++--+-'
        y = '++++++--+--------------------------------'

        # Simulate activity
        self.current_time = 1000
        self.debug[0].SetTime(self.current_time)
        self.debug[0].SetFakeState(2)
        self.debug[0].Tick(1)

        self.current_time = 3600 - 90
        self.debug[0].SetTime(self.current_time)
        self.debug[0].Tick(1)
        for t in range(len(a)) :
            if a[t] == '-' :
                self.debug[0].SetFakeState(2)
            else:
                self.debug[0].SetFakeState(4)

            self.current_time = self.current_time + 1
            self.debug[0].SetTime(self.current_time)
            self.debug[0].Tick(1)
                
        # Check result
        hist = self.debug[0].GetActivityHistory()
        self.failUnlessEqual(hist[0][0], 1000, "Incorrect start of history " + str(hist[0][0]))
        ret = self.history_to_string(hist, len(x), 3600 - 90 + 1)
        self.failUnlessEqual(x, ret, "Incorrect activity " +
                             "\nExpected:\n|" + x + "|\ngot\n|" + ret + "|")


        self.debug[0].SetTime(25*60*60 - 9)
        self.debug[0].Tick(10)

        # Check result
        hist = self.debug[0].GetActivityHistory()
        self.failUnlessEqual(hist[0][0], 3601, "Incorrect start of history " + str(hist[0][0]))
        ret = self.history_to_string(hist, len(y), 3600 + 1)
        self.failUnlessEqual(y, ret, "Incorrect activity " +
                             "\nExpected:\n|" + y + "|\ngot\n|" + ret + "|")


        # Quit.
        self.debug[0].Quit()

        time.sleep(4)

        self.kill()

class HistoryTest16(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '++++++++++----------+++++++++++-----------++++++++++++++++++++++++++++++++++++++++----------++++++++-'
        b = '---------++++++++++++---------------------+++++++++++------------------++++++++++--------------------'
        x = '+++++++++++++++++++++++++++++++-----------++++++++++++++++++++++++++++++++++++++++----------++++++++-'

        for i in range(self.get_num_autostart_workraves()):
            self.config[i].SetInt("timers/micro_pause/limit", 20)
            self.config[i].SetInt("timers/micro_pause/auto_reset", 10)

        # Simulate activity
        self.simulate( [a, b] )
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 8, 31)
        
        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest1(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----+++++s++++++++++++++++s+++++++------------------------------++++++++++----------+++++-----'
        b = '----s-------------s-----------++++s+++++s+++++++++++++++++++----------++++++++++--------------------'
        x = '----------++++++++++++++++++++++++++++++++++++++++++++++++++----------++++++++++----------+++++-----'

        As = [ ('reset', 23), ('reset', 15), ('reset', 30) ] 
        bs = [ ('reset', 22), ('reset', 15), ('reset', 20), ('reset', 30) ] 

        xs = [ ('reset', 22), ('reset', 15), ('reset', 15), ('reset', 30), ('reset', 20), ('reset', 30) ] 

        # Simulate activity
        self.simulate( [a, b], [As, bs])
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 4, 65, 5)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest2(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++---------++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----s-----------------------------------------------------------------------------------------------'
        x = '----------++++++++++++++++++++---------++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10)                ]
        bs = [ ('reset', 10)  ] 

        xs = [ ('reset', 10) ] 

        # Simulate activity
        self.simulate( [a, b], [As, bs])
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 80, 20)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)


class HistorySettingsTest3(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----s-----------------------------------------------------------------------------------------------'
        x = '----------++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10)                ]
        bs = [ ('reset', 10)  ] 

        xs = [ ('reset', 10) ] 

        # Simulate activity
        self.simulate( [a, b], [As, bs])
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 59, 0)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest4(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----s----------------------------s------------------------------------------------------------------'
        x = '----------++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10)                ]
        bs = [ ('reset', 10), ('reset', 14)  ] 

        xs = [ ('reset', 10), ('reset', 14) ] 

        # Simulate activity
        self.simulate( [a, b], [As, bs])
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 79, 19)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest4A(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++---s------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----s-----------------------------------------------------------------------------------------------'
        x = '----------++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10), ('reset', 14) ]
        bs = [ ('reset', 10)  ] 

        xs = [ ('reset', 10), ('reset', 14) ] 

        for i in range(self.get_num_autostart_workraves()):
            self.config[i].SetInt("timers/micro_pause/limit", 200)
 
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)
        self.connected = True

        # Simulate activity
        self.simulate( [a, b], [As, bs])
        
        # Check result
        self.check_history(x, 0, 79, 0)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest5(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++-s---s----+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----s----------------------------s---s--------------------------------------------------------------'
        x = '----------++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10), ('reset', 5),  ('reset', 8) ]
        bs = [ ('reset', 10), ('reset', 11), ('reset', 14) ]
  
        xs = [ ('reset', 10), ('reset', 5), ('reset', 11), ('reset', 8), ('reset', 14) ]

        # Simulate activity
        self.simulate( [a, b], [As, bs])
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 79, 19)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest5A(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++-s-s-s-s--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----------------------------------------------------------------------------------------------------'
        x = '----------++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10), ('reset', 5), ('reset', 11), ('reset', 7), ('reset', 14) ]
        bs = [ ('reset', 10) ]
  
        xs = [ ('reset', 10), ('reset', 5), ('reset', 11), ('reset', 7), ('reset', 14) ]

        for i in range(self.get_num_autostart_workraves()):
            self.config[i].SetInt("timers/micro_pause/limit", 200)

        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)
        self.connected = True

        # Simulate activity
        self.simulate( [a, b], [As, bs])

        # Check result
        self.check_history(x, 0, 79, 0)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest6(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++-s---s----+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----s----------------------------s---s--------------------------------------------------------------'
        x = '----------++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10), ('reset', 8), ('reset', 7) ]
        bs = [ ('reset', 10), ('reset', 4), ('reset', 14) ]
  
        xs = [ ('reset', 10), ('reset', 8), ('reset', 4), ('reset', 7), ('reset', 14) ]

        # Simulate activity
        self.simulate( [a, b], [As, bs])
        
        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)

        # Check result
        self.check_history(x, 0, 59, 0)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

class HistorySettingsTest6A(HistoryBaseTest):

    def get_num_autostart_workraves(self):
        return 2

    def runTest(self):

        #    0         1         2         3         4         5         6         7         8         9         A
        #    0.........0.........0.........0.........0.........0.........0.........0.........0.........0.........0
        a = '----s-----++++++++++++++++++++-s-s-s-s--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'
        b = '----------------------------------------------------------------------------------------------------'
        x = '----------++++++++++++++++++++----------+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-'

        As = [ ('reset', 10), ('reset', 8), ('reset', 4), ('reset', 7), ('reset', 14) ]
        bs = []
        xs = [ ('reset', 10), ('reset', 8), ('reset', 4), ('reset', 7), ('reset', 14) ]

        for i in range(self.get_num_autostart_workraves()):
            self.config[i].SetInt("timers/micro_pause/limit", 200)

        # Join workraves
        link1 = self.network[0].Connect("localhost", 2702)
        time.sleep(8)
        self.connected = True

        # Simulate activity
        self.simulate( [a, b], [As, bs])

        # Check result
        self.check_history(x, 0, 59, 0)
        self.check_settings([a, b], xs)

        # Quit.
        self.debug[0].Quit()
        self.debug[1].Quit()

        time.sleep(4)

if __name__ == '__main__':
    unittest.main()
