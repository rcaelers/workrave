// Copyright (C) 2010, 2011, 2012 by Rob Caelers <robc@krandor.nl>
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

#include "HttpExecuteSoup.hh"

#include <map>
#include <string>

using namespace std;

HttpExecuteSoup::HttpExecuteSoup(SoupSession *session, HttpRequest::Ptr request) : session(session), request(request)
{
  reply = HttpReply::create(request);
}


HttpExecuteSoup::~HttpExecuteSoup()
{
}


SoupMessage *
HttpExecuteSoup::create_request_message()
{
  SoupMessage *message = soup_message_new(request->method.c_str(), request->uri.c_str());
  if (message == NULL)
    {
      throw std::exception(); // HttpBackendException("Cannot create HTTP request: " + request->method + " " + request->uri);
    }

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

  return message;
}


void
HttpExecuteSoup::process_reply_message(SoupMessage *message)
{
  reply->status = message->status_code;
  reply->body = (message->response_body->length > 0) ? message->response_body->data : "";

  SoupMessageHeadersIter iter;
  const char *name, *value;
  soup_message_headers_iter_init(&iter, message->response_headers);

  while (soup_message_headers_iter_next(&iter, &name, &value))
    {
      string n = name;
      string v = value;
      reply->headers[n] = v;
    }
}
