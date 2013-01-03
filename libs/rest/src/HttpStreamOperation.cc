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

#define LIBSOUP_USE_UNSTABLE_REQUEST_API
#include <libsoup/soup-requester.h>
#include <libsoup/soup-request.h>
#include <libsoup/soup-request-http.h>

#include "rest/HttpError.hh"

#include "HttpStreamOperation.hh"
#include "HttpUtils.hh"

using namespace std;
using namespace workrave::rest;

HttpStreamOperation::Ptr
HttpStreamOperation::create(SoupSession *session, HttpRequest::Ptr request)
{
  return Ptr(new HttpStreamOperation(session, request));
}


HttpStreamOperation::HttpStreamOperation(SoupSession *session, HttpRequest::Ptr request)
  : session(session),
    request(request),
    buffer(0),
    buffer_size(0)
{
}


HttpStreamOperation::~HttpStreamOperation()
{
  delete [] buffer;
}


void
HttpStreamOperation::start()
{
  request->apply_filters([&] {
      CallbackData *data = new CallbackData;
      data->self = shared_from_this();
      g_debug("start");

      buffer_size = 1024;
      buffer = new char[buffer_size];

      SoupRequester *requester = SOUP_REQUESTER(soup_session_get_feature(session, SOUP_TYPE_REQUESTER));

      SoupURI *uri = soup_uri_new(request->uri.c_str());
      SoupRequest *soup_request = soup_requester_request_uri(requester, uri, NULL);
      SoupMessage *message = soup_request_http_get_message(SOUP_REQUEST_HTTP(soup_request));

      HttpUtils::setup_request_message(message, request);

      g_signal_connect(message, "got-headers", G_CALLBACK(got_headers), data);
          
      soup_request_send_async(soup_request, NULL, send_ready, data);

      soup_uri_free(uri);
    });
}


void
HttpStreamOperation::cancel()
{
  // TODO:
}

void
HttpStreamOperation::got_headers(SoupMessage *message, gpointer user_data)
{
  CallbackData *data = (CallbackData *) user_data; 
  HttpStreamOperation::Ptr self = data->self;

  HttpReply::Ptr reply = HttpUtils::process_reply_message(message);
  self->headers_signal(reply);
}


void
HttpStreamOperation::stream_closed(GObject *source, GAsyncResult *res, gpointer user_data)
{
	GInputStream *stream = G_INPUT_STREAM(source);
  CallbackData *data = (CallbackData *) user_data; 

	g_input_stream_close_finish(stream, res, NULL);
  delete data;
}


void
HttpStreamOperation::read_ready(GObject *source, GAsyncResult *res, gpointer user_data)
{
  CallbackData *data = (CallbackData *) user_data; 
  HttpStreamOperation::Ptr self = data->self;
	GInputStream *stream = G_INPUT_STREAM(source);
	GError *error = NULL;
  
	gssize nread = g_input_stream_read_finish(stream, res, &error);
	if (nread == -1)
    {
      g_input_stream_close(stream, NULL, NULL);
      g_object_unref(stream);

      self->closed_signal(HttpErrorCode::Failure, error->message);
      delete data;
	}
  else if (nread == 0)
    {
      g_input_stream_close_async(stream, G_PRIORITY_DEFAULT, NULL, stream_closed, user_data);
      self->closed_signal(HttpErrorCode::Success, "");
    }
  else
    {
      self->data_signal(string(self->buffer, nread));
      g_input_stream_read_async(stream, self->buffer, self->buffer_size, G_PRIORITY_DEFAULT, NULL, read_ready, user_data);
    }
  
  g_clear_error(&error);
}


void
HttpStreamOperation::send_ready(GObject *source, GAsyncResult *res, gpointer user_data)
{
  CallbackData *data = (CallbackData *) user_data; 
  HttpStreamOperation::Ptr self = data->self;
	GError *error = NULL;

	GInputStream *stream = soup_request_send_finish(SOUP_REQUEST(source), res, &error);
  if (!stream)
    {
      self->closed_signal(HttpErrorCode::Failure, error->message);
      delete data;
    }
  else
    {
      g_input_stream_read_async(stream, self->buffer, self->buffer_size, G_PRIORITY_DEFAULT, NULL, read_ready, user_data);
    }
  
  g_clear_error(&error);
}


HttpStreamOperation::HeadersSignal &
HttpStreamOperation::signal_headers()
{
  return headers_signal;
}


HttpStreamOperation::DataSignal &
HttpStreamOperation::signal_data()
{
  return data_signal;
}


HttpStreamOperation::ClosedSignal &
HttpStreamOperation::signal_closed()
{
  return closed_signal;
}
