#if defined(WIN32)

#include <gdk/gdkwin32.h>

#include <iostream>
#include <fstream>
using namespace std;

#define APPEND(x, y)	{ \
	ofstream append; \
	append.open( "c:\\temp\\workrave.txt", std::ios::app ); \
	if( append.is_open() ) \
	{ \
		append << x << ": " << y << endl; \
		append.close(); \
	} \
}

#else

#define APPEND(x, y)

#endif
