// Copyright (C) 2013 by Rob Caelers <robc@krandor.nl>
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

#include "CloudControl.hh"

#include <stdio.h>
#include <time.h>

#include <glib.h>

#include <iostream>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "json/json.h"

#include "rest/IOAuth2.hh"
#include "rest/IHttpSession.hh"
#include "rest/AuthError.hh"

#include "Util.hh"

using namespace std;
using namespace workrave::cloud;
using namespace workrave::rest;

CloudControl::Ptr
CloudControl::create()
{
  return Ptr(new CloudControl());
}


CloudControl::CloudControl()
{
}


CloudControl::~CloudControl()
{
}

void
CloudControl::init()
{
  WorkraveAuth::Ptr auth = WorkraveAuth::create();
  auth->init(boost::bind(&CloudControl::on_auth_ready, this, _1, _2, auth));
}


void
CloudControl::init_myid()
{
  bool ok = false;
  stringstream ss;

  ss << Util::get_home_directory() << "id" << ends;
  string idfilename = ss.str();

  if (Util::file_exists(idfilename))
    {
      ifstream file(idfilename.c_str());
      
      if (file)
        {
          string id_str;
          file >> id_str;

          myid = UUID::from_str(id_str);              

          file.close();
        }
    }

  if (! ok)
    {
      ofstream file(idfilename.c_str());

      file << myid.str() << endl;
      file.close();
    }
}

void
CloudControl::on_auth_ready(AuthErrorCode error, const string &detal, WorkraveAuth::Ptr auth)
{
  session = auth->get_session();

  signon();
}


void
CloudControl::signon()
{
  Json::Value root;
  root["client"]["version"] = VERSION;
  root["client"]["instance"] = myid.str();

  Json::FastWriter writer;
  std::string body = writer.write(root);
  
  IHttpRequest::Ptr request = IHttpRequest::create();
  request->uri = boost::str(boost::format("http://localhost:8000/api/signon/%1%/") % myid);
  request->content_type = "application/json";
  request->body = body;
  request->method = "POST";
  
  session->send(request, boost::bind(&CloudControl::on_signon_ready, this, _1));
}


void
CloudControl::on_signon_ready(IHttpReply::Ptr reply)
{
  Json::Value root;
  Json::Reader reader;
  bool ok = reader.parse(reply->body, root);

  if (!ok)
    {
      g_debug("Unable to parse JSON data: %s", reply->body.c_str());
      return;
    }

  g_debug("bosy = %s", reply->body.c_str());
  g_debug("root = %s", root.toStyledString().c_str());
  g_debug("root = %d", root.type());
  
  if (root.isMember("error"))
    {
      g_debug("Unable to parse JSON data: %s", root["error"].asString().c_str());
      return;
    }
      
  if (root.isMember("event_url"))
    {
      event_url = root["event_url"].asString();
    }
  
  subscribe();
}

void
CloudControl::subscribe()
{
  IHttpRequest::Ptr request = IHttpRequest::create();
  request->uri = event_url;
  request->method = "GET";

  IHttpStreamOperation::Ptr stream = session->stream(request);
  stream->signal_headers().connect(boost::bind(&CloudControl::on_event_headers, this, _1));
  stream->signal_closed().connect(boost::bind(&CloudControl::on_event_closed, this, _1, _2, stream));
  stream->signal_data().connect(boost::bind(&CloudControl::on_event_data, this, _1));
  stream->start();
}


void
CloudControl::on_event_headers(IHttpReply::Ptr reply)
{
  g_debug("event headers : %d %s", reply->status, reply->body.c_str());
  for (map<string, string>::const_iterator i = reply->headers.begin(); i != reply->headers.end(); i++)
    {
      g_debug("  header %s -> %s", i->first.c_str(), i->second.c_str());
    }
}


void
CloudControl::on_event_data(const std::string &data)
{
  g_debug("event data : %s", data.c_str());
}


void
CloudControl::on_event_closed(HttpErrorCode error, const std::string &detail, IHttpStreamOperation::Ptr stream)
{
  g_debug("event closed : %d %s", error, detail.c_str());
}
