#include <gtkmm/label.h>

#include "Hig.hh"
#include "GtkUtil.hh"

HigDialog::HigDialog()
{
  set_hig_defaults();
}

HigDialog::HigDialog(const Glib::ustring& title, bool modal,
                     bool use_separator)
  : Gtk::Dialog(title, modal, use_separator)
{
  set_hig_defaults();
  vbox = NULL;
}

Gtk::VBox *
HigDialog::get_vbox()
{
  if (vbox == NULL)
    {
      vbox = manage(new Gtk::VBox());
      vbox->set_border_width(6);
      Gtk::Dialog::get_vbox()->pack_start(*vbox, true, true, 0);
    }
  return vbox;
}

void
HigDialog::set_hig_defaults()
{
  set_border_width(6);
}

HigCategoryPanel::HigCategoryPanel(Gtk::Widget &lab)
{
  init(lab);
}

HigCategoryPanel::HigCategoryPanel(const char *lab)
{
  Gtk::Label *widg = manage(GtkUtil::create_label(std::string(lab), true));
  widg->set_alignment(0.0);
  init(*widg);
}

void
HigCategoryPanel::init(Gtk::Widget &lab)
{
  size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  set_spacing(6);
  pack_start(lab, false, false, 0);

  Gtk::HBox *ibox = manage(new Gtk::HBox());
  pack_start(*ibox, false, false, 0);

  Gtk::Label *indent_lab = manage(new Gtk::Label("    "));
  ibox->pack_start(*indent_lab, false, false, 0);
  options_box = manage(new Gtk::VBox());
  ibox->pack_start(*options_box, false, false, 0);
  options_box->set_spacing(6);
}

Gtk::Label *
HigCategoryPanel::add(const char *text, Gtk::Widget &widget)
{
  Gtk::Label *lab = manage(new Gtk::Label(text));
  add(*lab, widget);
  return lab;
}

void
HigCategoryPanel::add(Gtk::Label &label, Gtk::Widget &widget)
{
  label.set_alignment(0.0);
  size_group->add_widget(label);
  Gtk::HBox *box = manage(new Gtk::HBox());
  box->set_spacing(6);
  box->pack_start(label, false, true, 0);
  box->pack_start(widget, false, false, 0);
  options_box->pack_start(*box, false, false, 0);
}

void
HigCategoryPanel::add(Gtk::Widget &widget)
{
  options_box->pack_start(widget, false, false, 0);
}

void
HigCategoryPanel::add_caption(const char *text)
{
  Gtk::Label *lab = manage(GtkUtil::create_label(std::string(text), true));
  lab->set_alignment(0.0);
  add_caption(*lab);
}

void
HigCategoryPanel::add_caption(Gtk::Widget &lab)
{
  pack_start(lab, false, false, 0);

  Gtk::HBox *ibox = manage(new Gtk::HBox());
  pack_start(*ibox, false, false, 0);

  Gtk::Label *indent_lab = manage(new Gtk::Label("    "));
  ibox->pack_start(*indent_lab, false, false, 0);
  options_box = manage(new Gtk::VBox());
  ibox->pack_start(*options_box, false, false, 0);
  options_box->set_spacing(6);
}


HigCategoriesPanel::HigCategoriesPanel()
{
  set_spacing(18);
}

void
HigCategoriesPanel::add(Gtk::Widget &panel)
{
  pack_start(panel, false, false, 0);
}


Glib::ustring
HigUtil::create_alert_text(const char *caption,
                           const char *body)
{
  Glib::ustring txt = "<span weight=\"bold\" size=\"larger\">";
  txt += caption;
  txt += "</span>";
  if (body != NULL)
    {
      txt += "\n\n";
      txt += body;
    }
  return txt;
}
