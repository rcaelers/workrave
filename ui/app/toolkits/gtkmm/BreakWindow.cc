// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#include <memory>

#if defined(PLATFORM_OS_WINDOWS)
#  include "ui/windows/WindowsCompat.hh"
#  include "ui/windows/WindowsForceFocus.hh"
#  include <gdk/gdkwin32.h>
#  undef ERROR
#  undef IN
#  undef OUT
#  undef WINDING
#endif

#if defined(PLATFORM_OS_WINDOWS_NATIVE)
#  undef max
#endif

#include "debug.hh"
#include <gtkmm.h>
#include <gdk/gdkkeysyms.h>
#include <cairomm/cairomm.h>

#include "BreakWindow.hh"

#include "core/IBreak.hh"
#include "GtkUtil.hh"
#include "Frame.hh"
#include "session/System.hh"
#include "utils/Platform.hh"
#include "ui/IApplicationContext.hh"
#include "commonui/nls.h"

#if defined(PLATFORM_OS_WINDOWS)
#  include "ui/windows/DesktopWindow.hh"
#  include "config/IConfigurator.hh"
#  include "core/ICore.hh"
#elif defined(PLATFORM_OS_UNIX)
#  include "desktop-window.h"
#  include "WaylandWindowManager.hh"
#endif

using namespace workrave;
using namespace workrave::utils;

BreakWindow::BreakWindow(std::shared_ptr<IApplicationContext> app,
                         BreakId break_id,
                         HeadInfo &head,
                         BreakFlags break_flags,
                         BlockMode mode)
  : Gtk::Window(Gtk::WINDOW_TOPLEVEL)
  , app(app)
  , head(head)
  , block_mode(mode)
  , break_flags(break_flags)
  , break_id(break_id)
{
  TRACE_ENTRY();

#if defined(PLATFORM_OS_UNIX)
  if (Platform::running_on_wayland())
    {
      auto wm = std::make_shared<WaylandWindowManager>();
      bool success = wm->init();
      if (success)
        {
          window_manager = wm;
        }
    }
#endif

  fullscreen_grab = !app->get_toolkit()->get_locker()->can_lock();

  // Keep the break window on top of all other Workrave windows
  set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);

  if (mode != BlockMode::Off)
    {
      // Disable titlebar to appear like a popup
      set_decorated(false);
      set_skip_taskbar_hint(true);
      set_skip_pager_hint(true);

      if (fullscreen_grab)
        {
          set_app_paintable(true);
          signal_draw().connect(sigc::mem_fun(*this, &BreakWindow::on_draw), false);
          signal_screen_changed().connect(sigc::mem_fun(*this, &BreakWindow::on_screen_changed), false);
          on_screen_changed(get_screen());
          set_size_request(head.get_width(), head.get_height());
        }
    }
#if defined(PLATFORM_OS_UNIX)
  else
    {
      if (Platform::running_on_wayland())
        {
          set_modal(true);
        }
    }
#endif

  // On W32, must be *before* realize, otherwise a border is drawn.
  set_resizable(false);

  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize();

#if defined(PLATFORM_OS_WINDOWS)
  // Here's the secret: IMMEDIATELY after your window creation, set focus to it
  // THEN position it. So:
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  SetFocus(hwnd);
  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif

  if (mode == BlockMode::Off)
    {
      Glib::RefPtr<Gdk::Window> window = get_window();
      window->set_functions(Gdk::FUNC_MOVE);
    }

  bool initial_ignore_activity = false;

#if defined(PLATFORM_OS_WINDOWS)
  if (WindowsForceFocus::GetForceFocusValue())
    initial_ignore_activity = true;

  app->get_configurator()->get_value_with_default("advanced/force_focus_on_break_start",
                                                              force_focus_on_break_start,
                                                              true);
#endif

  auto core = app->get_core();
  core->set_insist_policy(initial_ignore_activity ? InsistPolicy::Ignore : InsistPolicy::Halt);
}

//! Init Application
void
BreakWindow::init_gui()
{
  if (gui == nullptr)
    {
      gui = Gtk::manage(create_gui());

      if (block_mode == BlockMode::Off)
        {
          set_border_width(12);
          add(*gui);
        }
      else
        {
          set_border_width(0);
          Frame *window_frame = Gtk::manage(new Frame());
          window_frame->set_border_width(0);
          window_frame->set_frame_style(Frame::STYLE_BREAK_WINDOW);

          frame = Gtk::manage(new Frame());
          frame->set_frame_style(Frame::STYLE_SOLID);
          frame->set_frame_width(6);
          frame->set_border_width(6);
          frame->set_frame_flashing(0);
          frame->set_frame_visible(false);

          window_frame->add(*frame);
          frame->add(*gui);

          if (block_mode == BlockMode::All && !fullscreen_grab)
            {
#if defined(PLATFORM_OS_WINDOWS)
              desktop_window = new DesktopWindow(head.get_x(), head.get_y(), head.get_width(), head.get_height());
              add(*window_frame);

#elif defined(PLATFORM_OS_UNIX)
              set_size_request(head.get_width(), head.get_height());
              set_app_paintable(true);
              Glib::RefPtr<Gdk::Window> window = get_window();
              set_desktop_background(window->gobj());
              Gtk::Alignment *align = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0.0, 0.0));
              align->add(*window_frame);
              add(*align);
#endif
            }
          else
            {
              if (fullscreen_grab)
                {
                  Gtk::Alignment *align = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0.0, 0.0));
                  align->add(*window_frame);
                  add(*align);
                }
              else
                {
                  add(*window_frame);
                }
            }
        }

      if (break_id != BREAK_ID_REST_BREAK)
        {
          set_can_focus(false);
        }

      show_all_children();
      stick();
    }
}

//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTRY();
  if (frame != nullptr)
    {
      frame->set_frame_flashing(0);
    }

#if defined(PLATFORM_OS_WINDOWS)
  delete desktop_window;
#endif
}

//! Centers the window.
void
BreakWindow::center()
{
  if (!fullscreen_grab)
    {
      GtkUtil::center_window(*this, head);
    }
}

void
BreakWindow::get_operation_name_and_icon(System::SystemOperation::SystemOperationType type,
                                         const char **name,
                                         const char **icon_name)
{
  switch (type)
    {
    case System::SystemOperation::SYSTEM_OPERATION_NONE:
      *name = _("Lock...");
      *icon_name = "lock.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN:
      *name = _("Lock");
      *icon_name = "lock.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_SHUTDOWN:
      *name = _("Shutdown");
      *icon_name = "shutdown.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_SUSPEND:
      *name = _("Suspend");
      *icon_name = "shutdown.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_HIBERNATE:
      *name = _("Hibernate");
      *icon_name = "shutdown.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID:
      *name = _("Suspend hybrid");
      *icon_name = "shutdown.png";
      break;
    default:
      throw "System::execute: Unknown system operation";
    };
}

void
BreakWindow::append_row_to_sysoper_model(Glib::RefPtr<Gtk::ListStore> &model, System::SystemOperation::SystemOperationType type)
{
  TRACE_ENTRY();
  const char *name = nullptr;
  const char *icon_name = nullptr;
  get_operation_name_and_icon(type, &name, &icon_name);

  Gtk::TreeRow row = *(model->append());
  if (sysoper_model_columns->has_button_images)
    {
      row[sysoper_model_columns->icon] = GtkUtil::create_pixbuf(icon_name);
    }
  row[sysoper_model_columns->name] = Glib::ustring(name);
  row[sysoper_model_columns->id] = type;
}

//! Creates the combo box containing options to suspend/hibernate/shutdown/etc
// based on:
//  https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-model.html.en
//  http://www.lugod.org/presentations/gtkmm/treeview.html
//  http://stackoverflow.com/questions/5894344/gtkmm-how-to-put-a-pixbuf-in-a-treeview
Gtk::ComboBox *
BreakWindow::create_sysoper_combobox()
{
  TRACE_ENTRY();
  supported_system_operations = System::get_supported_system_operations();
  bool has_button_images = GtkUtil::has_button_images();

  if (supported_system_operations.empty())
    {
      return nullptr;
    }

  sysoper_model_columns = new SysoperModelColumns(has_button_images);

  Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(*sysoper_model_columns);

  append_row_to_sysoper_model(model, System::SystemOperation::SYSTEM_OPERATION_NONE);

  for (auto &operation: supported_system_operations)
    {
      append_row_to_sysoper_model(model, operation.type);
    }

  // if there are no operations to put in the combobox
  if (model->children().empty())
    {
      delete sysoper_model_columns;
      sysoper_model_columns = nullptr;
      return nullptr;
    }

  auto *comboBox = new Gtk::ComboBox();
  comboBox->set_model(model);
  if (has_button_images)
    {
      comboBox->pack_start(sysoper_model_columns->icon, false);
    }
  comboBox->pack_start(sysoper_model_columns->name);
  comboBox->set_active(0);
  comboBox->signal_changed().connect(sigc::mem_fun(*this, &BreakWindow::on_sysoper_combobox_changed));
  return comboBox;
}

//! Shutdown/suspend/etc. button was clicked
void
BreakWindow::on_sysoper_combobox_changed()
{
  // based on https://developer.gnome.org/gtkmm-tutorial/stable/combobox-example-full.html.en
  TRACE_ENTRY();
  Gtk::ListStore::const_iterator iter = sysoper_combobox->get_active();
  if (!iter)
    {
      TRACE_MSG("!iter");
      return;
    }

  Gtk::ListStore::Row row = *iter;
  if (!row)
    {
      TRACE_MSG("!row");
      return;
    }

  if (row[sysoper_model_columns->id] == System::SystemOperation::SYSTEM_OPERATION_NONE)
    {
      TRACE_MSG("SYSTEM_OPERATION_NONE");
      return;
    }

  auto locker = app->get_toolkit()->get_locker();
  locker->unlock();

  Glib::signal_timeout().connect(
    [locker]() {
      locker->lock();
      return 0;
    },
    5000);

  System::execute(row[sysoper_model_columns->id]);

  // this will fire this method again with SYSTEM_OPERATION_NONE active
  sysoper_combobox->set_active(0);
}

//! Creates the skip button.
Gtk::Button *
BreakWindow::create_skip_button()
{
  Gtk::Button *ret = Gtk::manage(GtkUtil::create_custom_stock_button(_("_Skip"), "window-close"));
  ret->signal_clicked().connect(sigc::mem_fun(*this, &BreakWindow::on_skip_button_clicked));
  ret->set_can_focus(false);
  return ret;
}

//! Creates the postpone button.
Gtk::Button *
BreakWindow::create_postpone_button()
{
  Gtk::Button *ret = Gtk::manage(GtkUtil::create_custom_stock_button(_("_Postpone"), "edit-redo"));
  ret->signal_clicked().connect(sigc::mem_fun(*this, &BreakWindow::on_postpone_button_clicked));
  ret->set_can_focus(false);
  return ret;
}

//! Creates the lock button.
Gtk::Button *
BreakWindow::create_lock_button()
{
  const char *name = nullptr;
  const char *icon_name = nullptr;
  get_operation_name_and_icon(System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN, &name, &icon_name);
  Gtk::Button *ret = Gtk::manage(GtkUtil::create_image_button(name, icon_name));
  ret->signal_clicked().connect(sigc::mem_fun(*this, &BreakWindow::on_lock_button_clicked));
  ret->set_can_focus(false);
  return ret;
}

void
BreakWindow::update_skip_postpone_lock()
{
  if ((postpone_button != nullptr && !postpone_button->get_sensitive())
      || (skip_button != nullptr && !skip_button->get_sensitive()))
    {
      bool skip_locked = false;
      bool postpone_locked = false;
      BreakId overdue_break_id = BREAK_ID_NONE;
      check_skip_postpone_lock(skip_locked, postpone_locked, overdue_break_id);

      if (progress_bar != nullptr)
        {
          if (overdue_break_id != BREAK_ID_NONE)
            {
              auto core = app->get_core();
              auto b = core->get_break(overdue_break_id);

              progress_bar->set_fraction(1.0 - ((double)b->get_elapsed_idle_time()) / (double)b->get_auto_reset());
            }
          else
            {
              progress_bar->hide();
            }
        }

      if (!postpone_locked && postpone_button != nullptr)
        {
          postpone_button->set_has_tooltip(false);
          postpone_button->set_sensitive(true);
        }
      if (!skip_locked && skip_button != nullptr)
        {
          skip_button->set_has_tooltip(false);
          skip_button->set_sensitive(true);
        }
    }
}

//! User has closed the main window.
bool
BreakWindow::on_delete_event(GdkEventAny *)
{
  if (block_mode == BlockMode::Off)
    {
      on_postpone_button_clicked();
    }
  return TRUE;
}

//! The postpone button was clicked.
void
BreakWindow::on_postpone_button_clicked()
{
  auto core = app->get_core();
  auto b = core->get_break(break_id);
  b->postpone_break();
}

//! The skip button was clicked.
void
BreakWindow::on_skip_button_clicked()
{
  auto core = app->get_core();
  auto b = core->get_break(break_id);
  b->skip_break();
}

//! The lock button was clicked.
void
BreakWindow::on_lock_button_clicked()
{
  auto locker = app->get_toolkit()->get_locker();
  locker->unlock();
  locker->lock();
  System::execute(System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN);
}

void
BreakWindow::check_skip_postpone_lock(bool &skip_locked, bool &postpone_locked, BreakId &overdue_break_id)
{
  TRACE_ENTRY();
  skip_locked = false;
  postpone_locked = false;
  overdue_break_id = BREAK_ID_NONE;

  auto core = app->get_core();
  OperationMode mode = core->get_active_operation_mode();

  if (mode == OperationMode::Normal)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          auto b = core->get_break(BreakId(id));
          bool overdue = b->get_elapsed_time() > b->get_limit();

          if ((!(break_flags & BREAK_FLAGS_USER_INITIATED)) || b->is_max_preludes_reached())
            {
              if (!GUIConfig::break_ignorable(BreakId(id))())
                {
                  postpone_locked = overdue;
                }
              if (!GUIConfig::break_skippable(BreakId(id))())
                {
                  skip_locked = overdue;
                }
              if (skip_locked || postpone_locked)
                {
                  overdue_break_id = BreakId(id);
                  return;
                }
            }
        }
    }
}

//! Control buttons.
Gtk::Box *
BreakWindow::create_bottom_box(bool lockable, bool shutdownable)
{
  accel_group = Gtk::AccelGroup::create();
  add_accel_group(accel_group);

  auto *vbox = new Gtk::VBox(false, 0);

  button_size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  box_size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);

  if ((break_flags != BREAK_FLAGS_NONE) || lockable || shutdownable)
    {
      Gtk::HBox *top_box = Gtk::manage(new Gtk::HBox(false, 0));
      Gtk::HBox *bottom_box = Gtk::manage(new Gtk::HBox(false, 6));

      vbox->pack_end(*bottom_box, Gtk::PACK_SHRINK, 2);

      if (break_flags != BREAK_FLAGS_NONE)
        {
          Gtk::HButtonBox *button_box = Gtk::manage(new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6));
          bottom_box->pack_end(*button_box, Gtk::PACK_SHRINK, 0);

          bool skip_locked = false;
          bool postpone_locked = false;
          BreakId overdue_break_id = BREAK_ID_NONE;
          check_skip_postpone_lock(skip_locked, postpone_locked, overdue_break_id);

          if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
            {
              skip_button = create_skip_button();
              skip_button->set_sensitive(!skip_locked);
              if (skip_locked)
                {
                  const char *msg = _("You cannot skip this break while another non-skippable break is overdue.");
                  skip_button->set_tooltip_text(msg);
                }

              button_box->pack_end(*skip_button, Gtk::PACK_EXPAND_WIDGET, 0);
              button_size_group->add_widget(*skip_button);
            }

          if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
            {
              postpone_button = create_postpone_button();
              postpone_button->set_sensitive(!postpone_locked);
              if (postpone_locked)
                {
                  const char *msg = _("You cannot postpone this break while another non-postponable break is overdue.");
                  postpone_button->set_tooltip_text(msg);
                }
              button_box->pack_end(*postpone_button, Gtk::PACK_EXPAND_WIDGET, 0);
              button_size_group->add_widget(*postpone_button);
            }

          if (skip_locked || postpone_locked)
            {
              Gtk::HBox *progress_bar_box = Gtk::manage(new Gtk::HBox(false, 0));

              progress_bar = Gtk::manage(new Gtk::ProgressBar);
              progress_bar->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
              progress_bar->set_fraction(0);
              progress_bar->set_name("locked-progress");
              update_skip_postpone_lock();

              vbox->pack_end(*top_box, Gtk::PACK_SHRINK, 0);
              top_box->pack_end(*progress_bar_box, Gtk::PACK_SHRINK, 0);

              Gtk::Alignment *align = nullptr;
              if (skip_locked && postpone_locked)
                {
                  align = Gtk::manage(new Gtk::Alignment(0, 0, 1.0, 0.0));
                }
              else if (skip_locked)
                {
                  align = Gtk::manage(new Gtk::Alignment(0, 1, 0.0, 0.0));
                }
              else
                {
                  align = Gtk::manage(new Gtk::Alignment(1, 0, 0, 0.0));
                }
              align->add(*progress_bar);
              progress_bar_box->pack_end(*align, Gtk::PACK_EXPAND_WIDGET, 6);

              box_size_group->add_widget(*progress_bar_box);
              box_size_group->add_widget(*button_box);
              button_size_group->add_widget(*progress_bar);

              const char style[] =
                "progress, trough {\n"
                "min-width: 1px;\n"
                "}";
              Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
              Glib::RefPtr<Gtk::StyleContext> style_context = progress_bar->get_style_context();
              css_provider->load_from_data(style);
              style_context->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }
        }

      if (lockable || shutdownable)
        {
          if (shutdownable)
            {
              sysoper_combobox = create_sysoper_combobox();
              if (sysoper_combobox != nullptr)
                {
                  bottom_box->pack_start(*sysoper_combobox, Gtk::PACK_SHRINK, 0);
                }
            }
          else
            {
              lock_button = create_lock_button();
              if (lock_button != nullptr)
                {
                  bottom_box->pack_start(*lock_button, Gtk::PACK_SHRINK, 0);
                }
            }
        }
    }

  return vbox;
}

void
BreakWindow::init()
{
  TRACE_ENTRY();
  init_gui();
}

static void
disable_button_focus(GtkWidget *w, gpointer data)
{
  if (GTK_IS_CONTAINER(w))
    {
      gtk_container_forall(GTK_CONTAINER(w), (GtkCallback)disable_button_focus, nullptr);
    }

  if (GTK_IS_BUTTON(w))
    {
      gtk_widget_set_can_focus(w, FALSE);
    }
}

void
BreakWindow::start()
{
  TRACE_ENTRY();
  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize_if_needed();

#if defined(PLATFORM_OS_UNIX)
  if (window_manager)
    {
      window_manager->init_surface(*this, head.get_monitor(), true);
    }
#endif

  // Set some window hints.
  set_skip_pager_hint(true);
  set_skip_taskbar_hint(true);

  GtkUtil::set_always_on_top(this, true);

  update_break_window();
  center();

#if defined(PLATFORM_OS_WINDOWS)
  if (desktop_window != nullptr)
    {
      desktop_window->set_visible(true);
    }
#endif
  show_all();

  GtkUtil::set_always_on_top(this, true);

#if defined(PLATFORM_OS_WINDOWS)
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  if (force_focus_on_break_start && this->head.is_primary())
    {
      WindowsForceFocus::ForceWindowFocus(hwnd);
    }
  // TODO: next two lines taken from original grab() function, which is called after start(). Still needed?
  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  BringWindowToTop(hwnd);
#endif

  // In case the show_all resized the window...
  center();

  if (sysoper_combobox != nullptr)
    {
      // Setting "can focus" of the sysoper combobox to false is not enough to
      // prevent the combobox from taking the focus. A combobox has an internal
      // button that still has 'can focus' set to true.
      // So, unset 'can focus' of this button...
      disable_button_focus(GTK_WIDGET(sysoper_combobox->gobj()), nullptr);

      // ...and clear the focus of the break window, which already focussed
      // the button.
      GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(sysoper_combobox->gobj()));
      if (gtk_widget_is_toplevel(toplevel))
        {
          gtk_window_set_focus(GTK_WINDOW(toplevel), nullptr);
        }
    }
}

void
BreakWindow::stop()
{
  TRACE_ENTRY();
  if (frame != nullptr)
    {
      frame->set_frame_flashing(0);
    }

  hide();

#if defined(PLATFORM_OS_UNIX)
  if (window_manager)
    {
      window_manager->clear_surfaces();
    }
#endif

#if defined(PLATFORM_OS_WINDOWS)
  if (desktop_window != nullptr)
    {
      desktop_window->set_visible(false);
    }
#endif
}

void
BreakWindow::refresh()
{
  TRACE_ENTRY();
  update_skip_postpone_lock();

  update_break_window();

#if defined(PLATFORM_OS_WINDOWS)
  refresh_break_window();
#endif
}

void
BreakWindow::update_break_window()
{
}

bool
BreakWindow::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  cr->save();
  if (block_mode == BlockMode::All)
    {
      cr->set_source_rgba(0, 0, 0, 0.95);
    }
  else
    {
      cr->set_source_rgba(0.1, 0.1, 0.1, 0.1);
    }
#if CAIROMM_CHECK_VERSION(1, 15, 4)
  cr->set_operator(Cairo::Context::Operator::SOURCE);
#else
  cr->set_operator(Cairo::OPERATOR_SOURCE);
#endif
  cr->paint();
  cr->restore();

  return Gtk::Window::on_draw(cr);
}

void
BreakWindow::on_screen_changed(const Glib::RefPtr<Gdk::Screen> &previous_screen)
{
  (void)previous_screen;

  const Glib::RefPtr<Gdk::Screen> screen = get_screen();
  const Glib::RefPtr<Gdk::Visual> visual = screen->get_rgba_visual();

  if (visual)
    {
      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual->gobj());
    }
}

#if defined(PLATFORM_OS_WINDOWS)
/* WindowsCompat::RefreshBreakWindow()

Refresh a BreakWindow:
- Make keyboard shortcuts available without pressing ALT after five seconds of inactivity
- Set our window topmost unless a tooltip is visible (tooltips are topmost when visible)
- Make sure the window manager has not disabled our topmost status; reset if necessary
- Force focus to the main break window if the preference advanced/force_focus is true
*/
void
BreakWindow::refresh_break_window()
{
  auto core = app->get_core();
  bool user_active = core->is_user_active();

  // GTK keyboard shortcuts can be accessed by using the ALT key. This appear
  // to be non-standard behaviour on windows, so make shortcuts available
  // without ALT after the user is idle for 5s
  if (!user_active && !accel_added)
    {
      auto b = core->get_break(BreakId(break_id));

      // TRACE_VAR(b->get_elapsed_idle_time());
      if (b->get_elapsed_idle_time() > 5)
        {
          if (postpone_button != NULL)
            {
              GtkUtil::update_mnemonic(postpone_button, accel_group);
            }
          if (skip_button != NULL)
            {
              GtkUtil::update_mnemonic(skip_button, accel_group);
            }
          // FIXME:
          // if( shutdown_button != NULL )
          // {
          //     GtkUtil::update_mnemonic( shutdown_button, accel_group );
          // }
          // if( lock_button != NULL )
          // {
          //     GtkUtil::update_mnemonic( lock_button, accel_group );
          // }
          accel_added = true;
        }
    }

  /* We can't call GtkUtil::set_always_on_top() or WindowsCompat::SetWindowOnTop() for every
  refresh. While the logic here is similar it is adjusted for the specific case of refreshing.
  */

  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  if (!hwnd)
    return;

  // don't enforce topmost while a tooltip is visible, otherwise we could cover the tooltip
  if (!GtkUtil::get_visible_tooltip_window())
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

  // this checks if the window manager has disabled our topmost ability and resets it if necessary
  WindowsCompat::ResetWindow(hwnd, true);

  /* If there are multiple break windows only force focus on the first, otherwise focus would be
  continuously switched to each break window on every refresh, making interaction very difficult.
  */
  if (WindowsForceFocus::GetForceFocusValue() && (head.is_primary()))
    {
      WindowsForceFocus::ForceWindowFocus(hwnd, 0); // try without blocking
    }
}
#endif
