/*
 * nls.h --- i18n-isation 
 *
 * Copyright (C) 2002 Raymond Penners
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 * $Id$
 *
 */

#include "config.h"

#include <glib/glist.h>
#include "nls.h"

#if defined(HAVE_GNOME)
#include <libgnome/gnome-i18n.h>
#elif defined(G_OS_WIN32)
#include <glib/gwin32.h>
#endif


#if !defined(HAVE_GNOME) && !defined(G_OS_WIN32)
static GHashTable *category_table= NULL;

/* The following is (partly) taken from the gettext package.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */
static const gchar *
guess_category_value (const gchar *categoryname)
{
  const gchar *retval;

  /* The highest priority value is the `LANGUAGE' environment
     variable.  This is a GNU extension.  */
  retval = g_getenv ("LANGUAGE");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* `LANGUAGE' is not set.  So we have to proceed with the POSIX
     methods of looking to `LC_ALL', `LC_xxx', and `LANG'.  On some
     systems this can be done by the `setlocale' function itself.  */

  /* Setting of LC_ALL overwrites all other.  */
  retval = g_getenv ("LC_ALL");  
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* Next comes the name of the desired category.  */
  retval = g_getenv (categoryname);
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* Last possibility is the LANG environment variable.  */
  retval = g_getenv ("LANG");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  return NULL;
}
#endif





/* The following is (partly) taken from the libgnome package.
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.  
 */
const GList *
nls_get_language_list (const gchar *category_name)
{
#if defined(HAVE_GNOME)

  return gnome_i18n_get_language_list("LC_MESSAGES");

#elif defined(G_OS_WIN32)

  static GList *language_list = NULL;
  (void) category_name;
  if (! language_list)
    {
      gchar *locale = g_win32_getlocale();
      /* This list is never freed, I know.. */
      language_list = g_list_append(NULL, locale);
    }
  return language_list;

#else

  GList *list;

  if (!category_name)
    category_name= "LC_ALL";

  if (category_table)
    {
      list= g_hash_table_lookup (category_table, (const gpointer) category_name);
    }
  else
    {
      category_table= g_hash_table_new (g_str_hash, g_str_equal);
      list= NULL;
    }

  if (!list)
    {
      gint c_locale_defined= FALSE;
  
      const gchar *category_value;
      gchar *category_memory, *orig_category_memory;

      category_value = guess_category_value (category_name);
      if (! category_value)
	category_value = "C";
      orig_category_memory = category_memory =
	g_malloc (strlen (category_value)+1);
      
      while (category_value[0] != '\0')
	{
	  while (category_value[0] != '\0' && category_value[0] == ':')
	    ++category_value;
	  
	  if (category_value[0] != '\0')
	    {
	      char *cp= category_memory;
	      
	      while (category_value[0] != '\0' && category_value[0] != ':')
		*category_memory++= *category_value++;
	      
	      category_memory[0]= '\0'; 
	      category_memory++;
	      
	      cp = unalias_lang(cp);
	      
	      if (strcmp (cp, "C") == 0)
		c_locale_defined= TRUE;
	      
	      list= g_list_concat (list, compute_locale_variants (cp));
	    }
	}
      g_free (orig_category_memory);
      
      if (!c_locale_defined)
	list= g_list_append (list, "C");

      g_hash_table_insert (category_table, (gpointer) category_name, list);
    }
  return list;

#endif
}


