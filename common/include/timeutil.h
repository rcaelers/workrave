/*
 * timeutil.h --- Utilities
 *
 * Copyright (C) 2001, 2002, 2003, 2007 Rob Caelers <robc@krandor.nl>
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <glib.h>

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
