// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include <QtGui>
#include <QtWidgets>

#include "IconListNotebook.hh"
#include "ui/IApplicationContext.hh"

class PanelList;

class PreferencesPage
{
public:
  PreferencesPage(const std::string &id, QTabWidget *notebook);

  void add_panel(const std::string &id, QWidget *widget, const QString &label);
  void add_panel(const std::string &id, QWidget *widget, const QString &label, QIcon icon);

private:
  std::string id;
  std::map<std::string, QWidget *> panels;
  std::list<std::string> panel_order;
  QTabWidget *notebook{nullptr};
};

class PreferencesDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PreferencesDialog(std::shared_ptr<IApplicationContext> app);
  ~PreferencesDialog() override;

private:
  std::shared_ptr<PreferencesPage> add_page(const std::string &id, const QString &label, const std::string &image);

  void init_ui();
  void create_timers_page();
  void create_ui_page();
  void create_monitoring_page();
  void create_sounds_page();

  void create_plugin_pages();
  void create_plugin_panels();
  void create_panel(std::shared_ptr<ui::prefwidgets::Def> &def);

  // bool on_focus_in_event(GdkEventFocus *event) override;
  // bool on_focus_out_event(GdkEventFocus *event) override;

private:
  std::shared_ptr<IApplicationContext> app;
  std::map<std::string, std::shared_ptr<PreferencesPage>> pages;
  IconListNotebook *notebook{nullptr};
  // QStackedWidget *stack{nullptr};
  // QHBoxLayout *layout{nullptr};
};

#endif // PREFERENCESDIALOG_HH
