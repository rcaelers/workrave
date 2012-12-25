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

#include <boost/bind.hpp>

#include "OAuth2Filter.hh"
#include <glib.h>

using namespace std;

OAuth2Filter::Ptr
OAuth2Filter::create(IOAuth2::Ptr oauth)
{
  return Ptr(new OAuth2Filter(oauth));
}


OAuth2Filter::OAuth2Filter(IOAuth2::Ptr oauth)
{
  oauth2 = boost::dynamic_pointer_cast<OAuth2>(oauth);

  oauth2->signal_credentials_updated().connect(boost::bind(&OAuth2Filter::on_access_token, this, _1, _2));

  string refresh_token;
  oauth2->get_tokens(access_token, refresh_token, valid_until);
  g_debug("OAuth2Filter credentials %s %ld", access_token.c_str(), valid_until);
}


void
OAuth2Filter::on_access_token(const std::string &access_token, time_t valid_until)
{
  g_debug("OAuth2Filter::on_new_credentials %s", access_token.c_str());
  this->access_token = access_token;
  this->valid_until = valid_until;
  for (list<RequestData>::const_iterator it = waiting.begin(); it != waiting.end(); it++)
    {
      const RequestData &data = *it;

      data.request->headers["Authorization"] = string("Bearer ") + access_token;
      data.callback();
    }
}


void
OAuth2Filter::filter(IHttpRequest::Ptr request, Ready callback)
{
  g_debug("OAuth2Filter:execute");
  IHttpReply::Ptr reply;
          
  if (access_token != "")
    {
      if (time(NULL) + 60 > valid_until)
        {
          RequestData data;
          data.request = request;
          data.callback = callback;
          
          waiting.push_back(data);

          if (waiting.size() == 0)
            {
              oauth2->refresh_access_token();
            }
          return;
        }

      g_debug("OAuth2Filter::filter %s", access_token.c_str());
      
      request->headers["Authorization"] = string("Bearer ") + access_token;
    }

  callback();
}
