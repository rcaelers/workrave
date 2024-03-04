// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "MonitoringPreferencePanel.hh"

#include "commonui/nls.h"
#include "debug.hh"

#include "DataConnector.hh"

#include "GtkUtil.hh"
#include "core/CoreConfig.hh"

using namespace workrave;
using namespace workrave::utils;

MonitoringPreferencePanel::MonitoringPreferencePanel(std::shared_ptr<IApplicationContext> app)
  : Gtk::VBox(false, 6)
  , app(app)
  , connector(std::make_shared<DataConnector>(app))

{
  TRACE_ENTRY();
  create_panel();
}

MonitoringPreferencePanel::~MonitoringPreferencePanel()
{
  TRACE_ENTRY();
}

void
MonitoringPreferencePanel::create_panel()
{
  set_border_width(12);

#if defined(PLATFORM_OS_WINDOWS)
  Gtk::Label *monitor_type_lab = Gtk::manage(GtkUtil::create_label(_("Use alternate monitor"), false));
  monitor_type_cb = Gtk::manage(new Gtk::CheckButton());
  monitor_type_cb->add(*monitor_type_lab);
  monitor_type_cb->signal_toggled().connect(sigc::mem_fun(*this, &MonitoringPreferencePanel::on_monitor_type_toggled));
  pack_start(*monitor_type_cb, false, false, 0);

  Gtk::Label *monitor_type_help1 = Gtk::manage(
    GtkUtil::create_label(_("Enable this option if Workrave fails to detect when you are using your computer"), false));
  pack_start(*monitor_type_help1, false, false, 0);
  Gtk::Label *monitor_type_help2 = Gtk::manage(
    GtkUtil::create_label(_("Workrave needs to be restarted manually after changing this setting"), false));
  pack_start(*monitor_type_help2, false, false, 0);

  sensitivity_box = Gtk::manage(new Gtk::HBox());
  Gtk::Widget *sensitivity_lab = Gtk::manage(
    GtkUtil::create_label_with_tooltip(_("Mouse sensitivity:"),
                                       _("Number of pixels the mouse should move before it is considered activity.")));
  Gtk::SpinButton *sensitivity_spin = Gtk::manage(new Gtk::SpinButton(sensitivity_adjustment));
  sensitivity_box->pack_start(*sensitivity_lab, false, false, 0);
  sensitivity_box->pack_start(*sensitivity_spin, false, false, 0);
  pack_start(*sensitivity_box, false, false, 0);

  connector->connect(CoreConfig::monitor_sensitivity(), dc::wrap(sensitivity_adjustment));

  std::string monitor_type;
  app->get_configurator()->get_value_with_default("advanced/monitor", monitor_type, "default");

  monitor_type_cb->set_active(monitor_type != "default");

  sensitivity_box->set_sensitive(monitor_type != "default");
#endif

  debug_btn = Gtk::manage(new Gtk::Button(_("Debug monitoring")));
  debug_btn->signal_clicked().connect(sigc::mem_fun(*this, &MonitoringPreferencePanel::on_debug_pressed));
  pack_start(*debug_btn, false, false, 0);
}

#if defined(PLATFORM_OS_WINDOWS)
void
MonitoringPreferencePanel::on_monitor_type_toggled()
{
  bool on = monitor_type_cb->get_active();
  app->get_configurator()->set_value("advanced/monitor", on ? "lowlevel" : "default");
  sensitivity_box->set_sensitive(on);
}
#endif

void
MonitoringPreferencePanel::on_debug_pressed()
{
  app->get_toolkit()->show_window(IToolkit::WindowType::Debug);
}
