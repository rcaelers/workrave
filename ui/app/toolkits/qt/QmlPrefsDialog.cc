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

#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QLibraryInfo>
#include <QDir>
#include <QScreen>
#include <QGuiApplication>
#include <QCoreApplication>

#include "QmlPrefsBridges.hh"
#include "QmlPluginPrefBridges.hh"
#include "debug.hh"

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
  monitoringBridge   = new MonitoringPrefBridge(this->app, this);
  soundsBridge       = new SoundsPrefBridge(this->app, this);
  activeBridge    = new ActivePluginPageBridge(this);
  extensionBridge = new ActivePluginPageBridge(this);
  create_plugin_bridges();

  view = new QQuickView();
  // QML import path resolution: macdeployqt copies QML modules to Contents/Resources/qml/ in
  // a deployed .app bundle. For build-tree runs (no bundle), fall back to the compile-time path
  // baked in from Qt's CMake config dir (QLibraryInfo resolves incorrectly for custom framework
  // builds, pointing to lib/qml instead of the real prefix/qml).
#ifdef Q_OS_MACOS
  {
    QDir bundleQml(QCoreApplication::applicationDirPath() + "/../Resources/qml");
    if (bundleQml.exists())
      {
        view->engine()->addImportPath(bundleQml.canonicalPath());
      }
  }
#endif
#ifdef QT_QML_IMPORT_PATH
  view->engine()->addImportPath(QStringLiteral(QT_QML_IMPORT_PATH));
#endif
  view->engine()->addImportPath(QLibraryInfo::path(QLibraryInfo::QmlImportsPath));
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
  ctx->setContextProperty("monitoringPrefBridge",   monitoringBridge);
  ctx->setContextProperty("soundsPrefBridge",       soundsBridge);
  ctx->setContextProperty("activePluginBridge",  activeBridge);
  ctx->setContextProperty("pageExtensionBridge", extensionBridge);

  // Build nav entries for plugin pages
  QVariantList pluginNavEntries;
  auto registry = this->app->get_internal_preferences_registry();
  for (auto &[id, info] : registry->get_pages())
    {
      auto &[label, image] = info;
      QVariantMap entry;
      entry["id"]    = QString::fromStdString(id);
      entry["title"] = QString::fromStdString(label);
      pluginNavEntries.append(entry);
    }
  ctx->setContextProperty("prefPluginNavEntries", pluginNavEntries);

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
          }
      }
  });

  view->setSource(QUrl("qrc:/sanctuary/PrefsShell.qml"));
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
  for (auto &[id, deflist] : registry->get_widgets())
    {
      for (auto &def : deflist)
        {
          auto pageid = def->get_page();
          auto *page  = QmlPluginPageBuilder::build(def, this);
          pluginBridges[pageid] = page;
        }
    }
}

void
QmlPrefsDialog::onNavigateTo(const QString &section, const QString &page)
{
  QQuickItem *root = view->rootObject();
  if (root == nullptr)
    {
      return;
    }

  if (section == "plugin")
    {
      auto it = pluginBridges.find(page.toStdString());
      if (it != pluginBridges.end())
        {
          activeBridge->loadPanel(it->second);
        }
      extensionBridge->loadPanel(nullptr);
    }
  else
    {
      auto it = pluginBridges.find(page.toStdString());
      extensionBridge->loadPanel(it != pluginBridges.end() ? it->second : nullptr);
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
