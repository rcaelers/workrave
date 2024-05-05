// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_LOGIN_SESSION_HH
#define WORKRAVE_UI_LOGIN_SESSION_HH

#include <memory>

#include <giomm.h>

#include "ui/Plugin.hh"

class LoginSession : public Plugin<LoginSession>
{
public:
  using Ptr = std::shared_ptr<LoginSession>;

  explicit LoginSession(std::shared_ptr<IPluginContext> context);
  ~LoginSession() override;
  void init();

  std::string get_plugin_id() const override
  {
    return "workrave.LoginSession";
  }

private:
  void on_signal(const Glib::ustring &sender, const Glib::ustring &signal_name, const Glib::VariantContainerBase &params);

private:
  std::shared_ptr<IPluginContext> context;
  Glib::RefPtr<Gio::DBus::Proxy> proxy;
};

#endif // WORKRAVE_UI_LOGIN_SESSION_HH
