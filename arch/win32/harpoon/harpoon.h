/*
 * harpoon.h 
 *
 * Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
 * All rights reserved.
 *
 * Time-stamp: <2003-04-07 23:53:42 pennersr>
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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HARPOON_EXPORTS
#define HARPOON_API __declspec(dllexport)
#else
#define HARPOON_API __declspec(dllimport)
#endif


HARPOON_API BOOL harpoon_init(void);
HARPOON_API void harpoon_exit(void);

HARPOON_API void harpoon_unhook_mouse();
HARPOON_API void harpoon_hook_mouse(HOOKPROC hf);

HARPOON_API void harpoon_unhook_keyboard();
HARPOON_API void harpoon_hook_keyboard(HOOKPROC hf);

HARPOON_API void harpoon_block_input(BOOL block, HWND unblocked);

#ifdef __cplusplus
}
#endif

#endif /* HARPOON_H */
