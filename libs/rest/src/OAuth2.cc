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

#include "OAuth2.hh"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <glib.h>
#include "json/json.h"

#include "Uri.hh"

#include "rest/RestFactory.hh"

#include "OAuth2Filter.hh"

using namespace std;

IOAuth2::Ptr
IOAuth2::create(const Settings &settings)
{
  return OAuth2::create(settings);
}

OAuth2::Ptr
OAuth2::create(const Settings &settings)
{
  return Ptr(new OAuth2(settings));
}

OAuth2::OAuth2(const OAuth2::Settings &settings)
  : settings(settings),
    valid_until(0)
{
  callback_uri_path = "/oauth2";

  client = RestFactory::create_client();
  server = RestFactory::create_server();
}


OAuth2::~OAuth2()
{
  if (server)
    {
      server->stop();
    }
}


void
OAuth2::init(AuthReadyCallback callback)
{
  client->set_request_filter(create_filter());

  this->callback = callback;
  request_authorization_grant();
}


void
OAuth2::init(std::string access_token, std::string refresh_token, time_t valid_until, AuthReadyCallback callback)
{
  g_debug("OAuth2::init(%s, %s, %ld)\n", access_token.c_str(), refresh_token.c_str(), valid_until);
  this->callback = callback;
  
  this->access_token = access_token;
  this->refresh_token = refresh_token;
  this->valid_until = valid_until;

  credentials_updated_signal(access_token, valid_until);
  client->set_request_filter(create_filter());
  
  if (valid_until + 60 < time(NULL))
    {
      g_debug("OAuth2::init: token expired");
      refresh_access_token();
    }
  else
    {
      report_async_result(Ok);
    }
}


void
OAuth2::get_tokens(std::string &access_token, std::string &refresh_token, time_t &valid_until)
{
  access_token = this->access_token;
  refresh_token = this->refresh_token;
  valid_until = this->valid_until;
}


void
OAuth2::request_authorization_grant()
{
  try
    {
      int port = 0;
      port = server->start(callback_uri_path, boost::bind(&OAuth2::on_authorization_grant_ready, this, _1));

      callback_uri = boost::str(boost::format("http://localhost:%1%%2%") % port % callback_uri_path);
      
      gchar *program = g_find_program_in_path("xdg-open");
      if (program == NULL)
        {
          g_debug("Cannot find xdg-open");
          throw std::exception();
        }

      RequestParams parameters;
      string command = string(program) + " " + create_login_url(callback_uri, parameters);

      gint exit_code = 0;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
        {
          g_debug("Failed to execute xdg-open: %s %s", command.c_str(), error->message);
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
      report_async_result(Failed);
    }
}


IHttpReply::Ptr
OAuth2::on_authorization_grant_ready(IHttpRequest::Ptr request)
{
  IHttpReply::Ptr reply = IHttpReply::create();

  try
    {
      g_debug("on_authorization_grant_ready status = %d, resp = %s", reply->status, reply->body.c_str());

      reply->content_type = "text/html";
      reply->body = settings.failure_html;
      
      if (request->method != "GET")
        {
          g_debug("Resource owner authorization only supports GET callback");
          throw std::exception();
        }          

      if (request->uri == "")
        {
          g_debug("Empty response for authorization grant");
          throw std::exception();
        }          

      RequestParams response_parameters;
      parse_query(request->uri, response_parameters);

      string error = response_parameters["error"];
      if (error != "")
        {
          reply->body = settings.failure_html;
          g_debug("Error set");
          throw std::exception();
        }
      
      string code = response_parameters["code"];

      if (code == "")
        {
          g_debug("Code must be set");
          throw std::exception();
        }
      
      reply->body = settings.success_html;
      request_access_token(code);
    }
  catch(...)
    {
      report_async_result(Failed);
    }

  g_debug("Returning %s", reply->body.c_str());
  return reply;
}


void
OAuth2::request_access_token(const string &code)
{
  try
    {
      string body = boost::str(boost::format("code=%1%&client_id=%2%&client_secret=%3%&redirect_uri=%4%&grant_type=authorization_code")
                               % code
                               % settings.client_id
                               % settings.client_secret
                               % Uri::escape(callback_uri)
                               );

      g_debug("request_access_token %s", body.c_str());
      
      IHttpRequest::Ptr request = IHttpRequest::create();
      request->uri = settings.token_endpoint;
      request->method = "POST";
      request->content_type = "application/x-www-form-urlencoded";
      request->body = body;

      client->execute(request, boost::bind(&OAuth2::on_access_token_ready, this, _1));
    }
  catch(...)
    {
      report_async_result(Failed);
    }
}


void
OAuth2::on_access_token_ready(IHttpReply::Ptr reply)
{
  AuthResult result = Failed;

  g_debug("on_access_token_ready status = %d, resp = %s", reply->status, reply->body.c_str());
  
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
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (ok && !root.isMember("error"))
        {
          access_token = root["access_token"].asString();
          refresh_token = root["refresh_token"].asString();
          valid_until = root["expires_in"].asInt() + time(NULL);
          
          g_debug("access_token : %sm valid %d", access_token.c_str(), root["expires_in"].asInt());

          credentials_updated_signal(access_token, valid_until);
          result = Ok;
        }
    }
  catch(...)
    {
    }

  report_async_result(result);
}


void
OAuth2::refresh_access_token()
{
  g_debug("refresh_access_token");
  access_token = "";

  try
    {
      RequestParams parameters;

      string body = boost::str(boost::format("client_id=%1%&client_secret=%2%&refresh_token=%3%&grant_type=refresh_token")
                               % settings.client_id
                               % settings.client_secret
                               % refresh_token
                               );
      g_debug("body %s", body.c_str());

      IHttpRequest::Ptr request = IHttpRequest::create();
      request->uri = settings.token_endpoint;
      request->method = "POST";
      request->content_type = "application/x-www-form-urlencoded";
      request->body = body;
          
      client->execute(request, boost::bind(&OAuth2::on_refresh_access_token_ready, this, _1));
    }
  catch(...)
    {
      report_async_result(Failed);
    }
}


void
OAuth2::on_refresh_access_token_ready(IHttpReply::Ptr reply)
{
  try
    {
      g_debug("status = %d, resp = %s", reply->status, reply->body.c_str());
      
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
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (ok && !root.isMember("error"))
        {
          access_token = root["access_token"].asString();
          valid_until = root["expires_in"].asInt() + time(NULL);
          
          g_debug("access_token : %sm valid %d", access_token.c_str(), root["expires_in"].asInt());
          credentials_updated_signal(access_token, valid_until);
        }

      report_async_result(Ok);
    }
  catch(...)
    {
      report_async_result(Failed);
    }
}


const string
OAuth2::parameters_to_string(const RequestParams &parameters) const
{
  string ret;

  for(RequestParams::const_iterator it = parameters.begin(); it != parameters.end(); it++)
    {
      string param = Uri::escape(it->first) + "=" + Uri::escape(it->second);
      if (ret != "")
        {
          ret += "&";
        }
      ret += param;
    }
  
  return ret;
}


const string
OAuth2::create_login_url(const string &redirect_uri, const RequestParams &parameters)
{
  string uri = boost::str(boost::format("%1%?response_type=code&client_id=%2%&redirect_uri=%3%&scope=%4%")
                          % settings.auth_endpoint
                          % Uri::escape(settings.client_id)
                          % Uri::escape(redirect_uri)
                          % Uri::escape(settings.scope));

  string extra_parameters = parameters_to_string(parameters);
  if (extra_parameters != "")
    {
      uri += "&" + extra_parameters;
    }

  return uri;
}


void
OAuth2::parse_query(const string &query, RequestParams &params) const
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


void
OAuth2::report_async_result(AuthResult result)
{
  // TODO: do somewhere else.
  // if (server)
  //   {
  //     server->stop();
  //   }
  if (!callback.empty())
    {
      callback(result);
    }
}


IHttpRequestFilter::Ptr
OAuth2::create_filter()
{
  return OAuth2Filter::create(shared_from_this());
}
