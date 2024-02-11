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

#include "Toolkit.hh"

#include "DailyLimitWindow.hh"
#include "GtkUtil.hh"
#include "MicroBreakWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "commonui/credits.h"
#include "commonui/nls.h"
#include "debug.hh"
#include "ui/GUIConfig.hh"

using namespace workrave;
using namespace workrave::config;

Toolkit::Toolkit(int argc, char **argv)
  : argc(argc)
  , argv(argv)
{
  TRACE_ENTRY();
}

Toolkit::~Toolkit()
{
  TRACE_ENTRY();
  delete main_window;
#if defined(HAVE_STATUSICON)
  delete status_icon;
#endif
  delete statistics_dialog;
  delete preferences_dialog;
  delete debug_dialog;
  delete exercises_dialog;
  delete about_dialog;
}

void
Toolkit::init(std::shared_ptr<IApplicationContext> app)
{
  TRACE_ENTRY();
  this->app = app;

  gapp = Gtk::Application::create(argc, argv, "org.workrave.Workrave");

  menu_model = app->get_menu_model();
  sound_theme = app->get_sound_theme();
  exercises = app->get_exercises();

  main_window = new MainWindow(app);

  gapp->signal_startup().connect([this]() {
    gapp->add_window(*main_window);
    gapp->hold();
    gapp->register_application();
  });

  event_connections.emplace_back(main_window->signal_closed().connect(sigc::mem_fun(*this, &Toolkit::on_main_window_closed)));

#if defined(HAVE_STATUSICON)
  auto status_icon_menu = std::make_shared<ToolkitMenu>(menu_model);
  status_icon_menu->get_menu()->attach_to_widget(*main_window);
  status_icon = new StatusIcon(app, status_icon_menu);
  status_icon->init();

  event_connections.emplace_back(
    status_icon->signal_activated().connect(sigc::mem_fun(*this, &Toolkit::on_status_icon_activated)));
  event_connections.emplace_back(
    status_icon->signal_balloon_activated().connect(sigc::mem_fun(*this, &Toolkit::on_status_icon_balloon_activated)));
#endif

  Glib::signal_timeout().connect(sigc::mem_fun(*this, &Toolkit::on_timer), 1000);

  init_multihead();
  init_gui();
  init_debug();

  gtk_icon_theme_add_resource_path(gtk_icon_theme_get_default(), "/workrave/icons/scalable");
}

void
Toolkit::deinit()
{
  gapp->remove_window(*main_window);
  get_locker()->unlock();
}

void
Toolkit::terminate()
{
  gapp->quit();
}

void
Toolkit::hold()
{
  TRACE_ENTRY_PAR(hold_count);
  hold_count++;
  main_window->set_can_close(hold_count > 0);
}

void
Toolkit::release()
{
  TRACE_ENTRY_PAR(hold_count);
  hold_count--;
  main_window->set_can_close(hold_count > 0);
}

bool
Toolkit::can_close() const
{
  return hold_count > 0;
}

void
Toolkit::run()
{
  gapp->run();
}

HeadInfo
Toolkit::get_head_info(int screen_index) const
{
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  Glib::RefPtr<Gdk::Monitor> monitor = display->get_monitor(screen_index);

  HeadInfo head;
  head.primary = monitor->is_primary();
  head.monitor = monitor;
  monitor->get_geometry(head.geometry);
  logger->info("Display #{}: primary={} x={} y={} w={} h={}",
               screen_index,
               head.primary,
               head.geometry.get_x(),
               head.geometry.get_y(),
               head.geometry.get_width(),
               head.geometry.get_height());
  return head;
}

int
Toolkit::get_head_count() const
{
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  logger->info("Toolkit reported # displays : {}", display->get_n_monitors());
  return display->get_n_monitors();
}

IBreakWindow::Ptr
Toolkit::create_break_window(int screen_index, BreakId break_id, BreakFlags break_flags)
{
  IBreakWindow::Ptr ret;

  HeadInfo head = get_head_info(screen_index);

  BlockMode block_mode = GUIConfig::block_mode()();

  if (break_id == BREAK_ID_MICRO_BREAK)
    {
      ret = std::make_shared<MicroBreakWindow>(app, head, break_flags, block_mode);
    }
  else if (break_id == BREAK_ID_REST_BREAK)
    {
      ret = std::make_shared<RestBreakWindow>(app, head, break_flags, block_mode);
    }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      ret = std::make_shared<DailyLimitWindow>(app, head, break_flags, block_mode);
    }

  return ret;
}

IPreludeWindow::Ptr
Toolkit::create_prelude_window(int screen_index, workrave::BreakId break_id)
{
  HeadInfo head = get_head_info(screen_index);
  return std::make_shared<PreludeWindow>(head, break_id);
}

void
Toolkit::show_window(WindowType type)
{
  switch (type)
    {
    case WindowType::Main:
      show_main_window();
      break;

    case WindowType::Statistics:
      show_statistics();
      break;

    case WindowType::Preferences:
      show_preferences();
      break;

    case WindowType::About:
      show_about();
      break;

    case WindowType::Exercises:
      show_exercises();
      break;

    case WindowType::Debug:
      show_debug();
      break;
    }
}

void
Toolkit::show_about()
{
  if (about_dialog == nullptr)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = GtkUtil::create_pixbuf("workrave.png");
      about_dialog = new Gtk::AboutDialog;

      about_dialog->set_name("Workrave");
      std::vector<Glib::ustring> authors;
      for (int index = 0; workrave_authors[index] != nullptr; index++)
        {
          authors.emplace_back(workrave_authors[index]);
        }
      about_dialog->set_authors(authors);
      about_dialog->set_copyright(workrave_copyright);
      about_dialog->set_comments(
        _("This program assists in the prevention and recovery"
          " of Repetitive Strain Injury (RSI)."));
      about_dialog->set_logo(pixbuf);
      about_dialog->set_translator_credits(workrave_translators);

#if defined(WORKRAVE_GIT_VERSION)
      about_dialog->set_version(WORKRAVE_VERSION "\n(" WORKRAVE_GIT_VERSION ")");
#else
      about_dialog->set_version(WORKRAVE_VERSION);
#endif
      about_dialog->set_website("http://www.workrave.org/");
      about_dialog->set_website_label("www.workrave.org");

      about_dialog->signal_response().connect([this](int reponse) {
        about_dialog->hide();
        delete about_dialog;
        about_dialog = nullptr;
      });
    }
  about_dialog->present();
}

void
Toolkit::show_debug()
{
  if (debug_dialog == nullptr)
    {
      debug_dialog = new DebugDialog();
      debug_dialog->signal_response().connect([this](int reponse) {
        debug_dialog->hide();
        delete debug_dialog;
        debug_dialog = nullptr;
      });
      debug_dialog->run();
    }
  else
    {
      debug_dialog->present();
    }
}

void
Toolkit::show_exercises()
{
  if (exercises_dialog == nullptr)
    {
      exercises_dialog = new ExercisesDialog(sound_theme, exercises);
      exercises_dialog->signal_response().connect([this](int reponse) {
        exercises_dialog->hide();
        delete exercises_dialog;
        exercises_dialog = nullptr;
      });
      exercises_dialog->run();
    }
  else
    {
      exercises_dialog->present();
    }
}

void
Toolkit::show_main_window()
{
  TRACE_ENTRY();
  main_window->open_window();
  main_window->show();
  main_window->raise();
  GUIConfig::timerbox_enabled("main_window").set(true);
}

void
Toolkit::show_preferences()
{
  if (preferences_dialog == nullptr)
    {
      preferences_dialog = new PreferencesDialog(app);
      preferences_dialog->signal_response().connect([this](int reponse) {
        preferences_dialog->hide();
        delete preferences_dialog;
        preferences_dialog = nullptr;
      });
    }
  else
    {
      preferences_dialog->present();
    }
}

void
Toolkit::show_statistics()
{
  if (statistics_dialog == nullptr)
    {
      statistics_dialog = new StatisticsDialog(app);
      statistics_dialog->signal_response().connect([this](int reponse) {
        statistics_dialog->hide();
        delete statistics_dialog;
        statistics_dialog = nullptr;
      });

      statistics_dialog->run();
    }
  else
    {
      statistics_dialog->present();
    }
}

boost::signals2::signal<void()> &
Toolkit::signal_timer()
{
  return timer_signal;
}

boost::signals2::signal<void()> &
Toolkit::signal_main_window_closed()
{
  return main_window_closed_signal;
}

boost::signals2::signal<void(bool)> &
Toolkit::signal_session_idle_changed()
{
  return session_idle_changed_signal;
}

boost::signals2::signal<void()> &
Toolkit::signal_session_unlocked()
{
  return session_unlocked_signal;
}

boost::signals2::signal<void()> &
Toolkit::signal_status_icon_activated()
{
  return status_icon_activated_signal;
}

const char *
Toolkit::get_display_name() const
{
  auto *x = gdk_display_get_default();
  return gdk_display_get_name(x);
}

void
Toolkit::create_oneshot_timer(int ms, std::function<void()> func)
{
  if (ms == 0)
    {
      Glib::signal_idle().connect([func]() {
        func();
        return false;
      });
    }
  else
    {
      Glib::signal_timeout().connect(
        [func]() {
          func();
          return false;
        },
        ms);
    }
}

void
Toolkit::show_notification(const std::string &id,
                           const std::string &title,
                           const std::string &balloon,
                           std::function<void()> func)
{
  notify_add_confirm_function(id, func);
#if defined(HAVE_STATUSICON)
  status_icon->show_balloon(id, balloon);
#endif
}

void
Toolkit::show_tooltip(const std::string &tip)
{
#if defined(HAVE_STATUSICON)
  status_icon->set_tooltip(tip);
#endif
}

void
Toolkit::attach_menu(Gtk::Menu *menu)
{
  menu->attach_to_widget(*main_window);
}

void
Toolkit::init_multihead()
{
  TRACE_ENTRY();
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  Glib::RefPtr<Gdk::Screen> screen = display->get_default_screen();
}

void
Toolkit::init_gui()
{
#if defined(IS_THIS_NEEDED_FOR_GTK3)
  static const gchar *rc_string = {
    "style \"progressBarWidth\"\n"
    "{\n"
    "   GtkProgressBar::min-horizontal-bar-width = 10\n"
    "   GtkProgressBar::min-horizontal-bar-height = 2\n"
    "}\n"
    "\n"
    "widget \"*.locked-progress\" style \"progressBarWidth\"\n"
    // "class \"GtkProgressBar\" style \"progressBarWidth\"\n"
  };

  gtk_rc_parse_string(rc_string);

#endif
#if defined(PLATFORM_OS_WINDOWS)
  auto settings = Gtk::Settings::get_default();
  settings->property_gtk_application_prefer_dark_theme().set_value(GUIConfig::theme_dark()());
  std::string theme_name = GUIConfig::theme_name()();
  if (!theme_name.empty())
    {
      settings->property_gtk_theme_name().set_value(theme_name);
    }

  settings->property_gtk_application_prefer_dark_theme().signal_changed().connect(
    [settings]() { GUIConfig::theme_dark().set(settings->property_gtk_application_prefer_dark_theme().get_value()); });
  settings->property_gtk_theme_name().signal_changed().connect(
    [settings]() { GUIConfig::theme_name().set(settings->property_gtk_theme_name().get_value()); });

  GUIConfig::theme_dark().connect(tracker, [settings](auto dark) {
    settings->property_gtk_application_prefer_dark_theme().set_value(dark);
  });
  GUIConfig::theme_name().connect(tracker, [settings](auto name) { settings->property_gtk_theme_name().set_value(name); });
#endif
}

#if defined(NDEBUG)
static void
my_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
}
#endif

void
Toolkit::init_debug()
{
#if defined(NDEBUG)
  TRACE_ENTRY();
  const char *domains[] = {NULL, "Gtk", "GLib", "Gdk", "gtkmm", "GLib-GObject"};
  for (unsigned int i = 0; i < sizeof(domains) / sizeof(char *); i++)
    {
      g_log_set_handler(domains[i],
                        (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION),
                        my_log_handler,
                        NULL);
    }
#endif
}

// void
// Menus::locale_changed()
// {
//   static bool syncing = false;
//   if (syncing)
//     return;
//   syncing = true;

//   for (int i = 0; i < MENU_SIZEOF; i++)
//     {
//       if (menus[i] != NULL)
//         {
//           menus[i]->init();
//         }
//     }

//   resync();

//   syncing = false;
// }

bool
Toolkit::on_timer()
{
  timer_signal();
  main_window->update();
  return true;
}

void
Toolkit::on_main_window_closed()
{
  if (can_close())
    {
      GUIConfig::timerbox_enabled("main_window").set(false);
      main_window_closed_signal();
    }
  else
    {
      terminate();
    }
}

void
Toolkit::on_status_icon_balloon_activated(const std::string &id)
{
  TRACE_ENTRY();
  notify_confirm(id);
}

void
Toolkit::on_status_icon_activated()
{
  TRACE_ENTRY();
  status_icon_activated_signal();
}

void
Toolkit::notify_add_confirm_function(const std::string &id, std::function<void()> func)
{
  TRACE_ENTRY();
  notifiers[id] = func;
}

void
Toolkit::notify_confirm(const std::string &id)
{
  TRACE_ENTRY();
  if (notifiers.find(id) != notifiers.end())
    {
      notifiers[id]();
    }
}
