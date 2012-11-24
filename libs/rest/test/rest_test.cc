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
#include "rest/IHttpBackend.hh"
#include "rest/IOAuth2.hh"

using namespace std;

static GMainLoop *loop = NULL;

static void
on_reply(HttpReply::Ptr reply)
{
  g_debug("google async : %d %s", reply->status, reply->body.c_str());
  for (map<string, string>::const_iterator i = reply->headers.begin(); i != reply->headers.end(); i++)
    {
      g_debug("google async : header %s -> %s", i->first.c_str(), i->second.c_str());
    }
}

static void
on_auth_ready(bool success, WorkraveAuth::Ptr auth)
{
  IHttpBackend::Ptr backend = auth->get_backend();

  HttpRequest::Ptr request = HttpRequest::create();
  request->uri = "https://www.googleapis.com/drive/v2/changes?pageToken=8395";
  // https://docs.google.com/feeds/metadata/default?v=3";
  request->method = "GET";
  //request->body = "Hello World";
  
  //  HttpReply::Ptr reply = backend->request(request, boost::bind(on_reply, _1));
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
