// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#include "ui/prefwidgets/qtquick/Builder.hh"
#include "ui/prefwidgets/Box.hh"
#include "ui/prefwidgets/Frame.hh"

#include <QResource>
#include <spdlog/spdlog.h>

static void
initialize_resources()
{
  Q_INIT_RESOURCE(prefwidgets_quick);
}

using namespace ui::prefwidgets;
using namespace ui::prefwidgets::qtquick;

namespace
{
  RowModel *build_row(const std::shared_ptr<Widget> &widget, QObject *parent)
  {
    if (auto def = std::dynamic_pointer_cast<Toggle>(widget); def != nullptr)
      {
        return new RowModel(def, parent);
      }
    if (auto def = std::dynamic_pointer_cast<Time>(widget); def != nullptr)
      {
        return new RowModel(def, parent);
      }
    if (auto def = std::dynamic_pointer_cast<Value>(widget); def != nullptr)
      {
        return new RowModel(def, parent);
      }
    if (auto def = std::dynamic_pointer_cast<Choice>(widget); def != nullptr)
      {
        return new RowModel(def, parent);
      }
    if (auto def = std::dynamic_pointer_cast<Entry>(widget); def != nullptr)
      {
        return new RowModel(def, parent);
      }
    spdlog::warn("Cannot render unsupported preference widget");
    return nullptr;
  }

  void build_widget(const std::shared_ptr<Widget> &widget, PageModel *page, GroupModel *group = nullptr)
  {
    if (widget == nullptr)
      {
        return;
      }
    if (auto frame = std::dynamic_pointer_cast<Frame>(widget); frame != nullptr)
      {
        const auto presentation = frame->get_presentation();
        const auto label = presentation.label.empty() ? frame->get_label() : presentation.label;
        auto *child_group = new GroupModel(QString::fromStdString(label), page);
        page->add_group(child_group);
        for (const auto &child: frame->get_content())
          {
            build_widget(child, page, child_group);
          }
      }
    else if (auto box = std::dynamic_pointer_cast<Box>(widget); box != nullptr)
      {
        for (const auto &child: box->get_content())
          {
            build_widget(child, page, group);
          }
      }
    else
      {
        auto *row = build_row(widget, group != nullptr ? static_cast<QObject *>(group) : page);
        if (row == nullptr)
          {
            return;
          }
        if (group == nullptr)
          {
            group = new GroupModel(QString{}, page);
            page->add_group(group);
            row->setParent(group);
          }
        group->add_row(row);
      }
  }
} // namespace

void
Builder::initialize()
{
  static const bool initialized = [] {
    initialize_resources();
    return true;
  }();
  (void)initialized;
}

void
Builder::append(const std::shared_ptr<Def> &def, PageModel *page)
{
  initialize();
  if (def == nullptr || page == nullptr)
    {
      spdlog::warn("Cannot append an empty preference definition or page");
      return;
    }
  build_widget(def->get_widget(), page);
}

PageModel *
Builder::build(const std::shared_ptr<Def> &def, QObject *parent)
{
  initialize();
  if (def == nullptr)
    {
      spdlog::warn("Cannot build an empty preference definition");
      return nullptr;
    }
  QString label;
  if (auto panel = std::dynamic_pointer_cast<PanelDef>(def); panel != nullptr)
    {
      label = QString::fromStdString(panel->get_label());
    }
  auto *page = new PageModel(label, parent);
  append(def, page);
  return page;
}
