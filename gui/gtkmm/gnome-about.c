/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-about.c - An about box widget for gnome.

   Copyright (C) 2001, 2002 CodeFactory AB
   Copyright (C) 2001, 2002 Anders Carlsson

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Anders Carlsson <andersca@gnu.org>
*/

#include <config.h>

#include "gnome-about.h"

#include <gtk/gtkbbox.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktextview.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkviewport.h>

#include <glib/gmacros.h>

G_BEGIN_DECLS

/* Macros for defining classes.  Ideas taken from Nautilus and GOB. */

/* Define the boilerplate type stuff to reduce typos and code size.  Defines
 * the get_type method and the parent_class static variable. */

#define BONOBO_BOILERPLATE(type, type_as_function, corba_type,		\
			   parent_type, parent_type_macro,		\
			   register_type_macro)				\
static void type_as_function ## _class_init    (type ## Class *klass);	\
static void type_as_function ## _instance_init (type          *object);	\
static parent_type ## Class *parent_class = NULL;			\
static void								\
type_as_function ## _class_init_trampoline (gpointer klass,		\
					    gpointer data)		\
{									\
	parent_class = g_type_class_ref (parent_type_macro);		\
	type_as_function ## _class_init (klass);			\
}									\
GType									\
type_as_function ## _get_type (void)					\
{									\
	static GType object_type = 0;					\
	if (object_type == 0) {						\
		static const GTypeInfo object_info = {			\
		    sizeof (type ## Class),				\
		    NULL,		/* base_init */			\
		    NULL,		/* base_finalize */		\
		    type_as_function ## _class_init_trampoline,		\
		    NULL,		/* class_finalize */		\
		    NULL,               /* class_data */		\
		    sizeof (type),					\
		    0,                  /* n_preallocs */		\
		    (GInstanceInitFunc) type_as_function ## _instance_init \
		};							\
		object_type = register_type_macro			\
			(type, type_as_function, corba_type,		\
			 parent_type, parent_type_macro);		\
	}								\
	return object_type;						\
}

/* Just call the parent handler.  This assumes that there is a variable
 * named parent_class that points to the (duh!) parent class.  Note that
 * this macro is not to be used with things that return something, use
 * the _WITH_DEFAULT version for that */
#define BONOBO_CALL_PARENT(parent_class_cast, name, args)		\
	((parent_class_cast(parent_class)->name != NULL) ?		\
	 parent_class_cast(parent_class)->name args : (void)0)

/* Same as above, but in case there is no implementation, it evaluates
 * to def_return */
#define BONOBO_CALL_PARENT_WITH_DEFAULT(parent_class_cast,		\
					name, args, def_return)		\
	((parent_class_cast(parent_class)->name != NULL) ?		\
	 parent_class_cast(parent_class)->name args : def_return)


#define BONOBO_CLASS_BOILERPLATE(type, type_as_function,		\
				 parent_type, parent_type_macro)	\
	BONOBO_BOILERPLATE(type, type_as_function, type,		\
			   parent_type, parent_type_macro,		\
			   BONOBO_REGISTER_TYPE)
#define BONOBO_REGISTER_TYPE(type, type_as_function, corba_type,	\
			     parent_type, parent_type_macro)		\
	bonobo_type_unique (parent_type_macro, NULL, NULL, 0,		\
			    &object_info, #type)

#define BONOBO_CLASS_BOILERPLATE_FULL(type, type_as_function,		\
				      corba_type,			\
				      parent_type, parent_type_macro)	\
	BONOBO_BOILERPLATE(type, type_as_function, corba_type,		\
			   parent_type, parent_type_macro,		\
			   BONOBO_REGISTER_TYPE_FULL)
#define BONOBO_REGISTER_TYPE_FULL(type, type_as_function, corba_type,	\
				  parent_type, parent_type_macro)	\
	bonobo_type_unique (parent_type_macro,				\
			    POA_##corba_type##__init,			\
			    POA_##corba_type##__fini,			\
			    G_STRUCT_OFFSET (type##Class, epv),		\
			    &object_info, #type)

G_END_DECLS

/* Macros for defining classes.  Ideas taken from Nautilus and GOB. */

/* Define the boilerplate type stuff to reduce typos and code size.  Defines
 * the get_type method and the parent_class static variable. */
#define GNOME_CLASS_BOILERPLATE(type, type_as_function,			\
				parent_type, parent_type_macro)		\
	BONOBO_BOILERPLATE(type, type_as_function, type,		\
			  parent_type, parent_type_macro,		\
			  GNOME_REGISTER_TYPE)
#define GNOME_REGISTER_TYPE(type, type_as_function, corba_type,		\
			    parent_type, parent_type_macro)		\
	g_type_register_static (parent_type_macro, #type, &object_info, 0)

/* Just call the parent handler.  This assumes that there is a variable
 * named parent_class that points to the (duh!) parent class.  Note that
 * this macro is not to be used with things that return something, use
 * the _WITH_DEFAULT version for that */
#define GNOME_CALL_PARENT(parent_class_cast, name, args)		\
	BONOBO_CALL_PARENT (parent_class_cast, name, args)

/* Same as above, but in case there is no implementation, it evaluates
 * to def_return */
#define GNOME_CALL_PARENT_WITH_DEFAULT(parent_class_cast,		\
				       name, args, def_return)		\
	BONOBO_CALL_PARENT_WITH_DEFAULT (				\
		parent_class_cast, name, args, def_return)


/* FIXME: More includes! */

struct _GnomeAboutPrivate {
        gchar *name;
        gchar *version;
        gchar *copyright;
        gchar *comments;
        gchar *translator_credits;
        
        GSList *authors;
        GSList *documenters;
        
        GtkWidget *logo_image;
        GtkWidget *name_label;
        GtkWidget *comments_label;
        GtkWidget *copyright_label;

        GtkWidget *credits_dialog;
};

enum {
        PROP_0,
        PROP_NAME,
        PROP_VERSION,
        PROP_COPYRIGHT,
        PROP_COMMENTS,
        PROP_AUTHORS,
        PROP_DOCUMENTERS,
        PROP_TRANSLATOR_CREDITS,
        PROP_LOGO,
};

#define GNOME_RESPONSE_CREDITS 1

static void gnome_about_finalize (GObject *object);
static void gnome_about_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void gnome_about_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

GNOME_CLASS_BOILERPLATE (GnomeAbout, gnome_about,
                         GtkDialog, GTK_TYPE_DIALOG)

static void
gnome_about_update_authors_label (GnomeAbout *about, GtkWidget *label)
{
        GString *string;
        GSList *list;
        gchar *tmp;
        
        if (about->_priv->authors == NULL) {
                gtk_widget_hide (label);
                return;
        }
        else {
                gtk_widget_show (label);
        }

        string = g_string_new ("");

        for (list = about->_priv->authors; list; list = list->next) {
                tmp = g_markup_escape_text (list->data, -1);
                g_string_append (string, tmp);

                if (list->next)
                        g_string_append (string, "\n");
                g_free (tmp);
        }
        
        gtk_label_set_markup (GTK_LABEL (label), string->str);
        g_string_free (string, TRUE);
}

static void
gnome_about_update_documenters_label (GnomeAbout *about, GtkWidget *label)
{
        GString *string;
        GSList *list;
        gchar *tmp;
                
        if (about->_priv->documenters == NULL) {
                gtk_widget_hide (label);
                return;
        }
        else {
                gtk_widget_show (label);
        }

        string = g_string_new ("");

        for (list = about->_priv->documenters; list; list = list->next) {
                tmp = g_markup_escape_text (list->data, -1);
                g_string_append (string, tmp);

                if (list->next)
                        g_string_append (string, "\n");
                g_free (tmp);
        }
        
        gtk_label_set_markup (GTK_LABEL (label), string->str);
        g_string_free (string, TRUE);
}

static void
gnome_about_update_translation_information_label (GnomeAbout *about, GtkWidget *label)
{
        GString *string;
        gchar *tmp;
        
        if (about->_priv->translator_credits == NULL) {
                gtk_widget_hide (label);
                return;
        }
        else {
                gtk_widget_show (label);
        }

        string = g_string_new ("");

        tmp = g_markup_escape_text (about->_priv->translator_credits, -1);
        g_string_append (string, tmp);
        g_free (tmp);
        
        gtk_label_set_markup (GTK_LABEL (label), string->str);
        g_string_free (string, TRUE);
}

static GtkWidget *
create_label (void)
{
        GtkWidget *label;
        
        label = gtk_label_new ("");
        gtk_label_set_selectable (GTK_LABEL (label), TRUE);
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
        gtk_misc_set_padding (GTK_MISC (label), 8, 8);

        gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

        return label;
}

static void
gnome_about_display_credits_dialog (GnomeAbout *about)
{
        GtkWidget *dialog, *label, *notebook, *sw;

        if (about->_priv->credits_dialog != NULL) {
                gtk_window_present (GTK_WINDOW (about->_priv->credits_dialog));
                return;
        }
        
        dialog = gtk_dialog_new_with_buttons ("Credits",
                                              GTK_WINDOW (about),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_OK, GTK_RESPONSE_OK,
                                              NULL);
        about->_priv->credits_dialog = dialog;
        gtk_window_set_default_size (GTK_WINDOW (dialog), 360, 260);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
        g_signal_connect (dialog, "response",
                          G_CALLBACK (gtk_widget_destroy), dialog);
        g_signal_connect (dialog, "destroy",
                          G_CALLBACK (gtk_widget_destroyed),
                          &(about->_priv->credits_dialog));

        notebook = gtk_notebook_new ();
        gtk_container_set_border_width (GTK_CONTAINER (notebook), 8);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), notebook, TRUE, TRUE, 0);

        if (about->_priv->authors != NULL) {
                label = create_label ();

                sw = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                                GTK_POLICY_AUTOMATIC,
                                                GTK_POLICY_AUTOMATIC);
                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), label);
                gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (sw)->child), GTK_SHADOW_NONE);
                
                gtk_notebook_append_page (GTK_NOTEBOOK (notebook), sw,
                                          gtk_label_new_with_mnemonic ("_Written by"));
                gnome_about_update_authors_label (about, label);
        }

        if (about->_priv->documenters != NULL) {
                label = create_label ();
                
                sw = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                                GTK_POLICY_AUTOMATIC,
                                                GTK_POLICY_AUTOMATIC);
                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), label);
                gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (sw)->child), GTK_SHADOW_NONE);

                gtk_notebook_append_page (GTK_NOTEBOOK (notebook), sw,
                                          gtk_label_new_with_mnemonic ("_Documented by"));
                gnome_about_update_documenters_label (about, label);
        }

        if (about->_priv->translator_credits != NULL) {
                label = create_label ();
                
                sw = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                                GTK_POLICY_AUTOMATIC,
                                                GTK_POLICY_AUTOMATIC);
                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), label);
                gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (sw)->child), GTK_SHADOW_NONE);

                gtk_notebook_append_page (GTK_NOTEBOOK (notebook), sw,
                                          gtk_label_new_with_mnemonic ("_Translated by"));
                gnome_about_update_translation_information_label (about, label);
        }
        
        gtk_widget_show_all (dialog);
}

static void
gnome_about_instance_init (GnomeAbout *about)
{
        GnomeAboutPrivate *priv;
        GtkWidget *vbox, *button;

        /* Data */
        
        priv = g_new0 (GnomeAboutPrivate, 1);
        about->_priv = priv;

        priv->name = NULL;
        priv->version = NULL;
        priv->copyright = NULL;
        priv->comments = NULL;
        priv->translator_credits = NULL;
        priv->authors = NULL;
        priv->documenters = NULL;

        /* Widgets */
        vbox = gtk_vbox_new (FALSE, 8);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);

        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (about)->vbox), vbox, TRUE, TRUE, 0);

        priv->logo_image = gtk_image_new ();
        gtk_box_pack_start (GTK_BOX (vbox), priv->logo_image, FALSE, FALSE, 0);
        priv->name_label = gtk_label_new (NULL);
        gtk_label_set_selectable (GTK_LABEL (priv->name_label), TRUE);
        gtk_label_set_justify (GTK_LABEL (priv->name_label), GTK_JUSTIFY_CENTER);
        gtk_box_pack_start (GTK_BOX (vbox), priv->name_label, FALSE, FALSE, 0);

        priv->comments_label = gtk_label_new (NULL);
        gtk_label_set_selectable (GTK_LABEL (priv->comments_label), TRUE);
        gtk_label_set_justify (GTK_LABEL (priv->comments_label), GTK_JUSTIFY_CENTER);
        gtk_label_set_line_wrap (GTK_LABEL (priv->comments_label), TRUE);
        gtk_box_pack_start (GTK_BOX (vbox), priv->comments_label, FALSE, FALSE, 0);

        priv->copyright_label = gtk_label_new (NULL);
        gtk_label_set_selectable (GTK_LABEL (priv->copyright_label), TRUE);     
        gtk_label_set_justify (GTK_LABEL (priv->copyright_label), GTK_JUSTIFY_CENTER);
        gtk_box_pack_start (GTK_BOX (vbox), priv->copyright_label, FALSE, FALSE, 0);

        gtk_widget_show_all (vbox);
        
        /* Add the OK button */
        gtk_dialog_add_button (GTK_DIALOG (about), GTK_STOCK_OK, GTK_RESPONSE_OK);
        gtk_dialog_set_default_response (GTK_DIALOG (about), GTK_RESPONSE_OK);

        /* Add the credits button */
        button = gtk_dialog_add_button (GTK_DIALOG (about), "_Credits", GNOME_RESPONSE_CREDITS);
        gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (GTK_DIALOG (about)->action_area), button, TRUE);
        
        gtk_window_set_resizable (GTK_WINDOW (about), FALSE);

        priv->credits_dialog = NULL;
}

static void
gnome_about_response (GtkDialog *dialog, gint response)
{
        switch (response) {
        case GNOME_RESPONSE_CREDITS:
                gnome_about_display_credits_dialog (GNOME_ABOUT (dialog));
                break;
        default:
                gtk_widget_destroy (GTK_WIDGET (dialog));
                break;
        }
}

static void
gnome_about_class_init (GnomeAboutClass *klass)
{
        GObjectClass *object_class;
        GtkWidgetClass *widget_class;
        GtkDialogClass *dialog_class;
        
        object_class = (GObjectClass *)klass;
        widget_class = (GtkWidgetClass *)klass;
        dialog_class = (GtkDialogClass *)klass;
        
        object_class->set_property = gnome_about_set_property;
        object_class->get_property = gnome_about_get_property;

        object_class->finalize = gnome_about_finalize;

        dialog_class->response = gnome_about_response;
        
        g_object_class_install_property (object_class,
                                         PROP_NAME,
                                         g_param_spec_string ("name",
                                                              "Program name",
                                                              "The name of the program",
                                                              NULL,
                                                              G_PARAM_READWRITE));

        g_object_class_install_property (object_class,
                                         PROP_VERSION,
                                         g_param_spec_string ("version",
                                                              "Program version",
                                                              "The version of the program",
                                                              NULL,
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_COPYRIGHT,
                                         g_param_spec_string ("copyright",
                                                              "Copyright string",
                                                              "Copyright information for the program",
                                                              NULL,
                                                              G_PARAM_READWRITE));
        
        g_object_class_install_property (object_class,
                                         PROP_COMMENTS,
                                         g_param_spec_string ("comments",
                                                              "Comments string",
                                                              "Comments about the program",
                                                              NULL,
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_AUTHORS,
                                         g_param_spec_value_array ("authors",
                                                                   "Authors",
                                                                   "List of authors of the programs",
                                                                   g_param_spec_string ("author-entry",
                                                                                        "Author entry",
                                                                                        "A single author entry",
                                                                                        NULL,
                                                                                        G_PARAM_READWRITE),
                                                                   G_PARAM_WRITABLE));
        g_object_class_install_property (object_class,
                                         PROP_DOCUMENTERS,
                                         g_param_spec_value_array ("documenters",
                                                                   "Documenters",
                                                                   "List of people documenting the program",
                                                                   g_param_spec_string ("documenter-entry",
                                                                                        "Documenter entry",
                                                                                        "A single documenter entry",
                                                                                        NULL,
                                                                                        G_PARAM_READWRITE),
                                                                   G_PARAM_WRITABLE));

        g_object_class_install_property (object_class,
                                         PROP_TRANSLATOR_CREDITS,
                                         g_param_spec_string ("translator_credits",
                                                              "Translator credits",
                                                              "Credits to the translators. This string should be marked as translatable",
                                                              NULL,
                                                              G_PARAM_READWRITE));
        
        g_object_class_install_property (object_class,
                                         PROP_LOGO,
                                         g_param_spec_object ("logo",
                                                              "Logo",
                                                              "A logo for the about box",
                                                              GDK_TYPE_PIXBUF,
                                                              G_PARAM_WRITABLE));

}

static void
gnome_about_set_comments (GnomeAbout *about, const gchar *comments)
{
        g_free (about->_priv->comments);
        about->_priv->comments = comments ? g_strdup (comments) : NULL;

        gtk_label_set_text (GTK_LABEL (about->_priv->comments_label), about->_priv->comments);
}

static void
gnome_about_set_translator_credits (GnomeAbout *about, const gchar *translator_credits)
{
        g_free (about->_priv->translator_credits);

        about->_priv->translator_credits = g_strdup (translator_credits);
}

static void
gnome_about_set_copyright (GnomeAbout *about, const gchar *copyright)
{
        char *copyright_string, *tmp;
        
        g_free (about->_priv->copyright);
        about->_priv->copyright = copyright ? g_strdup (copyright) : NULL;

        if (about->_priv->copyright != NULL) {
                tmp = g_markup_escape_text (about->_priv->copyright, -1);
                copyright_string = g_strdup_printf ("<span size=\"small\">%s</span>", tmp);
                g_free (tmp);
        }
        else {
                copyright_string = NULL;
        }

        gtk_label_set_markup (GTK_LABEL (about->_priv->copyright_label), copyright_string);

        g_free (copyright_string);
}

static void
gnome_about_set_version (GnomeAbout *about, const gchar *version)
{
        gchar *name_string, *tmp_name, *tmp_version;
        
        g_free (about->_priv->version);
        about->_priv->version = version ? g_strdup (version) : NULL;

        tmp_name = g_markup_escape_text (about->_priv->name, -1);
        
        if (about->_priv->version != NULL) {
                tmp_version = g_markup_escape_text (about->_priv->version, -1);
                name_string = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">%s %s</span>", tmp_name, tmp_version);
                g_free (tmp_version);
        }
        else {
                name_string = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">%s</span>", tmp_name);
        }

        gtk_label_set_markup (GTK_LABEL (about->_priv->name_label), name_string);
        g_free (name_string);
        g_free (tmp_name);
}

static void
gnome_about_set_name (GnomeAbout *about, const gchar *name)
{
        gchar *title_string;
        gchar *name_string;
        gchar *tmp_name, *tmp_version;
        
        g_free (about->_priv->name);
        about->_priv->name = g_strdup (name ? name : "");

        title_string = g_strdup_printf ("About %s", name);
        gtk_window_set_title (GTK_WINDOW (about), title_string);
        g_free (title_string);

        tmp_name = g_markup_escape_text (about->_priv->name, -1);

        if (about->_priv->version != NULL) {
                tmp_version = g_markup_escape_text (about->_priv->version, -1);
                name_string = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">%s %s</span>", tmp_name, tmp_version);
                g_free (tmp_version);
        }
        else {
                name_string = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">%s</span>", tmp_name);
        }

        gtk_label_set_markup (GTK_LABEL (about->_priv->name_label), name_string);
        g_free (name_string);
        g_free (tmp_name);
}

static void
gnome_about_free_person_list (GSList *list)
{
        if (list == NULL)
                return;
        
        g_slist_foreach (list, (GFunc) g_free, NULL);
        g_slist_free (list);
}

static void
gnome_about_finalize (GObject *object)
{
        GnomeAbout *about = GNOME_ABOUT (object);

        g_free (about->_priv->name);
        g_free (about->_priv->version);
        g_free (about->_priv->copyright);
        g_free (about->_priv->comments);

        gnome_about_free_person_list (about->_priv->authors);
        gnome_about_free_person_list (about->_priv->documenters);

        g_free (about->_priv->translator_credits);

        g_free (about->_priv);
        about->_priv = NULL;

        GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gnome_about_set_persons (GnomeAbout *about, guint prop_id, const GValue *persons)
{
                GValueArray *value_array;
        gint i;
        GSList *list;

        /* Free the old list */
        switch (prop_id) {
        case PROP_AUTHORS:
                list = about->_priv->authors;
                break;
        case PROP_DOCUMENTERS:
                list = about->_priv->documenters;
                break;
        default:
                g_assert_not_reached ();
                list = NULL; /* silence warning */
        }

        gnome_about_free_person_list (list);
        list = NULL;
        
        value_array = g_value_get_boxed (persons);

        if (value_array == NULL) {
                return;
        }
        
        for (i = 0; i < value_array->n_values; i++) {
                list = g_slist_prepend (list, g_value_dup_string (&value_array->values[i]));
        }

        list = g_slist_reverse (list);
        
        switch (prop_id) {
        case PROP_AUTHORS:
                about->_priv->authors = list;
                break;
        case PROP_DOCUMENTERS:
                about->_priv->documenters = list;
                break;
        default:
                g_assert_not_reached ();
        }
}

static void
set_value_array_from_list (GValue *value, GSList *list)
{
        GValueArray *array;
        GValue tmp_value = { 0 };
        GSList *tmp;
        gint length;

        length = g_slist_length (list);
        array = g_value_array_new (length);

        for (tmp = list; tmp; tmp = tmp->next) {
                char *str = tmp->data;
                
                g_value_init (&tmp_value, G_TYPE_STRING);
                g_value_set_string (&tmp_value, str);
                g_value_array_append (array, &tmp_value);
        }

        g_value_set_boxed (value, array);
        g_value_array_free (array);
}

static void
gnome_about_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
        switch (prop_id) {
        case PROP_NAME:
                gnome_about_set_name (GNOME_ABOUT (object), g_value_get_string (value));
                break;
        case PROP_VERSION:
                gnome_about_set_version (GNOME_ABOUT (object), g_value_get_string (value));
                break;
        case PROP_COMMENTS:
                gnome_about_set_comments (GNOME_ABOUT (object), g_value_get_string (value));
                break;
        case PROP_COPYRIGHT:
                gnome_about_set_copyright (GNOME_ABOUT (object), g_value_get_string (value));
                break;
        case PROP_LOGO:
                if (g_value_get_object (value) != NULL) {
                        gtk_image_set_from_pixbuf (GTK_IMAGE (GNOME_ABOUT (object)->_priv->logo_image),
                                                   g_value_get_object (value));
                }
#if 0
                else {
                        gtk_image_set_from_file (GTK_IMAGE (GNOME_ABOUT (object)->_priv->logo_image),
                                                 GNOMEUIDATADIR"/pixmaps/gnome-about-logo.png");
                }
#endif
                break;
        case PROP_AUTHORS:
        case PROP_DOCUMENTERS:
                gnome_about_set_persons (GNOME_ABOUT (object), prop_id, value);
                break;
        case PROP_TRANSLATOR_CREDITS:
                gnome_about_set_translator_credits (GNOME_ABOUT (object), g_value_get_string (value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

static void
gnome_about_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
        GnomeAbout *about;

        about = GNOME_ABOUT (object);
        
        switch (prop_id) {
        case PROP_NAME:
                g_value_set_string (value, about->_priv->name);
                break;
        case PROP_VERSION:
                g_value_set_string (value, about->_priv->version);
                break;
        case PROP_COPYRIGHT:
                g_value_set_string (value, about->_priv->copyright);
                break;
        case PROP_COMMENTS:
                g_value_set_string (value, about->_priv->comments);
                break;
        case PROP_TRANSLATOR_CREDITS:
                g_value_set_string (value, about->_priv->translator_credits);
                break;
        case PROP_AUTHORS:
                set_value_array_from_list (value, about->_priv->authors);
                break;
        case PROP_DOCUMENTERS:
                set_value_array_from_list (value, about->_priv->documenters);
                break;
        case PROP_LOGO:
                g_value_set_object (value, gtk_image_get_pixbuf (GTK_IMAGE (about->_priv->logo_image)));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

/**
 * gnome_about_new:
 * @name: The name of the application.
 * @version: The version string of the application.
 * @copyright: The application's copyright statement.
 * @comments: A short miscellaneous string.
 * @authors: An %NULL terminated array of the application authors.
 * @documenters: An array of the application documenters.
 * @translator_credits: The translator for the current locale.
 * @logo_pixbuf: The application's logo.
 *
 * Construct an application's credits box. The @authors array canot be empty
 * and the @translator_credits should be marked as a translatable string (so
 * that only the translator for the currently active locale is displayed).
 *
 * Returns: A new "About" dialog.
 */
GtkWidget *
gnome_about_new (const gchar  *name,
                 const gchar  *version,
                 const gchar  *copyright,
                 const gchar  *comments,
                 const gchar **authors,
                 const gchar **documenters,
                 const gchar  *translator_credits,
                 GdkPixbuf    *logo_pixbuf)
{
        GnomeAbout *about;
        
        g_return_val_if_fail (authors != NULL, NULL);
        
        about = g_object_new (GNOME_TYPE_ABOUT, NULL);
        gnome_about_construct(about, 
                              name, version, copyright, 
                              comments, authors, documenters, 
                              translator_credits, logo_pixbuf);

        return GTK_WIDGET(about);
}

/**
 * gnome_about_construct:
 * @about: An existing #GnomeAbout instance.
 * @name: The name of the application.
 * @version: The version string of the application.
 * @copyright: The application's copyright statement.
 * @comments: A short miscellaneous string.
 * @authors: An %NULL terminated array of the application authors.
 * @documenters: An array of the application documenters.
 * @translator_credits: The translator for the current locale.
 * @logo_pixbuf: The application's logo.
 *
 * Similar to gnome_about_new() except that the pre-existing @about widget is
 * used. Note that in this version of the function, @authors is not checked to
 * be non-%NULL, so callers must be careful, since bad things will happen if
 * this condition is not met.
 */
void
gnome_about_construct (GnomeAbout *about,
                       const gchar  *name,
                       const gchar  *version,
                       const gchar  *copyright,
                       const gchar  *comments,
                       const gchar **authors,
                       const gchar **documenters,
                       const gchar  *translator_credits,
                       GdkPixbuf    *logo_pixbuf)
{
        GValueArray *authors_array;
        GValueArray *documenters_array;
        gint i;

        authors_array = g_value_array_new (0);
        
        for (i = 0; authors[i] != NULL; i++) {
                GValue value = {0, };
                
                g_value_init (&value, G_TYPE_STRING);
                        g_value_set_static_string (&value, authors[i]);
                        authors_array = g_value_array_append (authors_array, &value);
        }

        if (documenters != NULL) {
                documenters_array = g_value_array_new (0);

                for (i = 0; documenters[i] != NULL; i++) {
                        GValue value = {0, };
                        
                        g_value_init (&value, G_TYPE_STRING);
                        g_value_set_static_string (&value, documenters[i]);
                        documenters_array = g_value_array_append (documenters_array, &value);
                }

        }
        else {
                documenters_array = NULL;
        }

        g_object_set (G_OBJECT (about),
                      "name", name,
                      "version", version,
                      "copyright", copyright,
                      "comments", comments,
                      "authors", authors_array,
                      "documenters", documenters_array,
                      "translator_credits", translator_credits,
                      "logo", logo_pixbuf,
                      NULL);

        if (authors_array != NULL) {
                g_value_array_free (authors_array);
        }
        if (documenters_array != NULL) {
                g_value_array_free (documenters_array);
        }
}



