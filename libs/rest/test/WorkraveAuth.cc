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

#include "WorkraveAuth.hh"

#include <glib.h>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "rest/IOAuth2.hh"
#include "rest/IHttpSession.hh"

using namespace std;
using namespace workrave::rest;

const SecretSchema *
workrave_get_schema (void)
{
    static const SecretSchema the_schema = {
        "org.krandor.Password", SECRET_SCHEMA_NONE,
        {
            {  "number", SECRET_SCHEMA_ATTRIBUTE_INTEGER },
            {  "string", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  "even", SECRET_SCHEMA_ATTRIBUTE_BOOLEAN },
            {  "NULL", (SecretSchemaAttributeType)0 },
        }
    };
    return &the_schema;
}


WorkraveAuth::Ptr
WorkraveAuth::create()
{
  return Ptr(new WorkraveAuth());
}


WorkraveAuth::WorkraveAuth()
{
  // TODO: Make HTML response customisable
  oauth_settings.success_html = 
    "<html>"
    "<head><title>Authorization</title></head>"
    "<body><h1>Authorization OK</h1></body>"
    "</html>";
  
  oauth_settings.failure_html =
    "<html>"
    "<head><title>Authorization</title></head>"
    "<body><h1>Authorization FAILED</h1></body>"
    "</html>";

  // TODO: 
  oauth_settings.auth_endpoint = "http://localhost:8000/oauth2/authorize/";
  oauth_settings.token_endpoint = "http://localhost:8000/oauth2/token/";
  // oauth_settings.client_id = "a6b44cb35e93fb4205b48af1acf5cf";
  // oauth_settings.client_secret = "5320b1e2b32b2affaa4f5fde96a162";
  //oauth_settings.auth_endpoint = "https://avon.home.krandor.org/oauth2/authorize/";
  //oauth_settings.token_endpoint = "https://avon.home.krandor.org/oauth2/token/";
  oauth_settings.client_id = "4a1cb098b443b198c8ef91bb81ab7640";
  oauth_settings.client_secret = "e470f6a4d5292794689ab42d66a6a8f8";
  oauth_settings.scope = "https://api.workrave.org/workrave.state https://api.workrave.org/workrave.config https://api.workrave.org/workrave.history"; 
}


WorkraveAuth::~WorkraveAuth()
{
}


void
WorkraveAuth::init(AsyncAuthResult callback)
{
  g_debug("WorkraveAuth::init");

  this->callback = callback;

  workflow = IOAuth2::create(oauth_settings);
  
  session = IHttpSession::create("");
  session->add_request_filter(workflow->create_filter());

  secret_password_lookup(WORKRAVE_SCHEMA, NULL, on_password_lookup, this, NULL);
}
 

void
WorkraveAuth::on_password_lookup(GObject *source, GAsyncResult *result, gpointer data)
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
      self->workflow->init(boost::bind(&WorkraveAuth::on_auth_result, self, _1, _2));
    }
}


void
WorkraveAuth::on_auth_result(AuthErrorCode result, const string &details)
{
  string access_token;
  string refresh_token;
  time_t valid_until;

  g_debug("on_auth_result: %d %s", result, details.c_str());
  
  workflow->get_tokens(access_token, refresh_token, valid_until);
  
  string password = boost::str(boost::format("%1%:%2%:%3%") % access_token % refresh_token % valid_until);
  g_debug("on_auth_result: %s", password.c_str());

  // TODO: when failed
  // callback(false);
  
  secret_password_store(WORKRAVE_SCHEMA, SECRET_COLLECTION_DEFAULT, "OAuth2",
                        password.c_str(), NULL, on_password_stored, this,
                        NULL);

}

void
WorkraveAuth::on_password_stored(GObject *source, GAsyncResult *result, gpointer data)
{
  WorkraveAuth *self = (WorkraveAuth *)data;
  GError *error = NULL;

  g_debug("secret_password_store");
  
  secret_password_store_finish(result, &error);
  if (error != NULL)
    {
      g_debug("secret_password_store: %s", error->message);
      g_error_free(error);
    }

  // TODO:
  self->callback(true);
}
