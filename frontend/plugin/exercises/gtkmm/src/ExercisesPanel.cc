// ExercisesPanel.cc --- Exercises panel
//
// Copyright (C) 2002, 2003, 2004 Raymond Penners <raymond@dotsphinx.com>
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

#include "config.h"

#ifdef HAVE_EXERCISES

#include "preinclude.h"

#include <gtkmm/stock.h>

#include "ExercisesPanel.hh"
#include "GtkUtil.hh"
#include "GUI.hh"
#include "Util.hh"
#include "Hig.hh"
#include "nls.h"
#include "SoundPlayerInterface.hh"
#include "debug.hh"

// This code can be removed once the following bug is closed:
// http://bugzilla.gnome.org/show_bug.cgi?id=59390

#include <gtk/gtktextbuffer.h>

static void
text_buffer_insert_markup_real (GtkTextBuffer *buffer,
                                GtkTextIter   *textiter,
                                const gchar   *markup,
                                gint           len)
{
	PangoAttrIterator  *paiter;
	PangoAttrList      *attrlist;
	GtkTextMark        *mark;
	GError             *error = NULL;
	gchar              *text;

  g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (textiter != NULL);
  g_return_if_fail (markup != NULL);
  g_return_if_fail (gtk_text_iter_get_buffer (textiter) == buffer);

	if (len == 0)
		return;

	if (!pango_parse_markup(markup, len, 0, &attrlist, &text, NULL, &error))
	{
		g_warning("Invalid markup string: %s", error->message);
		g_error_free(error);
		return;
	}

	len = strlen(text); /* TODO: is this needed? */

	if (attrlist == NULL)
	{
		gtk_text_buffer_insert(buffer, textiter, text, len);
		g_free(text);
		return;
	}

	/* create mark with right gravity */
	mark = gtk_text_buffer_create_mark(buffer, NULL, textiter, FALSE);

	paiter = pango_attr_list_get_iterator(attrlist);

	do
	{
		PangoAttribute *attr;
		GtkTextTag     *tag;
		gint            start, end;

		pango_attr_iterator_range(paiter, &start, &end);

		if (end == G_MAXINT)  /* last chunk */
			end = strlen(text);

		tag = gtk_text_tag_new(NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_LANGUAGE)))
			g_object_set(tag, "language", pango_language_to_string(((PangoAttrLanguage*)attr)->value), NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_FAMILY)))
			g_object_set(tag, "family", ((PangoAttrString*)attr)->value, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_STYLE)))
			g_object_set(tag, "style", ((PangoAttrInt*)attr)->value, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_WEIGHT)))
			g_object_set(tag, "weight", ((PangoAttrInt*)attr)->value, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_VARIANT)))
			g_object_set(tag, "variant", ((PangoAttrInt*)attr)->value, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_STRETCH)))
			g_object_set(tag, "stretch", ((PangoAttrInt*)attr)->value, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_SIZE)))
			g_object_set(tag, "size", ((PangoAttrInt*)attr)->value, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_FONT_DESC)))
			g_object_set(tag, "font-desc", ((PangoAttrFontDesc*)attr)->desc, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_FOREGROUND)))
		{
			GdkColor col = { 0,
			                 ((PangoAttrColor*)attr)->color.red,
			                 ((PangoAttrColor*)attr)->color.green,
			                 ((PangoAttrColor*)attr)->color.blue
			               };

			g_object_set(tag, "foreground-gdk", &col, NULL);
		}

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_BACKGROUND)))
		{
			GdkColor col = { 0,
			                 ((PangoAttrColor*)attr)->color.red,
			                 ((PangoAttrColor*)attr)->color.green,
			                 ((PangoAttrColor*)attr)->color.blue
			               };

			g_object_set(tag, "background-gdk", &col, NULL);
		}

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_UNDERLINE)))
			g_object_set(tag, "underline", ((PangoAttrInt*)attr)->value, NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_STRIKETHROUGH)))
			g_object_set(tag, "strikethrough", (gboolean)(((PangoAttrInt*)attr)->value != 0), NULL);

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_RISE)))
			g_object_set(tag, "rise", ((PangoAttrInt*)attr)->value, NULL);

		/* PANGO_ATTR_SHAPE cannot be defined via markup text */

		if ((attr = pango_attr_iterator_get(paiter, PANGO_ATTR_SCALE)))
			g_object_set(tag, "scale", ((PangoAttrFloat*)attr)->value, NULL);

		gtk_text_tag_table_add(gtk_text_buffer_get_tag_table(buffer), tag);

        	gtk_text_buffer_insert_with_tags(buffer, textiter, text+start, end - start, tag, NULL);

		/* mark had right gravity, so it should be
		 *  at the end of the inserted text now */
		gtk_text_buffer_get_iter_at_mark(buffer, textiter, mark);
	}
	while (pango_attr_iterator_next(paiter));

	gtk_text_buffer_delete_mark(buffer, mark);
	pango_attr_iterator_destroy(paiter);
	pango_attr_list_unref(attrlist);
	g_free(text);
}

static void
text_buffer_insert_markup (GtkTextBuffer *buffer,
                           GtkTextIter   *iter,
                           const gchar   *markup,
                           gint           len)
{
  text_buffer_insert_markup_real (buffer, iter, markup, len);
}



static void
text_buffer_set_markup (GtkTextBuffer *buffer,
                        const gchar   *markup,
                        gint           len)
{
	GtkTextIter start, end;

	g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
	g_return_if_fail (markup != NULL);

	if (len < 0)
		len = strlen (markup);

	gtk_text_buffer_get_bounds (buffer, &start, &end);

	gtk_text_buffer_delete (buffer, &start, &end);

	if (len > 0)
	{
		gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
		text_buffer_insert_markup (buffer, &start, markup, len);
	}
}
// (end code to be removed)

int ExercisesPanel::exercises_pointer = 0;

ExercisesPanel::ExercisesPanel(Gtk::HButtonBox *dialog_action_area)
  : Gtk::HBox(false, 6),
         exercises(Exercise::get_exercises())
{
  standalone = dialog_action_area != NULL;
  
  progress_bar.set_orientation(Gtk::PROGRESS_BOTTOM_TO_TOP);

  description_scroll.add(description_text);
  description_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  description_text.set_cursor_visible(false);
  description_text.set_wrap_mode(Gtk::WRAP_WORD);
  description_text.set_editable(false);
  image_frame.add(image);

  pause_button =  manage(new Gtk::Button());
  Gtk::Widget *description_widget;
  
  if (dialog_action_area != NULL)
    {
      back_button =  manage(new Gtk::Button(Gtk::Stock::GO_BACK));
      forward_button =  manage(new Gtk::Button(Gtk::Stock::GO_FORWARD));
      
      dialog_action_area->pack_start(*back_button, false, false, 0);
      dialog_action_area->pack_start(*pause_button, false, false, 0);
      dialog_action_area->pack_start(*forward_button, false, false, 0);
      description_widget = &description_scroll;
    }
  else
    {
      back_button =  GtkUtil::create_custom_stock_button
        (NULL, Gtk::Stock::GO_BACK);
      forward_button =  GtkUtil::create_custom_stock_button
        (NULL, Gtk::Stock::GO_FORWARD);
      
      Gtk::Button *stop_button =  GtkUtil::create_custom_stock_button
        (NULL, Gtk::Stock::CLOSE);
      stop_button->signal_clicked()
        .connect(MEMBER_SLOT(*this, &ExercisesPanel::on_stop));

      Gtk::HBox *button_box = manage(new Gtk::HBox());
      Gtk::Label *browse_label = manage(new Gtk::Label());
      string browse_label_text = "<b>";
      browse_label_text += _("Exercises player");
      browse_label_text += ":</b>";
      browse_label->set_markup(browse_label_text);
      button_box->pack_start(*browse_label, false, false, 6);
      button_box->pack_start(*back_button, false, false, 0);
      button_box->pack_start(*pause_button, false, false, 0);
      button_box->pack_start(*forward_button, false, false, 0);
      button_box->pack_start(*stop_button, false, false, 0);
      Gtk::Alignment *button_align
        = manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
      button_align->add(*button_box);
      Gtk::VBox *description_box = manage(new Gtk::VBox());
      description_box->pack_start(description_scroll, true, true, 0);
      description_box->pack_start(*button_align, false, false, 0);
      description_widget = description_box;
    }

  // This is ugly, but I could not find a decent way to do this otherwise.
  //  size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_BOTH);
  //  size_group->add_widget(image_frame);
  //  size_group->add_widget(*description_widget);
  image.set_size_request(250, 250);
  description_scroll.set_size_request(250, 200);
  // (end of ugly)

  back_button->signal_clicked()
    .connect(MEMBER_SLOT(*this, &ExercisesPanel::on_go_back));

  forward_button->signal_clicked()
    .connect(MEMBER_SLOT(*this, &ExercisesPanel::on_go_forward));

  pause_button->signal_clicked()
    .connect(MEMBER_SLOT(*this, &ExercisesPanel::on_pause));

  
  pack_start(image_frame, false, false, 0);
  pack_start(progress_bar, false, false, 0);
  pack_start(*description_widget, false, false, 0);

  heartbeat_signal = GUI::get_instance()->signal_heartbeat()
    .connect(MEMBER_SLOT(*this, &ExercisesPanel::heartbeat));

  exercise_count = 0;
  reset();

}

void
ExercisesPanel::on_realize()
{
  Gtk::HBox::on_realize();
  description_text.modify_base
    (Gtk::STATE_NORMAL, get_style()->get_background(Gtk::STATE_NORMAL));
} 


ExercisesPanel::~ExercisesPanel()
{
  if (heartbeat_signal.connected())
    {
      heartbeat_signal.disconnect();
    }
}

void
ExercisesPanel::reset()
{
  int i = adjust_exercises_pointer(1);
  exercise_iterator = exercises.begin();
  while (i > 0)
    {
      exercise_iterator++;
      i--;
    }
  exercise_num = 0;
  paused = false;
  stopped = false;
  refresh_pause();
  start_exercise();
}

void
ExercisesPanel::start_exercise()
{
  const Exercise &exercise = *exercise_iterator;
  Glib::RefPtr<Gtk::TextBuffer> buf = description_text.get_buffer();
  std::string txt = HigUtil::create_alert_text(exercise.title.c_str(),
                                                 exercise.description.c_str());
  text_buffer_set_markup(buf->gobj(), txt.c_str(), txt.length());
  exercise_time = 0;
  seq_time = 0;
  image_iterator = exercise.sequence.end();
  refresh_progress();
  refresh_sequence();
}

void
ExercisesPanel::show_image()
{
  TRACE_ENTER("ExercisesPanel::show_image");
  const Exercise::Image &img = (*image_iterator);
  seq_time += img.duration;
  TRACE_MSG("image=" << img.image);
  string file = Util::complete_directory(img.image,
                                         Util::SEARCH_PATH_EXERCISES);
  if (! img.mirror_x)
    {
      image.set(file);
    }
  else
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(file);
      Glib::RefPtr<Gdk::Pixbuf> flip = GtkUtil::flip_pixbuf(pixbuf, true, false);
      image.set(flip);
    }
  TRACE_EXIT();
}

void
ExercisesPanel::refresh_sequence()
{
  if (exercise_time >= seq_time)
    {
      image_iterator++;
      const Exercise &exercise = *exercise_iterator;
      if (image_iterator == exercise.sequence.end())
        {
          image_iterator = exercise.sequence.begin();
        }
      show_image();
    }
}

void
ExercisesPanel::refresh_progress()
{
  const Exercise &exercise = *exercise_iterator;
  progress_bar.set_fraction(1.0 - (double) exercise_time
                            / exercise.duration);
}

void
ExercisesPanel::on_stop()
{
  if (! stopped)
    {
      stopped = true;
      stop_signal();
    }
}

void
ExercisesPanel::on_go_back()
{
  adjust_exercises_pointer(-1);
  if (exercise_iterator == exercises.begin())
    {
      exercise_iterator = --(exercises.end());
    }
  else
    {
      exercise_iterator--;
    }
  start_exercise();
}

void
ExercisesPanel::on_go_forward()
{
  adjust_exercises_pointer(1);
  exercise_iterator++;
  if (exercise_iterator == exercises.end())
    {
      exercise_iterator = exercises.begin();;
    }
  start_exercise();
}

void
ExercisesPanel::refresh_pause()
{
  Gtk::StockID stock_id = paused ? Gtk::Stock::EXECUTE : Gtk::Stock::STOP;
  const char *label = label = paused ? _("Resume") : _("Pause");
  GtkUtil::update_custom_stock_button(pause_button,
                                      standalone ? label : NULL,
                                      stock_id);
}

void
ExercisesPanel::on_pause()
{
  paused = ! paused;
  refresh_pause();
}

void
ExercisesPanel::heartbeat()
{
  if (paused || stopped)
    return;
  
  const Exercise &exercise = *exercise_iterator;
  exercise_time++;
  if (exercise_time >= exercise.duration)
    {
      on_go_forward();
      SoundPlayerInterface *snd = GUI::get_instance()->get_sound_player();
      exercise_num++;
      if (exercise_num == exercise_count)
        {
          on_stop();
        }
      snd->play_sound(stopped
                      ? SoundPlayerInterface::SOUND_EXERCISES_ENDED
                      : SoundPlayerInterface::SOUND_EXERCISE_ENDED);
    }
  else
    {
      refresh_sequence();
      refresh_progress();
    }
}


void
ExercisesPanel::set_exercise_count(int num)
{
  exercise_count = num;
}

#endif // HAVE_EXERCISES
