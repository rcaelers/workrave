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

#ifndef BREAK_HH
#define BREAK_HH

#include <memory>

#include "BreakConfig.hh"
#include "BreakDBus.hh"
#include "BreakStateModel.hh"
#include "BreakStatistics.hh"
#include "IActivityMonitor.hh"
#include "Statistics.hh"
#include "Timer.hh"

#include "config/IConfigurator.hh"
#include "core/IBreak.hh"
#include "dbus/IDBus.hh"

class Break : public workrave::IBreak
{
public:
  using Ptr = std::shared_ptr<Break>;

public:
  Break(workrave::BreakId id,
        workrave::IApp *app,
        Timer::Ptr timer,
        IActivityMonitor::Ptr activity_monitor,
        Statistics::Ptr statistics,
        std::shared_ptr<workrave::dbus::IDBus> dbus,
        CoreHooks::Ptr hooks);

  // IBreak
  boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event() override;
  [[nodiscard]] std::string get_name() const override;
  [[nodiscard]] bool is_enabled() const override;
  [[nodiscard]] bool is_running() const override;
  [[nodiscard]] bool is_taking() const override;
  [[nodiscard]] bool is_max_preludes_reached() const override;
  [[nodiscard]] bool is_active() const override;
  [[nodiscard]] int64_t get_elapsed_time() const override;
  [[nodiscard]] int64_t get_elapsed_idle_time() const override;
  [[nodiscard]] int64_t get_auto_reset() const override;
  [[nodiscard]] bool is_auto_reset_enabled() const override;
  [[nodiscard]] int64_t get_limit() const override;
  [[nodiscard]] bool is_limit_enabled() const override;
  [[nodiscard]] int64_t get_timer_remaining() const;
  [[nodiscard]] int64_t get_total_overdue_time() const override;
  void postpone_break() override;
  void skip_break() override;

  void process();
  void start_break();
  void stop_break();
  void force_start_break(workrave::utils::Flags<workrave::BreakHint> break_hint);
  void override(workrave::BreakId id);
  void daily_reset();
  [[nodiscard]] bool is_microbreak_used_for_activity() const;
  [[nodiscard]] std::string get_break_stage() const;

  static std::string get_stage_text(BreakStage stage);

private:
  workrave::BreakId break_id;
  Timer::Ptr timer;
  BreakStateModel::Ptr break_state_model;
  BreakStatistics::Ptr break_statistics;
  BreakConfig::Ptr break_configuration;
  BreakDBus::Ptr break_dbus;
};

#endif // BREAK_HH
