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

private:
    enum RESTART_REASON {MONOSPACE         = 1 << 0, EMBFILE_SIZE = 1 << 1,
                         SHOW_EMBFILE_NAME = 1 << 2, LINKS        = 1 << 3,
                         ANCHOR_SIZE       = 1 << 4, COLOR        = 1 << 5,
                         SCHEME            = 1 << 6, LANG         = 1 << 7,
                         TOOLBAR           = 1 << 8, SHORTCUT     = 1 << 9};

    const std::string reset_warning = std::string("<b>")+_("Are you sure to Reset to Default?")+"</b>";

private:
    Glib::RefPtr<Gdk::Pixbuf> get_icon(const std::string& name);
    bool                      user_confirm(const std::string& warning);
    void                      user_inform(const std::string& info);
    void                      need_restart(RESTART_REASON reason, const gchar* msg = nullptr);

private:
    std::string get_code_exec_term_run();

    void fill_commands_model(Glib::RefPtr<Gtk::ListStore> model);
    void add_new_command_in_model(Glib::RefPtr<Gtk::ListStore> model);

    void fill_toolbar_model(Glib::RefPtr<Gtk::ListStore> model);
    void add_new_item_in_toolbar_model(Gtk::TreeModel::iterator row, const Glib::ustring& key);
    bool add_new_item_in_toolbar_model(Gtk::TreeView* treeview, Glib::RefPtr<Gtk::ListStore> model);
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

private:
    UniversalModelColumns _commandModelColumns;
    UniversalModelColumns _toolbarModelColumns;
    UniversalModelColumns _shortcutModelColumns;
    CtMenu*               _pCtMenu;
    int                   _restartReasons;
};

