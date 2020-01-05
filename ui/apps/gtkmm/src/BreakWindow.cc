// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#include <ctime>

#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef PLATFORM_OS_WINDOWS
#include "W32Compat.hh"
#include "W32ForceFocus.hh"
#include <gdk/gdkwin32.h>
#endif

#include <gdk/gdkkeysyms.h>

#ifdef PLATFORM_OS_WINDOWS_NATIVE
#undef max
#endif

#include "debug.hh"
#include "commonui/nls.h"

#include <gtkmm.h>
#include <math.h>

#include "BreakWindow.hh"

#include "config/IConfigurator.hh"

#include "GUI.hh"
#include "core/IBreak.hh"
#include "GtkUtil.hh"
#include "Grab.hh"
#include "Frame.hh"
#include "session/System.hh"
#include "core/ICore.hh"
#include "commonui/Backend.hh"
#include "utils/AssetPath.hh"

#if defined(PLATFORM_OS_WINDOWS)
#include "DesktopWindow.hh"
#elif defined(PLATFORM_OS_UNIX)
#include "desktop-window.h"
#endif

using namespace std;
using namespace workrave;
using namespace workrave::utils;



//! Constructor
/*!
 *  \param control The controller.
 */
BreakWindow::BreakWindow(BreakId break_id, HeadInfo &head,
                         BreakFlags break_flags,
                         GUIConfig::BlockMode mode) :
  Gtk::Window(Gtk::WINDOW_TOPLEVEL),
  block_mode(mode),
  break_flags(break_flags),
  frame(nullptr),
  fullscreen_grab(false),
  gui(nullptr),
  visible(false),
  sysoper_model_columns(nullptr),
  accel_added(false),
  accel_group(nullptr),
  postpone_button(nullptr),
  skip_button(nullptr),
  sysoper_combobox(nullptr)
#ifdef PLATFORM_OS_WINDOWS
  ,
  desktop_window(NULL),
  force_focus_on_break_start(false),
  parent(0)
#endif
{
  TRACE_ENTER("BreakWindow::BreakWindow");
  this->break_id = break_id;

  fullscreen_grab = !Grab::instance()->can_grab();

  if (mode != GUIConfig::BLOCK_MODE_NONE)
    {
      // Disable titlebar to appear like a popup
      set_decorated(false);
      set_skip_taskbar_hint(true);
      set_skip_pager_hint(true);

      if (fullscreen_grab)
        {
          set_app_paintable(true);
          signal_draw().connect(sigc::mem_fun(*this, &BreakWindow::on_draw));
          signal_screen_changed().connect(sigc::mem_fun(*this, &BreakWindow::on_screen_changed));
          on_screen_changed(get_screen());
          fullscreen();
        }
    }

  // On W32, must be *before* realize, otherwise a border is drawn.
  set_resizable(false);

  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize();

#ifdef PLATFORM_OS_WINDOWS
  // Here's the secret: IMMEDIATELY after your window creation, set focus to it
  // THEN position it. So:
  HWND hwnd = (HWND) GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  SetFocus(hwnd);
  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif

  if (mode == GUIConfig::BLOCK_MODE_NONE)
    {
      Glib::RefPtr<Gdk::Window> window = get_window();
      window->set_functions(Gdk::FUNC_MOVE);
    }

  this->head = head;
  Gtk::Window::set_screen(head.screen);

  bool initial_ignore_activity = false;

#ifdef PLATFORM_OS_WINDOWS
   if( W32ForceFocus::GetForceFocusValue() )
       initial_ignore_activity = true;

   Backend::get_configurator()->get_value_with_default(
     "advanced/force_focus_on_break_start",
     force_focus_on_break_start,
     true
     );
#endif

  ICore::Ptr core = Backend::get_core();
  core->set_insist_policy(initial_ignore_activity ?
                          InsistPolicy::Ignore :
                          InsistPolicy::Halt);
  TRACE_EXIT();
}


//! Init GUI
void
BreakWindow::init_gui()
{
  if (gui == nullptr)
    {
      gui = Gtk::manage(create_gui());

      if (block_mode == GUIConfig::BLOCK_MODE_NONE)
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

          if (block_mode == GUIConfig::BLOCK_MODE_ALL && !fullscreen_grab)
            {
              if (!fullscreen_grab)
                {
#ifdef PLATFORM_OS_WINDOWS
                  desktop_window = new DesktopWindow(head);
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
  TRACE_ENTER("BreakWindow::~BreakWindow");

  if (frame != nullptr)
    {
      frame->set_frame_flashing(0);
    }

#ifdef PLATFORM_OS_WINDOWS
  delete desktop_window;
#endif
  TRACE_EXIT();
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
BreakWindow::get_operation_name_and_icon(
    System::SystemOperation::SystemOperationType type, const char **name, const char **icon_name)
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
BreakWindow::append_row_to_sysoper_model(Glib::RefPtr<Gtk::ListStore> &model,
    System::SystemOperation::SystemOperationType type)
{
  TRACE_ENTER("BreakWindow::append_row_to_sysoper_model");

  const char *name;
  const char *icon_name;
  get_operation_name_and_icon(type, &name, &icon_name);

  Gtk::TreeRow row = *(model->append());
  if (sysoper_model_columns->has_button_images)
    {
      try {
          string icon_path = AssetPath::complete_directory(icon_name,
                                            AssetPath::SEARCH_PATH_IMAGES);
          Glib::RefPtr<Gdk::Pixbuf> img = Gdk::Pixbuf::create_from_file(icon_path);
          row[sysoper_model_columns->icon] = img;
      } catch (Glib::Error &e) {
        TRACE_MSG("Cannot read image: " << icon_name);
      }

    }
  row[sysoper_model_columns->name] = Glib::ustring(name);
  row[sysoper_model_columns->id] = type;
  TRACE_EXIT()
}

//! Creates the combo box containing options to suspend/hibernate/shutdown/etc
//based on:
//  https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-model.html.en
//  http://www.lugod.org/presentations/gtkmm/treeview.html
//  http://stackoverflow.com/questions/5894344/gtkmm-how-to-put-a-pixbuf-in-a-treeview
Gtk::ComboBox *
BreakWindow::create_sysoper_combobox()
{
  TRACE_ENTER("BreakWindow::create_sysoper_combobox");
  supported_system_operations = System::get_supported_system_operations();
  bool has_button_images = GtkUtil::has_button_images();

  if (supported_system_operations.empty())
    {
      return nullptr;
    }

  sysoper_model_columns = new SysoperModelColumns(has_button_images);

  Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(*sysoper_model_columns);

  append_row_to_sysoper_model(model,
      System::SystemOperation::SYSTEM_OPERATION_NONE);

  for (std::vector<System::SystemOperation>::iterator iter = supported_system_operations.begin();
      iter != supported_system_operations.end(); ++iter)
    {
      append_row_to_sysoper_model(model,
          iter->type);
    }

  //if there are no operations to put in the combobox
  if (model->children().empty())
    {
      delete sysoper_model_columns;
      sysoper_model_columns = nullptr;
      TRACE_EXIT();
      return nullptr;
    }

  Gtk::ComboBox *comboBox = new Gtk::ComboBox();
  comboBox->set_model(model);
  if (has_button_images)
    {
      comboBox->pack_start(sysoper_model_columns->icon, false);
    }
  comboBox->pack_start(sysoper_model_columns->name);
  comboBox->set_active(0);
  comboBox->signal_changed()
      .connect(
                sigc::mem_fun(*this, &BreakWindow::on_sysoper_combobox_changed)
          );
  TRACE_EXIT();
  return comboBox;
}

//! Shutdown/suspend/etc. button was clicked
void
BreakWindow::on_sysoper_combobox_changed()
{
  //based on https://developer.gnome.org/gtkmm-tutorial/stable/combobox-example-full.html.en
  TRACE_ENTER("BreakWindow::on_sysoper_combobox_changed")
  Gtk::ListStore::const_iterator iter = sysoper_combobox->get_active();
  if (!iter)
    {
      TRACE_RETURN("!iter");
      return;
    }

  Gtk::ListStore::Row row = *iter;
  if (!row)
    {
      TRACE_RETURN("!row");
      return;
    }

  if (row[sysoper_model_columns->id] == System::SystemOperation::SYSTEM_OPERATION_NONE)
    {
      TRACE_RETURN("SYSTEM_OPERATION_NONE");
      return;
    }

  IGUI *gui = GUI::get_instance();
  assert(gui != nullptr);
  gui->interrupt_grab();

  System::execute(row[sysoper_model_columns->id]);

  //this will fire this method again with SYSTEM_OPERATION_NONE active
  sysoper_combobox->set_active(0);
}


//! Creates the skip button.
Gtk::Button *
BreakWindow::create_skip_button()
{
  Gtk::Button *ret;
  ret = Gtk::manage(GtkUtil::create_custom_stock_button(_("_Skip"), Gtk::Stock::CLOSE));
  ret->signal_clicked()
    .connect(sigc::mem_fun(*this, &BreakWindow::on_skip_button_clicked));
  ret->set_can_focus(false);
  return ret;
}


//! Creates the postpone button.
Gtk::Button *
BreakWindow::create_postpone_button()
{
  Gtk::Button *ret;
  ret = Gtk::manage(GtkUtil::create_custom_stock_button(_("_Postpone"), Gtk::Stock::REDO));
  ret->signal_clicked()
    .connect(sigc::mem_fun(*this, &BreakWindow::on_postpone_button_clicked));
  ret->set_can_focus(false);
  return ret;
}

//! Creates the lock button.
Gtk::Button *
BreakWindow::create_lock_button()
{
  Gtk::Button *ret;
  const char *name;
  const char *icon_name;
  get_operation_name_and_icon(System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN, &name, &icon_name);
  ret = Gtk::manage(GtkUtil::create_image_button(name, icon_name));
  ret->signal_clicked()
    .connect(sigc::mem_fun(*this, &BreakWindow::on_lock_button_clicked));
  ret->set_can_focus(false);
  return ret;
}

//! User has closed the main window.
bool
BreakWindow::on_delete_event(GdkEventAny *)
{
  if (block_mode == GUIConfig::BLOCK_MODE_NONE)
    {
      on_postpone_button_clicked();
    }
  return TRUE;
}


//! The postpone button was clicked.
void
BreakWindow::on_postpone_button_clicked()
{
  TRACE_ENTER("BreakWindow::on_postpone_button_clicked");
  ICore::Ptr core = Backend::get_core();
  IBreak::Ptr b = core->get_break(break_id);

  b->postpone_break();
  resume_non_ignorable_break();

  TRACE_EXIT();
}



//! The skip button was clicked.
void
BreakWindow::on_skip_button_clicked()
{
  TRACE_ENTER("BreakWindow::on_postpone_button_clicked");
  ICore::Ptr core = Backend::get_core();
  IBreak::Ptr b = core->get_break(break_id);

  b->skip_break();
  resume_non_ignorable_break();

  TRACE_EXIT();
}

//! The lock button was clicked.
void
BreakWindow::on_lock_button_clicked()
{
  IGUI *gui = GUI::get_instance();
  assert(gui != nullptr);
  gui->interrupt_grab();
  System::execute(System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN);
}


void
BreakWindow::resume_non_ignorable_break()
{
  TRACE_ENTER("BreakWindow::resume_non_ignorable_break");
  ICore::Ptr core = Backend::get_core();
  OperationMode mode = core->get_operation_mode();

  TRACE_MSG("break flags " << break_flags);

  if (! (break_flags & BreakWindow::BREAK_FLAGS_USER_INITIATED) &&
      mode == OperationMode::Normal)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          TRACE_MSG("Break " << id << ": check ignorable");

          bool ignorable = GUIConfig::break_ignorable(BreakId(id))();
          if (!ignorable)
            {
              TRACE_MSG("Break " << id << " not ignorable");

              ICore::Ptr core = Backend::get_core();
              IBreak::Ptr b = core->get_break(BreakId(id));

              if (b->get_elapsed_time() > b->get_limit())
                {
                  TRACE_MSG("Break " << id << " not ignorable and overdue");

                  core->force_break(BreakId(id), BREAK_HINT_NONE);
                  break;
                }
            }
        }
    }
}

//! Control buttons.
Gtk::Box *
BreakWindow::create_bottom_box(bool lockable,
                                  bool shutdownable)
{
  Gtk::HBox *box = nullptr;

  accel_group = Gtk::AccelGroup::create();
  add_accel_group(accel_group);

  if ((break_flags != BREAK_FLAGS_NONE) || lockable || shutdownable)
    {
      box = new Gtk::HBox(false, 6);

      if (break_flags != BREAK_FLAGS_NONE)
        {
          Gtk::HButtonBox *button_box = new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6);
          if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
            {
              skip_button = create_skip_button();
              button_box->pack_end(*skip_button, Gtk::PACK_SHRINK, 0);
            }

          if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
            {
              postpone_button = create_postpone_button();
              button_box->pack_end(*postpone_button, Gtk::PACK_SHRINK, 0);
            }
          box->pack_end(*button_box, Gtk::PACK_SHRINK, 0);
        }

      if (lockable || shutdownable)
        {
          if (shutdownable)
            {
              sysoper_combobox = create_sysoper_combobox();
              if (sysoper_combobox != nullptr)
                {
                  box->pack_end(*sysoper_combobox, Gtk::PACK_SHRINK, 0);
                }
            }
          else
            {
              lock_button = create_lock_button();
              if (lock_button != nullptr)
                {
                  box->pack_end(*lock_button, Gtk::PACK_SHRINK, 0);
                }
            }
        }
    }

  return box;
}

void
BreakWindow::init()
{
  TRACE_ENTER("BreakWindow::init");
  init_gui();
  TRACE_EXIT();
}

static void
disable_button_focus(GtkWidget *w)
{
	if (GTK_IS_CONTAINER(w))
  {
		gtk_container_forall(GTK_CONTAINER(w),(GtkCallback)disable_button_focus, nullptr);
  }

  if (GTK_IS_BUTTON(w))
  {
    gtk_widget_set_can_focus(w, FALSE);
  }
}

void
BreakWindow::start()
{
  TRACE_ENTER("BreakWindow::start");


  update_break_window();
  center();

#ifdef PLATFORM_OS_WINDOWS
  if (desktop_window)
    desktop_window->set_visible(true);
#endif
  show_all();

  // Set window hints.
  set_skip_pager_hint(true);
  set_skip_taskbar_hint(true);
  WindowHints::set_always_on_top(this, true);
  raise();

#ifdef PLATFORM_OS_WINDOWS
  // if( force_focus_on_break_start && this->head.count == 0)
  //   {
  //     HWND hwnd = (HWND) GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  //     W32ForceFocus::ForceWindowFocus(hwnd);
  //   }
#endif

  // In case the show_all resized the window...
  center();

  if (sysoper_combobox != nullptr)
    {
      // Setting "can focus" of the sysoper combobox to false is not enough to
      // prevent the combobox from taking the focus. A combobox has an internal
      // button that still has 'can focus' set to true.
      // So, unset 'can focus' of this button...
      disable_button_focus(GTK_WIDGET(sysoper_combobox->gobj()));

      // ...and clear the focus of the break window, which already focussed
      // the button.
      GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(sysoper_combobox->gobj()));
      if (gtk_widget_is_toplevel (toplevel))
        {
          gtk_window_set_focus(GTK_WINDOW(toplevel), nullptr);
        }
    }

  TRACE_EXIT();
}

void
BreakWindow::stop()
{
  TRACE_ENTER("BreakWindow::stop");

  if (frame != nullptr)
    {
      frame->set_frame_flashing(0);
    }

  hide();

#ifdef PLATFORM_OS_WINDOWS
  if (desktop_window)
    desktop_window->set_visible(false);
#endif

  TRACE_EXIT();
}

void
BreakWindow::refresh()
{
  TRACE_ENTER("BreakWindow::refresh");

  update_break_window();

// #ifdef PLATFORM_OS_WINDOWS
//   W32Compat::RefreshBreakWindow( *this );
// #endif
  TRACE_EXIT();
}

Glib::RefPtr<Gdk::Window>
BreakWindow::get_gdk_window()
{
  return get_window();
}

void
BreakWindow::update_break_window()
{
}

bool
BreakWindow::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  cr->save();
   if (block_mode == GUIConfig::BLOCK_MODE_ALL)
     {
       cr->set_source_rgba(0, 0, 0, 0.95);
     }
   else
     {
      cr->set_source_rgba(0.1, 0.1, 0.1, 0.4);
     }
  cr->set_operator(Cairo::OPERATOR_SOURCE);
  cr->paint();
  cr->restore();

  return Gtk::Window::on_draw(cr);
}

void
BreakWindow::on_screen_changed(const Glib::RefPtr<Gdk::Screen>& previous_screen)
{
  (void) previous_screen;

  const Glib::RefPtr<Gdk::Screen> screen = get_screen();
  const Glib::RefPtr<Gdk::Visual> visual = screen->get_rgba_visual();

  if (visual)
    {
      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual->gobj());
    }
}
