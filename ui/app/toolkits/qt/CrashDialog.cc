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

#include "CrashDialog.hh"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string>
#include <fstream>
#include <streambuf>
#include <filesystem>
#include <clocale>
#include <locale>

#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>

#include "base/logging.h"
#include "handler/handler_main.h"
#include "build/build_config.h"
#include "tools/tool_support.h"

#include "commonui/nls.h"
#include "utils/StringUtils.hh"

CrashDetailsDialog::CrashDetailsDialog(const std::vector<base::FilePath> &attachments, QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle("Crash report details");
  setMinimumSize(600, 400);

  vbox = new QVBoxLayout(this);

  QLabel *info_label = new QLabel("<b>Workrave has crashed.</b>", this);
  vbox->addWidget(info_label);

  if (!attachments.empty())
    {
      QLabel *attachments_label = new QLabel("The following logging will be attached to the crash report:", this);
      vbox->addWidget(attachments_label);

      scroll_area = new QScrollArea(this);
      scroll_area->setWidgetResizable(true);
      vbox->addWidget(scroll_area);

      text_edit = new QTextEdit(this);
      text_edit->setReadOnly(true);
      scroll_area->setWidget(text_edit);

      for (const auto &a: attachments)
        {
          std::ifstream f(workrave::utils::utf16_to_utf8(a.value()).c_str());
          if (f.is_open())
            {
              text_edit->append(QString::fromStdString(workrave::utils::utf16_to_utf8(a.BaseName().value()) + ":\n\n"));

              std::string line;
              while (std::getline(f, line))
                {
                  text_edit->append(QString::fromStdString(line));
                }
              text_edit->append("\n");
            }
        }
    }

  QLabel *more_info_label = new QLabel(
    "Note that this crash report also contains technical information about the state of Workrave when it crashed.",
    this);
  vbox->addWidget(more_info_label);

  QPushButton *close_button = new QPushButton("Close", this);
  connect(close_button, &QPushButton::clicked, this, &QDialog::accept);
  vbox->addWidget(close_button);
}

QVBoxLayout *
create_indented_box(QVBoxLayout *container)
{
  QHBoxLayout *ibox = new QHBoxLayout();
  container->addLayout(ibox);

  QLabel *indent_lab = new QLabel("    ");
  ibox->addWidget(indent_lab);

  QVBoxLayout *box = new QVBoxLayout();
  ibox->addLayout(box);
  box->setSpacing(6);
  return box;
}

CrashDialog::CrashDialog(const std::map<std::string, std::string> &annotations,
                         const std::vector<base::FilePath> &attachments,
                         QWidget *parent)
  : QDialog(parent)
  , details_dlg(new CrashDetailsDialog(attachments, this))
{
  setWindowTitle("Workrave crash reporter");
  setMinimumSize(600, 400);

  vbox = new QVBoxLayout(this);

  QLabel *title_label = new QLabel("<b>Workrave has crashed.</b>", this);
  vbox->addWidget(title_label);

  QLabel *info_label = new QLabel(
    "Workrave encountered a problem and crashed. Please help us to diagnose and fix this problem by sending a crash report.",
    this);
  info_label->setWordWrap(true);
  vbox->addWidget(info_label);

  submit_cb = new QCheckBox("Submit crash report to the Workrave developers", this);
  connect(submit_cb, &QCheckBox::toggled, this, &CrashDialog::on_submit_toggled);
  vbox->addWidget(submit_cb);

  QVBoxLayout *ibox = create_indented_box(vbox);

  QPushButton *details_btn = new QPushButton("Details...", this);
  connect(details_btn, &QPushButton::clicked, this, &CrashDialog::on_details_clicked);
  ibox->addWidget(details_btn);

  user_text_frame = new QFrame(this);
  user_text_frame->setFrameShape(QFrame::StyledPanel);
  ibox->addWidget(user_text_frame);

  QVBoxLayout *frame_layout = new QVBoxLayout(user_text_frame);
  text_edit = new QTextEdit(this);
  frame_layout->addWidget(text_edit);

  QPushButton *close_button = new QPushButton("Close", this);
  connect(close_button, &QPushButton::clicked, this, &QDialog::accept);
  vbox->addWidget(close_button);

  submit_cb->setChecked(true);
  on_submit_toggled();
}

void
CrashDialog::on_submit_toggled()
{
  bool enabled = submit_cb->isChecked();
  user_text_frame->setEnabled(enabled);
}

void
CrashDialog::on_details_clicked()
{
  details_dlg->show();
}

std::string
CrashDialog::get_user_text() const
{
  return text_edit->toPlainText().toStdString();
}

bool
CrashDialog::get_consent() const
{
  return submit_cb->isChecked();
}

bool
UserInteraction::requestUserConsent(const std::map<std::string, std::string> &annotations,
                                    const std::vector<base::FilePath> &attachments)
{
  SetEnvironmentVariableA("GTK_DEBUG", 0);
  SetEnvironmentVariableA("G_MESSAGES_DEBUG", 0);
  SetEnvironmentVariableA("GTK_OVERLAY_SCROLLING", "0");
  SetEnvironmentVariableA("GTK_CSD", "0");
  SetEnvironmentVariableA("GDK_WIN32_DISABLE_HIDPI", "1");

  LOG(INFO) << "Creating user consent app.";
  int argc = 0;
  char **argv = nullptr;
  QApplication app(argc, argv);

  LOG(INFO) << "Creating user consent dialog.";
  CrashDialog dlg(annotations, attachments);
  dlg.exec();

  user_text = dlg.get_user_text();
  consent = dlg.get_consent();

  LOG(INFO) << "User consent complete:" << consent;
  return consent;
}

std::string
UserInteraction::getUserText()
{
  return user_text;
}

void
UserInteraction::reportCompleted(const crashpad::UUID &uuid)
{
  LOG(INFO) << "Report files as: " << uuid.ToString();
}

namespace
{
  int HandlerMainAdaptor(int argc, char *argv[])
  {
    LOG(INFO) << "Workrave crashed.";
    auto *user_interaction = new UserInteraction;
    int ret = crashpad::HandlerMain(argc, argv, nullptr, user_interaction);
    LOG(INFO) << "Crash handled";
    delete user_interaction;
    LOG(INFO) << "Exit:" << ret;
    return ret;
  }
} // namespace

int APIENTRY
wWinMain(HINSTANCE, HINSTANCE, wchar_t *, int)
{
  return crashpad::ToolSupport::Wmain(__argc, __wargv, HandlerMainAdaptor);
}

int
wmain(int argc, wchar_t *argv[])
{
  return crashpad::ToolSupport::Wmain(argc, argv, HandlerMainAdaptor);
}
