# Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

if (NOT TEST_BINARY_DIR)
  message(FATAL_ERROR "TEST_BINARY_DIR is required")
endif()

set(prefix "${TEST_BINARY_DIR}/prefix with spaces")
file(MAKE_DIRECTORY "${prefix}/bin" "${prefix}/lib")
file(WRITE "${prefix}/lib/libkernel32.a" "")

unset(ENV{MINGW_PREFIX})
unset(ENV{MSYSTEM_PREFIX})
unset(ENV{MINGW_ROOT})
unset(SYS_ROOT CACHE)
unset(SYS_ROOT)
set(CMAKE_CXX_COMPILER "${prefix}/bin/clang++.exe")
include("${CMAKE_CURRENT_LIST_DIR}/../msys2.cmake")

if (NOT SYS_ROOT STREQUAL prefix)
  message(FATAL_ERROR "Compiler inference produced '${SYS_ROOT}', expected '${prefix}'")
endif()
get_property(root_type CACHE SYS_ROOT PROPERTY TYPE)
if (NOT root_type STREQUAL "PATH")
  message(FATAL_ERROR "The MSYS2 root must be retained in the CMake cache")
endif()
list(FIND CMAKE_PREFIX_PATH "${prefix}" prefix_index)
if (prefix_index EQUAL -1)
  message(FATAL_ERROR "The inferred root must be used for dependency discovery")
endif()

# A later IDE invocation need not have MSYS2 environment variables or PATH.
unset(SYS_ROOT)
set(CMAKE_CXX_COMPILER "${TEST_BINARY_DIR}/unrelated/bin/clang++.exe")
include("${CMAKE_CURRENT_LIST_DIR}/../msys2.cmake")
if (NOT SYS_ROOT STREQUAL prefix)
  message(FATAL_ERROR "Reconfiguration did not retain the MSYS2 root")
endif()

# Native Windows environment paths must remain safe in generated CMake code.
unset(SYS_ROOT CACHE)
unset(SYS_ROOT)
cmake_path(NATIVE_PATH prefix native_prefix)
set(ENV{MINGW_ROOT} "${native_prefix}")
include("${CMAKE_CURRENT_LIST_DIR}/../msys2.cmake")
if (NOT SYS_ROOT STREQUAL prefix)
  message(FATAL_ERROR "The native environment root was not normalized: '${SYS_ROOT}'")
endif()
