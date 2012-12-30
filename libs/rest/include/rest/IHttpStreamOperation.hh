// Copyright (C) 2010 - 2012 by Rob Caelers <robc@krandor.nl>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef WORKRAVE_REST_IHTTPSTREAMOPERATION_HH
#define WORKRAVE_REST_IHTTPSTREAMOPERATION_HH

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/signals2.hpp>

#include "rest/IHttpReply.hh"

namespace workrave
{
  namespace rest
  {
    class IHttpStreamOperation
    {
    public:
      typedef boost::shared_ptr<IHttpStreamOperation> Ptr;

      typedef boost::signals2::signal<void(IHttpReply::Ptr)> HeadersSignal;
      typedef boost::signals2::signal<void(const std::string &)> DataSignal;
      typedef boost::signals2::signal<void(HttpErrorCode, const std::string &)> ClosedSignal;

      virtual ~IHttpStreamOperation() {}

      virtual void start() = 0;
      virtual void cancel() = 0;

      virtual HeadersSignal &signal_headers() = 0;
      virtual DataSignal &signal_data() = 0;
      virtual ClosedSignal &signal_closed() = 0;
    };
  }
}

#endif
