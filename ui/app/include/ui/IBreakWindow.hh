// Copyright (C) 2001 - 2024 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_IBREAKWINDOW_HH
#define WORKRAVE_UI_IBREAKWINDOW_HH

#include <memory>
#include "core/CoreTypes.hh"

// App-computed state for the Postpone/Skip buttons.  The app layer owns
// the logic; each toolkit's break window just renders whatever it receives.
struct BreakButtonState
{
  bool can_postpone{true};
  bool can_skip{true};

  // The lower-priority break that is currently locking the buttons.
  // BREAK_ID_NONE means no lock is active.
  workrave::BreakId overdue_break_id{workrave::BREAK_ID_NONE};

  // auto-reset window length and how far along the user's idle time is
  // (used to drive the "unlock progress" bar in the UI).
  int64_t lock_max{0};
  int64_t lock_value{0};

  double lock_progress() const
  {
    if (lock_max <= 0)
      {
        return 0.0;
      }
    return static_cast<double>(lock_value) / static_cast<double>(lock_max);
  }

  bool operator==(const BreakButtonState &o) const
  {
    return can_postpone == o.can_postpone && can_skip == o.can_skip && overdue_break_id == o.overdue_break_id
           && lock_max == o.lock_max && lock_value == o.lock_value;
  }
};

class IBreakWindow
{
public:
  using Ptr = std::shared_ptr<IBreakWindow>;
  virtual ~IBreakWindow() = default;

  //! Initializes the break window.
  virtual void init() = 0;

  //! Starts (i.e. shows) the break window.
  virtual void start() = 0;

  //! Stops (i.e. hides) the break window.
  virtual void stop() = 0;

  //! Refreshes the content of the break window.
  virtual void refresh() = 0;

  //! Sets the progress to the specified value and maximum value.
  virtual void set_progress(int value, int max_value) = 0;

  //! Delivers the app-computed button state. Default no-op so GTK windows
  //! are unaffected and keep driving their own logic.
  virtual void set_break_button_state(const BreakButtonState & /*state*/) {}
};

#endif // WORKRAVE_UI_IBREAKWINDOW_HH
