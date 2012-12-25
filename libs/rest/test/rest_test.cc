// main.cc --- OAuth test app
//
// Copyright (C) 2010, 2011, 2012 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include <boost/bind.hpp>

#include <glib.h>
#include <glib-object.h>

#include "WorkraveAuth.hh"
#include "rest/IHttpClient.hh"
#include "rest/IOAuth2.hh"

using namespace std;

static GMainLoop *loop = NULL;

static void
on_reply(IHttpReply::Ptr reply)
{
  g_debug("wr async : %d %s", reply->status, reply->body.c_str());
  for (map<string, string>::const_iterator i = reply->headers.begin(); i != reply->headers.end(); i++)
    {
      g_debug("wr async : header %s -> %s", i->first.c_str(), i->second.c_str());
    }
}

static void
on_auth_ready(bool success, WorkraveAuth::Ptr auth)
{
  g_debug("ready: test streaming");
  IHttpClient::Ptr backend = auth->get_backend();

  IHttpRequest::Ptr request = IHttpRequest::create();
  request->uri = "http://localhost:8000/stream1/";
  request->method = "GET";
  
  IHttpReply::Ptr reply = backend->stream(request, boost::bind(on_reply, _1));
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  loop = g_main_loop_new(NULL, TRUE);

  WorkraveAuth::Ptr auth = WorkraveAuth::create();
  auth->init(boost::bind(on_auth_ready, _1, auth));
  
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
