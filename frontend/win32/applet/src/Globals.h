// Globals.h --- Globals
//
// Copyright (C) 2004 Raymond Penners <raymond@dotsphinx.com>
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

extern HINSTANCE  g_hInst;
extern UINT       g_DllRefCount;

#undef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
