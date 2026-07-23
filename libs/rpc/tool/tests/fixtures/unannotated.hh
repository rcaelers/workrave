#pragma once

#include <cstdint>
#include <string>

// Deliberately carries zero @rpc tags anywhere in this file — proves the
// out-of-band annotations file (see external_annotations.rs) can expose a
// header nothing is allowed to touch (third-party, generated, ...).
namespace testutil
{
  class RpcUnannotatedFixture
  {
  public:
    std::string ping(std::string message);

    int32_t add(int32_t a, int32_t b);
  };
} // namespace testutil
