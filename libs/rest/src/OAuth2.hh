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
#include "rest/IHttpBackend.hh"
#include "OAuth2Filter.hh"

class OAuth2 : public IOAuth2, public IHttpDecoratorFactory
{
public:
  typedef boost::shared_ptr<OAuth2> Ptr;

public:
  static Ptr create(IHttpBackend::Ptr backend, const Settings &settings);

 	OAuth2(IHttpBackend::Ptr backend, const Settings &settings);
  virtual ~OAuth2();
  
  void init(AuthReadyCallback callback);
  void init(std::string access_token, std::string refresh_token, time_t valid_until, AuthReadyCallback callback);
  void get_tokens(std::string &access_token, std::string &refresh_token, time_t &valid_until);
  
private:
  typedef std::map<std::string, std::string> RequestParams;

  void request_authorization_grant();
  HttpReply::Ptr on_authorization_grant_ready(HttpRequest::Ptr request);

  void request_access_token(const std::string &code);
  void on_access_token_ready(HttpReply::Ptr reply);

  void request_refresh_token(bool sync = false);
  void on_refresh_token_ready(HttpReply::Ptr reply);

  void parse_query(const std::string &query, RequestParams &params) const;
  const std::string parameters_to_string(const RequestParams &parameters) const;
  const std::string create_login_url(const std::string &redirect_uri, const RequestParams &parameters);

  void report_async_result(AuthResult result);

  IHttpExecute::Ptr create_decorator(IHttpExecute::Ptr execute);
  void on_refresh_request(OAuth2Filter::Ptr filter);
  
private:  
  IHttpBackend::Ptr backend;
  IHttpServer::Ptr server;
  Settings settings;

  std::string callback_uri;
  std::string access_token;
  std::string refresh_token;
  time_t valid_until;
  
  AuthReadyCallback callback;
  std::list<OAuth2Filter::Ptr> waiting_for_refresh;
  
  std::string callback_uri_path;
};

#endif
