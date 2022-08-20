/*
 * nls.h --- i18n-isation
 *
 * Copyright (C) 2002, 2003, 2006, 2008, 2009 Raymond Penners
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WORKRAVE_UI_COMMON_NLS_H
#define WORKRAVE_UI_COMMON_NLS_H

#if defined(HAVE_QT)
#  include <QObject>
#  define _(String) QObject::tr(String).toUtf8().constData()
#elif defined(HAVE_LIBINTL)
#  include <locale.h>
#  include <libintl.h>
// libintl #defines 'snprintf' to 'libintl_snprintf'
// As a result, 'std::snprintf' becomes 'std::libintl_snprintf'.
// This results in compilation errors.
#  undef snprintf
#  undef sprintf
#  define _(String) ((const char *)gettext(String))
#  if defined(gettext_noop)
#    define N_(String) gettext_noop(String)
#  else
#    define N_(String) (String)
#  endif
#else
/* Stubs that do something close enough.  */
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain, Message) (Message)
#  define dcgettext(Domain, Message, Type) (Message)
#  define bindtextdomain(Domain, Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#endif // WORKRAVE_UI_COMMON_NLS_H
