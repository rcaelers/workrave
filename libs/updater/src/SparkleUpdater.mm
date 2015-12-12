// Copyright (C) 2014 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SparkleUpdater.hh"

#include <Cocoa/Cocoa.h>
#include <Sparkle/Sparkle.h>


// using namespace std;
// using namespace workrave;
// using namespace workrave::utils;

class SparkleUpdater::Private
{
public:
    SUUpdater* updater;
};

SparkleUpdater::SparkleUpdater(std::string appcast_url)
{
  d = new Private;

  d->updater = [SUUpdater sharedUpdater];
  [d->updater retain];

  NSURL* url = [NSURL URLWithString: [NSString stringWithUTF8String: appcast_url.c_str()]];

  [d->updater setAutomaticallyChecksForUpdates:YES];
  [d->updater setAutomaticallyDownloadsUpdates:NO];
  [d->updater setSendsSystemProfile:NO];
  [d->updater resetUpdateCycle];
  [d->updater retain];

  [d->updater setFeedURL: url];
}

SparkleUpdater::~SparkleUpdater()
{
  [d->updater release];
  delete d;
}

void 
SparkleUpdater::check_for_updates()
{
  [d->updater checkForUpdatesInBackground];
}
