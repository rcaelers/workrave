/*
Copyright (C) 2012 Jay Satiro <raysatiro@yahoo.com>
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

/**
This is a simple win32 example that will send a menu command to Workrave from the command line.

Visual Studio:
cl /W4 send_workrave_command.c

MinGW or cygwin:
gcc -Wall -Wextra -Wno-unknown-pragmas -o send_workrave_command.exe send_workrave_command.c


x:\Workrave\other>send_workrave_command.exe MENU_COMMAND_MODE_NORMAL
-
Copyright (C) 2012 Jay Satiro <raysatiro@yahoo.com>
All rights reserved. License GPLv3+: GNU GPL version 3 or later
<http://www.gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
-

Sending Workrave command MENU_COMMAND_MODE_NORMAL...
Command sent.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment( lib, "user32.lib" )


const char *command[] = 
{
	"MENU_COMMAND_PREFERENCES",
	"MENU_COMMAND_EXERCISES",
	"MENU_COMMAND_REST_BREAK",
	"MENU_COMMAND_MODE_NORMAL",
	"MENU_COMMAND_MODE_QUIET",
	"MENU_COMMAND_MODE_SUSPENDED",
	"MENU_COMMAND_NETWORK_CONNECT",
	"MENU_COMMAND_NETWORK_DISCONNECT",
	"MENU_COMMAND_NETWORK_LOG",
	"MENU_COMMAND_NETWORK_RECONNECT",
	"MENU_COMMAND_STATISTICS",
	"MENU_COMMAND_ABOUT",
	"MENU_COMMAND_MODE_READING", /* toggle reading mode */
	"MENU_COMMAND_OPEN",
	NULL
};


/* print_license()
Print the GPL license and copyright.

If you've made substantial modifications to this program add an additional copyright with your name.
*/
void print_license( void )
{
	printf( "-\n" );
	/* Example copyright notice. Leave this example intact. Copy it below to use as a template.
	printf( "Copyright (C) 2012 Your Name <your@email> \n" );
	*/
	printf( 
		"Copyright (C) 2012 Jay Satiro <raysatiro@yahoo.com> \n"
		"All rights reserved. License GPLv3+: GNU GPL version 3 or later \n"
		"<http://www.gnu.org/licenses/gpl.html>. \n"
		"This is free software: you are free to change and redistribute it. \n"
		"There is NO WARRANTY, to the extent permitted by law. \n"
	);
	printf( "-\n" );
	
	return;
}


void print_commands( void )
{
	int i;
	
	for( i = 0; command[ i ]; ++i )
		printf( "%s\n", command[ i ] );
	
	return;
}


void show_usage_and_exit( void )
{
	printf( "Specify one of the following commands:\n" );
	print_commands();
	exit( 1 );
}


int find_command( const char *str )
{
	int i;
	
	if( !str )
		return -1;
	
	for( i = 0; command[ i ]; ++i )
	{
		if( !_stricmp( str, command[ i ] ) )
			return i;
	}
	
	return -1;
}


int main( int argc, char **argv )
{
	int cmd = -1;
	HWND hwnd = NULL;
	
	
	print_license();
	printf( "\n" );
	
	if( argc < 2 )
		show_usage_and_exit();
	
	cmd = find_command( argv[ 1 ] );
	if( cmd == -1 )
		show_usage_and_exit();
	
	hwnd = FindWindowEx( NULL, NULL, "gdkWindowToplevel", "Workrave" );
	if( !hwnd )
	{
		printf( "Error: Workrave window not found!\n" );
		return 1;
	}
	
	printf( "Sending Workrave command %s...\n", command[ cmd ] );
	fflush( stdout );
	
	SetLastError( 0 );
	if( SendMessageTimeout( hwnd, WM_USER, cmd, 0, SMTO_BLOCK, 15000, NULL ) )
	{
		printf( "Command sent.\n" );
		return 0;
	}
	
	if( GetLastError() == ERROR_TIMEOUT )
		printf( "Error: SendMessageTimeout() timed out!\n" );
	else
		printf( "Error: SendMessageTimeout() failed!\n" );
	
	return 1;
}
