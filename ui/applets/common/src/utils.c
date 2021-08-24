/* Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "utils.h"

void
workrave_extract_shortcut(const gchar *text, gchar **out, gchar *shortcut)
{
  gboolean underscore = FALSE;

  *shortcut = '\0';
  *out = g_new(gchar, strlen(text) + 1);

  const gchar *src = text;
  gchar *dst = *out;

  for (; *src != '\0'; src++)
    {
      gchar ch = *src;
      if (underscore)
        {
          if (ch != '_' && *shortcut == '\0')
            {
              *shortcut = ch;
            }
          underscore = FALSE;
          *dst++ = ch;
        }
      else
        {
          if (ch == '_')
            {
              underscore = TRUE;
            }
          else
            {
              *dst++ = ch;
            }
        }
    }
  *dst++ = '\0';
}
