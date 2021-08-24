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

#ifndef MENUMODEL_HH
#define MENUMODEL_HH

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
    Node(std::string_view id, const std::string &text, Activated activated = Activated());
    virtual ~Node() = default;

    std::string get_text() const;
    std::string get_id() const;
    bool is_visible() const;

    void set_text(const std::string &text);
    void set_visible(bool visible);

    boost::signals2::signal<void()> &signal_changed();

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
    SubMenuNode(std::string_view id, const std::string &text);

    static Ptr create(std::string_view id, const std::string &text);

    void add(menus::Node::Ptr submenu, menus::Node::Ptr before = menus::Node::Ptr());
    void remove(menus::Node::Ptr submenu);

    const std::list<Node::Ptr> get_children() const;

  private:
    std::list<Node::Ptr> children;
  };

  class ActionNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<ActionNode>;

    ActionNode(std::string_view id, const std::string &text, Activated activated);

    static Ptr create(std::string_view id, const std::string &text, Activated activated);
  };

  class ToggleNode : public Node
  {
  public:
    using Ptr = std::shared_ptr<ToggleNode>;

    ToggleNode(std::string_view id, const std::string &text, Activated activated);

    static Ptr create(std::string_view id, const std::string &text, Activated activated);

    bool is_checked() const;
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

    RadioNode(std::shared_ptr<RadioGroupNode> group, std::string_view id, const std::string &text, int value, Activated activated);

    static Ptr create(std::shared_ptr<RadioGroupNode> group, std::string_view id, const std::string &text, int value, Activated activated);

    int get_value() const;
    void set_value(int value);

    bool is_checked() const;
    void set_checked(bool checked);

    std::string get_group_id() const;

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

    RadioGroupNode(std::string_view id, const std::string &text, Activated activated = Activated());

    static Ptr create(std::string_view id, const std::string &string, Activated activated = Activated());

    void add(menus::RadioNode::Ptr submenu, menus::RadioNode::Ptr before = menus::RadioNode::Ptr());
    void remove(menus::RadioNode::Ptr submenu);
    const std::list<RadioNode::Ptr> get_children() const;

    void select(std::string id);
    void select(int value);

    menus::RadioNode::Ptr get_selected_node() const;
    int get_selected_value() const;
    std::string get_selected_id() const;

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
    static Ptr create();
  };
} // namespace menus

class MenuModel
{
public:
  typedef std::shared_ptr<MenuModel> Ptr;

  MenuModel();

  menus::SubMenuNode::Ptr get_root() const;
  void update();

  boost::signals2::signal<void()> &signal_update();

private:
  std::shared_ptr<menus::SubMenuNode> root;
  boost::signals2::signal<void()> update_signal;
};

#endif // MENUMODEL_HH
