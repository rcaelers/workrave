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

#ifndef UPDATE_DIALOG_HH
#define UPDATE_DIALOG_HH

#include <optional>
#include <qdialog.h>
#include <string>
#include <memory>

#include <QtGui>
#include <QtWidgets>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "unfold/Unfold.hh"

class AutoUpdateDialog : public QDialog
{
  Q_OBJECT

public:
  enum class UpdateChoice
  {
    Skip,
    Later,
    Now
  };
  using update_choice_callback_t = std::function<void(UpdateChoice)>;

  AutoUpdateDialog(std::shared_ptr<unfold::UpdateInfo> info, update_choice_callback_t callback);
  ~AutoUpdateDialog() override = default;

  void set_progress_visible(bool visible);
  void set_stage(unfold::UpdateStage stage, double progress);
  void set_status(const std::string &status);
  void start_install();

private:
  update_choice_callback_t callback;
  QTextEdit *text_view{nullptr};
  QScrollArea *scrolled_window{nullptr};
  QFrame *progress_bar_frame{nullptr};
  QProgressBar *progress_bar{nullptr};
  QLabel *status_label{nullptr};
  QHBoxLayout *left_button_box{nullptr};
  QHBoxLayout *right_button_box{nullptr};
  QHBoxLayout *close_button_box{nullptr};
  QPushButton *install_button{nullptr};
  QPushButton *close_button{nullptr};
  QPushButton *skip_button{nullptr};
  QPushButton *remind_button{nullptr};

  std::optional<unfold::UpdateStage> current_stage;
  QTextBrowser *web{nullptr};
};

#endif // UPDATE_DIALOG_HH
