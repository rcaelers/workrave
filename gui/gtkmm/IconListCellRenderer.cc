#include <gtkmm.h>
#include "IconListCellRenderer.hh"

#define PAD 3
#define SPACE 5

IconListCellRenderer::IconListCellRenderer()
  : Glib::ObjectBase (typeid(IconListCellRenderer)),
          Gtk::CellRenderer (),
               property_pixbuf_ (*this, "pixbuf"),
               property_text_ (*this, "text"),
               property_active_      (*this, "active",      false)
{
  property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
  property_xpad() = 2;
  property_ypad() = 2;
}

IconListCellRenderer::~IconListCellRenderer()
{}

Glib::PropertyProxy<Glib::ustring> IconListCellRenderer::property_text()
{
  return property_text_.get_proxy();
}

Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > IconListCellRenderer::property_pixbuf()
{
  return property_pixbuf_.get_proxy();
}

Glib::PropertyProxy<bool>
IconListCellRenderer::property_active()
{
  return property_active_.get_proxy();
}


void IconListCellRenderer::get_size(Gtk::Widget& widget,
                                          const Gdk::Rectangle& cell_area,
                                          int& x_offset, int& y_offset,
                                          int& width,    int& height)
{
 int text_x_offset;
 int text_y_offset;
 int text_width;
 int text_height;

 int pb_x_offset;
 int pb_y_offset;
 int pb_width;
 int pb_height;
 
 pixbuf_renderer.get_size(widget, cell_area, 
                          pb_x_offset, pb_y_offset, pb_width, pb_height);
 
 text_renderer.get_size(widget, cell_area,
                        text_x_offset, text_y_offset, text_width, text_height);
  
 height = pb_height + text_height + PAD;
 width = 2*SPACE + MAX (pb_width, text_width);
}

SigC::Signal1<void, const Glib::ustring&>&
IconListCellRenderer::signal_toggled()
{
  return signal_toggled_;
}

void
IconListCellRenderer::render_vfunc(const Glib::RefPtr<Gdk::Window>& window,
                                   Gtk::Widget& widget,
                                   const Gdk::Rectangle&,
                                   const Gdk::Rectangle& cell_area,
                                   const Gdk::Rectangle&,
                                   Gtk::CellRendererState flags)
{
  const unsigned int cell_xpad = property_xpad();
  const unsigned int cell_ypad = property_ypad();

  int x_offset = 0, y_offset = 0, width = 0, height = 0;
  get_size(widget, cell_area, x_offset, y_offset, width, height);

  width  -= cell_xpad * 2;
  height -= cell_ypad * 2;

  if(width <= 0 || height <= 0)
    return;

  Gtk::StateType state = state = Gtk::STATE_NORMAL;

  if((flags & Gtk::CELL_RENDERER_SELECTED) != 0)
    state = (widget.has_focus()) ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE;

  const Gtk::ShadowType shadow = (property_active_) ? Gtk::SHADOW_IN : Gtk::SHADOW_OUT;

  widget.get_style()->paint_check(
                                  window, state, shadow,
                                  cell_area, widget, "cellcheck",
                                  cell_area.get_x() + x_offset + cell_xpad,
                                  cell_area.get_y() + y_offset + cell_ypad,
                                  width - 1, height - 1);
}

bool IconListCellRenderer::activate_vfunc(GdkEvent*,
                                          Gtk::Widget&,
                                          const Glib::ustring& path,
                                          const Gdk::Rectangle&,
                                          const Gdk::Rectangle&,
                                          Gtk::CellRendererState)
{
  signal_toggled_(path);
  return true;
}


