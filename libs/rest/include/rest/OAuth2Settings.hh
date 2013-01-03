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

#ifndef WORKRAVE_REST_OAUTH2SETTINGS_HH
#define WORKRAVE_REST_OAUTH2SETTINGS_HH

#include <string>

namespace workrave
{
  namespace rest
  {
    struct OAuth2Settings
    {
      OAuth2Settings()
      {
      }
    
      OAuth2Settings(const std::string &auth_endpoint,
                     const std::string &token_endpoint,
                     const std::string &client_id,
                     const std::string &client_secret,
                     const std::string &scope)
        : auth_endpoint(auth_endpoint),
          token_endpoint(token_endpoint),
          client_id(client_id),
          client_secret(client_secret),
        scope(scope)
      {
      }
    
      std::string auth_endpoint;
      std::string token_endpoint;
      std::string client_id;
      std::string client_secret;
      std::string scope;
    };
  }
}

#endif
