// main.cc --- OAuth test app
//
// Copyright (C) 2010 - 2013 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>
#include <fstream>

#include <boost/bind.hpp>

#include <glib.h>
#include <glib-object.h>

#include "WorkraveAuth.hh"
#include "rest/IHttpSession.hh"
#include "rest/IOAuth2.hh"

#include "json/json.h"

using namespace std;
using namespace workrave::rest;

static GMainLoop *loop = NULL;

static IHttpSession::Ptr session;

static void
on_updata_stats(IHttpReply::Ptr reply)
{
  g_debug("update stats: %d %s", reply->status, reply->body.c_str());
  for (map<string, string>::const_iterator i = reply->headers.begin(); i != reply->headers.end(); i++)
    {
      g_debug("  header %s -> %s", i->first.c_str(), i->second.c_str());
    }
}

static void update_stats()
{
  Json::Value root;
  Json::Reader reader;
  std::ifstream stats_file("stats.json", std::ifstream::binary);
  
  bool ok = reader.parse(stats_file, root, false);
  if (!ok)
    {
      g_debug("Parsing failed: %s", reader.getFormatedErrorMessages().c_str());
      return;
    }

  
  std::string encoding = root.get("encoding", "UTF-8" ).asString();
  std::cout << encoding << "\n";

  Json::FastWriter writer;
  std::string out = writer.write(root);
 
   std::cout << out << endl;

  IHttpRequest::Ptr request = IHttpRequest::create();
  request->uri = "http://localhost:8000/api/statistics/delta/";
  request->content_type = "application/json";
  request->body = out;
  request->method = "POST";
  
  IHttpOperation::Ptr op = session->send(request, boost::bind(on_updata_stats, _1));
}


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
on_closed(HttpErrorCode error, const std::string &detail, IHttpStreamOperation::Ptr stream)
{
  g_debug("streamn data : %d %s", error, detail.c_str());
  //stream->start();
}

// static void
// on_reply(IHttpReply::Ptr reply)
// {
//   g_debug("reply : %d %s", reply->status, reply->body.c_str());
//   for (map<string, string>::const_iterator i = reply->headers.begin(); i != reply->headers.end(); i++)
//     {
//       g_debug("replt : header %s -> %s", i->first.c_str(), i->second.c_str());
//     }
// }

static void
on_auth_ready(AuthErrorCode error, const string &detal, WorkraveAuth::Ptr auth)
{
  g_debug("ready: test streaming");
  session = auth->get_session();

  update_stats();
  
  IHttpRequest::Ptr request = IHttpRequest::create();
  request->uri = "http://localhost:8000/api/stream1/xx/";
  request->method = "GET";
  
  IHttpStreamOperation::Ptr stream = session->stream(request);
  stream->signal_headers().connect(boost::bind(on_headers, _1));
  stream->signal_closed().connect(boost::bind(on_closed, _1, _2, stream));
  stream->signal_data().connect(boost::bind(on_data, _1));
  stream->start();
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  //g_type_init();

  loop = g_main_loop_new(NULL, TRUE);

  WorkraveAuth::Ptr auth = WorkraveAuth::create();
  auth->init(boost::bind(on_auth_ready, _1, _2, auth));
  
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
