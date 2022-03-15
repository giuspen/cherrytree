/*
 * ct_actions.h
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

#include "ct_main_win.h"
#include "ct_dialogs.h"
#include "ct_codebox.h"
#include "ct_image.h"
#include "ct_table.h"
#include "ct_types.h"
#include "ct_filesystem.h"
#include <optional>

class CtMainWin;
class CtImporterInterface;
struct CtImportedNode;
struct TocEntry;
class CtActions
{
public:
    CtActions(CtMainWin* pCtMainWin) {
        _pCtMainWin = pCtMainWin;
        _find_init();
        _validate_enable_spell_check();
    }

public:
    CtCodebox*      curr_codebox_anchor{nullptr};
    CtTable*        curr_table_anchor{nullptr};
    CtImageAnchor*  curr_anchor_anchor{nullptr};
    CtImagePng*     curr_image_anchor{nullptr};
    CtImageEmbFile* curr_file_anchor{nullptr};

private:
    void _validate_enable_spell_check();
    CtLinkEntry _link_entry;

private:
    struct CtEmbFileOpened {
        fs::path tmp_filepath;
        time_t mod_time;
    };
    std::unordered_map<size_t, CtEmbFileOpened> _embfiles_opened;
    sigc::connection _embfiles_timeout_connection;

private:
    CtMainWin*   _pCtMainWin;

private:
    CtExportOptions _export_options;

private:
    CtSearchOptions _s_options;
    CtSearchState _s_state;

public:
    CtMainWin*   getCtMainWin() { return _pCtMainWin; }
    bool         get_were_embfiles_opened() { return _embfiles_opened.size(); }

private:
    Glib::RefPtr<Gtk::TextBuffer> _curr_buffer() { return _pCtMainWin->get_text_view().get_buffer(); }
    bool          _node_sel_and_rich_text();

public: // todo: fix naming
    bool          _is_there_selected_node_or_error();
    bool          _is_tree_not_empty_or_error();
    bool          _is_curr_node_not_read_only_or_error();
    bool          _is_curr_node_not_syntax_highlighting_or_error(bool plain_text_ok = false);
    bool          _is_there_text_selection_or_error();

public:
    void object_set_selection(CtAnchoredWidget* widget);

private:
    // helpers for file actions
    void _file_save(bool need_vacuum);

public:
    // file actions
    void file_new();
    void file_open();
    void file_save();
    void file_vacuum();
    void file_save_as();
    void quit_or_hide_window();
    void quit_window();
    void dialog_preferences();
    void preferences_import();
    void preferences_export();
    void command_palette();

private:
    // helpers for tree actions
    void          _node_add(bool duplicate,
                            bool add_child);
    Gtk::TreeIter _node_add_with_data(Gtk::TreeIter curr_iter,
                                      CtNodeData& nodeData,
                                      bool add_child,
                                      std::shared_ptr<CtNodeState> node_state);

public:
    Gtk::TreeIter node_child_exist_or_create(Gtk::TreeIter parentIter,
                                             const std::string& nodeName,
                                             const bool focusIfExisting = true);
    void node_move_after(Gtk::TreeIter iter_to_move,
                         Gtk::TreeIter father_iter,
                         Gtk::TreeIter brother_iter = Gtk::TreeIter{},
                         bool set_first = false);

private:
    bool _need_node_swap(Gtk::TreeIter& leftIter,
                         Gtk::TreeIter& rightIter,
                         bool ascendings);
    bool _tree_sort_level_and_sublevels(const Gtk::TreeNodeChildren& children,
                                        bool ascending);

public:
    // tree actions
    void node_add()                { _node_add(false, false); }
    void node_duplicate()          { _node_add(true, false); }
    void node_child_add()          { _node_add(false, true); }
    void node_subnodes_duplicate();
    void node_subnodes_copy();
    void node_subnodes_paste();
    void node_edit();
    void node_inherit_syntax();
    void node_delete();
    void node_toggle_read_only();
    void node_date();
    void node_up();
    void node_down();
    void node_right();
    void node_left();
    void node_change_father();
    bool node_move(Gtk::TreeModel::Path src_path,
                   Gtk::TreeModel::Path dest_path,
                   bool only_test_dest);
    void tree_sort_ascending();
    void tree_sort_descending();
    void tree_info();
    void tree_clear_property_exclude_from_search();
    void node_link_to_clipboard();
    void node_siblings_sort_ascending();
    void node_siblings_sort_descending();
    void node_go_back();    // was as go_back
    void node_go_forward(); // was as go_forward

    void bookmark_curr_node();
    void bookmark_curr_node_remove();
    void bookmarks_handle();

private:
    // helpers for find actions
    void _find_init();
    bool _parse_given_node_content(CtTreeIter node_iter,
                                   Glib::RefPtr<Glib::Regex> re_pattern,
                                   bool forward,
                                   bool first_fromsel,
                                   bool all_matches);
    bool _parse_node_content_iter(const CtTreeIter& tree_iter,
                                  Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                  Glib::RefPtr<Glib::Regex> re_pattern,
                                  bool forward,
                                  bool first_fromsel,
                                  bool all_matches,
                                  bool first_node);
    bool _parse_node_name_n_tags_iter(CtTreeIter& tree_iter,
                                      Glib::RefPtr<Glib::Regex> re_pattern,
                                      const bool all_matches);
    Gtk::TextIter _get_inner_start_iter(Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                        bool forward,
                                        const gint64& node_id);
    bool _is_node_within_time_filter(const CtTreeIter& node_iter);
    Glib::RefPtr<Glib::Regex> _create_re_pattern(Glib::ustring pattern);
    bool _find_pattern(CtTreeIter tree_iter,
                       Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                       Glib::RefPtr<Glib::Regex> re_pattern,
                       Gtk::TextIter start_iter,
                       bool forward,
                       bool all_matches);
    std::string         _get_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                          Gtk::TextIter text_iter);
    std::string         _get_first_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer);
    Glib::ustring       _check_pattern_in_object(Glib::RefPtr<Glib::Regex> pattern,
                                                 CtAnchoredWidget* obj);
    std::pair<int, int> _check_pattern_in_object_between(CtTreeIter tree_iter,
                                                         Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                                         Glib::RefPtr<Glib::Regex> pattern,
                                                         int start_offset,
                                                         int end_offset,
                                                         bool forward,
                                                         std::string& obj_content);
    int  _get_num_objs_before_offset(Glib::RefPtr<Gtk::TextBuffer> text_buffer, int max_offset);
    void _update_all_matches_progress();

public:
    // find actions
    void find_in_selected_node();
    void find_in_multiple_nodes();
    void find_in_multiple_nodes_act() { _s_options.node_content = true; find_in_multiple_nodes(); }
    void find_a_node() { _s_options.node_content = false; _s_options.node_name_n_tags = true; find_in_multiple_nodes(); }
    void find_again() { find_again_iter(false/*fromIterativeDialog*/); }
    void find_back() { find_back_iter(false/*fromIterativeDialog*/); }
    void replace_in_selected_node();
    void replace_in_multiple_nodes();
    void replace_again();
    void find_allmatchesdialog_restore();

public:
    // helpers for find actions
    void find_again_iter(const bool fromIterativeDialog);
    void find_back_iter(const bool fromIterativeDialog);

private:
    // helper for view actions
    void _menubar_in_titlebar_set(const bool setOn);

public:
    // view actions
    void toggle_show_hide_tree();
    void toggle_show_hide_menubar();
    void toggle_show_hide_toolbars();
    void toggle_show_hide_statusbar();
    void toggle_show_hide_tree_lines();
    void toggle_show_hide_node_name_header();
    void toggle_tree_text();
    void nodes_expand_all();
    void nodes_collapse_all();
    void toolbar_icons_size_increase();
    void toolbar_icons_size_decrease();
    void fullscreen_toggle();
    void disable_menubar_in_titlebar() { _menubar_in_titlebar_set(false); }
    void enable_menubar_in_titlebar() { _menubar_in_titlebar_set(true); }

public:
    // helper for format actions
    void apply_tag(const Glib::ustring& tag_property,
                   Glib::ustring property_value = "",
                   std::optional<Gtk::TextIter> iter_sel_start = std::nullopt,
                   std::optional<Gtk::TextIter> iter_sel_end = std::nullopt,
                   Glib::RefPtr<Gtk::TextBuffer> text_buffer = Glib::RefPtr<Gtk::TextBuffer>{});

private:
    struct text_view_n_buffer_codebox_proof {
        CtTextView*  text_view;
        std::string  syntax_highl;
        CtCodebox*   codebox;
        CtTable*     table;
    };
    text_view_n_buffer_codebox_proof _get_text_view_n_buffer_codebox_proof();
    CtCodebox* _codebox_in_use();
    CtTable* _table_in_use();
    void _save_tags_at_cursor_as_latest(Glib::RefPtr<Gtk::TextBuffer> rTextBuffer,
                                        int cursorOffset);
    bool _links_entries_pre_dialog(const Glib::ustring& curr_link,
                                   CtLinkEntry& link_entry);
    Glib::ustring _links_entries_post_dialog(CtLinkEntry& link_entry);
    Glib::ustring _link_check_around_cursor();

public:
    // format actions
    void save_tags_at_cursor_as_latest();
    void apply_tags_latest();
    void remove_text_formatting();
    void apply_tag_foreground();
    void apply_tag_background();
    void apply_tag_bold();
    void apply_tag_italic();
    void apply_tag_underline();
    void apply_tag_strikethrough();
    void apply_tag_indent();
    void reduce_tag_indent();
    void apply_tag_h1() { _apply_tag_hN(CtConst::TAG_PROP_VAL_H1); }
    void apply_tag_h2() { _apply_tag_hN(CtConst::TAG_PROP_VAL_H2); }
    void apply_tag_h3() { _apply_tag_hN(CtConst::TAG_PROP_VAL_H3); }
    void apply_tag_h4() { _apply_tag_hN(CtConst::TAG_PROP_VAL_H4); }
    void apply_tag_h5() { _apply_tag_hN(CtConst::TAG_PROP_VAL_H5); }
    void apply_tag_h6() { _apply_tag_hN(CtConst::TAG_PROP_VAL_H6); }
    void apply_tag_small();
    void apply_tag_superscript();
    void apply_tag_subscript();
    void apply_tag_monospace();
    void list_bulleted_handler();
    void list_numbered_handler();
    void list_todo_handler();
    void apply_tag_justify_left();
    void apply_tag_justify_center();
    void apply_tag_justify_right();
    void apply_tag_justify_fill();

private:
    // helper for edit actions
    void _image_edit_dialog(Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
                            Gtk::TextIter insertIter,
                            Gtk::TextIter* pIterBound);
    void _text_selection_change_case(gchar change_type);
    int  _find_previous_indent_margin();
    void _apply_tag_hN(const char* tagPropScaleVal);

public:
    void image_insert_png(Gtk::TextIter iter_insert,
                          Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                          const Glib::ustring& link,
                          const Glib::ustring& image_justification);
    void image_insert_anchor(Gtk::TextIter iter_insert,
                             const Glib::ustring& name,
                             const Glib::ustring& image_justification);

private:
    void _insert_toc_at_pos(Glib::RefPtr<Gtk::TextBuffer> text_buffer, const std::list<TocEntry>& entries);

public:
    // edit actions
    void requested_step_back();
    void requested_step_ahead();
    void image_handle();
    void table_handle();
    void codebox_handle();
    void embfile_insert();
    void embfile_insert_path(const std::string& filepath);
    void apply_tag_link();
    void anchor_handle();
    void toc_insert();
    void timestamp_insert();
    void special_char_insert();
    void horizontal_rule_insert();
    void text_selection_lower_case();
    void text_selection_upper_case();
    void text_selection_toggle_case();
    void toggle_ena_dis_spellcheck();
    void cut_as_plain_text();
    void copy_as_plain_text();
    void paste_as_plain_text();
    void text_row_cut();
    void text_row_copy();
    void text_row_delete();
    void text_row_selection_duplicate();
    void text_row_up();
    void text_row_down();
    void strip_trailing_spaces();
    void replace_tabs_with_spaces();

private:
    // helper for others actions
    void _anchor_edit_dialog(CtImageAnchor* anchor,
                             Gtk::TextIter insert_iter,
                             Gtk::TextIter* iter_bound);
    bool _on_embfiles_sentinel_timeout();

public:
    // others actions
    void link_cut();
    void link_copy();
    void link_dismiss();
    void link_delete();
    void anchor_cut();
    void anchor_copy();
    void anchor_delete();
    void anchor_edit();
    void anchor_link_to_clipboard();
    void embfile_cut();
    void embfile_copy();
    void embfile_delete();
    void embfile_save();
    void embfile_open();
    void embfile_rename();
    void image_save();
    void image_edit();
    void image_cut();
    void image_copy();
    void image_delete();
    void image_link_edit();
    void image_link_dismiss();
    void toggle_show_hide_main_window();

    void link_clicked(const Glib::ustring& tag_property_value, bool from_wheel);

    void codebox_cut();
    void codebox_copy();
    void codebox_delete();
    void codebox_delete_keeping_text();
    void codebox_change_properties();
    void exec_code();
    void codebox_load_from_file();
    void codebox_save_to_file();
    void codebox_increase_width();
    void codebox_decrease_width();
    void codebox_increase_height();
    void codebox_decrease_height();

    void table_cut();
    void table_copy();
    void table_delete();
    void table_column_add();
    void table_column_delete();
    void table_column_left();
    void table_column_right();
    void table_column_increase_width();
    void table_column_decrease_width();
    void table_row_add();
    void table_row_cut();
    void table_row_copy();
    void table_row_paste();
    void table_row_delete();
    void table_row_up();
    void table_row_down();
    void table_rows_sort_descending();
    void table_rows_sort_ascending();
    void table_edit_properties();
    void table_export();

private:
    // helper for import actions
    void _import_from_file(CtImporterInterface* importer, const bool dummy_root = false) noexcept;
    void _import_from_dir(CtImporterInterface* importer, const std::string& custom_dir) noexcept;
    void _create_imported_nodes(CtImportedNode* imported_nodes, const bool dummy_root = false);

public:
    // import actions
    void import_node_from_html_file() noexcept;
    void import_node_from_html_directory() noexcept;
    void import_node_from_plaintext_file() noexcept;
    void import_nodes_from_plaintext_directory() noexcept;
    void import_nodes_from_ct_file() noexcept;
    void import_nodes_from_zim_directory() noexcept;
    void import_node_from_md_file() noexcept;
    void import_nodes_from_md_directory() noexcept;
    void import_node_from_pandoc() noexcept;
    void import_directory_from_pandoc() noexcept;
    void import_nodes_from_gnote_directory() noexcept;
    void import_nodes_from_tomboy_directory() noexcept;
    void import_nodes_from_keepnote_directory() noexcept;
    void import_nodes_from_mempad_file() noexcept;
    void import_nodes_from_treepad_file() noexcept;
    void import_nodes_from_leo_file() noexcept;
    void import_nodes_from_rednotebook_html() noexcept;
    void import_nodes_from_notecase_html() noexcept;

private:
    // helper for export actions
    void _export_print(bool save_to_pdf, const fs::path& auto_path, bool auto_overwrite);
    void _export_to_html(const fs::path& auto_path, bool auto_overwrite);
    void _export_to_txt(const fs::path& auto_path, bool auto_overwrite);

    fs::path _get_pdf_filepath(const fs::path& proposed_name);
    fs::path _get_txt_filepath(const fs::path& dir_place, const fs::path& proposed_name);
    fs::path _get_txt_folder(fs::path dir_place, fs::path new_folder, bool export_overwrite);

public:
    // export actions
    void export_print_page_setup();
    void export_print();
    void export_to_pdf();
    void export_to_html();
    void export_to_txt();
    void export_to_ctd();

    void export_to_pdf_auto(const std::string& dir, bool overwrite);
    void export_to_html_auto(const std::string& dir, bool overwrite, bool single_file);
    void export_to_txt_auto(const std::string& dir, bool overwrite, bool single_file);

private:
    // helpers for help actions
    std::string _get_latest_version_from_server();

public:
    // help actions
    void check_for_newer_version();
    void online_help();
    void folder_cfg_open();
    void dialog_about();
};
