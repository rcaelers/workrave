/*
 * ChangeAutorun.c - Change autorun for a new and/or existing user
 *
 * Copyright (C) 2012 Ray Satiro on behalf of the Workrave project
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
 * along with this program.  If not, see <http:*www.gnu.org/licenses/>.
 */

/*****
 * README, CHANGES REQUIRED
 *
 * This program, ChangeAutorun, is called by Active Setup to change the autorun settings for a new
 * and/or existing user of your program.
 *
 * Your installer should have added to its Active Setup key a StubPath value with data that will run
 * this program. For more information search the internet for how to configure Active Setup.
 * Example:
 * "StubPath"="\"C:\\Program Files\\Workrave\\lib\\ChangeAutorun.exe\" -b"
 *
 * Typically you'd set your Active Setup StubPath to call ChangeAutorun with 'a' or 'b' or 'u'.
 * Run ChangeAutorun /? for all options.
 *
 * Options 'v' and 'z' are for testing purposes and must NOT be options in your ActiveSetup StubPath.
 * Either of those options will routinely show MessageBoxes with information only a developer could
 * care about and require user input to click 'OK' to the messages, stalling explorer initialization.
 *
 * REQUIRED CHANGES
 * If this is not the Workrave project you MUST change everything below (until the /END block), and
 * not use the GUID 180B0AC5-6FDA-438B-9466-C9894322B6BA. Generate your own GUID.
 */

/* This is the Active Setup GUID subkey generated by your installer to store its Active Setup. */
#define ACTIVE_SETUP_GUID "{180B0AC5-6FDA-438B-9466-C9894322B6BA}"

/* This is the autorun value stored in the key HKCU_AUTORUN_KEY. The value must be unique.
This program will add a value of AUTORUN_VALUE to the key with data pointing to the target, eg:
"Workrave"="\"C:\\Program Files\\Workrave\\lib\\workrave.exe\""
*/
#define AUTORUN_VALUE "Workrave"

/* This is the name of your project. It is the name that appears in this program's verbose output.
 */
#define PROJECT "Workrave"

/* This is an HKCU key that has been set by your program after it is run by the user. It should not
be a key that is set by your installer. If ChangeAutorun is run with the -b option it needs to
determine whether or not the target program has previously been run by the user. It checks for the
existence of this key to determine that.
*/
#define HKCU_PROJECT_KEY "AppEvents\\Schemes\\Apps\\Workrave"

/* This is your target program's executable name, with extension, and without path. It must be in
the same directory as this program.
*/
#define TARGET "workrave.exe"

/* These are the options to pass to the target program.
For example:
"--somearg \"abc\" -asdf"
If no options use ""
*/
#define TARGET_OPTIONS ""

/*****
 * /END REQUIRED CHANGES
 * You shouldn't need to change anything below here.
 */

/*****
 * COMPILING
 *
 * I wrote this to create an autorun entry for an x86 target program (TARGET). When compiled x86,
 * and run on x86 or x64 platform it will access the 32-bit ACTIVE_SETUP_GUID, HKCU_PROJECT_KEY, etc.
 *
 * You could probably use this program with an x64 TARGET if you compile it x64, and your installer
 * writes to the 64-bit ACTIVE_SETUP_GUID. In that case this program should access the 64-bit keys.
 * I haven't tested that. If you do test it let me know what happens.
 *
 * cl /O2 /W4 /DUNICODE ChangeAutorun.c
 * gcc -O2 -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter -DUNICODE -mwindows -o ChangeAutorun.exe ChangeAutorun.c -lpsapi
 */

/* Disable cl unused parameter warnings */
#pragma warning(disable : 4100)

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

/* Define both UNICODEs before any includes.
http://blogs.msdn.com/b/oldnewthing/archive/2004/02/12/71851.aspx#73016
http://en.wikibooks.org/wiki/Windows_Programming/Unicode#Unicode_Environment
*/
#if defined UNICODE && !defined _UNICODE
#  define _UNICODE
#endif

#if defined _UNICODE && !defined UNICODE
#  define UNICODE
#endif

#if defined _UNICODE && defined _MBCS
#  error Both _UNICODE and _MBCS are defined. Those defines are mutually exclusive.
#endif

/* Use the original GetModuleFilenameEx() location.
http://msdn.microsoft.com/en-us/library/windows/desktop/ms683198.aspx
*/
#define PSAPI_VERSION 1

#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0501

/* INCLUDES */
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
/* /INCLUDES */

/* MinGW has KEY_WOW64 defines only when >= 0x502 */
#ifndef KEY_WOW64_64KEY
#  define KEY_WOW64_64KEY 0x0100
#endif

#ifndef KEY_WOW64_32KEY
#  define KEY_WOW64_32KEY 0x0200
#endif

#define VERBOSE_ERR(msg) VERBOSE_ERR_DATA2(msg, NULL, NULL)
#define VERBOSE_ERR_DATA(msg, tstr1) VERBOSE_ERR_DATA2(msg, tstr1, NULL)
#define VERBOSE_ERR_DATA2(msg, tstr1, tstr2) \
  if (g_verbose)                             \
  show_message(__LINE__, __FUNCTION__, msg, _T("Error"), tstr1, tstr2)

#define VERBOSE_MSG(msg) VERBOSE_MSG_DATA2(msg, NULL, NULL)
#define VERBOSE_MSG_DATA(msg, tstr1) VERBOSE_MSG_DATA2(msg, tstr1, NULL)
#define VERBOSE_MSG_DATA2(msg, tstr1, tstr2) \
  if (g_verbose)                             \
  show_message(__LINE__, __FUNCTION__, msg, _T("Message"), tstr1, tstr2)

#define HKCU_AUTORUN_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

/* if _UNICODE then the lengths of these global variables will be a count of wide characters in the
string. if not _UNICODE then the lengths will be a count of single byte characters. lengths here
will not be a count of multibyte characters even if _MBCS.
*/

/* The target's full image name. Initialized at runtime. Example:
C:\Program Files\Workrave\lib\workrave.exe
*/
TCHAR *target_fullname;
int length_target_fullname;

/* The target's directory. Initialized at runtime. Example:
C:\Program Files\Workrave\lib\
*/
TCHAR *target_directory;
int length_target_directory;

/* The command line for the autorun entry. Initialized at runtime. Example:
"C:\Program Files\Workrave\lib\workrave.exe" -whatever "some arg with spaces" -abc
*/
TCHAR *target_command;
int length_target_command;

/* Verbosity */
BOOL g_verbose = FALSE;

enum users
{
  IGNORE_THIS_TAG = -1, /* force gcc signed enum */
  ALL_USERS       = 0,
  NEW_USERS,
  UPDATE_EXISTING_AUTORUN_ONLY,
  INVALID
};

/* Shows a MessageBox with information. Used by the VERBOSE_ macro functions. */
void
show_message(const int line, const char *function, const TCHAR *message, const TCHAR *title, const TCHAR *data, const TCHAR *data2)
{
  TCHAR *buffer         = NULL;
  int length_buffer     = 0;
  int max_length_buffer = 1024;

  if (data)
    max_length_buffer += _tcslen(data);

  if (data2)
    max_length_buffer += _tcslen(data2);

  buffer = malloc((max_length_buffer + 1) * sizeof(TCHAR));
  if (!buffer)
    return;

  length_buffer =
		_sntprintf(
			buffer,
			max_length_buffer,
			_T("%s - ChangeAutorun\n")
				_T("Line: %d\n")
				_T("Function: %hs()\n")
				_T("Project: ") _T(PROJECT) _T("\n")
				_T("\n%s")
				_T("%s%s\n%s"),
			( ( !title || !*title ) ? _T("<no title>") : title ),
			line,
			( ( !function || !*function ) ? "<no function>" : function ),
			( ( !message || !*message ) ? _T("<no message>") : message ),
			( ( data || data2 ) ? _T("\n\nRelated data:\n") : _T("") ),
			( data ? data : _T("") ),
			( data2 ? data2 : _T("") )
			);

  if (length_buffer < 0)
    length_buffer = max_length_buffer;

  buffer[length_buffer] = _T('\0');

  MessageBox(0, buffer, ((!title || !*title) ? _T("<no title>") : title), 0);

  free(buffer);
  return;
}

/* Check if a file exists. Returns TRUE on success. */
BOOL
file_exists(const TCHAR *filename)
{
  HANDLE hFile = NULL;

  hFile = CreateFile(filename, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

  if (hFile == INVALID_HANDLE_VALUE)
    return FALSE;

  CloseHandle(hFile);
  return TRUE;
}

/* Check if a registry key exists and can be opened for read. Returns TRUE on success. */
BOOL
regkey_test_read(HKEY mainkey, const TCHAR *subkey)
{
  HKEY hkey = NULL;
  LONG ret  = 0;

  ret = RegOpenKeyEx(mainkey, subkey, 0, STANDARD_RIGHTS_READ, &hkey);

  if (ret || !hkey)
    return FALSE;

  if (hkey)
    RegCloseKey(hkey);

  return TRUE;
}

/* Frees and zeroes all global variables except g_verbose and any const. */
void
free_global_variables()
{
  if (target_directory)
    {
      free(target_directory);
      target_directory = NULL;
    }
  length_target_directory = 0;

  if (target_fullname)
    {
      free(target_fullname);
      target_fullname = NULL;
    }
  length_target_fullname = 0;

  if (target_command)
    {
      free(target_command);
      target_command = NULL;
    }
  length_target_command = 0;

  return;
}

/* init_global_variables()

Initialize all global variables except g_verbose and any const.

Returns TRUE if all variables were successfully initialized. If only some variables were
initialized, those that couldn't be initialized will be cleared and this function will return FALSE.
*/
BOOL
init_global_variables()
{
  int i                         = 0;
  TCHAR *temp                   = NULL;
  int length_target             = 0;
  int max_length_target_command = 0;

  free_global_variables();

  /* target_directory
  Get this program's path from GetModuleFileNameEx().
  When using that function there's no sure way to know the exact size beforehand.
  */
  for (i = 512; i <= 65536; i *= 2, length_target_directory = 0)
    {
      temp = realloc(target_directory, (i * sizeof(TCHAR)));

      if (!temp)
        {
          VERBOSE_ERR(_T("realloc() failed. Could not initialize target_directory."));
          goto cleanup;
        }

      target_directory = temp;

      length_target_directory = GetModuleFileNameEx(GetCurrentProcess(), NULL, target_directory, i);

      if (length_target_directory && ((length_target_directory + 1) < i))
        break;
    }

  if (!length_target_directory)
    {
      VERBOSE_ERR(_T("GetModuleFileNameEx() failed. Could not initialize target_directory."));
      goto cleanup;
    }

  target_directory[length_target_directory] = _T('\0');

  if (!file_exists(target_directory))
    {
      VERBOSE_ERR_DATA(_T("File not found. Could not initialize target_directory."), target_directory);

      length_target_directory = 0;
      goto cleanup;
    }

  temp = _tcsrchr(target_directory, _T('\\'));

  if (!temp)
    {
      VERBOSE_ERR_DATA(_T("Backslash not found in string. Could not initialize target_directory."), target_directory);

      length_target_directory = 0;
      goto cleanup;
    }

  *++temp = _T('\0');

  length_target_directory = temp - target_directory;

  /* target_fullname
  Append the executable name (TARGET) to target_directory.
  */
  length_target          = _tcslen(_T(TARGET));
  length_target_fullname = length_target_directory + length_target;

  target_fullname = malloc((length_target_fullname + 1) * sizeof(TCHAR));
  if (!target_fullname)
    {
      VERBOSE_ERR(_T("malloc() failed. Could not initialize target_fullname."));

      length_target_fullname = 0;
      goto cleanup;
    }

  temp = target_fullname;

  memcpy(temp, target_directory, (length_target_directory * sizeof(TCHAR)));
  temp += length_target_directory;

  memcpy(temp, _T(TARGET), (length_target * sizeof(TCHAR)));
  temp += length_target;

  *temp = _T('\0');

  if (!file_exists(target_fullname))
    {
      VERBOSE_ERR_DATA(_T("File not found. Could not initialize target_fullname."), target_fullname);

      length_target_fullname = 0;
      goto cleanup;
    }

  /* target_command
  Make the target command to execute from the target_fullname and TARGET_OPTIONS
  */
  max_length_target_command = 1024 + length_target_fullname;

  target_command = malloc((max_length_target_command + 1) * sizeof(TCHAR));
  if (!target_command)
    {
      VERBOSE_ERR(_T("malloc() failed. Could not initialize target_command."));

      length_target_command = 0;
      goto cleanup;
    }

  /* what follows target_fullname cannot be all trailing spaces.
  "\"C:\whatever.exe\" "
  the legacy run key will not handle that properly and would not start whatever.exe
  it will not start a program with trailing spaces. if there are options then trailing spaces can
  follow that, so this is ok:
  "\"C:\whatever.exe\" --arf "
  */
  for (temp = _T(TARGET_OPTIONS); *temp == _T(' '); ++temp)
    ;

  length_target_command =
    _sntprintf(target_command, max_length_target_command, _T("\"%s\"%s%s"), target_fullname, (*temp ? _T(" ") : _T("")), temp);

  if (length_target_command < 0)
    {
      VERBOSE_ERR_DATA2(_T("_sntprintf() failed. Could not initialize target_command."), target_fullname, _T(TARGET_OPTIONS));

      length_target_command = 0;
      goto cleanup;
    }

  target_command[length_target_command] = _T('\0');

cleanup:
  if (!length_target_directory)
    {
      free(target_directory);
      target_directory = NULL;
    }

  if (!target_directory)
    length_target_directory = 0;

  if (!length_target_fullname)
    {
      free(target_fullname);
      target_fullname = NULL;
    }

  if (!target_fullname)
    length_target_fullname = 0;

  if (!length_target_command)
    {
      free(target_command);
      target_command = NULL;
    }

  if (!target_command)
    length_target_command = 0;

  return (target_directory && target_fullname && target_command);
}

/* add_autorun_entry()

Adds an autorun entry for your program. Returns TRUE on success.

if 'users' == NEW_USERS then add only if TARGET has never before been started by the user.
if 'users' == ALL_USERS then add always.
if 'users' == UPDATE_EXISTING_AUTORUN_ONLY then only update an existing entry.

regardless of the value of 'users', if the user already has an autorun entry it is updated with the
new location.

To determine if your TARGET program has ever before been run by the user, this function checks for
the existence of HKCU registry key HKCU_PROJECT_KEY.
*/
BOOL
add_autorun_entry(enum users users)
{
  LONG ret  = 0;
  HKEY hkey = NULL;

  if (!target_command)
    {
      VERBOSE_ERR(_T("Parameter validation failed. target_command == NULL"));
      return FALSE;
    }

  if ((users < 0) || (users >= INVALID))
    {
      VERBOSE_ERR(_T("Parameter validation failed. users enumeration tag unrecognized."));
      return FALSE;
    }

  ret = RegCreateKeyEx(HKEY_CURRENT_USER, _T(HKCU_AUTORUN_KEY), 0, NULL, 0, (KEY_READ | KEY_WRITE), NULL, &hkey, NULL);

  if (ret)
    {
      VERBOSE_ERR_DATA(_T("RegCreateKeyEx() failed. The HKCU autorun key couldn't be opened for writing."), _T(HKCU_AUTORUN_KEY));

      return FALSE;
    }

  /* if the autorun value doesn't exist */
  if (RegQueryValueEx(hkey, _T(AUTORUN_VALUE), NULL, NULL, NULL, NULL))
    {
      /* if setting an autorun entry for new users, and the user isn't new (project key exists) */
      if ((users == NEW_USERS) && regkey_test_read(HKEY_CURRENT_USER, _T(HKCU_PROJECT_KEY)))
        {
          VERBOSE_MSG_DATA(_T("Existing project key and no existing autorun entry. No entry has been added."),
                           _T(HKCU_PROJECT_KEY));

          return FALSE;
        }
      else if (users == UPDATE_EXISTING_AUTORUN_ONLY)
        {
          VERBOSE_ERR(_T("No existing autorun entry to update."));

          return FALSE;
        }
    }

  ret =
    RegSetValueEx(hkey, _T(AUTORUN_VALUE), 0, REG_SZ, (const BYTE *)target_command, ((length_target_command + 1) * sizeof(TCHAR)));

  RegCloseKey(hkey);

  if (ret)
    {
      VERBOSE_ERR_DATA2(_T("RegSetValueEx() failed. The autorun value couldn't be written."), _T(AUTORUN_VALUE), target_command);

      return FALSE;
    }

  VERBOSE_MSG_DATA(_T("The autorun data was successfully added."), target_command);
  return TRUE;
}

/* Returns TRUE if your project's HKCU autorun entry was deleted. */
BOOL
remove_autorun_entry()
{
  LONG ret  = 0;
  HKEY hkey = NULL;

  ret = RegOpenKeyEx(HKEY_CURRENT_USER, _T(HKCU_AUTORUN_KEY), 0, (KEY_READ | KEY_WRITE), &hkey);

  if (ret || !hkey)
    {
      VERBOSE_ERR_DATA(_T("RegOpenKeyEx() failed. The HKCU autorun key doesn't exist or couldn't be read."), _T(HKCU_AUTORUN_KEY));

      return FALSE;
    }

  ret = RegQueryValueEx(hkey, _T(AUTORUN_VALUE), NULL, NULL, NULL, NULL);

  if (ret)
    {
      if ((ret == ERROR_FILE_NOT_FOUND) || (ret == ERROR_PATH_NOT_FOUND))
        {
          VERBOSE_ERR_DATA(_T("The autorun value does not exist. There is nothing to delete."), _T(AUTORUN_VALUE));
        }
      else
        {
          VERBOSE_ERR_DATA(_T("RegQueryValueEx() failed. The value could not be read."), _T(AUTORUN_VALUE));
        }

      return FALSE;
    }

  ret = RegDeleteValue(hkey, _T(AUTORUN_VALUE));

  RegCloseKey(hkey);

  if (ret)
    {
      VERBOSE_ERR_DATA(_T("RegDeleteValue() failed. The autorun value doesn't exist or could not be read."), _T(AUTORUN_VALUE));

      return FALSE;
    }

  VERBOSE_MSG(_T("Your project's autorun entry was successfully deleted."));
  return TRUE;
}

/* Returns TRUE if your project's HKCU Active Setup key was deleted. */
BOOL
remove_activesetup_entry()
{
  BOOL deleted                                                 = FALSE;
  LONG(WINAPI * pRegDeleteKeyEx)(HKEY, LPCTSTR, REGSAM, DWORD) = NULL;

  TCHAR *subkey_default = _T("SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\") _T(ACTIVE_SETUP_GUID);

  TCHAR *subkey_wow = _T("SOFTWARE\\Wow6432Node\\Microsoft\\Active Setup\\Installed Components\\") _T(ACTIVE_SETUP_GUID);

  if (!RegDeleteKey(HKEY_CURRENT_USER, subkey_default))
    deleted = TRUE;

  /* This one probably won't work. */
  if (!RegDeleteKey(HKEY_CURRENT_USER, subkey_wow))
    deleted = TRUE;

  pRegDeleteKeyEx = (LONG(WINAPI *)(HKEY, LPCTSTR, REGSAM, DWORD))GetProcAddress(
    GetModuleHandleA("Advapi32"), ((sizeof(TCHAR) == 1) ? "RegDeleteKeyExA" : "RegDeleteKeyExW"));

  if (pRegDeleteKeyEx)
    {
      if (!pRegDeleteKeyEx(HKEY_CURRENT_USER, subkey_default, KEY_WOW64_32KEY, 0))
        deleted = TRUE;

      if (!pRegDeleteKeyEx(HKEY_CURRENT_USER, subkey_default, KEY_WOW64_64KEY, 0))
        deleted = TRUE;
    }

  if (!deleted)
    {
      VERBOSE_ERR_DATA(_T("Your project's Active Setup key either does not exist or could not be deleted."), subkey_default);

      return FALSE;
    }

  VERBOSE_MSG(_T("Your project's Active Setup key was successfully deleted."));
  return TRUE;
}

/* Returns FALSE if tstring is NULL or is empty or contains only whitespace. */
BOOL
param_check_tstring(const TCHAR *tstring)
{
  if (!tstring)
    return FALSE;

  while ((*tstring == _T(' ')) || (*tstring == _T('\t')))
    ++tstring;

  return !!*tstring;
}

/* Returns FALSE if some string literal is empty that shouldn't be. */
BOOL
check_global_literals()
{
  BOOL pass = TRUE;

#define CHECK_LITERAL(literal)                                                                                      \
  if (!param_check_tstring(_T(literal)))                                                                            \
    {                                                                                                               \
      pass = FALSE;                                                                                                 \
      VERBOSE_ERR_DATA(_T("Program to fail. tstring literal is empty or contains only whitespace."), _T(#literal)); \
    }

  CHECK_LITERAL(HKCU_AUTORUN_KEY);
  CHECK_LITERAL(ACTIVE_SETUP_GUID);
  CHECK_LITERAL(AUTORUN_VALUE);
  CHECK_LITERAL(PROJECT);
  CHECK_LITERAL(HKCU_PROJECT_KEY);
  CHECK_LITERAL(TARGET);
  /* TARGET_OPTIONS can be empty */

  return pass;
}

void
show_help()
{
  MessageBoxA(0,
              "This is a configuration tool added by this project's installer.\n"
              "This program is invoked by Microsoft's undocumented Active Setup.\n"
              "If you do not understand what this program does don't use it.\n"
              "\n"
              "ACTIVE_SETUP_GUID: " ACTIVE_SETUP_GUID
              "\n"
              "AUTORUN_VALUE: " AUTORUN_VALUE
              "\n"
              "PROJECT: " PROJECT
              "\n"
              "HKCU_PROJECT_KEY: " HKCU_PROJECT_KEY
              "\n"
              "TARGET: " TARGET
              "\n"
              "TARGET_OPTIONS: " TARGET_OPTIONS
              "\n"
              "\n"
              "Options:\n"
              "-a"
              "\t"
              "Add autorun always.\n"
              "-b"
              "\t"
              "Add autorun only if user has never before used your project.\n"
              "-d"
              "\t"
              "Remove the autorun entry.\n"
              "-r"
              "\t"
              "Remove the Active Setup entry.\n"
              "-u"
              "\t"
              "Update existing autorun entry.\n"
              "-v"
              "\t"
              "Be verbose. If multiple arguments this must be the first.\n"
              "-z"
              "\t"
              "Show each passed in argument for testing purposes.\n"
              "\n"
              "The 'Add autorun' options are mutually exclusive. Using either \n"
              "implies option 'u', which updates an existing autorun entry, if any.\n"
              "\n"
              "Option 'r' removes the HKCU Active Setup entry, if any, and not the\n"
              "HKLM Active Setup entry. What that means is Active Setup will be\n"
              "reset for the user, and will run the next time the user logs on.\n",
              "Help - ChangeAutorun",
              0);

  return;
}

void
just_testing(int argc, char **argv)
{
  int i = 0;

  for (i = 0; i < argc; ++i)
    MessageBoxA(0, argv[i], argv[i], 0);

  if (argv[i])
    MessageBoxA(0, "Final argv pointer is not NULL", "WARNING", 0);

  return;
}

int _CRT_glob = 0;

int APIENTRY
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  int i = 0, j = 0;
  int argc    = 0;
  char **argv = NULL;

  SetErrorMode((SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX));

  argv = __argv;
  argc = __argc;
  if (argc <= 1)
    return 1;

  if (argc >= 2 && ((argv[1][0] == '/') || (argv[1][0] == '-')))
    {
      if ((argv[1][1] == 'v') || (argv[1][1] == 'V'))
        {
          g_verbose = TRUE;
        }
      else if ((argv[1][1] == '?') || (argv[1][1] == 'h') || (argv[1][1] == 'H'))
        {
          show_help();
          return 0;
        }
    }

  if (!check_global_literals())
    return 1;

  init_global_variables();

  for (i = 1; i < argc; ++i)
    {
      if (argv[i][0] != '-' && argv[i][0] != '/')
        continue;

      for (j = 1; argv[i][j]; ++j)
        {
          switch (argv[i][j])
            {
            case 'a':
            case 'A':
              add_autorun_entry(ALL_USERS);
              break;

            case 'b':
            case 'B':
              add_autorun_entry(NEW_USERS);
              break;

            case 'd':
            case 'D':
              remove_autorun_entry();
              break;

            case 'r':
            case 'R':
              remove_activesetup_entry();
              break;

            case 'u':
            case 'U':
              add_autorun_entry(UPDATE_EXISTING_AUTORUN_ONLY);
              break;

            case 'z':
            case 'Z':
              just_testing(argc, argv);
              break;
            }
        }
    }

  free_global_variables();

  return 0;
}
