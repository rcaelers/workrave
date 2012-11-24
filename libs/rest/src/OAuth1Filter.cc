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

#include "OAuth1Filter.hh"

#include <sstream>
#include <string.h>
#include <list>
#include <vector>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <glib.h>

#ifdef HAVE_CRYPTOPP  
#include <crypto++/cryptlib.h>
#include <crypto++/sha.h>
#include <crypto++/hmac.h>
#include <crypto++/base64.h>
#endif

#include <gcrypt.h>
#include <libsoup/soup.h>

#include "rest/IHttpBackend.hh"
#include "Uri.hh"

using namespace std;

OAuth1Filter::Ptr
OAuth1Filter::create()
{
  return Ptr(new OAuth1Filter());
}

OAuth1Filter::OAuth1Filter()
{
  oauth_version = "1.0";
  signature_method = "HMAC-SHA1";
}

void OAuth1Filter::set_consumer(const std::string &consumer_key, const std::string &consumer_secret)
{
  this->consumer_key = consumer_key;
  this->consumer_secret = consumer_secret;
}

void OAuth1Filter::set_token(const std::string &token_key, const std::string &token_secret)
{
  this->token_key = token_key;
  this->token_secret = token_secret;
}
   
void OAuth1Filter::set_custom_headers(const RequestParams &custom_headers)
{
  this->custom_headers = custom_headers;
}

bool
OAuth1Filter::has_credentials() const
{
  return consumer_key != "" &&
    consumer_secret != "" &&
    token_key != "" &&
    token_secret != "";
}

void
OAuth1Filter::get_credentials(string &consumer_key, string &consumer_secret, string &token_key, string &token_secret)
{
  consumer_key = this->consumer_key;
  consumer_secret = this->consumer_secret;
  token_key = this->token_key;
  token_secret = this->token_secret;
}

const string
OAuth1Filter::get_timestamp() const
{
  time_t now = time (NULL);

  stringstream ss;
  ss << now;
  return ss.str();
}


const string
OAuth1Filter::get_nonce() const
{
  static bool random_seeded = 1;
  const int nonce_size = 32;
  const char *valid_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const int valid_chars_count = strlen(valid_chars);
  char nonce[nonce_size + 1] = { 0, };
  
  if (!random_seeded)
    {
      g_random_set_seed(time(NULL));
      random_seeded = true;
    }

  for (int i = 0; i < nonce_size; i++)
    {
      nonce[i] = valid_chars[g_random_int_range(0, valid_chars_count)];
    }

  return nonce;
}


const string
OAuth1Filter::normalize_uri(const string &uri, RequestParams &parameters) const
{
  SoupURI *u = soup_uri_new(uri.c_str());
  string ret = uri;
    
  if (u != NULL)
    {
      if (u->query != NULL)
        {
          vector<string> query_params;
          boost::split(query_params, Uri::unescape(u->query), boost::is_any_of("&"));

          for (size_t i = 0; i < query_params.size(); ++i)
            {
              vector<string> param_elements;
              boost::split(param_elements, query_params[i], boost::is_any_of("="));

              if (param_elements.size() == 2)
                {
                  parameters[param_elements[0]] = param_elements[1];
                }
            }

          g_free(u->query);
          u->query = NULL;
        }
      
      char *new_uri = soup_uri_to_string(u, FALSE);
      ret = new_uri;
      g_free(new_uri);
      soup_uri_free(u);
    }
 
  return ret;
}
  

const string
OAuth1Filter::parameters_to_string(const RequestParams &parameters, ParameterMode mode) const
{
  list<string> sorted;
  string sep;
  string quotes;
  bool only_oauth;
  
  switch (mode)
    {
    case ParameterModeRequest:
      quotes = "";
      sep = "&";
      only_oauth = false;
      break;
      
    case ParameterModeHeader:
      quotes = "\"";
      sep = ",";
      only_oauth = true;
      break;
      
    case ParameterModeSignatureBase:
      quotes = "";
      sep = "&";
      only_oauth = false;
      break;
    }
  
  for(RequestParams::const_iterator it = parameters.begin(); it != parameters.end(); it++)
    {
      string key = it->first;
      if (!only_oauth || key.find("oauth_") == 0 || custom_headers.find(key) != custom_headers.end())
        {
          string param = key + "=" + quotes + Uri::escape(it->second) +  quotes;
          sorted.push_back(param);
        }
    }
  sorted.sort();

  string norm;
  for(list<string>::iterator it = sorted.begin(); it != sorted.end(); it++)
    {
      if (norm.size() > 0)
        {
          norm += sep;
        }
      norm += *it;
    }

  return norm;
}


const string
OAuth1Filter::encrypt(const string &input, const string &key) const
{
#ifdef HAVE_CRYPTOPP  
  uint8_t digest[CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE];

  CryptoPP::HMAC<CryptoPP::SHA1> hmac((uint8_t*)key.c_str(), key.size());
  hmac.Update((uint8_t*)input.c_str(), input.size());
  hmac.Final(digest);

  CryptoPP::Base64Encoder base64; 
  base64.Put(digest, sizeof(digest));
  base64.MessageEnd();
  base64.MessageSeriesEnd();
  
  unsigned int size = (sizeof(digest) + 2 - ((sizeof(digest) + 2) % 3)) * 4 / 3;
  uint8_t* encoded_values = new uint8_t[size + 1]; // TODO: leak
  
  base64.Get(encoded_values, size);
  encoded_values[size] = 0;
  return string((char *)encoded_values);

#else
  string ret;
  gcry_md_hd_t hd = NULL;
  
  gcry_error_t err = gcry_md_open(&hd, GCRY_MD_SHA1, GCRY_MD_FLAG_SECURE | GCRY_MD_FLAG_HMAC);
  if (err == 0)
    {
      err = gcry_md_setkey(hd, key.c_str(), key.length());
    }
      
  if (err == 0)
    {
      gcry_md_write(hd, input.c_str(), input.length());
      
      size_t digest_length = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
      guchar *digest = (guchar *)gcry_md_read(hd, 0);
      gchar *digest64 = g_base64_encode(digest, digest_length);
      
      ret = digest64;
      g_free(digest64);
    }

  if (err != 0)
    {
      g_printerr("Failed to encrypt: %s", gcry_strerror(err));
    }

  if (hd != NULL)
    {
      gcry_md_close(hd);
    }

  return ret;
#endif  
}


const string
OAuth1Filter::create_oauth_header(const string &http_method,
                           const string &uri,
                           RequestParams &parameters) const
{
  parameters.insert(custom_headers.begin(), custom_headers.end());
  
  parameters["oauth_consumer_key"] = consumer_key;
  parameters["oauth_signature_method"] = signature_method;
  parameters["oauth_timestamp"] = get_timestamp();
  parameters["oauth_nonce"] = get_nonce();
  parameters["oauth_version"] = oauth_version;

  if (token_key != "")
    {
      parameters["oauth_token"] = token_key;
    }

  string key = consumer_secret + "&" + token_secret;
  string normalized_uri = normalize_uri(uri, parameters);
  string normalized_parameters = parameters_to_string(parameters, ParameterModeSignatureBase);

  string signature_base_string = ( http_method + "&" +
                                   Uri::escape(normalized_uri) + "&" +
                                   Uri::escape(normalized_parameters)
                                   );

  // g_debug("BASE %s", signature_base_string.c_str());
  // g_debug("KEY %s", key.c_str());
  
  string signature = encrypt(signature_base_string, key);

  parameters["oauth_signature"] = signature;
  
  return  "OAuth " + parameters_to_string(parameters, ParameterModeHeader);
}


void
OAuth1Filter::filter(HttpRequest::Ptr request)
{
  RequestParams parameters;
  string oauth_header;
  
  if (consumer_key != "" && consumer_secret != "")
    {
      oauth_header = create_oauth_header(request->method, request->uri, parameters);
    }

  if (oauth_header != "")
    {
      request->headers["Authorization"] = oauth_header;
    }
}
  

IHttpExecute::Ptr
OAuth1Filter::create_decorator(IHttpExecute::Ptr executor)
{
  return Decorator::create(shared_from_this(), executor);
}
