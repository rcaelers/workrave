// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#ifndef UI_PREFWIDGETS_QTQUICK_BUILDER_HH
#define UI_PREFWIDGETS_QTQUICK_BUILDER_HH

#include "ui/prefwidgets/Def.hh"
#include "Models.hh"

namespace ui::prefwidgets::qtquick
{
  class Builder
  {
  public:
    // Call before loading standalone QML controls. build() also initializes
    // the module automatically.
    static void initialize();
    static PageModel *build(const std::shared_ptr<Def> &def, QObject *parent = nullptr);
    static void append(const std::shared_ptr<Def> &def, PageModel *page);
  };
} // namespace ui::prefwidgets::qtquick

#endif
