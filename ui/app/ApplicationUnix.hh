// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef APPLICATION_UNIX_HH
#define APPLICATION_UNIX_HH

#include <memory>

#include "Application.hh"

class IToolkit;

class ApplicationUnix : public Application
{
public:
  ApplicationUnix(int argc, char **argv, std::shared_ptr<IToolkitFactory> toolkit_factory);
  ~ApplicationUnix() override = default;

  void init_platform_pre() override;
  void init_platform_post() override;
};

#endif // APPLICATION_UNIX_HH
