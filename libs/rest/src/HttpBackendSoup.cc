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

#include "HttpBackendSoup.hh"

#include <glib.h>

#include "Uri.hh"
#include "rest/HttpExecuteSoup.hh"
#include "HttpServerSoup.hh"

using namespace std;

IHttpBackend::Ptr
IHttpBackend::create()
{
  return HttpBackendSoup::create();
}

HttpBackendSoup::Ptr
HttpBackendSoup::create()
{
  return Ptr(new HttpBackendSoup());
}


HttpBackendSoup::HttpBackendSoup()
  : sync_session(NULL),
    async_session(NULL),
    proxy(NULL)
{
}


HttpBackendSoup::~HttpBackendSoup()
{
  if (sync_session != NULL)
    {
      g_object_unref(sync_session);
    }

  if (async_session != NULL)
    {
      g_object_unref(async_session);
    }
}


void
HttpBackendSoup::set_decorator_factory(IHttpDecoratorFactory::Ptr factory)
{
  this->decorator_factory = factory;
}


HttpReply::Ptr
HttpBackendSoup::request(HttpRequest::Ptr request)
{
  HttpExecuteSoup::Ptr exec = HttpExecuteSoup::create(request);
  exec->init(sync_session, true);

  IHttpExecute::Ptr wrapped_exec = exec;
  if (decorator_factory)
    {
      wrapped_exec = decorator_factory->create_decorator(exec);
    }
      
  return wrapped_exec->execute();
}


HttpReply::Ptr
HttpBackendSoup::request(HttpRequest::Ptr request, IHttpExecute::HttpExecuteReady callback)
{
  HttpExecuteSoup::Ptr exec = HttpExecuteSoup::create(request);
  exec->init(async_session, false);

  IHttpExecute::Ptr wrapped_exec = exec;
  if (decorator_factory)
    {
      wrapped_exec = decorator_factory->create_decorator(exec);
    }
      
  return wrapped_exec->execute(callback);
}


IHttpServer::Ptr
HttpBackendSoup::listen(const string &path, int &port, IHttpServer::HttpServerCallback callback)
{
  HttpServerSoup::Ptr server = HttpServerSoup::create(callback, user_agent, path);
  port = server->start();
  return server;
}


bool
HttpBackendSoup::init(const string &user_agent)
{
  this->user_agent = user_agent;
  
  async_session = soup_session_async_new_with_options(
#ifdef HAVE_SOUP_GNOME
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
			SOUP_SESSION_USER_AGENT, user_agent.c_str(),
			SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
			NULL);

  if (async_session == NULL)
    {
      g_debug("HttpBackendSoup::init: cannot create async session");
      return false;
    }
  
  sync_session = soup_session_sync_new_with_options (
#ifdef HAVE_SOUP_GNOME
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
			SOUP_SESSION_USER_AGENT, user_agent.c_str(),
			SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
			NULL);

  if (sync_session == NULL)
    {
      g_debug("HttpBackendSoup::init: cannot create sync session");
      return false;
    }
  
  if (proxy)
    {
      g_object_set(G_OBJECT(async_session), SOUP_SESSION_PROXY_URI, proxy, NULL);
      g_object_set(G_OBJECT(sync_session), SOUP_SESSION_PROXY_URI, proxy, NULL);
    }

  return true;
}
