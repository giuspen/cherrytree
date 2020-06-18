/*
 * ct_dialogs.h
 *
 * Copyright 2009-2020
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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

#include "ct_misc_utils.h"
#include "ct_filesystem.h"
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <array>

class CtMainWin;
class CtTreeStore;

class CtDialogTextEntry : public Gtk::Dialog
{
public:
    CtDialogTextEntry(const Glib::ustring& title,
                      const bool forPassword,
                      Gtk::Window* pParentWin);
    virtual ~CtDialogTextEntry() {}
    Glib::ustring get_entry_text() { return _entry.get_text(); }

protected:
    bool _on_entry_key_press_event(GdkEventKey* pEventKey);
    void _on_entry_icon_press(Gtk::EntryIconPosition /*iconPosition*/, const GdkEventButton* /*pEvent*/) { _entry.set_text(""); }
    Gtk::Entry _entry;
};

template<class GtkStoreBase> class CtChooseDialogStore : public GtkStoreBase
{
public:
    struct CtChooseDialogModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<std::string>   stock_id;
        Gtk::TreeModelColumn<Glib::ustring> key;
        Gtk::TreeModelColumn<Glib::ustring> desc;
        Gtk::TreeModelColumn<gint64>        node_id;

        CtChooseDialogModelColumns()
        {
            add(stock_id);
            add(key);
            add(desc);
            add(node_id);
        }
    } columns;

public:
    static Glib::RefPtr<CtChooseDialogStore<GtkStoreBase>> create()
    {
        Glib::RefPtr<CtChooseDialogStore<GtkStoreBase>> rModel{new CtChooseDialogStore<GtkStoreBase>()};
        rModel->set_column_types(rModel->columns);
        return rModel;
    }
    void add_row(const std::string& stock_id,
                 const std::string& key,
                 const std::string& desc,
                 gint64 node_id = 0)
    {
        Gtk::TreeRow row = *GtkStoreBase::append();
        row[columns.stock_id] = stock_id;
        row[columns.key] = key;
        row[columns.desc] = desc;
        row[columns.node_id] = node_id;
    }
};

typedef CtChooseDialogStore<Gtk::ListStore> CtChooseDialogListStore;
typedef CtChooseDialogStore<Gtk::TreeStore> CtChooseDialogTreeStore;

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
        CtMatchModelColumns()
        {
            add(node_id);
            add(node_name);
            add(node_hier_name);
            add(start_offset);
            add(end_offset);
            add(line_num);
            add(line_content);
        }
        virtual ~CtMatchModelColumns() {}
    } columns;

    std::array<int, 2> dlg_size;
    std::array<int, 2> dlg_pos;
    Gtk::TreePath      saved_path;

public:
    virtual ~CtMatchDialogStore() {}

    static Glib::RefPtr<CtMatchDialogStore> create()
    {
        Glib::RefPtr<CtMatchDialogStore> rModel{new CtMatchDialogStore()};
        rModel->set_column_types(rModel->columns);
        return rModel;
    }
    void add_row(gint64 node_id,
                 const Glib::ustring& node_name,
                 const Glib::ustring& node_hier_name,
                 int start_offset,
                 int end_offset,
                 int line_num,
                 const Glib::ustring& line_content)
    {
        Gtk::TreeRow row = *append();
        row[columns.node_id] = node_id;
        row[columns.node_name] = node_name;
        row[columns.node_hier_name] = node_hier_name;
        row[columns.start_offset] = start_offset;
        row[columns.end_offset] = end_offset;
        row[columns.line_num] = line_num;
        row[columns.line_content] = line_content;
    }
};

namespace CtDialogs {


Gtk::TreeIter choose_item_dialog(Gtk::Window& parent,
                                 const Glib::ustring& title,
                                 Glib::RefPtr<CtChooseDialogListStore> model,
                                 const gchar* single_column_name = nullptr);

// Dialog to select between the Selected Node/Selected Node + Subnodes/All Tree
enum CtProcessNode { NONE, SELECTED_TEXT, CURRENT_NODE, CURRENT_NODE_AND_SUBNODES, ALL_TREE };
CtProcessNode selnode_selnodeandsub_alltree_dialog(Gtk::Window& parent,
                                                   bool also_selection,
                                                   bool* last_include_node_name /*= nullptr*/,
                                                   bool* last_new_node_page /*= nullptr*/,
                                                   bool* last_index_in_page /*= nullptr*/);

// Dialog to select a color, featuring a palette
bool color_pick_dialog(CtMainWin* pCtMainWin,
                       Gdk::RGBA& color);

// The Question dialog, returns True if the user presses OK
bool question_dialog(const Glib::ustring& message,
                     Gtk::Window& parent);

// The Info dialog
void info_dialog(const Glib::ustring& message,
                 Gtk::Window& parent);

// The Warning dialog
void warning_dialog(const Glib::ustring& message,
                    Gtk::Window& parent);

// The Error dialog
void error_dialog(const Glib::ustring& message,
                  Gtk::Window& parent);

// Dialog to Select a Node
Gtk::TreeIter choose_node_dialog(CtMainWin* pCtMainWin,
                                 Gtk::TreeView& parentTreeView,
                                 const Glib::ustring& title,
                                 CtTreeStore* treestore,
                                 Gtk::TreeIter sel_tree_iter);

// Handle the Bookmarks List
void bookmarks_handle_dialog(CtMainWin* pCtMainWin);

// Dialog to select a Date
std::time_t date_select_dialog(Gtk::Window& parent,
                               const Glib::ustring& title,
                               const std::time_t& curr_time);

// the All Matches Dialog
void match_dialog(const Glib::ustring& title,
                  CtMainWin& ctMainWin,
                  Glib::RefPtr<CtMatchDialogStore> model);

// Insert/Edit Anchor Name
Glib::ustring img_n_entry_dialog(Gtk::Window& parent,
                                 const Glib::ustring& title,
                                 const Glib::ustring& entry_content,
                                 const char* img_stock);

struct CtLinkEntry
{
    Glib::ustring type;
    gint64        node_id{-1};
    Glib::ustring webs;
    Glib::ustring file;
    Glib::ustring fold;
    Glib::ustring anch;
};

// Dialog to Insert/Edit Links
bool link_handle_dialog(CtMainWin& ctMainWin,
                        const Glib::ustring& title,
                        Gtk::TreeIter sel_tree_iter,
                        CtLinkEntry& link_entries);

struct file_select_args
{
    Gtk::Window*                pParentWin{nullptr};
    fs::path                    curr_folder;
    fs::path                    curr_file_name;
    Glib::ustring               filter_name;
    std::vector<std::string>    filter_pattern;
    std::vector<std::string>    filter_mime;

    file_select_args(Gtk::Window* win) : pParentWin(win) {}
};

// The Select file dialog, Returns the retrieved filepath or None
std::string file_select_dialog(const file_select_args& args);

// The Select folder dialog, returns the retrieved folderpath or None
std::string folder_select_dialog(const std::string& curr_folder,
                                 Gtk::Window* pParentWin = nullptr);

// The Save file as dialog, Returns the retrieved filepath or None
std::string file_save_as_dialog(const file_select_args& args);

// Insert/Edit Image
Glib::RefPtr<Gdk::Pixbuf> image_handle_dialog(Gtk::Window& father_win,
                                              const Glib::ustring& title,
                                              Glib::RefPtr<Gdk::Pixbuf> rOriginalPixbuf);

// Opens the CodeBox Handle Dialog
bool codeboxhandle_dialog(CtMainWin* pCtMainWin,
                          const Glib::ustring& title);

struct storage_select_args
{
    Gtk::Window*  pParentWin{nullptr};
    CtDocType     ctDocType{CtDocType::None};
    CtDocEncrypt  ctDocEncrypt{CtDocEncrypt::None};
    std::string   password;

    storage_select_args(Gtk::Window* win) : pParentWin(win) {}
};

// Choose the CherryTree data storage type (xml or db) and protection
bool choose_data_storage_dialog(storage_select_args& args);

bool node_prop_dialog(const Glib::ustring &title,
                      CtMainWin* pCtMainWin,
                      CtNodeData& nodeData,
                      const std::set<Glib::ustring>& tags_set);

CtYesNoCancel exit_save_dialog(Gtk::Window& parent);

// Application About Dialog
void dialog_about(Gtk::Window& parent, Glib::RefPtr<Gdk::Pixbuf> icon);

std::string dialog_palette(CtMainWin* pCtMainWin);

void summary_info_dialog(CtMainWin* pCtMainWin, const CtSummaryInfo& summaryInfo);

} // namespace CtDialogs
