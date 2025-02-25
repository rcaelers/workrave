// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include <giomm.h>
#include <gtkmm.h>

#include "utils/Signals.hh"

using MenuNodeFilter = std::function<bool(menus::Node::Ptr)>;

namespace detail
{
  class ToolkitSubMenuEntry;

  class ToolkitMenuContext
  {
  public:
    using Ptr = std::shared_ptr<ToolkitMenuContext>;

    ToolkitMenuContext(Glib::RefPtr<Gio::SimpleActionGroup> action_group, MenuNodeFilter filter);

    Glib::RefPtr<Gio::SimpleActionGroup> get_action_group() const;
    MenuNodeFilter get_filter() const;

  private:
    Glib::RefPtr<Gio::SimpleActionGroup> action_group;
    MenuNodeFilter filter;
  };

  class ToolkitMenuEntry : public workrave::utils::Trackable
  {
  public:
    using Ptr = std::shared_ptr<ToolkitMenuEntry>;

    explicit ToolkitMenuEntry(ToolkitMenuContext::Ptr context);

    ToolkitMenuContext::Ptr get_context() const;

  private:
    ToolkitMenuContext::Ptr context;
  };

  class ToolkitSubMenuEntry : public ToolkitMenuEntry
  {
  public:
    using Ptr = std::shared_ptr<ToolkitSubMenuEntry>;

    ToolkitSubMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::SubMenuNode::Ptr node);

    void init();
    void add(Glib::RefPtr<Gio::MenuItem> item);
    void add_section();

    Glib::RefPtr<Gio::Menu> get_menu() const;

  private:
    ToolkitSubMenuEntry *parent;
    menus::SubMenuNode::Ptr node;
    std::list<ToolkitMenuEntry::Ptr> children;
    Glib::RefPtr<Gio::Menu> menu;
    Glib::RefPtr<Gio::Menu> current_section;
  };

  class ToolkitActionMenuEntry : public ToolkitMenuEntry
  {
  public:
    ToolkitActionMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::ActionNode::Ptr node);

  private:
    Glib::RefPtr<Gio::SimpleAction> action;
  };

  class ToolkitToggleMenuEntry : public ToolkitMenuEntry
  {
  public:
    ToolkitToggleMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::ToggleNode::Ptr node);

  private:
    Glib::RefPtr<Gio::SimpleAction> action;
  };

  class ToolkitRadioMenuEntry : public ToolkitMenuEntry
  {
  public:
    ToolkitRadioMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::RadioNode::Ptr node);
  };

  class ToolkitRadioGroupMenuEntry : public ToolkitMenuEntry
  {
  public:
    ToolkitRadioGroupMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::RadioGroupNode::Ptr node);

  private:
    std::list<ToolkitRadioMenuEntry::Ptr> children;
  };

  class ToolkitSeparatorMenuEntry : public ToolkitMenuEntry
  {
  public:
    ToolkitSeparatorMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::SeparatorNode::Ptr node);
  };

  class ToolkitSectionMenuEntry : public ToolkitMenuEntry
  {
  public:
    ToolkitSectionMenuEntry(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::SectionNode::Ptr node);

  private:
    std::list<ToolkitMenuEntry::Ptr> children;
  };

  class ToolkitMenuEntryFactory
  {
  public:
    static ToolkitMenuEntry::Ptr create(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::Node::Ptr node);
  };

} // namespace detail

class ToolkitMenu : public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<ToolkitMenu>;

  explicit ToolkitMenu(MenuModel::Ptr menu_model, MenuNodeFilter filter = nullptr);

  Glib::RefPtr<Gio::SimpleActionGroup> get_action_group() const;
  std::shared_ptr<Gtk::Menu> get_menu() const;

private:
  std::shared_ptr<detail::ToolkitMenuContext> context;
  std::shared_ptr<Gtk::Menu> gtk_menu;
  detail::ToolkitSubMenuEntry::Ptr entry;
};

#endif // TOOLKITMENU_HH
