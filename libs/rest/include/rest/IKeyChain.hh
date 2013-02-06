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

#ifndef IKEYCHAIN_HH
#define IKEYCHAIN_HH

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <string>

namespace workrave
{
  namespace rest
  {
    class IKeyChain
    {
    public:
      typedef boost::shared_ptr<IKeyChain> Ptr;

      typedef boost::function<void (bool) > StoreResult;
      typedef boost::function<void (bool, const std::string &, const std::string &) > RetrieveResult;

      static Ptr create(const std::string &path);
      
      virtual void store(const std::string &username, const std::string &secret, StoreResult callback) = 0;
      virtual void retrieve(const std::string &username, RetrieveResult callback) = 0;
    };
  }
}
#endif
