#pragma once

// Deliberately carries no @rpc.enum/@rpc.enum.value tags — those come from
// a separate annotations file in this fixture's test, proving out-of-band
// enum-name pinning works, not just the in-source form (see enum_names.hh).
enum class OperationMode
{
  Normal,
  Suspended,
  Quiet,
};

// @rpc(service="EnumNamesService")
class RpcEnumNamesFixture
{
public:
  // @rpc(name="SetOperationMode")
  void set_operation_mode(OperationMode mode);
};
