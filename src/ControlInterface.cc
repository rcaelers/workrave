// ControlInterface.cc --- The main controller interface
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ControlInterface.hh"

const string ControlInterface::CFG_KEY_TIMERS = "timers";
const string ControlInterface::CFG_KEY_TIMER = "timers/";
const string ControlInterface::CFG_KEY_TIMER_LIMIT = "/limit";
const string ControlInterface::CFG_KEY_TIMER_AUTO_RESET = "/auto_reset";
const string ControlInterface::CFG_KEY_TIMER_RESET_PRED = "/reset_pred";
const string ControlInterface::CFG_KEY_TIMER_SNOOZE = "/snooze";
const string ControlInterface::CFG_KEY_TIMER_RESTORE = "/restore";
const string ControlInterface::CFG_KEY_TIMER_COUNT_ACTIVITY = "/count_activity";
const string ControlInterface::CFG_KEY_TIMER_MONITOR = "/monitor";
const string ControlInterface::CFG_KEY_MONITOR = "monitor";
const string ControlInterface::CFG_KEY_MONITOR_NOISE = "monitor/noise";
const string ControlInterface::CFG_KEY_MONITOR_ACTIVITY = "monitor/activity";
const string ControlInterface::CFG_KEY_MONITOR_IDLE = "monitor/idle";

