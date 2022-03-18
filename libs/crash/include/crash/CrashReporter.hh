// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_CRASH_CRASHREPORTER_HH
#define WORKRAVE_CRASH_CRASHREPORTER_HH

#include <memory>

namespace workrave::crash
{
  class CrashHandler
  {
  public:
    virtual void on_crashed() = 0;
  };

  class CrashReporter
  {
  public:
    CrashReporter();
    ~CrashReporter() = default;

    static CrashReporter &instance();

    void register_crash_handler(CrashHandler *handler);
    void unregister_crash_handler(CrashHandler *handler);
    void init();

  private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;
  };
} // namespace workrave::crash

#endif // WORKRAVE_CRASH_CRASHREPORTER_HH
