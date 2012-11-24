// Copyright (C) 2010, 2011, 2012 by Rob Caelers <robc@krandor.nl>
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
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>

#include "HttpDecorator.hh"
#include "rest/HttpReply.hh"

class OAuth2Filter : public HttpDecorator
{
public:
  typedef boost::shared_ptr<OAuth2Filter> Ptr;

  typedef boost::signals2::signal<void()> RefreshRequestSignal;
  
public:
  static Ptr create(IHttpExecute::Ptr);

  OAuth2Filter(IHttpExecute::Ptr executor);
  void set_access_token(const std::string &acces_token);

  RefreshRequestSignal &signal_refresh_request()
  {
    return refresh_request_signal;
  }

  virtual HttpReply::Ptr execute(HttpExecuteReady callback = 0);
  
private:
  void filter();
  void on_reply(HttpReply::Ptr reply);

  std::string access_token;
  RefreshRequestSignal refresh_request_signal;
  bool waiting;
  HttpExecuteReady callback;
};

#endif // OAUTH2FILTER_HH
