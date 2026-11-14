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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "QmlPrefsDialog.hh"

#include <algorithm>
#include <set>

#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>
#include <QGuiApplication>

#include "QmlPrefsBridges.hh"
#include "QmlPluginPrefBridges.hh"
#include "utils/Ui.hh"

QmlPrefsDialog::QmlPrefsDialog(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
  microbreakBridge   = new MicrobreakPrefBridge(this->app, this);
  restBreakBridge    = new RestBreakPrefBridge(this->app, this);
  dailyLimitBridge   = new DailyLimitPrefBridge(this->app, this);
  statusWindowBridge = new StatusWindowPrefBridge(this->app, this);
  appletBridge       = new AppletPrefBridge(this->app, this);
  generalBridge      = new GeneralPrefBridge(this->app, this);
  remoteControlBridge = new RemoteControlPrefBridge(this);
  monitoringBridge   = new MonitoringPrefBridge(this->app, this);
  soundsBridge       = new SoundsPrefBridge(this->app, this);
  activeBridge    = new ActivePluginPageBridge(this);
  extensionBridge = new ActivePluginPageBridge(this);
  topExtensionBridge = new ActivePluginPageBridge(this);
  create_plugin_bridges();

  view = new QQuickView();
  Ui::add_qml_import_paths(view->engine());
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  view->setTitle(tr("Workrave — Preferences"));
  view->setMinimumSize(QSize(720, 520));
  view->resize(880, 620);

  auto *ctx = view->rootContext();
  ctx->setContextProperty("microbreakPrefBridge",   microbreakBridge);
  ctx->setContextProperty("restBreakPrefBridge",    restBreakBridge);
  ctx->setContextProperty("dailyLimitPrefBridge",   dailyLimitBridge);
  ctx->setContextProperty("statusWindowPrefBridge", statusWindowBridge);
  ctx->setContextProperty("appletPrefBridge",       appletBridge);
  ctx->setContextProperty("generalPrefBridge",      generalBridge);
  ctx->setContextProperty("remoteControlPrefBridge", remoteControlBridge);
  ctx->setContextProperty("monitoringPrefBridge",   monitoringBridge);
  ctx->setContextProperty("soundsPrefBridge",       soundsBridge);
  ctx->setContextProperty("activePluginBridge",  activeBridge);
  ctx->setContextProperty("pageExtensionBridge", extensionBridge);
  ctx->setContextProperty("pageTopExtensionBridge", topExtensionBridge);
#ifdef PLATFORM_OS_UNIX
  constexpr bool builtinApplet = true;
#else
  constexpr bool builtinApplet = false;
#endif
  ctx->setContextProperty("prefHasBuiltinApplet", builtinApplet);

  ctx->setContextProperty("prefPluginNavEntries", create_plugin_navigation());

  QObject::connect(view, &QQuickView::statusChanged, this, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err : view->errors())
          {
            spdlog::error("PrefsShell QML error: {}", err.toString().toStdString());
          }
      }
    else if (status == QQuickView::Ready)
      {
        QQuickItem *root = view->rootObject();
        if (root != nullptr)
          {
            QObject::connect(root, SIGNAL(navigateTo(QString, QString)), this, SLOT(onNavigateTo(QString, QString)));
            QObject::connect(root, SIGNAL(closeRequested()), this, SLOT(onCloseRequested()));
            onNavigateTo(root->property("currentSection").toString(), root->property("currentPage").toString());
          }
      }
  });

  view->setSource(QUrl("qrc:/sanctuary/PrefsShell.qml"));

  QObject::connect(view, &QWindow::visibleChanged, this, &QmlPrefsDialog::signal_visibility_changed);
}

auto
QmlPrefsDialog::is_visible() const -> bool
{
  return view != nullptr && view->isVisible();
}

QVariantList
QmlPrefsDialog::create_plugin_navigation()
{
  // Build nav entries for plugin pages
  QVariantList pluginNavEntries;
  auto registry = this->app->get_internal_preferences_registry();
  auto pluginPages = registry->get_pages();
  auto panelDefinitions = registry->get_widgets();
  std::map<std::string, std::string> sections{{"ui", "ui"}, {"timer", "timers"}, {"monitoring", "timers"}, {"sounds", "ui"}};
  std::set<std::string> builtinPanels{
    "ui.general",
    "ui.mainwindow",
    "timer.microbreak",
    "timer.restbreak",
    "timer.dailylimit",
    "monitoring.monitoring",
    "sounds.sounds",
  };
#ifdef PLATFORM_OS_UNIX
  builtinPanels.insert("ui.applet");
#endif
#ifdef HAVE_GRPC
  sections.emplace("remote-control", "remote-control");
  builtinPanels.insert("remote-control.grpc");
#endif
  for (const auto &[location, definitions]: panelDefinitions)
    {
      if (definitions.empty())
        {
          continue;
        }
      auto def = definitions.front();
      auto locationId = def->get_page() + "." + def->get_panel();
      if (builtinPanels.contains(locationId))
        {
          continue;
        }
      auto page = pluginPages.find(def->get_page());
      auto host = sections.find(def->get_page());
      if (page == pluginPages.end() && host == sections.end())
        {
          spdlog::error("Cannot add preferences panel '{}' to unknown page '{}'", def->get_panel(), def->get_page());
          continue;
        }
      auto panel = std::dynamic_pointer_cast<ui::prefwidgets::PanelDef>(def);
      if (panel == nullptr)
        {
          spdlog::error("Cannot create preferences panel '{}' from a widget definition", locationId);
          continue;
        }
      QVariantMap entry;
      auto section = host != sections.end() ? host->second : "plugin";
      entry.insert("id", QString::fromStdString(locationId));
      entry.insert("section", QString::fromStdString(section));
      pluginSections[locationId] = section;
      auto panelCount = std::count_if(panelDefinitions.begin(), panelDefinitions.end(), [&](const auto &item) {
        return !item.second.empty() && item.second.front()->get_page() == def->get_page();
      });
      entry.insert("title",
                   QString::fromStdString(section == "plugin" && panelCount == 1 ? page->second.first : panel->get_label()));
      pluginNavEntries.append(entry);
    }
  return pluginNavEntries;
}

QmlPrefsDialog::~QmlPrefsDialog()
{
  delete view;
}

void
QmlPrefsDialog::show()
{
  if (view != nullptr)
    {
      view->show();
      view->raise();
      view->requestActivate();
    }
}

void
QmlPrefsDialog::hide()
{
  if (view != nullptr)
    {
      view->hide();
    }
}

void
QmlPrefsDialog::retranslate()
{
  if (view != nullptr)
    {
      view->engine()->retranslate();
    }
}

void
QmlPrefsDialog::create_plugin_bridges()
{
  auto registry = app->get_internal_preferences_registry();
  auto append = [this](auto &bridges, const auto &key, const auto &def) {
    auto it = bridges.find(key);
    if (it == bridges.end())
      {
        bridges[key] = QmlPluginPageBuilder::build(def, this);
      }
    else
      {
        QmlPluginPageBuilder::append(def, it->second, this);
      }
  };
  for (auto &[id, deflist] : registry->get_widgets())
    {
      for (auto &def : deflist)
        {
          append(pluginBridges, def->get_page() + "." + def->get_panel(), def);
        }
    }
  for (const auto &[id, extensions]: registry->get_panel_extensions())
    {
      for (const auto &extension: extensions)
        {
          const auto &def = extension.def;
          auto key = extension.page + "." + extension.panel;
          auto &bridges = extension.placement == IPreferencesRegistryInternal::Placement::Top ? topExtensionBridges
                                                                                              : bottomExtensionBridges;
          append(bridges, key, def);
        }
    }
}

namespace
{
  std::string registry_location(const QString &section, const QString &page)
  {
    // Use the registry's existing GTK page/panel IDs in both toolkits.
    QString registryPage = section == "timers" ? "timer" : section;
    QString registryPanel = page;
    if (section == "timers" && page == "daily")
      {
        registryPanel = "dailylimit";
      }
    else if (section == "timers" && page == "monitoring")
      {
        registryPage = "monitoring";
      }
    else if (section == "ui" && page == "status")
      {
        registryPanel = "mainwindow";
      }
    else if (section == "ui" && page == "sounds")
      {
        registryPage = "sounds";
      }
    return registryPage.toStdString() + "." + registryPanel.toStdString();
  }
} // namespace

void
QmlPrefsDialog::onNavigateTo(const QString &section, const QString &page)
{
  QQuickItem *root = view->rootObject();
  if (root == nullptr)
    {
      return;
    }

  if (section == "plugin" || pluginSections.contains(page.toStdString()))
    {
      auto it = pluginBridges.find(page.toStdString());
      if (it == pluginBridges.end())
        {
          spdlog::error("Cannot select unknown preferences panel '{}'", page.toStdString());
        }
      activeBridge->loadPanel(it != pluginBridges.end() ? it->second : nullptr);
      auto bottom = bottomExtensionBridges.find(page.toStdString());
      extensionBridge->loadPanel(bottom != bottomExtensionBridges.end() ? bottom->second : nullptr);
      auto top = topExtensionBridges.find(page.toStdString());
      topExtensionBridge->loadPanel(top != topExtensionBridges.end() ? top->second : nullptr);
    }
  else
    {
      auto location = registry_location(section, page);
      auto top = topExtensionBridges.find(location);
      topExtensionBridge->loadPanel(top != topExtensionBridges.end() ? top->second : nullptr);
      auto bottom = bottomExtensionBridges.find(location);
      auto core = pluginBridges.find(location);
      extensionBridge->loadPanels({
        .main = core != pluginBridges.end() ? core->second : nullptr,
        .additional = bottom != bottomExtensionBridges.end() ? bottom->second : nullptr,
      });
    }

  root->setProperty("currentSection", section);
  root->setProperty("currentPage", page);
}

void
QmlPrefsDialog::onCloseRequested()
{
  hide();
}

void
QmlPrefsDialog::onQmlStatusChanged(int status)
{
  // handled inline via lambda in constructor
  (void)status;
}

void
QmlPrefsDialog::select_page(const std::string &section, const std::string &page)
{
  auto location = section + "." + page;
  if (auto it = pluginSections.find(location); it != pluginSections.end())
    {
      onNavigateTo(QString::fromStdString(it->second), QString::fromStdString(location));
    }
  else
    {
      onNavigateTo(QString::fromStdString(section), QString::fromStdString(page));
    }
}
