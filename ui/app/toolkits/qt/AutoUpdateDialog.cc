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
//

#define WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "AutoUpdateDialog.hh"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <fmt/os.h>

#include "cmark.h"

#include "ui/GUIConfig.hh"

#ifdef PLATFORM_OS_WINDOWS
#  include "ToolkitWindows.hh"
#endif

static constexpr const char *doc =
  R"(<!DOCTYPE html>
<html ang="en">
<head>
  <meta charset="utf-8">
{}
</head>
<body>
  <div>
    {}
  </div>
</body>
</html>)";

static constexpr const char *lightstyle =
  R"(<style type="text/css">
body {
  color: #222;
  background: #fff;
}
a {
  color: #224ba0;
}
</style>
)";

static constexpr const char *darkstyle =
  R"(<style type="text/css">
body {
  color: #eee;
  background: #2d2d2d;
}
body a {
  color: #224ba0;
}
</style>
)";

AutoUpdateDialog::AutoUpdateDialog(std::shared_ptr<unfold::UpdateInfo> info, AutoUpdateDialog::update_choice_callback_t callback)
  : callback(std::move(callback))
{
  setWindowTitle(tr("Software Update"));
  //  resize(800, 600);
  setMinimumSize(800, 600);

  auto *main_layout = new QVBoxLayout();
  setLayout(main_layout);

  auto *content_area = new QHBoxLayout();
  main_layout->addLayout(content_area);

  auto *logo_box = new QVBoxLayout();
  content_area->addLayout(logo_box);

  try
    {
      auto pix = QPixmap(":/workrave/workrave.png");
      auto *logo = new QLabel();
      logo->setPixmap(pix);
      logo_box->addWidget(logo);
    }
  catch (const std::exception &e)
    {
      spdlog::info("error loading image {}", std::string(e.what()));
    }

  auto *update_info_box = new QVBoxLayout();
  content_area->addLayout(update_info_box);

  QString bold = "<b>";
  QString end = "</b>";

  auto *title_label = new QLabel(bold + tr("A new version of %1 is available").arg(QString::fromStdString(info->title)) + end);
  update_info_box->addWidget(title_label);

  auto *info_label = new QLabel(tr("%1 %2 is now available -- you have %3. Would you like to download it now?")
                                  .arg(QString::fromStdString(info->title))
                                  .arg(QString::fromStdString(info->version))
                                  .arg(QString::fromStdString(info->current_version)));
  info_label->setWordWrap(true);
  update_info_box->addWidget(info_label);

  auto *notes_label = new QLabel(bold + tr("Release notes") + end);
  update_info_box->addWidget(notes_label);

  auto *notes_frame = new QFrame();
  notes_frame->setFrameShadow(QFrame::Sunken);
  notes_frame->setFrameShape(QFrame::StyledPanel);
  update_info_box->addWidget(notes_frame);

  auto *notes_box = new QVBoxLayout(notes_frame);

  web = new QTextBrowser();
  web->setOpenExternalLinks(true);

  std::string body;
  for (const auto &note: info->release_notes)
    {
      body += fmt::format(fmt::runtime(tr("<h3>Version {}</h3>\n").toStdString()), note.version);
      auto *html = cmark_markdown_to_html(note.markdown.c_str(), note.markdown.length(), CMARK_OPT_DEFAULT);

      if (html != nullptr)
        {
          spdlog::info("body: {}", html);
          body += html;
          free(html);
        }
    }

  bool dark = false;
  switch (GUIConfig::light_dark_mode()())
    {
    case LightDarkTheme::Light:
      dark = false;
      break;
    case LightDarkTheme::Dark:
      dark = true;
      break;
    case LightDarkTheme::Auto:
#ifdef PLATFORM_OS_WINDOWS
      dark = ToolkitWindows::is_windows_app_theme_dark();
#endif
      break;
    }

  web->setHtml(QString::fromStdString(fmt::format(doc, dark ? darkstyle : lightstyle, body)));
  notes_box->addWidget(web);

  auto *status_box = new QHBoxLayout();
  main_layout->addLayout(status_box);

  status_label = new QLabel();
  status_box->addWidget(status_label);

  progress_bar_frame = new QFrame();
  progress_bar_frame->setFrameShadow(QFrame::Sunken);
  progress_bar_frame->setFrameShape(QFrame::StyledPanel);
  auto *progress_bar_box = new QVBoxLayout(progress_bar_frame);

  status_box->addWidget(progress_bar_frame);

  progress_bar = new QProgressBar();
  progress_bar->setOrientation(Qt::Horizontal);
  progress_bar_box->addWidget(progress_bar);

  auto *bottom_box = new QHBoxLayout();
  main_layout->addLayout(bottom_box);

  left_button_box = new QHBoxLayout();
  bottom_box->addLayout(left_button_box);

  right_button_box = new QHBoxLayout();
  bottom_box->addLayout(right_button_box);

  close_button_box = new QHBoxLayout();
  bottom_box->addLayout(close_button_box);

  skip_button = new QPushButton(tr("Skip this version"));
  connect(skip_button, &QPushButton::clicked, [callback]() { callback(UpdateChoice::Skip); });
  left_button_box->addWidget(skip_button);

  remind_button = new QPushButton(tr("Remind me later"));
  connect(remind_button, &QPushButton::clicked, [callback]() { callback(UpdateChoice::Later); });
  right_button_box->addWidget(remind_button);

  install_button = new QPushButton(tr("Install update"));
  connect(install_button, &QPushButton::clicked, [callback]() { callback(UpdateChoice::Now); });
  right_button_box->addWidget(install_button);

  close_button = new QPushButton(tr("Close"));
  connect(close_button, &QPushButton::clicked, this, &QDialog::close);
  close_button_box->addWidget(close_button);

  // connect(this, &QDialog::closeEvent, [callback]() { callback(UpdateChoice::Later); });

  install_button->setEnabled(true);
  progress_bar->setVisible(false);
  progress_bar_frame->setVisible(false);
  close_button->setVisible(false);
}

void
AutoUpdateDialog::set_progress_visible(bool visible)
{
  progress_bar->setVisible(visible);
  progress_bar_frame->setVisible(true);
}

void
AutoUpdateDialog::set_stage(unfold::UpdateStage stage, double progress)
{
  if (!current_stage || *current_stage != stage)
    {
      spdlog::info("Update stage: {}", static_cast<int>(stage));
      current_stage = stage;
      progress_bar->setVisible(stage == unfold::UpdateStage::DownloadInstaller);
      progress_bar_frame->setVisible(stage == unfold::UpdateStage::DownloadInstaller);

      switch (stage)
        {
        case unfold::UpdateStage::DownloadInstaller:
          status_label->setText(tr("Downloading installer"));
          break;
        case unfold::UpdateStage::VerifyInstaller:
          status_label->setText(tr("Verifying installer"));
          break;
        case unfold::UpdateStage::RunInstaller:
          status_label->setText(tr("Running installer"));
          break;
        default:
          break;
        }
    }

  if (fabs(progress - (progress_bar->value() / 100.0)) >= 0.01)
    {
      progress_bar->setValue(static_cast<int>(progress * 100));
    }

  if (progress >= 1.0)
    {
      progress_bar->setVisible(false);
      progress_bar_frame->setVisible(false);
    }
}

void
AutoUpdateDialog::set_status(const std::string &status)
{
  spdlog::info("Update status: {}", status);
  status_label->setText(QString::fromStdString(status));
  progress_bar->setVisible(false);
  progress_bar_frame->setVisible(false);
}

void
AutoUpdateDialog::start_install()
{
  status_label->setText("");
  skip_button->setVisible(false);
  remind_button->setVisible(false);
  progress_bar->setVisible(true);
  progress_bar_frame->setVisible(true);
  install_button->setVisible(true);
}
