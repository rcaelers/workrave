#ifndef SCREENLOCKCUSTOMCOMMAND_HH
#define SCREENLOCKCUSTOMCOMMAND_HH

#include "session/IScreenLockMethod.hh"
#include <string>

class ScreenLockCustomCommand : public IScreenLockMethod
{
public:
  explicit ScreenLockCustomCommand(std::string cmd);

  bool is_lock_supported() override
  {
    return !cmd_.empty();
  }
  bool lock() override;

  void set_command(const std::string &cmd)
  {
    cmd_ = cmd;
  }

private:
  std::string cmd_;
};

#endif
