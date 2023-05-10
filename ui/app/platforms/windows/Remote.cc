// Copyright (C) 2023 Rob Caelers <robc@krandor.nl>
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

#include "Remote.hh"

#include <psapi.h>

#include <array>
#include <filesystem>
#include <string>
#include <boost/algorithm/string.hpp>

#include "utils/Paths.hh"
#include "commonui/MenuDefs.hh"

using namespace workrave::utils;

std::string
Remote::get_title(HWND hwnd)
{
  std::array<char, 256> title{};
  auto n = GetWindowTextA(hwnd, title.data(), title.size());
  if (n == 0)
    {
      throw std::runtime_error("failed to retrieve window title");
    }
  return title.data();
}

std::string
Remote::get_class_name(HWND hwnd)
{
  std::array<char, 256> class_name{};
  auto n = GetClassNameA(hwnd, class_name.data(), class_name.size());
  if (n == 0)
    {
      throw std::runtime_error("failed to retrieve window class name");
    }
  return class_name.data();
}

std::string
Remote::get_process_name(HWND hwnd)
{
  DWORD process_id = 0;
  auto r = GetWindowThreadProcessId(hwnd, &process_id);
  if (r == 0)
    {
      throw std::runtime_error("failed to retrieve process id");
    }

  HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
  if (process_handle == nullptr)
    {
      throw std::runtime_error("failed to open process");
    }

  std::array<char, MAX_PATH> process_name{};
  DWORD size = process_name.size();
  r = QueryFullProcessImageNameA(process_handle, 0, process_name.data(), &size);
  if (r == 0)
    {
      r = GetModuleFileNameExA(process_handle, nullptr, process_name.data(), process_name.size());
      if (r == 0)
        {
          CloseHandle(process_handle);
          throw std::runtime_error("failed to retrieve process name");
        }
    }

  CloseHandle(process_handle);
  return process_name.data();
}

BOOL CALLBACK
Remote::enum_windows_cb(HWND hwnd, LPARAM lParam)
{
  auto *self = reinterpret_cast<Remote *>(lParam);

  try
    {
      auto class_name = self->get_class_name(hwnd);
      auto title = self->get_title(hwnd);

      if (class_name == "gdkWindowToplevel" && title == "Workrave")
        {
          auto process_name = self->get_process_name(hwnd);
          auto base = process_name.substr(process_name.find_last_of("/\\") + 1);
          boost::algorithm::to_lower(base);

          if (base == "workrave.exe")
            {
              self->hwnd = hwnd;
              return FALSE;
            }
        }
      return TRUE;
    }
  catch (std::exception &e)
    {
      return TRUE;
    }
}

Remote::Remote()
{
  EnumWindows(enum_windows_cb, reinterpret_cast<LPARAM>(this));
}

void
Remote::open()
{
  if (hwnd != nullptr)
    {
      PostMessage(hwnd, WM_USER, MENU_COMMAND_OPEN, 0);
    }
}
