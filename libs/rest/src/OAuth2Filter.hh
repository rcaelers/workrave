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

#ifndef OAUTH2FILTER_HH
#define OAUTH2FILTER_HH

#include <string>
#include <list>

#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>

#include "rest/IHttpOperation.hh"
#include "rest/IHttpReply.hh"
#include "rest/IOAuth2.hh"

#include "OAuth2.hh"

class OAuth2Filter : public workrave::rest::IHttpRequestFilter
{
public:
  typedef boost::shared_ptr<OAuth2Filter> Ptr;
  
public:
  static Ptr create(workrave::rest::IOAuth2::Ptr oauth);

  OAuth2Filter(workrave::rest::IOAuth2::Ptr oauth);

  virtual void filter(workrave::rest::IHttpRequest::Ptr request, Ready callback = 0);

private:
  void on_access_token(const std::string &acces_token, time_t valid_until);

  struct RequestData
  {
    workrave::rest::IHttpRequest::Ptr request;
    workrave::rest::IHttpRequestFilter::Ready callback;
  };

  OAuth2::Ptr oauth2;
  std::string access_token;
  time_t valid_until;
  std::list<RequestData> waiting;
};

#endif // OAUTH2FILTER_HH
