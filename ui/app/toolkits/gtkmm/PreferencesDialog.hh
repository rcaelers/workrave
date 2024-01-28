// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#ifndef PREFERENCESDIALOG_HH
#define PREFERENCESDIALOG_HH

#include <memory>
#include <list>
#include <map>

#include <gtkmm.h>

#include "ui/IApplicationContext.hh"
#include "ui/prefwidgets/gtkmm/BoxWidget.hh"

#include "PanelList.hh"

class PreferencesPage
{
public:
  PreferencesPage(const std::string &id, Gtk::Notebook *notebook);

  void add_panel(const std::string &id, Gtk::Widget *widget, const std::string &label);
  void add_panel(const std::string &id, Gtk::Widget *widget, Gtk::Widget *label);

private:
  std::string id;
  std::map<std::string, Gtk::Widget *> panels;
  std::list<std::string> panel_order;
  Gtk::Notebook *notebook{nullptr};
};

class PreferencesDialog : public Gtk::Dialog
{
public:
  explicit PreferencesDialog(std::shared_ptr<IApplicationContext> app);
  ~PreferencesDialog() override;

private:
  std::shared_ptr<PreferencesPage> add_page(const std::string &id, const std::string &label, const std::string &image);

  void init_ui();
  void create_timers_page();
  void create_ui_page();
  void create_monitoring_page();
  void create_sounds_page();

  void create_plugin_pages();
  void create_plugin_panels();
  void create_panel(std::shared_ptr<ui::prefwidgets::Def> &def);

  bool on_focus_in_event(GdkEventFocus *event) override;
  bool on_focus_out_event(GdkEventFocus *event) override;

private:
  std::shared_ptr<IApplicationContext> app;
  std::map<std::string, std::shared_ptr<PreferencesPage>> pages;
  std::list<std::shared_ptr<ui::prefwidgets::gtkmm::BoxWidget>> frames;
  PanelList panel_list;
  Gtk::Stack stack;
};

#endif // PREFERENCESWINDOW_HH
