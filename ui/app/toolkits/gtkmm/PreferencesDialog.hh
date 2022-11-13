// Copyright (C) 2002 - 2013 Raymond Penners <raymond@dotsphinx.com>
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

#include <vector>

#include <gtkmm.h>

#include "Hig.hh"
#include "IconListNotebook.hh"

#include "core/ICore.hh"
#include "ui/IApplicationContext.hh"
#include "ui/SoundTheme.hh"
#include "ui/prefwidgets/gtkmm/Widget.hh"
#include "ui/prefwidgets/gtkmm/BoxWidget.hh"

class TimeEntry;
class DataConnector;

namespace Gtk
{
  class ComboBox;
  class ComboBox;
  class FileChooserButton;
  class HScale;
  class FileFilter;
} // namespace Gtk

class PreferencesDialog : public HigDialog
{
public:
  explicit PreferencesDialog(std::shared_ptr<IApplicationContext> app);
  ~PreferencesDialog() override;

  int run();

private:
  void add_page(const std::string &id, const std::string &label, const std::string &image, Gtk::Widget &widget);

  void create_timers_page();
  void create_ui_page();

  bool on_focus_in_event(GdkEventFocus *event) override;
  bool on_focus_out_event(GdkEventFocus *event) override;

private:
  std::shared_ptr<IApplicationContext> app;
  std::map<std::string, Gtk::Widget *> pages;
  IconListNotebook notebook;
};

#endif // PREFERENCESWINDOW_HH
