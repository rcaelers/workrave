// Exercise.cc --- Exercises
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

#include "Exercise.hh"
#include "Util.hh"
#include "nls.h"
#include "debug.hh"

#include <glib.h>
#include <unistd.h>
#include <assert.h>

struct ExerciseParser
{
  std::list<Exercise> *exercises;
  Exercise *exercise;
  std::string lang;
  std::string cdata;
  ExerciseParser(std::list<Exercise> &exe);
};

static const gchar *
exercise_parse_lookup_attribute(const gchar *find, const gchar **names,
                                const gchar **values)
{
  const gchar *ret = NULL;
  if (names != NULL)
    {
      for (int i = 0; ; i++)
        {
          if (! names[i])
            break;
          
          if (! strcmp(names[i], find))
            {
              ret = values[i];
              break;
            }
        }
    }
  return ret;
}
                                
/* Called for open tags <foo bar="baz"> */
static void
exercise_parser_start_element  (GMarkupParseContext *context,
                                const gchar         *element_name,
                                const gchar        **attribute_names,
                                const gchar        **attribute_values,
                                gpointer             user_data,
                                GError             **error)
{
  TRACE_ENTER_MSG("exercise_parser_start_element", element_name);
  ExerciseParser *ep = (ExerciseParser *) user_data;
  
  if (! strcmp(element_name, "exercise"))
    {
      ep->exercises->push_back(Exercise());
      ep->exercise = &(*(ep->exercises->end()));
    }

  const gchar *value = exercise_parse_lookup_attribute
    ("xml:lang", attribute_names, attribute_values);
  ep->lang = value ? value : "";
  ep->cdata = "";
  TRACE_EXIT();
}



/* Called for close tags </foo> */
static void
exercise_parser_end_element (GMarkupParseContext *context,
                             const gchar         *element_name,
                             gpointer             user_data,
                             GError             **error)
{
  TRACE_ENTER_MSG("exercise_parser_end_element", element_name);

  ExerciseParser *ep = (ExerciseParser *) user_data;
  if (! strcmp(element_name, "title"))
    {
      TRACE_MSG("title=" << ep->cdata);
    }
  else if (! strcmp(element_name, "description"))
    {
      TRACE_MSG("desc=" << ep->cdata);
    }
  TRACE_MSG("lang=" << ep->lang);
  TRACE_EXIT();
}

/* Called for character data */
/* text is not nul-terminated */
static void
exercise_parser_text (GMarkupParseContext *context,
                      const gchar         *text,
                      gsize                text_len,  
                      gpointer             user_data,
                      GError             **error)
{
  TRACE_ENTER_MSG("exercise_parser_text", text);
  ExerciseParser *ep = (ExerciseParser *) user_data;
  ep->cdata.append(text);
  TRACE_EXIT();
}


ExerciseParser::ExerciseParser(std::list<Exercise> &exe)
{
  exercises = &exe;
  exercise = NULL;
  lang = "";
}


   

void
Exercise::parse_exercises(const char *file_name,
                          std::list<Exercise> &exe)
{
  TRACE_ENTER_MSG("ExercisesParser::get_exercises", file_name);
  
  FILE *stream = fopen(file_name, "rb");
  if (stream)
    {
      GMarkupParser parser;

      parser.text = exercise_parser_text;
      parser.start_element = exercise_parser_start_element;
      parser.end_element = exercise_parser_end_element;
      parser.text = exercise_parser_text;
      parser.passthrough = NULL;
      parser.error = NULL;
        
      ExerciseParser eparser(exe);
      GMarkupParseContext *context
        = g_markup_parse_context_new(&parser, (GMarkupParseFlags) 0,
                                     &eparser, NULL);
      GError *error = NULL;

      char buf[1024];
      while (true)
        {
          int n = fread(buf, 1, sizeof(buf), stream);
          if (ferror(stream))
            break;
          g_markup_parse_context_parse(context, buf, n, &error);
          if (feof(stream))
            break;
        }
      fclose(stream);
      g_markup_parse_context_end_parse(context, &error);
      g_markup_parse_context_free(context);
    }
  TRACE_EXIT();
}


void
Exercise::parse_exercises(std::list<Exercise> &exercises)
{
  std::string file_name = Util::complete_directory
    ("exercises.xml", Util::SEARCH_PATH_EXERCISES);
  return parse_exercises(file_name.c_str(), exercises);

}

#endif // HAVE_EXERCISES
