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
}

void
HigDialog::set_hig_defaults()
{
  set_border_width(12);
}

HigCategoryPanel::HigCategoryPanel(const char *lab)
{
  size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  set_spacing(6);
  Gtk::Label *category = manage(GtkUtil::create_label(string(lab), true));
  category->set_alignment(0.0);
  pack_start(*category, false, false, 0);

  Gtk::HBox *ibox = manage(new Gtk::HBox());
  pack_start(*ibox, false, false, 0);
  
  Gtk::Label *indent_lab = manage(new Gtk::Label("    "));
  ibox->pack_start(*indent_lab, false, false, 0);
  options_box = manage(new Gtk::VBox());
  ibox->pack_start(*options_box, false, false, 0);
  options_box->set_spacing(6);
}

void
HigCategoryPanel::add(const char *text, Gtk::Widget &widget)
{
  Gtk::Label *lab = manage(new Gtk::Label(text));
  lab->set_alignment(0.0);
  size_group->add_widget(*lab);
  Gtk::HBox *box = manage(new Gtk::HBox());
  box->set_spacing(6);
  box->pack_start(*lab, false, true, 0);
  box->pack_start(widget, false, false, 0);
  options_box->pack_start(*box, false, false, 0);
}
void
HigCategoryPanel::add(Gtk::Widget &widget)
{
  options_box->pack_start(widget, false, false, 0);
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


