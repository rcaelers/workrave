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

#include "UnixKeyChain.hh"

using namespace std;
using namespace workrave::rest;

const SecretSchema *
workrave_get_schema (void)
{
    static const SecretSchema the_schema = {
        "org.workrave.Token", SECRET_SCHEMA_NONE,
        {
          {  "client_id", SECRET_SCHEMA_ATTRIBUTE_STRING },
          {  "user_name", SECRET_SCHEMA_ATTRIBUTE_STRING },
          {  "NULL", (SecretSchemaAttributeType)0 },
        }
    };
    return &the_schema;
}


IKeyChain::Ptr
IKeyChain::create(const std::string &client_id, const std::string &server)
{
  return Ptr(new UnixKeyChain(client_id, server));
}


UnixKeyChain::UnixKeyChain(const std::string &client_id, const std::string &server)
  : client_id(client_id), server(server)
{
}

UnixKeyChain::~UnixKeyChain()
{
}

void
UnixKeyChain::retrieve(const std::string &username, RetrieveResult callback)
{
  RetrieveCallbackData *data = new RetrieveCallbackData;
  data->self = this;
  data->callback = callback;
  data->username = username;

  secret_password_lookup(WORKRAVE_SCHEMA, NULL, on_password_rerieved, data,
                         "client_id", client_id.c_str(),
                         "user_name", username.c_str(),
                         NULL);
}

void
UnixKeyChain::store(const std::string &username, const std::string &secret, StoreResult callback)
{
  StoreCallbackData *data = new StoreCallbackData;
  data->self = this;
  data->callback = callback;
  data->username = username;

  secret_password_store(WORKRAVE_SCHEMA, SECRET_COLLECTION_DEFAULT,
                        server.c_str(),
                        secret.c_str(), NULL, on_password_stored, data,
                        "client_id", client_id.c_str(),
                        "user_name", username.c_str(),
                        NULL);
}

void
UnixKeyChain::on_password_rerieved(GObject *source, GAsyncResult *result, gpointer user_data)
{
  RetrieveCallbackData *data = (RetrieveCallbackData *)user_data;
  GError *error = NULL;

  gchar *password = secret_password_lookup_finish(result, &error);
  
  if (error != NULL)
    {
      g_debug("secret_password_lookup: %s", error->message);
      g_error_free(error);
      data->callback(false, data->username, "");
    }

  else if (password != NULL)
    {
      g_debug("secret_password_lookup: passwd=%s", password);
      data->callback(true, data->username, password);
      secret_password_free(password);
    }
  else
    {
      data->callback(false, data->username, "");
    }

  delete data;
}


void
UnixKeyChain::on_password_stored(GObject *source, GAsyncResult *result, gpointer user_data)
{
  StoreCallbackData *data = (StoreCallbackData *)user_data;
  GError *error = NULL;

  g_debug("secret_password_store");

  secret_password_store_finish(result, &error);
  if (error != NULL)
    {
      g_debug("secret_password_store: %s", error->message);
      data->callback(false);
      g_error_free(error);
    }
  else
    {
      data->callback(true);
    }
  delete data;
}
