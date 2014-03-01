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
#ifdef PLATFORM_OS_WIN32
  /*
    Windows will have a gtk toplevel window regardless of mode.
    Hopefully this takes care of the phantom parent problem.
    Also, the break window title now appears on the taskbar, and
    it will show up in Windows Task Manager's application list.
  */
  Gtk::Window( Gtk::WINDOW_TOPLEVEL ),
#else
  Gtk::Window(mode==GUIConfig::BLOCK_MODE_NONE
              ? Gtk::WINDOW_TOPLEVEL
              : Gtk::WINDOW_POPUP),
#endif
  block_mode(mode),
  break_flags(break_flags),
  frame(NULL),
  break_response(NULL),
  gui(NULL),
  visible(false),
  accel_added(false),
  accel_group(NULL),
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
  this->break_id = break_id;

  if (mode != GUIConfig::BLOCK_MODE_NONE)
    {
      // Disable titlebar to appear like a popup
      set_decorated(false);
      set_skip_taskbar_hint(true);
      set_skip_pager_hint(true);
    }

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

  // trace window handles:
  // FIXME: debug, remove later
#if defined( PLATFORM_OS_WIN32 ) && defined( TRACING )
  HWND _hwnd = (HWND) GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  HWND _scope = (HWND) GDK_WINDOW_HWND(gtk_widget_get_window(GTK_WIDGET( this->gobj())));
  HWND _hRoot = GetAncestor( _hwnd, GA_ROOT );
  HWND _hParent = GetAncestor( _hwnd, GA_PARENT );
  HWND _hDesktop = GetDesktopWindow();

  TRACE_MSG("BreakWindow created" <<  hex << _hwnd << dec);
  if (_hwnd != _scope)
    {
      TRACE_MSG("!!! Scope issue: " << hex << _scope << dec);
    }

  if (_hwnd != _hRoot)
    {
      TRACE_MSG("GetDesktopWindow()" <<  hex << _hDesktop << dec);
      TRACE_MSG("!!! BreakWindow GA_ROOT: " << hex << _hRoot << dec);
    }

  if (_hParent != _hDesktop)
    {
      TRACE_MSG("GetDesktopWindow()" <<  hex << _hDesktop << dec);
      TRACE_MSG("!!! PreludeWindow GA_PARENT: " << hex << _hParent << dec);

      HWND _hTemp;
      while( IsWindow( _hParent ) && _hParent != _hDesktop )
        {
          _hTemp = _hParent;
          _hParent = GetAncestor( _hTemp, GA_PARENT );
          HWND _hParent2 = (HWND)GetWindowLong( _hTemp, GWL_HWNDPARENT );
          if( _hParent == _hTemp )
            break;
          TRACE_MSG("!!!" <<  hex << _hTemp << " GA_PARENT: " << hex << _hParent  << dec);
          TRACE_MSG("!!!" <<  hex << _hTemp << " GWL_HWNDPARENT: " << hex << _hParent2  << dec);
        }
    }

#endif

  this->head = head;
  if (head.valid)
    {
      Gtk::Window::set_screen(head.screen);
    }

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

          if (block_mode == GUIConfig::BLOCK_MODE_ALL)
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
              add(*window_frame);
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
BreakWindow::create_sysoper_combobox(bool shutdownable)
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
      if (shutdownable || iter->type == System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN)
        {
          append_row_to_sysoper_model(model,
              iter->type);
        }
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
#ifdef HAVE_GTK3
  comboBox->set_can_focus(false);
#else
  GTK_WIDGET_UNSET_FLAGS(comboBox->gobj(), GTK_CAN_FOCUS);
#endif

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
      resume_non_ignorable_break();
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
      resume_non_ignorable_break();
    }
}


void
BreakWindow::resume_non_ignorable_break()
{
  TRACE_ENTER("BreakWindow::resume_non_ignorable_break");
  ICore *core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();

  TRACE_MSG("break flags " << break_flags);
  
  if (! (break_flags & BreakWindow::BREAK_FLAGS_USER_INITIATED) &&
      mode == OPERATION_MODE_NORMAL)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          TRACE_MSG("Break " << id << ": check ignorable");

          bool ignorable = GUIConfig::get_ignorable(BreakId(id));
          if (!ignorable)
            {
              TRACE_MSG("Break " << id << " not ignorable");

              IBreak *b = core->get_break(BreakId(id));
              assert(b != NULL);

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

      //#ifdef HAVE_GTK3
      if (lockable)
        {
          sysoper_combobox = create_sysoper_combobox(shutdownable);
          if (sysoper_combobox != NULL)
            {
              box->pack_end(*sysoper_combobox, Gtk::PACK_SHRINK, 0);
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

//! Starts the daily limit.
void
BreakWindow::start()
{
  TRACE_ENTER("BreakWindow::start");

  update_break_window();
  center();

#ifdef PLATFORM_OS_WIN32
  if (desktop_window)
    desktop_window->set_visible(true);
#endif
  show_all();

  // Set window hints.
  set_skip_pager_hint(true);
  set_skip_taskbar_hint(true);
  WindowHints::set_always_on_top(this, true);
  raise();

#ifdef PLATFORM_OS_WIN32
  if( force_focus_on_break_start && this->head.valid && ( this->head.count == 0 ) )
    {
      HWND hwnd = (HWND)GDK_WINDOW_HWND( Gtk::Widget::gobj()->window );
      bool focused = W32ForceFocus::ForceWindowFocus( hwnd );
      bool this_is_a_dummy_var_to_fool_visual_studio_debugger = focused;
    }
#endif

  // In case the show_all resized the window...
  center();
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
