// Condition.hh --- Condition synchronisation
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-08-11 16:36:45 robc>
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

#ifndef CONDITION_HH
#define CONDITION_HH

#include "Mutex.hh"

class Condition : public Mutex
{
private:
  pthread_cond_t m_cond;
  bool m_signaled;
  int m_count;

public:
  Condition();
  ~Condition();

  void signal();
  bool wait(long timer = 0);
  bool wait(struct timeval tv);
  bool wait_until(struct timeval tv);
};


#endif // CONDITION_HH
