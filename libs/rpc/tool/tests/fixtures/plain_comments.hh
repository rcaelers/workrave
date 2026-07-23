#pragma once

#include <cstdint>

// Proves @rpc tags are recognized from plain, non-documentation comments —
// `//!`/`///`/`/** */` are Doxygen's "this is documentation" markers, and
// the tool must not require annotated declarations to look like public API
// docs. This fixture uses only plain `//`, on purpose.
// @rpc(service="PlainService")
class RpcPlainCommentFixture
{
public:
  // @rpc(name="GetValue")
  int32_t get_value();
};
