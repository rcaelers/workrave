// GUIFactoryInterface.hh
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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
//

#ifndef GUIFACTORYINTERFACE_HH
#define GUIFACTORYINTERFACE_HH

class PreludeWindowInterface;
class BreakWindowInterface;
class SoundPlayerInterface;

#include "GUIControl.hh"
#include <string>

class GUIFactoryInterface
{
public:
  virtual PreludeWindowInterface *create_prelude_window() = 0;
  virtual BreakWindowInterface *create_break_window(GUIControl::BreakId break_id, bool ignorable) = 0;
  virtual SoundPlayerInterface *create_sound_player() = 0;
};

#endif // RESTGUIFACTORYINTERFACE_HH
