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

#ifndef QML_CRASH_DIALOG_HH
#define QML_CRASH_DIALOG_HH

#include <map>
#include <string>
#include <vector>

#include <QObject>
#include <QQuickView>
#include <QString>
#include <QVariantList>

#include "base/files/file_path.h"
#include "handler/user_hook.h"

// ── CrashBridge ───────────────────────────────────────────────────────────────
// Exposes the main crash dialog state and user actions to QML.

class CrashBridge : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool submitEnabled READ submitEnabled NOTIFY submitEnabledChanged)

public:
  explicit CrashBridge(QObject *parent = nullptr);

  bool submitEnabled() const { return submit_enabled_; }
  QString userText() const { return user_text_; }

  Q_INVOKABLE void setSubmitEnabled(bool v);
  Q_INVOKABLE void setUserText(const QString &v) { user_text_ = v; }
  Q_INVOKABLE void emitClose() { Q_EMIT closeRequested(); }

Q_SIGNALS:
  void submitEnabledChanged();
  void closeRequested();

private:
  bool submit_enabled_{true};
  QString user_text_;
};

// ── CrashDetailsBridge ────────────────────────────────────────────────────────
// Exposes the crash details (attachment list + content viewer) to QML.

class CrashDetailsBridge : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)
  Q_PROPERTY(QString selectedContent READ selectedContent NOTIFY selectedContentChanged)
  Q_PROPERTY(int selectedIndex READ selectedIndex NOTIFY selectedIndexChanged)

public:
  explicit CrashDetailsBridge(const std::vector<base::FilePath> &attachments,
                               const crashpad::CrashSummary &summary,
                               QObject *parent = nullptr);

  QVariantList items() const { return items_; }
  QString selectedContent() const { return selected_content_; }
  int selectedIndex() const { return selected_index_; }

  Q_INVOKABLE void selectItem(int index);
  Q_INVOKABLE void toggleAttachment(int attachment_index, bool enabled);

  std::vector<base::FilePath> getEnabledAttachments() const;

Q_SIGNALS:
  void itemsChanged();
  void selectedContentChanged();
  void selectedIndexChanged();

private:
  void showCrashInfo();
  void showFileContent(int attachment_index);
  QString formatCrashInfoHtml() const;

  struct AttachmentEntry
  {
    base::FilePath path;
    bool enabled{true};
  };

  QVariantList items_;
  QString selected_content_;
  int selected_index_{0};
  std::vector<AttachmentEntry> entries_;
  crashpad::CrashSummary summary_;
};

// ── QmlCrashDialog ────────────────────────────────────────────────────────────
// Replaces the old QDialog-based CrashDialog. Owns a QQuickView and the two
// bridge objects; exec() blocks until the user closes the dialog.

class QmlCrashDialog
{
public:
  QmlCrashDialog(const std::map<std::string, std::string> &annotations,
                 const std::vector<base::FilePath> &attachments,
                 const crashpad::CrashSummary &summary);
  ~QmlCrashDialog();

  int exec();

  std::string getUserText() const;
  bool getConsent() const;
  std::vector<base::FilePath> getSelectedAttachments() const;

private:
  CrashBridge *bridge_{nullptr};
  CrashDetailsBridge *details_bridge_{nullptr};
  QQuickView *view_{nullptr};
};

#endif // QML_CRASH_DIALOG_HH
