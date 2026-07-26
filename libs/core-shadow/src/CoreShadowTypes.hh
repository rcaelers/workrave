// Copyright (C) 2026 Rob Caelers
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef WORKRAVE_CORE_SHADOW_TYPES_HH
#define WORKRAVE_CORE_SHADOW_TYPES_HH

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace workrave::core_shadow
{
  enum class Backend
  {
    Core,
    CoreNext,
  };

  struct EventObservation
  {
    Backend backend{Backend::Core};
    int64_t tick{0};
    std::string source;
    int break_id{-1};
    std::string name;
    std::string detail;
  };

  struct TimerSnapshot
  {
    Backend backend{Backend::Core};
    int64_t tick{0};
    int break_id{-1};
    int64_t elapsed{0};
    int64_t idle{0};
    int64_t limit{0};
    int64_t auto_reset{0};
    bool enabled{false};
    bool running{false};
    bool taking{false};
    bool active{false};
    bool user_active{false};
  };

  struct ObservationBatch
  {
    std::vector<EventObservation> events;
    std::vector<TimerSnapshot> snapshots;
  };

  auto backend_to_string(Backend backend) -> std::string;
  auto backend_from_string(const std::string &text) -> std::optional<Backend>;

  auto escape_field(const std::string &text) -> std::string;
  auto unescape_field(const std::string &text) -> std::string;
  auto split_line(const std::string &line) -> std::vector<std::string>;

  auto serialize_event(const EventObservation &event) -> std::string;
  auto serialize_snapshot(const TimerSnapshot &snapshot) -> std::string;
  auto parse_observation(const std::string &line, ObservationBatch &batch) -> bool;
} // namespace workrave::core_shadow

#endif // WORKRAVE_CORE_SHADOW_TYPES_HH
