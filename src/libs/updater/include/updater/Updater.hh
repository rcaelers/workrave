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

#ifndef WORKRAVE_UPDATE_UPDATER_HH
#define WORKRAVE_UPDATE_UPDATER_HH

#include<string>
#include <memory>

namespace workrave
{
  namespace updater
  {
    class Updater
    {
    public:
      typedef std::shared_ptr<Updater> Ptr;

      virtual ~Updater() {}
      virtual void check_for_updates() = 0;

    };

    class UpdaterFactory
    {
    public:
      static Updater::Ptr create(std::string appcast_url);
    };
  }
}

#endif // WORKRAVE_UPDATE_UPDATER_HH
