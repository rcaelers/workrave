// Exercise.cc --- Exercises
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Raymond Penners <raymond@dotsphinx.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_EXERCISES

#include "Exercise.hh"
#include "Util.hh"
#include "nls.h"
#include "debug.hh"

#ifndef PLATFORM_OS_WIN32_NATIVE
#include <unistd.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

struct ExerciseParser
{
  std::list<Exercise> *exercises;
  Exercise *exercise;

  std::string lang;
  int title_lang_rank;
  int description_lang_rank;
  std::string cdata;
  ExerciseParser(std::list<Exercise> &exe);

  const gchar * const *i18n_languages;  
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
exercise_parser_start_element  (GMarkupParseContext *,
                                const gchar         *element_name,
                                const gchar        **attribute_names,
                                const gchar        **attribute_values,
                                gpointer             user_data,
                                GError             **)
{
  TRACE_ENTER_MSG("exercise_parser_start_element", element_name);
  ExerciseParser *ep = (ExerciseParser *) user_data;

  if (! strcmp(element_name, "exercise"))
    {
      ep->exercises->push_back(Exercise());
      ep->exercise = &(ep->exercises->back());

      ep->title_lang_rank = -1;
      ep->description_lang_rank = -1;
    }
  else if (! strcmp(element_name, "sequence"))
    {
      const gchar *duration = exercise_parse_lookup_attribute
        ("duration", attribute_names, attribute_values);
      if (duration)
        {
          ep->exercise->duration = atoi(duration);
        }
      else
        {
          ep->exercise->duration = 15;
        }
    }
  else if (! strcmp(element_name, "image"))
    {
      int dur = 1;
      const gchar *duration = exercise_parse_lookup_attribute
        ("duration", attribute_names, attribute_values);
      if (duration)
        {
          dur = atoi(duration);
        }
      const gchar *src = exercise_parse_lookup_attribute
        ("src", attribute_names, attribute_values);
      const gchar *mirrorx = exercise_parse_lookup_attribute
        ("mirrorx", attribute_names, attribute_values);
      bool mx = mirrorx != NULL && !strcmp(mirrorx, "yes");
      if (src != NULL && strlen(src) > 0)
        {
          TRACE_MSG("Image src=" << src);
          ep->exercise->sequence.push_back(Exercise::Image(src, dur, mx));
        }
    }
  else if (! strcmp(element_name, "exercises"))
    {
    }
  else if (! strcmp(element_name, "title")
           || ! strcmp(element_name, "description"))
    {
      const gchar *value = exercise_parse_lookup_attribute
        ("xml:lang", attribute_names, attribute_values);
      ep->lang = value ? value : "";
      ep->cdata = "";
    }
  else
    {
      TRACE_MSG(element_name);
      abort();
    }

  TRACE_EXIT();
}


/* Updates language dependent attribute */
static void
exercise_parse_update_i18n_attribute(const gchar * const *languages,
                                     std::string &cur_value, int &cur_rank,
                                     const std::string &new_value,
                                     const std::string &new_lang)
{
  if (languages != NULL)
    {
      const char *nl = new_lang.c_str();
      int nl_len = strlen(nl);
      int r;

      if (! nl_len)
        {
          nl = "en";
          nl_len = 2;
        }

      for (r = 0; languages[r] != NULL; r++)
        {
          const gchar *lang = (const gchar *) languages[r];

          if (! strncmp(lang, nl, nl_len))
            {
              break;
            }
        }

      if (languages[r] == NULL)
        {
          // Language not found...
          if (cur_rank < 0)
            {
              // ...and no previous value existed, so we're happy with just anything..
              cur_value = new_value;
              cur_rank = 9999;
            }
          else if (cur_rank == 9999 && !strcmp(nl, "en"))
            {
              // ...but we really prefer to default to English
              cur_value = new_value;
              cur_rank = 9998;
            }
        }
      else
        {
          // Language found
          cur_value = new_value;
          cur_rank = r;

        }
    }
  else
    {
      // No languages, default to English (0).
      if (cur_rank != 0)
        {
          cur_value = new_value;
          if (new_lang == "" || new_lang == "en")
            {
              cur_rank = 0;
            }
          else
            {
              cur_rank = 1;
            }
        }
    }
}


/* Called for close tags </foo> */
static void
exercise_parser_end_element (GMarkupParseContext *,
                             const gchar         *element_name,
                             gpointer             user_data,
                             GError             **)
{
  TRACE_ENTER_MSG("exercise_parser_end_element", element_name);

  ExerciseParser *ep = (ExerciseParser *) user_data;
  if (! strcmp(element_name, "title"))
    {
      exercise_parse_update_i18n_attribute
        (ep->i18n_languages, ep->exercise->title, ep->title_lang_rank,
         ep->cdata, ep->lang);
    }
  else if (! strcmp(element_name, "description"))
    {
      exercise_parse_update_i18n_attribute
        (ep->i18n_languages, ep->exercise->description,
         ep->description_lang_rank, ep->cdata, ep->lang);
    }
  TRACE_EXIT();
}

/* Called for character data */
/* text is not nul-terminated */
static void
exercise_parser_text (GMarkupParseContext *,
                      const gchar         *text,
                      gsize                text_len,
                      gpointer             user_data,
                      GError             **)
{
  TRACE_ENTER_MSG("exercise_parser_text", text);
  ExerciseParser *ep = (ExerciseParser *) user_data;
  ep->cdata.append(text, text_len);
  TRACE_EXIT();
}


ExerciseParser::ExerciseParser(std::list<Exercise> &exe)
{
  TRACE_ENTER("ExerciseParser::ExerciseParser");
  exercises = &exe;
  exercise = NULL;
  lang = "";

  i18n_languages =  g_get_language_names(); // nls_get_language_list("LC_MESSAGES");

  // FIXME: remove this debugging
  int i = 0;
  for (i = 0; i18n_languages[i] != NULL; i++)
    {
      TRACE_MSG("lang " << i18n_languages[i]);
    }
  TRACE_EXIT();
}


void
Exercise::parse_exercises(const char *file_name,
                          std::list<Exercise> &exe)
{
  FILE *stream = NULL;
  TRACE_ENTER_MSG("ExercisesParser::get_exercises", file_name);

  stream = fopen(file_name, "rb");
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

#ifndef NDEBUG
  for (std::list<Exercise>::iterator it = exe.begin(); it != exe.end(); it++)
    {
      Exercise &ex = *it;

      TRACE_MSG("exercise title=" << ex.title);
      TRACE_MSG("exercise desc=" << ex.description);
      TRACE_MSG("exercise duration=" << ex.duration);
      TRACE_MSG("exercise seq:");
      for (std::list<Exercise::Image>::iterator sit = ex.sequence.begin();
           sit != ex.sequence.end(); sit++)
        {
          Exercise::Image &img = *sit;
          TRACE_MSG("exercise seq src=" << img.image
                    << ", dur=" << img.duration);
        }
      TRACE_MSG("exercise end seq");
    }
#endif

  TRACE_EXIT();
}


std::string
Exercise::get_exercises_file_name()
{
  return Util::complete_directory
    ("exercises.xml", Util::SEARCH_PATH_EXERCISES);
}


std::list<Exercise>
Exercise::get_exercises()
{
  std::list<Exercise> exercises;
  std::string file_name = get_exercises_file_name();
  if (file_name.length () > 0)
    {
      parse_exercises(file_name.c_str(), exercises);
    }
  return exercises;
}

bool
Exercise::has_exercises()
{
  std::string file_name = get_exercises_file_name();
  return file_name.length() > 0;
}


#endif // HAVE_EXERCISES
