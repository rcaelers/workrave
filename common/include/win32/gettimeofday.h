/*
 * gettimeofday.h
 *
 * Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
 * All rights reserved.
 *
 * Time-stamp: <2007-08-30 16:10:56 robc>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id$
 *
 */

#ifndef GETTIMEOFDAY_H
#define GETTIMEOFDAY_H

#ifdef WIN32
#include <windows.h>
#endif

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

#ifndef HAVE_STRUCT_TIMESPEC
struct timespec
{
  long tv_sec;
  long tv_nsec;
};
#define HAVE_STRUCT_TIMESPEC
#endif /* HAVE_STRUCT_TIMESPEC */

#ifndef HAVE_STRUCT_TIMEZONE
struct timezone {
               int  tz_minuteswest; /* minutes W of Greenwich */
               int  tz_dsttime;     /* type of dst correction */
       };
#define HAVE_STRUCT_TIMEZONE
#endif

#ifndef HAVE_GETTIMEOFDAY

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

int gettimeofday(struct timeval *tv, struct timezone *tz);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* HAVE_GETTIMEOFDAY */


#endif /* GETTIMEOFDAY_H */
