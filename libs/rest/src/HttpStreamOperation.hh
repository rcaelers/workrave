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

#ifndef HTTPSTREAMOPERATION_HH
#define HTTPSTREAMOPERATION_HH

#ifdef HAVE_SOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "rest/IHttpStreamOperation.hh"

#include "HttpRequest.hh"
#include "HttpReply.hh"

class HttpStreamOperation : public workrave::rest::IHttpStreamOperation,
                            public boost::enable_shared_from_this<HttpStreamOperation>
{
public:
  typedef boost::shared_ptr<HttpStreamOperation> Ptr;

  static Ptr create(SoupSession *session, HttpRequest::Ptr request);

  HttpStreamOperation(SoupSession *session, HttpRequest::Ptr request);
  virtual ~HttpStreamOperation();

  virtual void start();
  virtual void cancel();

  virtual HeadersSignal &signal_headers();
  virtual DataSignal &signal_data();
  virtual ClosedSignal &signal_closed();
  
private:
  SoupSession *session;
  HttpRequest::Ptr request;
  char *buffer;
  gsize buffer_size;

  struct CallbackData
  {
    Ptr self;
  };
  
  static void stream_closed(GObject *source, GAsyncResult *res, gpointer user_data);
  static void send_ready(GObject *source, GAsyncResult *res, gpointer user_data);
  static void read_ready(GObject *source, GAsyncResult *res, gpointer user_data);
  static void got_headers(SoupMessage *msg, gpointer user_data);

  HeadersSignal headers_signal;
  DataSignal data_signal;
  ClosedSignal closed_signal;
};

#endif
