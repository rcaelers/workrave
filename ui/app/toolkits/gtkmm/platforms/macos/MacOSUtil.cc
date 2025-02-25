// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Based on code from ggmud
// Copyright Gabry (gabrielegreco@gmail.com)

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "debug.hh"
#include <glib.h>

#include <mach-o/dyld.h>
#include <sys/param.h>
#include <fstream>

#include "MacOSUtil.hh"

void
MacOSUtil::init()
{
  char execpath[MAXPATHLEN + 1];
  char path[MAXPATHLEN * 4];
  FILE *f;
  uint32_t pathsz = sizeof(execpath);

  _NSGetExecutablePath(execpath, &pathsz);
  gchar *dir_path = g_path_get_dirname(execpath);
  strcpy(path, dir_path);

  // Gtk
  strcat(path, "/../Resources/themes/Leopardish-normal");
  setenv("GTK_PATH", path, 1);

  // Locale
  strcat(path + strlen(dir_path), "/../Resources/locale");

  // write a pango.rc file and tell pango to use it
  strcpy(path + strlen(dir_path), "/../Resources/pango.rc");
  if ((f = fopen(path, "w")))
    {
      fprintf(f, "[Pango]\nModuleFiles=%s/../Resources/pango.modules\n", dir_path);
      fclose(f);
      setenv("PANGO_RC_FILE", path, 1);
    }

  // gettext charset aliases
  setenv("CHARSETALIASDIR", path, 1);

  // font config
  strcpy(path + strlen(dir_path), "/../Resources/etc/fonts/fonts.conf");
  setenv("FONTCONFIG_FILE", path, 1);

  // GDK Pixbuf loader module file
  strcpy(path + strlen(dir_path), "/../Resources/etc/gtk-2.0/gdk-pixbuf.loaders");
  setenv("GDK_PIXBUF_MODULE_FILE", path, 1);

  g_free(dir_path);
}
