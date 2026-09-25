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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ToolkitMenu.hh"
#include "commonui/MenuModel.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include <vector>
#endif

using namespace detail;

#if defined(PLATFORM_OS_WINDOWS)
namespace
{
  // Gtk::Menu(model) puts icons inside the label area. ImageMenuItem keeps them
  // in the shared check/icon column, so all labels line up, including submenus.
  std::vector<Gtk::MenuItem *> create_menu_items(const Glib::RefPtr<Gio::MenuModel> &model,
                                                 const Glib::RefPtr<Gio::SimpleActionGroup> &actions)
  {
    std::vector<Gtk::MenuItem *> items;
    for (int i = 0; i < model->get_n_items(); ++i)
      {
        if (auto section = model->get_item_link(i, Gio::MENU_LINK_SECTION))
          {
            auto children = create_menu_items(section, actions);
            if (!children.empty())
              {
                if (!items.empty())
                  {
                    items.push_back(Gtk::manage(new Gtk::SeparatorMenuItem()));
                  }
                items.insert(items.end(), children.begin(), children.end());
              }
            continue;
          }

        auto label_value = model->get_item_attribute(i, Gio::MENU_ATTRIBUTE_LABEL, Glib::VARIANT_TYPE_STRING);
        Glib::ustring label = label_value ? g_variant_get_string(label_value.gobj(), nullptr) : "";
        Gtk::MenuItem *item = nullptr;
        if (auto submenu_model = model->get_item_link(i, Gio::MENU_LINK_SUBMENU))
          {
            auto submenu = Gtk::manage(new Gtk::Menu());
            for (auto child: create_menu_items(submenu_model, actions))
              {
                submenu->append(*child);
              }
            item = Gtk::manage(new Gtk::MenuItem(label, true));
            item->set_submenu(*submenu);
          }
        else
          {
            auto action_value = model->get_item_attribute(i, Gio::MENU_ATTRIBUTE_ACTION, Glib::VARIANT_TYPE_STRING);
            Glib::ustring action_name = action_value ? g_variant_get_string(action_value.gobj(), nullptr) : "";
            Glib::VariantBase target(g_menu_model_get_item_attribute_value(model->gobj(), i, "target", nullptr), false);
            auto action = action_name.empty() ? Glib::RefPtr<Gio::Action>() : actions->lookup_action(action_name.substr(4));
            if (action && g_action_get_state_type(action->gobj()) != nullptr)
              {
                auto check = Gtk::manage(new Gtk::CheckMenuItem(label, true));
                check->set_draw_as_radio(bool(target));
                item = check;
              }
            else
              {
                auto image_item = Gtk::manage(new Gtk::ImageMenuItem(label, true));
                Glib::VariantBase icon_value(g_menu_model_get_item_attribute_value(model->gobj(), i, "icon", nullptr), false);
                if (icon_value)
                  {
                    auto image = Gtk::manage(new Gtk::Image(Gio::Icon::deserialize(icon_value), Gtk::ICON_SIZE_MENU));
                    image->set_pixel_size(16);
                    image_item->set_image(*image);
                    image_item->set_always_show_image(true);
                  }
                item = image_item;
              }
            if (target)
              {
                gtk_actionable_set_action_target_value(GTK_ACTIONABLE(item->gobj()), target.gobj());
              }
            if (!action_name.empty())
              {
                gtk_actionable_set_action_name(GTK_ACTIONABLE(item->gobj()), action_name.c_str());
              }
          }
        items.push_back(item);
      }
    return items;
  }
} // namespace
#endif

ToolkitMenu::ToolkitMenu(MenuModel::Ptr menu_model, MenuNodeFilter filter)
{
  menus::SubMenuNode::Ptr root = menu_model->get_root();

  auto action_group = Gio::SimpleActionGroup::create();

  context = std::make_shared<detail::ToolkitMenuContext>(action_group, filter);

  entry = std::make_shared<ToolkitSubMenuEntry>(context, nullptr, root);

#if defined(PLATFORM_OS_WINDOWS)
  gtk_menu = std::make_shared<Gtk::Menu>();
  update_menu();
#else
  gtk_menu = std::make_shared<Gtk::Menu>(entry->get_menu());
#endif
  workrave::utils::connect(menu_model->signal_update(), this, [this]() {
    entry->init();
#if defined(PLATFORM_OS_WINDOWS)
    update_menu();
#endif
  });
}

#if defined(PLATFORM_OS_WINDOWS)
void
ToolkitMenu::update_menu()
{
  for (auto child: gtk_menu->get_children())
    {
      gtk_menu->remove(*child);
    }
  for (auto item: create_menu_items(entry->get_menu(), context->get_action_group()))
    {
      gtk_menu->append(*item);
      item->show_all();
    }
}
#endif

std::shared_ptr<Gtk::Menu>
ToolkitMenu::get_menu() const
{
  return gtk_menu;
}

Glib::RefPtr<Gio::SimpleActionGroup>
ToolkitMenu::get_action_group() const
{
  return context->get_action_group();
}

ToolkitMenuContext::ToolkitMenuContext(Glib::RefPtr<Gio::SimpleActionGroup> action_group, MenuNodeFilter filter)
  : action_group(action_group)
  , filter(filter)
{
}

Glib::RefPtr<Gio::SimpleActionGroup>
ToolkitMenuContext::get_action_group() const
{
  return action_group;
}

MenuNodeFilter
ToolkitMenuContext::get_filter() const
{
  return filter;
}

ToolkitMenuEntry::Ptr
ToolkitMenuEntryFactory::create(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::Node::Ptr node)
{
  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      return std::make_shared<ToolkitSubMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      return std::make_shared<ToolkitRadioGroupMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      return std::make_shared<ToolkitActionMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      return std::make_shared<ToolkitToggleMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      return std::make_shared<ToolkitRadioMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      return std::make_shared<ToolkitSeparatorMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::SectionNode>(node); n)
    {
      return std::make_shared<ToolkitSectionMenuEntry>(context, parent, n);
    }

  return {};
}

ToolkitMenuEntry::ToolkitMenuEntry(ToolkitMenuContext::Ptr context)
  : context(context)
{
}

ToolkitMenuContext::Ptr
ToolkitMenuEntry::get_context() const
{
  return context;
}

ToolkitSubMenuEntry::ToolkitSubMenuEntry(ToolkitMenuContext::Ptr context,
                                         ToolkitSubMenuEntry *parent,
                                         menus::SubMenuNode::Ptr node)
  : ToolkitMenuEntry(context)
  , parent(parent)
  , node(node)
{
  init();
}

void
ToolkitSubMenuEntry::init()
{
  if (parent != nullptr)
    {
      const MenuNodeFilter &filter = get_context()->get_filter();
      if (!filter || filter(node))
        {
          menu = Gio::Menu::create();
          auto item = Gio::MenuItem::create(node->get_text(), std::string("app.") + node->get_id());
          item->set_submenu(menu);
          parent->add(item);
        }
    }
  else
    {
      if (!menu)
        {
          menu = Gio::Menu::create();
        }
      menu->remove_all();
    }

  add_section();
  for (auto menu_to_add: node->get_children())
    {
      ToolkitMenuEntry::Ptr child = ToolkitMenuEntryFactory::create(get_context(), this, menu_to_add);
      children.push_back(child);
    }
}

Glib::RefPtr<Gio::Menu>
ToolkitSubMenuEntry::get_menu() const
{
  return menu;
};

void
ToolkitSubMenuEntry::add(Glib::RefPtr<Gio::MenuItem> item)
{
  if (current_section)
    {
      current_section->append_item(item);
    }
};

void
ToolkitSubMenuEntry::add_section()
{
  if (menu)
    {
      current_section = Gio::Menu::create();
      menu->append_section(current_section);
    }
};

//////////////////////////////////////////////////////////////////////

ToolkitRadioGroupMenuEntry::ToolkitRadioGroupMenuEntry(ToolkitMenuContext::Ptr context,
                                                       ToolkitSubMenuEntry *parent,
                                                       menus::RadioGroupNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  auto action = context->get_action_group()->add_action_radio_integer(
    node->get_id(),
    [node](int value) { node->activate(value); },
    0);
  action->change_state(node->get_selected_value());

  workrave::utils::connect(node->signal_changed(), this, [action, node]() { action->change_state(node->get_selected_value()); });

  for (auto child_node: node->get_children())
    {
      auto child = std::make_shared<ToolkitRadioMenuEntry>(context, parent, child_node);
      children.push_back(child);
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitActionMenuEntry::ToolkitActionMenuEntry(ToolkitMenuContext::Ptr context,
                                               ToolkitSubMenuEntry *parent,
                                               menus::ActionNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  action = context->get_action_group()->add_action(node->get_id(), [node]() { node->activate(); });

  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      auto item = Gio::MenuItem::create(node->get_dynamic_text(), std::string("app.") + node->get_id());
#if defined(PLATFORM_OS_WINDOWS)
      if (auto icon = node->get_icon_name(); !icon.empty())
        {
          item->set_icon(Gio::ThemedIcon::create("workrave-menu-" + icon));
        }
#endif
      parent->add(item);
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitToggleMenuEntry::ToolkitToggleMenuEntry(ToolkitMenuContext::Ptr context,
                                               ToolkitSubMenuEntry *parent,
                                               menus::ToggleNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  action = context->get_action_group()->add_action_bool(node->get_id(), [this, node]() {
    bool active = false;
    action->get_state(active);
    node->activate(!active);
  });

  action->change_state(node->is_checked());

  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      auto item = Gio::MenuItem::create(node->get_dynamic_text(), std::string("app.") + node->get_id());
      workrave::utils::connect(node->signal_changed(), this, [this, node]() { action->change_state(node->is_checked()); });
      parent->add(item);
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitRadioMenuEntry::ToolkitRadioMenuEntry(ToolkitMenuContext::Ptr context,
                                             ToolkitSubMenuEntry *parent,
                                             menus::RadioNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      auto item = Gio::MenuItem::create(node->get_dynamic_text(), std::string("app.") + node->get_group_id());
      item->set_attribute_value("target", Glib::Variant<int>::create(node->get_value()));
      parent->add(item);
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitSeparatorMenuEntry::ToolkitSeparatorMenuEntry(ToolkitMenuContext::Ptr context,
                                                     ToolkitSubMenuEntry *parent,
                                                     menus::SeparatorNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      parent->add_section();
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitSectionMenuEntry::ToolkitSectionMenuEntry(ToolkitMenuContext::Ptr context,
                                                 ToolkitSubMenuEntry *parent,
                                                 menus::SectionNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  for (auto child_node: node->get_children())
    {
      auto child = ToolkitMenuEntryFactory::create(context, parent, child_node);
      children.push_back(child);
    }
}
