#pragma once

#include <cstdint>
#include <map>
#include <string>

// Mirrors dbusgen.py's <dictionary> support (std::map<K,V>, DBus signature
// "e{KV}") — the gRPC analog is protobuf's native `map<K, V>` type. Exercises
// a scalar-valued map and a struct-valued map (the value recurses through
// the same machinery as a sequence element / struct field).
struct MenuItem
{
  std::string text;
  uint32_t command;
};

// @rpc(service="MapTypesService")
class RpcMapTypesFixture
{
public:
  // @rpc(name="SetCounters")
  void set_counters(std::map<std::string, int32_t> counters);

  // @rpc(name="GetMenuByAction")
  // @rpc.param(out, dir=out)
  void get_menu_by_action(std::map<std::string, MenuItem> &out);
};
