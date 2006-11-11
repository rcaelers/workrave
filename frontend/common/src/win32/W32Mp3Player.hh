// W32Mp3Player.hh
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

#ifndef W32MP3PLAYER_HH
#define W32MP3PLAYER_HH

#include <string>
#include <windows.h>
#include <mmsystem.h>

class W32Mp3Player 
{
public:
  W32Mp3Player();
  ~W32Mp3Player();

  void load(const char *szFileName);
  void play();
  void stop();
  void pause();
  void resume();
  void unload();

private:
  void exec_mci_command(const char *cmd);
  std::string get_mci_command(const char *cmd);
  std::string get_file_name();
  bool is_paused();
  void set_paused(bool paused);
  void load();

  std::string file_name;
  bool paused;
};


#endif // W32MP3PLAYER_HH
