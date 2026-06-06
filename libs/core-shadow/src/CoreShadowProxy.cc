#include "CoreShadowProxy.hh"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string_view>

#include <spdlog/spdlog.h>

#include "utils/Enum.hh"

namespace workrave::core_shadow
{
  namespace
  {
    constexpr int64_t timer_warning_threshold = 2;

    auto
    command_line(const std::vector<std::string> &fields) -> std::string
    {
      std::ostringstream out;
      bool first = true;
      for (const auto &field: fields)
        {
          if (!first)
            {
              out << '\t';
            }
          first = false;
          out << escape_field(field);
        }
      return out.str();
    }

    auto
    event_key(const EventObservation &event) -> std::string
    {
      return event.source + "\t" + std::to_string(event.break_id) + "\t" + event.name + "\t" + event.detail;
    }

    auto
    event_history_key(const EventObservation &event) -> std::string
    {
      return event.source + "\t" + std::to_string(event.break_id) + "\t" + event.name;
    }

    template<typename Enum>
    auto enum_int(Enum value) -> std::string
    {
      return std::to_string(static_cast<std::underlying_type_t<Enum>>(value));
    }

    auto bool_text(bool value) -> const char *
    {
      return value ? "true" : "false";
    }

    auto break_name(int break_id) -> std::string_view
    {
      switch (break_id)
        {
        case static_cast<int>(BREAK_ID_MICRO_BREAK):
          return "micro-break";
        case static_cast<int>(BREAK_ID_REST_BREAK):
          return "rest-break";
        case static_cast<int>(BREAK_ID_DAILY_LIMIT):
          return "daily-limit";
        case static_cast<int>(BREAK_ID_NONE):
          return "global";
        default:
          return "unknown";
        }
    }

    auto find_snapshot(const std::vector<TimerSnapshot> &snapshots, int break_id) -> const TimerSnapshot *
    {
      const auto found = std::find_if(snapshots.begin(), snapshots.end(), [break_id](const auto &snapshot) {
        return snapshot.break_id == break_id;
      });
      return found != snapshots.end() ? &*found : nullptr;
    }

    auto html_escape(std::string_view text) -> std::string
    {
      std::string result;
      result.reserve(text.size());
      for (const auto character: text)
        {
          switch (character)
            {
            case '&':
              result += "&amp;";
              break;
            case '<':
              result += "&lt;";
              break;
            case '>':
              result += "&gt;";
              break;
            case '"':
              result += "&quot;";
              break;
            default:
              result += character;
              break;
            }
        }
      return result;
    }

    void append_value_cell(std::ostringstream &out, const std::string &value, bool differs)
    {
      if (differs)
        {
          out << "<td align=\"right\" bgcolor=\"#ffe6e6\"><font color=\"#c62828\"><b>" << value
              << "</b></font></td>";
        }
      else
        {
          out << "<td align=\"right\">" << value << "</td>";
        }
    }

    void append_number_row(std::ostringstream &out, std::string_view metric, int64_t core, int64_t corenext)
    {
      const auto differs = core != corenext;
      const auto delta = corenext - core;
      out << "<tr><td>" << metric << "</td>";
      append_value_cell(out, std::to_string(core), differs);
      append_value_cell(out, std::to_string(corenext), differs);
      append_value_cell(out, (delta > 0 ? "+" : "") + std::to_string(delta), differs);
      out << "</tr>";
    }

    void append_bool_row(std::ostringstream &out, std::string_view metric, bool core, bool corenext)
    {
      const auto differs = core != corenext;
      out << "<tr><td>" << metric << "</td>";
      append_value_cell(out, bool_text(core), differs);
      append_value_cell(out, bool_text(corenext), differs);
      out << "<td></td></tr>";
    }

    void append_missing_row(std::ostringstream &out, std::string_view backend)
    {
      out << "<tr><td colspan=\"4\" bgcolor=\"#ffe6e6\"><font color=\"#c62828\"><b>" << backend
          << " snapshot unavailable</b></font></td></tr>";
    }

    void append_break_comparison(std::ostringstream &out,
                                 int break_id,
                                 const TimerSnapshot *core,
                                 const TimerSnapshot *corenext)
    {
      out << "<h4>" << break_name(break_id) << "</h4>";
      out << "<table cellspacing=\"0\" cellpadding=\"4\" border=\"1\">"
          << "<tr bgcolor=\"#e8e8e8\"><th align=\"left\">metric</th><th>core</th><th>corenext</th><th>delta</th></tr>";

      if (core == nullptr)
        {
          append_missing_row(out, "core");
        }
      if (corenext == nullptr)
        {
          append_missing_row(out, "corenext");
        }

      if (core != nullptr && corenext != nullptr)
        {
          append_number_row(out, "elapsed", core->elapsed, corenext->elapsed);
          append_number_row(out, "idle", core->idle, corenext->idle);
          append_number_row(out, "limit", core->limit, corenext->limit);
          append_number_row(out, "auto-reset", core->auto_reset, corenext->auto_reset);
          append_bool_row(out, "enabled", core->enabled, corenext->enabled);
          append_bool_row(out, "running", core->running, corenext->running);
          append_bool_row(out, "taking", core->taking, corenext->taking);
          append_bool_row(out, "active", core->active, corenext->active);
        }

      out << "</table>";
    }
  } // namespace

  void
  CoreShadowComparator::record_live_event(EventObservation event)
  {
    record_event_history(event);
    live_events.push_back(std::move(event));
  }

  void
  CoreShadowComparator::record_event_history(const EventObservation &event)
  {
    auto &history = event_history[event_history_key(event)];
    history.source = event.source;
    history.break_id = event.break_id;
    history.name = event.name;
    if (event.backend == Backend::Core)
      {
        history.core_detail = event.detail;
        history.core_tick = event.tick;
      }
    else
      {
        history.corenext_detail = event.detail;
        history.corenext_tick = event.tick;
      }
  }

  void
  CoreShadowComparator::record_shadow_events(const ObservationBatch &shadow_batch)
  {
    for (const auto &event: shadow_batch.events)
      {
        record_event_history(event);
      }
  }

  std::string
  CoreShadowComparator::get_event_debug_state_html() const
  {
    std::ostringstream out;
    out << "<h4>Events</h4>";
    if (event_history.empty())
      {
        out << "<p>No events observed.</p>";
        return out.str();
      }

    out << "<table cellspacing=\"0\" cellpadding=\"4\" border=\"1\">"
        << "<tr bgcolor=\"#e8e8e8\"><th align=\"left\">source</th><th align=\"left\">break</th>"
        << "<th align=\"left\">event</th><th align=\"left\">core detail</th><th align=\"left\">corenext detail</th>"
        << "<th>last core tick</th><th>last corenext tick</th><th>delta</th></tr>";

    for (const auto &[key, event]: event_history)
      {
        (void)key;
        const auto both_present = event.core_tick.has_value() && event.corenext_tick.has_value();
        const auto tick_differs = !both_present || event.core_tick != event.corenext_tick;
        const auto detail_differs = !both_present || event.core_detail != event.corenext_detail;
        const auto core_tick = event.core_tick ? std::to_string(*event.core_tick) : "-";
        const auto corenext_tick = event.corenext_tick ? std::to_string(*event.corenext_tick) : "-";
        std::string delta{"-"};
        if (both_present)
          {
            const auto value = *event.corenext_tick - *event.core_tick;
            delta = (value > 0 ? "+" : "") + std::to_string(value);
          }

        out << "<tr><td>" << html_escape(event.source) << "</td><td>" << break_name(event.break_id) << "</td><td>"
            << html_escape(event.name) << "</td>";
        append_value_cell(out, html_escape(event.core_detail), detail_differs);
        append_value_cell(out, html_escape(event.corenext_detail), detail_differs);
        append_value_cell(out, core_tick, tick_differs);
        append_value_cell(out, corenext_tick, tick_differs);
        append_value_cell(out, delta, tick_differs);
        out << "</tr>";
      }

    out << "</table>";
    return out.str();
  }

  void
  CoreShadowComparator::compare(int64_t tick, const std::vector<TimerSnapshot> &live_snapshots, const ObservationBatch &shadow_batch)
  {
    std::vector<EventObservation> current_live;
    auto live_it = std::remove_if(live_events.begin(), live_events.end(), [&](const auto &event) {
      if (event.tick == tick)
        {
          current_live.push_back(event);
          return true;
        }
      return event.tick < tick - 2;
    });
    live_events.erase(live_it, live_events.end());

    record_shadow_events(shadow_batch);

    for (const auto &live_event: current_live)
      {
        auto key = event_key(live_event);
        auto found = std::find_if(shadow_batch.events.begin(), shadow_batch.events.end(), [&](const auto &shadow_event) {
          return event_key(shadow_event) == key;
        });
        if (found == shadow_batch.events.end())
          {
            spdlog::warn("Core shadow: corenext emitted {} {} for break {} at tick {}, missing from core",
                         live_event.source,
                         live_event.name,
                         live_event.break_id,
                         tick);
          }
      }

    for (const auto &shadow_event: shadow_batch.events)
      {
        auto key = event_key(shadow_event);
        auto found = std::find_if(current_live.begin(), current_live.end(), [&](const auto &live_event) {
          return event_key(live_event) == key;
        });
        if (found == current_live.end())
          {
            spdlog::warn("Core shadow: core emitted {} {} for break {} at tick {}, missing from corenext",
                         shadow_event.source,
                         shadow_event.name,
                         shadow_event.break_id,
                         tick);
          }
      }

    for (const auto &core_snapshot: shadow_batch.snapshots)
      {
        auto live_snapshot = std::find_if(live_snapshots.begin(), live_snapshots.end(), [&](const auto &snapshot) {
          return snapshot.break_id == core_snapshot.break_id;
        });
        if (live_snapshot == live_snapshots.end())
          {
            continue;
          }

        if (live_snapshot->user_active != core_snapshot.user_active && last_activity_warning_tick != tick)
          {
            spdlog::warn("Core shadow: activity differs at tick {}: core user_active={}, corenext user_active={}",
                         tick,
                         core_snapshot.user_active,
                         live_snapshot->user_active);
            last_activity_warning_tick = tick;
          }

        const auto elapsed_delta = live_snapshot->elapsed - core_snapshot.elapsed;
        const auto idle_delta = live_snapshot->idle - core_snapshot.idle;
        max_timer_delta = std::max({max_timer_delta, std::llabs(elapsed_delta), std::llabs(idle_delta)});

        if (std::llabs(elapsed_delta) >= timer_warning_threshold || std::llabs(idle_delta) >= timer_warning_threshold)
          {
            spdlog::warn("Core shadow: break {} corenext timer skew from core at tick {}: elapsed={}s idle={}s "
                         "(core elapsed={} idle={}, corenext elapsed={} idle={})",
                         core_snapshot.break_id,
                         tick,
                         elapsed_delta,
                         idle_delta,
                         core_snapshot.elapsed,
                         core_snapshot.idle,
                         live_snapshot->elapsed,
                         live_snapshot->idle);
          }

        if (live_snapshot->enabled != core_snapshot.enabled || live_snapshot->running != core_snapshot.running
            || live_snapshot->taking != core_snapshot.taking || live_snapshot->active != core_snapshot.active)
          {
            spdlog::warn("Core shadow: break {} state differs at tick {}: "
                         "core enabled={} running={} taking={} active={}, "
                         "corenext enabled={} running={} taking={} active={}",
                         core_snapshot.break_id,
                         tick,
                         core_snapshot.enabled,
                         core_snapshot.running,
                         core_snapshot.taking,
                         core_snapshot.active,
                         live_snapshot->enabled,
                         live_snapshot->running,
                         live_snapshot->taking,
                         live_snapshot->active);
          }
      }
    spdlog::debug("Core shadow: max timer delta observed={}s", max_timer_delta);
  }

  RecordingApp::RecordingApp(workrave::IApp *delegate, CoreShadowComparator &comparator, int64_t &tick)
    : delegate(delegate)
    , comparator(comparator)
    , tick(tick)
  {
  }

  void RecordingApp::record(const std::string &name, int break_id, const std::string &detail)
  {
    comparator.record_live_event(EventObservation{.backend = Backend::CoreNext,
                                                  .tick = tick,
                                                  .source = "app-callback",
                                                  .break_id = break_id,
                                                  .name = name,
                                                  .detail = detail});
  }

  void RecordingApp::create_prelude_window(BreakId break_id)
  {
    record("create_prelude_window", break_id);
    delegate->create_prelude_window(break_id);
  }

  void RecordingApp::create_break_window(BreakId break_id, workrave::utils::Flags<BreakHint> break_hint)
  {
    record("create_break_window", break_id, std::to_string(break_hint.get()));
    delegate->create_break_window(break_id, break_hint);
  }

  void RecordingApp::hide_break_window()
  {
    record("hide_break_window");
    delegate->hide_break_window();
  }

  void RecordingApp::show_break_window()
  {
    record("show_break_window");
    delegate->show_break_window();
  }

  void RecordingApp::refresh_break_window()
  {
    record("refresh_break_window");
    delegate->refresh_break_window();
  }

  void RecordingApp::set_break_progress(int value, int max_value)
  {
    record("set_break_progress", BREAK_ID_NONE, std::to_string(value) + "/" + std::to_string(max_value));
    delegate->set_break_progress(value, max_value);
  }

  void RecordingApp::set_prelude_stage(PreludeStage stage)
  {
    record("set_prelude_stage", BREAK_ID_NONE, std::string{workrave::utils::enum_to_string(stage)});
    delegate->set_prelude_stage(stage);
  }

  void RecordingApp::set_prelude_progress_text(PreludeProgressText text)
  {
    record("set_prelude_progress_text", BREAK_ID_NONE, std::string{workrave::utils::enum_to_string(text)});
    delegate->set_prelude_progress_text(text);
  }

  BreakShadowProxy::BreakShadowProxy(workrave::BreakId id, workrave::IBreak::Ptr live_break, CoreShadowProxy &core)
    : id(id)
    , live_break(std::move(live_break))
    , core(core)
  {
    live_break_connection = this->live_break->signal_break_event().connect([this](auto event) {
      this->core.record_live_break_event(this->id, event);
      break_event_signal(event);
    });
  }

  boost::signals2::signal<void(workrave::BreakEvent)> &BreakShadowProxy::signal_break_event()
  {
    return break_event_signal;
  }

  std::string BreakShadowProxy::get_name() const { return live_break->get_name(); }
  bool BreakShadowProxy::is_enabled() const { return live_break->is_enabled(); }
  bool BreakShadowProxy::is_running() const { return live_break->is_running(); }
  bool BreakShadowProxy::is_taking() const { return live_break->is_taking(); }
  bool BreakShadowProxy::is_max_preludes_reached() const { return live_break->is_max_preludes_reached(); }
  bool BreakShadowProxy::is_active() const { return live_break->is_active(); }
  int64_t BreakShadowProxy::get_elapsed_time() const { return live_break->get_elapsed_time(); }
  int64_t BreakShadowProxy::get_elapsed_idle_time() const { return live_break->get_elapsed_idle_time(); }
  int64_t BreakShadowProxy::get_auto_reset() const { return live_break->get_auto_reset(); }
  bool BreakShadowProxy::is_auto_reset_enabled() const { return live_break->is_auto_reset_enabled(); }
  int64_t BreakShadowProxy::get_limit() const { return live_break->get_limit(); }
  bool BreakShadowProxy::is_limit_enabled() const { return live_break->is_limit_enabled(); }
  int64_t BreakShadowProxy::get_total_overdue_time() const { return live_break->get_total_overdue_time(); }

  void BreakShadowProxy::postpone_break()
  {
    live_break->postpone_break();
    core.mirror_break_command("postpone", id);
  }

  void BreakShadowProxy::skip_break()
  {
    live_break->skip_break();
    core.mirror_break_command("skip", id);
  }

  auto BreakShadowProxy::snapshot(int64_t tick, bool user_active) const -> TimerSnapshot
  {
    return TimerSnapshot{.backend = Backend::CoreNext,
                         .tick = tick,
                         .break_id = id,
                         .elapsed = live_break->get_elapsed_time(),
                         .idle = live_break->get_elapsed_idle_time(),
                         .limit = live_break->get_limit(),
                         .auto_reset = live_break->get_auto_reset(),
                         .enabled = live_break->is_enabled(),
                         .running = live_break->is_running(),
                         .taking = live_break->is_taking(),
                         .active = live_break->is_active(),
                         .user_active = user_active};
  }

  CoreShadowProxy::CoreShadowProxy(workrave::ICore::Ptr live_core, workrave::config::IConfigurator::Ptr configurator)
    : live_core(std::move(live_core))
    , configurator(std::move(configurator))
  {
  }

  CoreShadowProxy::~CoreShadowProxy()
  {
    shadow_client.stop();
  }

  boost::signals2::signal<void(workrave::OperationMode)> &CoreShadowProxy::signal_operation_mode_changed()
  {
    return live_core->signal_operation_mode_changed();
  }

  boost::signals2::signal<void(workrave::UsageMode)> &CoreShadowProxy::signal_usage_mode_changed()
  {
    return live_core->signal_usage_mode_changed();
  }

  void CoreShadowProxy::init(workrave::IApp *app, const char *display)
  {
    shadow_available = shadow_client.start();
    recording_app = std::make_unique<RecordingApp>(app, comparator, tick);
    live_core->init(recording_app.get(), display);

    for (int i = 0; i < BREAK_ID_SIZEOF; i++)
      {
        auto id = BreakId(i);
        breaks[i] = std::make_shared<BreakShadowProxy>(id, live_core->get_break(id), *this);
      }
  }

  void CoreShadowProxy::heartbeat()
  {
    tick++;
    live_core->heartbeat();
    last_live_snapshots = live_snapshots();

    if (shadow_available)
      {
        ObservationBatch batch;
        const auto command = command_line({"heartbeat", std::to_string(tick)});
        if (shadow_client.command(command, batch))
          {
            last_shadow_snapshots = batch.snapshots;
            last_shadow_tick = tick;
            comparator.compare(tick, last_live_snapshots, batch);
          }
        else
          {
            shadow_available = false;
            spdlog::warn("Core shadow disabled after helper communication failure");
          }
      }
  }

  void CoreShadowProxy::force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint)
  {
    live_core->force_break(id, break_hint);
    shadow_command(command_line({"force_break", std::to_string(id), std::to_string(break_hint.get())}));
  }

  IBreak::Ptr CoreShadowProxy::get_break(BreakId id) const
  {
    return breaks[id];
  }

  IStatistics::Ptr CoreShadowProxy::get_statistics() const { return live_core->get_statistics(); }
  bool CoreShadowProxy::is_user_active() const { return live_core->is_user_active(); }
  bool CoreShadowProxy::is_taking() const { return live_core->is_taking(); }
  OperationMode CoreShadowProxy::get_active_operation_mode() { return live_core->get_active_operation_mode(); }
  OperationMode CoreShadowProxy::get_regular_operation_mode() { return live_core->get_regular_operation_mode(); }

  void CoreShadowProxy::set_operation_mode(OperationMode mode)
  {
    live_core->set_operation_mode(mode);
    shadow_command(command_line({"set_operation_mode", enum_int(mode)}));
  }

  void CoreShadowProxy::set_operation_mode_for(OperationMode mode, std::chrono::minutes duration)
  {
    live_core->set_operation_mode_for(mode, duration);
    shadow_command(command_line({"set_operation_mode_for", enum_int(mode), std::to_string(duration.count())}));
  }

  void CoreShadowProxy::set_operation_mode_override(OperationMode mode, const std::string &id)
  {
    live_core->set_operation_mode_override(mode, id);
    shadow_command(command_line({"set_operation_mode_override", enum_int(mode), id}));
  }

  void CoreShadowProxy::remove_operation_mode_override(const std::string &id)
  {
    live_core->remove_operation_mode_override(id);
    shadow_command(command_line({"remove_operation_mode_override", id}));
  }

  bool CoreShadowProxy::is_operation_mode_an_override() { return live_core->is_operation_mode_an_override(); }
  UsageMode CoreShadowProxy::get_usage_mode() { return live_core->get_usage_mode(); }

  void CoreShadowProxy::set_usage_mode(UsageMode mode)
  {
    live_core->set_usage_mode(mode);
    shadow_command(command_line({"set_usage_mode", enum_int(mode)}));
  }

  void CoreShadowProxy::set_powersave(bool down)
  {
    live_core->set_powersave(down);
    shadow_command(command_line({"set_powersave", down ? "1" : "0"}));
  }

  void CoreShadowProxy::set_insist_policy(InsistPolicy p)
  {
    live_core->set_insist_policy(p);
    shadow_command(command_line({"set_insist_policy", enum_int(p)}));
  }

  void CoreShadowProxy::force_idle()
  {
    live_core->force_idle();
    shadow_command(command_line({"force_idle"}));
  }

  ICoreHooks::Ptr CoreShadowProxy::get_hooks() const { return live_core->get_hooks(); }
  std::shared_ptr<workrave::dbus::IDBus> CoreShadowProxy::get_dbus() const { return live_core->get_dbus(); }

  std::string CoreShadowProxy::get_shadow_debug_state_html() const
  {
    std::ostringstream out;
    out << "<h3>Core shadow state</h3>";
    out << "<p>tick=" << tick << " &nbsp; helper=" << (shadow_available ? "running" : "not running");
    if (last_shadow_tick > 0)
      {
        out << " &nbsp; last-core-tick=" << last_shadow_tick;
      }
    out << "</p>";

    if (last_live_snapshots.empty() && last_shadow_snapshots.empty())
      {
        out << "<p>Waiting for first heartbeat.</p>";
        return out.str();
      }

    const auto *core_activity = last_shadow_snapshots.empty() ? nullptr : &last_shadow_snapshots.front();
    const auto *corenext_activity = last_live_snapshots.empty() ? nullptr : &last_live_snapshots.front();
    if (core_activity != nullptr && corenext_activity != nullptr)
      {
        out << "<h4>Activity</h4><table cellspacing=\"0\" cellpadding=\"4\" border=\"1\">"
            << "<tr bgcolor=\"#e8e8e8\"><th align=\"left\">metric</th><th>core</th><th>corenext</th><th>delta</th></tr>";
        append_bool_row(out, "user-active", core_activity->user_active, corenext_activity->user_active);
        out << "</table>";
      }

    out << "<table cellspacing=\"8\" cellpadding=\"0\"><tr>";
    for (int break_id = 0; break_id < BREAK_ID_SIZEOF; break_id++)
      {
        out << "<td valign=\"top\">";
        append_break_comparison(out,
                                break_id,
                                find_snapshot(last_shadow_snapshots, break_id),
                                find_snapshot(last_live_snapshots, break_id));
        out << "</td>";
      }
    out << "</tr></table>";

    out << comparator.get_event_debug_state_html();

    return out.str();
  }

  void CoreShadowProxy::mirror_break_command(const std::string &command, workrave::BreakId id)
  {
    shadow_command(command_line({command, std::to_string(id)}));
  }

  void CoreShadowProxy::record_live_break_event(workrave::BreakId id, workrave::BreakEvent event)
  {
    comparator.record_live_event(EventObservation{.backend = Backend::CoreNext,
                                                  .tick = tick,
                                                  .source = "break-signal",
                                                  .break_id = id,
                                                  .name = std::string{workrave::utils::enum_to_string(event)}});
  }

  void CoreShadowProxy::shadow_command(const std::string &command)
  {
    if (!shadow_available)
      {
        return;
      }
    ObservationBatch batch;
    if (!shadow_client.command(command, batch))
      {
        shadow_available = false;
        spdlog::warn("Core shadow disabled after helper communication failure");
      }
    else
      {
        comparator.record_shadow_events(batch);
      }
  }

  auto CoreShadowProxy::live_snapshots() const -> std::vector<TimerSnapshot>
  {
    std::vector<TimerSnapshot> snapshots;
    snapshots.reserve(BREAK_ID_SIZEOF);
    const auto user_active = live_core->is_user_active();
    for (const auto &b: breaks)
      {
        auto shadow_break = std::dynamic_pointer_cast<BreakShadowProxy>(b);
        if (shadow_break)
          {
            snapshots.push_back(shadow_break->snapshot(tick, user_active));
          }
      }
    return snapshots;
  }
} // namespace workrave::core_shadow
