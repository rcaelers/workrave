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

#ifndef UNIXKEYCHAIN_HH
#define UNIXKEYCHAIN_HH

#include <string>

#include <libsecret/secret.h>

#include "rest/IKeyChain.hh"

#ifdef __cplusplus
extern "C" {
#endif

  const SecretSchema * workrave_get_schema(void) G_GNUC_CONST;

#define WORKRAVE_SCHEMA  workrave_get_schema()

#ifdef __cplusplus
}
#endif

namespace workrave
{
  namespace rest
  {
    class UnixKeyChain : public IKeyChain
    {
    public:
      UnixKeyChain(const std::string &client_id, const std::string &server);
      virtual ~UnixKeyChain();
      
      virtual void store(const std::string &username, const std::string &password, StoreResult callback);
      virtual void retrieve(const std::string &username, RetrieveResult callback);

      static void on_password_stored(GObject *source, GAsyncResult *result, gpointer data);
      static void on_password_rerieved(GObject *source, GAsyncResult *result, gpointer data);
      
    private:
      struct StoreCallbackData
      {
        std::string username;
        UnixKeyChain *self;
        StoreResult callback;
      };

      struct RetrieveCallbackData
      {
        std::string username;
        UnixKeyChain *self;
        RetrieveResult callback;
      };
      
      const std::string &client_id;
    };
  }
}
#endif
