#ifndef SYSTEMSTATECHANGEMACOS_HH
#define SYSTEMSTATECHANGEMACOS_HH

#include "session/ISystemStateChangeMethod.hh"

class SystemStateChangeMacOS : public ISystemStateChangeMethod
{
public:
  bool canShutdown() override
  {
    return true;
  }
  bool canSuspend() override
  {
    return true;
  }

  bool shutdown() override;
  bool suspend() override;
};

#endif
