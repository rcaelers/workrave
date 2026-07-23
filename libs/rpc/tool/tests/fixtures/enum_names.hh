#pragma once

// @rpc.enum(name="operation_mode")
enum class OperationMode
{
  // @rpc.enum.value(name="normal")
  Normal,
  // @rpc.enum.value(name="suspended")
  Suspended,
  // No tag on this one — proves canonical_name is per-value optional, not
  // all-or-nothing for the enum.
  Quiet,
};

// @rpc(service="EnumNamesService")
class RpcEnumNamesFixture
{
public:
  // @rpc(name="SetOperationMode")
  void set_operation_mode(OperationMode mode);
};
