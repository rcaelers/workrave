// DistributedStateListener.hh
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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

#ifndef DISTRIBUTEDSTATEINTERFACE_HH
#define DISTRIBUTEDSTATEINTERFACE_HH

enum DistributedStateID
  {
    DISTR_STATE_TIMER_MP = 0x0010,
    DISTR_STATE_TIMER_RB = 0x0011,
    DISTR_STATE_TIMER_DL = 0x0012,
  };

class DistributedStateInterface
{
public:
  DistributedStateInterface() {}
  virtual ~DistributedStateInterface() {}
  virtual bool get_state(DistributedStateID id, unsigned char **buffer, int *size) = 0;
  virtual bool set_state(DistributedStateID id, unsigned char *buffer, int size) = 0;
};

#endif // DISTRIBUTEDSTATEINTERFACE_HH
