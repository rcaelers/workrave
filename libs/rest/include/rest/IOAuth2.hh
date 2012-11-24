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

#ifndef IOAUTH2_HH
#define IOAUTH2_HH

#include <string>
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "IHttpBackend.hh"

class IOAuth2 : public boost::enable_shared_from_this<IOAuth2>
{
public:
  enum AuthResult { Ok, Failed };
    
  typedef boost::shared_ptr<IOAuth2> Ptr;
  typedef boost::function<void (AuthResult) > AuthReadyCallback;

  struct Settings
  {
    Settings()
    {
    }
    
    Settings(const std::string &auth_endpoint,
             const std::string &token_endpoint,
             const std::string &client_id,
             const std::string &client_secret,
             const std::string &scope,
             const std::string &success_html,
             const std::string &failure_html)
      : auth_endpoint(auth_endpoint),
        token_endpoint(token_endpoint),
        client_id(client_id),
        client_secret(client_secret),
        scope(scope),
        success_html(success_html),
        failure_html(failure_html)
    {
    }
    
    std::string auth_endpoint;
    std::string token_endpoint;
    std::string client_id;
    std::string client_secret;
    std::string scope;
    std::string success_html;
    std::string failure_html;
  };
   
public:
  static Ptr create(IHttpBackend::Ptr backend, const Settings &settings);

  virtual void init(AuthReadyCallback callback) = 0;
  virtual void init(std::string access_token, std::string refresh_token, time_t valid_until, AuthReadyCallback callback) = 0;
  virtual void get_tokens(std::string &access_token, std::string &refresh_token, time_t &valid_until) = 0;
 
};

#endif
