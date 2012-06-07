/*
Copyright (C) 2011, 2012 Rob Caelers
All rights reserved.

This file is part of Workrave.

Workrave is free software: you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published by 
the Free Software Foundation, either version 3 of the License, or 
(at your option) any later version.

Workrave is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with Workrave.  If not, see <http://www.gnu.org/licenses/>.
*/

using System;
using System.Text;
using System.Runtime.InteropServices;

namespace WorkraveControl
{
class Program
{
    enum MenuCommand
    {
        MENU_COMMAND_PREFERENCES,
        MENU_COMMAND_EXERCISES,
        MENU_COMMAND_REST_BREAK,
        MENU_COMMAND_MODE_NORMAL,
        MENU_COMMAND_MODE_QUIET,
        MENU_COMMAND_MODE_SUSPENDED,
        MENU_COMMAND_NETWORK_CONNECT,
        MENU_COMMAND_NETWORK_DISCONNECT,
        MENU_COMMAND_NETWORK_LOG,
        MENU_COMMAND_NETWORK_RECONNECT,
        MENU_COMMAND_STATISTICS,
        MENU_COMMAND_ABOUT,
        MENU_COMMAND_MODE_READING,
        MENU_COMMAND_OPEN,
    };

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    static extern IntPtr FindWindowEx(IntPtr parentHandle, IntPtr childAfter, string lclassName, string windowTitle);

    [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
    static extern int GetClassName(IntPtr hWnd, StringBuilder lpClassName, int nMaxCount);

    [DllImport("user32.dll")]
    static extern int GetWindowText(IntPtr window, [In][Out] StringBuilder text, int copyCount);

    [DllImport("user32.dll")]
    static extern int SendMessage(IntPtr window, int message, int wparam, int lparam);

    const int WM_USER = 0x0400;

    static IntPtr RecursiveFindWindow(IntPtr hwnd, String className, String title)
    {
        StringBuilder c = new StringBuilder(100);
        int rc = GetClassName(hwnd, c, c.Capacity);

        StringBuilder t = new StringBuilder(100);
        rc = GetWindowText(hwnd, t, t.Capacity);

        if (c.ToString().Equals(className) && t.ToString().Equals(title))
        {
            return hwnd;
        }
        else
        {
            IntPtr result = IntPtr.Zero;
            IntPtr child = FindWindowEx(hwnd, IntPtr.Zero, null, null);

            while (child != IntPtr.Zero)
            {
                result = RecursiveFindWindow(child, className, title);
                if (result != IntPtr.Zero)
                {
                    return result;
                }

                child = FindWindowEx(hwnd, child, null, null);
            }
            return IntPtr.Zero;
        }
    }

    static void Main(string[] args)
    {
        IntPtr window = RecursiveFindWindow(IntPtr.Zero, "gdkWindowToplevel", "Workrave");
        if (window != IntPtr.Zero)
        {
            SendMessage(window, WM_USER, (int) MenuCommand.MENU_COMMAND_MODE_QUIET, 0);
        }
    }
}
}
