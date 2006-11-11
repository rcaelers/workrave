// W32Mp3Player.cc
//
// Copyright (C) 2006 Raymond Penners
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

#include "W32Mp3Player.hh"

W32Mp3Player::W32Mp3Player()
: paused(false)
{
}

W32Mp3Player::~W32Mp3Player()
{
  unload();
}

void
W32Mp3Player::load(const char *fn)
{
  file_name = fn;
  paused = false;
  load();
}

std::string
W32Mp3Player::get_mci_command(const char *cmd)
{
  std::string s = "" + std::string(cmd) + " \"" + get_file_name() + "\"";
  return s;
}


void
W32Mp3Player::exec_mci_command(const char *cmd)
{
  std::string s = get_mci_command(cmd);
  mciSendString(s.c_str(), NULL, 0, 0);
}


void
W32Mp3Player::play()
{
  std::string cmd = get_mci_command("play") + " from 0";
  mciSendString(cmd.c_str(), NULL, 0, 0);
}

void
W32Mp3Player::stop()
{
  exec_mci_command("stop");
}

void
W32Mp3Player::pause()
{
  if (! is_paused())
    {
      exec_mci_command("pause");
      set_paused(true);
    }
}

void
W32Mp3Player::resume()
{
  if (is_paused())
    {
      exec_mci_command("resume");
      set_paused(false);
    }
}

void
W32Mp3Player::unload()
{
  stop();
  exec_mci_command("close");
}

std::string
W32Mp3Player::get_file_name()
{
  return file_name;
}

bool
W32Mp3Player::is_paused()
{
  return paused;
}

void
W32Mp3Player::set_paused(bool p)
{
  paused = p;
}

void
W32Mp3Player::load()
{
  std::string cmd = get_mci_command("open") + " type mpegvideo alias "
    + "\"" + get_file_name() + "\"";		
  mciSendString(cmd.c_str(), NULL, 0, 0);
}

