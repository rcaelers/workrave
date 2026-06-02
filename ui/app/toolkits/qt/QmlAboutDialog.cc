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

#include "QmlAboutDialog.hh"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QUrl>
#include <QLibraryInfo>
#include <QDir>
#include <QCoreApplication>
#include <QString>
#include <QStringList>

#include <spdlog/spdlog.h>

#include "commonui/credits.h"
#include "debug.hh"

QmlAboutDialog::QmlAboutDialog(QObject *parent)
  : QObject(parent)
{
  TRACE_ENTRY();

  view = new QQuickView();
  view->setTitle(tr("About Workrave"));
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  view->setMinimumSize(QSize(540, 430));
  view->setMaximumSize(QSize(540, 430));
  view->resize(540, 430);

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

  QString version = QStringLiteral(WORKRAVE_VERSION);
#if defined(WORKRAVE_GIT_VERSION)
  version += QStringLiteral(" (" WORKRAVE_GIT_VERSION ")");
#endif

  QStringList authorList;
  for (const char *const *author = workrave_authors; *author != nullptr; ++author)
    {
      authorList << QString::fromUtf8(*author);
    }

  view->rootContext()->setContextProperty("aboutVersion", version);
  view->rootContext()->setContextProperty("aboutCopyright", QString::fromUtf8(workrave_copyright));
  view->rootContext()->setContextProperty("aboutAuthors", authorList.join('\n'));
  view->rootContext()->setContextProperty("aboutTranslators", QString::fromUtf8(workrave_translators));

  QObject::connect(view, &QQuickView::statusChanged, this, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err: view->errors())
          {
            spdlog::error("AboutDialog QML error: {}", err.toString().toStdString());
          }
      }
    else if (status == QQuickView::Ready)
      {
        QQuickItem *root = view->rootObject();
        if (root != nullptr)
          {
            QObject::connect(root, SIGNAL(closeRequested()), this, SLOT(onCloseRequested()));
          }
      }
  });

  QObject::connect(view, &QWindow::visibilityChanged, this, &QmlAboutDialog::onVisibilityChanged);

  view->setSource(QUrl(QStringLiteral("qrc:/sanctuary/AboutDialog.qml")));
}

QmlAboutDialog::~QmlAboutDialog()
{
  delete view;
}

void
QmlAboutDialog::show()
{
  view->show();
  view->raise();
  view->requestActivate();
}

void
QmlAboutDialog::onCloseRequested()
{
  view->hide();
}

void
QmlAboutDialog::onVisibilityChanged(QWindow::Visibility visibility)
{
  if (visibility == QWindow::Hidden)
    {
      deleteLater();
    }
}
