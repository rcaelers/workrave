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

#ifndef QMLPRELUDEWINDOW_HH
#define QMLPRELUDEWINDOW_HH

#include <memory>
#include <QEvent>
#include <QObject>
#include <QScreen>
#include <QQuickView>
#include <QString>

#include "core/IApp.hh"
#include "core/CoreTypes.hh"
#include "ui/IPreludeWindow.hh"
#include "ui/IApplicationContext.hh"
#include "ui/GUIConfig.hh"
#include "utils/Signals.hh"
#include "UiUtil.hh"

#if defined(HAVE_WAYLAND)
#  include "WaylandWindowManager.hh"
#endif
#if defined(PLATFORM_OS_MACOS)
#  include "MouseMonitor.hh"
#endif

// Fires a callback on QEvent::Enter so QmlPreludeWindow can avoid the mouse pointer
// without needing to be a QObject itself.
class ViewEventFilter : public QObject
{
  Q_OBJECT
public:
  using Callback = std::function<void()>;
  explicit ViewEventFilter(Callback cb, QObject *parent = nullptr) : QObject(parent), cb_(std::move(cb)) {}

  bool eventFilter(QObject * /*watched*/, QEvent *event) override
  {
    if (event->type() == QEvent::Enter)
      cb_();
    return false;
  }

private:
  Callback cb_;
};

// Data bridge exposed to PreludeShell.qml / PreludeOverlay.qml / PreludeClassic.qml.
class PreludeBridge
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

  Q_PROPERTY(int     breakType     READ breakType     CONSTANT)
  Q_PROPERTY(QString heading       READ heading       CONSTANT)
  Q_PROPERTY(double  ringProgress  READ ringProgress  NOTIFY progressChanged)
  Q_PROPERTY(QString timeLabel     READ timeLabel     NOTIFY progressChanged)
  Q_PROPERTY(QString timeShort     READ timeShort     NOTIFY progressChanged)
  Q_PROPERTY(QString countdownText READ countdownText NOTIFY progressChanged)
  Q_PROPERTY(int     stage         READ stage         NOTIFY stageChanged)
  Q_PROPERTY(bool    fullscreen    READ isFullscreen  NOTIFY layoutChanged)
  Q_PROPERTY(bool    cardAtBottom  READ isCardAtBottom NOTIFY layoutChanged)
  // classic == true  → PreludeClassic.qml (GTK look)
  // classic == false → PreludeOverlay.qml (Sanctuary)
  Q_PROPERTY(bool    classic       READ isClassic     NOTIFY classicChanged)
  Q_PROPERTY(int     cardW         READ cardW         NOTIFY classicChanged)
  Q_PROPERTY(int     cardH         READ cardH         NOTIFY classicChanged)

public:
  explicit PreludeBridge(workrave::BreakId break_id, QObject *parent = nullptr);

  int     breakType()      const { return static_cast<int>(break_id); }
  QString heading()        const;
  double  ringProgress()   const;
  QString timeLabel()      const;
  QString timeShort()      const;
  QString countdownText()  const;
  int     stage()          const { return stage_; }
  bool    isFullscreen()   const { return fullscreen_; }
  bool    isCardAtBottom() const { return card_at_bottom_; }
  bool    isClassic()      const { return classic_; }
  // Compact content-sized card like the Gtk prelude; roomier card for Sanctuary.
  int     cardW()          const { return classic_ ? 320 : 480; }
  int     cardH()          const { return classic_ ? 72 : 80; }

  void setProgress(int value, int max_value);
  void setStage(workrave::IApp::PreludeStage stage);
  void setProgressText(workrave::IApp::PreludeProgressText text);
  void setFullscreen(bool v)   { fullscreen_     = v; Q_EMIT layoutChanged(); }
  void setCardAtBottom(bool v) { card_at_bottom_ = v; Q_EMIT layoutChanged(); }

  Q_INVOKABLE void requestSkip();

Q_SIGNALS:
  void progressChanged();
  void stageChanged();
  void skipRequested();
  void layoutChanged();
  void classicChanged();

private:
  workrave::BreakId break_id;
  int progress_value{0};
  int progress_max{1};
  int stage_{0};
  QString progress_label;
  bool fullscreen_{false};
  bool card_at_bottom_{false};
  bool classic_{false};
};

// IPreludeWindow implementation hosting PreludeShell.qml in a QQuickView.
// Always QML-based; the shell switches between Sanctuary and Classic designs
// live when GUIConfig::sanctuary_ui_enabled changes.
class QmlPreludeWindow : public IPreludeWindow
{
public:
  QmlPreludeWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, workrave::BreakId break_id);
  ~QmlPreludeWindow() override;

  void start() override;
  void stop() override;
  void refresh() override {}
  void set_progress(int value, int max_value) override;
  void set_stage(workrave::IApp::PreludeStage stage) override;
  void set_progress_text(workrave::IApp::PreludeProgressText text) override;

private:
  static constexpr int MARGIN = 20;

  QRect top_rect()    const;
  QRect bottom_rect() const;
  void  avoid_pointer(int x, int y);
  void  update_input_region();

  QScreen       *screen{nullptr};
  QQuickView    *view{nullptr};
  PreludeBridge *bridge{nullptr};

  bool at_bottom{false};
  bool did_avoid{false};
  bool position_windows{true};

#if defined(PLATFORM_OS_MACOS)
  MouseMonitor::Ptr mouse_monitor;
#endif
#if defined(HAVE_WAYLAND)
  std::shared_ptr<WaylandWindowManager> window_manager;
#endif
};

#endif // QMLPRELUDEWINDOW_HH
