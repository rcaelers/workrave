// Copyright (C) 2025 Rob Caelers <rob.caelers@gmail.com>
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

#ifndef QML_AUTO_UPDATE_DIALOG_HH
#define QML_AUTO_UPDATE_DIALOG_HH

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <QObject>
#include <QQuickView>
#include <QString>
#include <QWindow>

#include "unfold/Unfold.hh"

// ── AutoUpdateBridge ──────────────────────────────────────────────────────────
// QObject bridge exposing update info and install state to the QML layer.

class AutoUpdateBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString infoTitle READ infoTitle CONSTANT)
  Q_PROPERTY(QString infoText READ infoText CONSTANT)
  Q_PROPERTY(QString releaseNotes READ releaseNotes CONSTANT)
  Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
  Q_PROPERTY(bool progressVisible READ progressVisible NOTIFY progressVisibleChanged)
  Q_PROPERTY(int progressValue READ progressValue NOTIFY progressValueChanged)
  Q_PROPERTY(bool showInstallButtons READ showInstallButtons NOTIFY showInstallButtonsChanged)
  Q_PROPERTY(bool showClose READ showClose NOTIFY showCloseChanged)
  Q_PROPERTY(bool installEnabled READ installEnabled NOTIFY installEnabledChanged)

public:
  explicit AutoUpdateBridge(QObject *parent = nullptr);

  void setInfoTitle(const QString &v);
  void setInfoText(const QString &v);
  void setReleaseNotes(const QString &v);
  void setStatusText(const QString &v);
  void setProgressVisible(bool v);
  void setProgressValue(int v);
  void setShowInstallButtons(bool v);
  void setShowClose(bool v);
  void setInstallEnabled(bool v);

  QString infoTitle() const { return info_title_; }
  QString infoText() const { return info_text_; }
  QString releaseNotes() const { return release_notes_; }
  QString statusText() const { return status_text_; }
  bool progressVisible() const { return progress_visible_; }
  int progressValue() const { return progress_value_; }
  bool showInstallButtons() const { return show_install_buttons_; }
  bool showClose() const { return show_close_; }
  bool installEnabled() const { return install_enabled_; }

  Q_INVOKABLE void skip();
  Q_INVOKABLE void later();
  Q_INVOKABLE void install();

  using action_callback_t = std::function<void(int)>; // 0=skip, 1=later, 2=install, 3=dismiss
  void setActionCallback(action_callback_t cb) { action_callback_ = std::move(cb); }

Q_SIGNALS:
  void statusTextChanged();
  void progressVisibleChanged();
  void progressValueChanged();
  void showInstallButtonsChanged();
  void showCloseChanged();
  void installEnabledChanged();
  void closeRequested();

private:
  QString info_title_;
  QString info_text_;
  QString release_notes_;
  QString status_text_;
  bool progress_visible_{false};
  int progress_value_{0};
  bool show_install_buttons_{true};
  bool show_close_{false};
  bool install_enabled_{true};

  action_callback_t action_callback_;
};

// ── QmlAutoUpdateDialog ───────────────────────────────────────────────────────
// Drop-in replacement for the old QDialog-based AutoUpdateDialog.
// Owns a QQuickView and an AutoUpdateBridge; exposes the same public API.

class QmlAutoUpdateDialog
{
public:
  enum class UpdateChoice
  {
    Skip,
    Later,
    Now
  };
  using update_choice_callback_t = std::function<void(UpdateChoice)>;

  QmlAutoUpdateDialog(std::shared_ptr<unfold::UpdateInfo> info, update_choice_callback_t callback);
  ~QmlAutoUpdateDialog();

  void show();
  void raise();
  void close();

  void set_progress_visible(bool visible);
  void set_stage(unfold::UpdateStage stage, double progress);
  void set_status(const std::string &status);
  void start_install();

private:
  update_choice_callback_t callback_;
  QQuickView *view_{nullptr};
  AutoUpdateBridge *bridge_{nullptr};
  std::optional<unfold::UpdateStage> current_stage_;
};

#endif // QML_AUTO_UPDATE_DIALOG_HH
