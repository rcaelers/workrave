// Copyright (C) 2002, 2003, 2004, 2005, 2006 Rob Caelers <robc@krandor.org>
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

#ifndef IDISTRIBUTIONCLIENTMESSAGE_HH
#define IDISTRIBUTIONCLIENTMESSAGE_HH

class PacketBuffer;

enum DistributionClientMessageID
{
  DCM_TIMERS = 0x0010,
  DCM_MONITOR = 0x0011,
  DCM_IDLELOG = 0x0012,
  DCM_SCRIPT = 0x0013,
  DCM_CONFIG = 0x0014,
  DCM_BREAKS = 0x0020,
  DCM_STATS = 0x0030,
  DCM_BREAKCONTROL = 0x0040,
};

enum DistributionClientMessageType
{
  DCMT_PASSIVE = 0x0000,
  DCMT_MASTER = 0x0010,
  DCMT_SIGNON = 0x0020,
};

class IDistributionClientMessage
{
public:
  virtual ~IDistributionClientMessage() = default;

  virtual bool request_client_message(DistributionClientMessageID id, PacketBuffer &buffer) = 0;

  virtual bool client_message(DistributionClientMessageID id, bool active, const char *client_id, PacketBuffer &buffer) = 0;
};

#endif // IDISTRIBUTIONCLIENTMESSAGE_HH
