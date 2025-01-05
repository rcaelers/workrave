// Copyright (C) 2013 - 2021 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef TOOLKITMENU_HH
#define TOOLKITMENU_HH

#include "commonui/MenuModel.hh"

#include <boost/signals2.hpp>
#include <memory>

#include <QMenu>

#include "utils/Signals.hh"

using MenuNodeFilter = std::function<bool(menus::Node::Ptr)>;

namespace detail
{
  class ToolkitSubMenuEntry;

  class ToolkitMenuContext
  {
  public:
    using Ptr = std::shared_ptr<ToolkitMenuContext>;

    explicit ToolkitMenuContext(MenuNodeFilter filter);

    [[nodiscard]] auto get_filter() const -> MenuNodeFilter;

  private:
    MenuNodeFilter filter;
  };

  class ToolkitMenuEntry
    : public QObject
    , public workrave::utils::Trackable
  {
    Q_OBJECT

  public:
    using Ptr = std::shared_ptr<ToolkitMenuEntry>;

    explicit ToolkitMenuEntry(ToolkitMenuContext::Ptr context);

    [[nodiscard]] auto get_context() const -> ToolkitMenuContext::Ptr;
    [[nodiscard]] virtual auto get_action() const -> QAction * = 0;

  private:
    ToolkitMenuContext::Ptr context;
  };

  class ToolkitSubMenuEntry : public ToolkitMenuEntry
  {
    Q_OBJECT

  public:
    using Ptr = std::shared_ptr<ToolkitSubMenuEntry>;

    ToolkitSubMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::SubMenuNode::Ptr node);

    [[nodiscard]] auto get_menu() const -> QMenu *;
    [[nodiscard]] auto get_action() const -> QAction * override;
    void init();

  private:
    ToolkitSubMenuEntry *parent;
    menus::SubMenuNode::Ptr node;
    std::list<ToolkitMenuEntry::Ptr> children;
    QMenu *menu{nullptr};
  };

  class ToolkitActionMenuEntry : public ToolkitMenuEntry
  {
    Q_OBJECT

  public:
    ToolkitActionMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::ActionNode::Ptr node);

    [[nodiscard]] auto get_action() const -> QAction * override;

  private:
    QAction *action{nullptr};
  };

  class ToolkitToggleMenuEntry : public ToolkitMenuEntry
  {
    Q_OBJECT

  public:
    ToolkitToggleMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::ToggleNode::Ptr node);

    [[nodiscard]] auto get_action() const -> QAction * override;

  private:
    QAction *action{nullptr};
  };

  class ToolkitRadioMenuEntry : public ToolkitMenuEntry
  {
    Q_OBJECT

  public:
    ToolkitRadioMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::RadioNode::Ptr node);

    [[nodiscard]] auto get_action() const -> QAction * override;

  private:
    QAction *action{nullptr};
  };

  class ToolkitRadioGroupMenuEntry : public ToolkitMenuEntry
  {
    Q_OBJECT

  public:
    ToolkitRadioGroupMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::RadioGroupNode::Ptr node);

    [[nodiscard]] auto get_action() const -> QAction * override;

  private:
    std::list<ToolkitRadioMenuEntry::Ptr> children;
    QMenu *menu{nullptr};
  };

  class ToolkitSeparatorMenuEntry : public ToolkitMenuEntry
  {
    Q_OBJECT

  public:
    ToolkitSeparatorMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::SeparatorNode::Ptr node);

    [[nodiscard]] auto get_action() const -> QAction * override;

  private:
    QAction *action{nullptr};
  };

  class ToolkitSectionMenuEntry : public ToolkitMenuEntry
  {
    Q_OBJECT

  public:
    ToolkitSectionMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::SectionNode::Ptr node);

    [[nodiscard]] auto get_action() const -> QAction * override;

  private:
    std::list<ToolkitMenuEntry::Ptr> children;
    QMenu *menu{nullptr};
  };

  class ToolkitMenuEntryFactory
  {
  public:
    static auto create(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::Node::Ptr node)
      -> ToolkitMenuEntry::Ptr;
  };

} // namespace detail

class ToolkitMenu
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  using Ptr = std::shared_ptr<ToolkitMenu>;

  explicit ToolkitMenu(MenuModel::Ptr menu_model, MenuNodeFilter filter = nullptr);

  [[nodiscard]] auto get_menu() const -> QMenu *;

private:
  std::shared_ptr<detail::ToolkitMenuContext> context;
  detail::ToolkitSubMenuEntry::Ptr entry;
};

#endif // TOOLKITMENU_HH
