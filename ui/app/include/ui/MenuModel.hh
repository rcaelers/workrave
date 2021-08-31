// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_MENUMODEL_HH
#define WORKRAVE_UI_MENUMODEL_HH

#include <string>
#include <list>
#include <functional>
#include <memory>
#include <boost/signals2.hpp>

namespace menus
{
  class Node
  {
  public:
    using Ptr = std::shared_ptr<Node>;
    using Activated = std::function<void()>;

    Node();
    Node(std::string_view id, std::string text, Activated activated = Activated());
    virtual ~Node() = default;

    [[nodiscard]] auto get_text() const -> std::string;
    [[nodiscard]] auto get_text_no_accel() const -> std::string;
    [[nodiscard]] auto get_id() const -> std::string;
    [[nodiscard]] auto is_visible() const -> bool;

    void set_text(std::string text);
    void set_visible(bool visible);

    auto signal_changed() -> boost::signals2::signal<void()> &;

    virtual void activate();

  protected:
    const std::string id;
    std::string text;
    bool visible{true};
    Activated activated;
    boost::signals2::signal<void()> changed_signal;
  };

  class SubMenuNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<SubMenuNode>;

    SubMenuNode() = default;
    SubMenuNode(std::string_view id, std::string text);

    static auto create(std::string_view id, std::string text) -> Ptr;

    void add(menus::Node::Ptr submenu, menus::Node::Ptr before = menus::Node::Ptr());
    void remove(menus::Node::Ptr submenu);

    [[nodiscard]] auto get_children() const -> std::list<Node::Ptr>;

  private:
    std::list<Node::Ptr> children;
  };

  class ActionNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<ActionNode>;

    ActionNode(std::string_view id, std::string text, Activated activated);

    static auto create(std::string_view id, std::string text, Activated activated) -> Ptr;
  };

  class ToggleNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<ToggleNode>;

    ToggleNode(std::string_view id, std::string text, Activated activated);

    static auto create(std::string_view id, std::string text, Activated activated) -> Ptr;

    [[nodiscard]] auto is_checked() const -> bool;
    void set_checked(bool checked);

    void activate() override;
    void activate(bool state);

  private:
    bool checked{false};
  };

  class RadioGroupNode;

  class RadioNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<RadioNode>;

    RadioNode(std::shared_ptr<RadioGroupNode> group, std::string_view id, std::string text, int value, Activated activated);

    static auto create(std::shared_ptr<RadioGroupNode> group, std::string_view id, std::string text, int value, Activated activated) -> Ptr;

    [[nodiscard]] auto get_value() const -> int;
    void set_value(int value);

    [[nodiscard]] auto is_checked() const -> bool;
    void set_checked(bool checked);

    [[nodiscard]] auto get_group_id() const -> std::string;

    void activate() override;

  private:
    int value{0};
    bool checked{false};
    std::weak_ptr<RadioGroupNode> group;
  };

  class RadioGroupNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<RadioGroupNode>;

    RadioGroupNode(std::string_view id, std::string text, Activated activated = Activated());

    static auto create(std::string_view id, std::string text, Activated activated = Activated()) -> Ptr;

    void add(menus::RadioNode::Ptr submenu, menus::RadioNode::Ptr before = menus::RadioNode::Ptr());
    void remove(menus::RadioNode::Ptr submenu);
    [[nodiscard]] auto get_children() const -> std::list<RadioNode::Ptr>;

    void select(std::string id);
    void select(int value);

    [[nodiscard]] auto get_selected_node() const -> menus::RadioNode::Ptr;
    [[nodiscard]] auto get_selected_value() const -> int;
    [[nodiscard]] auto get_selected_id() const -> std::string;

    void activate() override;
    void activate(int value);

  private:
    std::list<RadioNode::Ptr> children;
    std::string selected_id;
  };

  class SeparatorNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<SeparatorNode>;
    static auto create() -> Ptr;
  };
} // namespace menus

class MenuModel
{
public:
  using Ptr = std::shared_ptr<MenuModel>;

  MenuModel();

  [[nodiscard]] auto get_root() const -> menus::SubMenuNode::Ptr;
  void update();

  auto signal_update() -> boost::signals2::signal<void()> &;

private:
  std::shared_ptr<menus::SubMenuNode> root;
  boost::signals2::signal<void()> update_signal;
};

#endif // MENUMODEL_HH
