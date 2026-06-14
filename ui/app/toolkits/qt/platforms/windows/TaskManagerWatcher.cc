// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "TaskManagerWatcher.hh"

#include <algorithm>
#include <cctype>
#include <string_view>

#include <windows.h>
#include <tlhelp32.h>

namespace
{
  auto is_task_manager_process_name(const wchar_t *process_name) -> bool
  {
    std::wstring_view path(process_name);
    size_t separator = path.find_last_of(L"\\/");
    std::wstring_view base = (separator == std::wstring_view::npos) ? path : path.substr(separator + 1);
    constexpr std::wstring_view task_manager = L"taskmgr.exe";
    if (base.size() != task_manager.size())
      {
        return false;
      }

    return std::equal(base.begin(), base.end(), task_manager.begin(), [](char lhs, char rhs) {
      return std::tolower(static_cast<unsigned char>(lhs)) == std::tolower(static_cast<unsigned char>(rhs));
    });
  }
} // namespace

bool
TaskManagerWatcher::is_running()
{
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE)
    {
      return false;
    }

  PROCESSENTRY32W entry{};
  entry.dwSize = sizeof(entry);

  if (!Process32FirstW(snapshot, &entry))
    {
      CloseHandle(snapshot);
      return false;
    }

  while (true)
    {
      if (is_task_manager_process_name(entry.szExeFile))
        {
          CloseHandle(snapshot);
          return true;
        }

      if (!Process32NextW(snapshot, &entry))
        {
          break;
        }
    }

  CloseHandle(snapshot);
  return false;
}
