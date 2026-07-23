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

#ifndef WORKRAVE_RPC_INSTANCEREGISTRY_HH
#define WORKRAVE_RPC_INSTANCEREGISTRY_HH

#include <map>
#include <mutex>

#include "rpc/RpcException.hh"

// Generic key -> live-object-reference registry. This is the gRPC analog of
// DBus's IDBus::connect(path, interface, object)/find_object(path, ...):
// several C++ instances can implement the same generated <X>ServiceImpl,
// distinguished by a Key (e.g. workrave::BreakId) instead of a DBus object
// path. Instances register themselves on construction and unregister on
// destruction; the generated ServiceImpl never has to know how many
// instances exist or when they come and go, only how to resolve one by key.
namespace rpc
{
  template<typename Key, typename T>
  class InstanceRegistry
  {
  public:
    void register_instance(Key key, T &instance)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      instances_[key] = &instance;
    }

    void unregister_instance(Key key)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      instances_.erase(key);
    }

    T &resolve(Key key) const
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = instances_.find(key);
      if (it == instances_.end())
        {
          throw RpcException("no instance registered for the given key");
        }
      return *it->second;
    }

  private:
    mutable std::mutex mutex_;
    std::map<Key, T *> instances_;
  };
} // namespace rpc

#endif // WORKRAVE_RPC_INSTANCEREGISTRY_HH
