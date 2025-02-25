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

#ifndef WINDOWSFOCUSASSIST_HH
#define WINDOWSFOCUSASSIST_HH

#include "config.h"

#include <string>

#include <windows.h>
#include <commctrl.h>

#include "core/CoreTypes.hh"
#include "utils/Signals.hh"
#include "ui/Plugin.hh"

#include "wmf.h"

#include "ui/prefwidgets/Widgets.hh"

class WindowsFocusAssist : public Plugin<WindowsFocusAssist>
{
public:
  WindowsFocusAssist() = delete;
  ~WindowsFocusAssist() override;
  explicit WindowsFocusAssist(std::shared_ptr<IPluginContext> context);

  std::string get_plugin_id() const override
  {
    return "workrave.WindowsFocusAssist";
  }

private:
  void init();
  void enable();
  void disable();

  void on_focus_assist_changed(int32_t focus_assist_mode);
  void update_focus_assist();

  void create_focus_mode_menu();

  static NTSTATUS NTAPI wmf_notification_static(_In_ WNF_STATE_NAME state_name,
                                                _In_ WNF_CHANGE_STAMP change_stamp,
                                                _In_opt_ PWNF_TYPE_ID type_id,
                                                _In_opt_ PVOID context,
                                                _In_ const VOID *buffer,
                                                _In_ ULONG bufferSize);

private:
  std::shared_ptr<IPluginContext> context;

  PRTLQUERYWNFSTATEDATA rtlQueryWnfStateData{nullptr};
  PRTLSUBSCRIBEWNFSTATECHANGENOTIFICATION rtlSubscribeWnfStateChangeNotification{nullptr};
  PRTLUNSUBSCRIBEWNFSTATECHANGENOTIFICATION rtlUnsubscribeWnfStateChangeNotification{nullptr};

  DWORD64 subscription{0};
  workrave::OperationMode focus_operation_mode{workrave::OperationMode::Normal};
  bool focus_mode_active{false};
  std::shared_ptr<ui::prefwidgets::Def> focus_def;
  workrave::utils::Trackable tracker;

  static constexpr WNF_STATE_NAME WNF_SHEL_QUIETHOURS_ACTIVE_PROFILE_CHANGED{0xA3BF1C75, 0xD83063E};
  static constexpr std::string_view operation_mode_override_id{"focus"};
};

#endif // WINDOWSFOCUSASSIST_HH
