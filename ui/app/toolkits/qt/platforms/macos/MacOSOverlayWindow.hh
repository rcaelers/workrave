// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef MACOSOVERLAYWINDOW_HH
#define MACOSOVERLAYWINDOW_HH

class QWindow;

void begin_macos_overlay(QWindow *window);
void order_macos_overlay_front(QWindow *window);
void end_macos_overlay(QWindow *window);

#endif // MACOSOVERLAYWINDOW_HH
