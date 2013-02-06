// Copyright (C) 2013 by Rob Caelers <robc@krandor.nl>
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

#include <glib.h>

#include "OSXKeyChain.hh"

#include <Security/Security.h>

using namespace std;
using namespace workrave::rest;

IKeyChain::Ptr
IKeyChain::create(const std::string &server)
{
  return Ptr(new OSXKeyChain(server));
}


OSXKeyChain::OSXKeyChain(const std::string &server)
  : server(server)
{
}

OSXKeyChain::~OSXKeyChain()
{
}

void
OSXKeyChain::retrieve(const std::string &username, RetrieveResult callback)
{
	void *buf;
	UInt32 len;
	SecKeychainItemRef item;

  string path = "";
  
  OSStatus r = SecKeychainFindInternetPassword(NULL, 
                                               server.size(), server.c_str(),
                                               0, NULL,
                                               username.size(), username.c_str(),
                                               path.size(), path.c_str(),
                                               443,
                                               kSecProtocolTypeHTTPS,
                                               kSecAuthenticationTypeDefault,               
                                               &len, &buf, &item);

  string secret;
  if (r == errSecSuccess)
    {
      secret = string((char *)buf, len);
      SecKeychainItemFreeContent(NULL, buf);
    }

  g_debug("len = %d, b = %s", len, buf);
  
  callback(r == errSecSuccess, username, secret);
}

void
OSXKeyChain::store(const std::string &username, const std::string &secret, StoreResult callback)
{
  string path = "";
  
  OSStatus r = SecKeychainAddInternetPassword(NULL, 
                                              server.size(), server.c_str(),
                                              0, NULL,
                                              username.size(), username.c_str(),
                                              path.size(), path.c_str(),
                                              443,
                                              kSecProtocolTypeHTTPS,
                                              kSecAuthenticationTypeDefault,
                                              secret.size(), secret.c_str(), NULL);

  callback(r == errSecSuccess);
}
