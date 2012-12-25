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

#ifndef IOAUTH1_HH
#define IOAUTH1_HH

#include <string>
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "rest/IHttpClient.hh"

class IOAuth1 : public boost::enable_shared_from_this<IOAuth1>
{
public:
  typedef boost::shared_ptr<IOAuth1> Ptr;

  typedef boost::function<void () > SuccessCallback;
  typedef boost::function<void () > FailedCallback;

  struct Settings
  {
    Settings()
    {
    }
    
    Settings(const std::string &temporary_request_uri,
             const std::string &authorize_uri,
             const std::string &token_request_uri,
             const std::string &success_html,
             const std::string &failure_html)
      : temporary_request_uri(temporary_request_uri),
        authorize_uri(authorize_uri),
        token_request_uri(token_request_uri),
        success_html(success_html),
        failure_html(failure_html)
    {
    }
    
    std::string temporary_request_uri;
    std::string authorize_uri;
    std::string token_request_uri;
    std::string success_html;
    std::string failure_html;
  };
   
public:
  static Ptr create(const Settings &settings);

  virtual void init(const std::string &consumer_key,
                    const std::string &consumer_secret,
                    SuccessCallback success_cb,
                    FailedCallback failure_cb) = 0;

  virtual IHttpRequestFilter::Ptr create_filter() = 0;
};

#endif
