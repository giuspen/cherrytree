#pragma once

#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <glibmm/value.h>
#include <glibmm/ustring.h>

class CtPrefDlg : public Gtk::Dialog
{
public:
    CtPrefDlg(Gtk::Window& parent);

private:
    Gtk::Widget* build_tab_text_n_code();
    Gtk::Widget* build_tab_text();
    Gtk::Widget* build_tab_rich_text();
    Gtk::Widget* build_tab_plain_text_n_code();
    Gtk::Widget* build_tab_tree_1();
    Gtk::Widget* build_tab_tree_2();
    Gtk::Widget* build_tab_fonts();
    Gtk::Widget* build_tab_links();
    Gtk::Widget* build_tab_toolbar();
    Gtk::Widget* build_tab_kb_shortcuts();
    Gtk::Widget* build_tab_misc();

private:
    Gtk::Image* new_image_from_stock(const std::string& id, Gtk::IconSize size);
    bool question_warning(const std::string& warning) { return true; }
};

