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

#ifndef WORKRAVE_RPC_EVENTQUEUE_HH
#define WORKRAVE_RPC_EVENTQUEUE_HH

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

#include <grpcpp/grpcpp.h>

// Bridges a synchronous callback source (a boost::signals2::signal, firing
// on whatever thread calls e.g. Core::set_operation_mode()) into a gRPC
// server-streaming RPC handler, which must call ServerWriter::Write() only
// from its own handler thread. The signal's slot pushes events into this
// queue; the handler thread pops them and writes them out — never called
// concurrently from two threads.
//
// wait_and_pop() polls with a short timeout (rather than waiting forever)
// so it notices client disconnects/cancellation even when no new event
// ever arrives.
namespace rpc
{
  template<typename T>
  class EventQueue
  {
  public:
    void push(T value)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push_back(std::move(value));
      cv_.notify_one();
    }

    // Blocks until an event is available or the context is cancelled.
    // Returns false when the stream should end (client gone).
    bool wait_and_pop(T &out, grpc::ServerContext *context)
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (queue_.empty())
        {
          if (context->IsCancelled())
            {
              return false;
            }
          cv_.wait_for(lock, std::chrono::milliseconds(200));
        }
      if (context->IsCancelled())
        {
          return false;
        }
      out = std::move(queue_.front());
      queue_.pop_front();
      return true;
    }

  private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<T> queue_;
  };
} // namespace rpc

#endif // WORKRAVE_RPC_EVENTQUEUE_HH
