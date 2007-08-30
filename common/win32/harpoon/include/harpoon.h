/*
 * harpoon.h
 *
 * Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
 * All rights reserved.
 *
 * Time-stamp: <2007-08-30 16:03:06 robc>
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
 */
#ifndef HARPOON_H
#define HARPOON_H

#include <windows.h>
#include <mbstring.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define HARPOON_MAX_UNBLOCKED_APPS 3
#define HARPOON_WINDOW_CLASS "HarpoonNotificationWindow"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef HARPOON_EXPORTS
#define HARPOON_API __declspec(dllexport)
#else
#define HARPOON_API __declspec(dllimport)
#endif

typedef enum
{
  HARPOON_NOTHING = -1,
  HARPOON_BUTTON_PRESS = 0,
  HARPOON_BUTTON_RELEASE,
  HARPOON_2BUTTON_PRESS,
  HARPOON_MOUSE_WHEEL,
  HARPOON_KEY_PRESS,
  HARPOON_KEY_RELEASE,
  HARPOON_MOUSE_MOVE,
  HARPOON_EVENT__SIZEOF
} HarpoonEventType;

typedef enum
{
  HARPOON_KEY_REPEAT_FLAG = (1 << 0)
} HarpoonKeyFlags;

typedef struct
{
  HarpoonEventType type;
  int x;
  int y;
  int button;
  int wheel;
} HarpoonEventMouse;

typedef struct
{
  HarpoonEventType type;
  int flags;
} HarpoonEventKeyboard;

typedef union HarpoonEventUnion
{
  HarpoonEventType type;
  HarpoonEventMouse mouse;
  HarpoonEventKeyboard keyboard;
} HarpoonEvent;




typedef void (* HarpoonHookFunc) (HarpoonEvent *event);


HARPOON_API BOOL harpoon_init(
    char imported_critical_filename_list[][ 511 ], BOOL debug );
HARPOON_API void harpoon_exit (void);

HARPOON_API void harpoon_unhook();
HARPOON_API BOOL harpoon_hook(
    HarpoonHookFunc func, BOOL keyboard_lowlevel, BOOL mouse_lowlevel );

HARPOON_API void harpoon_block_input(void);
HARPOON_API void harpoon_unblock_input(void);

HARPOON_API char *_mbstrncpy_lowercase( const char *, const char *, int );

#ifdef __cplusplus
}
#endif

#endif /* HARPOON_H */
