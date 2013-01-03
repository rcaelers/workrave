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

#ifndef WORKRAVEAUTH_HH
#define WORKRAVEAUTH_HH

#include <string>
#include <map>
#include <gio/gio.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <libsecret/secret.h>

#include "rest/IOAuth2.hh"

#include "rest/IHttpSession.hh"

#ifdef __cplusplus
extern "C" {
#endif

  const SecretSchema * workrave_get_schema(void) G_GNUC_CONST;

#define WORKRAVE_SCHEMA  workrave_get_schema()

#ifdef __cplusplus
}
#endif
  
class WorkraveAuth
{
public:
  typedef boost::shared_ptr<WorkraveAuth> Ptr;
  typedef boost::function<void (workrave::rest::AuthErrorCode, const std::string &) > AuthResultCallback;

  static Ptr create();
  
 	WorkraveAuth();
  virtual ~WorkraveAuth();
  
  void init(std::string access_token, std::string refresh_token);
  void init(AuthResultCallback callback);

  workrave::rest::IHttpSession::Ptr get_session() const
  {
    return session;
  }

private:
  void on_auth_result(workrave::rest::AuthErrorCode result, const std::string &details);
  void on_auth_feedback(const std::string &error, const std::string &error_description, workrave::rest::IHttpReply::Ptr reply);

  static void on_password_lookup(GObject *source, GAsyncResult *result, gpointer data);
  static void on_password_stored(GObject *source, GAsyncResult *result, gpointer data);
  
private:  
  workrave::rest::IOAuth2::Ptr workflow;
  workrave::rest::IHttpSession::Ptr session;
  AuthResultCallback callback;
  workrave::rest::OAuth2Settings oauth_settings;
};

  
#endif
