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

#include <algorithm>

#include "ui/prefwidgets/qtquick/Formatting.hh"
#include <QPointer>
#include "ui/prefwidgets/qtquick/Models.hh"

using namespace ui::prefwidgets::qtquick;

// ── RowModel ─────────────────────────────────────────────────────────────

RowModel::RowModel(const ui::prefwidgets::Widget &def, Kind kind, QObject *parent)
  : QObject(parent)
  , kind_(kind)
  , label_(QString::fromStdString(def.get_label()))
  , presentation_(def.get_presentation())
{
  if (!presentation_.label.empty())
    {
      label_ = QString::fromStdString(presentation_.label);
    }
}

RowModel::RowModel(std::shared_ptr<ui::prefwidgets::Toggle> def, QObject *parent)
  : RowModel(*def, Kind::Toggle, parent)
{
  toggle_ = std::move(def);
  toggle_->init([guard = QPointer<RowModel>(this)](bool) {
    if (guard != nullptr)
      {
        Q_EMIT guard->changed();
      }
  });
}

RowModel::RowModel(std::shared_ptr<ui::prefwidgets::Time> def, QObject *parent)
  : RowModel(*def, Kind::Time, parent)
{
  time_ = std::move(def);
  timeMin_ = time_->get_min();
  timeMax_ = time_->get_max();
  time_->init([guard = QPointer<RowModel>(this)](int) {
    if (guard != nullptr)
      {
        Q_EMIT guard->changed();
      }
  });
}

RowModel::RowModel(std::shared_ptr<ui::prefwidgets::Value> def, QObject *parent)
  : RowModel(*def, Kind::Spin, parent)
{
  value_presentation_ = def->get_value_presentation();
  value_ = std::move(def);
  valueMin_ = value_->get_min();
  valueMax_ = value_->get_max();
  value_->init([guard = QPointer<RowModel>(this)](int) {
    if (guard != nullptr)
      {
        Q_EMIT guard->changed();
      }
  });
}

RowModel::RowModel(std::shared_ptr<ui::prefwidgets::Choice> def, QObject *parent)
  : RowModel(*def, Kind::Choice, parent)
{
  choice_presentation_ = def->get_choice_presentation();
  choice_ = std::move(def);
  choice_->init([guard = QPointer<RowModel>(this)](int) {
    if (guard != nullptr)
      {
        Q_EMIT guard->changed();
      }
  });
}

RowModel::RowModel(std::shared_ptr<ui::prefwidgets::Entry> def, QObject *parent)
  : RowModel(*def, Kind::Entry, parent)
{
  entry_ = std::move(def);
  entry_->init([guard = QPointer<RowModel>(this)](const std::string &) {
    if (guard != nullptr)
      {
        Q_EMIT guard->changed();
      }
  });
}

QString
RowModel::presentation() const
{
  using ChoiceControl = ui::prefwidgets::ChoicePresentation::Control;
  if (choice_ != nullptr && !options().empty())
    {
      switch (choice_presentation_.control)
        {
        case ChoiceControl::Cards:
          return QStringLiteral("cards");
        case ChoiceControl::Segmented:
          return QStringLiteral("segmented");
        case ChoiceControl::Default:
          break;
        }
    }
  if (value_ != nullptr && value_presentation_.control == ui::prefwidgets::ValuePresentation::Control::Duration)
    {
      return QStringLiteral("duration");
    }
  return QStringLiteral("default");
}

QString
RowModel::hint() const
{
  return QString::fromStdString(presentation_.hint);
}

namespace
{
  QVariantMap color_map(const ui::prefwidgets::Presentation::Color &color)
  {
    QVariantMap result;
    result.insert(QStringLiteral("role"), QString::fromStdString(color.role));
    result.insert(QStringLiteral("light"), QString::fromStdString(color.light));
    result.insert(QStringLiteral("dark"), QString::fromStdString(color.dark));
    return result;
  }
} // namespace

QVariantMap
RowModel::accent() const
{
  return color_map(presentation_.accent);
}

QVariantList
RowModel::previews() const
{
  QVariantList result;
  if (choice_presentation_.previews.size() != static_cast<size_t>(options().size()))
    {
      return result;
    }
  for (const auto &preview: choice_presentation_.previews)
    {
      QVariantMap entry;
      entry.insert(QStringLiteral("asset"), QString::fromStdString(preview.asset));
      entry.insert(QStringLiteral("description"), QString::fromStdString(preview.description));
      result.append(entry);
    }
  return result;
}

int
RowModel::previewHeight() const
{
  return std::max(1, choice_presentation_.preview_height);
}

std::pair<int, int>
RowModel::timeRange() const
{
  int minimum = time_ != nullptr ? timeMin_ : valueMin_;
  int maximum = time_ != nullptr ? timeMax_ : valueMax_;
  if (value_presentation_.slider_max > value_presentation_.slider_min)
    {
      int slider_min = std::clamp(value_presentation_.slider_min, minimum, maximum);
      int slider_max = std::clamp(value_presentation_.slider_max, minimum, maximum);
      if (slider_max > slider_min)
        {
          return {slider_min, slider_max};
        }
    }
  return {minimum, maximum};
}

QVariantList
RowModel::timeTicks() const
{
  QVariantList result;
  const auto [minimum, maximum] = timeRange();
  for (int seconds: value_presentation_.ticks)
    {
      if (seconds >= minimum && seconds <= maximum)
        {
          QVariantMap tick;
          tick.insert(QStringLiteral("at"), Formatting::normalize(seconds, {.minimum = minimum, .maximum = maximum}));
          tick.insert(QStringLiteral("label"), tr("%1s").arg(seconds));
          result.append(tick);
        }
    }
  return result;
}

bool
RowModel::enabled() const
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
RowModel::checked() const
{
  return toggle_ ? toggle_->get_value() : false;
}

QString
RowModel::timeDisplay() const
{
  if (time_ != nullptr)
    {
      return Formatting::formatTime(time_->get_value());
    }
  return value_ != nullptr ? Formatting::formatTime(value_->get_value()) : QString();
}

double
RowModel::timeNorm() const
{
  const auto [minimum, maximum] = timeRange();
  int value = minimum;
  if (time_ != nullptr)
    {
      value = time_->get_value();
    }
  else if (value_ != nullptr)
    {
      value = value_->get_value();
    }
  return Formatting::normalize(value, {.minimum = minimum, .maximum = maximum});
}

QString
RowModel::spinDisplay() const
{
  return value_ ? QString::number(value_->get_value()) : QString();
}

QStringList
RowModel::options() const
{
  if (!choice_)
    {
      return {};
    }
  QStringList list;
  auto content = choice_->get_content();
  if (choice_presentation_.options.size() == content.size())
    {
      content = choice_presentation_.options;
    }
  for (const auto &s: content)
    {
      list << QString::fromStdString(s);
    }
  return list;
}

int
RowModel::currentIndex() const
{
  return choice_ ? choice_->get_value() : 0;
}

QString
RowModel::entryText() const
{
  return entry_ ? QString::fromStdString(entry_->get_value()) : QString();
}

void
RowModel::setChecked(bool v)
{
  if (toggle_)
    {
      toggle_->set_value(v);
      Q_EMIT changed();
    }
}

void
RowModel::increment()
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
RowModel::decrement()
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
RowModel::setTimeNorm(double n)
{
  const auto [minimum, maximum] = timeRange();
  setSeconds(Formatting::denormalize(n, {.minimum = minimum, .maximum = maximum, .step = 1}));
}

void
RowModel::setSeconds(int seconds)
{
  if (time_ != nullptr)
    {
      time_->set_value(std::clamp(seconds, timeMin_, timeMax_));
      Q_EMIT changed();
    }
  else if (value_ != nullptr)
    {
      value_->set_value(std::clamp(seconds, valueMin_, valueMax_));
      Q_EMIT changed();
    }
}

void
RowModel::setCurrentIndex(int idx)
{
  if (choice_ != nullptr && idx >= 0 && std::cmp_less(idx, choice_->get_content().size()))
    {
      choice_->set_value(idx);
      Q_EMIT changed();
    }
}

void
RowModel::setEntryText(const QString &v)
{
  if (entry_)
    {
      entry_->set_value(v.toStdString());
      Q_EMIT changed();
    }
}

// ── GroupModel ─────────────────────────────────────────────────────────

GroupModel::GroupModel(QString label, QObject *parent)
  : QObject(parent)
  , label_(std::move(label))
{
}

void
GroupModel::add_row(RowModel *row)
{
  rows_.append(QVariant::fromValue(row));
  Q_EMIT changed();
}

// ── PageModel ──────────────────────────────────────────────────────────

PageModel::PageModel(QString label, QObject *parent)
  : QObject(parent)
  , label_(std::move(label))
{
}

void
PageModel::add_group(GroupModel *group)
{
  groups_.append(QVariant::fromValue(group));
  Q_EMIT changed();
}

// ── ActivePageModel ────────────────────────────────────────────────────

ActivePageModel::ActivePageModel(QObject *parent)
  : QObject(parent)
{
}

void
ActivePageModel::set_page(PageModel *panel)
{
  set_pages({.main = panel});
}

void
ActivePageModel::set_pages(Panels panels)
{
  if (current_ != nullptr)
    {
      QObject::disconnect(current_, nullptr, this, nullptr);
    }
  if (additional_ != nullptr)
    {
      QObject::disconnect(additional_, nullptr, this, nullptr);
    }
  current_ = panels.main;
  additional_ = panels.additional;
  for (auto *page: {panels.main, panels.additional})
    {
      if (page != nullptr)
        {
          connect(page, &PageModel::changed, this, &ActivePageModel::changed);
          connect(page, &QObject::destroyed, this, [this] { Q_EMIT changed(); });
        }
    }
  Q_EMIT changed();
}

QString
ActivePageModel::label() const
{
  return current_ != nullptr ? current_->label() : QString();
}

QVariantList
ActivePageModel::groups() const
{
  auto groups = current_ != nullptr ? current_->groups() : QVariantList{};
  if (additional_ != nullptr)
    {
      groups.append(additional_->groups());
    }
  return groups;
}
