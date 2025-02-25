// Copyright (C) 2008, 2009 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id: MainWindow.cc 1367 2007-10-23 19:07:55Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#import "AppController.h"

@implementation AppController

+ (void) initialize
{
    //make sure another Workrave.app isn't running already
    NSString * bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
    int processIdentifier = [[NSProcessInfo processInfo] processIdentifier];

    NSDictionary * dic;
    NSEnumerator * enumerator = [[[NSWorkspace sharedWorkspace] launchedApplications] objectEnumerator];
    while ((dic = [enumerator nextObject]))
    {
        if ([[dic objectForKey: @"NSApplicationBundleIdentifier"] isEqualToString: bundleIdentifier]
            && [[dic objectForKey: @"NSApplicationProcessIdentifier"] intValue] != processIdentifier)
        {
            NSAlert * alert = [[NSAlert alloc] init];
            [alert addButtonWithTitle: NSLocalizedString(@"Quit", "Workrave already running alert -> button")];
            [alert setMessageText: NSLocalizedString(@"Workrave is already running.",
                                                    "Workrave already running alert -> title")];
            [alert setInformativeText: NSLocalizedString(@"There is already a copy of Workrave running. "
                "This copy cannot be opened until that instance is quit.", "Workrave already running alert -> message")];
            [alert setAlertStyle: NSWarningAlertStyle];

            [alert runModal];
            [alert release];

            //kill ourselves right away
            exit(0);
        }
    }

//     [[NSUserDefaults standardUserDefaults] registerDefaults: [NSDictionary dictionaryWithContentsOfFile:
//         [[NSBundle mainBundle] pathForResource: @"Defaults" ofType: @"plist"]]];

}

- (id) init
{
    if ((self = [super init]))
    {
        [NSApp setDelegate: self];
    }

    return self;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
  TRACE_ENTRY();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
  TRACE_ENTRY();
}

@end
