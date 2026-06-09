// Copyright (C) 2026 Rob Caelers <robc@krandor.org>
// All rights reserved.

#ifndef WORKRAVE_INPUT_MONITOR_FACTORY_STUB_HH
#define WORKRAVE_INPUT_MONITOR_FACTORY_STUB_HH

namespace workrave::input_monitor::test
{
  void fire_mouse(int x, int y, int wheel = 0);
  void fire_button(bool is_press);
  void fire_keyboard(bool repeat);
} // namespace workrave::input_monitor::test

#endif // WORKRAVE_INPUT_MONITOR_FACTORY_STUB_HH
