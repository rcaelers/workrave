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

#ifndef OAUTH1FILTER_HH
#define OAUTH1FILTER_HH

#include <string>
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "HttpDecorator.hh"

class OAuth1Filter : public boost::enable_shared_from_this<OAuth1Filter>
{
public:
  typedef boost::shared_ptr<OAuth1Filter> Ptr;

  typedef std::map<std::string, std::string> RequestParams;

public:
 	OAuth1Filter();

  static Ptr create();
  
  void set_consumer(const std::string &consumer_key, const std::string &consumer_secret);
  void set_token(const std::string &token_key, const std::string &token_secret);
  void set_custom_headers(const RequestParams &custom_headers = RequestParams());

  bool has_credentials() const;
  void get_credentials(std::string &consumer_key,
                       std::string &consumer_secret,
                       std::string &token_key,
                       std::string &token_secret);


  IHttpExecute::Ptr create_decorator(IHttpExecute::Ptr executor);
  void filter(HttpRequest::Ptr request);
  
private:
  enum ParameterMode { ParameterModeHeader, ParameterModeRequest, ParameterModeSignatureBase };

  class Decorator : public HttpDecorator
  {
  public:
    typedef boost::shared_ptr<Decorator> Ptr;

  public:
    static Ptr create(OAuth1Filter::Ptr filter, IHttpExecute::Ptr executor)
    {
      return Ptr(new Decorator(filter, executor));
    }

    Decorator(OAuth1Filter::Ptr filter, IHttpExecute::Ptr executor) : HttpDecorator(executor)
    {
      filter->filter(executor->get_request());
    }
  };
  
private:
  const std::string get_timestamp() const;
  const std::string get_nonce() const;
  const std::string normalize_uri(const std::string &uri, RequestParams &parameters) const;
  const std::string parameters_to_string(const RequestParams &parameters, ParameterMode mode) const;
  const std::string encrypt(const std::string &input, const std::string &key) const;
  const std::string create_oauth_header(const std::string &http_method, const std::string &uri, RequestParams &parameters) const;

private:  
  RequestParams custom_headers;
 	std::string consumer_key;
 	std::string consumer_secret;
 	std::string token_key;
 	std::string token_secret;
 	std::string signature_method;
  std::string oauth_version;
};

#endif
