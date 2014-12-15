// Plug.hh --- Time Bar
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2009, 2011 Rob Caelers & Raymond Penners
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

#ifndef PLUG_HH
#define PLUG_HH

#include <string>

#include <gtkmm.h>
#include <gdkmm.h>
#include <gtkmm/plug.h>

class Plug : public Gtk::Plug
{
public:
  Plug();
  Plug(int id);
  virtual ~Plug();

private:

protected:
#ifdef HAVE_GTK3
  virtual void on_realize();
  virtual void on_unrealize();
  virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif
private:
};


#endif // PLUG_HH
