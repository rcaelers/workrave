// NetworkJoinDialog.cc --- NetworkJoin dialog
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2011 Rob Caelers & Raymond Penners
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

#ifdef HAVE_DISTRIBUTION

#  include <cassert>

#  include <gtkmm/image.h>
#  include <gtkmm/label.h>
#  include <gtkmm/alignment.h>
#  include <gtkmm/entry.h>
#  include <gtkmm/spinbutton.h>
#  include <gtkmm/checkbutton.h>
#  include <gtkmm/stock.h>

#  include "commonui/nls.h"
#  include "debug.hh"

#  include "NetworkJoinDialog.hh"

#  include "core/IDistributionManager.hh"
#  include "config/IConfigurator.hh"
#  include "GtkUtil.hh"

using namespace workrave;

NetworkJoinDialog::NetworkJoinDialog(std::shared_ptr<IApplication> app)
  : HigDialog(_("Network connect"), true, false)
  , app(app)
{
  TRACE_ENTRY();
  auto core = app->get_core();
  IDistributionManager *dist_manager = core->get_distribution_manager();

  // Icon
  Gtk::Image *title_img = GtkUtil::create_image("network.png");
  Gtk::Alignment *img_aln = Gtk::manage(new Gtk::Alignment(Gtk::ALIGN_START, Gtk::ALIGN_END, 0.0, 0.0));

  img_aln->add(*title_img);

  Gtk::Label *title_lab = Gtk::manage(new Gtk::Label());
  Glib::ustring text = HigUtil::create_alert_text(_("Network connect"),
                                                  _("Enter the host name and port number of a computer\n"
                                                    "in the network you wish to connect to."));
  title_lab->set_markup(text);

  host_entry.set_width_chars(40);

  port_entry.set_range(1024, 65535);
  port_entry.set_increments(1, 10);
  port_entry.set_numeric(true);
  port_entry.set_width_chars(10);
  port_entry.set_value(dist_manager->get_port());

  Gtk::Label *host_lab = Gtk::manage(new Gtk::Label(_("Host name:")));
  Gtk::Label *port_lab = Gtk::manage(new Gtk::Label(_("Port:")));

  // Table
  Gtk::Table *table = Gtk::manage(new Gtk::Table(4, 2, false));
  table->set_spacings(6);
  title_lab->set_alignment(0.0);
  table->attach(*title_lab, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 0, 6);
  GtkUtil::table_attach_left_aligned(*table, *host_lab, 0, 1);
  GtkUtil::table_attach_left_aligned(*table, host_entry, 1, 1);
  GtkUtil::table_attach_left_aligned(*table, *port_lab, 0, 2);
  GtkUtil::table_attach_left_aligned(*table, port_entry, 1, 2);

  // Page
  Gtk::HBox *page = Gtk::manage(new Gtk::HBox(false, 12));
  page->pack_start(*img_aln, false, true, 0);
  page->pack_start(*table, false, true, 0);

  get_vbox()->pack_start(*page, false, false, 0);

  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

  show_all();
}

//! Destructor.
NetworkJoinDialog::~NetworkJoinDialog()
{
  TRACE_ENTRY();
}

std::string
NetworkJoinDialog::get_connect_url()
{
  std::string peer = "tcp://" + host_entry.get_text() + ":" + port_entry.get_text();
  return peer;
}

#endif
