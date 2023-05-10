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

#ifndef REMOTE_H
#define REMOTE_H

#include <windows.h>
#include <string>

class Remote
{
public:
  Remote();
  void open();

private:
  std::string get_class_name(HWND hwnd);
  std::string get_title(HWND hwnd);
  std::string get_process_name(HWND hwnd);

  static BOOL CALLBACK enum_windows_cb(HWND hwnd, LPARAM lParam);

private:
  HWND hwnd = nullptr;
};

#endif /* REMOTE_H */
