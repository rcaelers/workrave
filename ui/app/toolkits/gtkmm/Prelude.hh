// Copyright (C) 2001 - 2017 Rob Caelers & Raymond Penners
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

#ifndef PRELUDE_HH
#define PRELUDE_HH

#include "core/ICore.hh"
#include "core/IApp.hh"

class Prelude
{
public:
  Prelude() = default;
  ~Prelude() = default;

  std::string get_title(workrave::BreakId break_id) const;
  std::string get_progress_text(workrave::IApp::PreludeProgressText text);
};

#endif // PRELUDE_HH
