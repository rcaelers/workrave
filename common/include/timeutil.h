/*
 * timeutil.h --- Utilities
 *
 * Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * $Id$
 *
 */

#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef WIN32
#include "gettimeofday.h"
#endif

/*****************************************************************************/
/*  Time functions                                                           */

#define MICRO_PER_SEC 1000000

#define tvRESETTIME(tv) \
  { (tv).tv_sec = 0; (tv).tv_usec = 0; }

#define tvSETTIME(tv, sec, usec) \
  { (tv).tv_sec = (sec); (tv).tv_usec = (usec); }

#define tvADDTIME(ret, a, b) \
  { (ret).tv_sec = (a).tv_sec + (b).tv_sec; \
  if (((ret).tv_usec = (a).tv_usec+(b).tv_usec) >= MICRO_PER_SEC) \
        { \
    (ret).tv_sec++; \
    (ret).tv_usec -= MICRO_PER_SEC; \
  } }

#define tvSUBTIME(ret, a, b) \
  { (ret).tv_sec = (a).tv_sec - (b).tv_sec; \
  if (((ret).tv_usec = (a).tv_usec - (b).tv_usec) < 0) \
        { \
          (ret).tv_sec--; \
          (ret).tv_usec += MICRO_PER_SEC; \
  } }

#define tvTIMEGEQ(a, b) ((a).tv_sec > (b).tv_sec || ((a).tv_sec == (b).tv_sec && (a).tv_usec >= (b).tv_usec))
#define tvTIMEGT(a, b) ((a).tv_sec > (b).tv_sec || ((a).tv_sec == (b).tv_sec && (a).tv_usec > (b).tv_usec))
#define tvTIMELT(a, b) ((a).tv_sec < (b).tv_sec || ((a).tv_sec == (b).tv_sec && (a).tv_usec < (b).tv_usec))
#define tvTIMELEQ0(a) ((a).tv_sec < 0 || ((a).tv_sec == 0 && (a).tv_usec <= 0))
#define tvTIMELT0(a)  ((a).tv_sec < 0 || ((a).tv_sec == 0 && (a).tv_usec < 0))
#define tvTIMEEQ0(a) (((a).tv_sec == 0 && (a).tv_usec == 0))

#endif /* TIMEUTIL_H */
