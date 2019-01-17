#pragma once

#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <glibmm/value.h>
#include <glibmm/ustring.h>
#include "ct_menu.h"


class CtPrefDlg : public Gtk::Dialog
{
public:
    CtPrefDlg(Gtk::Window& parent, CtMenu* pCtMenu);

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

public:
    enum RESTART_REASON {MONOSPACE         = 1 << 0, EMBFILE_SIZE = 1 << 1,
                         SHOW_EMBFILE_NAME = 1 << 2, LINKS        = 1 << 3,
                         ANCHOR_SIZE       = 1 << 4, COLOR        = 1 << 5,
                         SCHEME            = 1 << 6, LANG         = 1 << 7,
                         TOOLBAR           = 1 << 8, SHORTCUT     = 1 << 9};

private:
    Glib::RefPtr<Gdk::Pixbuf> get_icon(const std::string& name);
    Gtk::Image* new_image_from_stock(const std::string& id, Gtk::IconSize size);
    bool        question_warning(const std::string& warning);
    std::string rgb_any_to_24(Gdk::RGBA color);
    std::string rgb_to_string(Gdk::RGBA color);
    void        need_restart(RESTART_REASON reason, const gchar* msg = nullptr);
    std::string choose_item_dialog(Glib::RefPtr<Gtk::ListStore> model);

private:
    std::string get_code_exec_term_run();

    void fill_commands_model(Glib::RefPtr<Gtk::ListStore> model);
    void add_new_command_in_model(Glib::RefPtr<Gtk::ListStore> model);

    void fill_toolbar_model(Glib::RefPtr<Gtk::ListStore> model);
    void add_new_item_in_toolbar_model(const std::string& key, Glib::RefPtr<Gtk::ListStore> model);
    bool add_new_item_in_toolbar_model(Glib::RefPtr<Gtk::ListStore> model);
    void update_config_toolbar_from_model(Glib::RefPtr<Gtk::ListStore> model);

    void fill_shortcut_model(Glib::RefPtr<Gtk::TreeStore> model);
    bool edit_shortcut(Gtk::TreeView* treeview);
    bool edit_shortcut_dialog(std::string& shortcut);

private:
    struct UniversalModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>  icon;
       Gtk::TreeModelColumn<Glib::ustring>              key;
       Gtk::TreeModelColumn<Glib::ustring>              desc;
       Gtk::TreeModelColumn<Glib::ustring>              shortcut;
       UniversalModelColumns() { add(icon); add(key); add(desc); add(shortcut); }
    };
    UniversalModelColumns _commandModelColumns;
    UniversalModelColumns _toolbarModelColumns;
    UniversalModelColumns _shortcutModelColumns;
    UniversalModelColumns _chooseItemColumns;
    CtMenu*               _pCtMenu;
    int                   _restartReasons;
};

