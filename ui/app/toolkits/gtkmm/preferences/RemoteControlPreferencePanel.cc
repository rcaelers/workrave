// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "RemoteControlPreferencePanel.hh"

#include "commonui/nls.h"
#include "core/CoreConfig.hh"
#include "utils/Paths.hh"

#include "GtkUtil.hh"
#include "Hig.hh"

using namespace workrave;

RemoteControlPreferencePanel::RemoteControlPreferencePanel()
  : Gtk::VBox(false, 6)
{
  create_panel();
}

void
RemoteControlPreferencePanel::create_panel()
{
  auto *grpc_panel = Gtk::manage(new HigCategoryPanel(_("gRPC")));

  auto *grpc_enabled_label = Gtk::manage(GtkUtil::create_label_with_tooltip(
    _("Enable gRPC"),
    _("Allows local applications to control Workrave through gRPC. Disabling this closes the server immediately.")));
  grpc_enabled_cb = Gtk::manage(new Gtk::CheckButton());
  grpc_enabled_cb->add(*grpc_enabled_label);
  grpc_enabled_cb->set_active(CoreConfig::grpc_enabled()());
  grpc_enabled_cb->signal_toggled().connect(sigc::mem_fun(*this, &RemoteControlPreferencePanel::on_grpc_enabled_toggled));
  grpc_panel->add_widget(*grpc_enabled_cb, false, false);

  grpc_transport_combo = Gtk::manage(new Gtk::ComboBoxText());
  grpc_transport_combo->append(_("Unix domain socket"));
  grpc_transport_combo->append(_("TCP/IP (loopback only)"));
  grpc_transport_combo->set_active(CoreConfig::grpc_transport()() == "tcp" ? 1 : 0);
  grpc_transport_combo->signal_changed().connect(sigc::mem_fun(*this, &RemoteControlPreferencePanel::on_grpc_transport_changed));
  grpc_panel->add_label(std::string(_("Connection type")) + ":", *grpc_transport_combo);

  grpc_socket_label = Gtk::manage(new Gtk::Label(workrave::utils::Paths::get_rpc_socket_path().string()));
  grpc_socket_label->set_xalign(0.0);
  grpc_socket_label->set_selectable(true);
  grpc_socket_label->set_tooltip_text(_("Filesystem path of the Unix-domain socket used by Workrave."));
  grpc_panel->add_label(std::string(_("Socket filename")) + ":", *grpc_socket_label);

  grpc_port_spin = Gtk::manage(new Gtk::SpinButton());
  grpc_port_spin->set_range(1, 65535);
  grpc_port_spin->set_increments(1, 100);
  grpc_port_spin->set_numeric(true);
  grpc_port_spin->set_width_chars(5);
  grpc_port_spin->set_value(CoreConfig::grpc_port()());
  grpc_port_spin->signal_value_changed().connect(sigc::mem_fun(*this, &RemoteControlPreferencePanel::on_grpc_port_changed));
  grpc_panel->add_label(std::string(_("TCP port")) + ":", *grpc_port_spin);

  update_grpc_widgets();
  grpc_panel->set_border_width(12);
  pack_start(*grpc_panel, false, false, 0);
}

void
RemoteControlPreferencePanel::on_grpc_enabled_toggled()
{
  CoreConfig::grpc_enabled().set(grpc_enabled_cb->get_active());
  update_grpc_widgets();
}

void
RemoteControlPreferencePanel::on_grpc_transport_changed()
{
  CoreConfig::grpc_transport().set(grpc_transport_combo->get_active_row_number() == 1 ? "tcp" : "unix");
  update_grpc_widgets();
}

void
RemoteControlPreferencePanel::on_grpc_port_changed()
{
  CoreConfig::grpc_port().set(grpc_port_spin->get_value_as_int());
}

void
RemoteControlPreferencePanel::update_grpc_widgets()
{
  const bool enabled = grpc_enabled_cb->get_active();
  grpc_transport_combo->set_sensitive(enabled);
  grpc_socket_label->set_sensitive(enabled && grpc_transport_combo->get_active_row_number() == 0);
  grpc_port_spin->set_sensitive(enabled && grpc_transport_combo->get_active_row_number() == 1);
}
