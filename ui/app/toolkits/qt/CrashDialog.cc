// Copyright (C) 2020-2021 Rob Caelers <robc@krandor.nl>
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

#include "CrashDialog.hh"
#include "QmlCrashDialog.hh"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <clocale>
#include <filesystem>

#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>

#include "base/logging.h"
#include "handler/handler_main.h"
#include "build/build_config.h"
#include "tools/tool_support.h"

#include <QApplication>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "utils/Paths.hh"

bool
UserInteraction::requestUserConsent(const std::map<std::string, std::string> &annotations,
                                    std::vector<base::FilePath> &attachments,
                                    const crashpad::CrashSummary &summary)
{
  SetEnvironmentVariableA("GTK_DEBUG", nullptr);
  SetEnvironmentVariableA("G_MESSAGES_DEBUG", nullptr);
  SetEnvironmentVariableA("GTK_OVERLAY_SCROLLING", "0");
  SetEnvironmentVariableA("GTK_CSD", "0");
  SetEnvironmentVariableA("GDK_WIN32_DISABLE_HIDPI", "1");

  LOG(INFO) << "Creating user consent app.";
  int argc = 0;
  char **argv = nullptr;
  QApplication app(argc, argv);

  LOG(INFO) << "Creating user consent dialog.";
  QmlCrashDialog dlg(annotations, attachments, summary);
  dlg.exec();

  user_text = dlg.getUserText();
  consent = dlg.getConsent();
  attachments = dlg.getSelectedAttachments();

  LOG(INFO) << "User consent complete:" << consent;
  return consent;
}

std::string
UserInteraction::getUserText()
{
  return user_text;
}

void
UserInteraction::reportCompleted(const crashpad::UUID &uuid)
{
  LOG(INFO) << "Report filed as: " << uuid.ToString();
}

namespace
{
  int HandlerMainAdaptor(int argc, char *argv[])
  {
    const std::filesystem::path log_dir = workrave::utils::Paths::get_log_directory();
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((log_dir / "workrave-crashhandler.log").string(),
                                                                            1024 * 1024,
                                                                            5,
                                                                            true);
    auto logger = std::make_shared<spdlog::logger>("crashhandler", file_sink);
    logger->flush_on(spdlog::level::critical);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");

    logging::SetLogMessageHandler(
      [](logging::LogSeverity severity, const char *file_path, int line, size_t message_start, const std::string &string) {
        (void)file_path;
        (void)line;

        std::string msg = string.substr(message_start);
        if (!msg.empty() && msg.back() == '\n')
          {
            msg.pop_back();
          }
        switch (severity)
          {
          case logging::LOG_VERBOSE:
            spdlog::debug("Crashpad: {}", msg);
            break;
          case logging::LOG_INFO:
            spdlog::info("Crashpad: {}", msg);
            break;
          case logging::LOG_WARNING:
            spdlog::warn("Crashpad: {}", msg);
            break;
          case logging::LOG_ERROR:
            spdlog::error("Crashpad: {}", msg);
            break;
          case logging::LOG_FATAL:
            spdlog::critical("Crashpad: {}", msg);
            break;
          default:
            spdlog::trace("Crashpad: {}", msg);
            break;
          }
        return false;
      });

    logging::SetMinLogLevel(logging::LOG_VERBOSE);

    LOG(INFO) << "Workrave crashed.";
    auto *user_interaction = new UserInteraction;
    int ret = crashpad::HandlerMain(argc, argv, nullptr, user_interaction);
    LOG(INFO) << "Crash handled";
    delete user_interaction;
    LOG(INFO) << "Exit:" << ret;
    spdlog::shutdown();
    return ret;
  }
} // namespace

int APIENTRY
wWinMain(HINSTANCE, HINSTANCE, wchar_t *, int)
{
  return crashpad::ToolSupport::Wmain(__argc, __wargv, HandlerMainAdaptor);
}

int
wmain(int argc, wchar_t *argv[])
{
  return crashpad::ToolSupport::Wmain(argc, argv, HandlerMainAdaptor);
}
