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

#ifndef HTTPCLIENT_HH
#define HTTPCLIENT_HH

#ifdef HAVE_SOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include <boost/shared_ptr.hpp>

#include "rest/IHttpClient.hh"

class HttpClient : public IHttpClient
{
public:
  typedef boost::shared_ptr<HttpClient> Ptr;

  static Ptr create(const std::string &user_agent);

  HttpClient(const std::string &user_agent);
  virtual ~HttpClient();

  virtual void set_request_filter(IHttpRequestFilter::Ptr filter);
  virtual IHttpReply::Ptr execute(IHttpRequest::Ptr request, IHttpClient::HttpBackendReady callback);
  virtual IHttpReply::Ptr stream(IHttpRequest::Ptr request, IHttpClient::HttpBackendReady callback);
  
private:
  typedef boost::function<void (IHttpRequest::Ptr request, IHttpClient::HttpBackendReady callback) > Execute;

  struct RequestData
  {
    RequestData(gsize size);
    ~RequestData();
  
    IHttpReply::Ptr reply;
    IHttpClient::HttpBackendReady callback;
    char *buffer;
    gsize size;
  };

  void execute(IHttpRequest::Ptr request, IHttpReply::Ptr reply, IHttpClient::HttpBackendReady callback);
  void stream(IHttpRequest::Ptr request, IHttpReply::Ptr reply, IHttpClient::HttpBackendReady callback);
  
  static void stream_closed(GObject *source, GAsyncResult *res, gpointer user_data);
  static void send_ready(GObject *source, GAsyncResult *res, gpointer user_data);
  static void read_ready(GObject *source, GAsyncResult *res, gpointer user_data);

  static void reply_ready(SoupSession *session, SoupMessage *message, gpointer user_data);
  static void got_headers(SoupMessage *msg, gpointer user_data);

  SoupSession *session;
  IHttpRequestFilter::Ptr request_filter;
};

#endif
