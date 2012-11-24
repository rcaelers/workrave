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

#include "OAuth1.hh"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <glib.h>

#ifdef HAVE_CRYPTOPP  
#include <crypto++/cryptlib.h>
#include <crypto++/sha.h>
#include <crypto++/hmac.h>
#include <crypto++/base64.h>
#endif

#include "rest/IHttpBackend.hh"
#include "OAuth1.hh"
#include "Uri.hh"

using namespace std;

IOAuth1::Ptr
IOAuth1::create(IHttpBackend::Ptr backend, const Settings &settings)
{
  return OAuth1::create(backend, settings);
}

OAuth1::Ptr
OAuth1::create(IHttpBackend::Ptr backend, const Settings &settings)
{
  return Ptr(new OAuth1(backend, settings));
}

OAuth1::OAuth1(IHttpBackend::Ptr backend, const OAuth1::Settings &settings)
  : backend(backend),
    settings(settings)
{
  verified_path = "/oauth-verfied";
  filter = OAuth1Filter::create();

  backend->set_decorator_factory(boost::dynamic_pointer_cast<OAuth1>(shared_from_this()));
}

OAuth1::~OAuth1()
{
  if (server)
    {
      server->stop();
    }
  filter->set_custom_headers();
}

void
OAuth1::init(const string &consumer_key, const string &consumer_secret,
                    SuccessCallback success_cb, FailedCallback failure_cb)
{
  filter->set_consumer(consumer_key, consumer_secret);

  this->success_cb = success_cb;
  this->failure_cb = failure_cb;
  
  request_temporary_credentials();
}


void
OAuth1::request_temporary_credentials()
{
  try
    {
      int port = 0;
      server = backend->listen(verified_path, port, boost::bind(&OAuth1::on_resource_owner_authorization_ready, this, _1));

      oauth_callback = boost::str(boost::format("http://127.0.0.1:%1%%2%") % port % verified_path);
      
      OAuth1Filter::RequestParams parameters;
      parameters["oauth_callback"] = oauth_callback;
      filter->set_custom_headers(parameters);

      HttpRequest::Ptr request = HttpRequest::create();
      request->uri = settings.temporary_request_uri;
      request->method = "POST";
      
      backend->request(request, boost::bind(&OAuth1::on_temporary_credentials_ready, this, _1));
    }
  catch(...)
    {
      failure();
    }
}


void
OAuth1::on_temporary_credentials_ready(HttpReply::Ptr reply)
{
  try
    {
      g_debug("Resp : %s", reply->body.c_str());
      if (reply->status != 200)
        {
          g_debug("Invalid response for temporary credentials %d", reply->status);
          throw std::exception();
        }

      if (reply->body == "")
        {
          g_debug("Empty response for temporary credentials");
          throw std::exception();
        }          
      
      OAuth1Filter::RequestParams response_parameters;
      parse_query(reply->body, response_parameters);

#ifdef DROPBOX
      if (response_parameters["oauth_callback_confirmed"] != "true")
        {
          g_debug("Callback not confirmed:%s", response_parameters["oauth_callback_confirmed"].c_str());
          throw std::exception();
        }
#endif
      
      token = response_parameters["oauth_token"];
      string secret = response_parameters["oauth_token_secret"];

      filter->set_token(token, secret);

      request_resource_owner_authorization();
    }
  catch(...)
    {
      failure();
    }
}

void
OAuth1::request_resource_owner_authorization()
{
  try
    {
      gchar *program = g_find_program_in_path("xdg-open");
      if (program == NULL)
        {
          g_debug("Cannot find xdg-open");
          throw std::exception();
        }

      string command = boost::str(boost::format("%1% %2%?oauth_token=%3%&oauth_callback=%4%")
                                  % program % settings.authorize_uri % Uri::escape(token) % Uri::escape(oauth_callback));
 
      gint exit_code;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
        {
          g_debug("Failed to execute xdg-open");
          throw std::exception();
        }

      if (WEXITSTATUS(exit_code) != 0)
        {
          g_debug("xdg-open returned an error exit-code");
          throw std::exception();
        }
    }
  catch(...)
    {
      failure();
    }
}


HttpReply::Ptr
OAuth1::on_resource_owner_authorization_ready(HttpRequest::Ptr request)
{
  HttpReply::Ptr reply = HttpReply::create(request);
  
  try
    {
      reply->content_type = "text/html";
      reply->body = settings.failure_html;
      
      if (request->method != "GET")
        {
          g_debug("Resource owner authorization only supports GET callback");
          throw std::exception();
        }          

      if (request->uri == "")
        {
          g_debug("Empty response for resource owner authorization");
          throw std::exception();
        }          

      OAuth1Filter::RequestParams response_parameters;
      parse_query(request->uri, response_parameters);

      string token = response_parameters["oauth_token"];
      string verifier = response_parameters["oauth_verifier"];

      if (token == "")
        {
          g_debug("Token must be set");
          throw std::exception();
        }

      if (token != this->token)
        {
          g_debug("Reply for incorrect");
          throw std::exception();
        }

      reply->body = settings.success_html;

      request_token(verifier);
    }
  catch(...)
    {
      failure();
    }

  return reply;
}


void
OAuth1::request_token(const string &verifier)
{
  g_debug("request_token %s", verifier.c_str());

  try
    {
      OAuth1Filter::RequestParams parameters;
      if (verifier != "")
        {
          parameters["oauth_verifier"] = verifier;
        }
      filter->set_custom_headers(parameters);

      HttpRequest::Ptr request = HttpRequest::create();
      request->uri = settings.token_request_uri;
      request->method = "POST";
      
      backend->request(request, boost::bind(&OAuth1::on_token_ready, this, _1));
    }
  catch(...)
    {
      failure();
    }
}


void
OAuth1::on_token_ready(HttpReply::Ptr reply)
{
  try
    {
      if (reply->status != 200)
        {
          g_debug("Invalid response for token %d", reply->status);
          throw std::exception();
        }

      if (reply->body == "")
        {
          g_debug("Empty response for token");
          throw std::exception();
        }          
      
      OAuth1Filter::RequestParams response_parameters;
      parse_query(reply->body, response_parameters);

      string key = response_parameters["oauth_token"];
      string secret = response_parameters["oauth_token_secret"];

      if (key == "" || secret == "")
        {
          g_debug("No token/secret received");
          throw std::exception();
        }

      g_debug("key %s", key.c_str());
      g_debug("Secret %s", secret.c_str());
      
      filter->set_token(key, secret);
      success();
    }
  catch(...)
    {
      failure();
    }
}


void
OAuth1::parse_query(const string &query, OAuth1Filter::RequestParams &params) const
{
  if (query != "")
    {
      g_debug("Query: %s", query.c_str());
      vector<string> query_params;
      boost::split(query_params, query, boost::is_any_of("&"));

      for (size_t i = 0; i < query_params.size(); ++i)
        {
          vector<string> param_elements;
          boost::split(param_elements, query_params[i], boost::is_any_of("="));
      
          if (param_elements.size() == 2)
            {
              params[param_elements[0]] = param_elements[1];
            }
        }
    }
}


void OAuth1::failure()
{
  if (server)
    {
      server->stop();
    }
  filter->set_custom_headers();

  failure_cb();
}


void OAuth1::success()
{
  if (server)
    {
      server->stop();
    }
  filter->set_custom_headers();

  success_cb();
}


IHttpExecute::Ptr
OAuth1::create_decorator(IHttpExecute::Ptr executor)
{
  return filter->create_decorator(executor);
}
    
