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

#ifndef HTTPSESSION_HH
#define HTTPSESSION_HH

#include <list>

#ifdef HAVE_SOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include <boost/shared_ptr.hpp>

#include "rest/IHttpSession.hh"

class HttpSession : public workrave::rest::IHttpSession
{
public:
  typedef boost::shared_ptr<HttpSession> Ptr;

  HttpSession(const std::string &user_agent);
  virtual ~HttpSession();

  virtual void add_request_filter(workrave::rest::IHttpRequestFilter::Ptr filter);
  virtual void remove_request_filter(workrave::rest::IHttpRequestFilter::Ptr filter);

  virtual workrave::rest::IHttpOperation::Ptr send(workrave::rest::IHttpRequest::Ptr request, Ready ready);
  virtual workrave::rest::IHttpOperation::Ptr request(workrave::rest::IHttpRequest::Ptr request);
  virtual workrave::rest::IHttpStreamOperation::Ptr stream(workrave::rest::IHttpRequest::Ptr request);
  virtual workrave::rest::IHttpServer::Ptr listen(const std::string &path, workrave::rest::IHttpServer::RequestCallback callback);
  
private:
  SoupSession *session;
  std::list<workrave::rest::IHttpRequestFilter::Ptr> request_filters;
  const std::string &user_agent;
};

#endif
