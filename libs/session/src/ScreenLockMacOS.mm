#include "ScreenLockMacOS.hh"

#import <Foundation/Foundation.h>
#include <dlfcn.h>

bool
ScreenLockMacOS::lock()
{
  // Try SACLockScreenImmediate from SecurityFoundation (private but stable since 10.9)
  static bool resolved = false;
  static void (*sacLock)() = nullptr;

  if (!resolved)
    {
      resolved = true;
      // login.framework is the current home on macOS 13+;
      // SecurityFoundation.framework is the older location (macOS 10.x–12.x)
      static const char *const kPaths[] = {
        "/System/Library/PrivateFrameworks/login.framework/Versions/Current/login",
        "/System/Library/PrivateFrameworks/SecurityFoundation.framework/SecurityFoundation",
      };
      for (const char *path: kPaths)
        {
          void *handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
          if (handle != nullptr)
            {
              // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
              sacLock = reinterpret_cast<void (*)()>(dlsym(handle, "SACLockScreenImmediate"));
              if (sacLock != nullptr)
                {
                  break;
                }
            }
        }
    }

  if (sacLock != nullptr)
    {
      sacLock();
      return true;
    }

  // Fallback: CGSession -suspend
  NSTask *task = [[NSTask alloc] init];
  task.executableURL = [NSURL fileURLWithPath:@"/System/Library/CoreServices/Menu Extras/User.menu/Contents/Resources/CGSession"];
  task.arguments = @[@"-suspend"];
  NSError *error = nil;
  [task launchAndReturnError:&error];
  if (error == nil)
    {
      [task waitUntilExit];
      return task.terminationStatus == 0;
    }

  return false;
}
