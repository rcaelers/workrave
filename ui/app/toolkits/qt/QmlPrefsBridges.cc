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

#include "QmlPrefsBridges.hh"

#include <algorithm>
#include <filesystem>

#include <QFileDialog>
#include <QFileInfo>
#include <QLocale>

#include "core/CoreConfig.hh"
#include "core/CoreTypes.hh"
#include "session/System.hh"
#include "ui/GUIConfig.hh"
#include "ui/IToolkit.hh"
#include "ui/SoundTheme.hh"
#include "utils/AssetPath.hh"
#include "utils/Platform.hh"
#include "utils/Ui.hh"
#include "qformat.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
namespace
{
  constexpr auto RUNKEY = R"(Software\Microsoft\Windows\CurrentVersion\Run)";
}
#endif

using namespace workrave;

// ── Utilities ─────────────────────────────────────────────────────────────────

namespace PrefUtils
{
  QString
  formatTime(int seconds)
  {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    if (h > 0)
      {
        return QStringLiteral("%1:%2:%3").arg(h).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));
      }
      return QStringLiteral("%1:%2").arg(m).arg(s, 2, 10, QLatin1Char('0'));
  }

  double
  normalize(int value, int minVal, int maxVal)
  {
    if (maxVal <= minVal)
      {
        return 0.0;
      }
    return std::clamp(static_cast<double>(value - minVal) / (maxVal - minVal), 0.0, 1.0);
  }

  int
  denormalize(double norm, int minVal, int maxVal, int step)
  {
    double raw = minVal + norm * (maxVal - minVal);
    int snapped = static_cast<int>(std::round(raw / step)) * step;
    return std::clamp(snapped, minVal, maxVal);
  }

  int
  clampStep(int value, int delta, int minVal, int maxVal, int step)
  {
    return std::clamp(value + delta * step, minVal, maxVal);
  }
} // namespace PrefUtils

// ── MicrobreakPrefBridge ───────────────────────────────────────────────────────

MicrobreakPrefBridge::MicrobreakPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
}

bool
MicrobreakPrefBridge::enabled() const
{
  return CoreConfig::break_enabled(BREAK_ID_MICRO_BREAK)();
}

void
MicrobreakPrefBridge::setEnabled(bool v)
{
  CoreConfig::break_enabled(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT enabledChanged();
}

QString
MicrobreakPrefBridge::limitDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK)());
}

double
MicrobreakPrefBridge::limitNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK)(), LIMIT_MIN, LIMIT_MAX);
}

QString
MicrobreakPrefBridge::durationDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK)());
}

double
MicrobreakPrefBridge::durationNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK)(), DUR_MIN, DUR_MAX);
}

QString
MicrobreakPrefBridge::snoozeDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK)());
}

double
MicrobreakPrefBridge::snoozeNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK)(), SNOOZE_MIN, SNOOZE_MAX);
}

bool
MicrobreakPrefBridge::showPostpone() const
{
  return GUIConfig::break_ignorable(BREAK_ID_MICRO_BREAK)();
}

void
MicrobreakPrefBridge::setShowPostpone(bool v)
{
  GUIConfig::break_ignorable(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT optionsChanged();
}

bool
MicrobreakPrefBridge::showSkip() const
{
  return GUIConfig::break_skippable(BREAK_ID_MICRO_BREAK)();
}

void
MicrobreakPrefBridge::setShowSkip(bool v)
{
  GUIConfig::break_skippable(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT optionsChanged();
}

bool
MicrobreakPrefBridge::preludeEnabled() const
{
  return CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK)() != 0;
}

void
MicrobreakPrefBridge::setPreludeEnabled(bool v)
{
  if (!v)
    {
      CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK).set(0);
    }
  else if (CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK)() == 0)
    {
      CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK).set(-1);
    }
  Q_EMIT optionsChanged();
}

bool
MicrobreakPrefBridge::hasMaxPreludes() const
{
  return CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK)() > 0;
}

void
MicrobreakPrefBridge::setHasMaxPreludes(bool v)
{
  if (v)
    {
      if (CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK)() <= 0)
        {
          CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK).set(3);
        }
    }
  else
    {
      CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK).set(-1);
    }
  Q_EMIT optionsChanged();
}

int
MicrobreakPrefBridge::maxPreludes() const
{
  int v = CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK)();
  return (v > 0) ? v : 3;
}

void
MicrobreakPrefBridge::incrementLimit()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK)(), +1, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::decrementLimit()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK)(), -1, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::setLimitNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::setLimitSeconds(int seconds)
{
  CoreConfig::timer_limit(BREAK_ID_MICRO_BREAK).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::incrementDuration()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK)(), +1, DUR_MIN, DUR_MAX, DUR_STEP);
  CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::decrementDuration()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK)(), -1, DUR_MIN, DUR_MAX, DUR_STEP);
  CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::setDurationNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, DUR_MIN, DUR_MAX, DUR_STEP);
  CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::setDurationSeconds(int seconds)
{
  CoreConfig::timer_auto_reset(BREAK_ID_MICRO_BREAK).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::incrementSnooze()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK)(), +1, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::decrementSnooze()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK)(), -1, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::setSnoozeNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::setSnoozeSeconds(int seconds)
{
  CoreConfig::timer_snooze(BREAK_ID_MICRO_BREAK).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
MicrobreakPrefBridge::incrementMaxPreludes()
{
  int cur = CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK)();
  cur = std::max(cur, 1);
  CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK).set(std::min(cur + 1, 10));
  Q_EMIT optionsChanged();
}

void
MicrobreakPrefBridge::decrementMaxPreludes()
{
  int cur = CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK)();
  if (cur > 1)
    {
      CoreConfig::break_max_preludes(BREAK_ID_MICRO_BREAK).set(cur - 1);
      Q_EMIT optionsChanged();
    }
}

// ── RestBreakPrefBridge ────────────────────────────────────────────────────────

RestBreakPrefBridge::RestBreakPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
}

bool
RestBreakPrefBridge::enabled() const
{
  return CoreConfig::break_enabled(BREAK_ID_REST_BREAK)();
}

void
RestBreakPrefBridge::setEnabled(bool v)
{
  CoreConfig::break_enabled(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT enabledChanged();
}

QString
RestBreakPrefBridge::limitDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_limit(BREAK_ID_REST_BREAK)());
}

double
RestBreakPrefBridge::limitNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_limit(BREAK_ID_REST_BREAK)(), LIMIT_MIN, LIMIT_MAX);
}

QString
RestBreakPrefBridge::durationDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK)());
}

double
RestBreakPrefBridge::durationNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK)(), DUR_MIN, DUR_MAX);
}

QString
RestBreakPrefBridge::snoozeDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_snooze(BREAK_ID_REST_BREAK)());
}

double
RestBreakPrefBridge::snoozeNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_snooze(BREAK_ID_REST_BREAK)(), SNOOZE_MIN, SNOOZE_MAX);
}

int
RestBreakPrefBridge::exercises() const
{
  return GUIConfig::break_exercises(BREAK_ID_REST_BREAK)();
}

bool
RestBreakPrefBridge::autoNatural() const
{
  return GUIConfig::break_auto_natural(BREAK_ID_REST_BREAK)();
}

void
RestBreakPrefBridge::setAutoNatural(bool v)
{
  GUIConfig::break_auto_natural(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT optionsChanged();
}

bool
RestBreakPrefBridge::enableShutdown() const
{
  return GUIConfig::break_enable_shutdown(BREAK_ID_REST_BREAK)();
}

void
RestBreakPrefBridge::setEnableShutdown(bool v)
{
  GUIConfig::break_enable_shutdown(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT optionsChanged();
}

bool
RestBreakPrefBridge::showPostpone() const
{
  return GUIConfig::break_ignorable(BREAK_ID_REST_BREAK)();
}

void
RestBreakPrefBridge::setShowPostpone(bool v)
{
  GUIConfig::break_ignorable(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT optionsChanged();
}

bool
RestBreakPrefBridge::showSkip() const
{
  return GUIConfig::break_skippable(BREAK_ID_REST_BREAK)();
}

void
RestBreakPrefBridge::setShowSkip(bool v)
{
  GUIConfig::break_skippable(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT optionsChanged();
}

bool
RestBreakPrefBridge::preludeEnabled() const
{
  return CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK)() != 0;
}

void
RestBreakPrefBridge::setPreludeEnabled(bool v)
{
  if (!v)
    {
      CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK).set(0);
    }
  else if (CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK)() == 0)
    {
      CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK).set(-1);
    }
  Q_EMIT optionsChanged();
}

void
RestBreakPrefBridge::incrementLimit()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_limit(BREAK_ID_REST_BREAK)(), +1, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::decrementLimit()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_limit(BREAK_ID_REST_BREAK)(), -1, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::setLimitNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::setLimitSeconds(int seconds)
{
  CoreConfig::timer_limit(BREAK_ID_REST_BREAK).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::incrementDuration()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK)(), +1, DUR_MIN, DUR_MAX, DUR_STEP);
  CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::decrementDuration()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK)(), -1, DUR_MIN, DUR_MAX, DUR_STEP);
  CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::setDurationNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, DUR_MIN, DUR_MAX, DUR_STEP);
  CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::setDurationSeconds(int seconds)
{
  CoreConfig::timer_auto_reset(BREAK_ID_REST_BREAK).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::incrementSnooze()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_snooze(BREAK_ID_REST_BREAK)(), +1, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::decrementSnooze()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_snooze(BREAK_ID_REST_BREAK)(), -1, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::setSnoozeNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::setSnoozeSeconds(int seconds)
{
  CoreConfig::timer_snooze(BREAK_ID_REST_BREAK).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
RestBreakPrefBridge::incrementExercises()
{
  int v = std::min(GUIConfig::break_exercises(BREAK_ID_REST_BREAK)() + 1, 10);
  GUIConfig::break_exercises(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT optionsChanged();
}

void
RestBreakPrefBridge::decrementExercises()
{
  int v = std::max(GUIConfig::break_exercises(BREAK_ID_REST_BREAK)() - 1, 0);
  GUIConfig::break_exercises(BREAK_ID_REST_BREAK).set(v);
  Q_EMIT optionsChanged();
}

bool
RestBreakPrefBridge::hasMaxPreludes() const
{
  return CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK)() > 0;
}

void
RestBreakPrefBridge::setHasMaxPreludes(bool v)
{
  if (v)
    {
      if (CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK)() <= 0)
        {
          CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK).set(3);
        }
    }
  else
    {
      CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK).set(-1);
    }
  Q_EMIT optionsChanged();
}

int
RestBreakPrefBridge::maxPreludes() const
{
  int v = CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK)();
  return (v > 0) ? v : 3;
}

void
RestBreakPrefBridge::incrementMaxPreludes()
{
  int cur = CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK)();
  cur = std::max(cur, 1);
  CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK).set(std::min(cur + 1, 10));
  Q_EMIT optionsChanged();
}

void
RestBreakPrefBridge::decrementMaxPreludes()
{
  int cur = CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK)();
  if (cur > 1)
    {
      CoreConfig::break_max_preludes(BREAK_ID_REST_BREAK).set(cur - 1);
      Q_EMIT optionsChanged();
    }
}

// ── DailyLimitPrefBridge ───────────────────────────────────────────────────────

DailyLimitPrefBridge::DailyLimitPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
}

bool
DailyLimitPrefBridge::enabled() const
{
  return CoreConfig::break_enabled(BREAK_ID_DAILY_LIMIT)();
}

void
DailyLimitPrefBridge::setEnabled(bool v)
{
  CoreConfig::break_enabled(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT enabledChanged();
}

QString
DailyLimitPrefBridge::limitDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT)());
}

double
DailyLimitPrefBridge::limitNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT)(), LIMIT_MIN, LIMIT_MAX);
}

QString
DailyLimitPrefBridge::snoozeDisplay() const
{
  return PrefUtils::formatTime(CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT)());
}

double
DailyLimitPrefBridge::snoozeNorm() const
{
  return PrefUtils::normalize(CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT)(), SNOOZE_MIN, SNOOZE_MAX);
}

bool
DailyLimitPrefBridge::useMicroBreakActivity() const
{
  return CoreConfig::timer_daily_limit_use_micro_break_activity()();
}

void
DailyLimitPrefBridge::setUseMicroBreakActivity(bool v)
{
  CoreConfig::timer_daily_limit_use_micro_break_activity().set(v);
  Q_EMIT optionsChanged();
}

bool
DailyLimitPrefBridge::enableShutdown() const
{
  return GUIConfig::break_enable_shutdown(BREAK_ID_DAILY_LIMIT)();
}

void
DailyLimitPrefBridge::setEnableShutdown(bool v)
{
  GUIConfig::break_enable_shutdown(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT optionsChanged();
}

bool
DailyLimitPrefBridge::showPostpone() const
{
  return GUIConfig::break_ignorable(BREAK_ID_DAILY_LIMIT)();
}

void
DailyLimitPrefBridge::setShowPostpone(bool v)
{
  GUIConfig::break_ignorable(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT optionsChanged();
}

bool
DailyLimitPrefBridge::showSkip() const
{
  return GUIConfig::break_skippable(BREAK_ID_DAILY_LIMIT)();
}

void
DailyLimitPrefBridge::setShowSkip(bool v)
{
  GUIConfig::break_skippable(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT optionsChanged();
}

bool
DailyLimitPrefBridge::preludeEnabled() const
{
  return CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT)() != 0;
}

void
DailyLimitPrefBridge::setPreludeEnabled(bool v)
{
  if (!v)
    {
      CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT).set(0);
    }
  else if (CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT)() == 0)
    {
      CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT).set(-1);
    }
  Q_EMIT optionsChanged();
}

bool
DailyLimitPrefBridge::hasMaxPreludes() const
{
  return CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT)() > 0;
}

void
DailyLimitPrefBridge::setHasMaxPreludes(bool v)
{
  if (v)
    {
      if (CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT)() <= 0)
        {
          CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT).set(3);
        }
    }
  else
    {
      CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT).set(-1);
    }
  Q_EMIT optionsChanged();
}

int
DailyLimitPrefBridge::maxPreludes() const
{
  int v = CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT)();
  return (v > 0) ? v : 3;
}

void
DailyLimitPrefBridge::incrementLimit()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT)(), +1, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::decrementLimit()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT)(), -1, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::setLimitNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, LIMIT_MIN, LIMIT_MAX, LIMIT_STEP);
  CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::setLimitSeconds(int seconds)
{
  CoreConfig::timer_limit(BREAK_ID_DAILY_LIMIT).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::incrementSnooze()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT)(), +1, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::decrementSnooze()
{
  int v = PrefUtils::clampStep(CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT)(), -1, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::setSnoozeNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, SNOOZE_MIN, SNOOZE_MAX, SNOOZE_STEP);
  CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT).set(v);
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::setSnoozeSeconds(int seconds)
{
  CoreConfig::timer_snooze(BREAK_ID_DAILY_LIMIT).set(std::clamp(seconds, 1, 86400));
  Q_EMIT timingChanged();
}

void
DailyLimitPrefBridge::incrementMaxPreludes()
{
  int cur = CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT)();
  cur = std::max(cur, 1);
  CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT).set(std::min(cur + 1, 10));
  Q_EMIT optionsChanged();
}

void
DailyLimitPrefBridge::decrementMaxPreludes()
{
  int cur = CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT)();
  if (cur > 1)
    {
      CoreConfig::break_max_preludes(BREAK_ID_DAILY_LIMIT).set(cur - 1);
      Q_EMIT optionsChanged();
    }
}

// ── StatusWindowPrefBridge ────────────────────────────────────────────────────

StatusWindowPrefBridge::StatusWindowPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
}

int
StatusWindowPrefBridge::flagsToVisibility(int flags)
{
  if ((flags & GUIConfig::BREAK_HIDE) != 0)
    {
      return 2;
    }
  if ((flags & GUIConfig::BREAK_WHEN_FIRST) != 0)
    {
      return 1;
    }
  return 0;
}

int
StatusWindowPrefBridge::visibilityToFlags(int v)
{
  if (v == 2)
    {
      return GUIConfig::BREAK_HIDE;
    }
  if (v == 1)
    {
      return GUIConfig::BREAK_WHEN_FIRST;
    }
  return 0;
}

bool
StatusWindowPrefBridge::enabled() const
{
  return GUIConfig::timerbox_enabled("main_window")();
}

void
StatusWindowPrefBridge::setEnabled(bool v)
{
  GUIConfig::timerbox_enabled("main_window").set(v);
  Q_EMIT changed();
}

bool
StatusWindowPrefBridge::alwaysOnTop() const
{
  return GUIConfig::main_window_always_on_top()();
}

void
StatusWindowPrefBridge::setAlwaysOnTop(bool v)
{
  GUIConfig::main_window_always_on_top().set(v);
  Q_EMIT changed();
}

int
StatusWindowPrefBridge::displayStyle() const
{
  return static_cast<int>(GUIConfig::display_style()());
}

void
StatusWindowPrefBridge::setDisplayStyle(int v)
{
  GUIConfig::display_style().set(static_cast<DisplayStyle>(v));
  Q_EMIT changed();
}

int
StatusWindowPrefBridge::placement() const
{
  int mp = GUIConfig::timerbox_slot("main_window", BREAK_ID_MICRO_BREAK)();
  int rb = GUIConfig::timerbox_slot("main_window", BREAK_ID_REST_BREAK)();
  int dl = GUIConfig::timerbox_slot("main_window", BREAK_ID_DAILY_LIMIT)();
  if (mp < rb && rb < dl)
    {
      return 0;
    }
  if (mp == rb && rb == dl)
    {
      return 3;
    }
  if (mp == rb)
    {
      return 1;
    }
  return 2;
}

void
StatusWindowPrefBridge::setPlacement(int v)
{
  int pos[3] = {0, 1, 2};
  switch (v)
    {
    case 1: pos[1] = 0; pos[2] = 1; break;
    case 2: pos[2] = 1; break;
    case 3: pos[1] = 0; pos[2] = 0; break;
    default: break;  // 0: keep {0,1,2}
    }
  GUIConfig::timerbox_slot("main_window", BREAK_ID_MICRO_BREAK).set(pos[0]);
  GUIConfig::timerbox_slot("main_window", BREAK_ID_REST_BREAK).set(pos[1]);
  GUIConfig::timerbox_slot("main_window", BREAK_ID_DAILY_LIMIT).set(pos[2]);
  Q_EMIT changed();
}

QString
StatusWindowPrefBridge::cycleDisplay() const
{
  return PrefUtils::formatTime(GUIConfig::timerbox_cycle_time("main_window")());
}

double
StatusWindowPrefBridge::cycleNorm() const
{
  return PrefUtils::normalize(GUIConfig::timerbox_cycle_time("main_window")(), CYCLE_MIN, CYCLE_MAX);
}

void
StatusWindowPrefBridge::incrementCycle()
{
  int v = PrefUtils::clampStep(GUIConfig::timerbox_cycle_time("main_window")(), +1, CYCLE_MIN, CYCLE_MAX, CYCLE_STEP);
  GUIConfig::timerbox_cycle_time("main_window").set(v);
  Q_EMIT changed();
}

void
StatusWindowPrefBridge::decrementCycle()
{
  int v = PrefUtils::clampStep(GUIConfig::timerbox_cycle_time("main_window")(), -1, CYCLE_MIN, CYCLE_MAX, CYCLE_STEP);
  GUIConfig::timerbox_cycle_time("main_window").set(v);
  Q_EMIT changed();
}

void
StatusWindowPrefBridge::setCycleNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, CYCLE_MIN, CYCLE_MAX, CYCLE_STEP);
  GUIConfig::timerbox_cycle_time("main_window").set(v);
  Q_EMIT changed();
}

void
StatusWindowPrefBridge::setCycleSeconds(int seconds)
{
  GUIConfig::timerbox_cycle_time("main_window").set(std::clamp(seconds, 1, 86400));
  Q_EMIT changed();
}

int
StatusWindowPrefBridge::microVisibility() const
{
  return flagsToVisibility(GUIConfig::timerbox_flags("main_window", BREAK_ID_MICRO_BREAK)());
}

void
StatusWindowPrefBridge::setMicroVisibility(int v)
{
  GUIConfig::timerbox_flags("main_window", BREAK_ID_MICRO_BREAK).set(visibilityToFlags(v));
  Q_EMIT changed();
}

int
StatusWindowPrefBridge::restVisibility() const
{
  return flagsToVisibility(GUIConfig::timerbox_flags("main_window", BREAK_ID_REST_BREAK)());
}

void
StatusWindowPrefBridge::setRestVisibility(int v)
{
  GUIConfig::timerbox_flags("main_window", BREAK_ID_REST_BREAK).set(visibilityToFlags(v));
  Q_EMIT changed();
}

int
StatusWindowPrefBridge::dailyVisibility() const
{
  return flagsToVisibility(GUIConfig::timerbox_flags("main_window", BREAK_ID_DAILY_LIMIT)());
}

void
StatusWindowPrefBridge::setDailyVisibility(int v)
{
  GUIConfig::timerbox_flags("main_window", BREAK_ID_DAILY_LIMIT).set(visibilityToFlags(v));
  Q_EMIT changed();
}

// ── MonitoringPrefBridge ───────────────────────────────────────────────────────

MonitoringPrefBridge::MonitoringPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
}

bool
MonitoringPrefBridge::hasAlternateMonitor() const
{
#if defined(PLATFORM_OS_WINDOWS)
  return true;
#else
  return false;
#endif
}

bool
MonitoringPrefBridge::alternateMonitor() const
{
#if defined(PLATFORM_OS_WINDOWS)
  std::string monitor_type;
  app->get_configurator()->get_value_with_default("advanced/monitor", monitor_type, "default");
  return monitor_type != "default";
#else
  return false;
#endif
}

int
MonitoringPrefBridge::sensitivity() const
{
  return CoreConfig::monitor_sensitivity()();
}

void
MonitoringPrefBridge::setAlternateMonitor(bool v)
{
#if defined(PLATFORM_OS_WINDOWS)
  app->get_configurator()->set_value("advanced/monitor", v ? std::string("lowlevel") : std::string("default"));
  Q_EMIT monitoringChanged();
#else
  (void)v;
#endif
}

void
MonitoringPrefBridge::setSensitivity(int v)
{
  CoreConfig::monitor_sensitivity().set(v);
  Q_EMIT monitoringChanged();
}

void
MonitoringPrefBridge::openDebugWindow()
{
  app->get_toolkit()->show_window(IToolkit::WindowType::Debug);
}

// ── SoundsPrefBridge ───────────────────────────────────────────────────────────

SoundsPrefBridge::SoundsPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
}

bool
SoundsPrefBridge::enabled() const
{
  return app->get_sound_theme()->sound_enabled()();
}

bool
SoundsPrefBridge::hasVolume() const
{
  return app->get_sound_theme()->capability(workrave::audio::SoundCapability::VOLUME);
}

int
SoundsPrefBridge::volume() const
{
  return app->get_sound_theme()->sound_volume()();
}

bool
SoundsPrefBridge::hasMute() const
{
  return app->get_sound_theme()->capability(workrave::audio::SoundCapability::MUTE);
}

bool
SoundsPrefBridge::mute() const
{
  return app->get_sound_theme()->sound_mute()();
}

QVariantList
SoundsPrefBridge::themes() const
{
  QVariantList result;
  for (const auto &theme : app->get_sound_theme()->get_themes())
    {
      QVariantMap m;
      m["id"]   = QString::fromStdString(theme->theme_id);
      m["name"] = QString::fromStdString(theme->description);
      result.append(m);
    }
  return result;
}

QString
SoundsPrefBridge::currentThemeId() const
{
  auto active = app->get_sound_theme()->get_active_theme();
  return active ? QString::fromStdString(active->theme_id) : QString{};
}

QVariantList
SoundsPrefBridge::events() const
{
  QVariantList result;
  for (SoundEvent event : SoundTheme::events())
    {
      auto id = SoundTheme::sound_event_to_id(event);
      QVariantMap m;
      m["id"]       = QString::fromStdString(id);
      m["name"]     = Ui::get_sound_event_name(event);
      m["enabled"]  = app->get_sound_theme()->sound_event_enabled(event)();
      m["filename"] = QString::fromStdString(app->get_sound_theme()->sound_event(event)());
      result.append(m);
    }
  return result;
}

void
SoundsPrefBridge::setEnabled(bool v)
{
  app->get_sound_theme()->sound_enabled().set(v);
  Q_EMIT soundsChanged();
}

void
SoundsPrefBridge::setVolume(int v)
{
  app->get_sound_theme()->sound_volume().set(v);
  Q_EMIT soundsChanged();
}

void
SoundsPrefBridge::setMute(bool v)
{
  app->get_sound_theme()->sound_mute().set(v);
  Q_EMIT soundsChanged();
}

void
SoundsPrefBridge::setTheme(const QString &id)
{
  app->get_sound_theme()->activate_theme(id.toStdString());
  Q_EMIT soundsChanged();
}

void
SoundsPrefBridge::setEventEnabled(const QString &id, bool v)
{
  auto event = SoundTheme::sound_id_to_event(id.toStdString());
  app->get_sound_theme()->sound_event_enabled(event).set(v);
  Q_EMIT soundsChanged();
}

void
SoundsPrefBridge::pickEventFile(const QString &id)
{
  auto event    = SoundTheme::sound_id_to_event(id.toStdString());
  auto current  = QString::fromStdString(app->get_sound_theme()->sound_event(event)());
  auto dir      = current.isEmpty() ? QString{} : QFileInfo(current).absolutePath();
  auto filename = QFileDialog::getOpenFileName(nullptr,
                                               tr("Choose sound file"),
                                               dir,
                                               tr("Sound files (*.wav *.mp3 *.ogg);;All files (*)"));
  if (!filename.isEmpty())
    {
      app->get_sound_theme()->sound_event(event).set(filename.toStdString());
      Q_EMIT soundsChanged();
    }
}

void
SoundsPrefBridge::playEvent(const QString &id)
{
  auto event = SoundTheme::sound_id_to_event(id.toStdString());
  app->get_sound_theme()->play_sound(event);
}

void
SoundsPrefBridge::clearEventFile(const QString &id)
{
  auto event = SoundTheme::sound_id_to_event(id.toStdString());
  app->get_sound_theme()->sound_event(event).set(std::string{});
  Q_EMIT soundsChanged();
}

// ── GeneralPrefBridge ──────────────────────────────────────────────────────────

GeneralPrefBridge::GeneralPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
  System::set_custom_lock_command(GUIConfig::custom_lock_command()());

  // Enumerate icon themes once (Linux: subdirectories under the images search path)
  for (const auto &dirname : workrave::utils::AssetPath::get_search_path(workrave::utils::SearchPathId::Images))
    {
      auto path = dirname.string();
      if (path.size() < 6 || path.substr(path.size() - 6) != "images")
        {
          continue;
        }

      try
        {
          for (const auto &entry : std::filesystem::directory_iterator(dirname))
            {
              if (entry.is_directory())
                {
                  QVariantMap m;
                  m["id"]   = QString::fromStdString(entry.path().filename().string());
                  m["name"] = QString::fromStdString(entry.path().filename().string());
                  iconThemes_.append(m);
                }
            }
        }
      catch (...)
        {
        }
    }

  // Build language list from ALL_LINGUAS + English
  {
    QVariantMap sys;
    sys["id"]            = QString{};
    sys["localizedName"] = tr("System default");
    sys["nativeName"]    = QString{};
    languages_.append(sys);

    QString linguas = QString::fromLatin1(ALL_LINGUAS);
    QStringList codes = linguas.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    codes.prepend(QStringLiteral("en"));

    std::sort(codes.begin(), codes.end(), [](const QString &a, const QString &b) {
      return QLocale(a).nativeLanguageName().compare(QLocale(b).nativeLanguageName(), Qt::CaseInsensitive) < 0;
    });

    for (const QString &code : std::as_const(codes))
      {
        // QLocale doesn't understand the @variant suffix used in gettext locale codes
        QString qtCode = code.section(QLatin1Char('@'), 0, 0);
        QLocale loc(qtCode);
        QVariantMap m;
        m["id"]            = code;
        m["localizedName"] = QLocale::languageToString(loc.language());
        m["nativeName"]    = loc.nativeLanguageName();
        languages_.append(m);
      }
  }
}

int
GeneralPrefBridge::blockMode() const
{
  return static_cast<int>(GUIConfig::block_mode()());
}

void
GeneralPrefBridge::setBlockMode(int v)
{
  GUIConfig::block_mode().set(static_cast<BlockMode>(v));
  Q_EMIT blockModeChanged();
}

bool
GeneralPrefBridge::sanctuaryEnabled() const
{
  return GUIConfig::sanctuary_ui_enabled()();
}

void
GeneralPrefBridge::setSanctuaryEnabled(bool v)
{
  GUIConfig::sanctuary_ui_enabled().set(v);
  Q_EMIT systemChanged();
}

bool
GeneralPrefBridge::trayIconEnabled() const
{
  return GUIConfig::trayicon_enabled()();
}

void
GeneralPrefBridge::setTrayIconEnabled(bool v)
{
  GUIConfig::trayicon_enabled().set(v);
  Q_EMIT systemChanged();
}

bool
GeneralPrefBridge::autostartEnabled() const
{
  return GUIConfig::autostart_enabled()();
}

void
GeneralPrefBridge::setAutostartEnabled(bool v)
{
  GUIConfig::autostart_enabled().set(v);
  Q_EMIT systemChanged();
}

bool
GeneralPrefBridge::hasDarkMode() const
{
  return true;
}

int
GeneralPrefBridge::darkMode() const
{
  return static_cast<int>(GUIConfig::light_dark_mode()());
}

void
GeneralPrefBridge::setDarkMode(int v)
{
  GUIConfig::light_dark_mode().set(static_cast<LightDarkTheme>(v));
  Q_EMIT systemChanged();
}

bool
GeneralPrefBridge::hasForceX11() const
{
#if defined(PLATFORM_OS_UNIX)
  return true;
#else
  return false;
#endif
}

bool
GeneralPrefBridge::forceX11() const
{
#if defined(PLATFORM_OS_UNIX)
  return GUIConfig::force_x11()();
#else
  return false;
#endif
}

void
GeneralPrefBridge::setForceX11(bool v)
{
#if defined(PLATFORM_OS_UNIX)
  GUIConfig::force_x11().set(v);
  Q_EMIT systemChanged();
#else
  (void)v;
#endif
}

bool
GeneralPrefBridge::hasGnomeShellPreludes() const
{
#if defined(PLATFORM_OS_UNIX)
  return true;
#else
  return false;
#endif
}

bool
GeneralPrefBridge::gnomeShellPreludes() const
{
#if defined(PLATFORM_OS_UNIX)
  return GUIConfig::use_gnome_shell_preludes()();
#else
  return false;
#endif
}

void
GeneralPrefBridge::setGnomeShellPreludes(bool v)
{
#if defined(PLATFORM_OS_UNIX)
  GUIConfig::use_gnome_shell_preludes().set(v);
  Q_EMIT systemChanged();
#else
  (void)v;
#endif
}

QVariantList
GeneralPrefBridge::iconThemes() const
{
  return iconThemes_;
}

QString
GeneralPrefBridge::currentIconTheme() const
{
  return QString::fromStdString(GUIConfig::icon_theme()());
}

void
GeneralPrefBridge::setIconTheme(const QString &id)
{
  GUIConfig::icon_theme().set(id.toStdString());
  Q_EMIT systemChanged();
}

QVariantList
GeneralPrefBridge::languages() const
{
  return languages_;
}

QString
GeneralPrefBridge::currentLanguage() const
{
  return QString::fromStdString(GUIConfig::locale()());
}

void
GeneralPrefBridge::setLanguage(const QString &locale)
{
  GUIConfig::locale().set(locale.toStdString());
  Q_EMIT systemChanged();
}

namespace
{
  const char *
  sleep_op_id(System::SystemOperation::SystemOperationType type)
  {
    switch (type)
      {
      case System::SystemOperation::SYSTEM_OPERATION_SUSPEND:
        return "suspend";
      case System::SystemOperation::SYSTEM_OPERATION_HIBERNATE:
        return "hibernate";
      case System::SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID:
        return "suspend_hybrid";
      default:
        return "";
      }
  }

  const char *
  sleep_op_label(System::SystemOperation::SystemOperationType type)
  {
    switch (type)
      {
      case System::SystemOperation::SYSTEM_OPERATION_SUSPEND:
        return "Suspend";
      case System::SystemOperation::SYSTEM_OPERATION_HIBERNATE:
        return "Hibernate";
      case System::SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID:
        return "Hybrid sleep";
      default:
        return "";
      }
  }
} // namespace

bool
GeneralPrefBridge::hasLockMethods() const
{
  return !System::get_lock_methods().empty();
}

QStringList
GeneralPrefBridge::lockMethodOptions() const
{
  QStringList list;
  for (const auto &m: System::get_lock_methods())
    {
      list << QString::fromStdString(m.label);
    }
  return list;
}

int
GeneralPrefBridge::lockMethodIndex() const
{
  const std::string pref = GUIConfig::preferred_lock_method()();
  auto methods = System::get_lock_methods();
  for (int i = 0; i < static_cast<int>(methods.size()); i++)
    {
      if (methods[i].id == pref)
        return i;
    }
  return 0;
}

void
GeneralPrefBridge::setLockMethodIndex(int idx)
{
  auto methods = System::get_lock_methods();
  if (idx >= 0 && idx < static_cast<int>(methods.size()))
    {
      GUIConfig::preferred_lock_method().set(methods[idx].id);
      Q_EMIT lockPowerChanged();
    }
}

QString
GeneralPrefBridge::customLockCommand() const
{
  return QString::fromStdString(GUIConfig::custom_lock_command()());
}

void
GeneralPrefBridge::setCustomLockCommand(const QString &cmd)
{
  const std::string s = cmd.toStdString();
  GUIConfig::custom_lock_command().set(s);
  System::set_custom_lock_command(s);
  Q_EMIT lockPowerChanged();
}

bool
GeneralPrefBridge::hasSleepOperations() const
{
  return !System::get_sleep_operations().empty();
}

QStringList
GeneralPrefBridge::sleepOperationOptions() const
{
  QStringList list;
  for (const auto &op: System::get_sleep_operations())
    {
      list << QString::fromLatin1(sleep_op_label(op.type));
    }
  return list;
}

int
GeneralPrefBridge::sleepOperationIndex() const
{
  const std::string pref = GUIConfig::preferred_sleep_operation()();
  auto ops = System::get_sleep_operations();
  for (int i = 0; i < static_cast<int>(ops.size()); i++)
    {
      if (sleep_op_id(ops[i].type) == pref)
        return i;
    }
  return 0;
}

void
GeneralPrefBridge::setSleepOperationIndex(int idx)
{
  auto ops = System::get_sleep_operations();
  if (idx >= 0 && idx < static_cast<int>(ops.size()))
    {
      GUIConfig::preferred_sleep_operation().set(sleep_op_id(ops[idx].type));
      Q_EMIT lockPowerChanged();
    }
}

// ── AppletPrefBridge ──────────────────────────────────────────────────────────

AppletPrefBridge::AppletPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
}

int
AppletPrefBridge::flagsToVisibility(int flags)
{
  if ((flags & GUIConfig::BREAK_HIDE) != 0)
    {
      return 2;
    }
  if ((flags & GUIConfig::BREAK_WHEN_FIRST) != 0)
    {
      return 1;
    }
  return 0;
}

int
AppletPrefBridge::visibilityToFlags(int v)
{
  if (v == 2)
    {
      return GUIConfig::BREAK_HIDE;
    }
  if (v == 1)
    {
      return GUIConfig::BREAK_WHEN_FIRST;
    }
  return 0;
}

bool
AppletPrefBridge::enabled() const
{
  return GUIConfig::timerbox_enabled("applet")();
}

void
AppletPrefBridge::setEnabled(bool v)
{
  GUIConfig::timerbox_enabled("applet").set(v);
  Q_EMIT changed();
}

int
AppletPrefBridge::displayStyle() const
{
  return static_cast<int>(GUIConfig::display_style()());
}

void
AppletPrefBridge::setDisplayStyle(int v)
{
  GUIConfig::display_style().set(static_cast<DisplayStyle>(v));
  Q_EMIT changed();
}

int
AppletPrefBridge::placement() const
{
  int mp = GUIConfig::timerbox_slot("applet", BREAK_ID_MICRO_BREAK)();
  int rb = GUIConfig::timerbox_slot("applet", BREAK_ID_REST_BREAK)();
  int dl = GUIConfig::timerbox_slot("applet", BREAK_ID_DAILY_LIMIT)();
  if (mp < rb && rb < dl)
    {
      return 0;
    }
  if (mp == rb && rb == dl)
    {
      return 3;
    }
  if (mp == rb)
    {
      return 1;
    }
  return 2;
}

void
AppletPrefBridge::setPlacement(int v)
{
  int pos[3] = {0, 1, 2};
  switch (v)
    {
    case 1: pos[1] = 0; pos[2] = 1; break;
    case 2: pos[2] = 1; break;
    case 3: pos[1] = 0; pos[2] = 0; break;
    default: break;
    }
  GUIConfig::timerbox_slot("applet", BREAK_ID_MICRO_BREAK).set(pos[0]);
  GUIConfig::timerbox_slot("applet", BREAK_ID_REST_BREAK).set(pos[1]);
  GUIConfig::timerbox_slot("applet", BREAK_ID_DAILY_LIMIT).set(pos[2]);
  Q_EMIT changed();
}

QString
AppletPrefBridge::cycleDisplay() const
{
  return PrefUtils::formatTime(GUIConfig::timerbox_cycle_time("applet")());
}

double
AppletPrefBridge::cycleNorm() const
{
  return PrefUtils::normalize(GUIConfig::timerbox_cycle_time("applet")(), CYCLE_MIN, CYCLE_MAX);
}

void
AppletPrefBridge::incrementCycle()
{
  int v = PrefUtils::clampStep(GUIConfig::timerbox_cycle_time("applet")(), +1, CYCLE_MIN, CYCLE_MAX, CYCLE_STEP);
  GUIConfig::timerbox_cycle_time("applet").set(v);
  Q_EMIT changed();
}

void
AppletPrefBridge::decrementCycle()
{
  int v = PrefUtils::clampStep(GUIConfig::timerbox_cycle_time("applet")(), -1, CYCLE_MIN, CYCLE_MAX, CYCLE_STEP);
  GUIConfig::timerbox_cycle_time("applet").set(v);
  Q_EMIT changed();
}

void
AppletPrefBridge::setCycleNorm(double norm)
{
  int v = PrefUtils::denormalize(norm, CYCLE_MIN, CYCLE_MAX, CYCLE_STEP);
  GUIConfig::timerbox_cycle_time("applet").set(v);
  Q_EMIT changed();
}

void
AppletPrefBridge::setCycleSeconds(int seconds)
{
  GUIConfig::timerbox_cycle_time("applet").set(std::clamp(seconds, 1, 86400));
  Q_EMIT changed();
}

int
AppletPrefBridge::microVisibility() const
{
  return flagsToVisibility(GUIConfig::timerbox_flags("applet", BREAK_ID_MICRO_BREAK)());
}

void
AppletPrefBridge::setMicroVisibility(int v)
{
  GUIConfig::timerbox_flags("applet", BREAK_ID_MICRO_BREAK).set(visibilityToFlags(v));
  Q_EMIT changed();
}

int
AppletPrefBridge::restVisibility() const
{
  return flagsToVisibility(GUIConfig::timerbox_flags("applet", BREAK_ID_REST_BREAK)());
}

void
AppletPrefBridge::setRestVisibility(int v)
{
  GUIConfig::timerbox_flags("applet", BREAK_ID_REST_BREAK).set(visibilityToFlags(v));
  Q_EMIT changed();
}

int
AppletPrefBridge::dailyVisibility() const
{
  return flagsToVisibility(GUIConfig::timerbox_flags("applet", BREAK_ID_DAILY_LIMIT)());
}

void
AppletPrefBridge::setDailyVisibility(int v)
{
  GUIConfig::timerbox_flags("applet", BREAK_ID_DAILY_LIMIT).set(visibilityToFlags(v));
  Q_EMIT changed();
}
