// BreakWindow.cc --- base class for the break windows
//
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

#ifdef PLATFORM_OS_WIN32
#include "W32Compat.hh"
#include "W32ForceFocus.hh"
#include <gdk/gdkwin32.h>
#endif

#include <gdk/gdkkeysyms.h>

#include "preinclude.h"
#include "debug.hh"
#include "nls.h"

#ifdef PLATFORM_OS_WIN32_NATIVE
#undef max
#endif

#include <gtkmm.h>
#include <math.h>

#include "BreakWindow.hh"

#include "GUI.hh"
#include "IBreak.hh"
#include "IBreakResponse.hh"
#include "GtkUtil.hh"
#include "WindowHints.hh"
#include "Frame.hh"
#include "System.hh"
#include "Util.hh"
#include "ICore.hh"
#include "IConfigurator.hh"
#include "CoreFactory.hh"

#if defined(PLATFORM_OS_WIN32)
#include "DesktopWindow.hh"
#elif defined(PLATFORM_OS_UNIX)
#include "desktop-window.h"
#endif

using namespace workrave;


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
  frame(NULL),
  break_response(NULL),
  break_id(break_id),
  gui(NULL),
  visible(false),
  sysoper_model_columns(NULL),
  accel_added(false),
  accel_group(NULL),
  lock_button(NULL),
  postpone_button(NULL),
  skip_button(NULL),
  sysoper_combobox(NULL)
#ifdef PLATFORM_OS_WIN32
  ,
  desktop_window( NULL ),
  force_focus_on_break_start( false ),
  parent( 0 )
#endif
{
  TRACE_ENTER("BreakWindow::BreakWindow");

  if (mode != GUIConfig::BLOCK_MODE_NONE)
    {
      // Disable titlebar to appear like a popup
      set_decorated(false);
      set_skip_taskbar_hint(true);
      set_skip_pager_hint(true);

#ifdef HAVE_GTK3
      if (GtkUtil::running_on_wayland())
        {
          set_app_paintable(true);
          signal_draw().connect(sigc::mem_fun(*this, &BreakWindow::on_draw));
          signal_screen_changed().connect(sigc::mem_fun(*this, &BreakWindow::on_screen_changed));
          on_screen_changed(get_screen());
          set_size_request(head.get_width(), head.get_height());
        }
#endif
    }
#ifdef HAVE_GTK3
  else
    {
      if (GtkUtil::running_on_wayland())
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

#ifdef PLATFORM_OS_WIN32
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

#ifdef PLATFORM_OS_WIN32
  if( W32ForceFocus::GetForceFocusValue() )
      initial_ignore_activity = true;

  CoreFactory::get_configurator()->get_value_with_default(
    "advanced/force_focus_on_break_start",
    force_focus_on_break_start,
    true
    );
#endif

  ICore *core = CoreFactory::get_core();
  assert(core != NULL);
  core->set_insist_policy(initial_ignore_activity ?
                          ICore::INSIST_POLICY_IGNORE :
                          ICore::INSIST_POLICY_HALT);
  TRACE_EXIT();
}


//! Init GUI
void
BreakWindow::init_gui()
{
  if (gui == NULL)
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

          if (block_mode == GUIConfig::BLOCK_MODE_ALL && !GtkUtil::running_on_wayland())
            {
#ifdef PLATFORM_OS_WIN32
              desktop_window = new DesktopWindow(head);
              add(*window_frame);

#elif defined(PLATFORM_OS_UNIX)
              set_size_request(head.get_width(),
                               head.get_height());
              set_app_paintable(true);
#ifdef HAVE_GTK3
              Glib::RefPtr<Gdk::Window> window = get_window();
              set_desktop_background(window->gobj());
#else
              set_desktop_background(GTK_WIDGET(gobj())->window);
#endif
              Gtk::Alignment *align
                = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0.0, 0.0));
              align->add(*window_frame);
              add(*align);
#endif
            }
          else
            {
              if (GtkUtil::running_on_wayland())
                {
                  Gtk::Alignment *align
                    = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0.0, 0.0));
                  align->add(*window_frame);
                  add(*align);
                }
              else
                {
                  add(*window_frame);
                }
            }
        }

      // FIXME: check if it was intentionally not unset for RB
      if (break_id != BREAK_ID_REST_BREAK)
        {
#ifdef HAVE_GTK3
          set_can_focus(false);
#else
          unset_flags(Gtk::CAN_FOCUS);
#endif
        }

      show_all_children();
      stick();
    }
}



//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTER("BreakWindow::~BreakWindow");

  if (frame != NULL)
    {
      frame->set_frame_flashing(0);
    }

#ifdef PLATFORM_OS_WIN32
  delete desktop_window;
#endif
  TRACE_EXIT();
}


//! Centers the window.
void
BreakWindow::center()
{
  GtkUtil::center_window(*this, head);
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
          string icon_path = Util::complete_directory(icon_name,
                                            Util::SEARCH_PATH_IMAGES);
          Glib::RefPtr<Gdk::Pixbuf> img = Gdk::Pixbuf::create_from_file(icon_path);
          row[sysoper_model_columns->icon] = img;
      } catch (Glib::Error &e) {
          TRACE_MSG2("Cannot read image:", icon_name);
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
      return NULL;
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
      sysoper_model_columns = NULL;
      TRACE_EXIT();
      return NULL;
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
  assert(gui != NULL);
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
#ifdef HAVE_GTK3
  ret->set_can_focus(false);
#else
  GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
#endif
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
#ifdef HAVE_GTK3
  ret->set_can_focus(false);
#else
  GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
#endif
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
#ifdef HAVE_GTK3
  ret->set_can_focus(false);
#else
  GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
#endif
  return ret;
}

void
BreakWindow::update_skip_postpone_sensitivity()
{
  if (postpone_button != NULL && !postpone_button->get_sensitive())
    {
      if (!is_non_ignorable_break_overdue())
        {
          if (postpone_button != NULL)
            {
              postpone_button->set_has_tooltip(false);
              postpone_button->set_sensitive(true);
            }
        }
    }

  if (skip_button != NULL && !skip_button->get_sensitive())
    {
      if (!is_non_skipable_break_overdue())
        {
          if (skip_button != NULL)
            {
              skip_button->set_has_tooltip(false);
              skip_button->set_sensitive(true); 
            }
        }
    }
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


//! Break response
inline void
BreakWindow::set_response(IBreakResponse *bri)
{
  break_response = bri;
}


//! The postpone button was clicked.
void
BreakWindow::on_postpone_button_clicked()
{
  TRACE_ENTER("BreakWindow::on_postpone_button_clicked");
  if (break_response != NULL)
    {
      break_response->postpone_break(break_id);
    }
  TRACE_EXIT();
}



//! The skip button was clicked.
void
BreakWindow::on_skip_button_clicked()
{
  if (break_response != NULL)
    {
      break_response->skip_break(break_id);
    }
}

//! The lock button was clicked.
void
BreakWindow::on_lock_button_clicked()
{
  IGUI *gui = GUI::get_instance();
  assert(gui != NULL);
  gui->interrupt_grab();
  System::execute(System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN);
}


bool
BreakWindow::is_non_ignorable_break_overdue()
{
  TRACE_ENTER("BreakWindow::is_non_ignorable_break_overdue");
  ICore *core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();

  if (! (break_flags & BreakWindow::BREAK_FLAGS_USER_INITIATED) && mode == OPERATION_MODE_NORMAL)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          if (!GUIConfig::get_ignorable(BreakId(id)))
            {
              IBreak *b = core->get_break(BreakId(id));
              if (b->get_elapsed_time() > b->get_limit())
                {
                  return true;
                }
            }
        }
    }
  return false;
}

bool
BreakWindow::is_non_skipable_break_overdue()
{
  TRACE_ENTER("BreakWindow::is_non_skipable_break_overdue");
  ICore *core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();

  if (! (break_flags & BreakWindow::BREAK_FLAGS_USER_INITIATED) && mode == OPERATION_MODE_NORMAL)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          if (!GUIConfig::get_skippable(BreakId(id)))
            {
              IBreak *b = core->get_break(BreakId(id));
              if (b->get_elapsed_time() > b->get_limit())
                {
                  return true;
                }
            }
        }
    }
  return false;
}


//! Control buttons.
Gtk::Box *
BreakWindow::create_bottom_box(bool lockable,
                               bool shutdownable)
{
  Gtk::HBox *box = NULL;

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
              bool enabled = !is_non_skipable_break_overdue();

              skip_button = create_skip_button();
              skip_button->set_sensitive(enabled);
              if (!enabled)
                {
                  const char *msg = _("You cannot skip this break while another non-skippable break is overdue.");
                  skip_button->set_tooltip_text(msg);
                }
              button_box->pack_end(*skip_button, Gtk::PACK_SHRINK, 0);
            }

          if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
            {
              bool enabled = !is_non_ignorable_break_overdue();

              postpone_button = create_postpone_button();
              postpone_button->set_sensitive(enabled);
              if (!enabled)
                {
                  const char *msg = _("You cannot postpone this break while another non-postponable break is overdue.");
                  postpone_button->set_tooltip_text(msg);
                }
              button_box->pack_end(*postpone_button, Gtk::PACK_SHRINK, 0);
            }
          box->pack_end(*button_box, Gtk::PACK_SHRINK, 0);
        }

      //#ifdef HAVE_GTK3
      if (lockable || shutdownable)
        {
          if (shutdownable)
            {
              sysoper_combobox = create_sysoper_combobox();
              if (sysoper_combobox != NULL)
                {
                  box->pack_end(*sysoper_combobox, Gtk::PACK_SHRINK, 0);
                }
            }
          else
            {
              lock_button = create_lock_button();
              if (lock_button != NULL)
                {
                  box->pack_end(*lock_button, Gtk::PACK_SHRINK, 0);
                }
            }
        }
      //#endif

    }

  return box;
}


//! Starts the daily limit.
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
		gtk_container_forall(GTK_CONTAINER(w),	(GtkCallback)disable_button_focus, NULL);
  }

  if (GTK_IS_BUTTON(w))
  {
    gtk_widget_set_can_focus(w, FALSE);
  }
}

//! Starts the daily limit.
void
BreakWindow::start()
{
  TRACE_ENTER("BreakWindow::start");

  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize_if_needed();

  // Set some window hints.
  set_skip_pager_hint(true);
  set_skip_taskbar_hint(true);

  WindowHints::set_always_on_top(this, true);

  update_break_window();
  center();

#ifdef PLATFORM_OS_WIN32
  if (desktop_window)
    desktop_window->set_visible(true);
#endif
  show_all();

  WindowHints::set_always_on_top(this, true);

#ifdef PLATFORM_OS_WIN32
  if( force_focus_on_break_start && ( this->head.count == 0 ) )
    {
      HWND hwnd = (HWND)GDK_WINDOW_HWND( Gtk::Widget::gobj()->window );
      bool focused = W32ForceFocus::ForceWindowFocus( hwnd );
      bool this_is_a_dummy_var_to_fool_visual_studio_debugger = focused;
    }
#endif

  // In case the show_all resized the window...
  center();

  if (sysoper_combobox != NULL)
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
          gtk_window_set_focus(GTK_WINDOW(toplevel), NULL);
        }
    }

  TRACE_EXIT();
}

//! Stops the daily limit.
void
BreakWindow::stop()
{
  TRACE_ENTER("BreakWindow::stop");

  if (frame != NULL)
    {
      frame->set_frame_flashing(0);
    }

#ifdef HAVE_GTK3
  hide();
#else
  hide_all();
#endif
  visible = false;

#ifdef PLATFORM_OS_WIN32
  if (desktop_window)
    desktop_window->set_visible(false);
#endif

  TRACE_EXIT();
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  IBreakWindow. it is NOT possible to do a delete on
 *  this interface...
 */
void
BreakWindow::destroy()
{
  delete this;
}

//! Refresh
void
BreakWindow::refresh()
{
  TRACE_ENTER("BreakWindow::refresh");

  update_skip_postpone_sensitivity();

  update_break_window();

#ifdef PLATFORM_OS_WIN32
  W32Compat::RefreshBreakWindow( *this );
#endif
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

#ifdef HAVE_GTK3
bool BreakWindow::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  cr->save();
  if (block_mode == GUIConfig::BLOCK_MODE_ALL)
    {
      cr->set_source_rgba(0, 0, 0, 0.95);
    }
  else
    {
      cr->set_source_rgba(0.1, 0.1, 0.1, 0.1);
    }
  cr->set_operator(Cairo::OPERATOR_SOURCE);
  cr->paint();
  cr->restore();
 
  return Gtk::Window::on_draw(cr);
}

void BreakWindow::on_screen_changed(const Glib::RefPtr<Gdk::Screen>& previous_screen)
{
  (void) previous_screen;

  const Glib::RefPtr<Gdk::Screen> screen = get_screen();
  const Glib::RefPtr<Gdk::Visual> visual = screen->get_rgba_visual();

  if (visual)
    {
      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual->gobj());
    }
}
#endif
