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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdexcept>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "rest/HttpError.hh"
#include "HttpUtils.hh"

#ifdef PLATFORM_OS_OSX
#import <AppKit/NSWorkspace.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#endif

using namespace std;
using namespace workrave::rest;

HttpReply::Ptr
HttpUtils::process_reply_message(SoupMessage *message)
{
  HttpReply::Ptr reply = HttpReply::create();
  
  reply->error = HttpErrorCode::Success;
  reply->status = message->status_code;
  if (SOUP_STATUS_IS_TRANSPORT_ERROR(reply->status))
    {
      // TODO: translate errors.
      reply->error = HttpErrorCode::Transport;
    }
  
  reply->body = (message->response_body->length > 0) ? message->response_body->data : "";

  SoupMessageHeadersIter iter;
  soup_message_headers_iter_init(&iter, message->response_headers);

  const char *name, *value;
  while (soup_message_headers_iter_next(&iter, &name, &value))
    {
      string n = name;
      string v = value;
      reply->headers[n] = v;
    }

  return reply;
}


SoupMessage *
HttpUtils::create_request_message(HttpRequest::Ptr request)
{
  SoupMessage *message = soup_message_new(request->method.c_str(), request->uri.c_str());
  if (message == NULL)
    {
      throw HttpError(HttpErrorCode::Failure, "Cannot create SOUP message.");
    }

  setup_request_message(message, request);
  return message;
}


void
HttpUtils::setup_request_message(SoupMessage *message, HttpRequest::Ptr request)
{
  if (request->body != "")
    {
      soup_message_set_request(message, request->content_type.c_str(), SOUP_MEMORY_COPY,
                               request->body.c_str(), request->body.size());
    }

  for (map<string, string>::const_iterator i = request->headers.begin(); i != request->headers.end(); i++)
    {
      soup_message_headers_append(message->request_headers, i->first.c_str(), i->second.c_str());
    }
  
	soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);
}


void
HttpUtils::open_in_browser(const std::string &location)
{
#if defined(PLATFORM_OS_UNIX)     
  gchar *program = g_find_program_in_path("xdg-open");
  if (program == NULL)
    {
      throw HttpError(HttpErrorCode::Failure, "Cannot find xdg-open");
    }

  string command = string(program) + " " + location;

  gint exit_code = 0;
  GError *error = NULL;
  if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
    {
      throw HttpError(HttpErrorCode::Failure, boost::str(boost::format("Failed to execute '%1%' (%2%)") % command % error->message));
    }

  if (WEXITSTATUS(exit_code) != 0)
    {
      throw HttpError(HttpErrorCode::Failure, boost::str(boost::format("xdg-open returned an error exit-code %1%") % WEXITSTATUS(exit_code)));
    }
#elif defined(PLATFORM_OS_OSX)
  NSString* uri = [NSString stringWithCString:location.c_str() encoding: NSUTF8StringEncoding];
  [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: uri]];
#else
#error Not ported
#endif
}
