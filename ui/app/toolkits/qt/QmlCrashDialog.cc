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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "QmlCrashDialog.hh"

#include <fstream>
#include <iomanip>
#include <sstream>

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QLibraryInfo>
#include <QObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QString>
#include <QVariantMap>

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
} // namespace

// ── CrashBridge ───────────────────────────────────────────────────────────────

CrashBridge::CrashBridge(QObject *parent)
  : QObject(parent)
{
}

void
CrashBridge::setSubmitEnabled(bool v)
{
  if (submit_enabled_ != v)
    {
      submit_enabled_ = v;
      Q_EMIT submitEnabledChanged();
    }
}

// ── CrashDetailsBridge ────────────────────────────────────────────────────────

CrashDetailsBridge::CrashDetailsBridge(const std::vector<base::FilePath> &attachments,
                                       const crashpad::CrashSummary &summary,
                                       QObject *parent)
  : QObject(parent)
  , summary_(summary)
{
  QVariantMap crash_info_item;
  crash_info_item["name"] = QObject::tr("Crash Info");
  crash_info_item["isAttachment"] = false;
  crash_info_item["enabled"] = true;
  crash_info_item["attachIndex"] = -1;
  items_.append(crash_info_item);

  int idx = 0;
  for (const auto &p: attachments)
    {
      entries_.push_back({p, true});
      QVariantMap item;
      item["name"] = QString::fromStdString(workrave::utils::utf16_to_utf8(p.BaseName().value()));
      item["isAttachment"] = true;
      item["enabled"] = true;
      item["attachIndex"] = idx++;
      items_.append(item);
    }

  showCrashInfo();
}

void
CrashDetailsBridge::selectItem(int index)
{
  if (index < 0 || index >= static_cast<int>(items_.size()))
    {
      return;
    }

  selected_index_ = index;
  Q_EMIT selectedIndexChanged();

  if (index == 0)
    {
      showCrashInfo();
    }
  else
    {
      showFileContent(index - 1);
    }
}

void
CrashDetailsBridge::toggleAttachment(int attachment_index, bool enabled)
{
  if (attachment_index < 0 || attachment_index >= static_cast<int>(entries_.size()))
    {
      return;
    }

  entries_[attachment_index].enabled = enabled;

  int list_index = attachment_index + 1;
  auto item = items_[list_index].toMap();
  item["enabled"] = enabled;
  items_[list_index] = item;
  Q_EMIT itemsChanged();
}

std::vector<base::FilePath>
CrashDetailsBridge::getEnabledAttachments() const
{
  std::vector<base::FilePath> result;
  for (const auto &entry: entries_)
    {
      if (entry.enabled)
        {
          result.push_back(entry.path);
        }
    }
  return result;
}

void
CrashDetailsBridge::showCrashInfo()
{
  selected_content_ = formatCrashInfoHtml();
  Q_EMIT selectedContentChanged();
}

void
CrashDetailsBridge::showFileContent(int attachment_index)
{
  if (attachment_index < 0 || attachment_index >= static_cast<int>(entries_.size()))
    {
      return;
    }

  const auto &path = entries_[attachment_index].path;
  std::ifstream f(workrave::utils::utf16_to_utf8(path.value()).c_str());
  QString content;
  if (f.is_open())
    {
      std::ostringstream oss;
      std::string line;
      while (std::getline(f, line))
        {
          oss << line << '\n';
        }
      content = QString::fromStdString(oss.str()).toHtmlEscaped().replace('\n', "<br/>");
    }
  else
    {
      content = QObject::tr("(file not found or not readable)");
    }

  selected_content_ = content;
  Q_EMIT selectedContentChanged();
}

QString
CrashDetailsBridge::formatCrashInfoHtml() const
{
  auto kv = [](const QString &key, const QString &value) -> QString {
    return QStringLiteral("<b>") + key.toHtmlEscaped() + QStringLiteral("</b>&nbsp;&nbsp;") + value.toHtmlEscaped()
           + QStringLiteral("<br/>");
  };

  QString html;
  html += kv(QObject::tr("Exception:"), QString::fromStdString(format_exception(summary_)));
  html += kv(QObject::tr("Address:  "), QString::fromStdString(format_address(summary_)));
  html += kv(QObject::tr("Thread:   "), QString::fromStdString(format_thread(summary_)));
  html += QStringLiteral("<br/>");
  html += QStringLiteral("<b>") + QObject::tr("Stack Trace:").toHtmlEscaped() + QStringLiteral("</b><br/>");

  if (summary_.stack_frames.empty())
    {
      html += QObject::tr("(not available)").toHtmlEscaped() + QStringLiteral("<br/>");
    }
  else
    {
      int frame_num = 0;
      for (const auto &[addr, sym]: summary_.stack_frames)
        {
          std::ostringstream line;
          line << "#" << std::setw(2) << std::left << frame_num++ << "  " << format_hex(addr);
          if (!sym.empty())
            {
              line << "  " << sym;
            }
          html += QString::fromStdString(line.str()).toHtmlEscaped() + QStringLiteral("<br/>");
        }
    }

  return html;
}

// ── QmlCrashDialog ────────────────────────────────────────────────────────────

QmlCrashDialog::QmlCrashDialog(const std::map<std::string, std::string> & /*annotations*/,
                               const std::vector<base::FilePath> &attachments,
                               const crashpad::CrashSummary &summary)
{
  bridge_ = new CrashBridge;
  details_bridge_ = new CrashDetailsBridge(attachments, summary);

  view_ = new QQuickView;
  view_->setTitle(QObject::tr("Workrave crash reporter"));
  view_->setResizeMode(QQuickView::SizeRootObjectToView);
  view_->setMinimumSize(QSize(600, 420));
  view_->resize(600, 420);

#ifdef QT_QML_IMPORT_PATH
  view_->engine()->addImportPath(QStringLiteral(QT_QML_IMPORT_PATH));
#endif
  view_->engine()->addImportPath(QLibraryInfo::path(QLibraryInfo::QmlImportsPath));

  view_->rootContext()->setContextProperty("crashBridge", bridge_);
  view_->rootContext()->setContextProperty("detailsBridge", details_bridge_);

  QObject::connect(view_, &QQuickView::statusChanged, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err: view_->errors())
          {
            qWarning("CrashDialog QML error: %s", qPrintable(err.toString()));
          }
      }
  });

  view_->setSource(QUrl(QStringLiteral("qrc:/crash/CrashDialog.qml")));
}

QmlCrashDialog::~QmlCrashDialog()
{
  delete view_;
  delete details_bridge_;
  delete bridge_;
}

int
QmlCrashDialog::exec()
{
  QEventLoop loop;
  QObject::connect(bridge_, &CrashBridge::closeRequested, &loop, &QEventLoop::quit);
  view_->show();
  view_->raise();
  loop.exec();
  view_->hide();
  return 0;
}

std::string
QmlCrashDialog::getUserText() const
{
  return bridge_->userText().toStdString();
}

bool
QmlCrashDialog::getConsent() const
{
  return bridge_->submitEnabled();
}

std::vector<base::FilePath>
QmlCrashDialog::getSelectedAttachments() const
{
  return details_bridge_->getEnabledAttachments();
}
