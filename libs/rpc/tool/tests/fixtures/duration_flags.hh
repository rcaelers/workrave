#pragma once

#include <chrono>

// Deliberately mirrors just the RPC-relevant shape of
// libs/utils/include/utils/Enum.hh's Flags<Enum> (constructible from a
// single Enum value, `|=`-combinable, gated on an `@rpc.bitmask`-tagged
// primary template) — this fixture stays independent of the wider Workrave
// tree, so it re-derives that shape locally instead of including the real
// header. The namespace is deliberately NOT workrave::utils, to prove the
// tool's Flags<> detection isn't hardcoded to one namespace (see
// clang_index.rs::flags_enum_type).
namespace testutil
{
  enum class Perm
  {
    Read,
    Write,
    Execute,
  };

  // @rpc.bitmask
  template<typename Enum>
  class Flags
  {
  public:
    Flags() = default;

    constexpr Flags(Enum e) noexcept
      : value{static_cast<int>(e)}
    {
    }

    constexpr Flags &operator|=(Enum e) noexcept
    {
      value |= static_cast<int>(e);
      return *this;
    }

    [[nodiscard]] constexpr int get() const noexcept
    {
      return value;
    }

  private:
    int value{0};
  };
} // namespace testutil

// @rpc(service="DurationFlagsService")
class RpcDurationFlagsFixture
{
public:
  // A plain std::chrono::duration<> parameter needs no annotation at all —
  // detected structurally by type shape (any Rep/Period).
  // @rpc(name="SetTimeout")
  void set_timeout(std::chrono::minutes duration);

  // A Flags<Enum> parameter, recognized via the @rpc.bitmask tag on the
  // primary Flags template above rather than anything at this call site.
  // @rpc(name="SetPermissions")
  void set_permissions(testutil::Flags<testutil::Perm> perms);
};
