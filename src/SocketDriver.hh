// SocketDriver.hh
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef SOCKETDRIVER_HH
#define SOCKETDRIVER_HH

class SocketConnection
{
public:
  //! Client data.
  void *data;

public:
  SocketConnection();
  virtual ~SocketConnection();

  // virtual char *get_canonical_name() = 0;
  virtual bool read(void *buf, int count, int &bytes_read) = 0;
  virtual bool write(void *buf, int count, int &bytes_written) = 0;
  virtual bool close() = 0;
  
  void *get_data() const
  {
    return data;
  }
  
  void set_data(void *d)
  {
    data = d;
  }
};


class SocketListener
{
public:
  virtual void socket_accepted(SocketConnection *con) = 0;
  virtual void socket_connected(SocketConnection *con, void *data) = 0;
  virtual void socket_io(SocketConnection *con, void *data) = 0;
  virtual void socket_closed(SocketConnection *con, void *data) = 0;
};


class SocketDriver
{
public:
  SocketDriver();
  virtual ~SocketDriver();

  // virtual char *get_my_canonical_name() = 0;
  // virtual char *canonicalize(char *) = 0;
  
  virtual bool init() = 0;
  virtual SocketConnection *connect(char *hostname, int port, void *data) = 0;
  virtual SocketConnection *listen(int port, void *data) = 0;

  void set_listener(SocketListener *l);

protected:
  SocketListener *listener;
};

#endif // SOCKETDRIVER_HH
