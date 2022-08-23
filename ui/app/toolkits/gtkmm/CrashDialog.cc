// Copyright (C) 2020-2021 Rob Caelers <robc@krandor.nl>
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

#include "CrashDialog.hh"
#include "gtkmm/box.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string>
#include <fstream>
#include <streambuf>
#include <filesystem>
#include <clocale>
#include <locale>

#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>

#include "base/logging.h"
#include "handler/handler_main.h"
#include "build/build_config.h"
#include "tools/tool_support.h"

#include "commonui/nls.h"
#include "utils/StringUtils.hh"

namespace
{
} // namespace

CrashDetailsDialog::CrashDetailsDialog(const std::vector<base::FilePath> &attachments)
  : Gtk::Dialog(_("Crash report details"), true)
{
  set_default_size(600, 400);
  set_title(_("crash details"));
  set_border_width(6);

  vbox = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::VERTICAL));
  vbox->set_border_width(6);
  vbox->set_spacing(6);

  get_vbox()->pack_start(*vbox, true, true, 0);

  std::string bold = "<span weight=\"bold\">";
  std::string end = "</span>";

  auto *info_label = Gtk::manage(new Gtk::Label(bold + _("Workrave has crashed.") + end, Gtk::ALIGN_START));
  info_label->set_use_markup();
  vbox->pack_start(*info_label, false, false, 0);

  if (!attachments.empty())
    {
      auto *attachments_label = Gtk::manage(
        new Gtk::Label(_("The following logging will be attached to the crash report:"), Gtk::ALIGN_START));
      vbox->pack_start(*attachments_label, false, false, 0);

      auto *attachments_frame = Gtk::manage(new Gtk::Frame);
      attachments_frame->set_shadow_type(Gtk::SHADOW_IN);
      vbox->pack_start(*attachments_frame, true, true, 0);

      Glib::RefPtr<Gtk::TextBuffer> attachments_text_buffer = Gtk::TextBuffer::create();
      Gtk::TextView *attachments_text_view = Gtk::manage(new Gtk::TextView(attachments_text_buffer));
      attachments_text_view->set_cursor_visible(false);
      attachments_text_view->set_editable(false);

      scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
      scrolled_window.add(*attachments_text_view);

      auto *attachments_box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 6));
      attachments_box->pack_start(scrolled_window, true, true, 0);

      attachments_frame->add(*attachments_box);
      Gtk::TextIter iter = attachments_text_buffer->end();

      for (const auto &a: attachments)
        {
          std::ifstream f(workrave::utils::utf16_to_utf8(a.value()).c_str());
          if (f.is_open())
            {
              iter = attachments_text_buffer->insert(iter, workrave::utils::utf16_to_utf8(a.BaseName().value()) + ":\n\n");

              std::string line;
              while (std::getline(f, line))
                {
                  iter = attachments_text_buffer->insert(iter, line + "\n");
                }
              iter = attachments_text_buffer->insert(iter, "\n");
            }
        }
    }

  auto *more_info_label = Gtk::manage(new Gtk::Label(
    _("Note that this crash report also contains technical information about the state of Workrave when it crashed."),
    Gtk::ALIGN_START));
  vbox->pack_start(*more_info_label, false, false, 0);

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
}

auto *
create_indented_box(GtkCompat::Box *container)
{
  auto *ibox = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL));
  container->pack_start(*ibox, true, true, 0);

  Gtk::Label *indent_lab = Gtk::manage(new Gtk::Label("    "));
  ibox->pack_start(*indent_lab, false, false, 10);
  auto *box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::VERTICAL));
  ibox->pack_start(*box, true, true, 0);
  box->set_spacing(6);
  return box;
}

CrashDialog::CrashDialog(const std::map<std::string, std::string> &annotations, const std::vector<base::FilePath> &attachments)
  : Gtk::Dialog(_("Crash report"), true)
  , details_dlg(new CrashDetailsDialog(attachments))
{
  set_default_size(600, 400);
  set_title(_("Workrave crash reporter"));
  set_border_width(6);

  vbox = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::VERTICAL));
  vbox->set_border_width(6);
  vbox->set_spacing(6);

  get_vbox()->pack_start(*vbox, true, true, 0);

  std::string bold = "<span weight=\"bold\">";
  std::string end = "</span>";

  auto *title_label = Gtk::manage(new Gtk::Label(bold + _("Workrave has crashed.") + end, Gtk::ALIGN_START));
  title_label->set_use_markup();
  vbox->pack_start(*title_label, false, false, 0);

  auto *info_hbox = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL));
  vbox->pack_start(*info_hbox, false, false, 0);

  auto *info_label = Gtk::manage(new Gtk::Label(
    _("Workrave encountered a problem and crashed. Please help us to diagnose and fix this problem by sending a crash report."),
    Gtk::ALIGN_START));
  info_label->set_line_wrap();
  info_label->set_xalign(0);
  info_hbox->pack_start(*info_label, false, false, 0);

  submit_cb = Gtk::manage(new Gtk::CheckButton(_("Submit crash report to the Workrave developers")));
  submit_cb->signal_toggled().connect(sigc::mem_fun(*this, &CrashDialog::on_submit_toggled));
  vbox->pack_start(*submit_cb, false, false, 0);

  auto *ibox = create_indented_box(vbox);

  auto *details_hbox = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL));
  ibox->pack_start(*details_hbox, false, false, 0);

  auto *details_btn = Gtk::manage(new Gtk::Button(_("Details...")));
  details_btn->signal_clicked().connect(sigc::mem_fun(*this, &CrashDialog::on_details_clicked));
  details_hbox->pack_start(*details_btn, false, false, 0);

  // user_text_label = Gtk::manage(new Gtk::Label(_("Please describe what was happening just before the crash:"),
  // Gtk::ALIGN_START)); ibox->pack_start(*user_text_label, false, false, 0);

  user_text_frame = Gtk::manage(new Gtk::Frame);
  user_text_frame->set_shadow_type(Gtk::SHADOW_IN);
  ibox->pack_start(*user_text_frame, true, true, 0);
  text_buffer = Gtk::TextBuffer::create();
  text_view = Gtk::manage(new Gtk::TextView(text_buffer));
  text_view->set_cursor_visible(true);
  text_view->set_editable(true);

  scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled_window.add(*text_view);

  auto *box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 6));
  box->pack_start(scrolled_window, true, true, 0);

  user_text_frame->add(*box);
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  submit_cb->set_active(true);

  details_dlg->signal_response().connect([this](int reponse) { details_dlg->hide(); });

  show_all();
  text_view->grab_focus();
}

void
CrashDialog::on_submit_toggled()
{
  auto enabled = submit_cb->get_active();

  user_text_frame->set_sensitive(enabled);
  // user_text_label->set_sensitive(enabled);
}

void
CrashDialog::on_details_clicked()
{
  details_dlg->show_all();
  details_dlg->present();
}

std::string
CrashDialog::get_user_text() const
{
  return text_buffer->get_text();
}

bool
CrashDialog::get_consent() const
{
  return submit_cb->get_active();
}

bool
UserInteraction::requestUserConsent(const std::map<std::string, std::string> &annotations,
                                    const std::vector<base::FilePath> &attachments)
{
  SetEnvironmentVariableA("GTK_DEBUG", 0);
  SetEnvironmentVariableA("G_MESSAGES_DEBUG", 0);
  // No auto hide scrollbars
  SetEnvironmentVariableA("GTK_OVERLAY_SCROLLING", "0");
  // No Windows-7 style client-side decorations on Windows 10...
  SetEnvironmentVariableA("GTK_CSD", "0");
  SetEnvironmentVariableA("GDK_WIN32_DISABLE_HIDPI", "1");

  LOG(INFO) << "Creating user consent app.";
  auto app = Gtk::Application::create();
  app->register_application();

  LOG(INFO) << "Creating user consent dialog.";
  auto dlg = new CrashDialog(annotations, attachments);
  dlg->signal_response().connect([this, app, dlg](int response) {
    LOG(INFO) << "User response: " << response;
    user_text = dlg->get_user_text();
    LOG(INFO) << "User text: " << user_text;
    consent = dlg->get_consent();
    app->quit();
  });
  LOG(INFO) << "Showing user consent dialog.";
  app->hold();
  dlg->show();
  LOG(INFO) << "Running main loop.";
  app->run();
  dlg->hide();
  LOG(INFO) << "User consent complete:" << consent;
  return consent;
}

std::string
UserInteraction::getUserText()
{
  return user_text;
}
void
UserInteraction::reportCompleted(const crashpad::UUID &uuid)
{
  LOG(INFO) << "Report files as: " << uuid.ToString();
}

namespace
{
  int HandlerMainAdaptor(int argc, char *argv[])
  {
    LOG(INFO) << "Workrave crashed.";
    auto *user_interaction = new UserInteraction;
    int ret = crashpad::HandlerMain(argc, argv, nullptr, user_interaction);
    LOG(INFO) << "Crash handled";
    delete user_interaction;
    LOG(INFO) << "Exit:" << ret;
    return ret;
  }
} // namespace

int APIENTRY
wWinMain(HINSTANCE, HINSTANCE, wchar_t *, int)
{
  return crashpad::ToolSupport::Wmain(__argc, __wargv, HandlerMainAdaptor);
}

int
wmain(int argc, wchar_t *argv[])
{
  return crashpad::ToolSupport::Wmain(argc, argv, HandlerMainAdaptor);
}
