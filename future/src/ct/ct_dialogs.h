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

bool color_pick_dialog(Gtk::Window& parent, Gdk::RGBA& color);

} // namespace ct_dialogs
