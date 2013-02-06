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

#include <Security/Security.h>

using namespace std;
using namespace workrave::rest;


const SecretSchema *
workrave_get_schema (void)
{
    static const SecretSchema the_schema = {
        "org.workrave.Token", SECRET_SCHEMA_NONE,
        {
          {  "client_id", SECRET_SCHEMA_ATTRIBUTE_STRING },
          {  "NULL", (SecretSchemaAttributeType)0 },
        }
    };
    return &the_schema;
}


IKeyChain::Ptr
IKeyChain::create(const std::string &server)
{
  return Ptr(new UnixKeyChain(server));
}

UnixKeyChain::UnixKeyChain(const std::string &server)
  : server(server)
{
}

UnixKeyChain::~UnixKeyChain()
{
}

void
UnixKeyChain::retrieve(const std::string &username, RetrieveResult callback)
{
  secret_password_lookup(WORKRAVE_SCHEMA, NULL, on_password_lookup, this,
                         "client_id", oauth_settings.client_id.c_str(),
                         NULL);
}

void
UnixKeyChain::store(const std::string &username, const std::string &secret, StoreResult callback)
{
      secret_password_store(WORKRAVE_SCHEMA, SECRET_COLLECTION_DEFAULT,
                            "Workrave",
                            password.c_str(), NULL, on_password_stored, this,
                            "client_id", oauth_settings.client_id.c_str(),
                            NULL);
}

#if PLATFORM_OS_UNIX
void
WorkraveAuth::on_store_result(bool ok, const std::string &username, const std::string &secret)
{
  WorkraveAuth *self = (WorkraveAuth *)data;
  GError *error = NULL;

  bool success = false;

  gchar *password = secret_password_lookup_finish(result, &error);
  
  if (error != NULL)
    {
      g_debug("secret_password_lookup: %s", error->message);
      g_error_free(error);
    }

  else if (password != NULL)
    {
      g_debug("secret_password_lookup: passwd=%s", password);

      vector<string> elements;
      boost::split(elements, password, boost::is_any_of(":"));
  
      if (elements.size() == 3 &&
          elements[0].length() > 0 &&
          elements[1].length() > 0)
        {
          time_t valid_until = 0;

          try
            {
              valid_until = boost::lexical_cast<int>(elements[2]);
            }
          catch(boost::bad_lexical_cast &) {}
          g_debug("secret_password_lookup: valid=%d", (int)valid_until);
          
          self->workflow->init(elements[0], elements[1], valid_until, boost::bind(&WorkraveAuth::on_auth_result, self, _1, _2));
          success = true;
        }
      secret_password_free(password);
    }

  if (!success)
    {
      g_debug("secret_password_lookup: obtain access");
      self->workflow->init(boost::bind(&WorkraveAuth::on_auth_result, self, _1, _2),
                           boost::bind(&WorkraveAuth::on_auth_feedback, self, _1, _2, _3));
    }
}
#endif


void
WorkraveAuth::on_password_stored(GObject *source, GAsyncResult *result, gpointer data)
{
  WorkraveAuth *self = (WorkraveAuth *)data;
  GError *error = NULL;

  g_debug("secret_password_store");

#if PLATFORM_OS_UNIX
  secret_password_store_finish(result, &error);
  if (error != NULL)
    {
      g_debug("secret_password_store: %s", error->message);

      self->callback(AuthErrorCode::Failed, "Failed to store token");

      g_error_free(error);
    }
#endif
  
  self->callback(AuthErrorCode::Success, "");
}
