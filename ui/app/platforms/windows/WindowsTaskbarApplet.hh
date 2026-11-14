// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#ifndef WORKRAVE_WINDOWS_TASKBAR_APPLET_HH
#define WORKRAVE_WINDOWS_TASKBAR_APPLET_HH

#include <array>
#include <memory>
#include <vector>
#include <windows.h>
#include <boost/signals2/connection.hpp>

#include "ui/Plugin.hh"
#include "ui/ITimerBoxView.hh"
#include "ui/TimerBoxControl.hh"
#include "utils/Signals.hh"

class TaskbarLayout;

class WindowsTaskbarApplet
  : public Plugin<WindowsTaskbarApplet>
  , public ITimerBoxView
  , public workrave::utils::Trackable
{
public:
  explicit WindowsTaskbarApplet(std::shared_ptr<IPluginContext> context);
  ~WindowsTaskbarApplet() override;
  WindowsTaskbarApplet(const WindowsTaskbarApplet &) = delete;
  WindowsTaskbarApplet &operator=(const WindowsTaskbarApplet &) = delete;
  WindowsTaskbarApplet(WindowsTaskbarApplet &&) = delete;
  WindowsTaskbarApplet &operator=(WindowsTaskbarApplet &&) = delete;

  std::string get_plugin_id() const override
  {
    return "workrave.WindowsTaskbarApplet";
  }

  void set_slot(workrave::BreakId id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;
  void set_icon(OperationModeIcon icon) override;
  void update_view() override;

private:
  struct Timer
  {
    int value{0};
    int primary{0};
    int primary_max{0};
    int secondary{0};
    int secondary_max{0};
    TimerColorId color{TimerColorId::Inactive};
    TimerColorId secondary_color{TimerColorId::Inactive};
  };

  struct Palette
  {
    bool high_contrast{false};
    COLORREF background{0};
    COLORREF foreground{0};
    COLORREF fill_foreground{0};
    COLORREF frame{0};
  };

  void init_preferences();
  void load_icons();
  void tick();
  void reposition();
  void sync_hold();
  bool attach(HWND taskbar);
  void detach();
  void paint(HDC dc, RECT rect);
  void paint_timer(HDC dc, RECT cell, workrave::BreakId id, const Palette &palette);
  void append_menu(HMENU menu, const menus::Node::Ptr &node, std::vector<menus::Node::Ptr> &commands);
  void show_menu(POINT point);
  void update_tooltip();
  static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);

  std::shared_ptr<IPluginContext> context;
  std::shared_ptr<ui::prefwidgets::PanelDef> preferences;
  std::unique_ptr<TimerBoxControl> control;
  std::unique_ptr<TaskbarLayout> layout;
  boost::signals2::scoped_connection timer_connection;
  HWND observer{nullptr};
  HWND window{nullptr};
  HWND tooltip{nullptr};
  HFONT font{nullptr};
  int font_height{0};
  std::array<HICON, workrave::BREAK_ID_SIZEOF> icons{};
  UINT dpi{0};
  bool held{false};
  OperationModeIcon mode{OperationModeIcon::Normal};
  std::array<workrave::BreakId, workrave::BREAK_ID_SIZEOF> timer_slots{};
  std::array<Timer, workrave::BREAK_ID_SIZEOF> timers{};
  std::wstring tooltip_text;
};

#endif
