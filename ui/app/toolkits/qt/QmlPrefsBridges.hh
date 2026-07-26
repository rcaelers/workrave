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

#ifndef QMLPREFSBRIDGES_HH
#define QMLPREFSBRIDGES_HH

#include <memory>
#include <QObject>
#include <QString>
#include <QVariantList>

#include "core/CoreTypes.hh"
#include "ui/IApplicationContext.hh"

// ── Shared utility ─────────────────────────────────────────────────────────────

namespace PrefUtils
{
  QString formatTime(int seconds);
  double  normalize(int value, int minVal, int maxVal);
  int     denormalize(double norm, int minVal, int maxVal, int step);
  int     clampStep(int value, int delta, int minVal, int maxVal, int step);
  int     snapStep(int value, int minVal, int maxVal, int step);
} // namespace PrefUtils

// ── MicrobreakPrefBridge ───────────────────────────────────────────────────────

class MicrobreakPrefBridge : public QObject
{
  Q_OBJECT

  // Enabled toggle
  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

  // Timing display + slider
  Q_PROPERTY(QString limitDisplay    READ limitDisplay    NOTIFY timingChanged)
  Q_PROPERTY(double  limitNorm       READ limitNorm       NOTIFY timingChanged)
  Q_PROPERTY(QString durationDisplay READ durationDisplay NOTIFY timingChanged)
  Q_PROPERTY(double  durationNorm    READ durationNorm    NOTIFY timingChanged)
  Q_PROPERTY(QString snoozeDisplay   READ snoozeDisplay   NOTIFY timingChanged)
  Q_PROPERTY(double  snoozeNorm      READ snoozeNorm      NOTIFY timingChanged)

  // Break window options
  Q_PROPERTY(bool showPostpone  READ showPostpone  WRITE setShowPostpone  NOTIFY optionsChanged)
  Q_PROPERTY(bool showSkip      READ showSkip      WRITE setShowSkip      NOTIFY optionsChanged)
  Q_PROPERTY(bool preludeEnabled READ preludeEnabled WRITE setPreludeEnabled NOTIFY optionsChanged)
  Q_PROPERTY(bool hasMaxPreludes READ hasMaxPreludes NOTIFY optionsChanged)
  Q_PROPERTY(int  maxPreludes   READ maxPreludes   NOTIFY optionsChanged)

public:
  explicit MicrobreakPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  bool    enabled() const;
  Q_INVOKABLE void setEnabled(bool v);

  QString limitDisplay() const;
  double  limitNorm() const;
  QString durationDisplay() const;
  double  durationNorm() const;
  QString snoozeDisplay() const;
  double  snoozeNorm() const;

  bool showPostpone() const;
  Q_INVOKABLE void setShowPostpone(bool v);
  bool showSkip() const;
  Q_INVOKABLE void setShowSkip(bool v);
  bool preludeEnabled() const;
  Q_INVOKABLE void setPreludeEnabled(bool v);
  bool hasMaxPreludes() const;
  Q_INVOKABLE void setHasMaxPreludes(bool v);
  int  maxPreludes() const;

  Q_INVOKABLE void incrementLimit();
  Q_INVOKABLE void decrementLimit();
  Q_INVOKABLE void setLimitNorm(double norm);
  Q_INVOKABLE void setLimitSeconds(int seconds);

  Q_INVOKABLE void incrementDuration();
  Q_INVOKABLE void decrementDuration();
  Q_INVOKABLE void setDurationNorm(double norm);
  Q_INVOKABLE void setDurationSeconds(int seconds);

  Q_INVOKABLE void incrementSnooze();
  Q_INVOKABLE void decrementSnooze();
  Q_INVOKABLE void setSnoozeNorm(double norm);
  Q_INVOKABLE void setSnoozeSeconds(int seconds);

  Q_INVOKABLE void incrementMaxPreludes();
  Q_INVOKABLE void decrementMaxPreludes();

Q_SIGNALS:
  void enabledChanged();
  void timingChanged();
  void optionsChanged();

private:
  std::shared_ptr<IApplicationContext> app;

  static constexpr int LIMIT_MIN  = 60;
  static constexpr int LIMIT_MAX  = 600;
  static constexpr int LIMIT_STEP = 5;

  static constexpr int DUR_MIN  = 15;
  static constexpr int DUR_MAX  = 120;
  static constexpr int DUR_STEP = 5;

  static constexpr int SNOOZE_MIN  = 60;
  static constexpr int SNOOZE_MAX  = 600;
  static constexpr int SNOOZE_STEP = 60;
};

// ── RestBreakPrefBridge ────────────────────────────────────────────────────────

class RestBreakPrefBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

  Q_PROPERTY(QString limitDisplay    READ limitDisplay    NOTIFY timingChanged)
  Q_PROPERTY(double  limitNorm       READ limitNorm       NOTIFY timingChanged)
  Q_PROPERTY(QString durationDisplay READ durationDisplay NOTIFY timingChanged)
  Q_PROPERTY(double  durationNorm    READ durationNorm    NOTIFY timingChanged)
  Q_PROPERTY(QString snoozeDisplay   READ snoozeDisplay   NOTIFY timingChanged)
  Q_PROPERTY(double  snoozeNorm      READ snoozeNorm      NOTIFY timingChanged)

  Q_PROPERTY(int  exercises     READ exercises     NOTIFY optionsChanged)
  Q_PROPERTY(bool autoNatural   READ autoNatural   WRITE setAutoNatural   NOTIFY optionsChanged)
  Q_PROPERTY(bool enableShutdown READ enableShutdown WRITE setEnableShutdown NOTIFY optionsChanged)
  Q_PROPERTY(bool showPostpone  READ showPostpone  WRITE setShowPostpone  NOTIFY optionsChanged)
  Q_PROPERTY(bool showSkip      READ showSkip      WRITE setShowSkip      NOTIFY optionsChanged)
  Q_PROPERTY(bool preludeEnabled READ preludeEnabled WRITE setPreludeEnabled NOTIFY optionsChanged)
  Q_PROPERTY(bool hasMaxPreludes READ hasMaxPreludes NOTIFY optionsChanged)
  Q_PROPERTY(int  maxPreludes   READ maxPreludes   NOTIFY optionsChanged)

public:
  explicit RestBreakPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  bool    enabled() const;
  Q_INVOKABLE void setEnabled(bool v);

  QString limitDisplay() const;
  double  limitNorm() const;
  QString durationDisplay() const;
  double  durationNorm() const;
  QString snoozeDisplay() const;
  double  snoozeNorm() const;

  int  exercises() const;
  bool autoNatural() const;
  Q_INVOKABLE void setAutoNatural(bool v);
  bool enableShutdown() const;
  Q_INVOKABLE void setEnableShutdown(bool v);
  bool showPostpone() const;
  Q_INVOKABLE void setShowPostpone(bool v);
  bool showSkip() const;
  Q_INVOKABLE void setShowSkip(bool v);
  bool preludeEnabled() const;
  Q_INVOKABLE void setPreludeEnabled(bool v);
  bool hasMaxPreludes() const;
  Q_INVOKABLE void setHasMaxPreludes(bool v);
  int  maxPreludes() const;

  Q_INVOKABLE void incrementLimit();
  Q_INVOKABLE void decrementLimit();
  Q_INVOKABLE void setLimitNorm(double norm);
  Q_INVOKABLE void setLimitSeconds(int seconds);

  Q_INVOKABLE void incrementDuration();
  Q_INVOKABLE void decrementDuration();
  Q_INVOKABLE void setDurationNorm(double norm);
  Q_INVOKABLE void setDurationSeconds(int seconds);

  Q_INVOKABLE void incrementSnooze();
  Q_INVOKABLE void decrementSnooze();
  Q_INVOKABLE void setSnoozeNorm(double norm);
  Q_INVOKABLE void setSnoozeSeconds(int seconds);

  Q_INVOKABLE void incrementExercises();
  Q_INVOKABLE void decrementExercises();

  Q_INVOKABLE void incrementMaxPreludes();
  Q_INVOKABLE void decrementMaxPreludes();

Q_SIGNALS:
  void enabledChanged();
  void timingChanged();
  void optionsChanged();

private:
  std::shared_ptr<IApplicationContext> app;

  static constexpr int LIMIT_MIN  = 900;
  static constexpr int LIMIT_MAX  = 5400;
  static constexpr int LIMIT_STEP = 300;

  static constexpr int DUR_MIN  = 300;
  static constexpr int DUR_MAX  = 1200;
  static constexpr int DUR_STEP = 60;

  static constexpr int SNOOZE_MIN  = 60;
  static constexpr int SNOOZE_MAX  = 600;
  static constexpr int SNOOZE_STEP = 60;
};

// ── DailyLimitPrefBridge ───────────────────────────────────────────────────────

class DailyLimitPrefBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

  Q_PROPERTY(QString limitDisplay  READ limitDisplay  NOTIFY timingChanged)
  Q_PROPERTY(double  limitNorm     READ limitNorm     NOTIFY timingChanged)
  Q_PROPERTY(QString snoozeDisplay READ snoozeDisplay NOTIFY timingChanged)
  Q_PROPERTY(double  snoozeNorm    READ snoozeNorm    NOTIFY timingChanged)

  Q_PROPERTY(bool useMicroBreakActivity READ useMicroBreakActivity WRITE setUseMicroBreakActivity NOTIFY optionsChanged)
  Q_PROPERTY(bool enableShutdown READ enableShutdown WRITE setEnableShutdown NOTIFY optionsChanged)
  Q_PROPERTY(bool showPostpone   READ showPostpone   WRITE setShowPostpone   NOTIFY optionsChanged)
  Q_PROPERTY(bool showSkip       READ showSkip       WRITE setShowSkip       NOTIFY optionsChanged)
  Q_PROPERTY(bool preludeEnabled READ preludeEnabled WRITE setPreludeEnabled NOTIFY optionsChanged)
  Q_PROPERTY(bool hasMaxPreludes READ hasMaxPreludes NOTIFY optionsChanged)
  Q_PROPERTY(int  maxPreludes    READ maxPreludes    NOTIFY optionsChanged)

public:
  explicit DailyLimitPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  bool    enabled() const;
  Q_INVOKABLE void setEnabled(bool v);

  QString limitDisplay() const;
  double  limitNorm() const;
  QString snoozeDisplay() const;
  double  snoozeNorm() const;

  bool useMicroBreakActivity() const;
  Q_INVOKABLE void setUseMicroBreakActivity(bool v);
  bool enableShutdown() const;
  Q_INVOKABLE void setEnableShutdown(bool v);
  bool showPostpone() const;
  Q_INVOKABLE void setShowPostpone(bool v);
  bool showSkip() const;
  Q_INVOKABLE void setShowSkip(bool v);
  bool preludeEnabled() const;
  Q_INVOKABLE void setPreludeEnabled(bool v);
  bool hasMaxPreludes() const;
  Q_INVOKABLE void setHasMaxPreludes(bool v);
  int  maxPreludes() const;

  Q_INVOKABLE void incrementLimit();
  Q_INVOKABLE void decrementLimit();
  Q_INVOKABLE void setLimitNorm(double norm);
  Q_INVOKABLE void setLimitSeconds(int seconds);

  Q_INVOKABLE void incrementSnooze();
  Q_INVOKABLE void decrementSnooze();
  Q_INVOKABLE void setSnoozeNorm(double norm);
  Q_INVOKABLE void setSnoozeSeconds(int seconds);

  Q_INVOKABLE void incrementMaxPreludes();
  Q_INVOKABLE void decrementMaxPreludes();

Q_SIGNALS:
  void enabledChanged();
  void timingChanged();
  void optionsChanged();

private:
  std::shared_ptr<IApplicationContext> app;

  static constexpr int LIMIT_MIN  = 7200;
  static constexpr int LIMIT_MAX  = 43200;
  static constexpr int LIMIT_STEP = 1800;

  static constexpr int SNOOZE_MIN  = 60;
  static constexpr int SNOOZE_MAX  = 600;
  static constexpr int SNOOZE_STEP = 60;
};

// ── StatusWindowPrefBridge ────────────────────────────────────────────────────

class StatusWindowPrefBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool    enabled         READ enabled         NOTIFY changed)
  Q_PROPERTY(bool    alwaysOnTop     READ alwaysOnTop     NOTIFY changed)
  // displayStyle: 0=Rings, 1=Bars, 2=Focus
  Q_PROPERTY(int     displayStyle    READ displayStyle    NOTIFY changed)
  // placement: 0=separate, 1=M+R, 2=R+D, 3=all-in-one
  Q_PROPERTY(int     placement       READ placement       NOTIFY changed)
  Q_PROPERTY(QString cycleDisplay    READ cycleDisplay    NOTIFY changed)
  Q_PROPERTY(double  cycleNorm       READ cycleNorm       NOTIFY changed)
  // visibility: 0=show, 1=first-due, 2=hide
  Q_PROPERTY(int     microVisibility READ microVisibility NOTIFY changed)
  Q_PROPERTY(int     restVisibility  READ restVisibility  NOTIFY changed)
  Q_PROPERTY(int     dailyVisibility READ dailyVisibility NOTIFY changed)

public:
  explicit StatusWindowPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  bool    enabled() const;
  bool    alwaysOnTop() const;
  int     displayStyle() const;
  int     placement() const;
  QString cycleDisplay() const;
  double  cycleNorm() const;
  int     microVisibility() const;
  int     restVisibility() const;
  int     dailyVisibility() const;

  Q_INVOKABLE void setEnabled(bool v);
  Q_INVOKABLE void setAlwaysOnTop(bool v);
  Q_INVOKABLE void setDisplayStyle(int v);
  Q_INVOKABLE void setPlacement(int v);
  Q_INVOKABLE void incrementCycle();
  Q_INVOKABLE void decrementCycle();
  Q_INVOKABLE void setCycleNorm(double norm);
  Q_INVOKABLE void setCycleSeconds(int seconds);
  Q_INVOKABLE void setMicroVisibility(int v);
  Q_INVOKABLE void setRestVisibility(int v);
  Q_INVOKABLE void setDailyVisibility(int v);

Q_SIGNALS:
  void changed();

private:
  static int flagsToVisibility(int flags);
  static int visibilityToFlags(int v);

  std::shared_ptr<IApplicationContext> app;

  static constexpr int CYCLE_MIN  = 2;
  static constexpr int CYCLE_MAX  = 30;
  static constexpr int CYCLE_STEP = 1;
};

// ── MonitoringPrefBridge ───────────────────────────────────────────────────────

class MonitoringPrefBridge : public QObject
{
  Q_OBJECT

  // Windows-only activity-detection settings
  Q_PROPERTY(bool hasAlternateMonitor READ hasAlternateMonitor CONSTANT)
  Q_PROPERTY(bool alternateMonitor    READ alternateMonitor    NOTIFY monitoringChanged)
  Q_PROPERTY(int  sensitivity         READ sensitivity         NOTIFY monitoringChanged)

public:
  explicit MonitoringPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  bool hasAlternateMonitor() const;
  bool alternateMonitor() const;
  int  sensitivity() const;

  Q_INVOKABLE void setAlternateMonitor(bool v);
  Q_INVOKABLE void setSensitivity(int v);
  Q_INVOKABLE void openDebugWindow();

Q_SIGNALS:
  void monitoringChanged();

private:
  std::shared_ptr<IApplicationContext> app;
};

// ── SoundsPrefBridge ───────────────────────────────────────────────────────────

class SoundsPrefBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool         enabled        READ enabled        NOTIFY soundsChanged)
  Q_PROPERTY(bool         hasVolume      READ hasVolume      CONSTANT)
  Q_PROPERTY(int          volume         READ volume         NOTIFY soundsChanged)
  Q_PROPERTY(bool         hasMute        READ hasMute        CONSTANT)
  Q_PROPERTY(bool         mute           READ mute           NOTIFY soundsChanged)
  Q_PROPERTY(QVariantList themes         READ themes         NOTIFY soundsChanged)
  Q_PROPERTY(QString      currentThemeId READ currentThemeId NOTIFY soundsChanged)
  Q_PROPERTY(QVariantList events         READ events         NOTIFY soundsChanged)

public:
  explicit SoundsPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  bool         enabled() const;
  bool         hasVolume() const;
  int          volume() const;
  bool         hasMute() const;
  bool         mute() const;
  QVariantList themes() const;
  QString      currentThemeId() const;
  QVariantList events() const;

  Q_INVOKABLE void setEnabled(bool v);
  Q_INVOKABLE void setVolume(int v);
  Q_INVOKABLE void setMute(bool v);
  Q_INVOKABLE void setTheme(const QString &id);
  Q_INVOKABLE void setEventEnabled(const QString &id, bool v);
  Q_INVOKABLE void pickEventFile(const QString &id);
  Q_INVOKABLE void playEvent(const QString &id);
  Q_INVOKABLE void clearEventFile(const QString &id);

Q_SIGNALS:
  void soundsChanged();

private:
  std::shared_ptr<IApplicationContext> app;
};

// ── GeneralPrefBridge ──────────────────────────────────────────────────────────

class GeneralPrefBridge : public QObject
{
  Q_OBJECT

  // Block mode: 0=Off, 1=Input, 2=All
  Q_PROPERTY(int  blockMode        READ blockMode        WRITE setBlockMode        NOTIFY blockModeChanged)
  Q_PROPERTY(bool sanctuaryEnabled READ sanctuaryEnabled WRITE setSanctuaryEnabled NOTIFY systemChanged)
  Q_PROPERTY(bool trayIconEnabled  READ trayIconEnabled  WRITE setTrayIconEnabled  NOTIFY systemChanged)
  Q_PROPERTY(bool autostartEnabled READ autostartEnabled WRITE setAutostartEnabled NOTIFY systemChanged)

  // Windows-only
  Q_PROPERTY(bool hasDarkMode  READ hasDarkMode  CONSTANT)
  // darkMode: 0=Light, 1=Dark, 2=Auto
  Q_PROPERTY(int  darkMode     READ darkMode     NOTIFY systemChanged)

  // Unix-only
  Q_PROPERTY(bool hasForceX11           READ hasForceX11           CONSTANT)
  Q_PROPERTY(bool forceX11              READ forceX11              NOTIFY systemChanged)
  Q_PROPERTY(bool hasGnomeShellPreludes READ hasGnomeShellPreludes CONSTANT)
  Q_PROPERTY(bool gnomeShellPreludes    READ gnomeShellPreludes    NOTIFY systemChanged)

  // Linux icon-theme picker (empty list ⇒ row hidden)
  Q_PROPERTY(QVariantList iconThemes       READ iconThemes       CONSTANT)
  Q_PROPERTY(QString      currentIconTheme READ currentIconTheme NOTIFY systemChanged)

  // Language picker — each entry: {id, localizedName, nativeName}
  Q_PROPERTY(QVariantList languages       READ languages       CONSTANT)
  Q_PROPERTY(QString      currentLanguage READ currentLanguage NOTIFY systemChanged)

  // Lock & power (global, not per-break)
  Q_PROPERTY(bool         hasLockMethods        READ hasLockMethods        NOTIFY lockPowerChanged)
  Q_PROPERTY(QStringList  lockMethodOptions     READ lockMethodOptions     NOTIFY lockPowerChanged)
  Q_PROPERTY(int          lockMethodIndex       READ lockMethodIndex       NOTIFY lockPowerChanged)
  Q_PROPERTY(QString      customLockCommand     READ customLockCommand     NOTIFY lockPowerChanged)
  Q_PROPERTY(bool         hasSleepOperations    READ hasSleepOperations    CONSTANT)
  Q_PROPERTY(QStringList  sleepOperationOptions READ sleepOperationOptions CONSTANT)
  Q_PROPERTY(int          sleepOperationIndex   READ sleepOperationIndex   NOTIFY lockPowerChanged)

public:
  explicit GeneralPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  int  blockMode() const;
  Q_INVOKABLE void setBlockMode(int v);

  bool sanctuaryEnabled() const;
  Q_INVOKABLE void setSanctuaryEnabled(bool v);

  bool trayIconEnabled() const;
  Q_INVOKABLE void setTrayIconEnabled(bool v);

  bool autostartEnabled() const;
  Q_INVOKABLE void setAutostartEnabled(bool v);

  bool         hasDarkMode() const;
  int          darkMode() const;

  bool         hasForceX11() const;
  bool         forceX11() const;

  bool         hasGnomeShellPreludes() const;
  bool         gnomeShellPreludes() const;

  QVariantList iconThemes() const;
  QString      currentIconTheme() const;

  QVariantList languages() const;
  QString      currentLanguage() const;

  Q_INVOKABLE void setDarkMode(int v);
  Q_INVOKABLE void setForceX11(bool v);
  Q_INVOKABLE void setGnomeShellPreludes(bool v);
  Q_INVOKABLE void setIconTheme(const QString &id);
  Q_INVOKABLE void setLanguage(const QString &locale);

  bool        hasLockMethods() const;
  QStringList lockMethodOptions() const;
  int         lockMethodIndex() const;
  Q_INVOKABLE void setLockMethodIndex(int idx);
  QString     customLockCommand() const;
  Q_INVOKABLE void setCustomLockCommand(const QString &cmd);

  bool        hasSleepOperations() const;
  QStringList sleepOperationOptions() const;
  int         sleepOperationIndex() const;
  Q_INVOKABLE void setSleepOperationIndex(int idx);

Q_SIGNALS:
  void blockModeChanged();
  void systemChanged();
  void lockPowerChanged();

private:
  QVariantList iconThemes_;   // computed once in constructor
  QVariantList languages_;    // computed once in constructor

  std::shared_ptr<IApplicationContext> app;
};

// ── AppletPrefBridge ──────────────────────────────────────────────────────────
// Mirrors StatusWindowPrefBridge for the "applet" timerbox; no alwaysOnTop.

class AppletPrefBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool    enabled         READ enabled         NOTIFY changed)
  Q_PROPERTY(int     displayStyle    READ displayStyle    NOTIFY changed)
  Q_PROPERTY(int     placement       READ placement       NOTIFY changed)
  Q_PROPERTY(QString cycleDisplay    READ cycleDisplay    NOTIFY changed)
  Q_PROPERTY(double  cycleNorm       READ cycleNorm       NOTIFY changed)
  Q_PROPERTY(int     microVisibility READ microVisibility NOTIFY changed)
  Q_PROPERTY(int     restVisibility  READ restVisibility  NOTIFY changed)
  Q_PROPERTY(int     dailyVisibility READ dailyVisibility NOTIFY changed)

public:
  explicit AppletPrefBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  bool    enabled() const;
  int     displayStyle() const;
  int     placement() const;
  QString cycleDisplay() const;
  double  cycleNorm() const;
  int     microVisibility() const;
  int     restVisibility() const;
  int     dailyVisibility() const;

  Q_INVOKABLE void setEnabled(bool v);
  Q_INVOKABLE void setDisplayStyle(int v);
  Q_INVOKABLE void setPlacement(int v);
  Q_INVOKABLE void incrementCycle();
  Q_INVOKABLE void decrementCycle();
  Q_INVOKABLE void setCycleNorm(double norm);
  Q_INVOKABLE void setCycleSeconds(int seconds);
  Q_INVOKABLE void setMicroVisibility(int v);
  Q_INVOKABLE void setRestVisibility(int v);
  Q_INVOKABLE void setDailyVisibility(int v);

Q_SIGNALS:
  void changed();

private:
  static int flagsToVisibility(int flags);
  static int visibilityToFlags(int v);

  std::shared_ptr<IApplicationContext> app;

  static constexpr int CYCLE_MIN  = 2;
  static constexpr int CYCLE_MAX  = 30;
  static constexpr int CYCLE_STEP = 1;
};

#endif // QMLPREFSBRIDGES_HH
