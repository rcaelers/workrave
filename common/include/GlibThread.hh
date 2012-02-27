// GlibThread.hh --- GLib Thread class
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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

#ifndef GLIBTHREAD_HH
#define GLIBTHREAD_HH

#include "Runnable.hh"
#include <glib.h>

/*!
 * Thread class.
 */
class Thread
{
public:
  Thread();
  Thread(Runnable *runnable);
  virtual ~Thread();

  virtual void run();

  void start();
  void wait();

private:
  void internal_run();

private:
  static gpointer thread_handler(gpointer data);

  GThread *thread_handle;
  Runnable *runnable;
};


#endif // GLIBTHREAD_HH
