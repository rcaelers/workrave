#include "CoreShadowClient.hh"

#include <cstdlib>

#include <spdlog/spdlog.h>

#include "utils/Paths.hh"

namespace workrave::core_shadow
{
  CoreShadowClient::CoreShadowClient()
    : process(create_core_shadow_process())
  {
  }

  CoreShadowClient::~CoreShadowClient()
  {
    stop();
  }

  auto
  CoreShadowClient::start() -> bool
  {
    if (started)
      {
        return true;
      }
    if (!process)
      {
        spdlog::warn("Core shadow helper is not supported on this platform");
        return false;
      }

    auto helper = find_helper();
    if (!helper)
      {
        spdlog::warn("Core shadow helper not found");
        return false;
      }

    started = process->start(*helper);
    if (!started)
      {
        spdlog::warn("Failed to start core shadow helper: {}", helper->string());
      }
    return started;
  }

  void
  CoreShadowClient::stop()
  {
    if (started && process)
      {
        ObservationBatch batch;
        process->exchange("quit", batch);
        process->stop();
        started = false;
      }
  }

  auto
  CoreShadowClient::command(const std::string &command, ObservationBatch &batch) -> bool
  {
    if (!started)
      {
        return false;
      }
    return process->exchange(command, batch);
  }

  auto
  CoreShadowClient::find_helper() const -> std::optional<std::filesystem::path>
  {
    if (auto *helper = std::getenv("WORKRAVE_CORE_SHADOW_HELPER"); helper != nullptr && *helper != '\0')
      {
        std::filesystem::path path{helper};
        if (std::filesystem::exists(path))
          {
            return path;
          }
      }

    const auto app_dir = workrave::utils::Paths::get_application_directory();
    for (const auto &candidate: {app_dir / "workrave-core-shadow-helper",
                                 app_dir / "MacOS" / "workrave-core-shadow-helper",
                                 std::filesystem::current_path() / "workrave-core-shadow-helper",
                                 std::filesystem::current_path() / "libs/core-shadow/src/workrave-core-shadow-helper"})
      {
        if (std::filesystem::exists(candidate))
          {
            return candidate;
          }
      }

    return std::nullopt;
  }
} // namespace workrave::core_shadow
