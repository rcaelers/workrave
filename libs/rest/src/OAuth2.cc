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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "OAuth2.hh"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <glib.h>
#include "json/json.h"

#include "rest/IHttpSession.hh"

#include "Uri.hh"
#include "OAuth2Filter.hh"

using namespace std;
using namespace workrave::rest;

IOAuth2::Ptr
IOAuth2::create(const workrave::rest::OAuth2Settings &settings)
{
  return OAuth2::create(settings);
}


OAuth2::Ptr
OAuth2::create(const workrave::rest::OAuth2Settings &settings)
{
  return Ptr(new OAuth2(settings));
}


OAuth2::OAuth2(const workrave::rest::OAuth2Settings &settings)
  : state(Idle),
    settings(settings),
    valid_until(0)
{
  callback_uri_path = "/oauth2";
  session = IHttpSession::create("");
}


OAuth2::~OAuth2()
{
  if (server)
    {
      server->stop();
    }
}


void
OAuth2::init(AuthResultCallback callback)
{
  if (state != Idle)
    {
      report_result(AuthErrorCode::Busy, "Cannot initialize. OAuth operation in progress");
      return;
    }
  
  session->add_request_filter(create_filter());

  this->callback = callback;
  request_authorization_grant();
}


void
OAuth2::init(std::string access_token, std::string refresh_token, time_t valid_until, AuthResultCallback callback)
{
  g_debug("OAuth2::init(%s, %s, %ld)\n", access_token.c_str(), refresh_token.c_str(), valid_until);

  if (state != Idle)
    {
      report_result(AuthErrorCode::Busy, "Cannot initialize. OAuth operation in progress");
      return;
    }

  this->callback = callback;
  this->access_token = access_token;
  this->refresh_token = refresh_token;
  this->valid_until = valid_until;

  credentials_updated_signal(access_token, valid_until);
  session->add_request_filter(create_filter());
  
  if (valid_until + 60 < time(NULL))
    {
      g_debug("OAuth2::init: token expired");
      refresh_access_token();
    }
  else
    {
      report_result(AuthErrorCode::Success);
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
      state = AuthorizationGrantRequest;
      
      int port = 0;
      server = session->listen(callback_uri_path, boost::bind(&OAuth2::on_authorization_grant_ready, this, _1));
      port = server->get_port();
      callback_uri = boost::str(boost::format("http://localhost:%1%%2%") % port % callback_uri_path);
      
      gchar *program = g_find_program_in_path("xdg-open");
      if (program == NULL)
        {
          throw AuthError(AuthErrorCode::System, "Cannot find xdg-open");
        }

      RequestParams parameters;
      string command = string(program) + " " + create_login_url(callback_uri, parameters);

      gint exit_code = 0;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
        {
          throw AuthError(AuthErrorCode::System, boost::str(boost::format("Failed to execute '%1%' (%2%)") % command % error->message));
        }

      if (WEXITSTATUS(exit_code) != 0)
        {
          throw AuthError(AuthErrorCode::System, boost::str(boost::format("xdg-open returned an error exit-code %1%") % WEXITSTATUS(exit_code)));
        }
    }
  catch(std::exception &e)
    {
      report_result(e);
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
          throw AuthError(AuthErrorCode::Protocol, "Resource owner authorization only supports GET callback");
        }          

      if (request->uri == "")
        {
          throw AuthError(AuthErrorCode::Protocol, "Empty response for authorization grant");
        }          

      RequestParams response_parameters;
      parse_query(request->uri, response_parameters);

      string error = response_parameters["error"];
      if (error != "")
        {
          reply->body = settings.failure_html;
          throw AuthError(AuthErrorCode::Server, error);
        }
      
      string code = response_parameters["code"];

      if (code == "")
        {
          throw AuthError(AuthErrorCode::Protocol, "No code received");
        }
      
      reply->body = settings.success_html;
      request_access_token(code);
    }
  catch(std::exception &e)
    {
      report_result(e);
    }

  g_debug("Returning %s", reply->body.c_str());
  return reply;
}


void
OAuth2::request_access_token(const string &code)
{
  try
    {
      state = AccessTokenRequest;

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

      session->send(request, boost::bind(&OAuth2::on_access_token_ready, this, _1));
    }
  catch(std::exception &e)
    {
      report_result(e);
    }
}


void
OAuth2::on_access_token_ready(IHttpReply::Ptr reply)
{
  g_debug("on_access_token_ready status = %d, resp = %s", reply->status, reply->body.c_str());
  
  try
    {
      if (reply->status != 200)
        {
          throw AuthError(AuthErrorCode::Protocol, boost::str(boost::format("Access token request returned HTTP error %1%") % reply->status));
        }

      if (reply->body == "")
        {
          throw AuthError(AuthErrorCode::Protocol, "Access token request returned empty body");
        }          
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (!ok)
        {
          throw AuthError(AuthErrorCode::Protocol, boost::str(boost::format("Unable to parse JSON data from access token request: %1%") % reply->body));
        }
      
      if (root.isMember("error"))
        {
          throw AuthError(AuthErrorCode::Server, root["error"].asString());
        }
      
      access_token = root["access_token"].asString();
      refresh_token = root["refresh_token"].asString();
      valid_until = root["expires_in"].asInt() + time(NULL);
          
      g_debug("access_token : %sm valid %d", access_token.c_str(), root["expires_in"].asInt());

      credentials_updated_signal(access_token, valid_until);
    }
  catch(std::exception &e)
    {
      report_result(e);
      return;
    }

  report_result(AuthErrorCode::Success);
}


void
OAuth2::refresh_access_token()
{
  g_debug("refresh_access_token");
  access_token = "";
  credentials_updated_signal(access_token, 0);

  if (state == RefreshAccessTokenRequest)
    {
      g_debug("Already refreshing");
      return;
    }
  else if (state != Idle)
    {
      report_result(AuthErrorCode::Busy, "Cannot refresh token. OAuth operation in progress");
      return;
    }
  
  try
    {
      state = RefreshAccessTokenRequest;

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
          
      session->send(request, boost::bind(&OAuth2::on_refresh_access_token_ready, this, _1));
    }
  catch(std::exception &e)
    {
      report_result(e);
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
          throw AuthError(AuthErrorCode::Protocol, boost::str(boost::format("Refresh access token request returned HTTP error %1%") % reply->status));
        }

      if (reply->body == "")
        {
          throw AuthError(AuthErrorCode::Protocol, "Refresh access token request returned empty body");;
        }          
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (!ok)
        {
          throw AuthError(AuthErrorCode::Protocol, boost::str(boost::format("Unable to parse JSON data from refresh access token request: %1%") % reply->body));
        }
      
      if (root.isMember("error"))
        {
          throw AuthError(AuthErrorCode::Server, root["error"].asString());
        }
      
      access_token = root["access_token"].asString();
      valid_until = root["expires_in"].asInt() + time(NULL);
          
      g_debug("access_token : %sm valid %d", access_token.c_str(), root["expires_in"].asInt());
      credentials_updated_signal(access_token, valid_until);
    }
  catch(std::exception &e)
    {
      report_result(e);
      return;
    }

  report_result(AuthErrorCode::Success);
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
OAuth2::report_result(const std::exception &exp)
{
  try
    {
      const AuthError &error = dynamic_cast<const AuthError&>(exp);
      report_result(error.code(), error.what());
    }
  catch(std::bad_cast)
    {
      report_result(AuthErrorCode::Failed, exp.what());
    }
}

void
OAuth2::report_result(AuthErrorCode code, const string &details)
{
  if (code == AuthErrorCode::Success)
    {
      state = Idle;
    }
  else if (code != AuthErrorCode::Busy)
    {
      state = Error;
    }
  
  if (!callback.empty())
    {
      callback(code, details);
    }
}


IHttpRequestFilter::Ptr
OAuth2::create_filter()
{
  return OAuth2Filter::create(shared_from_this());
}
