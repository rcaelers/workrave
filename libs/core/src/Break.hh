// Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
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

#include "core/ICore.hh"
#include "config/IConfigurator.hh"
#include "config/IConfiguratorListener.hh"
#include "core/IBreak.hh"
#include "Timer.hh"

using namespace workrave;

// Forward declarion of external interface.
namespace workrave
{
  class IApp;
  class IBreak;
} // namespace workrave

class BreakControl;

class Break
  : public IBreak
  , public workrave::config::IConfiguratorListener
{
private:
  //! ID of the break.
  BreakId break_id{BREAK_ID_NONE};

  //! Name of the break (used in configuration)
  std::string break_name;

  //! Break config prefix
  std::string break_prefix;

  //! The Configurator
  workrave::config::IConfigurator::Ptr config;

  //!
  IApp *application{nullptr};

  //! Interface pointer to the timer.
  Timer *timer{nullptr};

  //! Interface pointer to the break controller.
  BreakControl *break_control{nullptr};

  //! Break enabled?
  bool enabled{true};

  //!
  UsageMode usage_mode{UsageMode::Normal};

public:
  Break();
  ~Break() override;

  void init(BreakId id, workrave::config::IConfigurator::Ptr config, IApp *app);

  static std::string expand(const std::string &str, BreakId id);
  static std::string get_name(BreakId id);

  std::string expand(const std::string &str);
  std::string get_name() const override;
  BreakId get_id() const override;

  Timer *get_timer() const;
  BreakControl *get_break_control();

  // IBreak
  bool is_enabled() const override;
  bool is_running() const override;
  int64_t get_elapsed_time() const override;
  int64_t get_elapsed_idle_time() const override;
  int64_t get_auto_reset() const override;
  bool is_auto_reset_enabled() const override;
  int64_t get_limit() const override;
  bool is_limit_enabled() const override;
  bool is_taking() const override;
  bool is_max_preludes_reached() const override;
  void postpone_break() override;
  void skip_break() override;

  void set_usage_mode(UsageMode mode);

  bool get_timer_activity_sensitive() const;

  void override(BreakId id);

  boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event() override;
  bool is_active() const override;
  int64_t get_total_overdue_time() const override;

private:
  void config_changed_notify(const std::string &key) override;

private:
  void init_defaults();

  void init_timer();
  void load_timer_config();

  void init_break_control();
  void load_break_control_config();

  bool starts_with(const std::string &key, std::string prefix, std::string &timer_name);
};

#endif // TIMERDATA_HH
