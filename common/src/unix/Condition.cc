// Condition.cc --- Condition synchronisation
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2007-08-30 16:10:56 robc>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define NEED_FTIME

#include <stdio.h>
#include <sys/time.h>
#include <errno.h>

#include "Condition.hh"
#include "timeutil.h"


//! Constructs the condition.
Condition::Condition()
{
  pthread_cond_init(&m_cond, NULL);
  m_signaled = false;
  m_count = 0;
}


//! Destructs the condition.
Condition::~Condition()
{
  pthread_cond_destroy(&m_cond);
}


//! Signals the condition.
void
Condition::signal(void)
{
  lock();
  m_signaled = true;
  m_count++;
  pthread_cond_broadcast(&m_cond);
  unlock();
}


//! Waits at most the specified amount of time until the condition is signaled.
bool
Condition::wait(long timer)
{
  int ret = 0;

  lock();
  long count = m_count;
  while (!m_signaled && m_count == count)
    {
      if (timer != 0)
        {
          struct timeval current;
          struct timespec spec;

          gettimeofday(&current, NULL);

          spec.tv_sec = current.tv_sec + timer / 1000;
          spec.tv_nsec = (current.tv_usec + (timer % 1000) * 1000) * 1000;
          ret = pthread_cond_timedwait(&m_cond, &m_mutex, &spec);
        }
      else
        {
          pthread_cond_wait(&m_cond, &m_mutex);
        }

      if (ret == ETIMEDOUT)
        break;
    }

  m_signaled = false;
  unlock();
  if (ret == ETIMEDOUT)
    return false;

  return true;
}


//! Waits at most the specified amount of time until the condition is signaled.
bool
Condition::wait(struct timeval tv)
{
  int ret = 0;

  lock();
  long count = m_count;
  while (!m_signaled && m_count == count)
    {
      struct timeval current;
      struct timeval w;
      struct timespec spec;

      gettimeofday(&current, NULL);

      tvADDTIME(w, current, tv);
      spec.tv_sec = w.tv_sec;
      spec.tv_nsec = w.tv_usec * 1000;
      ret = pthread_cond_timedwait(&m_cond, &m_mutex, &spec);

      if (ret == ETIMEDOUT)
        break;
    }

  m_signaled = false;
  unlock();
  if (ret == ETIMEDOUT)
    return false;

  return true;
}


//! Waits at most until the specified time until the condition is signaled.
bool
Condition::wait_until(struct timeval tv)
{
  int ret = 0;

  lock();
  long count = m_count;
  while (!m_signaled && m_count == count)
    {
      struct timespec spec;

      spec.tv_sec = tv.tv_sec;
      spec.tv_nsec = tv.tv_usec * 1000;
      ret = pthread_cond_timedwait(&m_cond, &m_mutex, &spec);

      if (ret == ETIMEDOUT)
        break;
    }

  m_signaled = false;
  unlock();
  if (ret == ETIMEDOUT)
    return false;

  return true;
}
