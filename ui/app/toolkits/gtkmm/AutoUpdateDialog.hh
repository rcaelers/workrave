// Copyright (C) 2022 Rob Caelers <rob.caelers@gmail.com>
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

#ifndef AUTO_UPDATE_DIALOG_HH
#define AUTO_UPDATE_DIALOG_HH

#include <string>

#if defined(PLATFORM_OS_WINDOWS)
#  include "Edge.hh"
#endif

#if defined(PLATFORM_OS_WINDOWS)
#  pragma push_macro("ERROR")
#  pragma push_macro("IN")
#  pragma push_macro("OUT")
#  pragma push_macro("WINDING")
#  undef ERROR
#  undef IN
#  undef OUT
#  undef WINDING
#endif

#include <gtkmm.h>

#if defined(PLATFORM_OS_WINDOWS)
#  pragma pop_macro("ERROR")
#  pragma pop_macro("IN")
#  pragma pop_macro("OUT")
#  pragma pop_macro("WINDING")
#endif

#include "unfold/Unfold.hh"

class AutoUpdateDialog : public Gtk::Window
{
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
  void set_stage(unfold::UpdateStage, double progress);
  void set_status(std::string status);
  void start_install();

private:
  void on_auto_toggled();

private:
  update_choice_callback_t callback;
  Gtk::TextView *text_view{nullptr};
  Gtk::ScrolledWindow scrolled_window;
  Gtk::Frame *progress_bar_frame{nullptr};
  Gtk::ProgressBar *progress_bar{nullptr};
  Gtk::Label *status_label{nullptr};
  Gtk::Box *left_button_box{nullptr};
  Gtk::Box *right_button_box{nullptr};
  Gtk::Box *close_button_box{nullptr};

  Glib::RefPtr<Gtk::TextBuffer> text_buffer;
  std::optional<unfold::UpdateStage> current_stage;

#if defined(PLATFORM_OS_WINDOWS)
  Edge *web{nullptr};
#endif
};

#endif // AUTO_UPDATE_DIALOG_HH
