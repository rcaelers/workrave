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

#ifndef WORKRAVE_NLS_H
#define WORKRAVE_NLS_H

#ifdef HAVE_GNOME
#  include <gnome.h>
#else
#  ifdef ENABLE_NLS
#    include <locale.h>
#      include <libintl.h>
#      define _(String) gettext (String)
#      ifdef gettext_noop
#          define N_(String) gettext_noop (String)
#      else
#          define N_(String) (String)
#      endif
#  else
/* Stubs that do something close enough.  */
#      define textdomain(String) (String)
#      define gettext(String) (String)
#      define dgettext(Domain,Message) (Message)
#      define dcgettext(Domain,Message,Type) (Message)
#      define bindtextdomain(Domain,Directory) (Domain)
#      define _(String) (String)
#      define N_(String) (String)
#  endif
#endif

#endif // WORKRAVE_NLS_H
