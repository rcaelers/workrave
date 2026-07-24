// Copyright (C) 2025 Rob Caelers <rob.caelers@gmail.com>
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

#include "QmlAutoUpdateDialog.hh"

#include <cmath>

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "debug.hh"

// ── AutoUpdateBridge ──────────────────────────────────────────────────────────

AutoUpdateBridge::AutoUpdateBridge(QObject *parent)
  : QObject(parent)
{
}

void
AutoUpdateBridge::setInfoTitle(const QString &v)
{
  info_title_ = v;
}

void
AutoUpdateBridge::setInfoText(const QString &v)
{
  info_text_ = v;
}

void
AutoUpdateBridge::setReleaseNotes(const QString &v)
{
  release_notes_ = v;
}

void
AutoUpdateBridge::setStatusText(const QString &v)
{
  if (status_text_ != v)
    {
      status_text_ = v;
      Q_EMIT statusTextChanged();
    }
}

void
AutoUpdateBridge::setProgressVisible(bool v)
{
  if (progress_visible_ != v)
    {
      progress_visible_ = v;
      Q_EMIT progressVisibleChanged();
    }
}

void
AutoUpdateBridge::setProgressValue(int v)
{
  if (progress_value_ != v)
    {
      progress_value_ = v;
      Q_EMIT progressValueChanged();
    }
}

void
AutoUpdateBridge::setShowInstallButtons(bool v)
{
  if (show_install_buttons_ != v)
    {
      show_install_buttons_ = v;
      Q_EMIT showInstallButtonsChanged();
    }
}

void
AutoUpdateBridge::setShowClose(bool v)
{
  if (show_close_ != v)
    {
      show_close_ = v;
      Q_EMIT showCloseChanged();
    }
}

void
AutoUpdateBridge::setInstallEnabled(bool v)
{
  if (install_enabled_ != v)
    {
      install_enabled_ = v;
      Q_EMIT installEnabledChanged();
    }
}

void
AutoUpdateBridge::skip()
{
  if (action_callback_)
    {
      action_callback_(0);
    }
}

void
AutoUpdateBridge::later()
{
  if (action_callback_)
    {
      action_callback_(1);
    }
}

void
AutoUpdateBridge::install()
{
  if (action_callback_)
    {
      action_callback_(2);
    }
}

// ── QmlAutoUpdateDialog ───────────────────────────────────────────────────────

QmlAutoUpdateDialog::QmlAutoUpdateDialog(std::shared_ptr<unfold::UpdateInfo> info, update_choice_callback_t callback)
  : callback_(std::move(callback))
{
  TRACE_ENTRY();

  bridge_ = new AutoUpdateBridge;

  auto title_str = QString::fromStdString(info->title);
  auto new_ver   = QString::fromStdString(info->version);
  auto cur_ver   = QString::fromStdString(info->current_version);

  bridge_->setInfoTitle(QObject::tr("A new version of %1 is available").arg(title_str));
  bridge_->setInfoText(QObject::tr("%1 %2 is now available — you have %3. Would you like to download it now?")
                         .arg(title_str)
                         .arg(new_ver)
                         .arg(cur_ver));

  QString notes;
  for (const auto &note: info->release_notes)
    {
      notes += QStringLiteral("## Version ") + QString::fromStdString(note.version) + QStringLiteral("\n\n");
      notes += QString::fromStdString(note.markdown) + QStringLiteral("\n\n");
    }
  bridge_->setReleaseNotes(notes);

  bridge_->setActionCallback([this](int action) {
    switch (action)
      {
      case 0:
        callback_(UpdateChoice::Skip);
        break;
      case 1:
        callback_(UpdateChoice::Later);
        break;
      case 2:
        callback_(UpdateChoice::Now);
        break;
      default:
        break;
      }
  });

  view_ = new QQuickView;
  view_->setTitle(QObject::tr("Software Update"));
  view_->setResizeMode(QQuickView::SizeRootObjectToView);
  view_->setMinimumSize(QSize(760, 520));
  view_->resize(760, 520);

#ifdef Q_OS_MACOS
  {
    QDir bundleQml(QCoreApplication::applicationDirPath() + "/../Resources/qml");
    if (bundleQml.exists())
      {
        view_->engine()->addImportPath(bundleQml.canonicalPath());
      }
  }
#endif
#ifdef QT_QML_IMPORT_PATH
  view_->engine()->addImportPath(QStringLiteral(QT_QML_IMPORT_PATH));
#endif
  view_->engine()->addImportPath(QLibraryInfo::path(QLibraryInfo::QmlImportsPath));

  view_->rootContext()->setContextProperty("updateBridge", bridge_);

  QObject::connect(view_, &QQuickView::statusChanged, bridge_, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err : view_->errors())
          {
            spdlog::error("AutoUpdateDialog QML error: {}", err.toString().toStdString());
          }
      }
    else if (status == QQuickView::Ready)
      {
        QQuickItem *root = view_->rootObject();
        if (root != nullptr)
          {
            QObject::connect(root, SIGNAL(closeRequested()), view_, SLOT(hide()));
          }
      }
  });

  view_->setSource(QUrl(QStringLiteral("qrc:/sanctuary/AutoUpdateDialog.qml")));
}

QmlAutoUpdateDialog::~QmlAutoUpdateDialog()
{
  delete view_;
  delete bridge_;
}

void
QmlAutoUpdateDialog::show()
{
  view_->show();
}

void
QmlAutoUpdateDialog::raise()
{
  view_->raise();
}

void
QmlAutoUpdateDialog::close()
{
  view_->hide();
}

void
QmlAutoUpdateDialog::set_progress_visible(bool visible)
{
  bridge_->setProgressVisible(visible);
}

void
QmlAutoUpdateDialog::set_stage(unfold::UpdateStage stage, double progress)
{
  if (!current_stage_ || *current_stage_ != stage)
    {
      spdlog::info("Update stage: {}", static_cast<int>(stage));
      current_stage_ = stage;

      bridge_->setProgressVisible(stage == unfold::UpdateStage::DownloadInstaller);

      switch (stage)
        {
        case unfold::UpdateStage::DownloadInstaller:
          bridge_->setStatusText(QObject::tr("Downloading installer"));
          break;
        case unfold::UpdateStage::VerifyInstaller:
          bridge_->setStatusText(QObject::tr("Verifying installer"));
          break;
        case unfold::UpdateStage::RunInstaller:
          bridge_->setStatusText(QObject::tr("Running installer"));
          break;
        default:
          break;
        }
    }

  if (std::fabs(progress - bridge_->progressValue() / 100.0) >= 0.01)
    {
      bridge_->setProgressValue(static_cast<int>(progress * 100));
    }

  if (progress >= 1.0)
    {
      bridge_->setProgressVisible(false);
      bridge_->setShowClose(true);
    }
}

void
QmlAutoUpdateDialog::set_status(const std::string &status)
{
  spdlog::info("Update status: {}", status);
  bridge_->setStatusText(QString::fromStdString(status));
  bridge_->setProgressVisible(false);
  bridge_->setShowClose(true);
}

void
QmlAutoUpdateDialog::start_install()
{
  bridge_->setStatusText(QString{});
  bridge_->setShowInstallButtons(false);
  bridge_->setProgressVisible(true);
  bridge_->setInstallEnabled(false);
}
