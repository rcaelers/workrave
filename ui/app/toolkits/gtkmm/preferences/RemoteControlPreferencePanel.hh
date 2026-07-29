// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REMOTECONTROLPREFERENCEPANEL_HH
#define REMOTECONTROLPREFERENCEPANEL_HH

#include <gtkmm.h>

class RemoteControlPreferencePanel : public Gtk::VBox
{
public:
  RemoteControlPreferencePanel();
  ~RemoteControlPreferencePanel() override = default;

private:
  void create_panel();
  void on_grpc_enabled_toggled();
  void on_grpc_transport_changed();
  void on_grpc_port_changed();
  void update_grpc_widgets();

  Gtk::CheckButton *grpc_enabled_cb{nullptr};
  Gtk::ComboBoxText *grpc_transport_combo{nullptr};
  Gtk::Label *grpc_socket_label{nullptr};
  Gtk::SpinButton *grpc_port_spin{nullptr};
};

#endif // REMOTECONTROLPREFERENCEPANEL_HH
