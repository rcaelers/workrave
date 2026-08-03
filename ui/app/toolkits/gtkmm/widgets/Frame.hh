// Copyright (C) 2001 - 2011 Raymond Penners <raymond@dotsphinx.com>
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

#include <gtkmm.h>

class Frame
  :
#if GTK_CHECK_VERSION(4, 0, 0)
  public Gtk::Widget
#else
  public Gtk::Bin
#endif
{
	public:
  using flash_signal_t = sigc::signal<void(bool)>;

  enum Style
  {
    STYLE_SOLID,
    STYLE_BREAK_WINDOW
  };

  Frame();
  ~Frame() override;

  void set_frame_width(guint width);
  void set_frame_style(Style style);
  void set_frame_color(const Gdk::RGBA &color);
  void set_frame_flashing(int delay);
  void set_frame_visible(bool visible);
  flash_signal_t &signal_flash();

#if GTK_CHECK_VERSION(4, 0, 0)
  // GTK4 dropped Gtk::Bin; this class now derives from Gtk::Widget directly
  // and has to provide its own single-child bookkeeping.
  void add(Gtk::Widget &child);
  Gtk::Widget *get_child() const;
  void set_border_width(guint width);
  guint get_border_width() const;
#endif

protected:
  bool on_timer();

#if GTK_CHECK_VERSION(4, 0, 0)
  void size_allocate_vfunc(int width, int height, int baseline) override;
  void measure_vfunc(Gtk::Orientation orientation,
                      int for_size,
                      int &minimum,
                      int &natural,
                      int &minimum_baseline,
                      int &natural_baseline) const override;
  void snapshot_vfunc(const Glib::RefPtr<Gtk::Snapshot> &snapshot) override;
#else
  void on_size_allocate(Gtk::Allocation &allocation) override;

  Gtk::SizeRequestMode get_request_mode_vfunc() const override;
  void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
  void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
  void get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const override;
  void get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const override;
  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
#endif

  void draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
  void set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::RGBA &color);

private:
  //! Frame border width
  guint frame_width{1};

  //! Color of the frame.
  Gdk::RGBA frame_color;

  //! Black
  Gdk::RGBA color_black;

  //! Style of the frame.
  Style frame_style{STYLE_SOLID};

  //! Visible;
  bool frame_visible{true};

  //! Flash delay;
  int flash_delay{-1};

  //! Flash timeout signal
  sigc::connection flash_signal;

  //! Flash signal source
  flash_signal_t flash_signal_src;

#if GTK_CHECK_VERSION(4, 0, 0)
  //! Single child (GTK4 has no Gtk::Bin to provide this).
  Gtk::Widget *child{nullptr};

  //! Border width (GTK4 has no Gtk::Container to provide this).
  guint border_width{0};
#endif
};
