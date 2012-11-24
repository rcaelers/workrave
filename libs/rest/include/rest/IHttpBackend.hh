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

#ifndef IHTTPBACKEND_HH
#define IHTTPBACKEND_HH

#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "HttpRequest.hh"
#include "HttpReply.hh"
#include "rest/IHttpExecute.hh"
#include "rest/IHttpServer.hh"
#include "IHttpDecoratorFactory.hh"

class IHttpBackend
{
public:
  typedef boost::shared_ptr<IHttpBackend> Ptr;

public:
  static Ptr create();

  virtual ~IHttpBackend() {}

  virtual bool init(const std::string &user_agent) = 0;
  virtual void set_decorator_factory(IHttpDecoratorFactory::Ptr factory) = 0;

  virtual HttpReply::Ptr request(HttpRequest::Ptr request) = 0;
  virtual HttpReply::Ptr request(HttpRequest::Ptr request, const IHttpExecute::HttpExecuteReady callback) = 0;
  virtual IHttpServer::Ptr listen(const std::string &path, int &port, IHttpServer::HttpServerCallback callback) = 0;
};

#endif
