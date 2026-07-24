#ifndef WORKRAVE_CORE_SHADOW_CLIENT_HH
#define WORKRAVE_CORE_SHADOW_CLIENT_HH

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "CoreShadowTypes.hh"

namespace workrave::core_shadow
{
  class CoreShadowProcess
  {
  public:
    virtual ~CoreShadowProcess() = default;

    virtual auto start(const std::filesystem::path &helper) -> bool = 0;
    virtual void stop() = 0;
    virtual auto exchange(const std::string &command, ObservationBatch &batch) -> bool = 0;
  };

  auto create_core_shadow_process() -> std::unique_ptr<CoreShadowProcess>;

  class CoreShadowClient
  {
  public:
    CoreShadowClient();
    ~CoreShadowClient();

    auto start() -> bool;
    void stop();

    auto command(const std::string &command, ObservationBatch &batch) -> bool;

  private:
    auto find_helper() const -> std::optional<std::filesystem::path>;

  private:
    std::unique_ptr<CoreShadowProcess> process;
    bool started{false};
  };
} // namespace workrave::core_shadow

#endif // WORKRAVE_CORE_SHADOW_CLIENT_HH
