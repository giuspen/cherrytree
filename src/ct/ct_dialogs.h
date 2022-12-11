/*
 * ct_dialogs.h
 *
 * Copyright 2009-2022
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
#include <gtkmm.h>
#include <array>

class CtMainWin;
class CtTreeStore;

class CtDialogTextEntry : public Gtk::Dialog
{
public:
    CtDialogTextEntry(const Glib::ustring& title,
                      const bool forPassword,
                      Gtk::Window* pParentWin);
    Glib::ustring get_entry_text() { return _entry.get_text(); }

protected:
    bool _on_entry_key_press_event(GdkEventKey* pEventKey);
    void _on_entry_icon_press(Gtk::EntryIconPosition /*iconPosition*/, const GdkEventButton* /*pEvent*/) { _entry.set_text(""); }
    Gtk::Entry _entry;
};

template<class GtkStoreBase> class CtChooseDialogStore : public GtkStoreBase
{
public:
    struct CtChooseDialogModelColumns : public Gtk::TreeModelColumnRecord 
    {
        Gtk::TreeModelColumn<std::string>   stock_id;
        Gtk::TreeModelColumn<Glib::ustring> key;
        Gtk::TreeModelColumn<Glib::ustring> desc;
        Gtk::TreeModelColumn<gint64>        node_id;
        CtChooseDialogModelColumns() {
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
    struct CtMatchModelColumns : public Gtk::TreeModelColumnRecord
    {
        Gtk::TreeModelColumn<gint64>         node_id;
        Gtk::TreeModelColumn<Glib::ustring>  node_name;
        Gtk::TreeModelColumn<Glib::ustring>  node_hier_name;
        Gtk::TreeModelColumn<int>            start_offset;
        Gtk::TreeModelColumn<int>            end_offset;
        Gtk::TreeModelColumn<int>            line_num;
        Gtk::TreeModelColumn<Glib::ustring>  line_content;
        CtMatchModelColumns() {
            add(node_id);
            add(node_name);
            add(node_hier_name);
            add(start_offset);
            add(end_offset);
            add(line_num);
            add(line_content);
        }
    } columns;

    std::array<int, 2> dlg_size;
    std::array<int, 2> dlg_pos;
    std::string        saved_path; // don't use Gtk::TreePath, see git log

public:
    virtual ~CtMatchDialogStore() {}

    static Glib::RefPtr<CtMatchDialogStore> create() {
        Glib::RefPtr<CtMatchDialogStore> rModel{new CtMatchDialogStore()};
        rModel->set_column_types(rModel->columns);
        return rModel;
    }
    Gtk::TreeIter add_row(gint64 node_id,
                          const Glib::ustring& node_name,
                          const Glib::ustring& node_hier_name,
                          int start_offset,
                          int end_offset,
                          int line_num,
                          const Glib::ustring& line_content)
    {
        Gtk::TreeIter retIter = append();
        Gtk::TreeRow row = *retIter;
        row[columns.node_id] = node_id;
        row[columns.node_name] = node_name;
        row[columns.node_hier_name] = node_hier_name;
        row[columns.start_offset] = start_offset;
        row[columns.end_offset] = end_offset;
        row[columns.line_num] = line_num;
        row[columns.line_content] = line_content;
        return retIter;
    }
};

namespace CtDialogs {

Gtk::TreeIter choose_item_dialog(Gtk::Window& parent,
                                 const Glib::ustring& title,
                                 Glib::RefPtr<CtChooseDialogListStore> model,
                                 const gchar* single_column_name = nullptr,
                                 const std::string& pathToSelect = "0");

// Dialog to select between the Selected Node/Selected Node + Subnodes/All Tree
CtExporting selnode_selnodeandsub_alltree_dialog(Gtk::Window& parent,
                                                 bool also_selection,
                                                 bool* last_include_node_name,
                                                 bool* last_new_node_page,
                                                 bool* last_index_in_page,
                                                 bool* last_single_file);

// Dialog to select a color, featuring a palette
enum class CtPickDlgState {SELECTED, CANCEL, REMOVE_COLOR };
CtPickDlgState color_pick_dialog(CtMainWin* pCtMainWin, const Glib::ustring& title,
                                 Gdk::RGBA& color, bool allow_remove_color);

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
                  CtMainWin* ctMainWin,
                  Glib::RefPtr<CtMatchDialogStore>& rModel);

void iterated_find_dialog(CtMainWin* pCtMainWin,
                          CtSearchState& s_state);

std::string dialog_search(Gtk::Window* pParentWin,
                          const std::string& title,
                          CtSearchOptions& s_options,
                          bool replace_on,
                          bool multiple_nodes);

// Insert/Edit Anchor Name
Glib::ustring img_n_entry_dialog(Gtk::Window& parent,
                                 const Glib::ustring& title,
                                 const Glib::ustring& entry_content,
                                 const char* img_stock);

// Dialog to Insert/Edit Links
bool link_handle_dialog(CtMainWin& ctMainWin,
                        const Glib::ustring& title,
                        Gtk::TreeIter sel_tree_iter,
                        CtLinkEntry& link_entries);

struct FileSelectArgs
{
    FileSelectArgs(Gtk::Window* win) : pParentWin{win} {}

    Gtk::Window*                pParentWin{nullptr};
    fs::path                    curr_folder;
    fs::path                    curr_file_name;
    Glib::ustring               filter_name;
    std::vector<Glib::ustring>  filter_pattern;
    std::vector<Glib::ustring>  filter_mime;
};

// The Select file dialog, Returns the retrieved filepath or None
std::string file_select_dialog(const FileSelectArgs& args);

// The Select folder dialog, returns the retrieved folderpath or None
std::string folder_select_dialog(const std::string& curr_folder, Gtk::Window* pParentWin);

// The Save file as dialog, Returns the retrieved filepath or None
std::string file_save_as_dialog(const FileSelectArgs& args);

// Insert/Edit Image
Glib::RefPtr<Gdk::Pixbuf> image_handle_dialog(Gtk::Window& father_win,
                                              Glib::RefPtr<Gdk::Pixbuf> rOriginalPixbuf);
// Insert/Edit Latex
Glib::ustring latex_handle_dialog(CtMainWin* pCtMainWin,
                                  const Glib::ustring& latex_text);

// Opens the CodeBox Handle Dialog
bool codeboxhandle_dialog(CtMainWin* pCtMainWin,
                          const Glib::ustring& title);

struct storage_select_args
{
    Gtk::Window*  pParentWin{nullptr};
    CtDocType     ctDocType{CtDocType::None};
    CtDocEncrypt  ctDocEncrypt{CtDocEncrypt::None};
    Glib::ustring password;

    storage_select_args(Gtk::Window* win) : pParentWin(win) {}
};

// Choose the CherryTree data storage type (xml or db) and protection
bool choose_data_storage_dialog(storage_select_args& args);

bool node_prop_dialog(const Glib::ustring &title,
                      CtMainWin* pCtMainWin,
                      CtNodeData& nodeData,
                      const std::set<Glib::ustring>& tags_set);

CtYesNoCancel exit_save_dialog(CtMainWin& ct_main_win);

bool exec_code_confirm_dialog(CtMainWin& ct_main_win,
                              const std::string& syntax_highl,
                              const Glib::ustring& code_txt);

// Application About Dialog
void dialog_about(Gtk::Window& parent, Glib::RefPtr<Gdk::Pixbuf> icon);

std::string dialog_palette(CtMainWin* pCtMainWin);

void summary_info_dialog(CtMainWin* pCtMainWin, const CtSummaryInfo& summaryInfo);

enum class TableHandleResp { Cancel, Ok, OkFromFile };
TableHandleResp table_handle_dialog(CtMainWin* pCtMainWin, const Glib::ustring& title, const bool is_insert);

} // namespace CtDialogs
