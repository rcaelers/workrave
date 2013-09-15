// ICoreTestHooks.hh
//
// Copyright (C) 2012, 2013 Rob Caelers
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

#ifndef ICORETESTHOOKS_HH
#define ICORETESTHOOKS_HH

#ifdef HAVE_TESTS

#include <string>
#include <map>

#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "config/IConfigurator.hh"

class ICoreTestHooks
{
public:
  typedef boost::shared_ptr<ICoreTestHooks> Ptr;

  virtual boost::function<workrave::config::IConfigurator::Ptr()> &hook_create_configurator() = 0;
  virtual boost::function<bool(bool)> &hook_is_user_active() = 0;
};

#endif

#endif // ICOREHOOKS_HH
