#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Exercises the DBus backend's struct/sequence path: a struct as an
// in-parameter and a return value, a sequence of scalar, and a sequence of
// struct as a return value.
struct Point
{
  int32_t x;
  int32_t y;
};

// @rpc(service="DBusFixture2Service")
// @rpc.dbus(interface="org.workrave.TestInterface2")
class RpcDBusFixture2
{
public:
  // @rpc(name="SetPoint")
  void set_point(Point p);

  // @rpc(name="GetPoint")
  Point get_point();

  // @rpc(name="SetTags")
  void set_tags(std::vector<int32_t> tags);

  // @rpc(name="GetPoints")
  std::vector<Point> get_points();
};
