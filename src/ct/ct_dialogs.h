/*
 * ct_dialogs.h
 *
 * Copyright 2009-2026
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
#include "ct_types.h"
#include <glibconfig.h>
#include <gtkmm.h>
#include <array>

class CtMainWin;
class CtTreeStore;
struct CtNodeData;

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
struct CtMatchRowData {
    gint64          node_id;
    Glib::ustring   node_name;
    Glib::ustring   node_hier_name;
    int             start_offset;
    int             end_offset;
    int             line_num;
    Glib::ustring   line_content;
    CtAnchWidgType  anch_type;
    int             anch_cell_idx;
    int             anch_offs_start;
    int             anch_offs_end;
};
class CtMatchDialogStore : public Gtk::ListStore
{
public:
    const size_t cMaxMatchesInPage;
    struct CtMatchModelColumns : public Gtk::TreeModelColumnRecord {
        Gtk::TreeModelColumn<gint64>         node_id;
        Gtk::TreeModelColumn<Glib::ustring>  node_name;
        Gtk::TreeModelColumn<Glib::ustring>  node_hier_name;
        Gtk::TreeModelColumn<int>            start_offset;
        Gtk::TreeModelColumn<int>            end_offset;
        Gtk::TreeModelColumn<int>            line_num;
        Gtk::TreeModelColumn<Glib::ustring>  line_content;
        Gtk::TreeModelColumn<CtAnchWidgType> anch_type;
        Gtk::TreeModelColumn<int>            anch_cell_idx;
        Gtk::TreeModelColumn<int>            anch_offs_start;
        Gtk::TreeModelColumn<int>            anch_offs_end;
        CtMatchModelColumns() {
            add(node_id);
            add(node_name);
            add(node_hier_name);
            add(start_offset);
            add(end_offset);
            add(line_num);
            add(line_content);
            add(anch_type);
            add(anch_cell_idx);
            add(anch_offs_start);
            add(anch_offs_end);
        }
    } columns;
    std::array<int, 2>  dlg_size{0,0};
    std::array<int, 2>  dlg_pos{0,0};
    std::string         saved_path;

    static Glib::RefPtr<CtMatchDialogStore> create(const size_t maxMatchesInPage);

    void deep_clear();
    CtMatchRowData* add_row(const gint64 node_id,
                            const Glib::ustring& node_name,
                            const Glib::ustring& node_hier_name,
                            const int start_offset,
                            const int end_offset,
                            const int line_num,
                            const Glib::ustring& line_content,
                            const CtAnchWidgType anch_type,
                            const int anch_cell_idx,
                            const int anch_offs_start,
                            const int anch_offs_end);
    void load_current_page();
    void load_next_page();
    void load_prev_page();
    size_t get_tot_matches();
    bool is_multipage();
    bool has_next_page();
    bool has_prev_page();
    std::string get_this_page_range();
    std::string get_next_page_range();
    std::string get_prev_page_range();

private:
    CtMatchDialogStore(const size_t maxMatchesInPage)
     : cMaxMatchesInPage{maxMatchesInPage}
    {}
    Gtk::TreeModel::iterator _add_row(const CtMatchRowData& row_data);

    int                         _page_idx{0};
    std::vector<CtMatchRowData> _all_matches;
};

namespace CtDialogs {

enum class CtStartDialogAction { None, NewDoc, OpenFile, OpenFolder, OpenRecent };

CtStartDialogAction start_dialog(CtMainWin* pCtMainWin,
                                 const CtRecentDocsFilepaths& recent_docs,
                                 bool remember_recent_docs,
                                 std::string& recent_filepath,
                                 bool& dont_show_again);

Gtk::TreeModel::iterator choose_item_dialog(Gtk::Window& parent,
                                 const Glib::ustring& title,
                                 Glib::RefPtr<CtChooseDialogListStore> model,
                                 const gchar* single_column_name = nullptr,
                                 const std::string& pathToSelect = "0",
                                 std::optional<std::pair<int,int>> use_size = std::nullopt,
                                 const bool column_is_colour = false);

// Dialog to select between the Selected Node/Selected Node + Subnodes/All Tree
CtExporting selnode_selnodeandsub_alltree_dialog(Gtk::Window& parent,
                                                 bool also_selection,
                                                 bool* last_include_node_name,
                                                 bool* last_new_node_page,
                                                 bool* last_index_in_page,
                                                 bool* last_single_file);

// Dialog to select a color, featuring a palette
enum class CtPickDlgState {SELECTED, CANCEL, REMOVE_COLOR, CALL_AGAIN };
CtPickDlgState colour_pick_dialog(CtMainWin* pCtMainWin, const Glib::ustring& title,
                                  Glib::ustring& colour, bool allow_remove_colour);

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
Gtk::TreeModel::iterator choose_node_dialog(CtMainWin* pCtMainWin,
                                 Gtk::TreeView& parentTreeView,
                                 const Glib::ustring& title,
                                 CtTreeStore* treestore,
                                 Gtk::TreeModel::iterator sel_tree_iter);

// Handle the Bookmarks List
void bookmarks_handle_dialog(CtMainWin* pCtMainWin);

// Dialog to select a Date
std::time_t date_select_dialog(Gtk::Window& parent,
                               const Glib::ustring& title,
                               const std::time_t& curr_time);

void no_matches_dialog(CtMainWin* pCtMainWin,
                       const Glib::ustring& title,
                       const Glib::ustring& message);

// the All Matches Dialog
void match_dialog(const std::string& str_find,
                  CtMainWin* ctMainWin,
                  CtSearchState& s_state);

void iterated_find_dialog(CtMainWin* pCtMainWin,
                          CtSearchState& s_state);

void dialog_search(CtMainWin* pCtMainWin,
                   const Glib::ustring& title,
                   CtSearchOptions& s_options,
                   CtSearchState& s_state,
                   bool multiple_nodes);

// Insert/Edit Anchor Name
Glib::ustring img_n_entry_dialog(Gtk::Window& parent,
                                 const Glib::ustring& title,
                                 const Glib::ustring& entry_content,
                                 const char* img_stock);

// Dialog to Insert/Edit Links
bool link_handle_dialog(CtMainWin& ctMainWin,
                        const Glib::ustring& title,
                        Gtk::TreeModel::iterator sel_tree_iter,
                        CtLinkEntry& link_entry);

struct CtFileSelectArgs
{
    fs::path                    curr_folder;
    fs::path                    curr_file_name;
    Glib::ustring               filter_name;
    std::vector<Glib::ustring>  filter_pattern;
    std::vector<Glib::ustring>  filter_mime;
    bool                        overwrite_confirmation{true};
};

std::string file_select_dialog(Gtk::Window* pParentWin, const CtFileSelectArgs& args);

std::string folder_select_dialog(Gtk::Window* pParentWin, const std::string& curr_folder);

std::string file_save_as_dialog(Gtk::Window* pParentWin, const CtFileSelectArgs& args);

std::string folder_save_as_dialog(Gtk::Window* pParentWin, const CtFileSelectArgs& args);

// Insert/Edit Image
Glib::RefPtr<Gdk::Pixbuf> image_handle_dialog(Gtk::Window& father_win,
                                              Glib::RefPtr<Gdk::Pixbuf> rOriginalPixbuf);
// Insert/Edit Latex
Glib::ustring latex_handle_dialog(CtMainWin* pCtMainWin,
                                  const Glib::ustring& latex_text);

// Opens the CodeBox Handle Dialog
bool codeboxhandle_dialog(CtMainWin* pCtMainWin,
                          const Glib::ustring& title);

struct CtStorageSelectArgs
{
    CtDocType     ctDocType{CtDocType::None};
    CtDocEncrypt  ctDocEncrypt{CtDocEncrypt::None};
    Glib::ustring password;
    bool          showAutosaveOptions{false};
};

// Choose the CherryTree data storage type and protection
bool choose_data_storage_dialog(CtMainWin* pCtMainWin, CtStorageSelectArgs& args);

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
gint64 dialog_selnode(CtMainWin* pCtMainWin, const Glib::ustring& entryStr);

void summary_info_dialog(CtMainWin* pCtMainWin, const CtSummaryInfo& summaryInfo);

enum class TableHandleResp { Cancel, Ok, OkFromFile };
TableHandleResp table_handle_dialog(CtMainWin* pCtMainWin,
                                    const Glib::ustring& title,
                                    const bool is_insert,
                                    bool& is_light);

} // namespace CtDialogs
