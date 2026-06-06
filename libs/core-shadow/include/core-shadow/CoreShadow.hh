// Copyright (C) 2026 Rob Caelers
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef WORKRAVE_CORE_SHADOW_HH
#define WORKRAVE_CORE_SHADOW_HH

#include <memory>
#include <string>

#include "config/IConfigurator.hh"
#include "core/ICore.hh"

namespace workrave::core_shadow
{
  class ICoreShadowDebug
  {
  public:
    virtual ~ICoreShadowDebug() = default;

    [[nodiscard]] virtual std::string get_shadow_debug_state_html() const = 0;
  };

  class CoreShadowFactory
  {
  public:
    static ICore::Ptr wrap(ICore::Ptr live_core, config::IConfigurator::Ptr configurator);
  };
} // namespace workrave::core_shadow

#endif // WORKRAVE_CORE_SHADOW_HH
