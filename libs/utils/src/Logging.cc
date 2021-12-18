// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include "utils/Logging.hh"

#include <memory>
#include <filesystem>
#include <initializer_list>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "utils/Paths.hh"

using namespace workrave::utils;

void
Logging::init()
{
  const auto log_dir = Paths::get_log_directory();
  std::filesystem::create_directory(log_dir);

  const auto log_file = log_dir / "workrave.log";

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file.string(), 1024 * 1024, 5, false);

  auto logger{std::make_shared<spdlog::logger>("workrave", std::initializer_list<spdlog::sink_ptr>{console_sink, file_sink})};
  spdlog::set_default_logger(logger);

  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");

  spdlog::debug("Logging started.");
}

std::shared_ptr<spdlog::logger>
Logging::create(std::string domain)
{
  return spdlog::default_logger()->clone(domain);
}
