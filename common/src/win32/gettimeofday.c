/*
 * gettimeofday.c
 *
 * Copyright (C) 2002, 2005 Rob Caelers <robc@krandor.nl>
 * All rights reserved.
 *
 * Probably (...) copied from Pthreads-win32 (POSIX Threads Library for Win32)
 * Copyright(C) 1998 John E. Bossom
 * Copyright(C) 1999,2005 Pthreads-win32 contributors
 *
 * Time-stamp: <2005-10-26 20:26:00 robc>
 *
 */

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettimeofday.h"

#ifndef HAVE_GETTIMEOFDAY
#ifdef WIN32

/*
 * time between jan 1, 1601 and jan 1, 1970 in units of 100 nanoseconds
 */
#define TIMESPEC_TO_FILETIME_OFFSET \
    ( ((LONGLONG) 27111902 << 32) + (LONGLONG) 3577643008 )


inline void
timespec_to_filetime(const struct timespec *ts, FILETIME *ft)
     /*
      * -------------------------------------------------------------------
      * converts struct timespec
      * where the time is expressed in seconds and nanoseconds from Jan 1, 1970.
      * into FILETIME (as set by GetSystemTimeAsFileTime), where the time is
      * expressed in 100 nanoseconds from Jan 1, 1601,
      * -------------------------------------------------------------------
      */
{
  *(LONGLONG *)ft = ts->tv_sec * 10000000
          + (ts->tv_nsec + 50) / 100
          + TIMESPEC_TO_FILETIME_OFFSET;
}

inline void
filetime_to_timespec(const FILETIME *ft, struct timespec *ts)
     /*
      * -------------------------------------------------------------------
      * converts FILETIME (as set by GetSystemTimeAsFileTime), where the time is
      * expressed in 100 nanoseconds from Jan 1, 1601,
      * into struct timespec
      * where the time is expressed in seconds and nanoseconds from Jan 1, 1970.
      * -------------------------------------------------------------------
      */
{
  ts->tv_sec = (int)((*(LONGLONG *)ft - TIMESPEC_TO_FILETIME_OFFSET) / 10000000);
  ts->tv_nsec = (int)((*(LONGLONG *)ft - TIMESPEC_TO_FILETIME_OFFSET - ((LONGLONG)ts->tv_sec * (LONGLONG)10000000)) * 100);
}


//FIXME: incomplete. verify spec
int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
  int ret = 1;

  (void) tz;

  if (tv != NULL)
    {
      struct timespec spec;
      FILETIME ft;
      SYSTEMTIME st;

      GetSystemTime(&st);
      SystemTimeToFileTime(&st, &ft);
      filetime_to_timespec(&ft, &spec);

      tv->tv_sec = spec.tv_sec;
      tv->tv_usec = spec.tv_nsec / 1000;

      ret = 0;
    }
  return ret;
}

#endif
#endif
