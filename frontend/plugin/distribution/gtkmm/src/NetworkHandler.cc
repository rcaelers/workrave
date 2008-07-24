// NetworkHandler.c --- Network Handler
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
// $Id: NetworkHandler.hh 1301 2007-08-30 21:25:47Z rcaelers $
//

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>

#include "ICore.hh"
#include "ResolveConfigurationLinkEvent.hh"
#include "CoreFactory.hh"
#include "Util.hh"

#include "NetworkHandler.hh"
#include "NetworkConflictDialog.hh"

using namespace workrave;

// struct Conflict
// {
//   char *setting;
//   char *name;
//   BreakId break_id;
// }

//   monitor_config("timers/micro_pause/activity_sensitive");
//   monitor_config("timers/micro_pause/auto_reset");
//   monitor_config("timers/micro_pause/limit");
//   monitor_config("timers/micro_pause/reset_pred");
//   monitor_config("timers/micro_pause/snooze");
//   monitor_config("timers/rest_break/activity_sensitive");
//   monitor_config("timers/rest_break/auto_reset");
//   monitor_config("timers/rest_break/limit");
//   monitor_config("timers/rest_break/reset_pred");
//   monitor_config("timers/rest_break/snooze");
//   monitor_config("timers/daily_limit/activity_sensitive");
//   monitor_config("timers/daily_limit/auto_reset");
//   monitor_config("timers/daily_limit/limit");
//   monitor_config("timers/daily_limit/reset_pred");
//   monitor_config("timers/daily_limit/snooze");
//   monitor_config("breaks/micro_pause/max_preludes");
//   monitor_config("breaks/rest_break/max_preludes");
//   monitor_config("breaks/daily_limit/max_preludes");

NetworkHandler::NetworkHandler()
{
  TRACE_ENTER("NetworkHandler::NetworkHandler");
  TRACE_EXIT();
}


//! Destructor.
NetworkHandler::~NetworkHandler()
{
  TRACE_ENTER("NetworkHandler::~NetworkHandler");
  TRACE_EXIT();
}


void
NetworkHandler::init()
{
  //ICore *core = CoreFactory::get_core();
  INetwork *network = CoreFactory::get_networking();

  network->subscribe("resolveconfigurationlinkevent", this);

//   NetworkConflictDialog *dialog = new NetworkConflictDialog();
//   dialog->run();
}


void 	 
NetworkHandler::event_received(LinkEvent *event) 	 
{ 	 
  TRACE_ENTER_MSG("GUI::event_received", event->str()); 	 
	  	 
  //INetwork *network = CoreFactory::get_networking(); 	 
	  	 
  ResolveConfigurationLinkEvent *resolve = dynamic_cast<ResolveConfigurationLinkEvent *>(event); 	 
  if (resolve == NULL) 	 
    { 	 
      return; 	 
    } 	 
	  	 
  map<string, string> changes = resolve->get_changes(); 	 
	  	 
  if (changes.size() == 0) 	 
    { 	 
      TRACE_MSG("All changes resolved"); 	 
    } 	 


//   NetworkConflictDialog *dialog = new NetworkConflictDialog();
//   dialog->run();
  
//   char *wr = getenv("WORKRAVE_DBUS_NAME"); 	 
//   if (strcmp(wr, "org.workrave.Workrave2") == 0) 	 
//     { 	 
//       map<string, string>::iterator i; 	 
	  	 
//       for (i = changes.begin(); i != changes.end(); i++) 	 
//         { 	 
//           TRACE_MSG("taking " << i->first); 	 
//           if (i->first == "internal/dontchange") 	 
//             { 	 
//               network->resolve_config(i->first, "string:workrave X"); 	 
//             } 	 
//           else if (i->first == "internal/self-destruct") 	 
//             { 	 
//               network->resolve_config(i->first, "int:666"); 	 
//             } 	 
//         } 	 
//     } 	 
	  	 
  TRACE_EXIT(); 	 
}
