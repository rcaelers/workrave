// Copyright (C) 2022 Rob Caelers <rob.caelers@gmail.com>
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

#define WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "AutoUpdateDialog.hh"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <gtkmm.h>

#include <cmath>
#include <utility>

#include "GtkUtil.hh"
#include "ui/GUIConfig.hh"
#include "commonui/nls.h"

#include "unfold/Unfold.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include "cmark.h"
#  include "Edge.hh"
#  include "ToolkitWindows.hh"

static constexpr const char *doc =
  R"(<!DOCTYPE html>
<html ang="en">
<head>
  <meta charset="utf-8">
{}
</head>
<body>
  <div>
    {}
  </div>
</body>
</html>)";
#endif

static constexpr const char *lightstyle =
  R"(<style type="text/css">
body {
  color: #222;
  background: #fff;
}
a {
  color: #224ba0;
}
</style>
)";

static constexpr const char *darkstyle =
  R"(<style type="text/css">
body {
  color: #eee;
  background: #2d2d2d;
}
body a {
  color: #224ba0;
}
</style>
)";

AutoUpdateDialog::AutoUpdateDialog(std::shared_ptr<unfold::UpdateInfo> info, AutoUpdateDialog::update_choice_callback_t callback)
  : Gtk::Window(Gtk::WINDOW_TOPLEVEL)
  , callback(std::move(callback))
{
  set_default_size(800, 600);
  set_border_width(6);
  set_title(_("Software Update"));

  auto *box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
  add(*box);
  auto *content_area = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  content_area->set_border_width(6);
  content_area->set_spacing(6);
  box->pack_start(*content_area, true, true, 0);

  auto *logo_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
  logo_box->set_border_width(6);
  logo_box->set_spacing(6);
  content_area->pack_start(*logo_box, false, false, 0);

  try
    {
      auto pix = Gdk::Pixbuf::create_from_resource("/workrave/workrave.png");
      Gtk::Image *logo = Gtk::manage(new Gtk::Image(pix));
      logo_box->pack_start(*logo, false, false, 0);
    }
#if GLIBMM_CHECK_VERSION(2, 68, 0)
  catch (std::exception &e)
    {
      spdlog::info("error loading image {}", std::string(e.what()));
    }
#else
  catch (const Glib::Exception &e)
    {
      spdlog::info("error loading image {}", std::string(e.what()));
    }
#endif

  auto *update_info_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
  update_info_box->set_border_width(6);
  update_info_box->set_spacing(10);
  content_area->pack_start(*update_info_box, true, true, 0);

  std::string bold = "<span weight=\"bold\">";
  std::string end = "</span>";

  auto *title_label = Gtk::manage(
    new Gtk::Label(bold + fmt::format(fmt::runtime(_("A new version of {} is available")), info->title) + end, Gtk::ALIGN_START));
  title_label->set_use_markup();
  update_info_box->pack_start(*title_label, false, false, 0);

  auto *info_hbox = Gtk::manage(new Gtk::HBox());
  update_info_box->pack_start(*info_hbox, false, false, 0);

  auto *info_label = Gtk::manage(
    new Gtk::Label(fmt::format(fmt::runtime(_("{} {} is now available -- you have {}. Would you like to download it now?")),
                               info->title,
                               info->version,
                               info->current_version),
                   Gtk::ALIGN_START));
  info_label->set_line_wrap();
  info_label->set_xalign(0);
  info_hbox->pack_start(*info_label, false, false, 0);

  auto *notes_label = Gtk::manage(new Gtk::Label(bold + _("Release notes") + end, Gtk::ALIGN_START));
  notes_label->set_use_markup();
  update_info_box->pack_start(*notes_label, false, false, 0);

  auto *notes_frame = Gtk::manage(new Gtk::Frame);
  notes_frame->set_shadow_type(Gtk::SHADOW_IN);
  update_info_box->pack_start(*notes_frame, true, true, 0);

#if defined(PLATFORM_OS_WINDOWS)
  if (Edge::is_supported())
    {
      web = Gtk::manage(new Edge);

      std::string body;
      for (auto note: info->release_notes)
        {
          body += fmt::format(fmt::runtime(_("<h3>Version {}</h3>\n")), note.version);
          auto *html = cmark_markdown_to_html(note.markdown.c_str(), note.markdown.length(), CMARK_OPT_DEFAULT);

          if (html != nullptr)
            {
              spdlog::info("body: {}", html);
              body += html;
              free(html);
            }
        }

      bool dark = false;
      switch (GUIConfig::light_dark_mode()())
        {
        case LightDarkTheme::Light:
          dark = false;
          break;
        case LightDarkTheme::Dark:
          dark = true;
          break;
        case LightDarkTheme::Auto:
          dark = ToolkitWindows::is_windows_app_theme_dark();
          break;
        }

      web->set_content(fmt::format(doc, dark ? darkstyle : lightstyle, body));
      notes_frame->add(*web);
    }
  else
#endif
    {
      text_buffer = Gtk::TextBuffer::create();
      text_view = Gtk::manage(new Gtk::TextView(text_buffer));
      text_view->set_cursor_visible(false);
      text_view->set_editable(false);

      scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
      scrolled_window.add(*text_view);

      Gtk::HBox *scrolled_box = Gtk::manage(new Gtk::HBox(false, 6));
      scrolled_box->pack_start(scrolled_window, true, true, 0);

      notes_frame->add(*scrolled_box);
      Gtk::TextIter iter = text_buffer->end();

      for (auto note: info->release_notes)
        {
          auto line = fmt::format(fmt::runtime(_("Version {}\n")), note.version);

          iter = text_buffer->insert(iter, line);
          iter = text_buffer->insert(iter, note.markdown + "\n\n");
        }
    }

  auto *status_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  status_box->set_border_width(6);
  status_box->set_spacing(10);
  box->pack_start(*status_box, false, true, 0);

  status_label = Gtk::manage(new Gtk::Label());
  status_label->set_use_markup();
  status_label->set_yalign(0.5);
  status_box->pack_start(*status_label, false, false, 6);

  progress_bar_frame = Gtk::manage(new Gtk::Frame);
  progress_bar_frame->set_shadow_type(Gtk::SHADOW_IN);

  progress_bar = Gtk::manage(new Gtk::ProgressBar());
  progress_bar->set_fraction(0);
  progress_bar->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  progress_bar_frame->add(*progress_bar);

  std::string css_string = R"(
        * {
            min-height: 2em;
        }
    )";
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  Glib::RefPtr<Gtk::StyleContext> style_context = progress_bar->get_style_context();
  css_provider->load_from_data(css_string);
  style_context->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  status_box->pack_start(*progress_bar_frame, true, true, 6);

  auto *bottom_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  box->pack_start(*bottom_box, Gtk::PACK_SHRINK, 0);

  left_button_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  bottom_box->pack_start(*left_button_box, Gtk::PACK_SHRINK, 6);
  right_button_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  bottom_box->pack_end(*right_button_box, Gtk::PACK_SHRINK, 6);
  close_button_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  bottom_box->pack_end(*close_button_box, Gtk::PACK_SHRINK, 6);

  auto *skip_button = Gtk::manage(new Gtk::Button(_("_Skip this version")));
  skip_button->signal_clicked().connect([callback]() { callback(UpdateChoice::Skip); });
  skip_button->set_use_underline();

  left_button_box->pack_end(*skip_button, Gtk::PACK_EXPAND_WIDGET, 6);

  auto *remind_button = Gtk::manage(new Gtk::Button(_("_Remind me later")));
  remind_button->signal_clicked().connect([callback]() { callback(UpdateChoice::Later); });
  remind_button->set_use_underline();
  auto *install_button = Gtk::manage(new Gtk::Button(_("_Install update")));
  install_button->signal_clicked().connect([callback]() { callback(UpdateChoice::Now); });
  install_button->set_use_underline();

  right_button_box->pack_start(*remind_button, Gtk::PACK_SHRINK, 6);
  right_button_box->pack_start(*install_button, Gtk::PACK_SHRINK, 6);

  auto *close_button = Gtk::manage(new Gtk::Button(_("_Close")));
  close_button->signal_clicked().connect([this]() { hide(); });
  close_button->set_use_underline();

  close_button_box->pack_start(*close_button, Gtk::PACK_SHRINK, 6);

  signal_hide().connect([callback]() { callback(UpdateChoice::Later); });

  install_button->set_sensitive();
  show_all();
  progress_bar->set_visible(false);
  progress_bar_frame->set_visible(false);
  close_button_box->set_visible(false);
}

void
AutoUpdateDialog::set_progress_visible(bool visible)
{
  progress_bar->set_visible(visible);
  progress_bar_frame->set_visible(true);
}

void
AutoUpdateDialog::set_stage(unfold::UpdateStage stage, double progress)
{
  if (!current_stage || *current_stage != stage)
    {
      spdlog::info("Update stage: {}", static_cast<int>(stage));
      current_stage = stage;
      progress_bar->set_visible(stage == unfold::UpdateStage::DownloadInstaller);
      progress_bar_frame->set_visible(stage == unfold::UpdateStage::DownloadInstaller);

      switch (stage)
        {
        case unfold::UpdateStage::DownloadInstaller:
          status_label->set_text(_("Downloading installer"));
          break;
        case unfold::UpdateStage::VerifyInstaller:
          status_label->set_text(_("Verifying installer"));
          break;
        case unfold::UpdateStage::RunInstaller:
          status_label->set_text(_("Running installer"));
          break;
        default:
          break;
        }
    }

  if (fabs(progress - progress_bar->get_fraction()) >= 0.01)
    {
      progress_bar->set_fraction(progress);
    }

  if (progress >= 1.0)
    {
      progress_bar->set_visible(false);
      progress_bar_frame->set_visible(false);
    }
}

void
AutoUpdateDialog::set_status(std::string status)
{
  spdlog::info("Update status: {}", status);
  status_label->set_text(status);
  progress_bar->set_visible(false);
  progress_bar_frame->set_visible(false);
}

void
AutoUpdateDialog::start_install()
{
  status_label->set_text("");
  left_button_box->set_visible(false);
  right_button_box->set_visible(false);
  progress_bar->set_visible(true);
  progress_bar_frame->set_visible(true);
  close_button_box->set_visible(true);
}
