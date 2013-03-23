// Copyright (C) 2010 - 2013 by Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_REST_IOAUTH2_HH
#define WORKRAVE_REST_IOAUTH2_HH

#include <string>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "rest/IHttpRequestFilter.hh"
#include "rest/OAuth2Settings.hh"

#include "rest/AuthError.hh"

namespace workrave
{
  namespace rest
  {
    class IOAuth2 : public boost::enable_shared_from_this<IOAuth2>
    {
    public:
      typedef boost::shared_ptr<IOAuth2> Ptr;
      typedef boost::function<void (AuthErrorCode, const std::string &) > AuthResultCallback;

      typedef boost::function<void (const std::string &, const std::string &, IHttpReply::Ptr) > ShowUserfeedbackCallback;

    public:
      static Ptr create(const OAuth2Settings &settings);

      virtual IHttpRequestFilter::Ptr create_filter() = 0;
  
      virtual void init(AuthResultCallback callback, ShowUserfeedbackCallback feedback) = 0;
      virtual void init(std::string access_token, std::string refresh_token, time_t valid_until, AuthResultCallback callback, ShowUserfeedbackCallback feedback) = 0;

      virtual void get_tokens(std::string &access_token, std::string &refresh_token, time_t &valid_until) = 0;
    };
  }
}

#endif
