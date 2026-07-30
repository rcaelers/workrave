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

#include "DBusPreludeWindow.hh"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QString>
#include <QVariant>

#include <utility>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "ui/GUIConfig.hh"
#include "utils/AssetPath.hh"
#include "utils/Enum.hh"
#include "utils/Exception.hh"
#include "commonui/Text.hh"

using namespace workrave;
using namespace workrave::utils;

namespace
{
  constexpr const char *service = "org.workrave.GnomeShellApplet";
  constexpr const char *object_path = "/org/workrave/Workrave/Preludes";
  constexpr const char *interface = "org.workrave.Preludes";

  //! Resolves an image name against the configured icon theme.
  std::string
  get_image_filename(const std::string &image)
  {
    std::string theme = GUIConfig::icon_theme()();
    if (!theme.empty())
      {
        theme += "/";
      }

    std::string full_path;
    if (!AssetPath::complete_directory(theme + image, SearchPathId::Images, full_path))
      {
        AssetPath::complete_directory(image, SearchPathId::Images, full_path);
      }

    return full_path;
  }
} // namespace

class DBusPreludeWindow::Impl
{
public:
  Impl()
  {
    proxy = std::make_unique<QDBusInterface>(QString::fromLatin1(service),
                                             QString::fromLatin1(object_path),
                                             QString::fromLatin1(interface),
                                             QDBusConnection::sessionBus());
    if (!proxy->isValid())
      {
        spdlog::error("Failed to create D-Bus proxy for prelude window: {}",
                      proxy->lastError().message().toStdString());
        throw workrave::utils::Exception("Failed to create D-Bus proxy for prelude window");
      }

    // The gtkmm toolkit lets the GTK theme override these through CSS. Qt has
    // no equivalent, so the defaults are used as-is.
    const QString color_warn = QStringLiteral("orange");
    const QString color_alert = QStringLiteral("red");

    call("Init",
         QString::fromStdString(get_image_filename("prelude-hint.png")),
         QString::fromStdString(get_image_filename("prelude-hint-sad.png")),
         color_warn,
         color_alert);

    // Older extension versions do not expose SetSanctuary.
    if (!try_call("SetSanctuary", GUIConfig::sanctuary_ui_enabled()()))
      {
        spdlog::debug("Workrave GNOME shell extension does not support Sanctuary preludes");
      }
  }

  Impl(const Impl &) = delete;
  Impl &operator=(const Impl &) = delete;
  Impl(Impl &&) = delete;
  Impl &operator=(Impl &&) = delete;

  ~Impl()
  {
    try_call("Terminate");
  }

  //! Calls @p method, logging any error. Returns false when the call failed.
  template<typename... Args>
  bool try_call(const char *method, Args &&...args)
  {
    QDBusMessage reply = proxy->call(QString::fromLatin1(method), QVariant::fromValue(std::forward<Args>(args))...);
    if (reply.type() == QDBusMessage::ErrorMessage)
      {
        spdlog::debug("D-Bus prelude call '{}' failed: {}", method, reply.errorMessage().toStdString());
        return false;
      }
    return true;
  }

  //! Calls @p method, throwing on failure.
  template<typename... Args>
  void call(const char *method, Args &&...args)
  {
    if (!try_call(method, std::forward<Args>(args)...))
      {
        throw workrave::utils::Exception("D-Bus prelude call failed");
      }
  }

private:
  std::unique_ptr<QDBusInterface> proxy;
};

DBusPreludeWindow::DBusPreludeWindow(BreakId break_id)
  : impl(std::make_unique<Impl>())
  , break_id(break_id)
{
}

DBusPreludeWindow::~DBusPreludeWindow() = default;

void
DBusPreludeWindow::start()
{
  impl->try_call("Start", QString::fromStdString(get_title(break_id)));
}

void
DBusPreludeWindow::stop()
{
  impl->try_call("Stop");
}

void
DBusPreludeWindow::refresh()
{
  impl->try_call("Refresh");
}

void
DBusPreludeWindow::set_progress(int value, int max_value)
{
  impl->try_call("SetProgress", value, max_value);
  auto text = fmt::format(fmt::runtime(progress_text), Text::time_to_string(max_value - value));
  impl->try_call("SetProgressText", QString::fromStdString(text));
}

void
DBusPreludeWindow::set_stage(workrave::IApp::PreludeStage stage)
{
  impl->try_call("SetStage", QString::fromStdString(std::string(workrave::utils::enum_to_string(stage))));
}

void
DBusPreludeWindow::set_progress_text(workrave::IApp::PreludeProgressText text)
{
  progress_text = get_progress_text(text);
}

bool
DBusPreludeWindow::is_gnome_shell_applet_available()
{
  QDBusConnection bus = QDBusConnection::sessionBus();
  if (!bus.isConnected())
    {
      spdlog::debug("Unable to connect to the session bus");
      return false;
    }

  QDBusReply<bool> reply = bus.interface()->isServiceRegistered(QString::fromLatin1(service));
  if (!reply.isValid())
    {
      spdlog::debug("Unable to query the GNOME Shell applet bus name: {}", reply.error().message().toStdString());
      return false;
    }

  return reply.value();
}
