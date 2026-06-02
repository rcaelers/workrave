// Copyright (C) 2020-2021 Rob Caelers <robc@krandor.nl>
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

#ifndef CRASH_DIALOG_HH
#define CRASH_DIALOG_HH

#include <string>

#include "handler/user_hook.h"

class UserInteraction : public crashpad::UserHook
{
public:
  UserInteraction() = default;
  ~UserInteraction() override = default;

  bool requestUserConsent(const std::map<std::string, std::string> &annotations,
                          std::vector<base::FilePath> &attachments,
                          const crashpad::CrashSummary &summary) override;
  std::string getUserText() override;
  void reportCompleted(const crashpad::UUID &uuid) override;

private:
  std::string user_text;
  bool consent{false};
};

#endif // CRASH_DIALOG_HH
