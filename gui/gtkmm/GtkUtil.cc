// GtkUtil.cc --- Gtk utilities
//
// Copyright (C) 2003 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "nls.h"

#include "debug.hh"

#include "GUI.hh"
#include "GtkUtil.hh"
#include "EventLabel.hh"

Gtk::Button *
GtkUtil::create_stock_button_without_text(const Gtk::StockID& stock_id)
{
  Gtk::Button *btn = new Gtk::Button();
  Gtk::Image *img = new Gtk::Image(stock_id, 
                                   Gtk::ICON_SIZE_BUTTON);
  Gtk::manage(img);
  btn->add(*img);
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
GtkUtil::create_label_for_break(GUIControl::BreakId id)
{
  GUIControl *guic = GUIControl::get_instance();
  GUIControl::TimerData *timer = guic->get_timer_data(id);
#if 0
  timer = &(guic->timers[(int)id]);
#endif  
  Gtk::Widget *label = 
    GtkUtil::create_label_with_icon (timer->label, timer->icon.c_str());
  return label;
}


void
GtkUtil::table_attach_aligned(Gtk::Table &table, Gtk::Widget &child,
                              guint left_attach, guint top_attach, bool left)
{
  Gtk::Alignment *a = Gtk::manage(new Gtk::Alignment
                                  (left ? Gtk::ALIGN_LEFT : Gtk::ALIGN_RIGHT,
                                   Gtk::ALIGN_BOTTOM,
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
  Gtk::Label *label = Gtk::manage(new Gtk::Label(text));
  Gtk::EventBox *eventbox = Gtk::manage(new Gtk::EventBox());
  eventbox->add(*label);

  GUI::get_instance()->get_tooltips()->set_tip(*eventbox, tooltip);
  return eventbox;
#else
  EventLabel *label = Gtk::manage(new EventLabel(text));
  
  GUI::get_instance()->get_tooltips()->set_tip(*label, tooltip);
  return label;
#endif
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



void
GtkUtil::set_wmclass(Gtk::Window &window, string class_postfix)
{
  string s = gdk_get_program_class();
  s += class_postfix;
  
  window.set_wmclass(g_get_prgname(), s);
}


/**
 * gdk_pixbuf_flip:
 * @pixbuf: a #GdkPixbuf
 * @horizontal: Whether to flip in horizontal direction.  If @horizontal is
 * #TRUE, then flip around in horizontal direction.
 * @vertical: Whether to flip in vertical direction.  If @vertical is #TRUE,
 * then flip in vertical direction.  (NOT IMPLEMENTED YET!)
 * @in_place: If in_place is #TRUE, the original @pixbuf is modified.
 *
 * Flips @pixbuf around horizontally and/or vertically.
 *
 * Return value: The original @pixbuf is modified and returned, if
 * in_place is #TRUE.  If in_place is #FALSE, a newly created pixbuf
 * with a reference count of 1 is returned.
 **/
static GdkPixbuf *
gdk_pixbuf_flip (GdkPixbuf *pixbuf,
		 gboolean horizontal,
		 gboolean vertical,
		 gboolean in_place)
{
  GdkPixbuf *dest;
  gint src_has_alpha;
  gint width, height, src_rs;
  gint dst_rs;

  guchar *src_pix, *src_p;
  guchar *dst_pix, *dst_p;

  gint i, j;
  gint alpha;

  (void) vertical; // FIXME: not supported yet
  g_return_val_if_fail (pixbuf != NULL, NULL);

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  src_has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);

  alpha = src_has_alpha ? 4 : 3;
	
  src_rs = gdk_pixbuf_get_rowstride (pixbuf);
  src_pix = gdk_pixbuf_get_pixels (pixbuf);

  if (in_place) {
    dest = pixbuf;
    dst_rs = gdk_pixbuf_get_rowstride (pixbuf);
    dst_pix = gdk_pixbuf_get_pixels (pixbuf);
  } else {
    dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB, src_has_alpha, 8, width, height);
    dst_rs = gdk_pixbuf_get_rowstride (dest);
    dst_pix = gdk_pixbuf_get_pixels (dest);
  }

  if (horizontal==TRUE) {
    for (i = 0; i < height; i++) {
      src_p = src_pix + (src_rs * i);
      dst_p = dst_pix + (dst_rs * i);
			
      for (j = 0; j < width; j++) {
        *(dst_p++) = *(src_p++);
        *(dst_p++) = *(src_p++);
        *(dst_p++) = *(src_p++);
        if (src_has_alpha)
          *(dst_p) = *(src_p++);
        dst_p = dst_p - (alpha + 3);
      }
    }
  }
	
  return dest;
}


/*
 * Returns a copy of pixbuf mirrored and or flipped.
 * TO do a 180 degree rotations set both mirror and flipped TRUE
 * if mirror and flip are FALSE, result is a simple copy.
 */
static GdkPixbuf *pixbuf_copy_mirror(GdkPixbuf *src, gint mirror, gint flip)
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
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				if (has_alpha) *(dp) = *(sp++);	/* a */
				dp -= (a + 3);
				}
			}
		else
			{
			for (j = 0; j < w; j++)
				{
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				if (has_alpha) *(dp++) = *(sp++);	/* a */
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
