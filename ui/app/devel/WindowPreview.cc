// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#include "WindowPreview.hh"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef HAVE_APP_GTK
#  include <gtkmm.h>
#else
#  include <QApplication>
#  include <QQmlContext>
#  include <QQuickView>
#  include <QScreen>
#endif

#include "Context.hh"
#include "DebugDialog.hh"
#include "MainWindow.hh"
#include "Menus.hh"
#include "RpcWindowPreviewServiceImpl.hh"
#include "ToolkitFactory.hh"
#include "ToolkitMenu.hh"

#include "config/ConfiguratorFactory.hh"
#include "core/CoreConfig.hh"
#include "core/ICore.hh"
#include "rpc/RpcServer.hh"
#include "session/System.hh"
#include "stats/IStatistics.hh"
#include "ui/GUIConfig.hh"
#include "ui/Plugin.hh"
#include "utils/AssetPath.hh"
#include "utils/Paths.hh"

#ifdef HAVE_APP_GTK
#  include "ExercisesDialog.hh"
#  include "PreferencesDialog.hh"
#  include "StatisticsDialog.hh"
#  include "widgets/Frame.hh"
#else
#  include "QmlAboutDialog.hh"
#  include "QmlExercisesDialog.hh"
#  include "QmlPrefsDialog.hh"
#  include "QmlRestBreakWindow.hh"
#  include "QmlStatisticsDialog.hh"
#endif

#ifdef PLATFORM_OS_WINDOWS
#  include "ui/windows/WindowsCompat.hh"
#  include "ui/windows/WindowsForceFocus.hh"
#endif

#if defined(PLATFORM_OS_WINDOWS) && defined(HAVE_APP_GTK)
#  include <shellapi.h>

#  include "ui/windows/WindowsStatusIcon.hh"
#endif

#ifdef HAVE_APP_GTK
namespace
{
#  include "commonui/credits.h"
}
#endif

struct WindowPreviewSettings
{
  struct Timer
  {
    int elapsed{0};
    int limit{180};
    int idle{0};
    int duration{30};
    bool enabled{true};
  };

  std::array<Timer, 3> timers{
    {
      {.elapsed = 0, .limit = 180, .idle = 0, .duration = 30, .enabled = true},
      {.elapsed = 0, .limit = 2700, .idle = 0, .duration = 600, .enabled = true},
      {.elapsed = 0, .limit = 14400, .idle = 0, .duration = 0, .enabled = true},
    },
  };
  bool user_active{false};
  int exercise_index{-1};
  int exercise_elapsed{0};
  workrave::BreakId locking_break{workrave::BREAK_ID_NONE};
  BreakButtonState buttons;
};

using namespace workrave;
using namespace workrave::stats;

namespace
{
  class PreviewStatistics final : public IStatistics
  {
  public:
    PreviewStatistics()
      : date(date_of(local_now()))
    {
      day.start = to_local_time(date, std::chrono::hours{9});
      day.stop = to_local_time(date, std::chrono::hours{17});
      day.total_active_time.set(std::chrono::seconds{21600});
      for (int i = 0; i < 3; ++i)
        {
          day.break_stats.at(i).at(workrave::utils::underlying_cast(BreakStatValue::Prompted)).set(12 - (i * 4));
          day.break_stats.at(i).at(workrave::utils::underlying_cast(BreakStatValue::UniqueBreaks)).set(10 - (i * 4));
          day.break_stats.at(i).at(workrave::utils::underlying_cast(BreakStatValue::Taken)).set(8 - (i * 3));
          day.break_stats.at(i).at(workrave::utils::underlying_cast(BreakStatValue::NaturalTaken)).set(2);
          day.break_stats.at(i).at(workrave::utils::underlying_cast(BreakStatValue::Skipped)).set(1);
          day.break_stats.at(i).at(workrave::utils::underlying_cast(BreakStatValue::Postponed)).set(2);
          day.total_overdue.at(i).set(std::chrono::seconds{60 + (i * 30)});
        }
    }

    void start_new_day() override
    {
    }

    bool delete_all_history() override
    {
      day = {};
      return true;
    }

    void update() override
    {
    }

    void save() override
    {
    }

    DailyStats *get_current_day() const override
    {
      return &day;
    }

    StatValue<int64_t> &break_counter(BreakId id, BreakStatValue type) override
    {
      return day.break_stats.at(id).at(workrave::utils::underlying_cast(type));
    }

    StatValue<std::chrono::seconds> &total_overdue(BreakId id) override
    {
      return day.total_overdue.at(id);
    }

    StatValue<std::chrono::seconds> &total_active_time() override
    {
      return day.total_active_time;
    }

    std::optional<DailyStats> get_day(const Date &d) const override
    {
      return d == date ? std::optional(day) : std::nullopt;
    }

    std::vector<Date> get_dates(const Date &from, const Date &to) const override
    {
      return from <= date && date <= to ? std::vector<Date>{date} : std::vector<Date>{};
    }

    std::optional<Date> get_previous_date(const Date &d) const override
    {
      return d > date ? std::optional(date) : std::nullopt;
    }

    std::optional<Date> get_next_date(const Date &d) const override
    {
      return d < date ? std::optional(date) : std::nullopt;
    }

    std::optional<Date> get_first_date() const override
    {
      return date;
    }

    std::optional<Date> get_last_date() const override
    {
      return date;
    }

    std::chrono::seconds get_total_active_time(const Date &from, const Date &to) const override
    {
      return from <= date && date <= to ? day.total_active_time.get() : std::chrono::seconds{0};
    }

  private:
    Date date{};
    mutable DailyStats day;
  };

  class PreviewBreak final : public IBreak
  {
  public:
    PreviewBreak(BreakId id, std::shared_ptr<WindowPreviewSettings> settings)
      : id(id)
      , settings(std::move(settings))
    {
    }

    boost::signals2::signal<void(BreakEvent)> &signal_break_event() override
    {
      return events;
    }

    std::string get_name() const override
    {
      static constexpr std::array names{"microbreak", "restbreak", "dailylimit"};
      return names.at(id);
    }
#ifndef HAVE_CORE_NEXT
    BreakId get_id() const override
    {
      return id;
    }
#endif
    bool is_enabled() const override
    {
      return timer().enabled;
    }

    bool is_running() const override
    {
      return settings->user_active;
    }

    bool is_taking() const override
    {
      return false;
    }

    bool is_max_preludes_reached() const override
    {
      return settings->locking_break == id;
    }

    bool is_active() const override
    {
      return timer().enabled;
    }

    int64_t get_elapsed_time() const override
    {
      return settings->locking_break == id ? timer().limit + 1 : timer().elapsed;
    }

    int64_t get_elapsed_idle_time() const override
    {
      return settings->locking_break == id ? settings->buttons.lock_value : timer().idle;
    }

    int64_t get_auto_reset() const override
    {
      return settings->locking_break == id ? settings->buttons.lock_max : timer().duration;
    }

    bool is_auto_reset_enabled() const override
    {
      return timer().duration > 0;
    }

    int64_t get_limit() const override
    {
      return timer().limit;
    }

    bool is_limit_enabled() const override
    {
      return timer().limit > 0;
    }

    int64_t get_total_overdue_time() const override
    {
      return std::max(0, timer().elapsed - timer().limit);
    }

    void postpone_break() override
    {
    }

    void skip_break() override
    {
    }

  private:
    const WindowPreviewSettings::Timer &timer() const
    {
      return settings->timers.at(id);
    }

    BreakId id;
    std::shared_ptr<WindowPreviewSettings> settings;
    boost::signals2::signal<void(BreakEvent)> events;
  };

  class PreviewCore final : public ICore
  {
  public:
    explicit PreviewCore(std::shared_ptr<WindowPreviewSettings> settings)
      : settings(std::move(settings))
    {
      for (int i = 0; i < 3; ++i)
        {
          breaks.at(i) = std::make_shared<PreviewBreak>(static_cast<BreakId>(i), this->settings);
        }
    }

    boost::signals2::signal<void(OperationMode)> &signal_operation_mode_changed() override
    {
      return modes;
    }

    boost::signals2::signal<void(UsageMode)> &signal_usage_mode_changed() override
    {
      return usages;
    }
#ifdef HAVE_CORE_NEXT
    void init(IApp * /*app*/, const char * /*display*/) override
    {
    }

    IBreak::Ptr get_break(BreakId id) const override
    {
      return breaks.at(id);
    }

    IStatistics::Ptr get_statistics() const override
    {
      return statistics;
    }
#else
    void init(int /*argc*/, char ** /*argv*/, IApp * /*app*/, const char * /*display*/) override
    {
    }

    IBreak *get_break(BreakId id) override
    {
      return breaks.at(id).get();
    }

    IBreak *get_break(std::string name) override
    {
      for (auto &b: breaks)
        {
          if (b->get_name() == name)
            {
              return b.get();
            }
        }
      return nullptr;
    }

    IStatistics *get_statistics() const override
    {
      return statistics.get();
    }

    void set_core_events_listener(ICoreEventListener * /*l*/) override
    {
    }

    void time_changed() override
    {
    }

    int64_t get_time() const override
    {
      return 0;
    }

    void post_event(CoreEvent /*event*/) override
    {
    }
#endif
    void heartbeat() override
    {
    }

    void force_break(BreakId /*id*/, workrave::utils::Flags<BreakHint> /*break_hint*/) override
    {
    }

    bool is_user_active() const override
    {
      return settings->user_active;
    }

    bool is_taking() const override
    {
      return false;
    }

    OperationMode get_active_operation_mode() override
    {
      return mode;
    }

    OperationMode get_regular_operation_mode() override
    {
      return mode;
    }

    void set_operation_mode(OperationMode value) override
    {
      mode = value;
      modes(mode);
    }

    void set_operation_mode_for(OperationMode value, std::chrono::minutes /*duration*/) override
    {
      set_operation_mode(value);
    }

    void set_operation_mode_override(OperationMode /*mode*/, const std::string & /*id*/) override
    {
    }

    void remove_operation_mode_override(const std::string & /*id*/) override
    {
    }

    bool is_operation_mode_an_override() override
    {
      return false;
    }

    UsageMode get_usage_mode() override
    {
      return usage;
    }

    void set_usage_mode(UsageMode value) override
    {
      usage = value;
      usages(usage);
    }

    void set_powersave(bool /*down*/) override
    {
    }

    void set_insist_policy(InsistPolicy /*p*/) override
    {
    }

    void force_idle() override
    {
    }

    void report_external_activity(std::string /*who*/, bool /*active*/) override
    {
    }

    ICoreHooks::Ptr get_hooks() const override
    {
      return nullptr;
    }

  private:
    std::shared_ptr<WindowPreviewSettings> settings;
    std::array<IBreak::Ptr, 3> breaks;
    std::shared_ptr<PreviewStatistics> statistics{std::make_shared<PreviewStatistics>()};
    OperationMode mode{OperationMode::Normal};
    UsageMode usage{UsageMode::Normal};
    boost::signals2::signal<void(OperationMode)> modes;
    boost::signals2::signal<void(UsageMode)> usages;
  };
} // namespace

namespace
{
  void validate(workrave::BreakId id, int progress, int duration)
  {
    if (id < workrave::BREAK_ID_MICRO_BREAK || id >= workrave::BREAK_ID_SIZEOF)
      {
        throw std::invalid_argument("Unknown break type");
      }
    if (duration < 2 || progress < 0 || progress >= duration)
      {
        throw std::invalid_argument("Require duration >= 2 and 0 <= progress < duration");
      }
  }
} // namespace

struct WindowPreview::Impl
{
  Impl(WindowPreview &owner, IToolkit::Ptr toolkit)
    : owner(owner)
    , toolkit(std::move(toolkit))
    , service(owner)
  {
  }

  void stop()
  {
    render_break = {};
    configured_exercises.clear();
    hide_windows();
    if (break_window)
      {
        break_window->stop();
        break_window.reset();
      }
    if (prelude_window)
      {
        prelude_window->stop();
        prelude_window.reset();
      }
  }

  uint64_t enqueue(std::function<void()> action)
  {
#if defined(PLATFORM_OS_WINDOWS) && defined(HAVE_APP_GTK)
    ++menu_generation;
    PostMessage(menu_owner, WM_CANCELMODE, 0, 0);
#endif
    uint64_t result = 0;
    {
      std::scoped_lock lock(mutex);
      if (closed)
        {
          throw std::invalid_argument("Preview session is closed");
        }
      if (pending.size() >= 16)
        {
          throw std::invalid_argument("Preview queue is full");
        }
      result = ++sequence;
      pending.emplace_back(result, std::move(action));
    }
    dispatch([weak = owner.weak_from_this()] {
      if (auto preview = weak.lock())
        {
          preview->process_pending();
        }
    });
    return result;
  }

  void dispatch(std::function<void()> action)
  {
#ifdef HAVE_APP_GTK
    g_idle_add_full(
      G_PRIORITY_DEFAULT,
      [](gpointer data) -> gboolean {
        (*static_cast<std::function<void()> *>(data))();
        return G_SOURCE_REMOVE;
      },
      new std::function<void()>(std::move(action)),
      [](gpointer data) { delete static_cast<std::function<void()> *>(data); });
#else
    QMetaObject::invokeMethod(qApp, [action = std::move(action)] { action(); }, Qt::QueuedConnection);
#endif
  }

  void find_main_window()
  {
    if (main != nullptr)
      {
        return;
      }
#ifdef HAVE_APP_GTK
    for (auto *window: Gtk::Window::list_toplevels())
#else
    for (auto *window: QApplication::topLevelWidgets())
#endif
      {
        if (auto *candidate = dynamic_cast<MainWindow *>(window))
          {
            main = candidate;
            return;
          }
      }
  }

  void hide_windows()
  {
#ifdef HAVE_APP_GTK
    if (menu)
      {
        menu->get_menu()->popdown();
      }
    menu.reset();
    dialog.reset();
#else
    if (menu)
      {
        auto *popup = menu->get_menu();
        popup->hide();
        menu.reset();
        delete popup;
      }
    dialog.reset();
    debug.reset();
#endif
    find_main_window();
    GUIConfig::timerbox_enabled("main_window").set(false);
    if (main != nullptr)
      {
        main->hide();
      }
  }

#ifdef HAVE_APP_GTK
  void configure_widget(Gtk::Widget *widget)
  {
    if (auto *stack = dynamic_cast<Gtk::Stack *>(widget))
      {
        stack->set_transition_type(Gtk::STACK_TRANSITION_TYPE_NONE);
      }
    if (auto *frame = dynamic_cast<Frame *>(widget))
      {
        frame->set_flashing_enabled(false);
      }
    if (auto *exercises = dynamic_cast<ExercisesPanel *>(widget))
      {
        if (configured_exercises.insert(exercises).second)
          {
            exercises->seek(settings->exercise_elapsed);
          }
        exercises->set_auto_advance(false);
      }
    if (auto *container = dynamic_cast<Gtk::Container *>(widget))
      {
        for (auto *child: container->get_children())
          {
            configure_widget(child);
          }
      }
  }
#endif

  void disable_animations() const
  {
#ifdef HAVE_APP_QT
    for (auto *window: QGuiApplication::topLevelWindows())
      {
        auto *view = qobject_cast<QQuickView *>(window);
        if (view == nullptr)
          {
            continue;
          }
        for (const char *property: {"bridge", "exercisesBridge"})
          {
            auto *bridge = view->rootContext()->contextProperty(property).value<QObject *>();
            if (bridge != nullptr && bridge->metaObject()->indexOfProperty("animationsEnabled") >= 0)
              {
                bridge->setProperty("animationsEnabled", false);
              }
          }
      }
#endif
  }

  void refresh()
  {
    disable_animations();
    if (break_window)
      {
        break_window->refresh();
      }
    if (main != nullptr)
      {
        main->refresh();
      }
#ifdef HAVE_APP_GTK
    for (auto *window: Gtk::Window::list_toplevels())
      {
        if (window->get_visible() && window != backdrop.get())
          {
            configure_widget(window);
            window->set_keep_above(true);
            window->check_resize();
            if (auto surface = window->get_window())
              {
                surface->process_updates(true);
              }
          }
      }
#else
    for (auto *window: QGuiApplication::topLevelWindows())
      {
        if (window->isVisible() && (!backdrop || window != backdrop->windowHandle()))
          {
            configure_window(window);
          }
      }
#endif
  }

#ifdef HAVE_APP_QT
  template<typename Bridge>
  void configure_exercises(Bridge *exercises)
  {
    if (exercises == nullptr)
      {
        return;
      }
    if (configured_exercises.insert(exercises).second)
      {
        exercises->seek(settings->exercise_elapsed);
      }
    exercises->setAutoAdvance(false);
  }

  void configure_window(QWindow *window)
  {
    if (!window->flags().testFlag(Qt::WindowStaysOnTopHint))
      {
        window->setFlag(Qt::WindowStaysOnTopHint);
        window->show();
      }
    auto *view = qobject_cast<QQuickView *>(window);
    if (view == nullptr)
      {
        return;
      }
    for (const char *property: {"bridge", "exercisesBridge"})
      {
        auto *bridge = view->rootContext()->contextProperty(property).value<QObject *>();
        configure_exercises(qobject_cast<ExercisesBridge *>(bridge));
        configure_exercises(qobject_cast<RestBreakBridge *>(bridge));
      }
  }

  void select_preferences(QmlPrefsDialog &prefs, const std::string &page, const std::string &panel) const
  {
    if (page.empty())
      {
        return;
      }
    auto registry = context->get_internal_preferences_registry();
    auto [section, target] = std::pair{page, panel};
    if (page == "timer")
      {
        section = "timers";
        if (target == "dailylimit")
          {
            target = "daily";
          }
      }
    else if (page == "monitoring")
      {
        section = "timers";
        target = "monitoring";
      }
    else if (page == "sounds")
      {
        section = "ui";
        target = "sounds";
      }
    else if (page == "ui" && target == "mainwindow")
      {
        target = "status";
      }
    else if (registry->get_pages().contains(page))
      {
        section = "plugin";
        target = page;
      }
    const bool known = (section == "timers"
                        && (target == "microbreak" || target == "restbreak" || target == "daily" || target == "monitoring"))
                       || (section == "ui"
                           && (target == "general" || target == "sounds" || target == "status" || target == "applet"))
                       || (section == "remote-control" && target == "grpc") || section == "plugin";
    if (!known)
      {
        throw std::invalid_argument("Unknown preferences page/panel");
      }
    prefs.select_page(section, target);
  }
#endif

  void show_window(PreviewWindowType type, const std::string &page, const std::string &panel)
  {
    if (type == PreviewWindowType::Main)
      {
        find_main_window();
        if (main == nullptr)
          {
            throw std::runtime_error("Toolkit has no main window");
          }
        main->open_window();
        main->show();
        return;
      }
#ifdef HAVE_APP_GTK
    switch (type)
      {
      case PreviewWindowType::Preferences:
        {
          auto prefs = std::make_unique<PreferencesDialog>(context);
          prefs->select_page(page, panel);
          dialog = std::move(prefs);
          break;
        }
      case PreviewWindowType::Statistics:
        dialog = std::make_unique<StatisticsDialog>(context);
        break;
      case PreviewWindowType::Exercises:
        dialog = std::make_unique<ExercisesDialog>(context->get_sound_theme(), context->get_exercises());
        break;
      case PreviewWindowType::Debug:
        dialog = std::make_unique<DebugDialog>();
        break;
      case PreviewWindowType::About:
        {
          auto about = std::make_unique<Gtk::AboutDialog>();
          about->set_program_name("Workrave");
          std::vector<Glib::ustring> authors;
          for (const auto *author: workrave_authors)
            {
              if (author == nullptr)
                {
                  break;
                }
              authors.emplace_back(author);
            }
          about->set_authors(authors);
          about->set_copyright(workrave_copyright);
          about->set_comments("This program assists in the prevention and recovery of Repetitive Strain Injury (RSI).");
          auto logo = workrave::utils::AssetPath::complete_directory("workrave.png", workrave::utils::SearchPathId::Images);
          about->set_logo(Gdk::Pixbuf::create_from_file(logo));
          about->set_translator_credits(workrave_translators);
          about->set_version(WORKRAVE_VERSION);
          about->set_website("https://www.workrave.org/");
          about->set_website_label("www.workrave.org");
          dialog = std::move(about);
          break;
        }
      default:
        throw std::invalid_argument("Unknown window");
      }
    dialog->show_all();
#else
    switch (type)
      {
      case PreviewWindowType::Preferences:
        {
          auto prefs = std::make_unique<QmlPrefsDialog>(context);
          prefs->show();
          select_preferences(*prefs, page, panel);
          dialog = std::move(prefs);
          break;
        }
      case PreviewWindowType::Statistics:
        {
          auto statistics = std::make_unique<QmlStatisticsDialog>(context);
          statistics->show();
          dialog = std::move(statistics);
          break;
        }
      case PreviewWindowType::Exercises:
        {
          auto exercises = std::make_unique<QmlExercisesDialog>(context);
          disable_animations();
          exercises->show();
          dialog = std::move(exercises);
          break;
        }
      case PreviewWindowType::About:
        {
          auto about = std::make_unique<QmlAboutDialog>();
          about->show();
          dialog = std::move(about);
          break;
        }
      case PreviewWindowType::Debug:
        debug = std::make_unique<DebugDialog>(context);
        debug->show();
        break;
      default:
        throw std::invalid_argument("Unknown window");
      }
#endif
  }

  void show_menu(bool tray, const std::string &submenu, int x, int y)
  {
#if defined(PLATFORM_OS_WINDOWS) && defined(HAVE_APP_GTK)
    if (tray)
      {
        // TrackPopupMenu owns a native nested loop. Open after the RPC ack;
        // the next RPC cancels it via its owner HWND before queueing UI work.
        const auto generation = menu_generation.load();
        dispatch([weak = owner.weak_from_this(), submenu = std::make_shared<const std::string>(submenu), x, y, generation] {
          if (auto preview = weak.lock(); preview && preview->impl->menu_generation == generation)
            {
              preview->impl->native_menu->show_menu(x, y, *submenu);
            }
        });
        return;
      }
#endif
    auto filter = [tray](menus::Node::Ptr node) { return tray || node->get_id() != "workrave.open"; };
#ifdef HAVE_APP_GTK
    if (menu)
      {
        menu->get_menu()->popdown();
      }
    menu = std::make_shared<ToolkitMenu>(context->get_menu_model(), filter, submenu);
    auto popup = menu->get_menu();
    popup->insert_action_group("app", menu->get_action_group());
    find_main_window();
    popup->attach_to_widget(*main);
    popup->show_all();
    popup->popup(
      // GTK fixes the order and types of these output parameters.
      // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
      [x, y](int &px, int &py, bool &push_in) {
        px = x;
        py = y;
        push_in = true;
      },
      0,
      gtk_get_current_event_time());
#else
    if (menu)
      {
        auto *popup = menu->get_menu();
        menu.reset();
        delete popup;
      }
    find_main_window();
    if (main != nullptr)
      {
        // Menus need a native parent even when it has never been shown.
        main->winId();
      }
    menu = std::make_shared<ToolkitMenu>(context->get_menu_model(), filter, main, submenu);
    auto *popup = menu->get_menu();
    popup->setWindowFlag(Qt::WindowStaysOnTopHint);
    popup->popup(QPoint(x, y));
#endif
  }

  void set_backdrop(const std::string &color)
  {
    if (color.empty())
      {
        backdrop.reset();
        return;
      }
#ifdef HAVE_APP_GTK
    if (!backdrop)
      {
        backdrop = std::make_unique<Gtk::Window>();
        backdrop->set_title("Workrave screenshot backdrop");
        backdrop->set_decorated(false);
        backdrop->set_accept_focus(false);
        backdrop->set_skip_taskbar_hint(true);
        backdrop->set_skip_pager_hint(true);
        backdrop->fullscreen();
      }
    backdrop->override_background_color(Gdk::RGBA(color));
    backdrop->show();
    if (auto surface = backdrop->get_window())
      {
        surface->lower();
      }
#else
    if (!backdrop)
      {
        backdrop = std::make_unique<QWidget>();
        backdrop->setWindowTitle("Workrave screenshot backdrop");
        backdrop->setAttribute(Qt::WA_ShowWithoutActivating);
        backdrop->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
        backdrop->setGeometry(QGuiApplication::primaryScreen()->geometry());
      }
    backdrop->setStyleSheet(QString("background-color: %1;").arg(QString::fromStdString(color)));
    backdrop->show();
    backdrop->lower();
#endif
  }

  MainWindow *main{nullptr};
  std::shared_ptr<ToolkitMenu> menu;
  std::list<Exercise> exercises;
  std::set<const void *> configured_exercises;
#ifdef HAVE_APP_GTK
  std::unique_ptr<Gtk::Window> dialog;
  std::unique_ptr<Gtk::Window> backdrop;
#else
  std::shared_ptr<void> dialog;
  std::unique_ptr<DebugDialog> debug;
  std::unique_ptr<QWidget> backdrop;
#endif
#if defined(PLATFORM_OS_WINDOWS) && defined(HAVE_APP_GTK)
  std::unique_ptr<WindowsStatusIcon> native_menu;
  HWND menu_owner{nullptr};
  std::atomic<uint64_t> menu_generation{0};
#endif

  WindowPreview &owner;
  IToolkit::Ptr toolkit;
  std::shared_ptr<Context> context;
  Menus::Ptr menus;
  std::shared_ptr<WindowPreviewSettings> settings{std::make_shared<WindowPreviewSettings>()};
  std::string last_error;
  workrave::ui::rpc::WindowPreviewService service;
  std::function<void()> render_break;
  IBreakWindow::Ptr break_window;
  IPreludeWindow::Ptr prelude_window;
  mutable std::mutex mutex;
  mutable std::condition_variable applied;
  std::deque<std::pair<uint64_t, std::function<void()>>> pending;
  std::deque<std::pair<uint64_t, std::string>> results;
  bool closed{false};
  uint64_t sequence{0};
  uint64_t applied_sequence{0};
};

WindowPreview::WindowPreview(IToolkit::Ptr toolkit)
  : impl(std::make_unique<Impl>(*this, std::move(toolkit)))
{
}

WindowPreview::~WindowPreview()
{
  try
    {
      if (impl->context)
        {
          impl->stop();
        }
    }
  catch (...)
    {
      // Explicit close() handles normal shutdown. Cleanup must not throw
      // while unwinding a failed development session.
      std::fputs("Window preview cleanup failed\n", stderr);
    }
}

void
WindowPreview::register_service(::rpc::RpcServer &server)
{
  server.register_service(impl->service);
}

uint64_t
WindowPreview::show_break(PreviewBreakType id, int progress, int duration, bool skippable, bool postponable)
{
  const auto break_id = static_cast<workrave::BreakId>(id);
  validate(break_id, progress, duration);
  return impl->enqueue([skippable, postponable, break_id, progress, duration, this] {
    impl->stop();
    impl->render_break = [skippable, postponable, break_id, progress, duration, this] {
      BreakFlags flags = impl->settings->exercise_index < 0 ? BREAK_FLAGS_NO_EXERCISES : BREAK_FLAGS_NONE;
      if (skippable)
        {
          flags |= BREAK_FLAGS_SKIPPABLE;
        }
      if (postponable)
        {
          flags |= BREAK_FLAGS_POSTPONABLE;
        }
      for (int i = 0; i < workrave::BREAK_ID_SIZEOF; ++i)
        {
          GUIConfig::break_skippable(static_cast<workrave::BreakId>(i)).set(true);
          GUIConfig::break_ignorable(static_cast<workrave::BreakId>(i)).set(true);
        }
      impl->settings->locking_break = break_id > workrave::BREAK_ID_MICRO_BREAK
                                          && (!impl->settings->buttons.can_skip || !impl->settings->buttons.can_postpone)
                                        ? static_cast<workrave::BreakId>(break_id - 1)
                                        : workrave::BREAK_ID_NONE;
      if (impl->settings->locking_break != workrave::BREAK_ID_NONE)
        {
          GUIConfig::break_skippable(impl->settings->locking_break).set(impl->settings->buttons.can_skip);
          GUIConfig::break_ignorable(impl->settings->locking_break).set(impl->settings->buttons.can_postpone);
        }
      impl->break_window = impl->toolkit->create_break_window(0, break_id, flags);
      if (!impl->break_window)
        {
          throw std::runtime_error("No display available");
        }
      impl->break_window->init();
      impl->disable_animations();
      impl->break_window->set_break_button_state(impl->settings->buttons);
      impl->break_window->set_progress(progress, duration);
      impl->break_window->start();
    };
    impl->render_break();
  });
}

uint64_t
WindowPreview::show_prelude(PreviewBreakType id,
                            workrave::IApp::PreludeStage stage,
                            workrave::IApp::PreludeProgressText text,
                            int progress,
                            int duration)
{
  const auto break_id = static_cast<workrave::BreakId>(id);
  validate(break_id, progress, duration);
  if (stage < workrave::IApp::PreludeStage::Initial || stage > workrave::IApp::PreludeStage::Alert)
    {
      throw std::invalid_argument("Unknown prelude stage");
    }
  if (text < workrave::IApp::PreludeProgressText::BreakIn || text > workrave::IApp::PreludeProgressText::SilentIn)
    {
      throw std::invalid_argument("Unknown prelude text");
    }
  return impl->enqueue([break_id, stage, text, progress, duration, this] {
    impl->stop();
    impl->prelude_window = impl->toolkit->create_prelude_window(0, break_id);
    if (!impl->prelude_window)
      {
        throw std::runtime_error("No display available");
      }
    impl->disable_animations();
    impl->prelude_window->set_stage(stage);
    impl->prelude_window->set_progress_text(text);
    impl->prelude_window->set_progress(progress, duration);
    impl->prelude_window->start();
  });
}

uint64_t
WindowPreview::hide()
{
  return impl->enqueue([this] { impl->stop(); });
}

uint64_t
WindowPreview::quit()
{
  return impl->enqueue([this] { impl->toolkit->terminate(); });
}

uint64_t
WindowPreview::get_applied_sequence() const
{
  std::scoped_lock lock(impl->mutex);
  return impl->applied_sequence;
}

std::string
WindowPreview::wait_for_applied_sequence(uint64_t sequence, uint64_t after_sequence) const
{
  std::unique_lock lock(impl->mutex);
  if (sequence <= after_sequence || sequence > impl->sequence)
    {
      throw std::invalid_argument("Invalid preview sequence range");
    }
  if (!impl->applied.wait_for(lock, std::chrono::seconds(30), [this, sequence] {
        return impl->closed || impl->applied_sequence >= sequence;
      }))
    {
      throw std::runtime_error("UI did not apply the preview request");
    }
  if (impl->closed)
    {
      throw std::runtime_error("Preview session is closed");
    }
  if (impl->results.empty() || impl->results.front().first > after_sequence + 1)
    {
      throw std::invalid_argument("Preview results are no longer available");
    }
  std::string errors;
  for (const auto &[number, error]: impl->results)
    {
      if (number > after_sequence && number <= sequence && !error.empty())
        {
          if (!errors.empty())
            {
              errors += '\n';
            }
          errors += error;
        }
    }
  return errors;
}

void
WindowPreview::close()
{
  {
    std::scoped_lock lock(impl->mutex);
    impl->closed = true;
    impl->pending.clear();
  }
  impl->applied.notify_all();
  impl->stop();
#if defined(PLATFORM_OS_WINDOWS) && defined(HAVE_APP_GTK)
  impl->native_menu.reset();
#endif
  impl->main = nullptr;
  impl->backdrop.reset();
  impl->context.reset();
  impl->toolkit.reset();
}

void
WindowPreview::process_pending()
{
  std::deque<std::pair<uint64_t, std::function<void()>>> pending;
  {
    std::scoped_lock lock(impl->mutex);
    pending.swap(impl->pending);
  }
  for (auto &[sequence, action]: pending)
    {
      std::string error;
      try
        {
          action();
          impl->refresh();
        }
      catch (const std::exception &e)
        {
          error = std::to_string(sequence) + ": " + e.what();
        }
      std::scoped_lock lock(impl->mutex);
      impl->last_error = std::move(error);
      impl->applied_sequence = sequence;
      impl->results.emplace_back(sequence, impl->last_error);
      if (impl->results.size() > 64)
        {
          impl->results.pop_front();
        }
      impl->applied.notify_all();
    }
}

void
WindowPreview::initialize(std::shared_ptr<Context> context)
{
  impl->context = std::move(context);
  impl->exercises = impl->context->get_exercises()->get_exercises();
  impl->context->set_core(std::make_shared<PreviewCore>(impl->settings));
  impl->menus = std::make_shared<Menus>(impl->context);
#if defined(PLATFORM_OS_WINDOWS) && defined(HAVE_APP_GTK)
  impl->native_menu = std::make_unique<WindowsStatusIcon>(impl->context);
  impl->menu_owner = impl->native_menu->get_window_handle();
#endif
}

uint64_t
WindowPreview::show_window(PreviewWindowType window, std::string page, std::string panel)
{
  if (window < PreviewWindowType::Main || window > PreviewWindowType::Debug)
    {
      throw std::invalid_argument("Unknown window");
    }
  if (page.size() > 128 || panel.size() > 128)
    {
      throw std::invalid_argument("Page/panel name too long");
    }
  return impl->enqueue([window, page, panel, this] {
    impl->stop();
    impl->show_window(window, page, panel);
  });
}

uint64_t
WindowPreview::show_menu(bool tray, std::string submenu, int x, int y)
{
  if (submenu.size() > 128 || x < 0 || y < 0 || x > 32767 || y > 32767)
    {
      throw std::invalid_argument("Invalid menu position/name");
    }
  return impl->enqueue([tray, submenu, x, y, this] { impl->show_menu(tray, submenu, x, y); });
}

uint64_t
WindowPreview::set_timer(PreviewBreakType id, int elapsed, int limit, int idle, int duration, bool enabled)
{
  if (id < PreviewBreakType::MicroBreak || id > PreviewBreakType::DailyLimit)
    {
      throw std::invalid_argument("Unknown break type");
    }
  if (elapsed < 0 || limit < 0 || idle < 0 || duration < 0 || elapsed > 604800 || limit > 604800 || idle > 604800
      || duration > 604800)
    {
      throw std::invalid_argument("Timer values must be between 0 and 604800 seconds");
    }
  return impl->enqueue([id, elapsed, limit, idle, duration, enabled, this] {
    impl->settings->timers.at(
      static_cast<int>(id)) = {.elapsed = elapsed, .limit = limit, .idle = idle, .duration = duration, .enabled = enabled};
  });
}

uint64_t
WindowPreview::set_appearance(PreviewBlockMode block_mode, bool user_active, int exercise_index, int exercise_elapsed)
{
  if (block_mode < PreviewBlockMode::Off || block_mode > PreviewBlockMode::All || exercise_index < -1 || exercise_index > 255
      || exercise_elapsed < 0 || exercise_elapsed > 3600)
    {
      throw std::invalid_argument("Invalid appearance/exercise settings");
    }
  return impl->enqueue([exercise_index, block_mode, user_active, exercise_elapsed, this] {
    if (exercise_index >= 0 && std::cmp_greater_equal(exercise_index, impl->exercises.size()))
      {
        throw std::invalid_argument("Exercise index exceeds the installed exercise collection");
      }
    auto exercises = impl->exercises;
    if (exercise_index >= 0)
      {
        auto selected = std::next(exercises.begin(), exercise_index);
        exercises = {*selected};
      }
    impl->context->get_exercises()->set_exercises(exercises);
    GUIConfig::block_mode().set(static_cast<BlockMode>(block_mode));
    GUIConfig::break_exercises(workrave::BREAK_ID_REST_BREAK).set(exercise_index >= 0 ? 1 : 0);
    impl->settings->user_active = user_active;
    impl->settings->exercise_index = exercise_index;
    impl->settings->exercise_elapsed = exercise_elapsed;
    impl->configured_exercises.clear();
  });
}

uint64_t
WindowPreview::set_button_lock(bool can_skip, bool can_postpone, int value, int maximum)
{
  if (value < 0 || maximum < 0 || value > maximum || maximum > 604800 || ((!can_skip || !can_postpone) && maximum == 0))
    {
      throw std::invalid_argument("Require 0 <= value <= maximum, with maximum > 0 for locked buttons");
    }
  return impl->enqueue([can_postpone, can_skip, maximum, value, this] {
    impl->settings->buttons = {
      .can_postpone = can_postpone,
      .can_skip = can_skip,
      .overdue_break_id = (!can_skip || !can_postpone) ? workrave::BREAK_ID_MICRO_BREAK : workrave::BREAK_ID_NONE,
      .lock_max = maximum,
      .lock_value = value,
    };
    if (impl->break_window)
      {
        impl->break_window->stop();
        impl->break_window.reset();
        impl->render_break();
      }
  });
}

uint64_t
WindowPreview::set_modes(workrave::OperationMode operation_mode, workrave::UsageMode usage_mode)
{
  if (operation_mode < workrave::OperationMode::Normal || operation_mode > workrave::OperationMode::Quiet
      || usage_mode < workrave::UsageMode::Normal || usage_mode > workrave::UsageMode::Reading)
    {
      throw std::invalid_argument("Unknown operation/usage mode");
    }
  return impl->enqueue([operation_mode, usage_mode, this] {
    impl->context->get_core()->set_operation_mode(operation_mode);
    impl->context->get_core()->set_usage_mode(usage_mode);
  });
}

std::string
WindowPreview::get_last_error() const
{
  std::scoped_lock lock(impl->mutex);
  return impl->last_error;
}

uint64_t
WindowPreview::set_backdrop(std::string color)
{
  if (!color.empty()
      && (color.size() != 7 || color.front() != '#' || color.find_first_not_of("0123456789abcdefABCDEF", 1) != std::string::npos))
    {
      throw std::invalid_argument("Backdrop requires #RRGGBB, or an empty string to hide it");
    }
  return impl->enqueue([color, this] { impl->set_backdrop(color); });
}

namespace
{
  void validate_setting(const std::string &key)
  {
    if (key.empty() || key.size() > 256 || key == "general/datadir")
      {
        throw std::invalid_argument("Invalid preview setting key");
      }
  }
} // namespace

uint64_t
WindowPreview::set_bool_setting(std::string key, bool value)
{
  validate_setting(key);
  return impl->enqueue([key, value, this] { impl->context->get_configurator()->set_value(key, value); });
}

uint64_t
WindowPreview::set_int_setting(std::string key, int value)
{
  validate_setting(key);
  return impl->enqueue([key, value, this] { impl->context->get_configurator()->set_value(key, static_cast<int32_t>(value)); });
}

uint64_t
WindowPreview::set_string_setting(std::string key, std::string value)
{
  validate_setting(key);
  if (value.size() > 4096)
    {
      throw std::invalid_argument("Setting value too long");
    }
  return impl->enqueue([key, value, this] { impl->context->get_configurator()->set_value(key, value); });
}

// This entry point owns the entire development session. The normal Application,
// Toolkit and Core neither create nor refer to the screenshot controller.
int
main(int argc, char **argv)
{
  using namespace workrave::utils;
  try
    {
      const char *directory = std::getenv("WORKRAVE_PREVIEW_DIRECTORY");
      auto profile = (directory != nullptr) && ((*directory) != 0)
                       ? std::filesystem::path(directory)
                       : std::filesystem::temp_directory_path() / "workrave-window-preview";
      std::filesystem::create_directories(profile);
      Paths::set_portable_directory(profile.string());
      auto config = workrave::config::ConfiguratorFactory::create(workrave::config::ConfigFileFormat::Ini);
      auto filename = Paths::get_config_directory() / "workrave.ini";
      if (!std::filesystem::exists(filename))
        {
          std::ofstream initial_config(filename);
        }
      config->load(filename.string());
      CoreConfig::init(config);
      GUIConfig::init(config);
      GUIConfig::timerbox_enabled("main_window").set(false);
      GUIConfig::trayicon_enabled().set(false);
      config->set_value("sound/enabled", false);

      auto toolkit = ToolkitFactory().create(argc, argv);
      toolkit->preinit(config);
      System::init();
      auto context = std::make_shared<Context>();
      context->set_configurator(config);
      context->set_toolkit(toolkit);
      auto sound = std::make_shared<SoundTheme>(config);
      sound->init();
      context->set_sound_theme(sound);
      auto exercises = std::make_shared<ExerciseCollection>();
      exercises->load();
      exercises->set_language(GUIConfig::locale()());
      context->set_exercises(exercises);
      context->set_preferences_registry(std::make_shared<PreferencesRegistry>());
      context->set_menu_model(std::make_shared<MenuModel>());
      auto preview = std::make_shared<WindowPreview>(toolkit);
      preview->initialize(context);
#ifdef PLATFORM_OS_WINDOWS
      WindowsForceFocus::init(config);
      WindowsCompat::init(config);
#endif
      toolkit->init(context);
      toolkit->hold();
#ifdef HAVE_APP_GTK
      auto *application = g_application_get_default();
      g_application_set_application_id(application, "org.workrave.WindowPreview");
      g_application_set_flags(application, G_APPLICATION_NON_UNIQUE);
      Gtk::Settings::get_default()->property_gtk_enable_animations() = false;
      for (auto *window: Gtk::Window::list_toplevels())
        {
          window->hide();
        }
#else
      QApplication::setApplicationName("Workrave Window Preview");
      QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);
      QApplication::setEffectEnabled(Qt::UI_FadeMenu, false);
      for (auto *window: QApplication::topLevelWidgets())
        {
          window->hide();
        }
#endif
      PluginRegistry::instance().build(context);
      const char *address = std::getenv("WORKRAVE_RPC_ADDRESS");
      ::rpc::RpcServer server({.listen_address = (address != nullptr) && ((*address) != 0) ? address : "127.0.0.1:50055"});
      preview->register_service(server);
      server.start();
      std::cout << "Window preview RPC listening on port " << server.bound_port() << std::endl;
      toolkit->run();
      preview->close();
      server.shutdown();
      PluginRegistry::instance().deinit();
      toolkit->deinit();
      return 0;
    }
  catch (const std::exception &error)
    {
      std::cerr << "Window preview: " << error.what() << '\n';
      return 1;
    }
}
