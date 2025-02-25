// Copyright (C) 2017 Tom Parker
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef WORKRAVE_UTILS_MACOS_HELPERS_HH
#define WORKRAVE_UTILS_MACOS_HELPERS_HH

#include <pthread.h>

#if !defined(_MACH_PORT_T)
#  define _MACH_PORT_T
#  include <sys/_types.h> /* __darwin_mach_port_t */
typedef __darwin_mach_port_t mach_port_t;
mach_port_t pthread_mach_thread_np(pthread_t);
#endif /* _MACH_PORT_T */

#include <mach-o/dyld.h>
#include <sys/param.h>

#endif
