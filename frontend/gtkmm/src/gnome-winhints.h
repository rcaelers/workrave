/* gnome-winhints.h: Copyright (C) 1998 Free Software Foundation
 * Convenience functions for working with XA_WIN_* hints.
 *
 * Written by: M.Watson <redline@pdq.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Cambridge, MA 02139, USA.
 */

#ifndef GNOME_WINHINTS_H
#define GNOME_WINHINTS_H

#ifdef HAVE_X

#ifdef __cplusplus
extern              "C"
{
#endif

//#include <libgnome/gnome-defs.h>
#include <gtk/gtkwidget.h>

/* The hints we recognize */
#define XA_WIN_PROTOCOLS           "_WIN_PROTOCOLS"
#define XA_WIN_ICONS               "_WIN_ICONS"
#define XA_WIN_WORKSPACE           "_WIN_WORKSPACE"
#define XA_WIN_WORKSPACE_COUNT     "_WIN_WORKSPACE_COUNT"
#define XA_WIN_WORKSPACE_NAMES     "_WIN_WORKSPACE_NAMES"    
#define XA_WIN_LAYER               "_WIN_LAYER"
#define XA_WIN_STATE               "_WIN_STATE"
#define XA_WIN_HINTS               "_WIN_HINTS"
#define XA_WIN_WORKAREA            "_WIN_WORKAREA"
#define XA_WIN_CLIENT_LIST         "_WIN_CLIENT_LIST"
#define XA_WIN_APP_STATE           "_WIN_APP_STATE"
#define XA_WIN_EXPANDED_SIZE       "_WIN_EXPANDED_SIZE"
#define XA_WIN_CLIENT_MOVING       "_WIN_CLIENT_MOVING"
#define XA_WIN_SUPPORTING_WM_CHECK "_WIN_SUPPORTING_WM_CHECK"

/* flags for the window layer */
typedef enum
{
  WIN_LAYER_DESKTOP     = 0,
  WIN_LAYER_BELOW       = 2,
  WIN_LAYER_NORMAL      = 4,
  WIN_LAYER_ONTOP       = 6,
  WIN_LAYER_DOCK        = 8,
  WIN_LAYER_ABOVE_DOCK  = 10
} GnomeWinLayer;

/* flags for the window state */
typedef enum
{
  WIN_STATE_STICKY          = (1<<0), /* everyone knows sticky */
  WIN_STATE_MINIMIZED       = (1<<1), /* ??? */
  WIN_STATE_MAXIMIZED_VERT  = (1<<2), /* window in maximized V state */
  WIN_STATE_MAXIMIZED_HORIZ = (1<<3), /* window in maximized H state */
  WIN_STATE_HIDDEN          = (1<<4), /* not on taskbar but window visible */
  WIN_STATE_SHADED          = (1<<5), /* shaded (NeXT style) */
  WIN_STATE_HID_WORKSPACE   = (1<<6), /* not on current desktop */
  WIN_STATE_HID_TRANSIENT   = (1<<7), /* owner of transient is hidden */
  WIN_STATE_FIXED_POSITION  = (1<<8), /* window is fixed in position even */
  WIN_STATE_ARRANGE_IGNORE  = (1<<9)  /* ignore for auto arranging */
} GnomeWinState;

/* flags for skip hint */
typedef enum
{
  WIN_HINTS_SKIP_FOCUS      = (1<<0), /* "alt-tab" skips this win */
  WIN_HINTS_SKIP_WINLIST    = (1<<1), /* not in win list */
  WIN_HINTS_SKIP_TASKBAR    = (1<<2), /* not on taskbar */
  WIN_HINTS_GROUP_TRANSIENT = (1<<3), /* ??????? */
  WIN_HINTS_FOCUS_ON_CLICK  = (1<<4), /* app only accepts focus when clicked */
  WIN_HINTS_DO_NOT_COVER    = (1<<5)  /* attempt to not cover this window */
} GnomeWinHints;

typedef enum
{
  WIN_APP_STATE_NONE,
  WIN_APP_STATE_ACTIVE1,
  WIN_APP_STATE_ACTIVE2,
  WIN_APP_STATE_ERROR1,
  WIN_APP_STATE_ERROR2,
  WIN_APP_STATE_FATAL_ERROR1,
  WIN_APP_STATE_FATAL_ERROR2,
  WIN_APP_STATE_IDLE1,
  WIN_APP_STATE_IDLE2,
  WIN_APP_STATE_WAITING1,
  WIN_APP_STATE_WAITING2,
  WIN_APP_STATE_WORKING1,
  WIN_APP_STATE_WORKING2,
  WIN_APP_STATE_NEED_USER_INPUT1,
  WIN_APP_STATE_NEED_USER_INPUT2,
  WIN_APP_STATE_STRUGGLING1,
  WIN_APP_STATE_STRUGGLING2,
  WIN_APP_STATE_DISK_TRAFFIC1,
  WIN_APP_STATE_DISK_TRAFFIC2,
  WIN_APP_STATE_NETWORK_TRAFFIC1,
  WIN_APP_STATE_NETWORK_TRAFFIC2,
  WIN_APP_STATE_OVERLOADED1,
  WIN_APP_STATE_OVERLOADED2,
  WIN_APP_STATE_PERCENT000_1,
  WIN_APP_STATE_PERCENT000_2,
  WIN_APP_STATE_PERCENT010_1,
  WIN_APP_STATE_PERCENT010_2,
  WIN_APP_STATE_PERCENT020_1,
  WIN_APP_STATE_PERCENT020_2,
  WIN_APP_STATE_PERCENT030_1,
  WIN_APP_STATE_PERCENT030_2,
  WIN_APP_STATE_PERCENT040_1,
  WIN_APP_STATE_PERCENT040_2,
  WIN_APP_STATE_PERCENT050_1,
  WIN_APP_STATE_PERCENT050_2,
  WIN_APP_STATE_PERCENT060_1,
  WIN_APP_STATE_PERCENT060_2,
  WIN_APP_STATE_PERCENT070_1,
  WIN_APP_STATE_PERCENT070_2,
  WIN_APP_STATE_PERCENT080_1,
  WIN_APP_STATE_PERCENT080_2,
  WIN_APP_STATE_PERCENT090_1,
  WIN_APP_STATE_PERCENT090_2,
  WIN_APP_STATE_PERCENT100_1,
  WIN_APP_STATE_PERCENT100_2
} GnomeWinAppState;

/* Called during gnome initiailisation */
void
gnome_win_hints_init(void);

void
gnome_win_hints_set_layer(GtkWidget *window, GnomeWinLayer layer);
GnomeWinLayer
gnome_win_hints_get_layer(GtkWidget *window);


void
gnome_win_hints_set_state(GtkWidget *window, GnomeWinState state);
GnomeWinState
gnome_win_hints_get_state(GtkWidget *window);

void
gnome_win_hints_set_hints(GtkWidget *window, GnomeWinHints skip);
GnomeWinHints
gnome_win_hints_get_hints(GtkWidget *window);

void
gnome_win_hints_set_workspace(GtkWidget *window, gint workspace);
gint
gnome_win_hints_get_workspace(GtkWidget *window);

void
gnome_win_hints_set_current_workspace(gint workspace);
gint
gnome_win_hints_get_current_workspace(void);

GList*
gnome_win_hints_get_workspace_names(void);
gint
gnome_win_hints_get_workspace_count(void);

void
gnome_win_hints_set_expanded_size(GtkWidget *window, gint x, gint y, gint width, gint height);
gboolean
gnome_win_hints_get_expanded_size(GtkWidget *window, gint *x, gint *y, gint *width, gint *height);

void
gnome_win_hints_set_moving(GtkWidget *window, gboolean moving);

void
gnome_win_hints_set_app_state(GtkWidget *window,  GnomeWinAppState state);
GnomeWinAppState
gnome_win_hints_get_app_state(GtkWidget *window);

void
gnome_win_hints_set_moving(GtkWidget *window, gboolean moving);

gboolean
gnome_win_hints_wm_exists(void);

GList*
gnome_win_hints_get_client_window_ids(void);


#ifdef __cplusplus
}
#endif

#endif

#endif
