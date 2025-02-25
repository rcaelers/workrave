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

#ifndef WORKRAVE_UI_MENUMODEL_HH
#define WORKRAVE_UI_MENUMODEL_HH

#include <string>
#include <list>
#include <functional>
#include <memory>
#include <optional>
#include <boost/signals2.hpp>

#include "spdlog/spdlog.h"

namespace menus
{
  class Node
  {
  public:
    using Ptr = std::shared_ptr<Node>;
    using Activated = std::function<void()>;

    Node();
    explicit Node(std::string_view id, std::string text = "", Activated activated = Activated());
    virtual ~Node() = default;

    [[nodiscard]] auto get_text() const -> std::string;
    [[nodiscard]] auto get_text_no_accel() const -> std::string;
    [[nodiscard]] auto get_dynamic_text() const -> std::string;
    [[nodiscard]] auto get_dynamic_text_no_accel() const -> std::string;
    [[nodiscard]] auto get_id() const -> std::string;
    [[nodiscard]] auto is_visible() const -> bool;

    void set_text(std::string text);
    void set_dynamic_text(std::string text);
    void unset_dynamic_text();
    void set_visible(bool visible);

    auto signal_changed() -> boost::signals2::signal<void()> &;

    virtual void activate();

  protected:
    const std::string id;
    std::string text;
    std::optional<std::string> text_dynamic;
    bool visible{true};
    Activated activated;
    boost::signals2::signal<void()> changed_signal;
  };

  class SectionNode;

  template<class T>
  class ContainerNodeImpl
  {
  public:
    using Ptr = std::shared_ptr<ContainerNodeImpl<T>>;
    using ChildNodePtr = typename T::Ptr;

    ContainerNodeImpl() = default;

    void add_before(ChildNodePtr node, std::string_view id)
    {
      auto pos = std::find_if(children.begin(), children.end(), [id](const auto &n) { return n->get_id() == std::string{id}; });
      if (pos != children.end())
        {
          children.insert(pos, node);
        }
      else
        {
          children.push_back(node);
        }
    }

    void add_after(ChildNodePtr node, std::string_view id)
    {
      auto pos = std::find_if(children.begin(), children.end(), [id](const auto &n) { return n->get_id() == std::string{id}; });
      if (pos != children.end())
        {
          pos++;
          children.insert(pos, node);
        }
      else
        {
          children.push_back(node);
        }
    }

    void add(ChildNodePtr node, menus::Node::Ptr before = menus::Node::Ptr())
    {
      auto pos = children.end();
      if (before)
        {
          pos = std::find(children.begin(), children.end(), before);
        }

      children.insert(pos, node);
    }

    void remove(ChildNodePtr node)
    {
      children.remove(node);
    }

    [[nodiscard]] auto get_children() const -> std::list<ChildNodePtr>
    {
      return children;
    }

  protected:
    std::list<ChildNodePtr> children;
  };

  class ContainerNode
    : public Node
    , public ContainerNodeImpl<Node>
  {
  public:
    using Ptr = std::shared_ptr<ContainerNode>;

    explicit ContainerNode(std::string_view id, std::string text = "", Activated activated = Activated());

    [[nodiscard]] auto find_section(std::string id) const -> std::shared_ptr<SectionNode>
    {
      for (auto &node: get_children())
        {
          if (auto n = std::dynamic_pointer_cast<menus::SectionNode>(node); n)
            {
              if (node->get_id() == id)
                {
                  return n;
                }
            }
          if (auto n = std::dynamic_pointer_cast<menus::ContainerNode>(node); n)
            {
              auto ret = n->find_section(id);
              if (ret)
                {
                  return ret;
                }
            }
        }
      return {};
    }
  };

  class SectionNode : public ContainerNode
  {
  public:
    using Ptr = std::shared_ptr<SectionNode>;

    explicit SectionNode(std::string_view id);

    static auto create(std::string_view id) -> Ptr;
  };

  class SubMenuNode : public ContainerNode
  {
  public:
    using Ptr = std::shared_ptr<SubMenuNode>;

    SubMenuNode(std::string_view id, std::string text);

    static auto create(std::string_view id, std::string text) -> Ptr;
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

    static auto create(std::shared_ptr<RadioGroupNode> group,
                       std::string_view id,
                       std::string text,
                       int value,
                       Activated activated) -> Ptr;

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

  class RadioGroupNode
    : public Node
    , public ContainerNodeImpl<RadioNode> // FIXME: merge into container node
  {
  public:
    using Ptr = std::shared_ptr<RadioGroupNode>;
    RadioGroupNode(std::string_view id, std::string text, Activated activated = Activated());

    static auto create(std::string_view id, std::string text, Activated activated = Activated()) -> Ptr;

    [[nodiscard]] auto get_selected_node() const -> menus::RadioNode::Ptr;
    [[nodiscard]] auto get_selected_value() const -> int;
    [[nodiscard]] auto get_selected_id() const -> std::string;

    void select(std::string id);
    void select(int value);

    void activate() override;
    void activate(int value);

  private:
    std::string selected_id;
  };

  class SeparatorNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<SeparatorNode>;
    SeparatorNode()
      : Node("seperator"){};
    static auto create() -> Ptr;
  };
} // namespace menus

class MenuModel
{
public:
  using Ptr = std::shared_ptr<MenuModel>;

  MenuModel();
  ~MenuModel();

  [[nodiscard]] auto get_root() const -> menus::SubMenuNode::Ptr;
  [[nodiscard]] auto find_section(std::string id) const -> menus::SectionNode::Ptr;
  void update();

  auto signal_update() -> boost::signals2::signal<void()> &;

private:
  std::shared_ptr<menus::SubMenuNode> root;
  boost::signals2::signal<void()> update_signal;
};

#endif // MENUMODEL_HH
