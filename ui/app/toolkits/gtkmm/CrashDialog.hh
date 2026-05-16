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

#ifndef CRASH_DIALOG_HH
#define CRASH_DIALOG_HH

#include <string>

#include <gtkmm.h>

#include "handler/user_hook.h"

namespace Gtk
{
  class TextView;
}

class CrashDetailsDialog : public Gtk::Dialog
{
public:
  CrashDetailsDialog(const std::vector<base::FilePath> &attachments,
                     const crashpad::CrashSummary &summary);
  ~CrashDetailsDialog() override = default;

  std::vector<base::FilePath> get_enabled_attachments() const;

private:
  void on_attachment_toggled(const Glib::ustring &path_str);
  void on_selection_changed();
  void load_content(int index);
  void display_crash_info();

  struct AttachmentEntry
  {
    base::FilePath path;
    bool enabled{true};
  };

  struct AttachmentColumns : Gtk::TreeModelColumnRecord
  {
    AttachmentColumns()
    {
      add(enabled);
      add(name);
      add(type);
      add(content);
      add(index);
    }
    Gtk::TreeModelColumn<bool> enabled;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<int> type;              // 0=crash info, 1=stack trace, 2=file attachment
    Gtk::TreeModelColumn<Glib::ustring> content; // pre-built text for type 0 and 1
    Gtk::TreeModelColumn<int> index;             // file index for type 2, -1 otherwise
  };

  AttachmentColumns columns_;
  Glib::RefPtr<Gtk::ListStore> list_store_;
  Gtk::TreeView *tree_view_{nullptr};
  Glib::RefPtr<Gtk::TextBuffer> content_buffer_;
  Glib::RefPtr<Gtk::TextTag> bold_tag_;
  std::vector<AttachmentEntry> entries_;
  crashpad::CrashSummary summary_;
  Gtk::VBox *vbox{nullptr};
};

class CrashDialog : public Gtk::Dialog
{
public:
  CrashDialog(const std::map<std::string, std::string> &annotations,
              const std::vector<base::FilePath> &attachments,
              const crashpad::CrashSummary &summary);
  ~CrashDialog() override = default;

  std::string get_user_text() const;
  bool get_consent() const;
  std::vector<base::FilePath> get_selected_attachments() const;

private:
  void on_submit_toggled();
  void on_details_clicked();

private:
  Gtk::TextView *text_view{nullptr};
  Gtk::VBox *vbox{nullptr};
  Gtk::ScrolledWindow scrolled_window;
  Glib::RefPtr<Gtk::TextBuffer> text_buffer;
  CrashDetailsDialog *details_dlg{nullptr};
  Gtk::CheckButton *submit_cb{nullptr};
  Gtk::Frame *user_text_frame{nullptr};
};

class UserInteraction : public crashpad::UserHook
{
public:
  UserInteraction() = default;
  ~UserInteraction() override = default;

  bool requestUserConsent(const std::map<std::string, std::string> &annotations,
                          std::vector<base::FilePath> &attachments,
                          const crashpad::CrashSummary &summary) override;
  std::string getUserText() override;
  void reportCompleted(const crashpad::UUID &uuid) override;

private:
  std::string user_text;
  bool consent{false};
};

#endif // CRASH_DIALOG_HH
