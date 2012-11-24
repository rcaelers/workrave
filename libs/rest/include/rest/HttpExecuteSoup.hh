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

#ifndef HTTPEXECUTESOUP_HH
#define HTTPEXECUTESOUP_HH

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

#include <libsoup/soup.h>

#include "rest/HttpReply.hh"
#include "rest/IHttpBackend.hh"

class HttpExecuteSoup : public IHttpExecute
{
public:
  typedef boost::shared_ptr<HttpExecuteSoup> Ptr;

  static Ptr create(HttpRequest::Ptr request);

  HttpExecuteSoup(HttpRequest::Ptr request);
  ~HttpExecuteSoup();

  virtual HttpReply::Ptr execute(IHttpExecute::HttpExecuteReady callback = 0);
  virtual HttpRequest::Ptr get_request() const;
  virtual bool is_sync() const;
  
  void init(SoupSession *session, bool sync);
  
private:
  struct CallbackData
  {
    Ptr self;
  };

  SoupSession *session;
  IHttpExecute::HttpExecuteReady callback;
  HttpRequest::Ptr request;
  HttpReply::Ptr reply;
  bool sync;
  
  SoupMessage *create_request_message();
  void process_reply_message(SoupMessage *message);

  static void reply_ready_static(SoupSession *session, SoupMessage *message, gpointer user_data);
  void reply_ready(SoupSession *session, SoupMessage *message);
};


#endif
