// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_WINDOWPREVIEW_HH
#define WORKRAVE_UI_WINDOWPREVIEW_HH

#include <cstdint>
#include <memory>
#include <string>

#include "core/IApp.hh"
#include "ui/IToolkit.hh"

enum class PreviewBreakType
{
  MicroBreak = 0,
  RestBreak,
  DailyLimit,
};

enum class PreviewWindowType
{
  Main = 0,
  Preferences,
  Statistics,
  Exercises,
  About,
  Debug,
};

enum class PreviewBlockMode
{
  Off = 0,
  Input,
  All,
};

class Context;

namespace rpc
{
  class RpcServer;
}

// Screenshot previews use isolated settings and a fixed display core.
// Mutations enqueue requests; process_pending runs on the UI thread.
// @rpc(service="workrave.WindowPreviewService")
class WindowPreview : public std::enable_shared_from_this<WindowPreview>
{
public:
  explicit WindowPreview(IToolkit::Ptr toolkit);
  ~WindowPreview();

  WindowPreview(const WindowPreview &) = delete;
  WindowPreview &operator=(const WindowPreview &) = delete;
  WindowPreview(WindowPreview &&) = delete;
  WindowPreview &operator=(WindowPreview &&) = delete;

  // @rpc(name="ShowBreak")
  uint64_t show_break(PreviewBreakType id, int progress, int duration, bool skippable, bool postponable);

  // @rpc(name="ShowPrelude")
  uint64_t show_prelude(PreviewBreakType id,
                        workrave::IApp::PreludeStage stage,
                        workrave::IApp::PreludeProgressText text,
                        int progress,
                        int duration);

  // @rpc(name="ShowWindow")
  uint64_t show_window(PreviewWindowType window, std::string page, std::string panel);

  // @rpc(name="ShowMenu")
  uint64_t show_menu(bool tray, std::string submenu, int x, int y);

  // @rpc(name="SetBackdrop")
  uint64_t set_backdrop(std::string color);

  // @rpc(name="SetTimer")
  uint64_t set_timer(PreviewBreakType id, int elapsed, int limit, int idle, int duration, bool enabled);

  // @rpc(name="SetAppearance")
  uint64_t set_appearance(PreviewBlockMode block_mode, bool user_active, int exercise_index, int exercise_elapsed);

  // @rpc(name="SetButtonLock")
  uint64_t set_button_lock(bool can_skip, bool can_postpone, int value, int maximum);

  // @rpc(name="SetModes")
  uint64_t set_modes(workrave::OperationMode operation_mode, workrave::UsageMode usage_mode);

  // @rpc(name="SetBoolSetting")
  uint64_t set_bool_setting(std::string key, bool value);

  // @rpc(name="SetIntSetting")
  uint64_t set_int_setting(std::string key, int value);

  // @rpc(name="SetStringSetting")
  uint64_t set_string_setting(std::string key, std::string value);

  // @rpc(name="GetLastError")
  std::string get_last_error() const;
  void initialize(std::shared_ptr<Context> context);

  // @rpc(name="Hide")
  uint64_t hide();

  // @rpc(name="Quit")
  uint64_t quit();

  // @rpc(name="GetAppliedSequence")
  uint64_t get_applied_sequence() const;

  // @rpc(name="WaitForAppliedSequence")
  std::string wait_for_applied_sequence(uint64_t sequence, uint64_t after_sequence) const;

  void register_service(::rpc::RpcServer &server);
  void process_pending();
  void close();

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};

#endif // WORKRAVE_UI_WINDOWPREVIEW_HH
