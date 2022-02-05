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

#include "core/CoreTypes.hh"
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ui/windows/WindowsFocusAssist.hh"
#include "ui/MenuModel.hh"
#include "ui/GUIConfig.hh"

#include <windows.h>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "nls.h"

WindowsFocusAssist::WindowsFocusAssist(std::shared_ptr<IApplication> app)
  : app(app)
{
  init();
}

void
WindowsFocusAssist::init()
{
  auto *ntdll = GetModuleHandleA("ntdll");
  rtlQueryWnfStateData = PRTLQUERYWNFSTATEDATA(GetProcAddress(ntdll, "RtlQueryWnfStateData"));
  rtlSubscribeWnfStateChangeNotification =
    PRTLSUBSCRIBEWNFSTATECHANGENOTIFICATION(GetProcAddress(ntdll, "RtlSubscribeWnfStateChangeNotification"));
  rtlUnsubscribeWnfStateChangeNotification =
    PRTLUNSUBSCRIBEWNFSTATECHANGENOTIFICATION(GetProcAddress(ntdll, "RtlUnsubscribeWnfStateChangeNotification"));

  if (rtlQueryWnfStateData == nullptr || rtlSubscribeWnfStateChangeNotification == nullptr
      || rtlUnsubscribeWnfStateChangeNotification == nullptr)
    {
      spdlog::error("Focus assist not available");
      return;
    }

  create_focus_mode_menu();

  GUIConfig::focus_mode().attach(tracker, [&](FocusMode mode) {
    focus_mode_group->select(static_cast<std::underlying_type_t<FocusMode>>(mode));

    if (mode == FocusMode::Off)
      {
        focus_mode_active = false;
        disable();
      }
    else
      {
        enable();
      }
    update_focus_assist();
  });
}

void
WindowsFocusAssist::enable()
{
  if (subscription == 0)
    {
      WNF_CHANGE_STAMP stamp = 0;
      NTSTATUS res = rtlQueryWnfStateData(&stamp, WNF_SHEL_QUIETHOURS_ACTIVE_PROFILE_CHANGED, wmf_notification_static, this, nullptr);

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
      switch (GUIConfig::focus_mode()())
        {
        case FocusMode::Off:
          requested_focus_operation_mode = workrave::OperationMode::Normal;
          break;
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

  if (requested_focus_operation_mode != focus_operation_mode)
    {
      focus_operation_mode = requested_focus_operation_mode;
      auto core = app->get_core();
      if (requested_focus_operation_mode == workrave::OperationMode::Normal)
        {
          spdlog::info("focus assist remove");
          core->remove_operation_mode_override(std::string{operation_mode_override_id});
        }
      else
        {
          spdlog::info("focus assist {}", focus_operation_mode);
          core->set_operation_mode_override(focus_operation_mode, std::string{operation_mode_override_id});
        }
    }
}

void
WindowsFocusAssist::create_focus_mode_menu()
{
  auto menu_model = app->get_menu_model();
  auto section = menu_model->find_section("workrave.section.modes");

  auto modemenu = menus::SubMenuNode::create(FOCUS_MODE_MENU, _("_Follow Focus Assist"));
  section->add_after(modemenu, "workrave.mode_menu");

  focus_mode_group = menus::RadioGroupNode::create(FOCUS_MODE, "");
  modemenu->add(focus_mode_group);

  focus_mode_off_item = menus::RadioNode::create(focus_mode_group,
                                                 FOCUS_MODE_OFF,
                                                 _("_Off"),
                                                 static_cast<std::underlying_type_t<FocusMode>>(FocusMode::Off),
                                                 [] { GUIConfig::focus_mode().set(FocusMode::Off); });
  focus_mode_group->add(focus_mode_off_item);

  focus_mode_suspended_item = menus::RadioNode::create(focus_mode_group,
                                                       FOCUS_MODE_SUSPENDED,
                                                       _("_Suspended"),
                                                       static_cast<std::underlying_type_t<FocusMode>>(FocusMode::Suspended),
                                                       [] { GUIConfig::focus_mode().set(FocusMode::Suspended); });
  focus_mode_group->add(focus_mode_suspended_item);

  focus_mode_quiet_item = menus::RadioNode::create(focus_mode_group,
                                                   FOCUS_MODE_QUIET,
                                                   _("Q_uiet"),
                                                   static_cast<std::underlying_type_t<FocusMode>>(FocusMode::Quiet),
                                                   [] { GUIConfig::focus_mode().set(FocusMode::Quiet); });
  focus_mode_group->add(focus_mode_quiet_item);
  focus_mode_group->select(static_cast<std::underlying_type_t<FocusMode>>(GUIConfig::focus_mode()()));

  menu_model->update();
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
