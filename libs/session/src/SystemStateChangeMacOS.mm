#include "SystemStateChangeMacOS.hh"

#import <Foundation/Foundation.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

bool
SystemStateChangeMacOS::suspend()
{
  io_connect_t port = IOPMFindPowerManagement(MACH_PORT_NULL);
  if (port == IO_OBJECT_NULL)
    {
      return false;
    }
  IOReturn result = IOPMSleepSystem(port);
  IOServiceClose(port);
  return result == kIOReturnSuccess;
}

bool
SystemStateChangeMacOS::shutdown()
{
  NSAppleScript *script = [[NSAppleScript alloc]
    initWithSource:@"tell application \"System Events\" to shut down"];
  NSDictionary *error = nil;
  [script executeAndReturnError:&error];
  return error == nil;
}
