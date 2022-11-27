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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "PreferencesDialog.hh"

#include <memory>
#include <boost/algorithm/string/split.hpp>
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

#include "ui/prefwidgets/gtkmm/Widget.hh"
#include "ui/prefwidgets/gtkmm/BoxWidget.hh"
#include "ui/prefwidgets/gtkmm/Builder.hh"

using namespace ui::prefwidgets;
using namespace workrave;
using namespace workrave::utils;

PreferencesDialog::PreferencesDialog(std::shared_ptr<IApplicationContext> app)
  : Gtk::Dialog(_("Preferences"))
  , app(app)
{
  TRACE_ENTRY();

  init_ui();

  create_timers_page();
  create_ui_page();
  create_monitoring_page();
  create_sounds_page();

  create_plugin_pages();
  create_plugin_panels();

  show_all();
}

PreferencesDialog::~PreferencesDialog()
{
  TRACE_ENTRY();
  auto core = app->get_core();
  core->remove_operation_mode_override("preferences");
}

void
PreferencesDialog::init_ui()
{
  set_default_size(960, 640);
  set_border_width(6);

  stack.set_hexpand(true);
  stack.set_vexpand(true);
  stack.set_transition_type(Gtk::StackTransitionType::STACK_TRANSITION_TYPE_CROSSFADE);

  auto *box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));
  box->set_hexpand(true);
  box->set_vexpand(true);
  box->pack_start(panel_list, true, true, 0);
  box->pack_start(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL)), false, false, 0);
  box->pack_start(stack, true, true, 0);
  get_content_area()->add(*box);

  panel_list.signal_activated().connect([this](const std::string &id) { stack.set_visible_child(id); });

  Gtk::Button *button = add_button(_("Close"), Gtk::RESPONSE_CLOSE);
  button->set_image_from_icon_name("window-close", Gtk::ICON_SIZE_BUTTON);

  show_all();
}

void
PreferencesDialog::create_timers_page()
{
  std::array<std::string, 3> timer_ids = {"microbreak", "restbreak", "dailylimit"};
  auto page = add_page("timer", _("Timers"), "timer.svg");

  Glib::RefPtr<Gtk::SizeGroup> hsize_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  Glib::RefPtr<Gtk::SizeGroup> vsize_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_VERTICAL);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Gtk::Widget *box = Gtk::manage(GtkUtil::create_label_for_break((BreakId)i));
      TimerPreferencePanel *tp = Gtk::manage(new TimerPreferencePanel(app, BreakId(i), hsize_group, vsize_group));
      box->show_all();
      page->add_panel(timer_ids.at(i), tp, box);
    }
}

void
PreferencesDialog::create_ui_page()
{
  auto page = add_page("ui", _("User interface"), "display.svg");

#if !defined(PLATFORM_OS_MACOS)
  Gtk::Widget *gui_general_page = Gtk::manage(new GeneralPreferencePanel(app));
  page->add_panel("general", gui_general_page, _("General"));
#endif

  Gtk::Widget *gui_mainwindow_page = Gtk::manage(new TimerBoxPreferencePanel(app, "main_window"));
  page->add_panel("mainwindow", gui_mainwindow_page, _("Status Window"));

#if !defined(PLATFORM_OS_MACOS)
  Gtk::Widget *gui_applet_page = Gtk::manage(new TimerBoxPreferencePanel(app, "applet"));
  page->add_panel("applet", gui_applet_page, _("Applet"));
#endif
}

void
PreferencesDialog::create_monitoring_page()
{
  auto page = add_page("monitoring", _("Monitoring"), "mouse.svg");
  Gtk::Widget *monitoring_panel = Gtk::manage(new MonitoringPreferencePanel(app));
  page->add_panel("monitoring", monitoring_panel, _("Monitoring"));
}

void
PreferencesDialog::create_sounds_page()
{
  auto page = add_page("sounds", _("Sounds"), "sound.svg");
  Gtk::Widget *gui_sounds_page = Gtk::manage(new SoundPreferencePanel(app));
  page->add_panel("sounds", gui_sounds_page, _("Sounds"));
}

void
PreferencesDialog::create_plugin_pages()
{
  auto preferences_registry = app->get_internal_preferences_registry();
  for (auto &[id, def]: preferences_registry->get_pages())
    {
      auto &[label, image] = def;
      auto page = add_page(id, label, image);
    }
}

void
PreferencesDialog::create_panel(std::shared_ptr<ui::prefwidgets::Def> &def)
{
  auto pageid = def->get_page();
  auto panelid = def->get_panel();
  auto widget = def->get_widget();

  auto pageit = pages.find(pageid);
  if (pageit == pages.end())
    {
      spdlog::error("Cannot find page {} when adding panel {}", pageid, panelid);
      return;
    }
  auto page = pageit->second;
  if (auto paneldef = std::dynamic_pointer_cast<ui::prefwidgets::PanelDef>(def); paneldef)
    {
      ui::prefwidgets::gtkmm::Builder builder;
      auto *box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 6));
      auto frame = std::make_shared<ui::prefwidgets::gtkmm::BoxWidget>(box);
      builder.build(widget, frame);
      page->add_panel(panelid, box, paneldef->get_label());
    }
}

void
PreferencesDialog::create_plugin_panels()
{
  auto preferences_registry = app->get_internal_preferences_registry();

  for (auto &[id, deflist]: preferences_registry->get_widgets())
    {
      for (std::shared_ptr<ui::prefwidgets::Def> def: deflist)
        {
          create_panel(def);
        }
    }
}

std::shared_ptr<PreferencesPage>
PreferencesDialog::add_page(const std::string &id, const std::string &label, const std::string &image)
{
  Gtk::Notebook *notebook = Gtk::manage(new Gtk::Notebook());
  notebook->set_show_tabs(false);
  notebook->set_show_border(false);

  notebook->set_border_width(6);
  notebook->set_tab_pos(Gtk::POS_TOP);

  auto page_info = std::make_shared<PreferencesPage>(id, notebook);
  pages[id] = page_info;

  panel_list.add_row(id, label, image);
  stack.add(*notebook, id);

  return page_info;
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
  return Gtk::Dialog::on_focus_in_event(event);
}

bool
PreferencesDialog::on_focus_out_event(GdkEventFocus *event)
{
  TRACE_ENTRY();
  auto core = app->get_core();
  core->remove_operation_mode_override("preferences");
  return Gtk::Dialog::on_focus_out_event(event);
}

PreferencesPage::PreferencesPage(const std::string &id, Gtk::Notebook *notebook)
  : id(id)
  , notebook(notebook)
{
}

void
PreferencesPage::add_panel(const std::string &id, Gtk::Widget *widget, const std::string &label)
{
  panels.emplace(id, widget);
  panel_order.push_back(id);

  notebook->append_page(*widget, label);
  notebook->set_show_tabs(notebook->get_n_pages() > 1);
}

void
PreferencesPage::add_panel(const std::string &id, Gtk::Widget *widget, Gtk::Widget *label)
{
  panels.emplace(id, widget);
  panel_order.push_back(id);

  notebook->append_page(*widget, *label);
  notebook->set_show_tabs(notebook->get_n_pages() > 1);
}
