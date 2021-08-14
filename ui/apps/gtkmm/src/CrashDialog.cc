// Copyright 2015 The Crashpad Authors. All rights reserved.
// Copyright 2021 Rob Caelers
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

#include "CrashDialog.hh"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <windows.h>

#include <gtkmm.h>

#include "handler/handler_main.h"
#include "build/build_config.h"
#include "tools/tool_support.h"

#include "commonui/nls.h"
#include "debug.hh"

CrashDialog::CrashDialog(const std::string &info)
  : Gtk::Dialog(_("Crash report"), true)
{
  TRACE_ENTER("CrashDialog::CrashDialog");

  set_default_size(600, 400);
  set_title(_("Workrave has crashed"));
  set_border_width(6);

  vbox = Gtk::manage(new Gtk::VBox());
  vbox->set_border_width(6);
  vbox->set_spacing(6);
  // vbox->set_halign(Gtk::ALIGN_START);

  get_vbox()->pack_start(*vbox, true, true, 0);

  std::string bold = "<span weight=\"bold\">";
  std::string end = "</span>";

  auto info_label = Gtk::manage(new Gtk::Label(bold + _("Workrave has crashed.") + end, Gtk::ALIGN_START));
  info_label->set_use_markup();
  vbox->pack_start(*info_label, false, false, 0);

  auto user_text_label = Gtk::manage(new Gtk::Label(_("Please describe what was happening just before the crash:"), Gtk::ALIGN_START));
  vbox->pack_start(*user_text_label, false, false, 0);

  auto user_text_frame = Gtk::manage(new Gtk::Frame);
  user_text_frame->set_shadow_type(Gtk::SHADOW_IN);
  vbox->pack_start(*user_text_frame, true, true, 0);

  text_buffer = Gtk::TextBuffer::create();
  text_view = Gtk::manage(new Gtk::TextView(text_buffer));
  text_view->set_cursor_visible(false);
  text_view->set_editable(true);

  scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled_window.add(*text_view);

  Gtk::HBox *box = Gtk::manage(new Gtk::HBox(false, 6));
  box->pack_start(scrolled_window, true, true, 0);

  user_text_frame->add(*box);

  // vbox->pack_start(*box, true, true, 0);

  auto consent_label =
    Gtk::manage(new Gtk::Label(_("Please help us fix this crash by submitting the crash report. Thanks!"), Gtk::ALIGN_START));
  vbox->pack_start(*consent_label, false, false, 0);

  add_button(_("Submit"), Gtk::RESPONSE_ACCEPT);
  add_button(_("Don't submit"), Gtk::RESPONSE_REJECT);

  show_all();

  TRACE_EXIT();
}

CrashDialog::~CrashDialog()
{
  TRACE_ENTER("CrashDialog::~CrashDialog");
  TRACE_EXIT();
}

std::string
CrashDialog::get_user_text() const
{
  return text_buffer->get_text();
}

bool
UserInteraction::reportCrash(const std::string &info)
{
  SetEnvironmentVariableA("GTK_DEBUG", 0);
  SetEnvironmentVariableA("G_MESSAGES_DEBUG", 0);

  auto app = Gtk::Application::create();
  app->register_application();

  auto dlg = new CrashDialog(info);
  dlg->signal_response().connect([this, app, dlg](int response) {
    user_text = dlg->get_user_text();
    consent = response == Gtk::RESPONSE_ACCEPT;
    app->quit();
  });
  app->hold();
  dlg->show();
  return app->run();
}

std::string
UserInteraction::getUserText()
{
  return user_text;
}

namespace
{

  int HandlerMainAdaptor(int argc, char *argv[])
  {
    TRACE_ENTER("HandlerMainAdaptor");
    UserInteraction *user_interaction = new UserInteraction;
    int ret = crashpad::HandlerMain(argc, argv, nullptr, user_interaction);
    // int ret = 0;
    // user_interaction->reportCrash("Foo");
    delete user_interaction;
    TRACE_RETURN(ret);
    return ret;
  }

} // namespace

// The default entry point for /subsystem:windows. In Crashpad’s own build, this
// is used by crashpad_handler.exe. It’s also used by crashpad_handler.com when
// produced by editbin from a copy of crashpad_handler.exe.
int APIENTRY
wWinMain(HINSTANCE, HINSTANCE, wchar_t *, int)
{
  return crashpad::ToolSupport::Wmain(__argc, __wargv, HandlerMainAdaptor);
}

// The default entry point for /subsystem:console. This is not currently used by
// Crashpad’s own build, but may be used by other builds.
int
wmain(int argc, wchar_t *argv[])
{
  return crashpad::ToolSupport::Wmain(argc, argv, HandlerMainAdaptor);
}
