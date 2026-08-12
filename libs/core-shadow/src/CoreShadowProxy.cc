#include "CoreShadowProxy.hh"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <spdlog/spdlog.h>

#include "utils/Enum.hh"
#if defined(HAVE_GRPC)
#  include <google/protobuf/descriptor.h>
#  include <google/protobuf/message.h>

#  include "rpc/Duration.hh"
#endif

namespace workrave::core_shadow
{
  namespace
  {
    constexpr int64_t timer_warning_threshold = 2;
    constexpr int64_t event_match_tolerance = 3;
    constexpr std::size_t event_comparison_history_limit = 100;

    auto command_line(const std::vector<std::string> &fields) -> std::string
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

#if defined(HAVE_GRPC)
    auto rpc_field(const google::protobuf::Message &message, std::string_view name) -> const google::protobuf::FieldDescriptor &
    {
      const auto *field = message.GetDescriptor()->FindFieldByName(std::string{name});
      if (field == nullptr)
        {
          throw std::runtime_error("gRPC request has no field named " + std::string{name});
        }
      return *field;
    }

    auto rpc_enum(const google::protobuf::Message &message, std::string_view name) -> int
    {
      const auto &field = rpc_field(message, name);
      return message.GetReflection()->GetEnumValue(message, &field);
    }

    auto rpc_repeated_enum_flags(const google::protobuf::Message &message, std::string_view name) -> int
    {
      const auto &field = rpc_field(message, name);
      const auto *reflection = message.GetReflection();
      int flags = 0;
      for (int index = 0; index < reflection->FieldSize(message, &field); ++index)
        {
          flags |= reflection->GetRepeatedEnumValue(message, &field, index);
        }
      return flags;
    }

    auto rpc_string(const google::protobuf::Message &message, std::string_view name) -> std::string
    {
      const auto &field = rpc_field(message, name);
      return message.GetReflection()->GetString(message, &field);
    }

    auto rpc_bool(const google::protobuf::Message &message, std::string_view name) -> bool
    {
      const auto &field = rpc_field(message, name);
      return message.GetReflection()->GetBool(message, &field);
    }

    auto rpc_int32(const google::protobuf::Message &message, std::string_view name) -> int32_t
    {
      const auto &field = rpc_field(message, name);
      return message.GetReflection()->GetInt32(message, &field);
    }

    auto rpc_int64(const google::protobuf::Message &message, std::string_view name) -> int64_t
    {
      const auto &field = rpc_field(message, name);
      return message.GetReflection()->GetInt64(message, &field);
    }

    auto rpc_double(const google::protobuf::Message &message, std::string_view name) -> double
    {
      const auto &field = rpc_field(message, name);
      return message.GetReflection()->GetDouble(message, &field);
    }

    auto double_text(double value) -> std::string
    {
      std::ostringstream out;
      out << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
      return out.str();
    }

    auto rpc_shadow_command(const rpc::RequestInfo &request) -> std::optional<std::string>
    {
      const auto &message = request.request;
      if (request.service == "workrave.CoreService")
        {
          if (request.method == "ForceBreak")
            {
              return command_line({"force_break",
                                   std::to_string(rpc_enum(message, "id")),
                                   std::to_string(rpc_repeated_enum_flags(message, "break_hint"))});
            }
          if (request.method == "SetOperationMode")
            {
              return command_line({"set_operation_mode", std::to_string(rpc_enum(message, "mode"))});
            }
          if (request.method == "SetOperationModeFor")
            {
              const auto duration = std::chrono::duration_cast<std::chrono::minutes>(
                rpc::parse_duration(rpc_string(message, "duration")));
              return command_line(
                {"set_operation_mode_for", std::to_string(rpc_enum(message, "mode")), std::to_string(duration.count())});
            }
          if (request.method == "SetUsageMode")
            {
              return command_line({"set_usage_mode", std::to_string(rpc_enum(message, "mode"))});
            }
          if (request.method == "ReportActivity")
            {
              return command_line({"report_activity", rpc_string(message, "who"), rpc_bool(message, "act") ? "1" : "0"});
            }
        }
      else if (request.service == "workrave.BreakService")
        {
          if (request.method == "PostponeBreak")
            {
              return command_line({"postpone", std::to_string(rpc_enum(message, "id"))});
            }
          if (request.method == "SkipBreak")
            {
              return command_line({"skip", std::to_string(rpc_enum(message, "id"))});
            }
        }
      else if (request.service == "workrave.ConfigService")
        {
          if (request.method == "RemoveKey")
            {
              return command_line({"config_remove", rpc_string(message, "key")});
            }
          if (request.method == "RenameKey")
            {
              return command_line({"config_rename", rpc_string(message, "key"), rpc_string(message, "new_key")});
            }

          const auto config_set = [&](std::string type, std::string value) {
            return command_line({"config_set",
                                 std::move(type),
                                 rpc_string(message, "key"),
                                 std::move(value),
                                 std::to_string(rpc_enum(message, "flags"))});
          };
          if (request.method == "SetString")
            {
              return config_set("string", rpc_string(message, "v"));
            }
          if (request.method == "SetInt")
            {
              return config_set("int32", std::to_string(rpc_int32(message, "v")));
            }
          if (request.method == "SetInt64")
            {
              return config_set("int64", std::to_string(rpc_int64(message, "v")));
            }
          if (request.method == "SetBool")
            {
              return config_set("bool", rpc_bool(message, "v") ? "1" : "0");
            }
          if (request.method == "SetDouble")
            {
              return config_set("double", double_text(rpc_double(message, "v")));
            }
        }
      return std::nullopt;
    }
#endif

    auto event_key(const EventObservation &event) -> std::string
    {
      return event.source + "\t" + std::to_string(event.break_id) + "\t" + event.name;
    }

    auto event_history_key(const EventObservation &event) -> std::string
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
          out << "<td align=\"right\" bgcolor=\"#ffe6e6\"><font color=\"#c62828\"><b>" << value << "</b></font></td>";
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

    void append_break_comparison(std::ostringstream &out, int break_id, const TimerSnapshot *core, const TimerSnapshot *corenext)
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

  void CoreShadowComparator::record_live_event(EventObservation event)
  {
    std::scoped_lock lock(mutex);
    record_event_history(event);
    pending_events.push_back(std::move(event));
  }

  void CoreShadowComparator::record_event_history(const EventObservation &event)
  {
    auto &history = event_history[event_history_key(event)];
    history.source = event.source;
    history.break_id = event.break_id;
    history.name = event.name;
    if (event.backend == Backend::Core)
      {
        history.core_detail = event.detail;
        history.core_tick = event.tick;
        history.core_count++;
      }
    else
      {
        history.corenext_detail = event.detail;
        history.corenext_tick = event.tick;
        history.corenext_count++;
      }
  }

  void CoreShadowComparator::record_shadow_events(const ObservationBatch &shadow_batch)
  {
    std::scoped_lock lock(mutex);
    for (const auto &event: shadow_batch.events)
      {
        record_event_history(event);
        pending_events.push_back(event);
      }
  }

  void CoreShadowComparator::record_event_comparison(EventComparison comparison)
  {
    recent_event_comparisons.push_front(std::move(comparison));
    if (recent_event_comparisons.size() > event_comparison_history_limit)
      {
        recent_event_comparisons.pop_back();
      }
  }

  void CoreShadowComparator::match_and_expire_events(int64_t tick)
  {
    // Match by semantic identity, not by heartbeat or detail. Core and corenext
    // legitimately reach the same transition a few seconds apart, and a detail
    // mismatch is useful evidence rather than proof that the counterpart is absent.
    for (std::size_t next_index = 0; next_index < pending_events.size();)
      {
        if (pending_events[next_index].backend != Backend::CoreNext)
          {
            ++next_index;
            continue;
          }

        const auto key = event_key(pending_events[next_index]);
        auto best_index = pending_events.size();
        auto best_distance = event_match_tolerance + 1;
        for (std::size_t core_index = 0; core_index < pending_events.size(); ++core_index)
          {
            const auto &core = pending_events[core_index];
            if (core.backend != Backend::Core || event_key(core) != key)
              {
                continue;
              }
            const auto distance = std::abs(core.tick - pending_events[next_index].tick);
            if (distance <= event_match_tolerance && distance < best_distance)
              {
                best_index = core_index;
                best_distance = distance;
              }
          }

        if (best_index == pending_events.size())
          {
            ++next_index;
            continue;
          }

        const auto core_event = pending_events[best_index];
        const auto corenext_event = pending_events[next_index];
        auto &history = event_history[event_history_key(corenext_event)];
        history.matched_count++;
        const auto detail_differs = core_event.detail != corenext_event.detail;
        if (detail_differs)
          {
            history.detail_mismatch_count++;
          }
        record_event_comparison(EventComparison{
          .source = corenext_event.source,
          .break_id = corenext_event.break_id,
          .name = corenext_event.name,
          .core_detail = core_event.detail,
          .corenext_detail = corenext_event.detail,
          .core_tick = core_event.tick,
          .corenext_tick = corenext_event.tick,
          .result = detail_differs ? "matched; detail differs" : (best_distance == 0 ? "matched" : "matched with skew")});
        if (detail_differs && history.detail_mismatch_count == 1)
          {
            spdlog::warn(
              "Core shadow: event detail differs: {} {} for break {} core_tick={} corenext_tick={} "
              "core_detail='{}' corenext_detail='{}'",
              corenext_event.source,
              corenext_event.name,
              corenext_event.break_id,
              core_event.tick,
              corenext_event.tick,
              core_event.detail,
              corenext_event.detail);
          }
        else if (detail_differs)
          {
            spdlog::debug(
              "Core shadow: repeated event detail difference #{}: {} {} for break {} core_tick={} corenext_tick={} "
              "core_detail='{}' corenext_detail='{}'",
              history.detail_mismatch_count,
              corenext_event.source,
              corenext_event.name,
              corenext_event.break_id,
              core_event.tick,
              corenext_event.tick,
              core_event.detail,
              corenext_event.detail);
          }
        if (best_distance != 0)
          {
            spdlog::debug(
              "Core shadow: event matched with skew: {} {} for break {} core_tick={} corenext_tick={} delta={}s detail='{}'",
              corenext_event.source,
              corenext_event.name,
              corenext_event.break_id,
              core_event.tick,
              corenext_event.tick,
              corenext_event.tick - core_event.tick,
              corenext_event.detail);
          }

        const auto high_index = std::max(next_index, best_index);
        const auto low_index = std::min(next_index, best_index);
        pending_events.erase(pending_events.begin() + static_cast<std::ptrdiff_t>(high_index));
        pending_events.erase(pending_events.begin() + static_cast<std::ptrdiff_t>(low_index));
        next_index = 0;
      }

    auto expired = std::remove_if(pending_events.begin(), pending_events.end(), [&](const auto &event) {
      if (tick - event.tick <= event_match_tolerance)
        {
          return false;
        }

      auto &history = event_history[event_history_key(event)];
      const auto missing_backend = event.backend == Backend::Core ? "corenext" : "core";
      if (event.backend == Backend::Core)
        {
          history.missing_from_corenext++;
        }
      else
        {
          history.missing_from_core++;
        }
      record_event_comparison(
        EventComparison{.source = event.source,
                        .break_id = event.break_id,
                        .name = event.name,
                        .core_detail = event.backend == Backend::Core ? event.detail : "",
                        .corenext_detail = event.backend == Backend::CoreNext ? event.detail : "",
                        .core_tick = event.backend == Backend::Core ? std::optional{event.tick} : std::nullopt,
                        .corenext_tick = event.backend == Backend::CoreNext ? std::optional{event.tick} : std::nullopt,
                        .result = std::string{"missing from "} + missing_backend});
      spdlog::warn("Core shadow: event missing from {} after {}s: {} {} for break {} observed_on={} tick={} detail='{}'",
                   missing_backend,
                   event_match_tolerance,
                   event.source,
                   event.name,
                   event.break_id,
                   backend_to_string(event.backend),
                   event.tick,
                   event.detail);
      return true;
    });
    pending_events.erase(expired, pending_events.end());
  }

  std::string CoreShadowComparator::get_event_debug_state_html() const
  {
    std::scoped_lock lock(mutex);
    std::ostringstream out;
    out << "<h4>Discrepancy episodes</h4>"
        << "<table cellspacing=\"0\" cellpadding=\"4\" border=\"1\">"
        << "<tr bgcolor=\"#e8e8e8\"><th>kind</th><th>break</th><th>status</th><th>started</th>"
        << "<th>current</th><th>worst</th></tr>";
    out << "<tr><td>activity</td><td>global</td>";
    append_value_cell(out, activity_differs ? "deviating" : "equal", activity_differs);
    out << "<td>" << (activity_differs ? std::to_string(activity_difference_started_at) : "-") << "</td><td></td><td></td></tr>";
    for (int break_id = 0; break_id < BREAK_ID_SIZEOF; ++break_id)
      {
        const auto timer = timer_discrepancies.find(break_id);
        const auto timer_active = timer != timer_discrepancies.end() && timer->second.active;
        out << "<tr><td>timer</td><td>" << break_name(break_id) << "</td>";
        append_value_cell(out, timer_active ? "deviating" : "equal", timer_active);
        out << "<td>" << (timer_active ? std::to_string(timer->second.started_at) : "-") << "</td><td>";
        if (timer_active)
          {
            out << "elapsed " << timer->second.elapsed_delta << "s, idle " << timer->second.idle_delta << "s";
          }
        out << "</td><td>" << (timer_active ? std::to_string(timer->second.worst_delta) + "s" : "-") << "</td></tr>";

        const auto state = state_discrepancies.find(break_id);
        const auto state_active = state != state_discrepancies.end() && state->second.active;
        out << "<tr><td>state</td><td>" << break_name(break_id) << "</td>";
        append_value_cell(out, state_active ? "deviating" : "equal", state_active);
        out << "<td>" << (state_active ? std::to_string(state->second.started_at) : "-") << "</td><td>"
            << (state_active ? std::to_string(state->second.current_count) + " fields" : "-") << "</td><td>"
            << (state_active ? std::to_string(state->second.worst_count) + " fields" : "-") << "</td></tr>";
      }
    out << "<tr><td>timer (lifetime)</td><td>all</td><td></td><td></td><td></td><td>" << max_timer_delta << "s</td></tr></table>";

    out << "<h4>Events</h4>";
    if (event_history.empty())
      {
        out << "<p>No events observed.</p>";
        return out.str();
      }

    out << "<table cellspacing=\"0\" cellpadding=\"4\" border=\"1\">"
        << "<tr bgcolor=\"#e8e8e8\"><th align=\"left\">source</th><th align=\"left\">break</th>"
        << "<th align=\"left\">event</th><th>core count</th><th>corenext count</th><th>matched</th><th>detail differs</th>"
        << "<th>missing core</th><th>missing corenext</th><th>last core tick</th><th>last corenext tick</th></tr>";

    for (const auto &[key, event]: event_history)
      {
        (void)key;
        const auto core_tick = event.core_tick ? std::to_string(*event.core_tick) : "-";
        const auto corenext_tick = event.corenext_tick ? std::to_string(*event.corenext_tick) : "-";
        const auto counts_differ = event.core_count != event.corenext_count;

        out << "<tr><td>" << html_escape(event.source) << "</td><td>" << break_name(event.break_id) << "</td><td>"
            << html_escape(event.name) << "</td>";
        append_value_cell(out, std::to_string(event.core_count), counts_differ);
        append_value_cell(out, std::to_string(event.corenext_count), counts_differ);
        append_value_cell(out, std::to_string(event.matched_count), false);
        append_value_cell(out, std::to_string(event.detail_mismatch_count), event.detail_mismatch_count != 0);
        append_value_cell(out, std::to_string(event.missing_from_core), event.missing_from_core != 0);
        append_value_cell(out, std::to_string(event.missing_from_corenext), event.missing_from_corenext != 0);
        append_value_cell(out, core_tick, !event.core_tick.has_value());
        append_value_cell(out, corenext_tick, !event.corenext_tick.has_value());
        out << "</tr>";
      }

    out << "</table>";
    out << "<h4>Recent event comparisons</h4>";
    if (recent_event_comparisons.empty())
      {
        out << "<p>Waiting for an event pair or an event matching timeout (" << event_match_tolerance << " seconds).</p>";
      }
    else
      {
        out << "<table cellspacing=\"0\" cellpadding=\"4\" border=\"1\">"
            << "<tr bgcolor=\"#e8e8e8\"><th>result</th><th>source</th><th>break</th><th>event</th>"
            << "<th>core detail</th><th>corenext detail</th><th>core tick</th><th>corenext tick</th><th>delta</th></tr>";
        for (const auto &event: recent_event_comparisons)
          {
            const auto missing = !event.core_tick || !event.corenext_tick;
            const auto differs = missing || event.core_detail != event.corenext_detail;
            std::string delta{"-"};
            if (!missing)
              {
                const auto value = *event.corenext_tick - *event.core_tick;
                delta = (value > 0 ? "+" : "") + std::to_string(value);
              }
            out << "<tr>";
            append_value_cell(out, html_escape(event.result), differs);
            out << "<td>" << html_escape(event.source) << "</td><td>" << break_name(event.break_id) << "</td><td>"
                << html_escape(event.name) << "</td><td>" << html_escape(event.core_detail) << "</td><td>"
                << html_escape(event.corenext_detail) << "</td>";
            append_value_cell(out, event.core_tick ? std::to_string(*event.core_tick) : "-", differs);
            append_value_cell(out, event.corenext_tick ? std::to_string(*event.corenext_tick) : "-", differs);
            append_value_cell(out, delta, differs);
            out << "</tr>";
          }
        out << "</table>";
      }
    return out.str();
  }

  void CoreShadowComparator::compare(int64_t tick,
                                     const std::vector<TimerSnapshot> &live_snapshots,
                                     const ObservationBatch &shadow_batch)
  {
    std::scoped_lock lock(mutex);
    for (const auto &event: shadow_batch.events)
      {
        record_event_history(event);
        pending_events.push_back(event);
      }
    match_and_expire_events(tick);

    for (const auto &core_snapshot: shadow_batch.snapshots)
      {
        auto live_snapshot = std::find_if(live_snapshots.begin(), live_snapshots.end(), [&](const auto &snapshot) {
          return snapshot.break_id == core_snapshot.break_id;
        });
        if (live_snapshot == live_snapshots.end())
          {
            continue;
          }

        const auto activity_now_differs = live_snapshot->user_active != core_snapshot.user_active;
        if (activity_now_differs && !activity_differs)
          {
            activity_differs = true;
            activity_difference_started_at = tick;
            spdlog::warn("Core shadow: activity deviation started at tick {}: core user_active={}, corenext user_active={}",
                         tick,
                         core_snapshot.user_active,
                         live_snapshot->user_active);
          }
        else if (!activity_now_differs && activity_differs)
          {
            spdlog::info("Core shadow: activity deviation resolved at tick {} after {}s: user_active={}",
                         tick,
                         tick - activity_difference_started_at,
                         core_snapshot.user_active);
            activity_differs = false;
          }

        const auto elapsed_delta = live_snapshot->elapsed - core_snapshot.elapsed;
        const auto idle_delta = live_snapshot->idle - core_snapshot.idle;
        const auto timer_delta = std::max(std::abs(elapsed_delta), std::abs(idle_delta));
        max_timer_delta = std::max(max_timer_delta, timer_delta);
        auto &timer_state = timer_discrepancies[core_snapshot.break_id];

        if (timer_delta >= timer_warning_threshold && !timer_state.active)
          {
            timer_state = TimerDiscrepancy{.active = true,
                                           .started_at = tick,
                                           .elapsed_delta = elapsed_delta,
                                           .idle_delta = idle_delta,
                                           .worst_delta = timer_delta};
            spdlog::warn(
              "Core shadow: break {} timer deviation started at tick {}: elapsed_delta={}s idle_delta={}s "
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
        else if (timer_state.active && timer_delta > timer_state.worst_delta)
          {
            timer_state.elapsed_delta = elapsed_delta;
            timer_state.idle_delta = idle_delta;
            timer_state.worst_delta = timer_delta;
            spdlog::warn(
              "Core shadow: break {} timer deviation worsened at tick {}: elapsed_delta={}s idle_delta={}s worst={}s "
              "(core elapsed={} idle={}, corenext elapsed={} idle={})",
              core_snapshot.break_id,
              tick,
              elapsed_delta,
              idle_delta,
              timer_state.worst_delta,
              core_snapshot.elapsed,
              core_snapshot.idle,
              live_snapshot->elapsed,
              live_snapshot->idle);
          }
        else if (timer_state.active && timer_delta < timer_warning_threshold)
          {
            spdlog::info(
              "Core shadow: break {} timer deviation resolved at tick {} after {}s: "
              "elapsed_delta={}s idle_delta={}s worst={}s",
              core_snapshot.break_id,
              tick,
              tick - timer_state.started_at,
              elapsed_delta,
              idle_delta,
              timer_state.worst_delta);
            timer_state = {};
          }

        const auto state_difference_count = static_cast<int>(live_snapshot->enabled != core_snapshot.enabled)
                                            + static_cast<int>(live_snapshot->running != core_snapshot.running)
                                            + static_cast<int>(live_snapshot->taking != core_snapshot.taking)
                                            + static_cast<int>(live_snapshot->active != core_snapshot.active);
        auto &state = state_discrepancies[core_snapshot.break_id];
        if (state_difference_count != 0 && !state.active)
          {
            state = StateDiscrepancy{.active = true,
                                     .started_at = tick,
                                     .current_count = state_difference_count,
                                     .worst_count = state_difference_count};
            spdlog::warn(
              "Core shadow: break {} state deviation started at tick {} ({} fields): "
              "core enabled={} running={} taking={} active={}, "
              "corenext enabled={} running={} taking={} active={}",
              core_snapshot.break_id,
              tick,
              state_difference_count,
              core_snapshot.enabled,
              core_snapshot.running,
              core_snapshot.taking,
              core_snapshot.active,
              live_snapshot->enabled,
              live_snapshot->running,
              live_snapshot->taking,
              live_snapshot->active);
          }
        else if (state.active && state_difference_count > state.worst_count)
          {
            state.worst_count = state_difference_count;
            spdlog::warn(
              "Core shadow: break {} state deviation worsened at tick {} ({} fields): "
              "core enabled={} running={} taking={} active={}, "
              "corenext enabled={} running={} taking={} active={}",
              core_snapshot.break_id,
              tick,
              state_difference_count,
              core_snapshot.enabled,
              core_snapshot.running,
              core_snapshot.taking,
              core_snapshot.active,
              live_snapshot->enabled,
              live_snapshot->running,
              live_snapshot->taking,
              live_snapshot->active);
          }
        else if (state.active && state_difference_count == 0)
          {
            spdlog::info("Core shadow: break {} state deviation resolved at tick {} after {}s; worst={} fields",
                         core_snapshot.break_id,
                         tick,
                         tick - state.started_at,
                         state.worst_count);
            state = {};
          }
        if (timer_state.active)
          {
            timer_state.elapsed_delta = elapsed_delta;
            timer_state.idle_delta = idle_delta;
          }
        if (state.active)
          {
            state.current_count = state_difference_count;
          }
      }
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

  std::string BreakShadowProxy::get_name() const
  {
    return live_break->get_name();
  }
  bool BreakShadowProxy::is_enabled() const
  {
    return live_break->is_enabled();
  }
  bool BreakShadowProxy::is_running() const
  {
    return live_break->is_running();
  }
  bool BreakShadowProxy::is_taking() const
  {
    return live_break->is_taking();
  }
  bool BreakShadowProxy::is_max_preludes_reached() const
  {
    return live_break->is_max_preludes_reached();
  }
  bool BreakShadowProxy::is_active() const
  {
    return live_break->is_active();
  }
  int64_t BreakShadowProxy::get_elapsed_time() const
  {
    return live_break->get_elapsed_time();
  }
  int64_t BreakShadowProxy::get_elapsed_idle_time() const
  {
    return live_break->get_elapsed_idle_time();
  }
  int64_t BreakShadowProxy::get_auto_reset() const
  {
    return live_break->get_auto_reset();
  }
  bool BreakShadowProxy::is_auto_reset_enabled() const
  {
    return live_break->is_auto_reset_enabled();
  }
  int64_t BreakShadowProxy::get_limit() const
  {
    return live_break->get_limit();
  }
  bool BreakShadowProxy::is_limit_enabled() const
  {
    return live_break->is_limit_enabled();
  }
  int64_t BreakShadowProxy::get_total_overdue_time() const
  {
    return live_break->get_total_overdue_time();
  }

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
#if defined(HAVE_GRPC)
    rpc_interceptor.reset();
#endif
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
#if defined(HAVE_GRPC)
    rpc_interceptor = rpc::register_request_interceptor(
      [this](const rpc::RequestInfo &request) { intercept_rpc_request(request); });
#endif
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

  workrave::stats::IStatistics::Ptr CoreShadowProxy::get_statistics() const
  {
    return live_core->get_statistics();
  }
  bool CoreShadowProxy::is_user_active() const
  {
    return live_core->is_user_active();
  }
  bool CoreShadowProxy::is_taking() const
  {
    return live_core->is_taking();
  }
  OperationMode CoreShadowProxy::get_active_operation_mode()
  {
    return live_core->get_active_operation_mode();
  }
  OperationMode CoreShadowProxy::get_regular_operation_mode()
  {
    return live_core->get_regular_operation_mode();
  }

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

  bool CoreShadowProxy::is_operation_mode_an_override()
  {
    return live_core->is_operation_mode_an_override();
  }
  UsageMode CoreShadowProxy::get_usage_mode()
  {
    return live_core->get_usage_mode();
  }

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

  void CoreShadowProxy::report_external_activity(std::string who, bool active)
  {
    live_core->report_external_activity(who, active);
    shadow_command(command_line({"report_activity", std::move(who), active ? "1" : "0"}));
  }

  ICoreHooks::Ptr CoreShadowProxy::get_hooks() const
  {
    return live_core->get_hooks();
  }

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

#if defined(HAVE_GRPC)
  void CoreShadowProxy::intercept_rpc_request(const rpc::RequestInfo &request)
  {
    try
      {
        if (auto command = rpc_shadow_command(request))
          {
            shadow_command(*command);
          }
      }
    catch (const std::exception &error)
      {
        spdlog::warn("Core shadow could not mirror gRPC call {}/{}: {}", request.service, request.method, error.what());
      }
  }
#endif

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
