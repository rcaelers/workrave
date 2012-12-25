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

#include <boost/bind.hpp>

#define LIBSOUP_USE_UNSTABLE_REQUEST_API
#include <libsoup/soup-requester.h>
#include <libsoup/soup-request.h>
#include <libsoup/soup-request-http.h>

#include "HttpClient.hh"
#include "HttpUtils.hh"

using namespace std;

HttpClient::Ptr
HttpClient::create(const std::string &user_agent)
{
  return Ptr(new HttpClient(user_agent));
}


HttpClient::HttpClient(const std::string &user_agent)
{
  session = soup_session_async_new_with_options(
#ifdef HAVE_SOUP_GNOME
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
			SOUP_SESSION_USER_AGENT, user_agent.c_str(),
			SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
      SOUP_SESSION_USE_THREAD_CONTEXT, TRUE,
			NULL);

  if (session == NULL)
    {
      //throw std::bad_alloc("Cannot create async SOUP session.");
    }

  SoupRequester *requester = soup_requester_new();
	soup_session_add_feature(session, SOUP_SESSION_FEATURE(requester));
	g_object_unref(requester);
}


HttpClient::~HttpClient()
{
  if (session != NULL)
    {
      g_object_unref(session);
    }  
}



IHttpReply::Ptr
HttpClient::execute(IHttpRequest::Ptr request, IHttpClient::HttpBackendReady callback)
{
  IHttpReply::Ptr reply = IHttpReply::create();

  if (callback.empty())
    {
      throw std::invalid_argument("Callback cannot be empty");
    }

  if (request_filter)
    {
      request_filter->filter(request, boost::bind(&HttpClient::execute, this, request, reply, callback));
    }
  else
    {
      execute(request, reply, callback);
    }

  return reply;
}

void
HttpClient::execute(IHttpRequest::Ptr request, IHttpReply::Ptr reply, IHttpClient::HttpBackendReady callback)
{
  RequestData *data = new RequestData(0);
  data->reply = reply;
  data->callback = callback;
  
  SoupMessage *message = HttpUtils::create_request_message(request);
  soup_session_queue_message(session, message, reply_ready, data);
}


IHttpReply::Ptr
HttpClient::stream(IHttpRequest::Ptr request, IHttpClient::HttpBackendReady callback)
{
  IHttpReply::Ptr reply = IHttpReply::create();

  if (callback.empty())
    {
      throw std::invalid_argument("Callback cannot be empty");
    }
  
  if (request_filter)
    {
      g_debug("Using filter");
      request_filter->filter(request, boost::bind(&HttpClient::stream, this, request, reply, callback));
    }
  else
    {
      stream(request, reply, callback);
    }

  return reply;
}


void
HttpClient::stream(IHttpRequest::Ptr request, IHttpReply::Ptr reply, IHttpClient::HttpBackendReady callback)
{
  RequestData *data = new RequestData(1024);
  data->reply = reply;
  data->callback = callback;
      
  SoupRequester *requester = SOUP_REQUESTER(soup_session_get_feature(session, SOUP_TYPE_REQUESTER));

  SoupURI *uri = soup_uri_new(request->uri.c_str());
  SoupRequest *soup_request = soup_requester_request_uri(requester, uri, NULL);
  SoupMessage *message = soup_request_http_get_message(SOUP_REQUEST_HTTP(soup_request));

  if (request->body != "")
    {
      soup_message_set_request(message, request->content_type.c_str(), SOUP_MEMORY_COPY,
                               request->body.c_str(), request->body.size());
    }

  for (map<string, string>::const_iterator i = request->headers.begin(); i != request->headers.end(); i++)
    {
      soup_message_headers_append(message->request_headers, i->first.c_str(), i->second.c_str());
    }
  
	soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);
  
  g_signal_connect(message, "got-headers", G_CALLBACK(got_headers), data);
          
  soup_request_send_async(soup_request, NULL, send_ready, data);

  soup_uri_free(uri);
  
}

void
HttpClient::set_request_filter(IHttpRequestFilter::Ptr filter)
{
  request_filter = filter;
}

HttpClient::RequestData::RequestData(gsize size)
{
  buffer = new char[size];
  this->size = size;
}


HttpClient::RequestData::~RequestData()
{
  delete [] buffer;
}


void
HttpClient::got_headers(SoupMessage *message, gpointer user_data)
{
  RequestData *data = (RequestData *) user_data; 
  HttpUtils::process_reply_message(data->reply, message);

  data->reply->chunked = true;
  data->callback(data->reply);
}


void
HttpClient::stream_closed(GObject *source, GAsyncResult *res, gpointer user_data)
{
	GInputStream *stream = G_INPUT_STREAM(source);
	GError *error = NULL;

	if (!g_input_stream_close_finish(stream, res, &error))
    {
      g_debug("close failed: %s", error->message);
      g_clear_error(&error);
    }
}


void
HttpClient::read_ready(GObject *source, GAsyncResult *res, gpointer user_data)
{
  RequestData *data = (RequestData *) user_data; 
	GInputStream *stream = G_INPUT_STREAM(source);
	GError *error = NULL;
  
	gssize nread = g_input_stream_read_finish(stream, res, &error);
	if (nread == -1)
    {
      g_debug("read_async failed: %s\n", error->message);
      g_error_free(error);
      g_input_stream_close(stream, NULL, NULL);
      g_object_unref(stream);
      delete data;
	}
  else if (nread == 0)
    {
      g_debug("read_async eof\n");
      g_input_stream_close_async(stream, G_PRIORITY_DEFAULT, NULL, stream_closed, NULL);

      data->reply->chunked = true;
      data->reply->final = true;
      data->callback(data->reply);
      delete data;
    }
  else
    {
      IHttpReply::Ptr reply = IHttpReply::create();
      reply->body = string(data->buffer, nread);

      g_debug("read: %s\n", string(data->buffer, nread).c_str());

      g_input_stream_read_async(stream, data->buffer, data->size, G_PRIORITY_DEFAULT, NULL, read_ready, user_data);
    }
}


void
HttpClient::send_ready(GObject *source, GAsyncResult *res, gpointer user_data)
{
  RequestData *data = (RequestData *) user_data; 
	GError *error = NULL;

	GInputStream *stream = soup_request_send_finish(SOUP_REQUEST(source), res, &error);
  if (!stream)
    {
      g_debug("send_async failed: %s\n", error->message);
      g_clear_error(&error);

      delete data;
      return;
    }

	g_input_stream_read_async(stream, data->buffer, data->size, G_PRIORITY_DEFAULT, NULL, read_ready, user_data);
}


void
HttpClient::reply_ready(SoupSession *session, SoupMessage *message, gpointer user_data)
{
  RequestData *data = (RequestData *) user_data;

  HttpUtils::process_reply_message(data->reply, message);
  data->callback(data->reply);

  delete data;
}
