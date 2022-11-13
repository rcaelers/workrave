// Copyright (C) 2002 - 2020 Raymond Penners <raymond@dotsphinx.com>
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

#include "gtkmm/object.h"
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "PreferencesDialog.hh"

#include <gtkmm.h>

#include "ui/GUIConfig.hh"
#include "commonui/nls.h"
#include "GtkUtil.hh"
#include "Hig.hh"
#include "debug.hh"

#include "GeneralPreferencePanel.hh"
#include "MonitoringPreferencePanel.hh"
#include "SoundPreferencePanel.hh"
#include "TimerBoxPreferencePanel.hh"
#include "TimerPreferencePanel.hh"

using namespace ui::prefwidgets;

using namespace workrave;
using namespace workrave::utils;

PreferencesDialog::PreferencesDialog(std::shared_ptr<IApplicationContext> app)
  : HigDialog(_("Preferences"), false, false)
  , app(app)
{
  TRACE_ENTRY();

  create_timers_page();
  create_ui_page();

  get_vbox()->pack_start(notebook, true, true, 0);
  Gtk::Button *button = add_button(_("Close"), Gtk::RESPONSE_CLOSE);
  button->set_image_from_icon_name("window-close", Gtk::ICON_SIZE_BUTTON);

  show_all();
}

PreferencesDialog::~PreferencesDialog()
{
  TRACE_ENTRY();
  auto core = app->get_core();
  core->remove_operation_mode_override("preferences");
}

void
PreferencesDialog::create_timers_page()
{
  Gtk::Notebook *tnotebook = Gtk::manage(new Gtk::Notebook());

  tnotebook->set_tab_pos(Gtk::POS_TOP);
  Glib::RefPtr<Gtk::SizeGroup> hsize_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  Glib::RefPtr<Gtk::SizeGroup> vsize_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_VERTICAL);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Gtk::Widget *box = Gtk::manage(GtkUtil::create_label_for_break((BreakId)i));
      TimerPreferencePanel *tp = Gtk::manage(new TimerPreferencePanel(app, BreakId(i), hsize_group, vsize_group));
      box->show_all();
      tnotebook->append_page(*tp, *box);
    }

  Gtk::Widget *box = Gtk::manage(GtkUtil::create_label("Monitoring", false));
  Gtk::Widget *monitoring_page = Gtk::manage(new MonitoringPreferencePanel(app));

  tnotebook->append_page(*monitoring_page, *box);

  add_page("timer", _("Timers"), "time.png", *tnotebook);
}

void
PreferencesDialog::create_ui_page()
{
  Gtk::Notebook *gui_page = Gtk::manage(new Gtk::Notebook());

#if !defined(PLATFORM_OS_MACOS)
  Gtk::Widget *gui_general_page = Gtk::manage(new GeneralPreferencePanel(app));
  gui_page->append_page(*gui_general_page, _("General"));
#endif

  Gtk::Widget *gui_sounds_page = Gtk::manage(new SoundPreferencePanel(app));
  gui_page->append_page(*gui_sounds_page, _("Sounds"));

  Gtk::Widget *gui_mainwindow_page = Gtk::manage(new TimerBoxPreferencePanel(app, "main_window"));
  gui_page->append_page(*gui_mainwindow_page, _("Status Window"));

#if !defined(PLATFORM_OS_MACOS)
  Gtk::Widget *gui_applet_page = Gtk::manage(new TimerBoxPreferencePanel(app, "applet"));
  gui_page->append_page(*gui_applet_page, _("Applet"));
#endif

  add_page("ui", _("User interface"), "display.png", *gui_page);
}

void
PreferencesDialog::add_page(const std::string &id, const std::string &label, const std::string &image, Gtk::Widget &widget)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = GtkUtil::create_pixbuf(image);
  notebook.add_page(label.c_str(), pixbuf, widget);
  pages[id] = &widget;
}

int
PreferencesDialog::run()
{
  show_all();
  return 0;
}

bool
PreferencesDialog::on_focus_in_event(GdkEventFocus *event)
{
  TRACE_ENTRY();
  BlockMode block_mode = GUIConfig::block_mode()();
  if (block_mode != BlockMode::Off)
    {
      auto core = app->get_core();
      OperationMode mode = core->get_active_operation_mode();
      if (mode == OperationMode::Normal)
        {
          core->set_operation_mode_override(OperationMode::Quiet, "preferences");
        }
    }
  return HigDialog::on_focus_in_event(event);
}

bool
PreferencesDialog::on_focus_out_event(GdkEventFocus *event)
{
  TRACE_ENTRY();
  auto core = app->get_core();

  core->remove_operation_mode_override("preferences");
  return HigDialog::on_focus_out_event(event);
}
