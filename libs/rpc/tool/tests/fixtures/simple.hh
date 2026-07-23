#pragma once

#include <cstdint>
#include <string>

#include <boost/signals2/signal.hpp>

// A C++23-only construct (if consteval, P1938), unrelated to any annotated
// declaration below — its presence proves the tool parses under real C++23,
// not just whatever subset the `clang` crate's `clang_10_0` binding-generation
// feature happens to name (see README.md: that feature only bounds which
// libclang *API functions* get Rust bindings, not the language standard the
// actual runtime-loaded libclang parses).
constexpr int cxx23_probe()
{
  if consteval
  {
    return 42;
  } else
  {
    return 0;
  }
}

enum class TestMode
{
  Idle,
  Active
};

// @rpc(service="TestService")
class RpcTestServer
{
public:
  // @rpc(name="Ping")
  std::string ping(std::string message);

  // @rpc(name="Add")
  int32_t add(int32_t a, int32_t b);

  // @rpc(name="SetFlag")
  void set_flag(bool value);

  bool get_flag() const;

  // @rpc(name="GetMode")
  // @rpc.param(mode, dir=out)
  bool get_mode(TestMode &mode);

  // A `const std::string &` in-parameter, distinct from Ping's by-value
  // `std::string` — the pointee of a reference type arrives from libclang
  // as "const std::string", not "std::string".
  // @rpc(name="Greet")
  std::string greet(const std::string &name);

  // A single-argument boost::signals2::signal accessor — the gRPC analog of
  // a DBus signal. No explicit fields= needed: defaults to a field named
  // "value" for the common single-argument case.
  // @rpc.signal(name="ModeChanged")
  boost::signals2::signal<void(TestMode)> &signal_mode_changed()
  {
    return signal_mode_changed_;
  }

  void fire_mode_changed_for_test(TestMode mode)
  {
    signal_mode_changed_(mode);
  }

private:
  bool flag_ = false;
  TestMode mode_ = TestMode::Idle;
  boost::signals2::signal<void(TestMode)> signal_mode_changed_;
};
