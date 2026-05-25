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
#include <QObject>
#include <QScreen>
#include <QQuickView>
#include <QString>

#include "core/IApp.hh"
#include "core/CoreTypes.hh"
#include "ui/IPreludeWindow.hh"
#include "UiUtil.hh"

// Data bridge exposed to PreludeOverlay.qml as "bridge" context property.
class PreludeBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(int     breakType     READ breakType     CONSTANT)
  Q_PROPERTY(QString heading       READ heading       CONSTANT)
  Q_PROPERTY(double  ringProgress  READ ringProgress  NOTIFY progressChanged)
  Q_PROPERTY(QString timeLabel     READ timeLabel     NOTIFY progressChanged)
  Q_PROPERTY(QString timeShort     READ timeShort     NOTIFY progressChanged)
  Q_PROPERTY(QString countdownText READ countdownText NOTIFY progressChanged)
  Q_PROPERTY(int     stage         READ stage         NOTIFY stageChanged)

public:
  explicit PreludeBridge(workrave::BreakId break_id, QObject *parent = nullptr);

  int     breakType()     const { return static_cast<int>(break_id); }
  QString heading()       const;
  double  ringProgress()  const;
  QString timeLabel()     const;
  QString timeShort()     const;
  QString countdownText() const;
  int     stage()         const { return stage_; }

  void setProgress(int value, int max_value);
  void setStage(workrave::IApp::PreludeStage stage);
  void setProgressText(workrave::IApp::PreludeProgressText text);

  Q_INVOKABLE void requestSkip();

Q_SIGNALS:
  void progressChanged();
  void stageChanged();
  void skipRequested();

private:
  workrave::BreakId break_id;
  int progress_value{0};
  int progress_max{1};
  int stage_{0};
  QString progress_label;
};

// IPreludeWindow implementation hosting PreludeOverlay.qml in a QQuickView.
class QmlPreludeWindow : public IPreludeWindow
{
public:
  QmlPreludeWindow(QScreen *screen, workrave::BreakId break_id);
  ~QmlPreludeWindow() override;

  void start() override;
  void stop() override;
  void refresh() override {}
  void set_progress(int value, int max_value) override;
  void set_stage(workrave::IApp::PreludeStage stage) override;
  void set_progress_text(workrave::IApp::PreludeProgressText text) override;

private:
  static constexpr int CARD_W  = 480;
  static constexpr int CARD_H  = 84;
  static constexpr int MARGIN  = 20;

  QRect top_rect()    const;
  QRect bottom_rect() const;

  QScreen *screen{nullptr};
  QQuickView *view{nullptr};
  PreludeBridge *bridge{nullptr};
  bool at_bottom{false};
};

#endif // QMLPRELUDEWINDOW_HH
