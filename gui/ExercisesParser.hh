// ExercisesParser.hh --- Exercises parser
//
// Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef EXERCISES_PARSER_HH
#define EXERCISES_PARSER_HH

#include "config.h"

#ifdef HAVE_EXERCISES

#include <glibmm/markup.h>
#include <list>

class ExercisesParser : public Glib::Markup::Parser
{
};

#endif // HAVE_EXERCISES

#endif // EXERCISES_PARSER_HH
