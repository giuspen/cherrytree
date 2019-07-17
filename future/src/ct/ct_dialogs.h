/*
 * ct_dialogs.h
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#pragma once

#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

class CtMainWin;
class CtTreeStore;
namespace ct_dialogs {

template<class GtkStoreBase>
class CtChooseDialogStore : public GtkStoreBase
{
public:
    struct CtChooseDialogModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<Glib::ustring> stock_id;
       Gtk::TreeModelColumn<Glib::ustring> key;
       Gtk::TreeModelColumn<Glib::ustring> desc;
       Gtk::TreeModelColumn<gint64>        node_id;

       CtChooseDialogModelColumns() { add(stock_id); add(key); add(desc); add(node_id); }
    } columns;

public:
    static Glib::RefPtr<CtChooseDialogStore<GtkStoreBase>> create()
    {
        Glib::RefPtr<CtChooseDialogStore<GtkStoreBase>> model(new CtChooseDialogStore<GtkStoreBase>());
        model->set_column_types(model->columns);
        return model;
    }
    void add_row(const std::string& stock_id, const std::string& key, const std::string& desc, gint64 node_id = 0)
    {
        auto row = *GtkStoreBase::append();
        row[columns.stock_id] = stock_id;
        row[columns.key] = key;
        row[columns.desc] = desc;
        row[columns.node_id] = node_id;
    }
};
typedef CtChooseDialogStore<Gtk::ListStore> CtChooseDialogListStore;
typedef CtChooseDialogStore<Gtk::TreeStore> CtChooseDialogTreeStore;

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

// Dialog to Select a Node
Gtk::TreeIter choose_node_dialog(Gtk::Window& parent, Gtk::TreeView& parentTreeView, const std::string& title, CtTreeStore* treestore, Gtk::TreeIter sel_tree_iter);

// Handle the Bookmarks List
void bookmarks_handle_dialog(CtMainWin* ctMainWin);

// Dialog to select a Date
std::time_t date_select_dialog(Gtk::Window& parent, const std::string& title, const std::time_t& curr_time);

class CtMatchDialogStore : public Gtk::TreeStore
{
public:
    struct CtMatchModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<gint64>         node_id;
       Gtk::TreeModelColumn<Glib::ustring>  node_name;
       Gtk::TreeModelColumn<Glib::ustring>  node_hier_name;
       Gtk::TreeModelColumn<int>            start_offset;
       Gtk::TreeModelColumn<int>            end_offset;
       Gtk::TreeModelColumn<int>            line_num;
       Gtk::TreeModelColumn<Glib::ustring>  line_content;
       CtMatchModelColumns() { add(node_id); add(node_name); add(node_hier_name);
                               add(start_offset); add(end_offset); add(line_num); add(line_content); }
       virtual ~CtMatchModelColumns();
    } columns;

    std::array<int, 2> dlg_size;
    std::array<int, 2> dlg_pos;
    Gtk::TreePath      saved_path;

public:
    virtual ~CtMatchDialogStore();

    static Glib::RefPtr<CtMatchDialogStore> create()
    {
        Glib::RefPtr<CtMatchDialogStore> model(new CtMatchDialogStore());
        model->set_column_types(model->columns);
        return model;
    }
    void add_row(gint64 node_id, const std::string& node_name, const std::string& node_hier_name,
                 int start_offset, int end_offset, int line_num, const std::string& line_content)
    {
        auto row = *append();
        row[columns.node_hier_name] = node_hier_name;
        row[columns.node_id] = node_id;             row[columns.node_name] = node_name;
        row[columns.start_offset] = start_offset;   row[columns.end_offset] = end_offset;
        row[columns.line_num] = line_num;           row[columns.line_content] = line_content;

    }
};

// the All Matches Dialog
void match_dialog(const std::string& title, CtMainWin& ctMainWin, Glib::RefPtr<CtMatchDialogStore> model);

// Insert/Edit Anchor Name
Glib::ustring img_n_entry_dialog(Gtk::Window& parent, const char* title,
                                 const Glib::ustring& entry_content, const char* img_stock);

struct CtLinkEntry
{
    Glib::ustring type;
    gint64        node_id = -1;
    Glib::ustring webs;
    Glib::ustring file;
    Glib::ustring fold;
    Glib::ustring anch;
};

// Dialog to Insert/Edit Links
bool link_handle_dialog(CtMainWin& ctMainWin, const Glib::ustring& title, Gtk::TreeIter sel_tree_iter,
                        CtLinkEntry& link_entries);

struct file_select_args
{
    Gtk::Window*                parent = nullptr;
    Glib::ustring               curr_folder;
    Glib::ustring               curr_file_name;
    Glib::ustring               filter_name;
    std::vector<Glib::ustring>  filter_pattern;
    std::vector<Glib::ustring>  filter_mime;
};
// The Select file dialog, Returns the retrieved filepath or None
Glib::ustring file_select_dialog(ct_dialogs::file_select_args args);

// The Select folder dialog, returns the retrieved folderpath or None
Glib::ustring folder_select_dialog(Glib::ustring curr_folder, Gtk::Window* parent = nullptr);

// The Save file as dialog, Returns the retrieved filepath or None
Glib::ustring file_save_as_dialog(ct_dialogs::file_select_args args);

// Insert/Edit Image
Glib::RefPtr<Gdk::Pixbuf> image_handle_dialog(Gtk::Window& father_win, Glib::ustring title,
                                              Glib::RefPtr<Gdk::Pixbuf> original_pixbuf);

// Opens the CodeBox Handle Dialog
bool codeboxhandle_dialog(Gtk::Window& father_win, const Glib::ustring& title);

} // namespace ct_dialogs
