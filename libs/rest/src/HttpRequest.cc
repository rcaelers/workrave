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

#include <boost/bind.hpp>

#include "HttpRequest.hh"

using namespace workrave::rest;

IHttpRequest::Ptr
IHttpRequest::create()
{
  return Ptr(new HttpRequest());
}

HttpRequest::HttpRequest() 
{
}

void
HttpRequest::set_filters(FilterList filters)
{
  this->filters = filters;
}


void
HttpRequest::apply_filters(Ready ready)
{
  this->ready = ready;
  
  filter_iter = filters.begin();
  step();
}


void
HttpRequest::step()
{
  if (filter_iter != filters.end())
    {
      workrave::rest::IHttpRequestFilter::Ptr filter = *filter_iter;
      filter_iter++;
        
      filter->filter(shared_from_this(), boost::bind(&HttpRequest::step, this));
    }
  else
    {
      ready();
    }
}
