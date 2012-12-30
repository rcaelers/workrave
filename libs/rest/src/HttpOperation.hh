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

#ifndef HTTPOPERATION_HH
#define HTTPOPERATION_HH

#ifdef HAVE_SOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "rest/IHttpOperation.hh"

#include "HttpRequest.hh"
#include "HttpReply.hh"

class HttpOperation : public workrave::rest::IHttpOperation, public boost::enable_shared_from_this<HttpOperation>
{
public:
  typedef boost::shared_ptr<HttpOperation> Ptr;

  static Ptr create(SoupSession *session, HttpRequest::Ptr request);

  HttpOperation(SoupSession *session, HttpRequest::Ptr request);
  virtual ~HttpOperation();

  virtual void start();
  virtual void cancel();
  virtual ReplySignal &signal_reply();
  
private:
  SoupSession *session;
  HttpRequest::Ptr request;

  struct CallbackData
  {
    Ptr self;
  };
  
  static void reply_ready(SoupSession *session, SoupMessage *message, gpointer user_data);

  ReplySignal reply_signal;
};

#endif
