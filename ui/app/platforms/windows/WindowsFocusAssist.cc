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

#include "WindowsFocusAssist.hh"

#include "core/CoreTypes.hh"
#include "ui/GUIConfig.hh"

#include <windows.h>

#include <utility>
#include "spdlog/spdlog.h"
#include "commonui/nls.h"
#include "debug.hh"

WindowsFocusAssist::WindowsFocusAssist(std::shared_ptr<IPluginContext> context)
  : context(std::move(context))
{
  TRACE_ENTRY();
  init();
}

WindowsFocusAssist::~WindowsFocusAssist()
{
  TRACE_ENTRY();
}

void
WindowsFocusAssist::init()
{
  auto *ntdll = GetModuleHandleA("ntdll");
  rtlQueryWnfStateData = PRTLQUERYWNFSTATEDATA(GetProcAddress(ntdll, "RtlQueryWnfStateData"));
  rtlSubscribeWnfStateChangeNotification = PRTLSUBSCRIBEWNFSTATECHANGENOTIFICATION(
    GetProcAddress(ntdll, "RtlSubscribeWnfStateChangeNotification"));
  rtlUnsubscribeWnfStateChangeNotification = PRTLUNSUBSCRIBEWNFSTATECHANGENOTIFICATION(
    GetProcAddress(ntdll, "RtlUnsubscribeWnfStateChangeNotification"));

  if (rtlQueryWnfStateData == nullptr || rtlSubscribeWnfStateChangeNotification == nullptr
      || rtlUnsubscribeWnfStateChangeNotification == nullptr)
    {
      spdlog::error("Focus assist not available");
      return;
    }

  GUIConfig::follow_focus_assist_enabled().attach(tracker, [&](bool enabled) {
    if (enabled)
      {
        enable();
      }
    else
      {
        focus_mode_active = false;
        disable();
      }
    update_focus_assist();
  });

  GUIConfig::focus_mode().attach(tracker, [&](FocusMode mode) { update_focus_assist(); });

  std::vector<std::string> focus_content{N_("Suspended"), N_("Quiet")};

  focus_def = ui::prefwidgets::PanelDef::create("monitoring", "focus-assist", N_("Focus Assist"))
              << (ui::prefwidgets::Frame::create(N_("Focus Assist"))
                  << ui::prefwidgets::Toggle::create(N_("Set operation mode based on Windows Focus Assist"))
                       ->connect(&GUIConfig::follow_focus_assist_enabled())
                  << ui::prefwidgets::Choice::create(N_("Operation mode during Focus Assist:"))
                       ->connect(&GUIConfig::focus_mode())
                       ->assign(focus_content)
                       ->when(&GUIConfig::follow_focus_assist_enabled()));

  context->get_preferences_registry()->add(focus_def);
}

void
WindowsFocusAssist::enable()
{
  if (subscription == 0)
    {
      WNF_CHANGE_STAMP stamp = 0;
      NTSTATUS res = rtlQueryWnfStateData(&stamp,
                                          WNF_SHEL_QUIETHOURS_ACTIVE_PROFILE_CHANGED,
                                          wmf_notification_static,
                                          this,
                                          nullptr);

      if (res == 0)
        {
          res = rtlSubscribeWnfStateChangeNotification(&subscription,
                                                       WNF_SHEL_QUIETHOURS_ACTIVE_PROFILE_CHANGED,
                                                       stamp,
                                                       wmf_notification_static,
                                                       this,
                                                       nullptr,
                                                       0,
                                                       1);
        }
    }
}

void
WindowsFocusAssist::disable()
{
  if (subscription != 0)
    {
      rtlUnsubscribeWnfStateChangeNotification(subscription);
      subscription = 0;
    }
}

void
WindowsFocusAssist::on_focus_assist_changed(int32_t focus_assist_mode)
{
  bool focus = focus_assist_mode > 0;

  if (focus_mode_active != focus)
    {
      focus_mode_active = focus;
      update_focus_assist();
    }
}

void
WindowsFocusAssist::update_focus_assist()
{
  workrave::OperationMode requested_focus_operation_mode = workrave::OperationMode::Normal;

  if (focus_mode_active)
    {
      if (GUIConfig::follow_focus_assist_enabled()())
        {
          switch (GUIConfig::focus_mode()())
            {
            case FocusMode::Suspended:
              requested_focus_operation_mode = workrave::OperationMode::Suspended;
              break;
            case FocusMode::Quiet:
              requested_focus_operation_mode = workrave::OperationMode::Quiet;
              break;
            }
        }
      else
        {
          requested_focus_operation_mode = workrave::OperationMode::Normal;
        }
    }
  else
    {
      requested_focus_operation_mode = workrave::OperationMode::Normal;
    }

  if (requested_focus_operation_mode != focus_operation_mode)
    {
      spdlog::info("Focus assist changed operation mode to {}", requested_focus_operation_mode);
      focus_operation_mode = requested_focus_operation_mode;
      auto core = context->get_core();
      if (requested_focus_operation_mode == workrave::OperationMode::Normal)
        {
          core->remove_operation_mode_override(std::string{operation_mode_override_id});
        }
      else
        {
          core->set_operation_mode_override(focus_operation_mode, std::string{operation_mode_override_id});
        }
    }
}

NTSTATUS NTAPI
WindowsFocusAssist::wmf_notification_static(_In_ WNF_STATE_NAME state_name,
                                            _In_ WNF_CHANGE_STAMP change_stamp,
                                            _In_opt_ PWNF_TYPE_ID type_id,
                                            _In_opt_ PVOID context,
                                            _In_ const VOID *buffer,
                                            _In_ ULONG bufferSize)
{
  auto *self = static_cast<WindowsFocusAssist *>(context);

  if (state_name == WNF_SHEL_QUIETHOURS_ACTIVE_PROFILE_CHANGED && bufferSize == sizeof(int32_t))
    {
      int32_t focus_assist_mode = *static_cast<const int32_t *>(buffer);
      self->on_focus_assist_changed(focus_assist_mode);
    }

  return 0;
}
