// Copyright (C) 2003 - 2011 Rob Caelers <robc@krandor.nl>
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

#include "config.h"
#include "WorkraveApplet.h"

#include <glib/gi18n-lib.h>
#include <libgnome-panel/gp-module.h>

static GpAppletInfo *
get_applet_info(const gchar *id)
{
  return gp_applet_info_new(workrave_applet_get_type, _("Workrave"), _("Workrave Applet"), "workrave");
}

static const gchar *
get_applet_id_from_iid(const gchar *iid)
{
  if (g_strcmp0(iid, "WorkraveAppletFactory::WorkraveApplet") == 0)
    return "workrave";

  return NULL;
}

void
gp_module_load(GpModule *module)
{
  bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  gp_module_set_gettext_domain(module, GETTEXT_PACKAGE);

  gp_module_set_abi_version(module, GP_MODULE_ABI_VERSION);

  gp_module_set_id(module, "org.workrave.workrave-applet");
  gp_module_set_version(module, WORKRAVE_VERSION);

  gp_module_set_applet_ids(module, "workrave", NULL);

  gp_module_set_get_applet_info(module, get_applet_info);
  gp_module_set_compatibility(module, get_applet_id_from_iid);
}
