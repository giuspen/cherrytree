#pragma once
#include "gtkmm/dialog.h"
#include <gtkmm/liststore.h>

namespace ct_dialogs {

class CtChooseDialogListStore : public Gtk::ListStore
{
public:
    struct CtChooseDialogModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<Glib::ustring> stock_id;
       Gtk::TreeModelColumn<Glib::ustring> key;
       Gtk::TreeModelColumn<Glib::ustring> desc;
       CtChooseDialogModelColumns() { add(stock_id); add(key); add(desc); }
    } columns;

public:
    static Glib::RefPtr<CtChooseDialogListStore> create();
    void add_row(const std::string& stock_id, const std::string& key, const std::string& desc);
};

Gtk::TreeModel::iterator choose_item_dialog(Gtk::Window& parent,const std::string& title,
                                            Glib::RefPtr<CtChooseDialogListStore> model,
                                            const gchar* one_column_name = nullptr);

// Dialog to select a color, featuring a palette
bool color_pick_dialog(Gtk::Window& parent, Gdk::RGBA& color);

// The Question dialog, returns True if the user presses OK
bool question_dialog(const std::string& message, Gtk::Window& parent);

// The Info dialog
void info_dialog(const std::string& message, Gtk::Window& parent);

// The Warning dialog
void warning_dialog(const std::string& message, Gtk::Window& parent);

// The Error dialog
void error_dialog(const std::string& message, Gtk::Window& parent);

} // namespace ct_dialogs
