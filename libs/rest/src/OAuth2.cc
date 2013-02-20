// Copyright (C) 2010 - 2013 by Rob Caelers <robc@krandor.nl>
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

#ifdef PLATFORM_OS_OSX
#import <AppKit/NSWorkspace.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#endif

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
}


void
OAuth2::init(AuthResultCallback callback, ShowUserfeedbackCallback feedback)
{
  if (state != Idle)
    {
      report_result(AuthErrorCode::Busy, "Cannot initialize. OAuth operation in progress");
      return;
    }
  
  session->add_request_filter(create_filter());

  this->callback = callback;
  this->feedback = feedback;
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
  g_debug("OAuth2::request_authorization_grant");
  try
    {
      state = AuthorizationGrantRequest;
      
      int port = 0;
      server = session->listen(callback_uri_path, boost::bind(&OAuth2::on_authorization_grant_ready, this, _1));
      port = server->get_port();
      callback_uri = boost::str(boost::format("http://localhost:%1%%2%") % port % callback_uri_path);

      RequestParams parameters;
      string login_uri = create_login_url(callback_uri, parameters);

      // TODO: move to utils.
      
#if defined(PLATFORM_OS_UNIX)     
      gchar *program = g_find_program_in_path("xdg-open");
      if (program == NULL)
        {
          throw AuthError(AuthErrorCode::System, "Cannot find xdg-open");
        }

      string command = string(program) + " " + login_uri;

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
#elif defined(PLATFORM_OS_OSX)
      g_debug("Redirecting to %s", login_uri.c_str());
      NSString* uri = [NSString stringWithCString:login_uri.c_str() encoding: NSUTF8StringEncoding];
      [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: uri]];
#else
#error Not ported
#endif
      
    }
  catch(HttpError &e)
    {
      report_result(AuthErrorCode::System, "Failed to open login website in browser");
    }
  catch(AuthError &e)
    {
      report_result(e);
    }
}


IHttpReply::Ptr
OAuth2::on_authorization_grant_ready(IHttpRequest::Ptr request)
{
  string error = "";
  string error_description = "";

  g_debug("OAuth2::on_authorization_grant_ready: uri = %s, resp = %s", request->uri.c_str(), request->body.c_str());
  
  try
    {
      server->stop();
      
      if (request->method != "GET")
        {
          error = "server_error";
          throw AuthError(AuthErrorCode::Protocol, "Resource owner authorization only supports GET callback");
        }          

      if (request->uri == "")
        {
          error = "server_error";
          throw AuthError(AuthErrorCode::Protocol, "Empty response for authorization grant");
        }          

      RequestParams response_parameters;
      parse_query(request->uri, response_parameters);

      string oauth_error = response_parameters["error"];
      if (oauth_error != "")
        { 
          error = oauth_error;
          error_description = response_parameters["error_description"];
          throw AuthError(AuthErrorCode::Server, oauth_error);
        }
      
      string code = response_parameters["code"];
      if (code == "")
        {
          error = "invalid_request";
          throw AuthError(AuthErrorCode::Protocol, "No code received");
        }
      
      request_access_token(code);
    }
  catch(AuthError &e)
    {
      report_result(e);
      if (error == "")
        {
          error = "server_error";
        }
    }

  IHttpReply::Ptr reply = IHttpReply::create();
  feedback(error, error_description, reply);
  return reply;
}


void
OAuth2::request_access_token(const string &code)
{
  g_debug("OAuth2::request_access_token");
  try
    {
      state = AccessTokenRequest;

      string body = boost::str(boost::format("code=%1%&client_id=%2%&client_secret=%3%&redirect_uri=%4%&grant_type=authorization_code")
                               % code
                               % settings.client_id
                               % settings.client_secret
                               % Uri::escape(callback_uri)
                               );

      IHttpRequest::Ptr request = IHttpRequest::create();
      request->uri = settings.token_endpoint;
      request->method = "POST";
      request->content_type = "application/x-www-form-urlencoded";
      request->body = body;

      session->send(request, boost::bind(&OAuth2::on_access_token_ready, this, _1));
    }
  catch(AuthError &e)
    {
      report_result(e);
    }
}


void
OAuth2::on_access_token_ready(IHttpReply::Ptr reply)
{
  g_debug("OAuth2::on_access_token_ready: status = %d, resp = %s", reply->status, reply->body.c_str());
  
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
          
      g_debug("OAuth2::on_access_token_ready: access_token=%sm valid=%d", access_token.c_str(), root["expires_in"].asInt());
      struct tm * timeinfo = localtime(&valid_until);
      g_debug("OAuth2::on_access_token_ready: valid until:  %s", asctime(timeinfo));

      credentials_updated_signal(access_token, valid_until);
    }
  catch(AuthError &e)
    {
      report_result(e);
      return;
    }

  report_result(AuthErrorCode::Success);
}


void
OAuth2::refresh_access_token()
{
  g_debug("OAuth2::refresh_access_token");
  access_token = "";
  credentials_updated_signal(access_token, 0);

  if (state == RefreshAccessTokenRequest)
    {
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

      IHttpRequest::Ptr request = IHttpRequest::create();
      request->uri = settings.token_endpoint;
      request->method = "POST";
      request->content_type = "application/x-www-form-urlencoded";
      request->body = body;
          
      session->send(request, boost::bind(&OAuth2::on_refresh_access_token_ready, this, _1));
    }
  catch(AuthError &e)
    {
      report_result(e);
    }
}


void
OAuth2::on_refresh_access_token_ready(IHttpReply::Ptr reply)
{
  g_debug("OAuth2::on_refresh_access_token_ready: status = %d, resp = %s", reply->status, reply->body.c_str());
  try
    {
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

      if (root.isMember("refresh_token"))
        {
          refresh_token = root["refresh_token"].asString();
          g_debug("OAuth2::on_refresh_access_token_ready: refresh_token : %s", refresh_token.c_str());
        }

      struct tm * timeinfo = localtime(&valid_until);
      g_debug("OAuth2::on_refresh_access_token_ready: Valid until:  %s", asctime(timeinfo));
       
      g_debug("OAuth2::on_refresh_access_token_ready: access_token=%s, valid %d", access_token.c_str(), root["expires_in"].asInt());
      credentials_updated_signal(access_token, valid_until);
    }
  catch(AuthError &e)
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
OAuth2::report_result(const AuthError &error)
{
  report_result(error.code(), error.what());
}

void
OAuth2::report_result(AuthErrorCode code, const string &details)
{
  g_debug("OAuth2::report_result: %d %s", code, details.c_str());
  
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
