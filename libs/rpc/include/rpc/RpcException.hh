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

#ifndef WORKRAVE_RPC_RPCEXCEPTION_HH
#define WORKRAVE_RPC_RPCEXCEPTION_HH

#include <stdexcept>
#include <string>

// Deliberately free of any Workrave-specific base class (e.g.
// workrave::utils::Exception) so this library has no dependency on the rest
// of the Workrave tree and can be lifted out on its own.
namespace rpc
{
  class RpcException : public std::runtime_error
  {
  public:
    explicit RpcException(const std::string &detail)
      : std::runtime_error(detail)
    {
    }
  };
} // namespace rpc

#endif // WORKRAVE_RPC_RPCEXCEPTION_HH
