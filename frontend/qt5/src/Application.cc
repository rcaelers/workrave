// Application.cc --- The WorkRave GUI
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

#include "Application.hh"

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

// Library includes
#include "config/IConfigurator.hh"

// Backend includes.
#include "IBreak.hh"
#include "ICore.hh"
#include "utils/Exception.hh"

// Frontend common
#include "CoreFactory.hh"
#include "SoundPlayer.hh"

#include "GUIConfig.hh"

// DBus
//#if defined(HAVE_DBUS)
//#if defined(interface)
//#undef interface
//#endif
//#include "dbus/DBus.hh"
//#include "dbus/DBusException.hh"
//#endif

using namespace std;


Application::Ptr
Application::create(int argc, char **argv, IToolkit::Ptr toolkit)
{
  return Ptr(new Application(argc, argv, toolkit));
}

//! GUI Constructor.
/*!
 *  \param argc number of command line parameters.
 *  \param argv all command line parameters.
 */
Application::Application(int argc, char **argv, IToolkit::Ptr toolkit) :
  toolkit(toolkit),
  sound_player(NULL),
  active_prelude_count(0),
  active_break_id(BREAK_ID_NONE),
  //main_window(NULL),
  //menus(0),
  //break_window_destroy(false),
  //prelude_window_destroy(false),
  //heads(NULL),
  //screen_width(-1),
  //screen_height(-1),
#if defined(PLATFORM_OS_UNIX)
  //grab_wanted(false),
#endif
  //grab_handle(NULL),
  //status_icon(NULL),
  //applet_control(NULL),
  muted(false)
  //closewarn_shown(false)
{
  TRACE_ENTER("GUI:GUI");

  this->argc = argc;
  this->argv = argv;

  TRACE_EXIT();
}


//! Destructor.
Application::~Application()
{
  TRACE_ENTER("GUI:~GUI");

  //ungrab();

  core.reset();

  delete sound_player;

  TRACE_EXIT();
}


//! Forces a restbreak.
void
Application::restbreak_now()
{
  core->force_break(BREAK_ID_REST_BREAK, BREAK_HINT_USER_INITIATED);
}


//! The main entry point.
void
Application::main()
{
  TRACE_ENTER("Application::main");

  init_core();

  menus = Menus::create(shared_from_this(), toolkit, core);

  // TODO: Toolkit
  toolkit->init(menus->get_top());
  
  //init_nls();
  //init_platform();
  //init_sound_player();
  //init_multihead();
  //init_dbus();
  init_session();
  //init_gui();
  //init_startup_warnings();

  toolkit->signal_timer().connect(boost::bind(&Application::on_timer, this)); 
  on_timer();

  TRACE_MSG("Initialized. Entering event loop.");

  // Enter the event loop
  toolkit->run();
  
  //cleanup_session();

  TRACE_EXIT();
}


//! Terminates the GUI.
void
Application::terminate()
{
  TRACE_ENTER("Application::terminate");

  CoreFactory::get_configurator()->save();

  //collect_garbage();

  TRACE_EXIT();
}


//! Periodic heartbeat.
bool
Application::on_timer()
{
  core->heartbeat();
  
  if (!break_windows.empty() && muted)
    {
      bool user_active = core->is_user_active();
 
      if (user_active)
        {
          sound_player->restore_mute();
          muted = false;
        }
    }
  
 return true;
}

//void
//Application::init_platform()
//{
//  TRACE_ENTER("Application::init_platform");
//
//#if defined(PLATFORM_OS_UNIX)
//  char *display = gdk_get_display();
//  System::init(display);
//  g_free(display);
//#else
//  System::init();
//#endif
//
//  srand((unsigned int)time(NULL));
//  TRACE_EXIT();
//}


void
Application::init_session()
{
 TRACE_ENTER("Application::init_session");

 session = Session::create();
 session->init();

 TRACE_EXIT();
}

//! Initializes i18n.
//void
//Application::init_nls()
//{
//#if defined(ENABLE_NLS)
//  string language = GUIConfig::get_locale();
//  if (language != "")
//    {
//      g_setenv("LANGUAGE", language.c_str(), 1);
//    }
//
//#  if !defined(HAVE_GTK3)
//  gtk_set_locale();
//#  endif
//  const char *locale_dir;
//
//#if defined(PLATFORM_OS_WIN32)
//  string dir = Util::get_application_directory();
//  // Use the pre-install locale location if workrave is running from its MSVC build directory.
//  dir += Util::file_exists( dir + "\\..\\Workrave.sln" ) ? "\\..\\frontend" : "\\lib\\locale";
//  locale_dir = dir.c_str();
//#elif defined(PLATFORM_OS_OSX)
//  char locale_path[MAXPATHLEN * 4];
//  char execpath[MAXPATHLEN+1];
//  uint32_t pathsz = sizeof (execpath);
//
//  _NSGetExecutablePath(execpath, &pathsz);
//
//  gchar *dir_path = g_path_get_dirname(execpath);
//  strcpy(locale_path, dir_path);
//  g_free(dir_path);
//
//  // Locale
//  strcat(locale_path, "/../Resources/locale");
//  locale_dir = locale_path;
//#else
//  locale_dir = GNOMELOCALEDIR;
//#  endif
//
//#  ifdef HAVE_SETLOCALE
//  setlocale(LC_ALL, "");
//#  endif
//
//#if defined(PLATFORM_OS_WIN32)
//  bindtextdomain("gtk20", locale_dir);
//  bindtextdomain("iso_3166", locale_dir);
//  bindtextdomain("iso_639", locale_dir);
//  bindtextdomain("glib20", locale_dir);
//  bind_textdomain_codeset("gk20", "UTF-8");
//  bind_textdomain_codeset("glib20", "UTF-8");
//  bind_textdomain_codeset("iso_3166", "UTF-8");
//  bind_textdomain_codeset("iso_639", "UTF-8");
//
//  CoreFactory::get_configurator()->add_listener(GUIConfig::CFG_KEY_LOCALE, this);
//#endif
//
//  bindtextdomain(GETTEXT_PACKAGE, locale_dir);
//  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
//  textdomain(GETTEXT_PACKAGE);
//
//#endif
//}


//! Initializes the core.
void
Application::init_core()
{
  string display_name;

  core = CoreFactory::get_core();
  core->init(argc, argv, this, toolkit->get_display_name());

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      IBreak::Ptr b = core->get_break(BreakId(i));
      b->signal_break_event().connect(boost::bind(&Application::on_break_event, this, BreakId(i), _1));
    }

  core->signal_operation_mode_changed().connect(boost::bind(&Application::on_operation_mode_changed, this, _1)); 

  GUIConfig::init();
}


//void
//Application::init_multihead()
//{
//  TRACE_ENTER("Application::init_multihead");
//
//  init_gtk_multihead();
//  if (num_heads == -1)
//    {
//      init_multihead_mem(1);
//
//      heads[0].valid = false;
//      heads[0].count = 0;
//      heads[0].geometry.set_width(gdk_screen_width());
//      heads[0].geometry.set_height(gdk_screen_height());
//      heads[0].geometry.set_x(0);
//      heads[0].geometry.set_y(0);
//    }
//
//  init_multihead_desktop();
//  TRACE_EXIT();
//}

//void
//Application::init_multihead_mem(int new_num_heads)
//{
//  TRACE_ENTER("Application::init_multihead_mem");
//  if (new_num_heads != num_heads || num_heads <= 0)
//    {
//      delete [] heads;
//      heads = new HeadInfo[new_num_heads];
//
//      PreludeWindow **old_prelude_windows = prelude_windows;
//      IBreakWindow **old_break_windows = break_windows;
//
//      prelude_windows = new PreludeWindow*[new_num_heads];/* LEAK */
//      break_windows = new IBreakWindow*[new_num_heads]; /* LEAK */
//
//      int max_heads = new_num_heads > num_heads ? new_num_heads : num_heads;
//
//      // Copy existing breaks windows.
//      for (int i = 0; i < max_heads; i++)
//        {
//          if (i < new_num_heads)
//            {
//              if (i < num_heads)
//                {
//                  prelude_windows[i] = old_prelude_windows[i];
//                  break_windows[i] = old_break_windows[i];
//                }
//              else
//                {
//                  prelude_windows[i] = NULL;
//                  break_windows[i] = NULL;
//                }
//            }
//
//          if (new_num_heads < num_heads && i >= new_num_heads)
//            {
//              // Number of heads get smaller,
//              // destroy breaks/preludes
//              if (old_prelude_windows != NULL &&
//                  old_prelude_windows[i] != NULL)
//                {
//                  old_prelude_windows[i]->destroy();
//                }
//              if (old_break_windows != NULL &&
//                  old_break_windows[i] != NULL)
//                {
//                  old_break_windows[i]->destroy();
//                }
//            }
//        }
//
//      if (active_prelude_count > new_num_heads)
//        {
//          active_prelude_count = new_num_heads;
//        }
//
//      if (active_break_count > new_num_heads)
//        {
//          active_break_count = new_num_heads;
//        }
//
//      delete [] old_prelude_windows;
//      delete [] old_break_windows;
//
//      num_heads = new_num_heads;
//    }
//  TRACE_EXIT();
//}
//
//void
//Application::init_multihead_desktop()
//{
//  TRACE_ENTER("Application::init_multihead_desktop");
//  TRACE_MSG("gdk width x height " << gdk_screen_width() << " " << gdk_screen_height());
//
//  int width = 0;
//  int height = 0;
//
//  // Use head info to determine screen size. I hope this results
//  // in the same size as the gdk_screen_xxx....
//  for (int i = 0; i < num_heads; i++)
//    {
//      if (!heads[i].valid)
//        {
//          // Not all heads have valid geometry. Use gdk.
//          width = gdk_screen_width();
//          height = gdk_screen_height();
//          break;
//        }
//
//      int w = heads[i].geometry.get_x() + heads[i].geometry.get_width();
//      int h = heads[i].geometry.get_y() + heads[i].geometry.get_height();
//
//      if (w > width)
//        {
//          width = w;
//        }
//      if (h > height)
//        {
//          height = h;
//        }
//    }
//
//  TRACE_MSG("width x height " << width << " " << height);
//  if (screen_width != width || screen_height != height)
//    {
//      if (main_window != NULL)
//        {
//          main_window->relocate_window(width, height);
//        }
//      screen_width = width;
//      screen_height = height;
//    }
//}
//
//
//void
//Application::init_gtk_multihead()
//{
//  TRACE_ENTER("Application::init_gtk_multihead");
//
//  int new_num_heads = 0;
//
//  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
//  int num_screens = display->get_n_screens();
//
//  TRACE_MSG("screens = " << num_screens);
//  if (num_screens >= 1)
//    {
//      for (int i = 0; i < num_screens; i++)
//        {
//          Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
//          if (screen)
//            {
//              new_num_heads += screen->get_n_monitors();
//              TRACE_MSG("num monitors on screen " << i << " = " << screen->get_n_monitors());
//            }
//        }
//
//      for (int i = 0; i < num_screens; i++)
//        {
//          Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
//          if (screen)
//            {
//              TRACE_MSG("num monitors on screen " << i << " = " << screen->get_n_monitors());
//            }
//        }
//
//      init_multihead_mem(new_num_heads);
//
//      int count = 0;
//      for (int i = 0; i < num_screens; i++)
//        {
//          Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
//          if (screen)
//            {
//              int num_monitors = screen->get_n_monitors();
//              TRACE_MSG("monitors = " << num_monitors);
//              for (int j = 0; j < num_monitors && count < new_num_heads; j++)
//                {
//                  Gdk::Rectangle rect;
//                  screen->get_monitor_geometry(j, rect);
//
//                  bool overlap = false;
//                  for (int k = 0; !overlap && k < count; k++)
//                    {
//                      Gdk::Rectangle irect = rect;
//
//                      if (heads[k].screen->get_number() == i)
//                        {
//                          irect.intersect(heads[k].geometry, overlap);
//                        }
//                    }
//
//                  if (!overlap)
//                    {
//                      heads[count].screen = screen;
//                      heads[count].monitor = j;
//                      heads[count].valid = true;
//                      heads[count].count = count;
//
//                      heads[count].geometry = rect;
//                      count++;
//                    }
//
//                  TRACE_MSG("Screen #" << i << " Monitor #" << j << "  "
//                            << rect.get_x() << " "
//                            << rect.get_y() << " "
//                            << rect.get_width() << " "
//                            << rect.get_height() << " "
//                            << " intersects " << overlap);
//                }
//            }
//        }
//      num_heads = count;
//      TRACE_MSG("# Heads = " << num_heads);
//    }
//  TRACE_EXIT();
//}

////! Initializes the GUI
//void
//Application::init_gui()
//{
//  workrave::dbus::DBus::Ptr dbus = CoreFactory::get_dbus();
//
//  if (dbus && dbus->is_available())
//    {
//    }
//
//  // Periodic timer.
//  //Glib::signal_timeout().connect(sigc::mem_fun(*this, &Application::on_timer), 1000);
//}


//void
//Application::init_dbus()
//{
//  workrave::dbus::DBus::Ptr dbus = CoreFactory::get_dbus();
//
//  if (dbus && dbus->is_available())
//    {
//      if (dbus->is_running("org.workrave.Workrave"))
//        {
//          // Gtk::MessageDialog dialog(_("Workrave failed to start"),
//          //                           false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
//          // dialog.set_secondary_text(_("Is Workrave already running?"));
//          // dialog.show();
//          // dialog.run();
//          exit(1);
//        }
//
//      try
//        {
//          dbus->register_object_path("/org/workrave/Workrave/UI");
//          dbus->register_service("org.workrave.Workrave");
//          
//          extern void init_DBusGUI(workrave::dbus::DBus *dbus);
//          init_DBusGUI(dbus.get());
//
//          //dbus->connect("/org/workrave/Workrave/UI",
//          //              "org.workrave.ControlInterface",
//          //              menus);
//        }
//      catch (workrave::dbus::DBusException &)
//        {
//        }
//    }
//}

//void
//Application::init_startup_warnings()
//{
//  OperationMode mode = core->get_operation_mode();
//  if (mode != OPERATION_MODE_NORMAL)
//    {
//      Glib::signal_timeout().connect(sigc::mem_fun(*this, &Application::on_operational_mode_warning_timer), 5000);
//    }
//}


//! Initializes the sound player.
void
Application::init_sound_player()
{
  TRACE_ENTER("GUI:init_sound_player");
  try
    {
      // Tell pulseaudio were are playing sound events
      g_setenv("PULSE_PROP_media.role", "event", TRUE);

      sound_player = new SoundPlayer(); /* LEAK */
      sound_player->init();
    }
  catch (workrave::utils::Exception)
    {
      TRACE_MSG("No sound");
    }
  TRACE_EXIT();
}


void
Application::on_break_event(BreakId break_id, IBreak::BreakEvent event)
{
  TRACE_ENTER_MSG("Application::on_break_event", break_id << " " << event);

  struct EventMap
  {
    BreakId id;
    IBreak::BreakEvent break_event;
    SoundEvent sound_event;
  } event_map[] =
      {
        { BREAK_ID_MICRO_BREAK, IBreak::BREAK_EVENT_PRELUDE_STARTED, SOUND_BREAK_PRELUDE },
        { BREAK_ID_MICRO_BREAK, IBreak::BREAK_EVENT_BREAK_IGNORED,   SOUND_BREAK_IGNORED },
        { BREAK_ID_MICRO_BREAK, IBreak::BREAK_EVENT_BREAK_STARTED,   SOUND_MICRO_BREAK_STARTED },
        { BREAK_ID_MICRO_BREAK, IBreak::BREAK_EVENT_BREAK_ENDED,     SOUND_MICRO_BREAK_ENDED },
        { BREAK_ID_REST_BREAK,  IBreak::BREAK_EVENT_PRELUDE_STARTED, SOUND_BREAK_PRELUDE },
        { BREAK_ID_REST_BREAK,  IBreak::BREAK_EVENT_BREAK_IGNORED,   SOUND_BREAK_IGNORED },
        { BREAK_ID_REST_BREAK,  IBreak::BREAK_EVENT_BREAK_STARTED,   SOUND_REST_BREAK_STARTED },
        { BREAK_ID_REST_BREAK,  IBreak::BREAK_EVENT_BREAK_ENDED,     SOUND_REST_BREAK_ENDED },
        { BREAK_ID_DAILY_LIMIT, IBreak::BREAK_EVENT_PRELUDE_STARTED, SOUND_BREAK_PRELUDE},
        { BREAK_ID_DAILY_LIMIT, IBreak::BREAK_EVENT_BREAK_IGNORED,   SOUND_BREAK_IGNORED},
        { BREAK_ID_DAILY_LIMIT, IBreak::BREAK_EVENT_BREAK_STARTED,   SOUND_MICRO_BREAK_ENDED },
      };

  if (sound_player != NULL)
    {
      for (unsigned int i = 0; i < sizeof(event_map)/sizeof(EventMap); i++)
        {
          if (event_map[i].id == break_id && event_map[i].break_event == event)
            {
              bool mute = false;
              SoundEvent snd = event_map[i].sound_event;
              TRACE_MSG("play " << event);

              CoreFactory::get_configurator()->get_value(SoundPlayer::CFG_KEY_SOUND_MUTE, mute);
              if (mute)
                {
                  muted = true;
                }
              TRACE_MSG("Mute after playback " << mute);
              sound_player->play_sound(snd, mute);
            }
        }
    }
}

//   if (event == CORE_EVENT_MONITOR_FAILURE)
//     {
//       string msg = _("Workrave could not monitor your keyboard and mouse activity.\n");

// #ifdef PLATFORM_OS_UNIX
//       msg += _("Make sure that the RECORD extension is enabled in the X server.");
// #endif
//       Gtk::MessageDialog dialog(_("Workrave failed to start"),
//                                 false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
//       dialog.set_secondary_text(msg);
//       dialog.show();
//       dialog.run();
//       terminate();
//     }
//   TRACE_EXIT();
// }


void
Application::on_operation_mode_changed(const OperationMode m)
{
  // if (status_icon)
  //   {
  //     status_icon->set_operation_mode(m);
  //   }
}

void
Application::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("Application::config_changed_notify", key);

#if defined(HAVE_LANGUAGE_SELECTION)
  if (key == GUIConfig::CFG_KEY_LOCALE)
    {
      string locale = GUIConfig::get_locale();
      Locale::set_locale(locale);

      // menus->locale_changed();
    }
#endif

  TRACE_EXIT();
}

void
Application::create_prelude_window(BreakId break_id)
{
  hide_break_window();
  //init_multihead();
  //collect_garbage();

  active_break_id = break_id;

  for (int i = 0; i < toolkit->get_screen_count(); i++)
    {
      prelude_windows.push_back(toolkit->create_prelude_window(i, break_id));
    }
}


void
Application::create_break_window(BreakId break_id, BreakHint break_hint)
{
  TRACE_ENTER("Application::start_break_window");
  hide_break_window();
  //init_multihead();
  //collect_garbage();

  IBreakWindow::BreakFlags break_flags = IBreakWindow::BREAK_FLAGS_NONE;
  bool ignorable = GUIConfig::get_ignorable(break_id);
  bool skippable = GUIConfig::get_skippable(break_id);

  if (break_hint & BREAK_HINT_USER_INITIATED)
  {
      break_flags = ( IBreakWindow::BREAK_FLAGS_POSTPONABLE |
                      IBreakWindow::BREAK_FLAGS_USER_INITIATED);

      if (skippable)
        {
          break_flags |=  IBreakWindow::BREAK_FLAGS_SKIPPABLE;
        }
    }
  else
    { 
      if (ignorable)
        {
          break_flags |= IBreakWindow::BREAK_FLAGS_POSTPONABLE;
        }

      if(skippable)
        {
          break_flags |= IBreakWindow::BREAK_FLAGS_SKIPPABLE;
        }
    }

  if (break_hint & BREAK_HINT_NATURAL_BREAK)
    {
      break_flags |=  (IBreakWindow::BREAK_FLAGS_NO_EXERCISES | IBreakWindow::BREAK_FLAGS_NATURAL |
                       IBreakWindow::BREAK_FLAGS_POSTPONABLE);
    }

  active_break_id = break_id;

  for (int i = 0; i < toolkit->get_screen_count(); i++)
    {
      IBreakWindow::Ptr break_window = toolkit->create_break_window(i, break_id, break_flags);
      break_window->init();
      break_windows.push_back(break_window);
    }

  TRACE_EXIT();
}

void
Application::hide_break_window()
{
  TRACE_ENTER("Application::hide_break_window");
  active_break_id = BREAK_ID_NONE;

  for (PreludeWindowsIter i = prelude_windows.begin(); i != prelude_windows.end(); i++)
    {
      (*i)->stop();
    }

  for (BreakWindowsIter i = break_windows.begin(); i != break_windows.end(); i++)
    {
      (*i)->stop();
    }

  toolkit->ungrab();

  TRACE_EXIT();
}


void
Application::show_break_window()
{
  TRACE_ENTER("Application::hide_break_window");

  for (PreludeWindowsIter i = prelude_windows.begin(); i != prelude_windows.end(); i++)
    {
      (*i)->start();
    }
  for (BreakWindowsIter i = break_windows.begin(); i != break_windows.end(); i++)
    {
      (*i)->start();
    }

  if (GUIConfig::get_block_mode() != GUIConfig::BLOCK_MODE_NONE)
    {
      toolkit->grab();
    }

  TRACE_EXIT();
}


void
Application::refresh_break_window()
{
  for (PreludeWindowsIter i = prelude_windows.begin(); i != prelude_windows.end(); i++)
    {
      (*i)->refresh();
    }
  for (BreakWindowsIter i = break_windows.begin(); i != break_windows.end(); i++)
    {
      (*i)->refresh();
    }
}


void
Application::set_break_progress(int value, int max_value)
{
  for (PreludeWindowsIter i = prelude_windows.begin(); i != prelude_windows.end(); i++)
    {
      (*i)->set_progress(value, max_value);
    }

  for (BreakWindowsIter i = break_windows.begin(); i != break_windows.end(); i++)
    {
      (*i)->set_progress(value, max_value);
    }
}


void
Application::set_prelude_stage(PreludeStage stage)
{
  for (PreludeWindowsIter i = prelude_windows.begin(); i != prelude_windows.end(); i++)
    {
      (*i)->set_stage(stage);
    }
}


void
Application::set_prelude_progress_text(PreludeProgressText text)
{
  for (PreludeWindowsIter i = prelude_windows.begin(); i != prelude_windows.end(); i++)
    {
      (*i)->set_progress_text(text);
    }
}


//bool
//Application::on_operational_mode_warning_timer()
//{
//  OperationMode mode = core->get_operation_mode();
//  if (mode == OPERATION_MODE_SUSPENDED)
//    {
//      status_icon->show_balloon("operation_mode",
//                                _("Workrave is in suspended mode. "
//                                  "Mouse and keyboard activity will not be monitored."));
//    }
//  else if (mode == OPERATION_MODE_QUIET)
//    {
//      status_icon->show_balloon("operation_mode",
//                                _("Workrave is in quiet mode. "
//                                  "No break windows will appear."));
//    }
//  return false;
//}

//HeadInfo &
//Application::get_head(int head)
//{
//  return heads[head < num_heads ? head : 0];
//}
//
//int
//Application::map_to_head(int &x, int &y)
//{
//  int head = -1;
//
//  for (int i = 0; i < num_heads; i++)
//    {
//      int left, top, width, height;
//
//      left = heads[i].get_x();
//      top = heads[i].get_y();
//      width = heads[i].get_width();
//      height = heads[i].get_height();
//
//      if (x >= left && y >= top && x < left + width && y < top + height)
//        {
//          x -= left;
//          y -= top;
//
//          // Use coordinates relative to right and butto edges of the
//          // screen if the mainwindow is closer to those edges than to
//          // the left/top edges.
//
//          if (x >= width / 2)
//            {
//              x -= width;
//            }
//          if (y >= height / 2)
//            {
//              y -= height;
//            }
//          head = i;
//          break;
//        }
//    }
//
//  if (head < 0)
//    {
//      head = 0;
//      x = y = 256;
//    }
//  return head;
//}
//
//void
//Application::map_from_head(int &x, int &y, int head)
//{
//  HeadInfo &h = get_head(head);
//  if (x < 0)
//    {
//      x += h.get_width();
//    }
//  if (y < 0)
//    {
//      y += h.get_height();
//    }
//
//  x += h.get_x();
//  y += h.get_y();
//}
//
//
//bool
//Application::bound_head(int &x, int &y, int width, int height, int &head)
//{
//  bool ret = false;
//
//  if (head >= num_heads)
//    {
//      head = 0;
//    }
//
//  HeadInfo &h = get_head(head);
//  if (x < - h.get_width())
//    {
//      x = 0;
//      ret = true;
//    }
//  if (y < - h.get_height())
//    {
//      y = 0;
//      ret = true;
//    }
//
//  // Make sure something remains visible..
//  if (x > - 10 && x < 0)
//    {
//      x = - 10;
//      ret = true;
//    }
//  if (y > - 10 && y < 0)
//    {
//      y = - 10;
//      ret = true;
//    }
//
//  if (x + width >= h.get_width())
//    {
//      x = h.get_width() - width - 10;
//      ret = true;
//    }
//
//  if (y + height >= h.get_height())
//    {
//      y = h.get_height() - height - 10;
//      ret = true;
//    }
//
//  return ret;
//}
