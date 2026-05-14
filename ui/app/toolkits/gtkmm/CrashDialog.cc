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

#include <iomanip>
#include <sstream>
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
  std::string format_hex(uint64_t value)
  {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << value;
    return oss.str();
  }

  std::string format_exception(const crashpad::CrashSummary &summary)
  {
    if (!summary.exception_name.empty())
      return summary.exception_name + "  (" + format_hex(summary.exception_code) + ")";
    return format_hex(summary.exception_code);
  }

  std::string format_address(const crashpad::CrashSummary &summary)
  {
    std::string s = format_hex(summary.exception_address);
    if (!summary.module_name.empty())
      s += "  (" + summary.module_name + " + " + format_hex(summary.module_offset) + ")";
    return s;
  }

  std::string format_thread(const crashpad::CrashSummary &summary)
  {
    std::string tid = "tid: " + std::to_string(summary.crashing_thread_id);
    if (!summary.crashing_thread_name.empty())
      return summary.crashing_thread_name + "  (" + tid + ")";
    return tid;
  }
} // namespace

CrashDetailsDialog::CrashDetailsDialog(const std::vector<base::FilePath> &attachments,
                                       const crashpad::CrashSummary &summary)
  : Gtk::Dialog(_("Crash report details"), false)
{
  set_default_size(950, 620);
  set_title(_("Crash details"));

  set_border_width(6);

  vbox = Gtk::manage(new Gtk::VBox());
  vbox->set_border_width(6);
  vbox->set_spacing(6);

  get_vbox()->pack_start(*vbox, true, true, 0);

  summary_ = summary;

  for (const auto &p : attachments)
    {
      entries_.push_back({p, true});
    }

  // Single HPaned fills the whole dialog
  auto *paned = Gtk::manage(new Gtk::HPaned());
  vbox->pack_start(*paned, true, true, 0);

  // Left panel: tree listing crash info, stack trace, and file attachments
  list_store_ = Gtk::ListStore::create(columns_);
  tree_view_ = Gtk::manage(new Gtk::TreeView(list_store_));
  tree_view_->set_headers_visible(false);

  // Toggle column — visible only for file attachment rows
  auto *toggle_renderer = Gtk::manage(new Gtk::CellRendererToggle());
  toggle_renderer->signal_toggled().connect(
    sigc::mem_fun(*this, &CrashDetailsDialog::on_attachment_toggled));
  auto *toggle_col = Gtk::manage(new Gtk::TreeViewColumn("", *toggle_renderer));
  toggle_col->set_cell_data_func(*toggle_renderer,
    [this](Gtk::CellRenderer *r, const Gtk::TreeModel::iterator &it)
    {
      auto *tr = static_cast<Gtk::CellRendererToggle *>(r);
      bool is_file = ((*it)[columns_.type] == 2);
      tr->property_visible() = is_file;
      tr->property_active() = is_file && static_cast<bool>((*it)[columns_.enabled]);
    });
  tree_view_->append_column(*toggle_col);

  tree_view_->append_column("", columns_.name);
  if (auto *col = tree_view_->get_column(1))
    col->set_expand(true);

  // Crash Info row (content is built dynamically with formatting in display_crash_info)
  {
    auto row = *(list_store_->append());
    row[columns_.enabled] = false;
    row[columns_.name] = _("Crash Info");
    row[columns_.type] = 0;
    row[columns_.content] = "";
    row[columns_.index] = -1;
  }

  // File attachment rows
  for (int i = 0; i < static_cast<int>(entries_.size()); ++i)
    {
      auto row = *(list_store_->append());
      row[columns_.enabled] = entries_[i].enabled;
      row[columns_.name] = workrave::utils::utf16_to_utf8(entries_[i].path.BaseName().value());
      row[columns_.type] = 2;
      row[columns_.content] = "";
      row[columns_.index] = i;
    }

  tree_view_->get_selection()->signal_changed().connect(
    sigc::mem_fun(*this, &CrashDetailsDialog::on_selection_changed));

  auto *list_sw = Gtk::manage(new Gtk::ScrolledWindow());
  list_sw->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  list_sw->add(*tree_view_);
  paned->add1(*list_sw);

  // Right panel: content viewer
  content_buffer_ = Gtk::TextBuffer::create();
  bold_tag_ = content_buffer_->create_tag("bold");
  bold_tag_->property_weight() = Pango::WEIGHT_BOLD;
  auto *content_view = Gtk::manage(new Gtk::TextView(content_buffer_));
  content_view->set_cursor_visible(false);
  content_view->set_editable(false);
  content_view->override_font(Pango::FontDescription("Monospace 9"));
  content_view->set_left_margin(8);
  content_view->set_right_margin(8);
  content_view->set_top_margin(6);
  content_view->set_bottom_margin(6);

  auto *content_sw = Gtk::manage(new Gtk::ScrolledWindow());
  content_sw->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  content_sw->add(*content_view);
  paned->add2(*content_sw);

  paned->set_position(200);

  // Pre-select Crash Info row
  tree_view_->get_selection()->select(list_store_->children().begin());

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
}

void
CrashDetailsDialog::on_attachment_toggled(const Glib::ustring &path_str)
{
  auto it = list_store_->get_iter(Gtk::TreeModel::Path(path_str));
  if (it && (*it)[columns_.type] == 2)
    {
      int idx = (*it)[columns_.index];
      bool current = (*it)[columns_.enabled];
      (*it)[columns_.enabled] = !current;
      entries_[idx].enabled = !current;
    }
}

void
CrashDetailsDialog::on_selection_changed()
{
  auto sel = tree_view_->get_selection()->get_selected();
  if (!sel)
    return;

  int type = (*sel)[columns_.type];
  if (type == 2)
    {
      load_content((*sel)[columns_.index]);
    }
  else
    {
      display_crash_info();
    }
}

void
CrashDetailsDialog::display_crash_info()
{
  content_buffer_->set_text("");

  auto append_kv = [this](const std::string &key, const std::string &value)
  {
    auto iter = content_buffer_->end();
    content_buffer_->insert_with_tag(iter, key, bold_tag_);
    iter = content_buffer_->end();
    content_buffer_->insert(iter, "  " + value + "\n");
  };

  append_kv("Exception:", format_exception(summary_));
  append_kv("Address:  ", format_address(summary_));
  append_kv("Thread:   ", format_thread(summary_));

  // Stack trace
  {
    auto iter = content_buffer_->end();
    content_buffer_->insert(iter, "\n");
    iter = content_buffer_->end();
    content_buffer_->insert_with_tag(iter, "Stack Trace:\n", bold_tag_);
    if (summary_.stack_frames.empty())
      {
        iter = content_buffer_->end();
        content_buffer_->insert(iter, "(not available)\n");
      }
    else
      {
        int frame_num = 0;
        for (const auto &[addr, sym] : summary_.stack_frames)
          {
            std::ostringstream line;
            line << "#" << std::setw(2) << std::left << frame_num++ << "  " << format_hex(addr);
            if (!sym.empty())
              line << "  " << sym;
            line << "\n";
            iter = content_buffer_->end();
            content_buffer_->insert(iter, line.str());
          }
      }
  }
}

void
CrashDetailsDialog::load_content(int index)
{
  content_buffer_->set_text("");
  const auto &path = entries_[index].path;
  std::ifstream f(workrave::utils::utf16_to_utf8(path.value()).c_str());
  if (f.is_open())
    {
      Gtk::TextIter iter = content_buffer_->end();
      std::string line;
      while (std::getline(f, line))
        {
          iter = content_buffer_->insert(iter, line + "\n");
        }
    }
  else
    {
      content_buffer_->set_text(_("(file not found or not readable)"));
    }
}

std::vector<base::FilePath>
CrashDetailsDialog::get_enabled_attachments() const
{
  std::vector<base::FilePath> result;
  for (const auto &entry : entries_)
    {
      if (entry.enabled)
        result.push_back(entry.path);
    }
  return result;
}

Gtk::VBox *
create_indented_box(Gtk::Box *container)
{
  Gtk::HBox *ibox = Gtk::manage(new Gtk::HBox());
  container->pack_start(*ibox, true, true, 0);

  Gtk::Label *indent_lab = Gtk::manage(new Gtk::Label("    "));
  ibox->pack_start(*indent_lab, false, false, 10);
  auto *box = Gtk::manage(new Gtk::VBox());
  ibox->pack_start(*box, true, true, 0);
  box->set_spacing(6);
  return box;
}

CrashDialog::CrashDialog(const std::map<std::string, std::string> &annotations,
                         const std::vector<base::FilePath> &attachments,
                         const crashpad::CrashSummary &summary)
  : Gtk::Dialog(_("Crash report"), false)
  , details_dlg(new CrashDetailsDialog(attachments, summary))
{
  details_dlg->set_transient_for(*this);
  set_default_size(600, 420);
  set_title(_("Workrave crash reporter"));
  set_border_width(0);

  vbox = Gtk::manage(new Gtk::VBox());
  vbox->set_border_width(0);
  vbox->set_spacing(0);
  get_vbox()->pack_start(*vbox, true, true, 0);

  // Colored crash header — override_background_color on every widget so the
  // Windows GTK theme cannot repaint over our custom color.
  const Gdk::RGBA header_color("#fde8e8");
  auto *header_eb = Gtk::manage(new Gtk::EventBox());
  header_eb->override_background_color(header_color, Gtk::STATE_FLAG_NORMAL);
  auto *header_box = Gtk::manage(new Gtk::VBox(false, 6));
  header_box->set_border_width(12);
  header_box->override_background_color(header_color, Gtk::STATE_FLAG_NORMAL);
  header_eb->add(*header_box);
  vbox->pack_start(*header_eb, false, false, 0);

  const Gdk::RGBA header_fg("#1a1a1a");

  auto *title_label = Gtk::manage(new Gtk::Label());
  title_label->set_markup("<span weight=\"bold\" size=\"large\">Workrave has crashed.</span>");
  title_label->set_xalign(0);
  title_label->override_background_color(header_color, Gtk::STATE_FLAG_NORMAL);
  title_label->override_color(header_fg, Gtk::STATE_FLAG_NORMAL);
  header_box->pack_start(*title_label, false, false, 0);

  auto *info_label = Gtk::manage(new Gtk::Label(
    _("Workrave encountered a problem and crashed. Please help us diagnose and fix this problem by sending a crash report."),
    Gtk::ALIGN_START));
  info_label->set_line_wrap();
  info_label->set_xalign(0);
  info_label->override_background_color(header_color, Gtk::STATE_FLAG_NORMAL);
  info_label->override_color(header_fg, Gtk::STATE_FLAG_NORMAL);
  header_box->pack_start(*info_label, false, false, 0);

  // Content area
  auto *content_box = Gtk::manage(new Gtk::VBox(false, 8));
  content_box->set_border_width(12);
  vbox->pack_start(*content_box, true, true, 0);

  submit_cb = Gtk::manage(new Gtk::CheckButton(_("Submit crash report to the Workrave developers")));
  submit_cb->signal_toggled().connect(sigc::mem_fun(*this, &CrashDialog::on_submit_toggled));
  content_box->pack_start(*submit_cb, false, false, 0);

  Gtk::VBox *ibox = create_indented_box(content_box);

  // Details button (left-aligned) with hint below it
  auto *details_btn_row = Gtk::manage(new Gtk::HBox(false, 0));
  ibox->pack_start(*details_btn_row, false, false, 0);
  auto *details_btn = Gtk::manage(new Gtk::Button(_("Details...")));
  details_btn->signal_clicked().connect(sigc::mem_fun(*this, &CrashDialog::on_details_clicked));
  details_btn_row->pack_start(*details_btn, false, false, 0);

  auto *details_hint = Gtk::manage(new Gtk::Label());
  details_hint->set_markup(_("<small><i>Shows the crash information and files that will be submitted.</i></small>"));
  details_hint->set_xalign(0);
  ibox->pack_start(*details_hint, false, false, 0);

  auto *sep = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
  ibox->pack_start(*sep, false, false, 2);

  auto *user_text_label = Gtk::manage(new Gtk::Label(_("Additional comments (optional):"), Gtk::ALIGN_START));
  user_text_label->set_xalign(0);
  ibox->pack_start(*user_text_label, false, false, 0);

  user_text_frame = Gtk::manage(new Gtk::Frame);
  user_text_frame->set_shadow_type(Gtk::SHADOW_IN);
  ibox->pack_start(*user_text_frame, true, true, 0);
  text_buffer = Gtk::TextBuffer::create();
  text_view = Gtk::manage(new Gtk::TextView(text_buffer));
  text_view->set_cursor_visible(true);
  text_view->set_editable(true);
  text_view->set_left_margin(4);
  text_view->set_right_margin(4);
  text_view->set_top_margin(4);

  scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled_window.add(*text_view);

  Gtk::HBox *box = Gtk::manage(new Gtk::HBox(false, 6));
  box->pack_start(scrolled_window, true, true, 0);
  user_text_frame->add(*box);

  get_action_area()->set_border_width(8);
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  submit_cb->set_active(true);
  details_dlg->signal_response().connect([this](int response) { details_dlg->hide(); });

  show_all();
  text_view->grab_focus();
}

void
CrashDialog::on_submit_toggled()
{
  auto enabled = submit_cb->get_active();

  user_text_frame->set_sensitive(enabled);
}

void
CrashDialog::on_details_clicked()
{
  details_dlg->show_all();

  // Manually center over the main window (WIN_POS_CENTER_ON_PARENT is unreliable on Windows)
  int px = 0, py = 0, pw = 0, ph = 0;
  get_position(px, py);
  get_size(pw, ph);
  int dw = 0, dh = 0;
  details_dlg->get_size(dw, dh);
  details_dlg->move(px + (pw - dw) / 2, py + (ph - dh) / 2);

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

std::vector<base::FilePath>
CrashDialog::get_selected_attachments() const
{
  return details_dlg->get_enabled_attachments();
}

bool
UserInteraction::requestUserConsent(const std::map<std::string, std::string> &annotations,
                                    std::vector<base::FilePath> &attachments,
                                    const crashpad::CrashSummary &summary)
{
  //SetEnvironmentVariableA("GTK_DEBUG", 0);
  //SetEnvironmentVariableA("G_MESSAGES_DEBUG", 0);
  // No auto hide scrollbars
  SetEnvironmentVariableA("GTK_OVERLAY_SCROLLING", "0");
  // No Windows-7 style client-side decorations on Windows 10...
  SetEnvironmentVariableA("GTK_CSD", "0");
  SetEnvironmentVariableA("GDK_WIN32_DISABLE_HIDPI", "1");

  LOG(INFO) << "Creating user consent app.";
  auto app = Gtk::Application::create();
  app->register_application();

  LOG(INFO) << "Creating user consent dialog.";
  auto dlg = new CrashDialog(annotations, attachments, summary);
  dlg->signal_response().connect([this, app, dlg](int response) {
    LOG(INFO) << "User response: " << response;
    user_text = dlg->get_user_text();
    LOG(INFO) << "User text: " << user_text;
    consent = dlg->get_consent();
    app->quit();
  });
  LOG(INFO) << "Showing user consent dialog.";
  app->hold();
 // dlg->set_keep_above(true);
  dlg->show();
  dlg->present();
  LOG(INFO) << "Running main loop.";
  app->run();
  attachments = dlg->get_selected_attachments();
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
