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

#include "QmlPluginPrefBridges.hh"

#include <algorithm>

#include "QmlPrefsBridges.hh"

// ── PrefRowBridge ─────────────────────────────────────────────────────────────

PrefRowBridge::PrefRowBridge(std::shared_ptr<ui::prefwidgets::Toggle> def, QObject *parent)
  : QObject(parent)
  , kind_(Kind::Toggle)
  , label_(QString::fromStdString(def->get_label()))
  , toggle_(std::move(def))
{
  toggle_->init([this](bool) { Q_EMIT changed(); });
}

PrefRowBridge::PrefRowBridge(std::shared_ptr<ui::prefwidgets::Time> def, QObject *parent)
  : QObject(parent)
  , kind_(Kind::Time)
  , label_(QString::fromStdString(def->get_label()))
  , time_(std::move(def))
  , timeMin_(time_->get_min())
  , timeMax_(time_->get_max())
{
  time_->init([this](int) { Q_EMIT changed(); });
}

PrefRowBridge::PrefRowBridge(std::shared_ptr<ui::prefwidgets::Value> def, QObject *parent)
  : QObject(parent)
  , kind_(Kind::Spin)
  , label_(QString::fromStdString(def->get_label()))
  , value_(std::move(def))
  , valueMin_(value_->get_min())
  , valueMax_(value_->get_max())
{
  value_->init([this](int) { Q_EMIT changed(); });
}

PrefRowBridge::PrefRowBridge(std::shared_ptr<ui::prefwidgets::Choice> def, QObject *parent)
  : QObject(parent)
  , kind_(Kind::Choice)
  , label_(QString::fromStdString(def->get_label()))
  , choice_(std::move(def))
{
  choice_->init([this](int) { Q_EMIT changed(); });
}

PrefRowBridge::PrefRowBridge(std::shared_ptr<ui::prefwidgets::Entry> def, QObject *parent)
  : QObject(parent)
  , kind_(Kind::Entry)
  , label_(QString::fromStdString(def->get_label()))
  , entry_(std::move(def))
{
  entry_->init([this](const std::string &) { Q_EMIT changed(); });
}

bool
PrefRowBridge::enabled() const
{
  switch (kind_)
    {
    case Kind::Toggle:
      return toggle_->get_sensitive();
    case Kind::Time:
      return time_->get_sensitive();
    case Kind::Spin:
      return value_->get_sensitive();
    case Kind::Choice:
      return choice_->get_sensitive();
    case Kind::Entry:
      return entry_->get_sensitive();
    default:
      return true;
    }
}

bool
PrefRowBridge::checked() const
{
  return toggle_ ? toggle_->get_value() : false;
}

QString
PrefRowBridge::timeDisplay() const
{
  return time_ ? PrefUtils::formatTime(time_->get_value()) : QString();
}

double
PrefRowBridge::timeNorm() const
{
  return time_ ? PrefUtils::normalize(time_->get_value(), timeMin_, timeMax_) : 0.0;
}

QString
PrefRowBridge::spinDisplay() const
{
  return value_ ? QString::number(value_->get_value()) : QString();
}

QStringList
PrefRowBridge::options() const
{
  if (!choice_)
    return {};
  QStringList list;
  for (const auto &s : choice_->get_content())
    {
      list << QString::fromStdString(s);
    }
  return list;
}

int
PrefRowBridge::currentIndex() const
{
  return choice_ ? choice_->get_value() : 0;
}

QString
PrefRowBridge::entryText() const
{
  return entry_ ? QString::fromStdString(entry_->get_value()) : QString();
}

void
PrefRowBridge::setChecked(bool v)
{
  if (toggle_)
    {
      toggle_->set_value(v);
      Q_EMIT changed();
    }
}

void
PrefRowBridge::increment()
{
  if (time_)
    {
      int v = std::clamp(time_->get_value() + 1, timeMin_, timeMax_);
      time_->set_value(v);
      Q_EMIT changed();
    }
  else if (value_)
    {
      int v = std::clamp(value_->get_value() + 1, valueMin_, valueMax_);
      value_->set_value(v);
      Q_EMIT changed();
    }
}

void
PrefRowBridge::decrement()
{
  if (time_)
    {
      int v = std::clamp(time_->get_value() - 1, timeMin_, timeMax_);
      time_->set_value(v);
      Q_EMIT changed();
    }
  else if (value_)
    {
      int v = std::clamp(value_->get_value() - 1, valueMin_, valueMax_);
      value_->set_value(v);
      Q_EMIT changed();
    }
}

void
PrefRowBridge::setTimeNorm(double n)
{
  if (time_)
    {
      int v = PrefUtils::denormalize(n, timeMin_, timeMax_, 1);
      time_->set_value(v);
      Q_EMIT changed();
    }
}

void
PrefRowBridge::setCurrentIndex(int idx)
{
  if (choice_)
    {
      choice_->set_value(idx);
      Q_EMIT changed();
    }
}

void
PrefRowBridge::setEntryText(const QString &v)
{
  if (entry_)
    {
      entry_->set_value(v.toStdString());
      Q_EMIT changed();
    }
}

// ── PluginGroupBridge ─────────────────────────────────────────────────────────

PluginGroupBridge::PluginGroupBridge(const QString &label, QObject *parent)
  : QObject(parent)
  , label_(label)
{
}

void
PluginGroupBridge::addRow(PrefRowBridge *row)
{
  rows_.append(QVariant::fromValue(row));
}

// ── PluginPageBridge ──────────────────────────────────────────────────────────

PluginPageBridge::PluginPageBridge(const QString &label, QObject *parent)
  : QObject(parent)
  , label_(label)
{
}

void
PluginPageBridge::addGroup(PluginGroupBridge *group)
{
  groups_.append(QVariant::fromValue(group));
}

// ── ActivePluginPageBridge ────────────────────────────────────────────────────

ActivePluginPageBridge::ActivePluginPageBridge(QObject *parent)
  : QObject(parent)
{
}

void
ActivePluginPageBridge::loadPanel(PluginPageBridge *panel)
{
  current_ = panel;
  Q_EMIT changed();
}

QString
ActivePluginPageBridge::label() const
{
  return current_ ? current_->label() : QString();
}

QVariantList
ActivePluginPageBridge::groups() const
{
  return current_ ? current_->groups() : QVariantList{};
}

// ── QmlPluginPageBuilder ──────────────────────────────────────────────────────

namespace QmlPluginPageBuilder
{
  static void buildLeaves(const std::list<std::shared_ptr<ui::prefwidgets::Widget>> &children,
                          PluginGroupBridge *group,
                          QObject *parent)
  {
    for (auto &child : children)
      {
        PrefRowBridge *row = nullptr;

        if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Toggle>(child); d)
          {
            row = new PrefRowBridge(d, parent);
          }
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Time>(child); d)
          {
            row = new PrefRowBridge(d, parent);
          }
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Value>(child); d)
          {
            row = new PrefRowBridge(d, parent);
          }
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Choice>(child); d)
          {
            row = new PrefRowBridge(d, parent);
          }
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Entry>(child); d)
          {
            row = new PrefRowBridge(d, parent);
          }

        if (row != nullptr)
          {
            group->addRow(row);
          }
      }
  }

  static void buildWidget(std::shared_ptr<ui::prefwidgets::Widget> widget,
                          PluginPageBridge *page,
                          QObject *parent)
  {
    if (auto frame = std::dynamic_pointer_cast<ui::prefwidgets::Frame>(widget); frame)
      {
        auto *group = new PluginGroupBridge(QString::fromStdString(frame->get_label()), parent);
        buildLeaves(frame->get_content(), group, parent);
        page->addGroup(group);
      }
    else if (auto box = std::dynamic_pointer_cast<ui::prefwidgets::Box>(widget); box)
      {
        for (auto &child : box->get_content())
          {
            buildWidget(child, page, parent);
          }
      }
    else
      {
        // bare leaf at root level — put in an unlabelled group
        auto *group = new PluginGroupBridge(QString{}, parent);
        PrefRowBridge *row = nullptr;

        if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Toggle>(widget); d)
          row = new PrefRowBridge(d, parent);
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Time>(widget); d)
          row = new PrefRowBridge(d, parent);
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Value>(widget); d)
          row = new PrefRowBridge(d, parent);
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Choice>(widget); d)
          row = new PrefRowBridge(d, parent);
        else if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Entry>(widget); d)
          row = new PrefRowBridge(d, parent);

        if (row != nullptr)
          {
            group->addRow(row);
            page->addGroup(group);
          }
      }
  }

  PluginPageBridge *build(std::shared_ptr<ui::prefwidgets::Def> def, QObject *parent)
  {
    QString pageLabel;
    if (auto pd = std::dynamic_pointer_cast<ui::prefwidgets::PanelDef>(def); pd)
      {
        pageLabel = QString::fromStdString(pd->get_label());
      }

    auto *page = new PluginPageBridge(pageLabel, parent);

    auto root = def->get_widget();
    if (root)
      {
        buildWidget(root, page, parent);
      }

    return page;
  }
} // namespace QmlPluginPageBuilder
