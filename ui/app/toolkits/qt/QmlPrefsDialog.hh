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

#ifndef Q_MOC_RUN
#  include <map>
#  include <memory>
#  include <string>
#  include <QObject>
#  include <QPointer>
#  include <QString>
#  include <QVariantList>
#endif

class QQuickView;
class MicrobreakPrefBridge;
class RestBreakPrefBridge;
class DailyLimitPrefBridge;
class StatusWindowPrefBridge;
class AppletPrefBridge;
class GeneralPrefBridge;
class RemoteControlPrefBridge;
class MonitoringPrefBridge;
class SoundsPrefBridge;
namespace ui::prefwidgets::qtquick
{
  class ActivePageModel;
  class PageModel;
} // namespace ui::prefwidgets::qtquick

#ifndef Q_MOC_RUN
#  include "ui/IApplicationContext.hh"
#endif

class QmlPrefsDialog : public QObject
{
  Q_OBJECT

public:
  explicit QmlPrefsDialog(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);
  ~QmlPrefsDialog() override;

  void select_page(const std::string &section, const std::string &page);
  void show();
  void hide();
  void retranslate();

private Q_SLOTS:
  void onNavigateTo(const QString &section, const QString &page);
  void onCloseRequested();
  void onQmlStatusChanged(int status);

private:
  using ActivePageModel = ui::prefwidgets::qtquick::ActivePageModel;
  using PageModel = ui::prefwidgets::qtquick::PageModel;
  void create_plugin_bridges();
  QVariantList create_plugin_navigation();

  std::shared_ptr<IApplicationContext> app;
  QQuickView *view{nullptr};
  MicrobreakPrefBridge *microbreakBridge{nullptr};
  RestBreakPrefBridge *restBreakBridge{nullptr};
  DailyLimitPrefBridge *dailyLimitBridge{nullptr};
  StatusWindowPrefBridge *statusWindowBridge{nullptr};
  AppletPrefBridge *appletBridge{nullptr};
  GeneralPrefBridge *generalBridge{nullptr};
  RemoteControlPrefBridge *remoteControlBridge{nullptr};
  MonitoringPrefBridge *monitoringBridge{nullptr};
  SoundsPrefBridge *soundsBridge{nullptr};

  ActivePageModel *activeBridge{nullptr};
  ActivePageModel *extensionBridge{nullptr};
  ActivePageModel *topExtensionBridge{nullptr};
  std::map<std::string, PageModel *> pluginBridges;
  std::map<std::string, PageModel *> topExtensionBridges;
  std::map<std::string, PageModel *> bottomExtensionBridges;
  std::map<std::string, std::string> pluginSections;
};

#endif // QMLPREFSDIALOG_HH
