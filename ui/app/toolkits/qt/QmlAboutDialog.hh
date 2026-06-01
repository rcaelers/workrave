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

#ifndef QML_ABOUT_DIALOG_HH
#define QML_ABOUT_DIALOG_HH

#include <QObject>
#include <QQuickView>
#include <QWindow>

class QmlAboutDialog : public QObject
{
  Q_OBJECT

public:
  explicit QmlAboutDialog(QObject *parent = nullptr);
  ~QmlAboutDialog() override;

  void show();

private slots:
  void onCloseRequested();
  void onVisibilityChanged(QWindow::Visibility visibility);

private:
  QQuickView *view{nullptr};
};

#endif // QML_ABOUT_DIALOG_HH
