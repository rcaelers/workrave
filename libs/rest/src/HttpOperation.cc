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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/bind.hpp>

#include "HttpOperation.hh"
#include "HttpUtils.hh"

using namespace std;
using namespace workrave::rest;

HttpOperation::Ptr
HttpOperation::create(SoupSession *session, HttpRequest::Ptr request)
{
  return Ptr(new HttpOperation(session, request));
}


HttpOperation::HttpOperation(SoupSession *session, HttpRequest::Ptr request)
  : session(session),
    request(request)
{
}


HttpOperation::~HttpOperation()
{
}


void
HttpOperation::start()
{
  request->apply_filters([&] {
                         
      CallbackData *data = new CallbackData;
      data->self = shared_from_this();

      SoupMessage *message = HttpUtils::create_request_message(request);
      soup_session_queue_message(session, message, reply_ready, data);
    });
}


void
HttpOperation::cancel()
{
  // TODO:
}


void
HttpOperation::reply_ready(SoupSession *session, SoupMessage *message, gpointer user_data)
{
  CallbackData *data = (CallbackData *) user_data;
  HttpOperation::Ptr self = data->self;

  IHttpReply::Ptr reply = HttpUtils::process_reply_message(message);
  self->reply_signal(reply);

  delete data;
}



HttpOperation::ReplySignal &
HttpOperation::signal_reply()
{
  return reply_signal;
}
