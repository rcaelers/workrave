// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#ifndef MACDOCKTILE_HH
#define MACDOCKTILE_HH

#include <array>
#include <memory>

#include "core/CoreTypes.hh"
#include "ui/IApplicationContext.hh"
#include "ui/ITimerBoxView.hh"
#include "ui/TimerBoxControl.hh"
#include "ui/UiTypes.hh"

struct DockBarData
{
  double remaining{0.0}; // 1=just reset, 0=break due
  bool overdue{false};
};

class MacDockTilePrivate;

class MacDockTile : public ITimerBoxView
{
public:
  explicit MacDockTile(std::shared_ptr<IApplicationContext> app);
  ~MacDockTile() override;

  void tick();

  // ITimerBoxView
  void set_slot(workrave::BreakId id, int slot) override {}
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;
  void set_icon(OperationModeIcon icon) override {}
  void update_view() override;

  const std::array<DockBarData, workrave::BREAK_ID_SIZEOF> &bars() const
  {
    return bars_;
  }

private:
  std::array<DockBarData, workrave::BREAK_ID_SIZEOF> bars_{};
  std::unique_ptr<TimerBoxControl> control_;
  MacDockTilePrivate *priv_{nullptr};
};

#endif // MACDOCKTILE_HH
