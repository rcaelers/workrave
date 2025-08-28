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

#include <algorithm>

#include "DailyLimitWindow.hh"
#include "GtkUtil.hh"
#include "MicroBreakWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "commonui/credits.h"
#include "commonui/nls.h"
#include "debug.hh"
#include "ui/GUIConfig.hh"
#include "config/IConfigurator.hh"
#include "utils/AssetPath.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::utils;

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
Toolkit::preinit(std::shared_ptr<IConfigurator> config)
{
  TRACE_ENTRY();
}

void
Toolkit::init(std::shared_ptr<IApplicationContext> app)
{
  TRACE_ENTRY();
  this->app = app;

  gapp = Gtk::Application::create(argc, argv, "org.workrave.Workrave");
  init_css();

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

std::optional<HeadInfo>
Toolkit::get_head_info(int screen_index) const
{
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  if (!display)
    {
      logger->error("Failed to get default display");
      return {};
    }

  std::vector<Glib::RefPtr<Gdk::Monitor>> unique_monitors = get_unique_monitors();

  if (screen_index < 0 || screen_index >= static_cast<int>(unique_monitors.size()))
    {
      logger->error("Invalid screen index {} (available: {})", screen_index, unique_monitors.size());
      return {};
    }

  Glib::RefPtr<Gdk::Monitor> monitor = unique_monitors[screen_index];
  if (!monitor)
    {
      logger->error("Failed to get monitor for screen index {}", screen_index);
      return {};
    }

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
  std::vector<Glib::RefPtr<Gdk::Monitor>> unique_monitors = get_unique_monitors();
  logger->info("Toolkit reported # displays : {} (after filtering duplicates)", unique_monitors.size());

  return static_cast<int>(unique_monitors.size());
}

IBreakWindow::Ptr
Toolkit::create_break_window(int screen_index, BreakId break_id, BreakFlags break_flags)
{
  IBreakWindow::Ptr ret;

  auto optional_head = get_head_info(screen_index);
  if (!optional_head)
    {
      logger->error("Failed to retrieve monitor info for screen index {}", screen_index);
      return nullptr;
    }
  HeadInfo head = *optional_head;

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
  auto optional_head = get_head_info(screen_index);
  if (!optional_head)
    {
      logger->error("Failed to retrieve monitor info for screen index {}", screen_index);
      return nullptr;
    }
  HeadInfo head = *optional_head;
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
      about_dialog->set_website("https://www.workrave.org/");
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

void
Toolkit::init_css()
{
  try
    {
      auto provider = Gtk::CssProvider::create();
      provider->load_from_resource("/workrave/ui/default.css");

      auto screen = Gdk::Screen::get_default();
      if (!screen)
        {
          spdlog::error("Failed to get default screen for CSS provider");
          return;
        }

      Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

      std::string css_file = AssetPath::complete_directory("user.css", SearchPathId::Config);
      if (std::filesystem::is_regular_file(css_file))
        {
          spdlog::info("Loading user CSS: {}", css_file);
          auto user_provider = Gtk::CssProvider::create();
          user_provider->load_from_path(css_file);
          Gtk::StyleContext::add_provider_for_screen(screen, user_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
        }
    }
  catch (const Glib::Exception &ex)
    {
      spdlog::error("Failed to load CSS: {}", ex.what().c_str());
    }
  catch (const std::exception &ex)
    {
      spdlog::error("Exception while loading CSS: {}", ex.what());
    }
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
  if (notifiers.contains(id))
    {
      notifiers[id]();
    }
}

std::vector<Glib::RefPtr<Gdk::Monitor>>
Toolkit::get_unique_monitors() const
{
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  if (!display)
    {
      logger->error("Failed to get default display");
      return {};
    }

  std::vector<Glib::RefPtr<Gdk::Monitor>> monitors;
  int n_monitors = display->get_n_monitors();

  for (int i = 0; i < n_monitors; ++i)
    {
      if (auto monitor = display->get_monitor(i))
        {
          Gdk::Rectangle geometry;
          monitor->get_geometry(geometry);

          auto is_duplicate = std::ranges::any_of(monitors, [&geometry](const auto &existing) {
            Gdk::Rectangle existing_geometry;
            existing->get_geometry(existing_geometry);
            return geometry.get_x() == existing_geometry.get_x() && geometry.get_y() == existing_geometry.get_y()
                   && geometry.get_width() == existing_geometry.get_width()
                   && geometry.get_height() == existing_geometry.get_height();
          });

          if (!is_duplicate)
            {
              monitors.push_back(monitor);
            }
        }
    }

  return monitors;
}
