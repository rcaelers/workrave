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

#include <boost/format/format_fwd.hpp>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Menus.hh"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "debug.hh"
#include "commonui/nls.h"
#include "core/CoreConfig.hh"
#include "utils/TimeSource.hh"

using namespace workrave;

Menus::Menus(std::shared_ptr<IApplication> app)
  : app(app)
  , toolkit(app->get_toolkit())
  , core(app->get_core())
  , menu_model(app->get_menu_model())
{
  init();
}

void
Menus::init()
{
  workrave::OperationMode mode = core->get_operation_mode();
  workrave::UsageMode usage = core->get_usage_mode();

  menus::SubMenuNode::Ptr root = menu_model->get_root();

  menus::Node::Ptr item = menus::ActionNode::create(OPEN, _("_Open"), [this] { on_menu_open_main_window(); });
  root->add(item);

  auto separator = menus::SeparatorNode::create();
  root->add(separator);

  item = menus::ActionNode::create(PREFERENCES, _("_Preferences"), [this] { on_menu_preferences(); });
  root->add(item);

  item = menus::ActionNode::create(REST_BREAK, _("_Rest break"), [this] { on_menu_restbreak_now(); });
  root->add(item);

  item = menus::ActionNode::create(EXERCISES, _("_Exercises"), [this] { on_menu_exercises(); });
  root->add(item);

  item = menus::ActionNode::create(STATISTICS, _("S_tatistics"), [this] { on_menu_statistics(); });
  root->add(item);

  separator = menus::SeparatorNode::create();
  root->add(separator);

  auto modemenu = menus::SubMenuNode::create(MODE_MENU, _("_Mode"));
  root->add(modemenu);

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

  auto moderesetmenu = create_mode_autoreset_menu();
  root->add(moderesetmenu);

  reading_item = menus::ToggleNode::create(MODE_READING, _("_Reading mode"), [this] { on_menu_reading(); });
  reading_item->set_checked(usage == workrave::UsageMode::Reading);
  root->add(reading_item);

  separator = menus::SeparatorNode::create();
  root->add(separator);

  item = menus::ActionNode::create(ABOUT, _("_About"), [this] { on_menu_about(); });
  root->add(item);

  item = menus::ActionNode::create(QUIT, _("_Quit"), [this] { on_menu_quit(); });
  root->add(item);
  menu_model->update();

  connect(core->signal_operation_mode_changed(), this, [this](auto mode) { on_operation_mode_changed(mode); });
  connect(core->signal_usage_mode_changed(), this, [this](auto mode) { on_usage_mode_changed(mode); });
}

menus::SubMenuNode::Ptr
Menus::create_mode_autoreset_menu()
{
  auto moderesetmenu = menus::SubMenuNode::create(MODE_AUTORESET_MENU, _("Reset to normal"));

  modereset_group = menus::RadioGroupNode::create(MODE_AUTORESET, "");
  moderesetmenu->add(modereset_group);

  auto mode_reset_options = CoreConfig::operation_mode_auto_reset_options()();
  if (mode_reset_options.empty())
    {
      mode_reset_options = {30, 60, 120, 480};
    }

  if (find(mode_reset_options.begin(), mode_reset_options.end(), CoreConfig::operation_mode_auto_reset()()) == mode_reset_options.end())
    {
      mode_reset_options.push_back(CoreConfig::operation_mode_auto_reset()());
      std::sort(mode_reset_options.begin(), mode_reset_options.end());
    }

  auto radio_item =
    menus::RadioNode::create(modereset_group, std::string(MODE_AUTORESET) + ".0", _("Off"), 0, [this] { on_menu_mode_autoreset(0); });

  modereset_group->add(radio_item);

  for (auto duration: mode_reset_options)
    {
      auto hours = duration / 60;
      auto minutes = duration % 60;

      std::string text = (hours == 0) ? "" : (hours == 1) ? _("1 hour") : boost::str(boost::format("%1% hours") % hours);
      if (!text.empty() && minutes > 0)
        {
          text += ", ";
        }
      text += (minutes == 0) ? "" : (minutes == 1) ? _("1 minute") : boost::str(boost::format("%1% minutes") % minutes);

      radio_item = menus::RadioNode::create(modereset_group,
                                            std::string(MODE_AUTORESET) + "." + std::to_string(duration),
                                            text,
                                            duration,
                                            [duration, this] { on_menu_mode_autoreset(duration); });
      modereset_group->add(radio_item);
    }
  modereset_group->select(CoreConfig::operation_mode_auto_reset()());

  return moderesetmenu;
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
Menus::on_menu_mode_autoreset(int duration)
{
  CoreConfig::operation_mode_last_change_time().set(workrave::utils::TimeSource::get_real_time_sec());
  CoreConfig::operation_mode_auto_reset().set(duration);
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
}

void
Menus::on_usage_mode_changed(UsageMode m)
{
  reading_item->set_checked(m == UsageMode::Reading);
}
