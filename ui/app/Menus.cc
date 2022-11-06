// Copyright (C) 2013 - 2021 Rob Caelers & Raymond Penners
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

#include "core/CoreTypes.hh"
#include "commonui/MenuModel.hh"
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Menus.hh"

#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <spdlog/spdlog.h>
#include "spdlog/fmt/ostr.h"

#include "debug.hh"
#include "commonui/nls.h"
#include "core/CoreConfig.hh"
#include "utils/TimeSource.hh"

using namespace workrave;
using namespace workrave::utils;

Menus::Menus(std::shared_ptr<IApplicationContext> app)
  : app(app)
  , toolkit(app->get_toolkit())
  , core(app->get_core())
  , menu_model(app->get_menu_model())
{
  TRACE_ENTRY();
  init();
}

Menus::~Menus()
{
  TRACE_ENTRY();
}

void
Menus::init()
{
  workrave::OperationMode mode = core->get_regular_operation_mode();
  workrave::UsageMode usage = core->get_usage_mode();

  menus::SubMenuNode::Ptr root = menu_model->get_root();

  auto section_main = menus::SectionNode::create(SECTION_MAIN);
  root->add(section_main);

  menus::Node::Ptr item = menus::ActionNode::create(OPEN, _("_Open"), [this] { on_menu_open_main_window(); });
  section_main->add(item);

  auto separator = menus::SeparatorNode::create();
  section_main->add(separator);

  item = menus::ActionNode::create(PREFERENCES, _("_Preferences"), [this] { on_menu_preferences(); });
  section_main->add(item);

  item = menus::ActionNode::create(REST_BREAK, _("_Rest break"), [this] { on_menu_restbreak_now(); });
  section_main->add(item);

  item = menus::ActionNode::create(EXERCISES, _("_Exercises"), [this] { on_menu_exercises(); });
  section_main->add(item);

  item = menus::ActionNode::create(STATISTICS, _("S_tatistics"), [this] { on_menu_statistics(); });
  section_main->add(item);

  separator = menus::SeparatorNode::create();
  root->add(separator);

  auto section_modes = menus::SectionNode::create(SECTION_MODES);
  root->add(section_modes);

  auto modemenu = menus::SubMenuNode::create(MODE_MENU, _("_Mode"));
  section_modes->add(modemenu);

  mode_group = menus::RadioGroupNode::create(MODE, "");
  modemenu->add(mode_group);

  normal_item = menus::RadioNode::create(mode_group,
                                         MODE_NORMAL,
                                         _("_Normal"),
                                         static_cast<std::underlying_type_t<OperationMode>>(OperationMode::Normal),
                                         [this] { on_menu_normal(); });
  mode_group->add(normal_item);

  suspended_item = menus::RadioNode::create(mode_group,
                                            MODE_SUSPENDED,
                                            _("_Suspended"),
                                            static_cast<std::underlying_type_t<OperationMode>>(OperationMode::Suspended),
                                            [this] { on_menu_suspend(); });
  mode_group->add(suspended_item);

  quiet_item = menus::RadioNode::create(mode_group,
                                        MODE_QUIET,
                                        _("Q_uiet"),
                                        static_cast<std::underlying_type_t<OperationMode>>(OperationMode::Quiet),
                                        [this] { on_menu_quiet(); });
  mode_group->add(quiet_item);
  mode_group->select(static_cast<std::underlying_type_t<OperationMode>>(mode));

  auto timed_suspended_menu = menus::SubMenuNode::create(MODE_TIMED_SUSPENDED_MENU, _("Temporarily Suspended..."));
  modemenu->add(timed_suspended_menu);

  auto timed_quiet_menu = menus::SubMenuNode::create(MODE_TIMED_QUIET_MENU, _("Temporarily Quiet..."));
  modemenu->add(timed_quiet_menu);

  create_mode_autoreset_menu(OperationMode::Quiet, timed_quiet_menu);
  create_mode_autoreset_menu(OperationMode::Suspended, timed_suspended_menu);

  reading_item = menus::ToggleNode::create(MODE_READING, _("_Reading mode"), [this] { on_menu_reading(); });
  reading_item->set_checked(usage == workrave::UsageMode::Reading);
  section_modes->add(reading_item);

  separator = menus::SeparatorNode::create();
  root->add(separator);

  auto section_tail = menus::SectionNode::create(SECTION_TAIL);
  root->add(section_tail);

  item = menus::ActionNode::create(ABOUT, _("_About"), [this] { on_menu_about(); });
  section_tail->add(item);

  item = menus::ActionNode::create(QUIT, _("_Quit"), [this] { on_menu_quit(); });
  section_tail->add(item);
  menu_model->update();

  connect(core->signal_operation_mode_changed(), this, [this](auto mode) { on_operation_mode_changed(mode); });
  connect(core->signal_usage_mode_changed(), this, [this](auto mode) { on_usage_mode_changed(mode); });

  CoreConfig::operation_mode_auto_reset_time().connect(tracker, [this](auto x) { update_autoreset(); });
  CoreConfig::operation_mode_auto_reset_duration().connect(tracker, [this](auto x) { update_autoreset(); });

  update_autoreset();
}

void
Menus::create_mode_autoreset_menu(workrave::OperationMode mode, menus::SubMenuNode::Ptr menu)
{
  using namespace std::chrono_literals;

  auto id = mode == OperationMode::Quiet ? MODE_TIMED_QUIET : MODE_TIMED_SUSPENDED;

  auto mode_reset_options = CoreConfig::operation_mode_auto_reset_options()();
  if (mode_reset_options.empty())
    {
      mode_reset_options = {30min, 60min, 120min, 180min, 240min};
    }

  auto node = menus::ActionNode::create(std::string(id) + ".0", _("Indefinitely"), [mode, this] {
    on_menu_mode_for(mode, 0min);
  });

  menu->add(node);

  for (auto duration: mode_reset_options)
    {
      auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
      auto minutes = duration % 60min;

      std::string text = (hours == 0h)   ? ""
                         : (hours == 1h) ? _("For 1 hour")
                                         : boost::str(boost::format("For %1% hours") % hours.count());
      if (minutes > 0min)
        {
          if (!text.empty())
            {
              text += " ";
              text += (minutes == 1min) ? _("and 1 minute") : boost::str(boost::format("and %1% minutes") % minutes.count());
            }
          else
            {
              text += (minutes == 1min) ? _("For 1 minute") : boost::str(boost::format("For %1% minutes") % minutes.count());
            }
        }

      node = menus::ActionNode::create(std::string(id) + "." + std::to_string(duration.count()), text, [mode, duration, this] {
        on_menu_mode_for(mode, duration);
      });
      menu->add(node);
    }
  node = menus::ActionNode::create(std::string(id) + ".nextday", _("Until next day"), [mode, this] {
    on_menu_mode_for(mode, -1min);
  });
  menu->add(node);
}

void
Menus::on_menu_open_main_window()
{
  toolkit->show_window(IToolkit::WindowType::Main);
}

void
Menus::on_menu_restbreak_now()
{
  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
}

void
Menus::on_menu_about()
{
  toolkit->show_window(IToolkit::WindowType::About);
}

void
Menus::on_menu_quit()
{
  app->get_toolkit()->terminate();
}

void
Menus::on_menu_preferences()
{
  toolkit->show_window(IToolkit::WindowType::Preferences);
}

void
Menus::on_menu_exercises()
{
  toolkit->show_window(IToolkit::WindowType::Exercises);
}

void
Menus::on_menu_statistics()
{
  toolkit->show_window(IToolkit::WindowType::Statistics);
}

void
Menus::on_menu_normal()
{
  set_operation_mode(OperationMode::Normal);
}

void
Menus::on_menu_suspend()
{
  set_operation_mode(OperationMode::Suspended);
}

void
Menus::on_menu_quiet()
{
  set_operation_mode(OperationMode::Quiet);
}

void
Menus::on_menu_mode_for(workrave::OperationMode mode, std::chrono::minutes duration)
{
  using namespace std::chrono_literals;
  if (duration == 0min)
    {
      core->set_operation_mode(mode);
    }
  else
    {
      core->set_operation_mode_for(mode, duration);
    }
}

void
Menus::update_autoreset()
{
  using namespace std::chrono_literals;

  workrave::OperationMode mode = core->get_regular_operation_mode();
  auto auto_reset_duration = CoreConfig::operation_mode_auto_reset_duration()();
  auto auto_reset_time = CoreConfig::operation_mode_auto_reset_time()();
  if ((auto_reset_time.time_since_epoch().count() > 0) && (mode != OperationMode::Normal))
    {
      std::time_t time = std::chrono::system_clock::to_time_t(auto_reset_time);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&time), "%H:%M");

      if (mode == OperationMode::Quiet)
        {
          quiet_item->set_dynamic_text(boost::str(boost::format(_("Q_uiet (until %1%)")) % ss.str()));
          suspended_item->unset_dynamic_text();
        }
      else if (mode == OperationMode::Suspended)
        {
          suspended_item->set_dynamic_text(boost::str(boost::format(_("_Suspended (until %1%)")) % ss.str()));
          quiet_item->unset_dynamic_text();
        }
    }
  else if ((auto_reset_duration == -1min) && (mode != OperationMode::Normal))
    {
      if (mode == OperationMode::Quiet)
        {
          quiet_item->set_dynamic_text(_("Q_uiet (until next day)"));
          suspended_item->unset_dynamic_text();
        }
      else if (mode == OperationMode::Suspended)
        {
          suspended_item->set_dynamic_text(_("_Suspended (until next day)"));
          quiet_item->unset_dynamic_text();
        }
    }
  else
    {
      suspended_item->unset_dynamic_text();
      quiet_item->unset_dynamic_text();
    }
  menu_model->update();
}

void
Menus::on_menu_reading()
{
  set_usage_mode(reading_item->is_checked() ? UsageMode::Reading : UsageMode::Normal);
}

void
Menus::on_menu_reading(bool on)
{
  set_usage_mode(on ? UsageMode::Reading : UsageMode::Normal);
}

void
Menus::set_operation_mode(OperationMode m)
{
  core->set_operation_mode(m);
}

void
Menus::set_usage_mode(UsageMode m)
{
  core->set_usage_mode(m);
}

void
Menus::on_operation_mode_changed(OperationMode m)
{
  mode_group->select(static_cast<std::underlying_type_t<OperationMode>>(m));
  update_autoreset();
}

void
Menus::on_usage_mode_changed(UsageMode m)
{
  reading_item->set_checked(m == UsageMode::Reading);
}
