// TimeBarInterface.hh --- Time Bar
//
// Copyright (C) 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
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

#ifndef TIMEBARINTERFACE_HH
#define TIMEBARINTERFACE_HH

#include <string>


class TimeBarInterface
{
public:
  enum ColorId
    {
      COLOR_ID_ACTIVE = 0,
      COLOR_ID_INACTIVE,
      COLOR_ID_OVERDUE,
      COLOR_ID_1_ACTIVE_DURING_BREAK,
      COLOR_ID_2_ACTIVE_DURING_BREAK,
      COLOR_ID_INACTIVE_OVER_ACTIVE,
      COLOR_ID_INACTIVE_OVER_OVERDUE,
      COLOR_ID_BG,
      COLOR_ID_SIZEOF
    };

  virtual ~TimeBarInterface() {}
  
  virtual void set_progress(int value, int max_value) = 0;
  virtual void set_secondary_progress(int value, int max_value) = 0;
  
  virtual void set_text(std::string text) = 0;

  virtual void update() = 0;
  virtual void set_bar_color(ColorId color) = 0;
  virtual void set_secondary_bar_color(ColorId color) = 0;
};


#endif // TIMEBARINTERFACE_HH
