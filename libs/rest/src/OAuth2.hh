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

#ifndef OAUTH2_HH
#define OAUTH2_HH

#include <string>
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "rest/IOAuth2.hh"
#include "rest/IHttpServer.hh"
#include "rest/IHttpClient.hh"

class OAuth2 : public IOAuth2
{
public:
  typedef boost::shared_ptr<OAuth2> Ptr;

public:
  static Ptr create(const Settings &settings);

 	OAuth2(const Settings &settings);
  virtual ~OAuth2();
  
  void init(AuthReadyCallback callback);
  void init(std::string access_token, std::string refresh_token, time_t valid_until, AuthReadyCallback callback);
  void get_tokens(std::string &access_token, std::string &refresh_token, time_t &valid_until);
  void refresh_access_token();

  virtual CredentialsUpdatedSignal &signal_credentials_updated()
  {
    return credentials_updated_signal;
  }
  
private:
  typedef std::map<std::string, std::string> RequestParams;

  void request_authorization_grant();
  IHttpReply::Ptr on_authorization_grant_ready(IHttpRequest::Ptr request);

  void request_access_token(const std::string &code);
  void on_access_token_ready(IHttpReply::Ptr reply);

  void on_refresh_access_token_ready(IHttpReply::Ptr reply);

  void parse_query(const std::string &query, RequestParams &params) const;
  const std::string parameters_to_string(const RequestParams &parameters) const;
  const std::string create_login_url(const std::string &redirect_uri, const RequestParams &parameters);

  void report_async_result(AuthResult result);

  IHttpRequestFilter::Ptr create_filter();

private:  
  IHttpClient::Ptr client;
  IHttpServer::Ptr server;
  Settings settings;

  std::string callback_uri;
  std::string access_token;
  std::string refresh_token;
  time_t valid_until;
  
  AuthReadyCallback callback;
  CredentialsUpdatedSignal credentials_updated_signal;
  
  std::string callback_uri_path;
};

#endif
