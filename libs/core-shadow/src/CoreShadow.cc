#include "core-shadow/CoreShadow.hh"

#include "CoreShadowProxy.hh"

namespace workrave::core_shadow
{
  auto
  CoreShadowFactory::wrap(ICore::Ptr live_core, config::IConfigurator::Ptr configurator) -> ICore::Ptr
  {
    return std::make_shared<CoreShadowProxy>(std::move(live_core), std::move(configurator));
  }
} // namespace workrave::core_shadow
