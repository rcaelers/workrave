#include <gtkmm.h>
#include <gtk/gtk.h>
#include "IconListCellRenderer.hh"

#define PAD 3
#define SPACE 5

IconListCellRenderer::IconListCellRenderer()
  : Glib::ObjectBase (typeid(IconListCellRenderer)),
          Gtk::CellRenderer (),
               property_text_      (*this, "text"),
               property_pixbuf_      (*this, "pixbuf")
{
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


void
IconListCellRenderer::update_properties()
{
  text_renderer.property_text() = property_text_;
  pixbuf_renderer.property_pixbuf() = property_pixbuf_;
}


void
IconListCellRenderer::get_size_vfunc(Gtk::Widget& widget,
                                     const Gdk::Rectangle* cell_area,
                                     int* x_offset, int* y_offset,
                                     int* width,    int* height)
{
  (void) x_offset;
  (void) y_offset;
  
  int text_x_offset;
  int text_y_offset;
  int text_width;
  int text_height;

  update_properties();
  GdkRectangle *rect = NULL;
  if (cell_area)
    {
      rect = (GdkRectangle *) cell_area->gobj();
    }
    
  
  GtkCellRenderer *rend = GTK_CELL_RENDERER(text_renderer.gobj());
  gtk_cell_renderer_get_size (rend, widget.gobj(), rect,
                              NULL, NULL, width, height);

  rend = GTK_CELL_RENDERER(pixbuf_renderer.gobj());
  gtk_cell_renderer_get_size (rend, widget.gobj(),
                              rect,
                              &text_x_offset, &text_y_offset,
                              &text_width, &text_height);
  if (height) {
    *height = *height + text_height + PAD;
  }

  if (width) {
    *width = MAX (*width, text_width);
    *width += SPACE * 2;
  }
}


void
IconListCellRenderer::render_vfunc(const Glib::RefPtr<Gdk::Window>& window,
                                   Gtk::Widget& widget,
                                   const Gdk::Rectangle& bg_area,
                                   const Gdk::Rectangle& cell_area,
                                   const Gdk::Rectangle& expose_area,
                                   Gtk::CellRendererState flags)
{
  update_properties();
#if 1
  GdkRectangle text_area;
  GdkRectangle pixbuf_area;
  int width, height;

  GdkRectangle *ca = (GdkRectangle *) cell_area.gobj();
  GtkWidget *widg = widget.gobj();
  GdkWindow *wind = window->gobj();
  GtkCellRenderer *prend = GTK_CELL_RENDERER(pixbuf_renderer.gobj());
  gtk_cell_renderer_get_size (prend, widg, ca, 
                              NULL, NULL, &width, &height);
  	
  pixbuf_area.y = ca->y;
  pixbuf_area.x = ca->x;
  pixbuf_area.height = height;
  pixbuf_area.width = ca->width;
  
  GtkCellRenderer *trend = GTK_CELL_RENDERER(text_renderer.gobj());
  gtk_cell_renderer_get_size (trend, widg, ca, 
                              NULL, NULL, &width, &height);
  
  text_area.x = ca->x + (ca->width - width) / 2;
  text_area.y = ca->y + (pixbuf_area.height + PAD);
  text_area.height = height;
  text_area.width = width;
  
  gtk_cell_renderer_render (prend, wind, widg, 
                            (GdkRectangle*)bg_area.gobj(), &pixbuf_area,
                            (GdkRectangle*)expose_area.gobj(),
                            (GtkCellRendererState) flags);
  
  gtk_cell_renderer_render (trend, wind, widg,
                            (GdkRectangle*)bg_area.gobj(),
                            &text_area,
                            (GdkRectangle*)expose_area.gobj(),
                            (GtkCellRendererState)  flags);  
#endif
#if 0
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
#endif
}

