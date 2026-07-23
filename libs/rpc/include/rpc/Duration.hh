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

#ifndef WORKRAVE_RPC_DURATION_HH
#define WORKRAVE_RPC_DURATION_HH

#include <chrono>
#include <string_view>

// A `std::chrono::duration<>`-typed parameter is detected structurally by
// clang-rpc-gen (any Rep/Period instantiation, no annotation needed — see
// clang_index.rs::is_chrono_duration()) and put on the wire as a proto
// `string` rather than a raw integer, so a caller doesn't need to know which
// unit the real C++ parameter happens to use. The generated adapter code
// parses it with parse_duration() below, then `duration_cast`s to whatever
// the real parameter's type is.
namespace rpc
{
  // Parses text made of "<number><unit>" tokens (h/m/s, case-insensitive),
  // optionally separated by whitespace, e.g. "1h30m", "1h 30m", "90m", "45s".
  // A bare number with no unit is treated as minutes. Throws RpcException on
  // malformed input.
  std::chrono::seconds parse_duration(std::string_view text);
} // namespace rpc

#endif // WORKRAVE_RPC_DURATION_HH
