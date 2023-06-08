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

#ifndef BREAKSTATEMODEL_HH
#define BREAKSTATEMODEL_HH

#include <memory>
#include <boost/signals2.hpp>

#include "config/IConfigurator.hh"

#include "core/IBreak.hh"
#include "core/CoreTypes.hh"

#include "IActivityMonitor.hh"
#include "Timer.hh"
#include "CoreHooks.hh"

// Forward declarion of external interface.
namespace workrave
{
  class IApp;
}

enum class BreakStage
{
  None,
  Snoozed,
  Prelude,
  Taking,
  Delayed
};

template<>
struct workrave::utils::enum_traits<BreakStage>
{
  static constexpr std::array<std::pair<std::string_view, BreakStage>, 5> names{{{"none", BreakStage::None},
                                                                                 {"snoozed", BreakStage::Snoozed},
                                                                                 {"prelude", BreakStage::Prelude},
                                                                                 {"taking", BreakStage::Taking},
                                                                                 {"delayed", BreakStage::Delayed}}};
};

class BreakStateModel
  : public IActivityMonitorListener
  , public std::enable_shared_from_this<BreakStateModel>
{
public:
  using Ptr = std::shared_ptr<BreakStateModel>;

public:
  BreakStateModel(workrave::BreakId id,
                  workrave::IApp *app,
                  Timer::Ptr timer,
                  IActivityMonitor::Ptr activity_monitor,
                  CoreHooks::Ptr hooks);
  ~BreakStateModel() override = default;

  void process();

  void start_break();
  void force_start_break(workrave::utils::Flags<workrave::BreakHint> break_hint);
  void postpone_break();
  void skip_break();
  void stop_break();
  void override(workrave::BreakId id);

  boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event();
  boost::signals2::signal<void(BreakStage)> &signal_break_stage_changed();

  bool is_taking() const;
  bool is_active() const;
  bool has_reached_max_preludes();
  BreakStage get_break_stage() const;

  void set_max_number_of_preludes(int max_preludes);
  void set_enabled(bool enabled);

private:
  void force_idle();
  void goto_stage(BreakStage stage);

  void break_window_start();
  void break_window_update();
  void break_window_stop();
  void prelude_window_start();
  void prelude_window_update();
  void prelude_window_stop();

  // IActivityMonitorListener
  bool action_notify() override;

private:
  //! ID of the break controlled by this Break.
  workrave::BreakId break_id;

  //! GUI Factory used to create the break/prelude windows.
  workrave::IApp *application;

  //! Interface to the timer controlling the break.
  Timer::Ptr timer;

  //!
  IActivityMonitor::Ptr activity_monitor;

  CoreHooks::Ptr hooks;

  //! Current stage in the break.
  BreakStage break_stage;

  //! How long is the prelude active.
  int prelude_time;

  //! forced break (i.e. RestBreak now, or screenlock)
  bool forced_break;

  //! How many times have we preluded (since the limit was reached)
  int prelude_count;

  //! After how many preludes do we force a break or give up?
  int max_number_of_preludes;

  //! Is this a break that is not controlled by the timer.
  bool fake_break;

  //! Fake break counter.
  int64_t fake_break_count;

  //! Break will be stopped because the user pressed postpone/skip.
  bool user_abort;

  //! User became active during delayed break.
  bool delayed_abort;

  //! Break hint if break has been started.
  workrave::utils::Flags<workrave::BreakHint> break_hint;

  //! Break enabled?
  bool enabled;

  //!
  boost::signals2::signal<void(workrave::BreakEvent)> break_event_signal;
  boost::signals2::signal<void(BreakStage)> break_stage_changed_signal;
};

#endif // BREAKSTATEMODEL_HH
