#pragma once

#include <cstdint>
#include <list>
#include <string>

// Mirrors the real class-nesting shape of ui/app/GenericDBusApplet.hh's
// TimerData/MenuItem structs and its
// `get_menu(std::list<MenuItem> &out)` method — the case that surfaced a
// real bug: Type::get_display_name() prints a class-nested type the way it
// appears from ITS OWN declaration's lexical scope (bare "TimerData"), which
// doesn't resolve in generated code living outside the class. See
// qualified_type_spelling() in clang_index.rs.
// @rpc(service="NestedService")
class RpcNestedFixture
{
public:
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

  // @rpc(name="SetTimerData")
  void set_timer_data(TimerData data);

  // @rpc(name="GetMenu")
  // @rpc.param(out, dir=out)
  void get_menu(std::list<MenuItem> &out);
};
