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

#include "OAuth2Filter.hh"
#include <glib.h>

using namespace std;

OAuth2Filter::Ptr
OAuth2Filter::create(IHttpExecute::Ptr executor)
{
  return Ptr(new OAuth2Filter(executor));
}


OAuth2Filter::OAuth2Filter(IHttpExecute::Ptr executor) : HttpDecorator(executor)
{
  this->waiting = false;
}


void
OAuth2Filter::set_access_token(const std::string &access_token)
{
  g_debug("OAuth2Filter::set_access_token %s", access_token.c_str());
  this->access_token = access_token;
  if (waiting)
    {
      execute(callback);
    }
}


void
OAuth2Filter::filter()
{
  if (access_token != "")
    {
      get_request()->headers["Authorization"] = string("Bearer ") + access_token;
    }
}


HttpReply::Ptr
OAuth2Filter::execute(IHttpExecute::HttpExecuteReady callback)
{
  g_debug("OAuth2Filter:execute");
  HttpReply::Ptr reply;

  this->callback = callback;
  this->waiting = false;
  
  filter();

  if (is_sync())
    {
      g_debug("OAuth2Filter:execute sync");
      reply = executor->execute();
      on_reply(reply);
    }
  else if (!callback.empty())
    {
      g_debug("OAuth2Filter:execute async");
      reply = executor->execute(boost::bind(&OAuth2Filter::on_reply, this, _1));
    }
  g_debug("OAuth2Filter:execute ok");
  return reply;
}

void
OAuth2Filter::on_reply(HttpReply::Ptr reply)
{
  g_debug("oauth2filter reply async : %d %s", reply->status, reply->body.c_str());
  if (reply->status == 401)
    {
      waiting = true;
      refresh_request_signal();
    }
  else
    {
      callback(reply);
    }
}
