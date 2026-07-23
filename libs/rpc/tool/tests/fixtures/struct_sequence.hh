#pragma once

#include <cstdint>
#include <list>
#include <string>
#include <vector>

#include <boost/signals2/signal.hpp>

// Exercises struct-as-param/return/signal-field, sequence-of-struct,
// sequence-of-scalar, and a nested `using` alias to a sequence-of-struct
// (`MenuItems`) — the pattern set ui/app/workrave-gui.xml needs (TimerData/
// MenuItem/MenuItems/TimersUpdated) but at file scope, not class-nested
// (see nested_struct.hh for that variant).
struct TimerData
{
  std::string bar_text;
  int slot;
  uint32_t bar_primary_val;
};

struct MenuItem
{
  std::string text;
  uint32_t command;
  uint8_t flags;
};

// @rpc(service="StructSeqService")
class RpcStructSeqFixture
{
public:
  using MenuItems = std::list<MenuItem>;

  // @rpc(name="SetTimerData")
  void set_timer_data(TimerData data);

  // @rpc(name="GetTimerData")
  TimerData get_timer_data();

  // @rpc(name="GetMenu")
  // @rpc.param(out, dir=out)
  void get_menu(MenuItems &out);

  // @rpc(name="SetTags")
  void set_tags(std::vector<int32_t> tags);

  // @rpc.signal(name="TimerUpdated")
  boost::signals2::signal<void(TimerData)> &signal_timer_updated()
  {
    return signal_timer_updated_;
  }

private:
  boost::signals2::signal<void(TimerData)> signal_timer_updated_;
};
