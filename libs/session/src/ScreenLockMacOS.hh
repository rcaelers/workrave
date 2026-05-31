#ifndef SCREENLOCKMACOSHH
#define SCREENLOCKMACOSHH

#include "session/IScreenLockMethod.hh"

class ScreenLockMacOS : public IScreenLockMethod
{
public:
  bool is_lock_supported() override
  {
    return true;
  }
  bool lock() override;
};

#endif
