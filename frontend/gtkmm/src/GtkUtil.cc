// GtkUtil.cc --- Gtk utilities
//
// Copyright (C) 2003, 2004, 2005, 2007, 2008, 2011 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "nls.h"

#include "debug.hh"

#include <glib-object.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "GUI.hh"
#include "Util.hh"
#include "GtkUtil.hh"
#include "EventLabel.hh"
#include "EventImage.hh"
#include "HeadInfo.hh"

Glib::Quark *GtkUtil::label_quark = new Glib::Quark("workrave-button-label");

bool
GtkUtil::has_button_images()
{
  // Bypassing gtkmm is necessary, because it does not offer
  // a find_property method yet.
  GtkSettings* settings = gtk_settings_get_default();
  GObjectClass * klazz = G_OBJECT_GET_CLASS(G_OBJECT(settings));
  bool ret = true;
  if (g_object_class_find_property (klazz, "gtk-button-images"))
    {
      gboolean gbi;
      g_object_get (G_OBJECT(settings), "gtk-button-images", &gbi, NULL);
      ret = gbi;
    }
  return ret;
}

Gtk::Button *
GtkUtil::create_custom_stock_button(const char *label_text,
                                    const Gtk::StockID& stock_id)
{
  Gtk::Button *ret = new Gtk::Button();
  update_custom_stock_button(ret, label_text, stock_id);
  return ret;
}

void
GtkUtil::update_custom_stock_button(Gtk::Button *btn,
                                    const char *label_text,
                                    const Gtk::StockID& stock_id)
{
  Gtk::Image *img = NULL;

  if (has_button_images() || !label_text)
    {
      img = Gtk::manage(new Gtk::Image(stock_id,
                                  Gtk::ICON_SIZE_BUTTON));
    }
  btn->remove();
  if (label_text != NULL)
    {
      Gtk::Label *label = Gtk::manage(new Gtk::Label(label_text));
      Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 2));
      Gtk::Alignment *align = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0.0, 0.0));
      if (img != NULL)
        {
          hbox->pack_start(*img, false, false, 0);
        }
      label->set_use_underline();
      btn->set_data(*label_quark, (void*)label);
      hbox->pack_end(*label, false, false, 0);
      btn->add(*align);
      align->add(*hbox);
      align->show_all();
    }
  else
    {
      btn->add(*img);
      img->show_all();
    }
}


Gtk::Button *
GtkUtil::create_image_button(const char *label_text,
                             const char *image_file,
                             bool label)
{
  Gtk::Button *btn = new Gtk::Button();
  Gtk::Image *img = NULL;
  if (has_button_images())
    {
      string icon = Util::complete_directory(image_file,
                                             Util::SEARCH_PATH_IMAGES);
      img = Gtk::manage(new Gtk::Image(icon));
    }
  else
    {
      /* Button witout images must have a label */
      label = true;
    }
  if (label_text != NULL && label)
    {
      Gtk::Label *label = Gtk::manage(new Gtk::Label(label_text));
      Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 2));
      Gtk::Alignment *align = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0.0, 0.0));
      if (img != NULL)
        {
          hbox->pack_start(*img, false, false, 0);
        }
      label->set_use_underline();
      btn->set_data(*label_quark, (void*)label);
      hbox->pack_end(*label, false, false, 0);
      btn->add(*align);
      align->add(*hbox);
      align->show_all();
    }
  else
    {
      btn->add(*img);
      img->show_all();
    }
  return btn;
}



Gtk::Widget *
GtkUtil::create_label_with_icon(string text, const char *icon)
{
  Gtk::HBox *box = new Gtk::HBox(false, 3);
  Gtk::Label *lab = Gtk::manage(new Gtk::Label(text));
  Gtk::Image *img = Gtk::manage(new Gtk::Image(icon));
  box->pack_start(*img, false, false, 0);
  box->pack_start(*lab, false, false, 0);
  return box;
}


Gtk::Widget *
GtkUtil::create_label_for_break(BreakId id)
{
  // FIXME: duplicate:
  const char *icons[] = { "timer-micro-break.png", "timer-rest-break.png", "timer-daily.png" };
  const char *labels[] = { _("Micro-break"), _("Rest break"), _("Daily limit") };

  string icon = Util::complete_directory(string(icons[id]), Util::SEARCH_PATH_IMAGES);

  Gtk::Widget *label =
    GtkUtil::create_label_with_icon(labels[id], icon.c_str());
  return label;
}


void
GtkUtil::table_attach_aligned(Gtk::Table &table, Gtk::Widget &child,
                              guint left_attach, guint top_attach, bool left)
{
  Gtk::Alignment *a = Gtk::manage(new Gtk::Alignment
#ifdef HAVE_GTK3
                                  (left ? Gtk::ALIGN_START : Gtk::ALIGN_END,
                                   Gtk::ALIGN_START,
#else                                  
                                  (left ? Gtk::ALIGN_LEFT : Gtk::ALIGN_RIGHT,
                                   Gtk::ALIGN_BOTTOM,
#endif                                   
                                   0.0, 0.0));
  a->add(child);
  table.attach(*a, left_attach, left_attach+1, top_attach, top_attach + 1,
               Gtk::FILL, Gtk::SHRINK);
}

void
GtkUtil::table_attach_left_aligned(Gtk::Table &table, Gtk::Widget &child,
                                   guint left_attach, guint top_attach)
{
  table_attach_aligned(table, child, left_attach, top_attach, true);
}

void
GtkUtil::table_attach_right_aligned(Gtk::Table &table, Gtk::Widget &child,
                                   guint left_attach, guint top_attach)
{
  table_attach_aligned(table, child, left_attach, top_attach, false);
}



Gtk::Widget *
GtkUtil::create_label_with_tooltip(string text, string tooltip)
{
#if 0
  // This doesn't (didn't ?) work.
  Gtk::Label *label = Gtk::manage(new Gtk::Label(text));
  Gtk::EventBox *eventbox = Gtk::manage(new Gtk::EventBox());
  eventbox->add(*label);

  eventbox->set_tooltip_text(tooltip);
  return eventbox;
#else
  EventLabel *label = Gtk::manage(new EventLabel(text));
  label->set_tooltip_text(tooltip);
  return label;
#endif
}


EventImage *
GtkUtil::create_image_with_tooltip(string file, string tooltip)
{
  EventImage *image = Gtk::manage(new EventImage(file));

  image->set_tooltip_text(tooltip);
  return image;
}


Gtk::Label *
GtkUtil::create_label(string text, bool bold)
{
  Gtk::Label *label
    = new Gtk::Label();
  if (bold)
    {
      label->set_markup(string("<span weight=\"bold\">") + text + "</span>");
    }
  else
    {
      label->set_text(text);
    }
  return label;
}


/*
 * Returns a copy of pixbuf mirrored and or flipped.
 * TO do a 180 degree rotations set both mirror and flipped TRUE
 * if mirror and flip are FALSE, result is a simple copy.
 */
static GdkPixbuf *
pixbuf_copy_mirror(GdkPixbuf *src, gint mirror, gint flip)
{
  GdkPixbuf *dest;
  gint has_alpha;
  gint w, h, srs;
  gint drs;
  guchar *s_pix;
  guchar *d_pix;
  guchar *sp;
  guchar *dp;
  gint i, j;
  gint a;

  if (!src) return NULL;

  w = gdk_pixbuf_get_width(src);
  h = gdk_pixbuf_get_height(src);
  has_alpha = gdk_pixbuf_get_has_alpha(src);
  srs = gdk_pixbuf_get_rowstride(src);
  s_pix = gdk_pixbuf_get_pixels(src);

  dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, has_alpha, 8, w, h);
  drs = gdk_pixbuf_get_rowstride(dest);
  d_pix = gdk_pixbuf_get_pixels(dest);

  a = has_alpha ? 4 : 3;

  for (i = 0; i < h; i++)
    {
      sp = s_pix + (i * srs);
      if (flip)
        {
          dp = d_pix + ((h - i - 1) * drs);
        }
      else
        {
          dp = d_pix + (i * drs);
        }
      if (mirror)
        {
          dp += (w - 1) * a;
          for (j = 0; j < w; j++)
            {
              *(dp++) = *(sp++);  /* r */
              *(dp++) = *(sp++);  /* g */
              *(dp++) = *(sp++);  /* b */
              if (has_alpha) *(dp) = *(sp++); /* a */
              dp -= (a + 3);
            }
        }
      else
        {
          for (j = 0; j < w; j++)
            {
              *(dp++) = *(sp++);  /* r */
              *(dp++) = *(sp++);  /* g */
              *(dp++) = *(sp++);  /* b */
              if (has_alpha) *(dp++) = *(sp++); /* a */
            }
        }
    }

  return dest;
}


Glib::RefPtr<Gdk::Pixbuf>
GtkUtil::flip_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, bool horizontal, bool vertical)
{
  GdkPixbuf *pb = pixbuf->gobj();
  GdkPixbuf *pbflip = pixbuf_copy_mirror(pb, horizontal, vertical);
  return Glib::wrap(pbflip, false);
}


//! Centers the window.
void
GtkUtil::center_window(Gtk::Window &window, HeadInfo &head)
{
  TRACE_ENTER("GtkUtil::center_window");

  if (head.valid)
    {
      Gtk::Requisition size;
#ifdef HAVE_GTK3
      // FIXME: GTK3
      Gtk::Requisition minsize;
      window.get_preferred_size(minsize, size);
#else
      window.size_request(size);
#endif      

      int x = head.geometry.get_x() + (head.geometry.get_width() - size.width) / 2;
      int y = head.geometry.get_y() + (head.geometry.get_height() - size.height) / 2;

      TRACE_MSG(head.geometry.get_x() << " "  << head.geometry.get_width() << " " << size.width);
      TRACE_MSG(head.geometry.get_y() << " "  << head.geometry.get_height() << " " << size.height);

      window.set_position(Gtk::WIN_POS_NONE);
      window.move(x, y);
    }
  else
    {
      window.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
    }
  TRACE_EXIT();
}

void
GtkUtil::update_mnemonic(Gtk::Widget *widget, Glib::RefPtr<Gtk::AccelGroup> accel_group)
{
  Gtk::Label *label = (Gtk::Label *)widget->get_data(*label_quark);
  if (label != NULL)
    {
      guint mnemonic = label->get_mnemonic_keyval();
      if (mnemonic != GDK_KEY_VoidSymbol)
        {
          widget->add_accelerator("activate", accel_group, mnemonic,  (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
        }
    }
}
