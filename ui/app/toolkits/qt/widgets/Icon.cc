// Copyright (C) 2023 Rob Caelers <robc@krandor.nl>
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

#include "Icon.hh"

#include <QApplication>
#include <QByteArray>
#include <QDomElement>
#include <QFile>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPixmapCache>
#include <QStyleHints>
#include <QSvgRenderer>

class IconEngine : public QIconEngine
{
public:
  IconEngine() = default;
  ~IconEngine() override = default;

  explicit IconEngine(const std::string &name)
    : icon_filename(QString::fromStdString(name))
  {
  }

  IconEngine(const IconEngine &other)
    : icon_filename(other.icon_filename)
  {
    monitor_color_scheme();
  }

  IconEngine(IconEngine &&) = delete;
  IconEngine &operator=(const IconEngine &) = delete;
  IconEngine &operator=(IconEngine &&) = delete;

  QIconEngine *clone() const override
  {
    return new IconEngine(*this);
  }

  QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override
  {
    return QIconEngine::actualSize(size, mode, state);
  }

  void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override
  {
    QColor color = get_foreground_color(mode, state);
    QString key = QString("icon:%1:%2x%3:%4").arg(icon_filename).arg(rect.width()).arg(rect.height()).arg(color.name());
    QPixmap pix;
    if (!QPixmapCache::find(key, &pix))
      {
        pix = load_icon_with_color(icon_filename, color, rect.size());
        QPixmapCache::insert(key, pix);
      }
    painter->drawPixmap(rect.topLeft(), pix);
  }

  QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
  {
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    paint(&painter, QRect(QPoint(0, 0), size), mode, state);
    return pix;
  }

private:
  void set_attributes_recursively(QDomElement &elem, QString name, QString attr, QString value)
  {
    if (elem.tagName().compare(name) == 0)
      {
        elem.setAttribute(attr, value);
      }
    for (int i = 0; i < elem.childNodes().count(); i++)
      {
        if (elem.childNodes().at(i).isElement())
          {
            QDomElement element = elem.childNodes().at(i).toElement();
            set_attributes_recursively(element, name, attr, value);
          }
      }
  }

  QPixmap load_icon_with_color(QString iconPath, QColor color, QSize size)
  {
    QFile file(iconPath);
    if (file.open(QIODevice::ReadOnly))
      {
        QByteArray data = file.readAll();

        QDomDocument doc;
        doc.setContent(data);

        QDomElement root = doc.documentElement();
        set_attributes_recursively(root, "path", "fill", color.name());

        QSvgRenderer svgRenderer(doc.toByteArray());
        QPixmap pix(size);
        pix.fill(Qt::transparent);
        QPainter pixPainter(&pix);
        svgRenderer.render(&pixPainter);
        return pix;
      }

    return {};
  }

  QColor get_foreground_color(QIcon::Mode mode, QIcon::State state)
  {
    if (mode == QIcon::Disabled)
      {
        return QApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
      }
    if (mode == QIcon::Selected)
      {
        return QApplication::palette().highlightedText().color();
      }
    return QApplication::palette().windowText().color();
  }

  void monitor_color_scheme()
  {
    dark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;

    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, [this](Qt::ColorScheme colorScheme) {
      dark = colorScheme == Qt::ColorScheme::Dark;
    });
  }

private:
  bool dark{false};
  QString icon_filename;
};

Icon::Icon(const std::string &name)
  : QIcon(new IconEngine(name))
{
}
