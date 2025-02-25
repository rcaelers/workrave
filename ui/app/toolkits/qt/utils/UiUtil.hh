// Copyright (C) 2003 - 2013 Raymond Penners & Rob Caelers
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

#ifndef UIUTIL_HH
#define UIUTIL_HH

#include <string>

#include <QtWidgets>

#include "core/ICore.hh"
#include "ui/IApplicationContext.hh"
#include "ui/prefwidgets/qt/Builder.hh"
#include "ui/prefwidgets/qt/BoxWidget.hh"

class UiUtil
{
public:
  Q_DECLARE_TR_FUNCTIONS(UiUtil);

public:
  static auto create_alert_text(const QString &caption, const QString &body) -> QString;
  static auto create_label(const QString &text, bool bold = false) -> QLabel *;
  static auto create_image_label(const QString &filename) -> QLabel *;
  static auto create_label_for_break(workrave::BreakId id) -> QLabel *;
  static auto create_label_with_tooltip(const QString &text, const QString &tooltip) -> QLabel *;
  static auto create_image_button(const QString &filename) -> QPushButton *;
  static auto create_image_text_button(const QString &filename, const QString &text) -> QPushButton *;

  static void clear_layout(QLayout *layout);

  static void add_widget(QBoxLayout *layout, const QString &text, QWidget *widget);
  static void add_widget(QBoxLayout *layout, QLabel *label, QWidget *widget);
  static auto add_label(QBoxLayout *layout, const QString &text, bool bold = false) -> QLabel *;

  static auto create_icon(const QString &filename) -> QIcon;
  static auto create_pixmap(const QString &filename, int height) -> QPixmap;

  static void invalidate(QLayout *layout);

  static auto time_to_string(time_t t, bool display_units = false) -> QString;
};

#endif // UIUTIL_HH
