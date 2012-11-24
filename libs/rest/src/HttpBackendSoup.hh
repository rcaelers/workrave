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

#ifndef HTTPBACKENDSOUP_HH
#define HTTPBACKENDSOUP_HH

#include <string>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>

#ifdef HAVE_SOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include "rest/IHttpBackend.hh"

class HttpBackendSoup : public IHttpBackend
{
public:
  typedef boost::shared_ptr<HttpBackendSoup> Ptr;

  static Ptr create();

public:
 	HttpBackendSoup();
  virtual ~HttpBackendSoup();

  virtual bool init(const std::string &user_agent);
  
  virtual void set_decorator_factory(IHttpDecoratorFactory::Ptr factory);

  virtual HttpReply::Ptr request(HttpRequest::Ptr request);
  virtual HttpReply::Ptr request(HttpRequest::Ptr request, const IHttpExecute::HttpExecuteReady callback);
  virtual IHttpServer::Ptr listen(const std::string &path, int &port, IHttpServer::HttpServerCallback callback);

private:
  
private:
  SoupSession *sync_session;
  SoupSession *async_session;
  SoupURI *proxy;
  std::string user_agent;
  
  IHttpDecoratorFactory::Ptr decorator_factory;
};

#endif
