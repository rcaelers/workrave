// Copyright 2015 The Crashpad Authors. All rights reserved.
// Copyright 2021 Rob Caelers <robc@krandor.nl>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CRASH_DIALOG_HH
#define CRASH_DIALOG_HH

#include <string>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textbuffer.h>

#include "handler/user_hook.h"

namespace Gtk
{
  class TextView;
}

class CrashDialog : public Gtk::Dialog
{
public:
  CrashDialog(const std::string &info);
  ~CrashDialog() override;

  std::string get_user_text() const;

private:
  Gtk::TextView *text_view{nullptr};
  Gtk::VBox *vbox{nullptr};
  Gtk::ScrolledWindow scrolled_window;
  Glib::RefPtr<Gtk::TextBuffer> text_buffer;
};

class UserInteraction : public crashpad::UserHook
{
public:
  UserInteraction() = default;
  ~UserInteraction() override = default;

  bool reportCrash(const std::string &info) override;
  std::string getUserText() override;

private:
  std::string user_text;
  bool consent{false};
};

#endif // CRASH_DIALOG_HH
