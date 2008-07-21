/*
 * dll_hell.c --- DLL hell checking
 *
 * Copyright (C) 2003, 2007 Raymond Penners <raymond@dotsphinx.com>
 * Adapted from Gaim, originally written by Herman Bloggs
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

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DLL_CONFLICT_EXTENSION ".conflict"

static char msg1[] = "The following duplicate of ";
static char msg2[] =
" has been found in your DLL search path and will likely\x0d\x0a"
"cause Workrave to malfunction:\x0d\x0a\x0d\x0a";

static char msg3[] = "\x0d\x0a\x0d\x0aWould you like to rename this DLL to ";
static char msg4[] =
DLL_CONFLICT_EXTENSION " in order to avoid any possible conflicts?\x0d\x0a"
"\x0d\x0a"
"Important: Doing so will likely cause the application that installed this DLL to stop functioning.\x0d\x0a"
"You may wish to file a bug report for that application so that future versions will not\x0d\x0a"
"cause this so-called \"DLL Hell\".\x0d\x0a"
"\x0d\x0a"
"Tip: If you do not understand what this all means, then please reinstall Workrave, but this time\x0d\x0a"
"select the full installation (including the GTK+ runtime).";


static void
check_dll(char* dll, char* orig)
{
  char tmp[MAX_PATH];
  char *last;

  if (SearchPath(NULL, dll, NULL, MAX_PATH, tmp, &last)) {
    char* patha = (char*)malloc(strlen(orig) + strlen(dll) + 4);
    strcpy(patha, orig);
    strcat(patha, "\\");
    strcat(patha, dll);
    /* Make sure that 2 paths are not the same */
    if(strcasecmp(patha, tmp) != 0) {
      char *warning = (char*)malloc(strlen(msg1)+
                                    strlen(msg2)+
                                    strlen(msg3)+
                                    strlen(msg4)+
                                    strlen(tmp)+
                                    (strlen(dll)*2)+4);
      sprintf(warning, "%s%s%s%s%s%s%s", msg1, dll, msg2, tmp, msg3, dll, msg4);
      if(MessageBox(NULL, warning, "Workrave Warning", MB_YESNO | MB_TOPMOST)==IDYES) {
        char *newname = (char*)malloc(strlen(tmp)+strlen(DLL_CONFLICT_EXTENSION)+1);
                                /* Rename offending dll */
        sprintf(newname, "%s%s", tmp, DLL_CONFLICT_EXTENSION);
        if(rename(tmp, newname) != 0) {
          MessageBox(NULL, "Error renaming file.", NULL, MB_OK | MB_TOPMOST);
        }
        else
          check_dll(dll, orig);
        free(newname);
      }
      free(warning);
    }
    free(patha);
  }
}

static void
dll_hell_check_path (char* gtkpath)
{
  HANDLE myHandle;
  WIN32_FIND_DATA fd;
  char* srchstr = (char*)malloc(strlen(gtkpath) + 8);

  sprintf(srchstr, "%s%s", gtkpath, "\\*.dll");
  myHandle = FindFirstFile(srchstr, &fd );
  if(myHandle != INVALID_HANDLE_VALUE) {
    check_dll(fd.cFileName, gtkpath);
    while(FindNextFile(myHandle, &fd)) {
      check_dll(fd.cFileName, gtkpath);
    }
  }
  free(srchstr);
}

static BOOL
read_reg_string(HKEY key, char* sub_key, char* val_name, LPBYTE data, LPDWORD data_len)
{
  HKEY hkey;
  BOOL ret = FALSE;
  int retv;

  if(ERROR_SUCCESS == RegOpenKeyEx(key,
                                   sub_key,
                                   0,  KEY_QUERY_VALUE, &hkey)) {
    if(ERROR_SUCCESS == (retv=RegQueryValueEx(hkey, val_name, 0, NULL, data, data_len)))
      ret = TRUE;
    else {
      printf("Error reading registry string value: %d\n", retv);
    }
    RegCloseKey(key);
  }
  return ret;
}

void
dll_hell_check ()
{
  char path[MAX_PATH];
  DWORD plen = MAX_PATH;
  int common_gtk = FALSE;
  BOOL gotreg;

  if (read_reg_string(HKEY_LOCAL_MACHINE, "SOFTWARE\\Workrave", "CommonGTK", (LPBYTE)&path, &plen))
    {
      common_gtk = strcasecmp(path, "true")==0;
    }

  if (common_gtk)
    {
      if (!(gotreg = read_reg_string(HKEY_LOCAL_MACHINE, "SOFTWARE\\GTK\\2.0", "Path", (LPBYTE)&path, &plen)))
        gotreg = read_reg_string(HKEY_CURRENT_USER, "SOFTWARE\\GTK\\2.0", "Path", (LPBYTE)&path, &plen);
      if(gotreg) {
        strcat(path, "\\lib");
        dll_hell_check_path (path);
      }
    }
}


