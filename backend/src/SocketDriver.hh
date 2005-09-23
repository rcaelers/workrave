// SocketDriver.hh
//
// Copyright (C) 2002, 2003, 2005 Rob Caelers <robc@krandor.org>
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


//! TCP Socket connection abstraction.
class SocketConnection
{
public:
  //! User-defined client data.
  void *data;

public:
  SocketConnection();
  virtual ~SocketConnection();

  //! Reads data from the connection.
  virtual bool read(void *buf, int count, int &bytes_read) = 0;

  //! Writes data to the connection
  virtual bool write(void *buf, int count, int &bytes_written) = 0;

  //! Closes the connection.
  virtual bool close() = 0;

  //! Returns the user defined data of this connection.
  void *get_data() const;

  //! Sets the user defined data of this connection.
  void set_data(void *d);
};


//! Asynchronous socket callbacks.
class SocketListener
{
public:
  virtual ~SocketListener() {}
  
  //! The specified server socket connection has accepted a new client connection
  virtual void socket_accepted(SocketConnection *server_con, SocketConnection *client_con) = 0;

  //! The specified socket connection has successfully connected.
  virtual void socket_connected(SocketConnection *con, void *data) = 0;

  //! The specified socket connection has data ready to be read.
  virtual void socket_io(SocketConnection *con, void *data) = 0;

  //! The specified socket connection closed its connection.
  virtual void socket_closed(SocketConnection *con, void *data) = 0;
};


//! TCP Socket abstraction.
class SocketDriver
{
public:
  SocketDriver();
  virtual ~SocketDriver();

  //! Returns this host's canonical host name.
  virtual char *get_my_canonical_name() = 0;

  //! Returns the canonical host name for the specified host name.
  virtual char *canonicalize(const char *) = 0;

  //! Initialize the socket driver.
  virtual bool init() = 0;

  //! Create a connection to the specified host and port.
  virtual SocketConnection *connect(const char *hostname, int port, void *data) = 0;

  //! Listen at the specified port.
  virtual SocketConnection *listen(int port, void *data) = 0;

  //! Sets the callback listener for asynchronous events.
  void set_listener(SocketListener *l);

protected:
  //! Async callbacks.
  SocketListener *listener;
};

#include "SocketDriver.icc"

#endif // SOCKETDRIVER_HH
