// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GlibLogging.hh"

#include <cstring>
#include <mutex>
#include <string_view>

#include <glib.h>
#include <spdlog/spdlog.h>

namespace
{
  void log_message(GLogLevelFlags flags, std::string_view domain, std::string_view message)
  {
    auto level = spdlog::level::info;
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL))
      {
        level = spdlog::level::critical;
      }
    else if (flags & G_LOG_LEVEL_CRITICAL)
      {
        level = spdlog::level::err;
      }
    else if (flags & G_LOG_LEVEL_WARNING)
      {
        level = spdlog::level::warn;
      }
    else if (flags & G_LOG_LEVEL_DEBUG)
      {
        level = spdlog::level::debug;
      }

    auto logger = spdlog::default_logger();
    logger->log(level, "[{}] {}", domain, message);
    // GLib may abort as soon as the callback returns, including for warnings
    // made fatal by G_DEBUG. Do not rely on normal shutdown or flush_on().
    logger->flush();
  }

  void legacy_log(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer)
  {
    try
      {
        log_message(level, domain != nullptr ? domain : "GLib", message != nullptr ? message : "");
      }
    catch (...)
      {
        // Never propagate a C++ exception into GLib's C logging machinery.
        g_log_default_handler(domain, level, message, nullptr);
      }
  }

  GLogWriterOutput structured_log(GLogLevelFlags level, const GLogField *fields, gsize count, gpointer)
  {
    try
      {
        std::string_view domain = "GLib";
        std::string_view message;
        for (gsize i = 0; i < count; ++i)
          {
            const auto &field = fields[i];
            if (field.value == nullptr)
              {
                continue;
              }
            // Only interpret the documented text fields; other fields may be pointers.
            if (std::strcmp(field.key, "GLIB_DOMAIN") == 0 || std::strcmp(field.key, "MESSAGE") == 0)
              {
                const auto *text = static_cast<const char *>(field.value);
                std::string_view value(text, field.length < 0 ? std::strlen(text) : static_cast<size_t>(field.length));
                if (std::strcmp(field.key, "GLIB_DOMAIN") == 0)
                  {
                    domain = value;
                  }
                else
                  {
                    message = value;
                  }
              }
          }
        log_message(level, domain, message);
      }
    catch (...)
      {
        // Keep GLib's stderr fallback available if file logging fails.
      }
    // Preserve GLib's default fatal-warning handling (including G_DEBUG),
    // after the message has been flushed to the application's log file.
    return g_log_writer_default(level, fields, count, nullptr);
  }
} // namespace

void
init_glib_logging()
{
  static std::once_flag initialized;
  std::call_once(initialized, []() {
    g_log_set_default_handler(legacy_log, nullptr);
    g_log_set_writer_func(structured_log, nullptr, nullptr);
  });
}
