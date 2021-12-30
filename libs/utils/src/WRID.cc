// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $Id$
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "debug.hh"

#include <string>

#include <glib.h>

#include "WRID.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
#  include <wincrypt.h>
#elif defined(PLATFORM_OS_UNIX) || defined(PLATFORM_OS_MACOS)
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <errno.h>
#  include <string.h>
#endif

using namespace workrave;

WRID::WRID()
{
  TRACE_ENTRY();
  create();

  TRACE_VAR(str());
}

WRID::WRID(const WRID &rhs)
{
  TRACE_ENTRY();
  memcpy(id, rhs.id, sizeof(id));
}

WRID::WRID(const std::string &str)
{
  TRACE_ENTRY();
  set(str);
}

WRID &
WRID::operator=(const WRID &lid)
{
  TRACE_ENTRY();
  if (this != &lid)
    {
      memcpy(id, lid.id, sizeof(id));
    }
  return *this;
}

bool
WRID::operator==(const WRID &lid) const
{
  return memcmp(id, lid.id, sizeof(id)) == 0;
}

bool
WRID::operator!=(const WRID &lid) const
{
  return memcmp(id, lid.id, sizeof(id)) != 0;
}

bool
WRID::operator<(const WRID &lid) const
{
  return memcmp(id, lid.id, sizeof(id)) < 0;
}

std::string
WRID::str() const
{
  static const char *hex = "0123456789abcdef";

  char uuid_str[STR_LENGTH + 1];

  for (unsigned int i = 0; i < RAW_LENGTH; i++)
    {
      uuid_str[2 * i] = hex[(id[i] & 0xf0) >> 4];
      uuid_str[2 * i + 1] = hex[id[i] & 0x0f];
    }
  uuid_str[STR_LENGTH] = '\0';

  return uuid_str;
}

guint8 *
WRID::raw() const
{
  return (guint8 *)&id;
}

void
WRID::create()
{
  gint64 now = g_get_real_time();

  guint32 *id32 = ((guint32 *)&id);
  id32[3] = GUINT32_TO_BE(now / G_USEC_PER_SEC);

  get_random_bytes(id, sizeof(id) - 4);
}

bool
WRID::set(const std::string &str)
{
  size_t len = str.length();
  bool ret = true;

  if (len != STR_LENGTH)
    {
      ret = false;
    }

  if (ret)
    {
      memset(id, 0, sizeof(id));
      for (unsigned int i = 0; ret && i < len; i++)
        {
          char nibble = str[i];

          if (nibble >= '0' && nibble <= '9')
            {
              nibble -= '0';
            }
          else if (nibble >= 'a' && nibble <= 'f')
            {
              nibble -= 'a';
              nibble += 10;
            }
          else if (nibble >= 'A' && nibble <= 'F')
            {
              nibble -= 'A';
              nibble += 10;
            }
          else
            {
              nibble = 0;
              ret = false;
            }

          if (i % 2 == 0)
            {
              id[i / 2] |= ((nibble << 4) & 0xf0);
            }
          else
            {
              id[i / 2] |= (nibble & 0x0f);
            }
        }
    }

  return ret;
}

#if defined(PLATFORM_OS_UNIX) || defined(PLATFORM_OS_MACOS)

void
WRID::get_random_bytes(unsigned char *buf, size_t length)
{
  int fd = -1;
  bool ok = false;

  do
    {
      fd = open("/dev/urandom", O_RDONLY);
    }
  while (fd < 0 && errno == EINTR);

  if (fd >= 0)
    {
      size_t s = 0;

      do
        {
          s = read(fd, buf, length);
        }
      while (errno == EINTR);

      if (s == length)
        {
          ok = true;
        }

      close(fd);
    }

  if (!ok)
    {
      GRand *grand = g_rand_new();

      for (unsigned int i = 0; i < length; i++)
        {
          guint32 r = 0;

          if (i % 4 == 0)
            {
              r = g_rand_int(grand);
            }

          buf[i] = r & 0xff;

          r >>= 8;
        }

      g_rand_free(grand);
    }
}

#elif defined(PLATFORM_OS_WINDOWS)

void
WRID::get_random_bytes(unsigned char *buf, size_t length)
{
  HCRYPTPROV hProv;

  /* 0x40 bit = CRYPT_SILENT, only introduced in more recent PSDKs
   * and will only work for Win2K and later.
   */
  DWORD flags = CRYPT_VERIFYCONTEXT;

  if (LOBYTE(LOWORD(GetVersion())) >= 5)
    {
      flags |= 0x40;
    }

  if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, flags))
    {
      CryptGenRandom(hProv, (DWORD)length, buf);
    }

  CryptReleaseContext(hProv, 0);
}

#else

#  error Unsupported platform

#endif
