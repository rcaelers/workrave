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

#ifndef WORKRAVE_REST_IHTTPREPLY_HH
#define WORKRAVE_REST_IHTTPREPLY_HH

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

#include "IHttpRequest.hh"
#include "HttpError.hh"

namespace workrave
{
  namespace rest
  {
    class IHttpReply
    {
    public:
      typedef boost::shared_ptr<IHttpReply> Ptr;
      typedef std::map<std::string, std::string> Headers;

      static Ptr create();
  
      virtual ~IHttpReply() {}

    public:
      // TODO: hide publics
      HttpErrorCode error;
      std::string error_detail;
      int status;
      Headers headers;
      std::string body;
      std::string content_type;
    };
  }
}

#endif
