// Copyright (C) 2002 - 2013 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2013 Rob Caelers <rob.caelers@gmail.com>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "commonui/Exercise.hh"

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "debug.hh"
#include "utils/AssetPath.hh"
#include "utils/Paths.hh"

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

using namespace std;
using namespace workrave::utils;

/* Updates language dependent attribute */
static void
exercise_parse_update_i18n_attribute(const char *const *languages,
                                     std::string &cur_value,
                                     int &cur_rank,
                                     const std::string &new_value,
                                     const std::string &new_lang)
{
  if (languages != nullptr)
    {
      const char *nl = new_lang.c_str();
      size_t nl_len = strlen(nl);
      int r = 0;

      if (!nl_len)
        {
          nl = "en";
          nl_len = 2;
        }

      for (r = 0; languages[r] != nullptr; r++)
        {
          const char *lang = (const char *)languages[r];

          if (!strncmp(lang, nl, nl_len))
            {
              break;
            }
        }

      if (languages[r] == nullptr)
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

void
ExerciseCollection::parse_exercises(const std::string &file_name)
{
  TRACE_ENTRY_PAR(file_name);

  boost::property_tree::ptree pt;
  read_xml(file_name, pt);

#if defined(HAVE_GLIB)
  const char *const *languages = g_get_language_names();
#else
  const char *const *languages = nullptr;
#endif

  for (boost::property_tree::ptree::value_type &v: pt.get_child("exercises"))
    {
      if (v.first == "exercise")
        {
          exercises.emplace_back();
          Exercise &exercise = exercises.back();

          int title_lang_rank = -1;
          int description_lang_rank = -1;

          for (boost::property_tree::ptree::value_type &ve: v.second)
            {
              string lang = ve.second.get<string>("<xmlattr>.xml:lang", "en");

              if (ve.first == "title")
                {
                  auto title = v.second.get<string>("title");

                  exercise_parse_update_i18n_attribute(nullptr, exercise.title, title_lang_rank, title, lang);
                }
              else if (ve.first == "description")
                {
                  auto description = v.second.get<string>("description");

                  exercise_parse_update_i18n_attribute(languages, exercise.description, description_lang_rank, description, lang);
                }
              else if (ve.first == "sequence")
                {
                  exercise.duration = ve.second.get<int>("<xmlattr>.duration", 15);

                  for (boost::property_tree::ptree::value_type &vs: ve.second)
                    {
                      if (vs.first == "image")
                        {
                          int duration = vs.second.get<int>("<xmlattr>.duration", 1);
                          auto src = vs.second.get<string>("<xmlattr>.src");
                          bool mirrorx = vs.second.get<string>("<xmlattr>.mirrorx", "no") == "yes";

                          exercise.sequence.emplace_back(src, duration, mirrorx);
                        }
                    }
                }
            }
        }
    }

#if defined(HAVE_TRACING)
  for (auto &exercise: exercises)
    {
      TRACE_MSG("exercise title= {}", exercise.title);
      TRACE_MSG("exercise desc= {}", exercise.description);
      TRACE_MSG("exercise duration= {}", exercise.duration);
      TRACE_MSG("exercise seq:");
      for (auto &image: exercise.sequence)
        {
          TRACE_MSG("exercise seq src={} dur={}", image.image, image.duration);
        }
      TRACE_MSG("exercise end seq");
    }
#endif
}

ExerciseCollection::ExerciseCollection()
{
  TRACE_ENTRY();
  load();
}

void
ExerciseCollection::load()
{
  auto main_file = AssetPath::complete_directory("exercises.xml", AssetPath::SEARCH_PATH_EXERCISES);

  if (!main_file.empty())
    {
      parse_exercises(main_file);
    }

  for (auto &directory: Paths::get_data_directories())
    {
      auto execises_directory = directory / "exercises";
      if (std::filesystem::is_directory(execises_directory))
        {
          for (const auto &file: std::filesystem::directory_iterator(execises_directory))
            {
              if (file.path().extension() == ".xml")
                {
                  parse_exercises(file.path().string());
                }
            }
        }
    }
}

std::list<Exercise>
ExerciseCollection::get_exercises()
{
  return exercises;
}

bool
ExerciseCollection::has_exercises()
{
  return !exercises.empty();
}
