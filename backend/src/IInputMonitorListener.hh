// IInputMonitorListener.hh
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006 Rob Caelers & Raymond Penners
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
// $Id: InputMonitorListenerInterface.hh 527 2005-09-23 19:22:33Z rcaelers $
//

#ifndef INPUTMONITORLISTENER_HH
#define INPUTMONITORLISTENER_HH

class IInputMonitorListener
{
public:
  virtual ~IInputMonitorListener() {}
  
  virtual void action_notify() = 0;
  virtual void mouse_notify(int x, int y, int wheel = 0) = 0;
  virtual void button_notify(int button_mask, bool is_press) = 0;
  virtual void keyboard_notify(int key_code, int modifier) = 0;
};

#endif // IINPUTMONITORLISTENER_HH
