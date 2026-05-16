// Copyright (C) 2020-2021 Rob Caelers <robc@krandor.nl>
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

#ifndef CRASH_DIALOG_HH
#define CRASH_DIALOG_HH

#include <string>

#include <QtGui>
#include <QtWidgets>

#include "handler/user_hook.h"

class CrashDetailsDialog : public QDialog
{
public:
  CrashDetailsDialog(const std::vector<base::FilePath> &attachments,
                     const crashpad::CrashSummary &summary,
                     QWidget *parent = nullptr);
  ~CrashDetailsDialog() override = default;

  std::vector<base::FilePath> get_enabled_attachments() const;

private:
  void on_attachment_toggled(QTreeWidgetItem *item, int column);
  void on_selection_changed(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  void load_content(int index);
  void display_crash_info();

  struct AttachmentEntry
  {
    base::FilePath path;
    bool enabled{true};
  };

  QVBoxLayout *vbox{nullptr};
  QTreeWidget *tree_widget{nullptr};
  QTextEdit *content_view{nullptr};
  std::vector<AttachmentEntry> entries;
  crashpad::CrashSummary summary;
};

class CrashDialog : public QDialog
{
public:
  CrashDialog(const std::map<std::string, std::string> &annotations,
              const std::vector<base::FilePath> &attachments,
              const crashpad::CrashSummary &summary,
              QWidget *parent = nullptr);
  ~CrashDialog() override = default;

  std::string get_user_text() const;
  bool get_consent() const;
  std::vector<base::FilePath> get_selected_attachments() const;

private:
  void on_submit_toggled();
  void on_details_clicked();

private:
  QTextEdit *text_edit{nullptr};
  QVBoxLayout *vbox{nullptr};
  CrashDetailsDialog *details_dlg{nullptr};
  QCheckBox *submit_cb{nullptr};
  QFrame *user_text_frame{nullptr};
};

class UserInteraction : public crashpad::UserHook
{
public:
  UserInteraction() = default;
  ~UserInteraction() override = default;

  bool requestUserConsent(const std::map<std::string, std::string> &annotations,
                          std::vector<base::FilePath> &attachments,
                          const crashpad::CrashSummary &summary) override;
  std::string getUserText() override;
  void reportCompleted(const crashpad::UUID &uuid) override;

private:
  std::string user_text;
  bool consent{false};
};

#endif // CRASH_DIALOG_HH
