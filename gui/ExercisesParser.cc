// PreferencesParser.cc --- Preferences parser
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_EXERCISES

#include "ExercisesParser.hh"
#include "Util.hh"

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <assert.h>

std::list<Exercise> *ExercisesParser::exercises = NULL;

void
ExercisesParser::on_start_element (Glib::Markup::ParseContext& context,
                                   const Glib::ustring& element_name,
                                   const AttributeMap& attributes)
{
  TRACE_ENTER_MSG("ExercisesParser::on_start_element", element_name);
  TRACE_EXIT();
}

void
ExercisesParser::on_end_element (Glib::Markup::ParseContext& context,
                                 const Glib::ustring& element_name)
{
  TRACE_ENTER_MSG("ExercisesParser::on_end_element", element_name);
  TRACE_EXIT();
}

void
ExercisesParser::on_text (Glib::Markup::ParseContext& context,
                          const Glib::ustring& text)
{
  TRACE_ENTER_MSG("ExercisesParser::on_text", text);
  TRACE_EXIT();
}

void
ExercisesParser::on_passthrough (Glib::Markup::ParseContext& context,
                                 const Glib::ustring& passthrough_text)
{
  TRACE_ENTER_MSG("ExercisesParser::on_passthrough", passthrough_text);
  TRACE_EXIT();
}
   

void
ExercisesParser::parse_exercises(std::string file_name,
                                 std::list<Exercise> &exe)
{
  TRACE_ENTER_MSG("ExercisesParser::get_exercises", file_name);
  exercises = &exe;
  
  // I hate C++ streams.
  FILE *stream = fopen(file_name.c_str(), "rb");
  if (stream)
    {
      ExercisesParser parser;
      Glib::Markup::ParseContext context(parser);

      char buf[1024];
      while (true)
        {
          int n = fread(buf, 1, sizeof(buf), stream);
          if (ferror(stream))
            break;
          context.parse(buf, buf + n);
          if (feof(stream))
            break;
        }
      fclose(stream);
      context.end_parse();
    }
  TRACE_EXIT();
}


void
ExercisesParser::parse_exercises(std::list<Exercise> &exercises)
{
  std::string file_name = Util::complete_directory
    ("exercises.xml", Util::SEARCH_PATH_EXERCISES);
  return get_exercises(file_name, exercises);
}

#endif // HAVE_EXERCISES
