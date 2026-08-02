// Copyright (C) 2003 - 2012 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef WORKRAVE_UI_GTK_COMPAT_HH
#define WORKRAVE_UI_GTK_COMPAT_HH

#include <gtkmm.h>

// Small compatibility shim that lets the same widget code compile against
// both gtkmm3 and gtkmm4. Only the pieces actually used by Workrave are
// covered; extend as needed rather than trying to be exhaustive.
namespace GtkCompat
{
#if GTK_CHECK_VERSION(4, 0, 0)
  constexpr auto ORIENTATION_HORIZONTAL = Gtk::Orientation::HORIZONTAL;
  constexpr auto ORIENTATION_VERTICAL = Gtk::Orientation::VERTICAL;

  constexpr auto ALIGN_START = Gtk::Align::START;
  constexpr auto ALIGN_END = Gtk::Align::END;
  constexpr auto ALIGN_CENTER = Gtk::Align::CENTER;
  constexpr auto ALIGN_FILL = Gtk::Align::FILL;

  constexpr auto POLICY_AUTOMATIC = Gtk::PolicyType::AUTOMATIC;
  constexpr auto POLICY_NEVER = Gtk::PolicyType::NEVER;
  constexpr auto POLICY_ALWAYS = Gtk::PolicyType::ALWAYS;

  constexpr auto RESPONSE_CLOSE = Gtk::ResponseType::CLOSE;
  constexpr auto RESPONSE_OK = Gtk::ResponseType::OK;
  constexpr auto RESPONSE_CANCEL = Gtk::ResponseType::CANCEL;

  constexpr auto STATE_FLAG_ACTIVE = Gtk::StateFlags::ACTIVE;

  constexpr auto STACK_TRANSITION_TYPE_CROSSFADE = Gtk::StackTransitionType::CROSSFADE;

  constexpr auto RESPONSE_YES = Gtk::ResponseType::YES;
  constexpr auto RESPONSE_NO = Gtk::ResponseType::NO;
  constexpr auto RESPONSE_NONE = Gtk::ResponseType::NONE;

  constexpr auto MESSAGE_WARNING = Gtk::MessageType::WARNING;
  constexpr auto MESSAGE_INFO = Gtk::MessageType::INFO;
  constexpr auto MESSAGE_ERROR = Gtk::MessageType::ERROR;

  constexpr auto BUTTONS_YES_NO = Gtk::ButtonsType::YES_NO;
  constexpr auto BUTTONS_OK = Gtk::ButtonsType::OK;

  constexpr auto SIZE_GROUP_HORIZONTAL = Gtk::SizeGroup::Mode::HORIZONTAL;
  constexpr auto SIZE_GROUP_VERTICAL = Gtk::SizeGroup::Mode::VERTICAL;
  constexpr auto SIZE_GROUP_BOTH = Gtk::SizeGroup::Mode::BOTH;

  constexpr auto WRAP_WORD = Gtk::WrapMode::WORD;

  constexpr auto POS_TOP = Gtk::PositionType::TOP;
#else
  constexpr auto ORIENTATION_HORIZONTAL = Gtk::ORIENTATION_HORIZONTAL;
  constexpr auto ORIENTATION_VERTICAL = Gtk::ORIENTATION_VERTICAL;

  constexpr auto ALIGN_START = Gtk::ALIGN_START;
  constexpr auto ALIGN_END = Gtk::ALIGN_END;
  constexpr auto ALIGN_CENTER = Gtk::ALIGN_CENTER;
  constexpr auto ALIGN_FILL = Gtk::ALIGN_FILL;

  constexpr auto POLICY_AUTOMATIC = Gtk::POLICY_AUTOMATIC;
  constexpr auto POLICY_NEVER = Gtk::POLICY_NEVER;
  constexpr auto POLICY_ALWAYS = Gtk::POLICY_ALWAYS;

  constexpr auto RESPONSE_CLOSE = Gtk::RESPONSE_CLOSE;
  constexpr auto RESPONSE_OK = Gtk::RESPONSE_OK;
  constexpr auto RESPONSE_CANCEL = Gtk::RESPONSE_CANCEL;

  constexpr auto STATE_FLAG_ACTIVE = Gtk::STATE_FLAG_ACTIVE;

  constexpr auto STACK_TRANSITION_TYPE_CROSSFADE = Gtk::STACK_TRANSITION_TYPE_CROSSFADE;

  constexpr auto RESPONSE_YES = Gtk::RESPONSE_YES;
  constexpr auto RESPONSE_NO = Gtk::RESPONSE_NO;
  constexpr auto RESPONSE_NONE = Gtk::RESPONSE_NONE;

  constexpr auto MESSAGE_WARNING = Gtk::MESSAGE_WARNING;
  constexpr auto MESSAGE_INFO = Gtk::MESSAGE_INFO;
  constexpr auto MESSAGE_ERROR = Gtk::MESSAGE_ERROR;

  constexpr auto BUTTONS_YES_NO = Gtk::BUTTONS_YES_NO;
  constexpr auto BUTTONS_OK = Gtk::BUTTONS_OK;

  constexpr auto SIZE_GROUP_HORIZONTAL = Gtk::SIZE_GROUP_HORIZONTAL;
  constexpr auto SIZE_GROUP_VERTICAL = Gtk::SIZE_GROUP_VERTICAL;
  constexpr auto SIZE_GROUP_BOTH = Gtk::SIZE_GROUP_BOTH;

  constexpr auto WRAP_WORD = Gtk::WRAP_WORD;

  constexpr auto POS_TOP = Gtk::POS_TOP;
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
  class Box : public Gtk::Box
  {
  public:
    using Gtk::Box::Box;

    explicit Box(Gtk::Orientation orientation = Gtk::Orientation::HORIZONTAL, int spacing = 0)
      : Gtk::Box(orientation, spacing)
    {
    }

    void pack_start(Gtk::Widget &child, bool expand, bool fill, guint padding = 0)
    {
      prepend(child);
      child.property_hexpand() = expand;
      child.property_halign() = fill ? Gtk::Align::FILL : Gtk::Align::START;
      if (padding > 0)
        {
          child.set_margin_start(static_cast<int>(padding));
          child.set_margin_end(static_cast<int>(padding));
        }
    }

    void pack_end(Gtk::Widget &child, bool expand, bool fill, guint padding = 0)
    {
      append(child);
      child.property_hexpand() = expand;
      child.property_halign() = fill ? Gtk::Align::FILL : Gtk::Align::END;
      if (padding > 0)
        {
          child.set_margin_start(static_cast<int>(padding));
          child.set_margin_end(static_cast<int>(padding));
        }
    }
  };

  class VBox : public Box
  {
  public:
    explicit VBox(bool homogeneous = false, int spacing = 0)
      : Box(Gtk::Orientation::VERTICAL, spacing)
    {
      set_homogeneous(homogeneous);
    }
  };

  class HBox : public Box
  {
  public:
    explicit HBox(bool homogeneous = false, int spacing = 0)
      : Box(Gtk::Orientation::HORIZONTAL, spacing)
    {
      set_homogeneous(homogeneous);
    }
  };

  class HSeparator : public Gtk::Separator
  {
  public:
    HSeparator()
      : Gtk::Separator(Gtk::Orientation::HORIZONTAL)
    {
    }
  };

  class VSeparator : public Gtk::Separator
  {
  public:
    VSeparator()
      : Gtk::Separator(Gtk::Orientation::VERTICAL)
    {
    }
  };

  class ButtonBox : public Box
  {
  };

  class HButtonBox : public Box
  {
  };

  // GTK4 removed Gtk::EventBox: any widget can receive input directly via
  // event controllers, so a windowless "just catch events" wrapper is no
  // longer needed. This stand-in keeps call sites that merely use it as a
  // single-child container (add()/tooltip) compiling; ports that actually
  // need press/release handling should attach a Gtk::GestureClick instead.
  class EventBox : public Box
  {
  public:
    EventBox() = default;

    void add(Gtk::Widget &child) { append(child); }
    void set_events(int /*events*/) {}
    int get_events() const { return 0; }
  };

  // GTK4 dropped single-child add() in favour of set_child(); this helper
  // lets call sites use one spelling regardless of toolkit version.
  template<typename Container>
  void set_child(Container &container, Gtk::Widget &child)
  {
    container.set_child(child);
  }

  // GTK4 widgets are visible by default and there is no recursive show_all();
  // showing the top-level widget is enough.
  inline void show_all(Gtk::Widget &widget)
  {
    widget.show();
  }

  // Border width was removed from GTK4; approximate it with margins.
  inline void set_border_width(Gtk::Widget &widget, guint width)
  {
    widget.set_margin_start(static_cast<int>(width));
    widget.set_margin_end(static_cast<int>(width));
    widget.set_margin_top(static_cast<int>(width));
    widget.set_margin_bottom(static_cast<int>(width));
  }

  // gtkmm4's Gtk::Requisition is a real wrapper class (get_width()/get_height());
  // gtkmm3's is a plain typedef for GtkRequisition with public fields.
  // GTK4's Image no longer derives from Gtk::Misc, so it lost set_alignment().
  inline void set_image_alignment(Gtk::Image &image, double xalign, double yalign)
  {
    image.set_halign(xalign <= 0.0 ? Gtk::Align::START : (xalign >= 1.0 ? Gtk::Align::END : Gtk::Align::CENTER));
    image.set_valign(yalign <= 0.0 ? Gtk::Align::START : (yalign >= 1.0 ? Gtk::Align::END : Gtk::Align::CENTER));
  }

  inline int req_width(const Gtk::Requisition &r)
  {
    return r.get_width();
  }

  inline int req_height(const Gtk::Requisition &r)
  {
    return r.get_height();
  }

  // GTK4 removed Widget::grab_default(); the toplevel window owns the
  // default widget instead.
  inline void grab_default(Gtk::Window &window, Gtk::Widget &widget)
  {
    window.set_default_widget(widget);
  }

  // GTK4 removed Gtk::Dialog::run(); pump a nested main loop until the
  // dialog emits a response, matching the old blocking behaviour.
  inline int run_dialog(Gtk::Dialog &dialog)
  {
    auto loop = Glib::MainLoop::create();
    int response = static_cast<int>(Gtk::ResponseType::NONE);
    sigc::connection conn = dialog.signal_response().connect([&](int r) {
      response = r;
      loop->quit();
    });
    dialog.set_visible(true);
    loop->run();
    conn.disconnect();
    dialog.set_visible(false);
    return response;
  }
#else
  using Box = ::Gtk::Box;
  using VBox = ::Gtk::VBox;
  using HBox = ::Gtk::HBox;
  using HSeparator = ::Gtk::HSeparator;
  using VSeparator = ::Gtk::VSeparator;
  using ButtonBox = ::Gtk::ButtonBox;
  using HButtonBox = ::Gtk::HButtonBox;
  using EventBox = ::Gtk::EventBox;

  template<typename Container>
  void set_child(Container &container, Gtk::Widget &child)
  {
    container.add(child);
  }

  inline void show_all(Gtk::Widget &widget)
  {
    widget.show_all();
  }

  inline void set_border_width(Gtk::Widget &widget, guint width)
  {
    if (auto *container = dynamic_cast<Gtk::Container *>(&widget))
      {
        container->set_border_width(width);
      }
  }

  inline void set_image_alignment(Gtk::Image &image, double xalign, double yalign)
  {
    image.set_alignment(xalign, yalign);
  }

  inline int req_width(const Gtk::Requisition &r)
  {
    return r.width;
  }

  inline int req_height(const Gtk::Requisition &r)
  {
    return r.height;
  }

  inline int run_dialog(Gtk::Dialog &dialog)
  {
    return dialog.run();
  }

  inline void grab_default(Gtk::Window & /*window*/, Gtk::Widget &widget)
  {
    widget.grab_default();
  }
#endif
} // namespace GtkCompat

#endif // WORKRAVE_UI_GTK_COMPAT_HH
