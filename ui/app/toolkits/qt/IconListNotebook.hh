// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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

#ifndef ICONLISTNOTEBOOK_HH
#define ICONLISTNOTEBOOK_HH

#include <memory>

#include <QWidget>
#include <QButtonGroup>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

class IconListNotebook : public QWidget
{
  Q_OBJECT

public:
  explicit IconListNotebook(QWidget *parent = nullptr);

  QSize sizeHint() const override;

  void add_page(QWidget *page, const QIcon &icon, const QString &title);

private:
  void on_button_selected(QAbstractButton *button);

private:
  std::unique_ptr<QButtonGroup> button_group;
  QHBoxLayout *layout{nullptr};
  QVBoxLayout *button_layout{nullptr};
  QStackedWidget *content{nullptr};
};

#endif
