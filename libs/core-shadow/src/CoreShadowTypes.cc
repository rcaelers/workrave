#include "CoreShadowTypes.hh"

#include <sstream>

namespace workrave::core_shadow
{
  auto
  backend_to_string(Backend backend) -> std::string
  {
    return backend == Backend::Core ? "core" : "corenext";
  }

  auto
  backend_from_string(const std::string &text) -> std::optional<Backend>
  {
    if (text == "core")
      {
        return Backend::Core;
      }
    if (text == "corenext")
      {
        return Backend::CoreNext;
      }
    return std::nullopt;
  }

  auto
  escape_field(const std::string &text) -> std::string
  {
    std::string out;
    out.reserve(text.size());
    for (char c: text)
      {
        switch (c)
          {
          case '\\':
            out += "\\\\";
            break;
          case '\t':
            out += "\\t";
            break;
          case '\n':
            out += "\\n";
            break;
          default:
            out += c;
            break;
          }
      }
    return out;
  }

  auto
  unescape_field(const std::string &text) -> std::string
  {
    std::string out;
    out.reserve(text.size());
    bool escaped = false;
    for (char c: text)
      {
        if (escaped)
          {
            switch (c)
              {
              case 't':
                out += '\t';
                break;
              case 'n':
                out += '\n';
                break;
              default:
                out += c;
                break;
              }
            escaped = false;
          }
        else if (c == '\\')
          {
            escaped = true;
          }
        else
          {
            out += c;
          }
      }
    if (escaped)
      {
        out += '\\';
      }
    return out;
  }

  auto
  split_line(const std::string &line) -> std::vector<std::string>
  {
    std::vector<std::string> fields;
    std::string field;
    std::istringstream in(line);
    while (std::getline(in, field, '\t'))
      {
        fields.push_back(unescape_field(field));
      }
    return fields;
  }

  auto
  serialize_event(const EventObservation &event) -> std::string
  {
    std::ostringstream out;
    out << "event"
        << '\t' << backend_to_string(event.backend)
        << '\t' << event.tick
        << '\t' << escape_field(event.source)
        << '\t' << event.break_id
        << '\t' << escape_field(event.name)
        << '\t' << escape_field(event.detail);
    return out.str();
  }

  auto
  serialize_snapshot(const TimerSnapshot &snapshot) -> std::string
  {
    std::ostringstream out;
    out << "snapshot"
        << '\t' << backend_to_string(snapshot.backend)
        << '\t' << snapshot.tick
        << '\t' << snapshot.break_id
        << '\t' << snapshot.elapsed
        << '\t' << snapshot.idle
        << '\t' << snapshot.limit
        << '\t' << snapshot.auto_reset
        << '\t' << (snapshot.enabled ? 1 : 0)
        << '\t' << (snapshot.running ? 1 : 0)
        << '\t' << (snapshot.taking ? 1 : 0)
        << '\t' << (snapshot.active ? 1 : 0)
        << '\t' << (snapshot.user_active ? 1 : 0);
    return out.str();
  }

  auto
  parse_observation(const std::string &line, ObservationBatch &batch) -> bool
  {
    auto fields = split_line(line);
    if (fields.empty())
      {
        return false;
      }

    if (fields[0] == "event" && fields.size() >= 7)
      {
        auto backend = backend_from_string(fields[1]);
        if (!backend)
          {
            return false;
          }
        batch.events.push_back(EventObservation{.backend = *backend,
                                                .tick = std::stoll(fields[2]),
                                                .source = fields[3],
                                                .break_id = std::stoi(fields[4]),
                                                .name = fields[5],
                                                .detail = fields[6]});
        return true;
      }

    if (fields[0] == "snapshot" && fields.size() >= 12)
      {
        auto backend = backend_from_string(fields[1]);
        if (!backend)
          {
            return false;
          }
        batch.snapshots.push_back(TimerSnapshot{.backend = *backend,
                                                .tick = std::stoll(fields[2]),
                                                .break_id = std::stoi(fields[3]),
                                                .elapsed = std::stoll(fields[4]),
                                                .idle = std::stoll(fields[5]),
                                                .limit = std::stoll(fields[6]),
                                                .auto_reset = std::stoll(fields[7]),
                                                .enabled = fields[8] == "1",
                                                .running = fields[9] == "1",
                                                .taking = fields[10] == "1",
                                                .active = fields[11] == "1",
                                                .user_active = fields.size() >= 13 && fields[12] == "1"});
        return true;
      }

    return false;
  }
} // namespace workrave::core_shadow
