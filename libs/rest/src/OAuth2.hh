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

#ifndef OAUTH2_HH
#define OAUTH2_HH

#include <string>
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include "rest/IOAuth2.hh"
#include "rest/IHttpSession.hh"
#include "rest/IHttpServer.hh"

class OAuth2 : public workrave::rest::IOAuth2
{
public:
  typedef boost::shared_ptr<OAuth2> Ptr;
  typedef boost::signals2::signal<void(const std::string&, time_t valid_until)> CredentialsUpdatedSignal;

public:
  static Ptr create(const workrave::rest::OAuth2Settings &settings);

 	OAuth2(const workrave::rest::OAuth2Settings &settings);
  virtual ~OAuth2();
  
  void init(AuthResultCallback callback, ShowUserfeedbackCallback feedback);
  void init(std::string access_token, std::string refresh_token, time_t valid_until, AuthResultCallback callback);
  void get_tokens(std::string &access_token, std::string &refresh_token, time_t &valid_until);
  void refresh_access_token();

  virtual CredentialsUpdatedSignal &signal_credentials_updated()
  {
    return credentials_updated_signal;
  }
  
private:
  typedef std::map<std::string, std::string> RequestParams;
  enum State { Idle,
               Error,
               AuthorizationGrantRequest,
               AccessTokenRequest,
               RefreshAccessTokenRequest
  };
               
  void request_authorization_grant();
  workrave::rest::IHttpReply::Ptr on_authorization_grant_ready(workrave::rest::IHttpRequest::Ptr request);

  void request_access_token(const std::string &code);
  void on_access_token_ready(workrave::rest::IHttpReply::Ptr reply);

  void on_refresh_access_token_ready(workrave::rest::IHttpReply::Ptr reply);

  void parse_query(const std::string &query, RequestParams &params) const;
  const std::string parameters_to_string(const RequestParams &parameters) const;
  const std::string create_login_url(const std::string &redirect_uri, const RequestParams &parameters);

  void report_result(workrave::rest::AuthErrorCode code, const std::string &details = "");
  void report_result(const workrave::rest::AuthError &e);

  workrave::rest::IHttpRequestFilter::Ptr create_filter();

private:
  State state;
  workrave::rest::IHttpSession::Ptr session;
  workrave::rest::IHttpServer::Ptr server;
  workrave::rest::OAuth2Settings settings;

  std::string callback_uri;
  std::string access_token;
  std::string refresh_token;
  time_t valid_until;
  
  ShowUserfeedbackCallback feedback;
  AuthResultCallback callback;
  CredentialsUpdatedSignal credentials_updated_signal;
  
  std::string callback_uri_path;
};

#endif
