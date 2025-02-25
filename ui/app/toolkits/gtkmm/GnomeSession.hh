// Copyright (C) 2010, 2011, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_COMMON_SESSION_HH
#define WORKRAVE_UI_COMMON_SESSION_HH

#include <memory>

#include <giomm.h>

#include "ui/Plugin.hh"

class GnomeSession : public Plugin<GnomeSession>
{
public:
  using Ptr = std::shared_ptr<GnomeSession>;

  explicit GnomeSession(std::shared_ptr<IPluginContext> context);
  ~GnomeSession() override;
  void init();

  std::string get_plugin_id() const override
  {
    return "workrave.GnomeSession";
  }

private:
  void on_signal(const Glib::ustring &sender, const Glib::ustring &signal_name, const Glib::VariantContainerBase &params);

private:
  std::shared_ptr<IPluginContext> context;
  Glib::RefPtr<Gio::DBus::Proxy> proxy;
};

#endif // WORKRAVE_UI_COMMON_SESSION_HH
