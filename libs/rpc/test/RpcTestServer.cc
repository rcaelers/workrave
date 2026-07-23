// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#include "RpcTestServer.hh"

std::string
RpcTestServer::ping(std::string message)
{
  return message + " pong";
}

int32_t
RpcTestServer::add(int32_t a, int32_t b)
{
  return a + b;
}

void
RpcTestServer::set_flag(bool value)
{
  flag_ = value;
}

bool
RpcTestServer::get_mode(TestMode &mode)
{
  mode = mode_;
  return true;
}

std::string
RpcTestServer::greet(const std::string &name)
{
  return "hello, " + name;
}
