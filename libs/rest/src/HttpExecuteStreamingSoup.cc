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

#define LIBSOUP_USE_UNSTABLE_REQUEST_API
#include <libsoup/soup-requester.h>
#include <libsoup/soup-request.h>
#include <libsoup/soup-request-http.h>

#include "HttpExecuteStreamingSoup.hh"

using namespace std;

char buf[1024];

HttpExecuteStreamingSoup::Ptr
HttpExecuteStreamingSoup::create(SoupSession *session, HttpRequest::Ptr request)
{
  return Ptr(new HttpExecuteStreamingSoup(session, request));
}


HttpExecuteStreamingSoup::HttpExecuteStreamingSoup(SoupSession *session, HttpRequest::Ptr request) : HttpExecuteSoup(session, request)
{
}


HttpExecuteStreamingSoup::~HttpExecuteStreamingSoup()
{
}

static void
got_header (SoupMessage *msg, gpointer session)
{
  g_debug("headers");
  SoupMessageHeadersIter iter;
  const char *name, *value;
  soup_message_headers_iter_init(&iter, msg->response_headers);

  while (soup_message_headers_iter_next(&iter, &name, &value))
    {
      g_debug("resp header %s : %s", name, value);
    }
}


static void
stream_closed (GObject *source, GAsyncResult *res, gpointer user_data)
{
	GInputStream *stream = G_INPUT_STREAM (source);
	GError *error = NULL;

	if (!g_input_stream_close_finish (stream, res, &error))
    {
		g_debug ("    close failed: %s", error->message);
		g_error_free (error);
	}
}

static void
test_read_ready(GObject *source, GAsyncResult *res, gpointer user_data)
{
	GInputStream *stream = G_INPUT_STREAM(source);
	GError *error = NULL;

	gssize nread = g_input_stream_read_finish(stream, res, &error);
	if (nread == -1)
    {
      g_debug("    read_async failed: %s\n", error->message);
      g_error_free(error);
      g_input_stream_close(stream, NULL, NULL);
      g_object_unref(stream);
      return;
      
	}
  else if (nread == 0)
    {
      g_input_stream_close_async(stream, G_PRIORITY_DEFAULT, NULL, stream_closed, NULL);
      return;
    }
  else
    {
      g_debug("read = %s", string(buf, nread).c_str());
    }

	g_input_stream_read_async(stream, buf, sizeof(buf), G_PRIORITY_DEFAULT, NULL, test_read_ready, user_data);
}


static void
test_sent(GObject *source, GAsyncResult *res, gpointer user_data)
{
	GInputStream *stream;
	GError *error = NULL;

	stream = soup_request_send_finish(SOUP_REQUEST(source), res, &error);
  if (!stream)
    {
      g_debug ("send_async failed: %s\n", error->message);
      return;
    }

	g_input_stream_read_async(stream, buf, sizeof(buf), G_PRIORITY_DEFAULT, NULL, test_read_ready, user_data);
}


HttpReply::Ptr
HttpExecuteStreamingSoup::execute(IHttpExecute::HttpExecuteReady callback)
{
  this->callback = callback;

  if (!callback.empty())
    {
      SoupRequester *requester = SOUP_REQUESTER(soup_session_get_feature(session, SOUP_TYPE_REQUESTER));

      SoupURI *uri = soup_uri_new(request->uri.c_str());
      SoupRequest *request = soup_requester_request_uri(requester, uri, NULL);
          
      SoupMessage *msg = soup_request_http_get_message(SOUP_REQUEST_HTTP(request));

      g_signal_connect(msg, "got-headers", G_CALLBACK(got_header), session);
          
      //SoupSocket *socket = NULL;

      //guint started_id = g_signal_connect(session, "request-started", G_CALLBACK(request_started), &socket);

      soup_request_send_async(request, NULL, test_sent, NULL);

      soup_uri_free(uri);
    }

  return reply;
}


HttpRequest::Ptr
HttpExecuteStreamingSoup::get_request() const
{
  return request;
}


bool
HttpExecuteStreamingSoup::is_sync() const
{
  return false;
}

void
HttpExecuteStreamingSoup::reply_ready_static(SoupSession *session, SoupMessage *message, gpointer user_data)
{
  CallbackData *data = (CallbackData *) user_data;
  HttpExecuteStreamingSoup::Ptr self = data->self;
  self->reply_ready(session, message);
  delete data;
}

void
HttpExecuteStreamingSoup::reply_ready(SoupSession *session, SoupMessage *message)
{
  (void)session;

  process_reply_message(message);

  g_debug("reply (async) %d", reply->status);

  if (!callback.empty())
    {
      callback(reply);
    }
}
