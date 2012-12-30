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

#include "HttpSession.hh"
#include "HttpUtils.hh"
#include "HttpRequest.hh"
#include "HttpServer.hh"
#include "HttpStreamOperation.hh"
#include "HttpOperation.hh"

using namespace std;
using namespace workrave::rest;

IHttpSession::Ptr
IHttpSession::create(const std::string &user_agent)
{
  return Ptr(new HttpSession(user_agent));
}


HttpSession::HttpSession(const std::string &user_agent) : user_agent(user_agent)
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
      throw HttpError(HttpErrorCode::Failure, "Cannot create async SOUP session.");
    }

  SoupRequester *requester = soup_requester_new();
	soup_session_add_feature(session, SOUP_SESSION_FEATURE(requester));
	g_object_unref(requester);
}


HttpSession::~HttpSession()
{
  if (session != NULL)
    {
      g_object_unref(session);
    }  
}


IHttpOperation::Ptr
HttpSession::send(IHttpRequest::Ptr request, Ready ready)
{
  HttpRequest::Ptr r = boost::dynamic_pointer_cast<HttpRequest>(request);
  r->set_filters(request_filters);
  
  HttpOperation::Ptr op = HttpOperation::create(session, r);
  if (!ready.empty())
    {
      op->signal_reply().connect(ready);
    }
  op->start();

  return op;
}

IHttpOperation::Ptr
HttpSession::request(IHttpRequest::Ptr request)
{
  HttpRequest::Ptr r = boost::dynamic_pointer_cast<HttpRequest>(request);
  r->set_filters(request_filters);
  
  return HttpOperation::create(session, r);
}

IHttpStreamOperation::Ptr
HttpSession::stream(IHttpRequest::Ptr request)
{
  HttpRequest::Ptr r = boost::dynamic_pointer_cast<HttpRequest>(request);
  r->set_filters(request_filters);

  return HttpStreamOperation::create(session, r);
}

IHttpServer::Ptr
HttpSession::listen(const std::string &path, IHttpServer::RequestCallback callback)
{
  HttpServer::Ptr server = HttpServer::create(user_agent, path, callback);
  if (server)
    {
      server->start();
    }
  return server;
}


void
HttpSession::add_request_filter(IHttpRequestFilter::Ptr filter)
{
  request_filters.push_back(filter);
}


void
HttpSession::remove_request_filter(IHttpRequestFilter::Ptr filter)
{
  request_filters.remove(filter);
}
