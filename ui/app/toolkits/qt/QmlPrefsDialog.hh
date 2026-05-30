// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#ifndef QMLPREFSDIALOG_HH
#define QMLPREFSDIALOG_HH

#include <map>
#include <memory>
#include <string>
#include <QObject>
#include <QPointer>
#include <QString>

class QQuickView;
class MicrobreakPrefBridge;
class RestBreakPrefBridge;
class DailyLimitPrefBridge;
class StatusWindowPrefBridge;
class AppletPrefBridge;
class GeneralPrefBridge;
class MonitoringPrefBridge;
class SoundsPrefBridge;
class ActivePluginPageBridge;
class PluginPageBridge;

#include "ui/IApplicationContext.hh"

class QmlPrefsDialog : public QObject
{
  Q_OBJECT

public:
  explicit QmlPrefsDialog(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);
  ~QmlPrefsDialog() override;

  void show();
  void hide();

private Q_SLOTS:
  void onNavigateTo(const QString &section, const QString &page);
  void onCloseRequested();
  void onQmlStatusChanged(int status);

private:
  void create_plugin_bridges();

  std::shared_ptr<IApplicationContext> app;
  QQuickView *view{nullptr};
  MicrobreakPrefBridge   *microbreakBridge{nullptr};
  RestBreakPrefBridge    *restBreakBridge{nullptr};
  DailyLimitPrefBridge   *dailyLimitBridge{nullptr};
  StatusWindowPrefBridge *statusWindowBridge{nullptr};
  AppletPrefBridge       *appletBridge{nullptr};
  GeneralPrefBridge      *generalBridge{nullptr};
  MonitoringPrefBridge   *monitoringBridge{nullptr};
  SoundsPrefBridge       *soundsBridge{nullptr};

  ActivePluginPageBridge                 *activeBridge{nullptr};
  ActivePluginPageBridge                 *extensionBridge{nullptr};
  std::map<std::string, PluginPageBridge *> pluginBridges;
};

#endif // QMLPREFSDIALOG_HH
