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

#ifndef WORKRAVE_REST_IHTTPSESSION_HH
#define WORKRAVE_REST_IHTTPSESSION_HH

#include <string>

#include "rest/IHttpOperation.hh"
#include "rest/IHttpStreamOperation.hh"
#include "rest/IHttpServer.hh"
#include "rest/IHttpReply.hh"
#include "rest/IHttpRequest.hh"
#include "rest/IHttpRequestFilter.hh"

namespace workrave
{
  namespace rest
  {
    class IHttpSession
    {
    public:
      typedef boost::shared_ptr<IHttpSession> Ptr;
      typedef boost::function<void (IHttpReply::Ptr) > Ready;

      static Ptr create(const std::string &user_agent);
  
      virtual void add_request_filter(IHttpRequestFilter::Ptr filter) = 0;
      virtual void remove_request_filter(IHttpRequestFilter::Ptr filter) = 0;
  
      virtual IHttpOperation::Ptr send(IHttpRequest::Ptr request, Ready ready) = 0;
      
      virtual IHttpOperation::Ptr request(IHttpRequest::Ptr request) = 0;
      virtual IHttpStreamOperation::Ptr stream(IHttpRequest::Ptr request) = 0;
      virtual IHttpServer::Ptr listen(const std::string &path, IHttpServer::RequestCallback callback) = 0;

      virtual ~IHttpSession() {}
    };
  }
}

#endif
