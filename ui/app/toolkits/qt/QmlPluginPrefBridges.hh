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

#ifndef QMLPLUGINPREFBRIDGES_HH
#define QMLPLUGINPREFBRIDGES_HH

#include <memory>
#include <map>
#include <string>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "ui/prefwidgets/Toggle.hh"
#include "ui/prefwidgets/Time.hh"
#include "ui/prefwidgets/Value.hh"
#include "ui/prefwidgets/Choice.hh"
#include "ui/prefwidgets/Entry.hh"
#include "ui/prefwidgets/Frame.hh"
#include "ui/prefwidgets/Box.hh"
#include "ui/prefwidgets/Def.hh"

// ── PrefRowBridge ─────────────────────────────────────────────────────────────
// Wraps one leaf prefwidget.  QML checks `kind` then reads the relevant props.
// kind: 0=unknown, 1=toggle, 2=time, 3=spin, 4=choice, 5=entry

class PrefRowBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(int        kind         READ kind         CONSTANT)
  Q_PROPERTY(QString    label        READ label        CONSTANT)
  Q_PROPERTY(bool       enabled      READ enabled      NOTIFY changed)

  // kind == 1 (Toggle)
  Q_PROPERTY(bool       checked      READ checked      NOTIFY changed)

  // kind == 2 (Time)
  Q_PROPERTY(QString    timeDisplay  READ timeDisplay  NOTIFY changed)
  Q_PROPERTY(double     timeNorm     READ timeNorm     NOTIFY changed)

  // kind == 3 (Spin / Value)
  Q_PROPERTY(QString    spinDisplay  READ spinDisplay  NOTIFY changed)

  // kind == 4 (Choice)
  Q_PROPERTY(QStringList options      READ options      CONSTANT)
  Q_PROPERTY(int         currentIndex READ currentIndex NOTIFY changed)

  // kind == 5 (Entry)
  Q_PROPERTY(QString    entryText    READ entryText    NOTIFY changed)

public:
  enum Kind
  {
    Unknown = 0,
    Toggle  = 1,
    Time    = 2,
    Spin    = 3,
    Choice  = 4,
    Entry   = 5,
  };

  explicit PrefRowBridge(std::shared_ptr<ui::prefwidgets::Toggle> def, QObject *parent = nullptr);
  explicit PrefRowBridge(std::shared_ptr<ui::prefwidgets::Time>   def, QObject *parent = nullptr);
  explicit PrefRowBridge(std::shared_ptr<ui::prefwidgets::Value>  def, QObject *parent = nullptr);
  explicit PrefRowBridge(std::shared_ptr<ui::prefwidgets::Choice> def, QObject *parent = nullptr);
  explicit PrefRowBridge(std::shared_ptr<ui::prefwidgets::Entry>  def, QObject *parent = nullptr);

  int         kind()         const { return kind_; }
  QString     label()        const { return label_; }
  bool        enabled()      const;
  bool        checked()      const;
  QString     timeDisplay()  const;
  double      timeNorm()     const;
  QString     spinDisplay()  const;
  QStringList options()      const;
  int         currentIndex() const;
  QString     entryText()    const;

  Q_INVOKABLE void setChecked(bool v);
  Q_INVOKABLE void increment();
  Q_INVOKABLE void decrement();
  Q_INVOKABLE void setTimeNorm(double n);
  Q_INVOKABLE void setCurrentIndex(int idx);
  Q_INVOKABLE void setEntryText(const QString &v);

Q_SIGNALS:
  void changed();

private:
  int     kind_;
  QString label_;

  std::shared_ptr<ui::prefwidgets::Toggle> toggle_;
  std::shared_ptr<ui::prefwidgets::Time>   time_;
  std::shared_ptr<ui::prefwidgets::Value>  value_;
  std::shared_ptr<ui::prefwidgets::Choice> choice_;
  std::shared_ptr<ui::prefwidgets::Entry>  entry_;

  int timeMin_{0};
  int timeMax_{3600};
  int valueMin_{0};
  int valueMax_{100};
};

// ── PluginGroupBridge ─────────────────────────────────────────────────────────
// Wraps a Frame or Box (one visual group = one PrefGroup in QML).

class PluginGroupBridge : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString     label READ label CONSTANT)
  Q_PROPERTY(QVariantList rows READ rows  CONSTANT)

public:
  explicit PluginGroupBridge(const QString &label, QObject *parent = nullptr);

  QString      label() const { return label_; }
  QVariantList rows()  const { return rows_; }

  void addRow(PrefRowBridge *row);

private:
  QString      label_;
  QVariantList rows_;
};

// ── PluginPageBridge ──────────────────────────────────────────────────────────
// Wraps a full PanelDef.  Built once at dialog startup; not exposed to QML
// directly — the ActivePluginPageBridge delegates to whichever page is current.

class PluginPageBridge : public QObject
{
  Q_OBJECT

public:
  explicit PluginPageBridge(const QString &label, QObject *parent = nullptr);

  QString      label()  const { return label_; }
  QVariantList groups() const { return groups_; }

  void addGroup(PluginGroupBridge *group);

private:
  QString      label_;
  QVariantList groups_;
};

// ── ActivePluginPageBridge ────────────────────────────────────────────────────
// Single always-registered context property "activePluginBridge".
// C++ calls loadPanel() when the user navigates to a plugin page.
// QML binds to label/groups; both re-read on changed().

class ActivePluginPageBridge : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString     label  READ label  NOTIFY changed)
  Q_PROPERTY(QVariantList groups READ groups NOTIFY changed)

public:
  explicit ActivePluginPageBridge(QObject *parent = nullptr);

  void loadPanel(PluginPageBridge *panel);

  QString      label()  const;
  QVariantList groups() const;

Q_SIGNALS:
  void changed();

private:
  PluginPageBridge *current_{nullptr};
};

// ── QmlPluginPageBuilder ──────────────────────────────────────────────────────
// Walks a prefwidgets Def tree and builds the bridge hierarchy.

namespace QmlPluginPageBuilder
{
  PluginPageBridge *build(std::shared_ptr<ui::prefwidgets::Def> def, QObject *parent);
}

#endif // QMLPLUGINPREFBRIDGES_HH
