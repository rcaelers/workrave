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

#ifndef CLOUDCONTROL_HH
#define CLOUDCONTROL_HH

#include <string>
#include <map>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "WorkraveAuth.hh"
#include "UUID.hh"

#include "rest/AuthError.hh"
#include "rest/IHttpReply.hh"

class CloudControl
{
public:
  typedef boost::shared_ptr<CloudControl> Ptr;

  static Ptr create();
  
 	CloudControl();
  virtual ~CloudControl();

  void init();
  
private:
  void on_auth_ready(workrave::rest::AuthErrorCode error, const std::string &detal, WorkraveAuth::Ptr auth);

  void init_myid();
  
  void signon();
  void on_signon_ready(workrave::rest::IHttpReply::Ptr reply);

  void signoff();
  void on_signoff_ready(workrave::rest::IHttpReply::Ptr reply);

  void take();
  void on_take_ready(workrave::rest::IHttpReply::Ptr reply);

  void release();
  void on_release_ready(workrave::rest::IHttpReply::Ptr reply);

  void subscribe();
  void publish_state();
  void retrieve_state();

  void on_event_headers(workrave::rest::IHttpReply::Ptr reply);
  void on_event_data(const std::string &data);
  void on_event_closed(workrave::rest::HttpErrorCode error, const std::string &detail, workrave::rest::IHttpStreamOperation::Ptr stream);
  void on_publish_state_ready(workrave::rest::IHttpReply::Ptr reply);
  void on_retrieve_state_ready(workrave::rest::IHttpReply::Ptr reply);
  
private:  
  //! My ID
  workrave::cloud::UUID myid;

  WorkraveAuth::Ptr auth;
  workrave::rest::IHttpSession::Ptr session;

  std::string event_url;
};

  
#endif
