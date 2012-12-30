// main.cc --- OAuth test app
//
// Copyright (C) 2010 - 2012 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include <boost/bind.hpp>

#include <glib.h>
#include <glib-object.h>

#include "WorkraveAuth.hh"
#include "rest/IHttpSession.hh"
#include "rest/IOAuth2.hh"

using namespace std;
using namespace workrave::rest;

static GMainLoop *loop = NULL;

static void
on_headers(IHttpReply::Ptr reply)
{
  g_debug("stream headers : %d %s", reply->status, reply->body.c_str());
  for (map<string, string>::const_iterator i = reply->headers.begin(); i != reply->headers.end(); i++)
    {
      g_debug("wr async : header %s -> %s", i->first.c_str(), i->second.c_str());
    }
}

static void
on_data(const std::string &data)
{
  g_debug("streamn data : %s", data.c_str());
}

static void
on_closed(HttpErrorCode error, const std::string &detail)
{
  g_debug("streamn data : %d %s", error, detail.c_str());
}

static void
on_auth_ready(bool success, WorkraveAuth::Ptr auth)
{
  g_debug("ready: test streaming");
  IHttpSession::Ptr session = auth->get_session();

  IHttpRequest::Ptr request = IHttpRequest::create();
  request->uri = "http://localhost:8000/stream1/";
  request->method = "GET";
  
  IHttpStreamOperation::Ptr stream = session->stream(request);
  stream->signal_headers().connect(boost::bind(on_headers, _1));
  stream->signal_closed().connect(boost::bind(on_closed, _1, _2));
  stream->signal_data().connect(boost::bind(on_data, _1));
  stream->start();
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
