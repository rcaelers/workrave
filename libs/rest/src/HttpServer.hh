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

#ifndef HTTPSERVER_HH
#define HTTPSERVER_HH

#include <string>
#include <boost/shared_ptr.hpp>

#ifdef HAVE_SOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include "rest/IHttpServer.hh"

class HttpServer : public workrave::rest::IHttpServer
{
public:
  typedef boost::shared_ptr<HttpServer> Ptr;

  static Ptr create(const std::string &user_agent, const std::string &path, workrave::rest::IHttpServer::RequestCallback callback);

public:
 	HttpServer(const std::string &user_agent, const std::string &path, workrave::rest::IHttpServer::RequestCallback callback);
  virtual ~HttpServer();

  void start();
  
  virtual void stop();
  virtual int get_port() const;
  
private:
  static void server_callback_static(SoupServer *server, SoupMessage *message, const char *path,
                                     GHashTable *query, SoupClientContext *context, gpointer data);

  void server_callback(SoupServer *, SoupMessage *message, const char *path,
                       GHashTable *query, SoupClientContext *context);
  
private:
  const std::string &user_agent;
  std::string path;
  workrave::rest::IHttpServer::RequestCallback callback;
  SoupServer *server;
  int port;
};

#endif
