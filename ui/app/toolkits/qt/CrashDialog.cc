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
#include <clocale>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>

#include "base/logging.h"
#include "handler/handler_main.h"
#include "build/build_config.h"
#include "tools/tool_support.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "utils/Paths.hh"
#include "utils/StringUtils.hh"

namespace
{
  std::string format_hex(uint64_t value)
  {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << value;
    return oss.str();
  }

  std::string format_exception(const crashpad::CrashSummary &summary)
  {
    if (!summary.exception_name.empty())
      {
        return summary.exception_name + "  (" + format_hex(summary.exception_code) + ")";
      }
    return format_hex(summary.exception_code);
  }

  std::string format_address(const crashpad::CrashSummary &summary)
  {
    std::string s = format_hex(summary.exception_address);
    if (!summary.module_name.empty())
      {
        s += "  (" + summary.module_name + " + " + format_hex(summary.module_offset) + ")";
      }
    return s;
  }

  std::string format_thread(const crashpad::CrashSummary &summary)
  {
    std::string tid = "tid: " + std::to_string(summary.crashing_thread_id);
    if (!summary.crashing_thread_name.empty())
      {
        return summary.crashing_thread_name + "  (" + tid + ")";
      }
    return tid;
  }

  auto qtr(const char *text) -> QString
  {
    return QCoreApplication::translate("CrashDialog", text);
  }
} // namespace

CrashDetailsDialog::CrashDetailsDialog(const std::vector<base::FilePath> &attachments,
                                       const crashpad::CrashSummary &summary,
                                       QWidget *parent)
  : QDialog(parent)
  , summary(summary)
{
  setWindowTitle(qtr("Crash details"));
  setMinimumSize(950, 620);

  vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(6, 6, 6, 6);
  vbox->setSpacing(6);

  for (const auto &p: attachments)
    {
      entries.push_back({p, true});
    }

  auto *splitter = new QSplitter(Qt::Horizontal, this);
  vbox->addWidget(splitter, 1);

  tree_widget = new QTreeWidget();
  tree_widget->setHeaderHidden(true);
  tree_widget->setColumnCount(1);

  auto *crash_info_item = new QTreeWidgetItem(tree_widget);
  crash_info_item->setText(0, qtr("Crash Info"));
  crash_info_item->setData(0, Qt::UserRole, 0);
  crash_info_item->setData(0, Qt::UserRole + 1, -1);

  for (int i = 0; i < static_cast<int>(entries.size()); ++i)
    {
      auto *item = new QTreeWidgetItem(tree_widget);
      item->setText(0, QString::fromStdString(workrave::utils::utf16_to_utf8(entries[i].path.BaseName().value())));
      item->setData(0, Qt::UserRole, 2);
      item->setData(0, Qt::UserRole + 1, i);
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      item->setCheckState(0, Qt::Checked);
    }

  content_view = new QTextEdit();
  content_view->setReadOnly(true);
  content_view->setLineWrapMode(QTextEdit::NoWrap);
  QFont fixed_font("Monospace");
  fixed_font.setStyleHint(QFont::TypeWriter);
  content_view->setFont(fixed_font);

  splitter->addWidget(tree_widget);
  splitter->addWidget(content_view);
  splitter->setStretchFactor(1, 1);
  splitter->setSizes({200, 730});

  auto *close_button = new QPushButton(qtr("Close"), this);
  connect(close_button, &QPushButton::clicked, this, &QDialog::accept);
  vbox->addWidget(close_button);

  connect(tree_widget, &QTreeWidget::itemChanged, this, &CrashDetailsDialog::on_attachment_toggled);
  connect(tree_widget, &QTreeWidget::currentItemChanged, this, &CrashDetailsDialog::on_selection_changed);

  tree_widget->setCurrentItem(crash_info_item);
  display_crash_info();
}

void
CrashDetailsDialog::on_attachment_toggled(QTreeWidgetItem *item, int column)
{
  if (item == nullptr || column != 0 || item->data(0, Qt::UserRole).toInt() != 2)
    {
      return;
    }

  int index = item->data(0, Qt::UserRole + 1).toInt();
  if (index >= 0 && index < static_cast<int>(entries.size()))
    {
      entries[index].enabled = item->checkState(0) == Qt::Checked;
    }
}

void
CrashDetailsDialog::on_selection_changed(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
  (void)previous;

  if (current == nullptr)
    {
      return;
    }

  if (current->data(0, Qt::UserRole).toInt() == 2)
    {
      load_content(current->data(0, Qt::UserRole + 1).toInt());
    }
  else
    {
      display_crash_info();
    }
}

void
CrashDetailsDialog::display_crash_info()
{
  content_view->clear();

  auto cursor = content_view->textCursor();
  QTextCharFormat normal_format;
  QTextCharFormat bold_format;
  bold_format.setFontWeight(QFont::Bold);

  auto append_kv = [&](const QString &key, const QString &value) {
    cursor.insertText(key, bold_format);
    cursor.insertText("  " + value + "\n", normal_format);
  };

  append_kv(qtr("Exception:"), QString::fromStdString(format_exception(summary)));
  append_kv(qtr("Address:  "), QString::fromStdString(format_address(summary)));
  append_kv(qtr("Thread:   "), QString::fromStdString(format_thread(summary)));

  cursor.insertText("\n", normal_format);
  cursor.insertText(qtr("Stack Trace:\n"), bold_format);
  if (summary.stack_frames.empty())
    {
      cursor.insertText(qtr("(not available)\n"), normal_format);
    }
  else
    {
      int frame_num = 0;
      for (const auto &[addr, sym]: summary.stack_frames)
        {
          std::ostringstream line;
          line << "#" << std::setw(2) << std::left << frame_num++ << "  " << format_hex(addr);
          if (!sym.empty())
            {
              line << "  " << sym;
            }
          line << "\n";
          cursor.insertText(QString::fromStdString(line.str()), normal_format);
        }
    }
}

void
CrashDetailsDialog::load_content(int index)
{
  content_view->clear();
  if (index < 0 || index >= static_cast<int>(entries.size()))
    {
      return;
    }

  const auto &path = entries[index].path;
  std::ifstream f(workrave::utils::utf16_to_utf8(path.value()).c_str());
  if (f.is_open())
    {
      std::ostringstream content;
      std::string line;
      while (std::getline(f, line))
        {
          content << line << '\n';
        }
      content_view->setPlainText(QString::fromStdString(content.str()));
    }
  else
    {
      content_view->setPlainText(qtr("(file not found or not readable)"));
    }
}

std::vector<base::FilePath>
CrashDetailsDialog::get_enabled_attachments() const
{
  std::vector<base::FilePath> result;
  for (const auto &entry: entries)
    {
      if (entry.enabled)
        {
          result.push_back(entry.path);
        }
    }
  return result;
}

QVBoxLayout *
create_indented_box(QVBoxLayout *container)
{
  auto *ibox = new QHBoxLayout();
  container->addLayout(ibox);

  auto *indent_lab = new QLabel("    ");
  ibox->addWidget(indent_lab);

  auto *box = new QVBoxLayout();
  ibox->addLayout(box);
  box->setSpacing(6);
  return box;
}

CrashDialog::CrashDialog(const std::map<std::string, std::string> &annotations,
                         const std::vector<base::FilePath> &attachments,
                         const crashpad::CrashSummary &summary,
                         QWidget *parent)
  : QDialog(parent)
  , details_dlg(new CrashDetailsDialog(attachments, summary, this))
{
  (void)annotations;
  setWindowTitle(qtr("Workrave crash reporter"));
  setMinimumSize(600, 420);

  vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);

  auto *header_frame = new QFrame(this);
  header_frame->setStyleSheet("QFrame { background: #fde8e8; color: #1a1a1a; }");
  auto *header_layout = new QVBoxLayout(header_frame);
  header_layout->setContentsMargins(12, 12, 12, 12);
  header_layout->setSpacing(6);

  auto *title_label = new QLabel(qtr("<span style=\"font-size: large; font-weight: bold;\">Workrave has crashed.</span>"), this);
  header_layout->addWidget(title_label);

  auto *info_label = new QLabel(
    qtr("Workrave encountered a problem and crashed. Please help us diagnose and fix this problem by sending a crash report."),
    this);
  info_label->setWordWrap(true);
  header_layout->addWidget(info_label);
  vbox->addWidget(header_frame);

  auto *content_box = new QWidget(this);
  auto *content_layout = new QVBoxLayout(content_box);
  content_layout->setContentsMargins(12, 12, 12, 12);
  content_layout->setSpacing(8);
  vbox->addWidget(content_box, 1);

  submit_cb = new QCheckBox(qtr("Submit crash report to the Workrave developers"), this);
  connect(submit_cb, &QCheckBox::toggled, this, &CrashDialog::on_submit_toggled);
  content_layout->addWidget(submit_cb);

  QVBoxLayout *ibox = create_indented_box(content_layout);

  auto *details_btn = new QPushButton(qtr("Details..."), this);
  connect(details_btn, &QPushButton::clicked, this, &CrashDialog::on_details_clicked);
  auto *details_row = new QHBoxLayout;
  details_row->addWidget(details_btn);
  details_row->addStretch();
  ibox->addLayout(details_row);

  auto *details_hint = new QLabel(qtr("<small><i>Shows the crash information and files that will be submitted.</i></small>"),
                                  this);
  ibox->addWidget(details_hint);

  auto *separator = new QFrame(this);
  separator->setFrameShape(QFrame::HLine);
  separator->setFrameShadow(QFrame::Sunken);
  ibox->addWidget(separator);

  auto *user_text_label = new QLabel(qtr("Additional comments (optional):"), this);
  ibox->addWidget(user_text_label);

  user_text_frame = new QFrame(this);
  user_text_frame->setFrameShape(QFrame::StyledPanel);
  ibox->addWidget(user_text_frame);

  auto *frame_layout = new QVBoxLayout(user_text_frame);
  text_edit = new QTextEdit(this);
  frame_layout->addWidget(text_edit);

  auto *button_box = new QDialogButtonBox(QDialogButtonBox::Close, this);
  button_box->setContentsMargins(8, 8, 8, 8);
  connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::accept);
  connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
  vbox->addWidget(button_box);

  submit_cb->setChecked(true);
  on_submit_toggled();
  text_edit->setFocus();
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
  QRect parent_geometry = geometry();
  QSize detail_size = details_dlg->size();
  details_dlg->move(parent_geometry.x() + (parent_geometry.width() - detail_size.width()) / 2,
                    parent_geometry.y() + (parent_geometry.height() - detail_size.height()) / 2);
  details_dlg->raise();
  details_dlg->activateWindow();
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

std::vector<base::FilePath>
CrashDialog::get_selected_attachments() const
{
  return details_dlg->get_enabled_attachments();
}

bool
UserInteraction::requestUserConsent(const std::map<std::string, std::string> &annotations,
                                    std::vector<base::FilePath> &attachments,
                                    const crashpad::CrashSummary &summary)
{
  SetEnvironmentVariableA("GTK_DEBUG", nullptr);
  SetEnvironmentVariableA("G_MESSAGES_DEBUG", nullptr);
  SetEnvironmentVariableA("GTK_OVERLAY_SCROLLING", "0");
  SetEnvironmentVariableA("GTK_CSD", "0");
  SetEnvironmentVariableA("GDK_WIN32_DISABLE_HIDPI", "1");

  LOG(INFO) << "Creating user consent app.";
  int argc = 0;
  char **argv = nullptr;
  QApplication app(argc, argv);

  LOG(INFO) << "Creating user consent dialog.";
  CrashDialog dlg(annotations, attachments, summary);
  dlg.exec();

  user_text = dlg.get_user_text();
  consent = dlg.get_consent();
  attachments = dlg.get_selected_attachments();

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
    const std::filesystem::path log_dir = workrave::utils::Paths::get_log_directory();
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((log_dir / "workrave-crashhandler.log").string(),
                                                                            1024 * 1024,
                                                                            5,
                                                                            true);
    auto logger = std::make_shared<spdlog::logger>("crashhandler", file_sink);
    logger->flush_on(spdlog::level::critical);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");

    logging::SetLogMessageHandler(
      [](logging::LogSeverity severity, const char *file_path, int line, size_t message_start, const std::string &string) {
        (void)file_path;
        (void)line;

        std::string msg = string.substr(message_start);
        if (!msg.empty() && msg.back() == '\n')
          {
            msg.pop_back();
          }
        switch (severity)
          {
          case logging::LOG_VERBOSE:
            spdlog::debug("Crashpad: {}", msg);
            break;
          case logging::LOG_INFO:
            spdlog::info("Crashpad: {}", msg);
            break;
          case logging::LOG_WARNING:
            spdlog::warn("Crashpad: {}", msg);
            break;
          case logging::LOG_ERROR:
            spdlog::error("Crashpad: {}", msg);
            break;
          case logging::LOG_FATAL:
            spdlog::critical("Crashpad: {}", msg);
            break;
          default:
            spdlog::trace("Crashpad: {}", msg);
            break;
          }
        return false;
      });

    logging::SetMinLogLevel(logging::LOG_VERBOSE);

    LOG(INFO) << "Workrave crashed.";
    auto *user_interaction = new UserInteraction;
    int ret = crashpad::HandlerMain(argc, argv, nullptr, user_interaction);
    LOG(INFO) << "Crash handled";
    delete user_interaction;
    LOG(INFO) << "Exit:" << ret;
    spdlog::shutdown();
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
