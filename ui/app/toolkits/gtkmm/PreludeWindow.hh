// Copyright (C) 2001 - 2017 Rob Caelers & Raymond Penners
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

#ifndef PRELUDEWINDOW_HH
#define PRELUDEWINDOW_HH

#include "BreakWindow.hh"
#include "ui/IPreludeWindow.hh"

class TimeBar;
class Frame;

namespace Gtk
{
  class Image;
  class Label;
} // namespace Gtk

class PreludeWindow
  : public Gtk::Window
  , public IPreludeWindow
{
public:
  PreludeWindow(HeadInfo head, workrave::BreakId break_id);
  ~PreludeWindow() override = default;

  void start() override;
  void stop() override;
  void refresh() override;
  void set_progress(int value, int max_value) override;
  void set_stage(workrave::IApp::PreludeStage stage) override;
  void set_progress_text(workrave::IApp::PreludeProgressText text) override;

private:
  void on_frame_flash_event(bool frame_visible);
  void add(Gtk::Widget &widget) override;

  bool on_enter_notify_event(GdkEventCrossing *event) override;
  void avoid_pointer();

  bool on_draw_event(const ::Cairo::RefPtr<::Cairo::Context> &cr);
  void on_screen_changed_event(const Glib::RefPtr<Gdk::Screen> &previous_screen);
  void update_input_region(Gtk::Allocation &allocation);
  void on_size_allocate_event(Gtk::Allocation &allocation);

  Gdk::Color rgba_to_color(const Gdk::RGBA &rgba);

private:
  //! Avoid margin.
  const int SCREEN_MARGIN{20};

  //! Did we avoid the pointer?
  bool did_avoid{false};

  //! Time bar
  TimeBar *time_bar{nullptr};

  //! Frame
  Frame *frame{nullptr};

  //! Frame
  Frame *window_frame{nullptr};

  //! Warn color
  Gdk::RGBA color_warn;

  //! Alert color
  Gdk::RGBA color_alert;

  //! Label
  Gtk::Label *label{nullptr};

  //! Icon
  Gtk::Image *image_icon{nullptr};

  //! Final prelude
  std::string progress_text;

  //! Progress values
  int progress_value{0};
  int progress_max_value{0};

  //! Flash
  bool flash_visible{false};

  //! Head
  HeadInfo head;

  // Alignment in Wayland
  Gtk::Alignment *align{nullptr};

#if defined(HAVE_WAYLAND)
  std::shared_ptr<WaylandWindowManager> window_manager;
#endif
};

#endif // PRELUDEWINDOW_HH
