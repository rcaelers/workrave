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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "HttpExecuteAsyncSoup.hh"

using namespace std;

HttpExecuteAsyncSoup::Ptr
HttpExecuteAsyncSoup::create(SoupSession *session, HttpRequest::Ptr request)
{
  return Ptr(new HttpExecuteAsyncSoup(session, request));
}


HttpExecuteAsyncSoup::HttpExecuteAsyncSoup(SoupSession *session, HttpRequest::Ptr request) : HttpExecuteSoup(session, request)
{
}


HttpExecuteAsyncSoup::~HttpExecuteAsyncSoup()
{
}

HttpReply::Ptr
HttpExecuteAsyncSoup::execute(IHttpExecute::HttpExecuteReady callback)
{
  this->callback = callback;

  if (!callback.empty())
    {
      CallbackData *data = new CallbackData;
      data->self = boost::dynamic_pointer_cast<HttpExecuteAsyncSoup>(shared_from_this());
      
      SoupMessage *message = create_request_message();
      soup_session_queue_message(session, message, reply_ready_static, data);
    }

  return reply;
}


HttpRequest::Ptr
HttpExecuteAsyncSoup::get_request() const
{
  return request;
}


bool
HttpExecuteAsyncSoup::is_sync() const
{
  return true;
}

void
HttpExecuteAsyncSoup::reply_ready_static(SoupSession *session, SoupMessage *message, gpointer user_data)
{
  CallbackData *data = (CallbackData *) user_data;
  HttpExecuteAsyncSoup::Ptr self = data->self;
  self->reply_ready(session, message);
  delete data;
}


void
HttpExecuteAsyncSoup::reply_ready(SoupSession *session, SoupMessage *message)
{
  (void)session;

  process_reply_message(message);

  g_debug("reply (async) %d", reply->status);

  if (!callback.empty())
    {
      callback(reply);
    }
}
