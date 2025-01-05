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
  explicit CrashDetailsDialog(const std::vector<base::FilePath> &attachments, QWidget *parent = nullptr);
  ~CrashDetailsDialog() override = default;

private:
  QVBoxLayout *vbox{nullptr};
  QScrollArea *scroll_area{nullptr};
  QTextEdit *text_edit{nullptr};
};

class CrashDialog : public QDialog
{
public:
  CrashDialog(const std::map<std::string, std::string> &annotations,
              const std::vector<base::FilePath> &attachments,
              QWidget *parent = nullptr);
  ~CrashDialog() override = default;

  std::string get_user_text() const;
  bool get_consent() const;

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
                          const std::vector<base::FilePath> &attachments) override;
  std::string getUserText() override;
  void reportCompleted(const crashpad::UUID &uuid) override;

private:
  std::string user_text;
  bool consent{false};
};

#endif // CRASH_DIALOG_HH
