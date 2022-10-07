/*
 * ct_menu_actions.cc
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

#include "ct_actions.h"
#include "ct_menu.h"
#include "ct_const.h"

CtMenuAction* CtMenu::find_action(const std::string& id)
{
    for (CtMenuAction& action : _actions) {
        if (action.id == id) {
            return &action;
        }
    }
    return nullptr;
}

void CtMenu::init_actions(CtActions* pActions)
{
    // stubs for menu bar
    _actions.push_back(CtMenuAction{"", "FileMenu", None, _("_File"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "EditMenu", None, _("_Edit"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "InsertMenu", None, _("_Insert"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "FormatMenu", None, _("F_ormat"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "TreeMenu", None, _("_Tree"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ToolsMenu", None, _("Too_ls"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "SearchMenu", None, _("_Search"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ViewMenu", None, _("_View"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "BookmarksMenu", None, _("_Bookmarks"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "HelpMenu", None, _("_Help"), None, None, sigc::signal<void>()});

    // stubs for sumenu bar
    _actions.push_back(CtMenuAction{"", "TreeMoveSubMenu", "ct_go-jump", _("Node _Move"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "TreeSortSubMenu", "ct_sort-asc", _("Nod_es Sort"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "BookmarksSubMenu", "ct_pin", _("B_ookmarks"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ImportSubMenu", CtConst::STR_STOCK_CT_IMP, _("_Import"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ExportSubMenu", "ct_export_from_cherrytree", _("_Export"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "PrefSubMenu", "ct_preferences", _("_Preferences"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "RecentDocsSubMenu", "ct_open", _("_Recent Documents"), None,
        _("Open a Recent CherryTree Document"), sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ChangeCaseSubMenu", "ct_case_toggle", _("C_hange Case"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ListSubMenu", "ct_list_bulleted", _("L_ist"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "JustifySubMenu", "ct_justify-center", _("_Justify"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "FontSubMenu", "ct_fonts", _("Toggle _Font Property"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "HeadingSubMenu", "ct_fmt-txt-h1", _("_Toggle Heading Property"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "InsertSubMenu", "ct_insert", _("I_nsert"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "FindSubMenu", "ct_find", _("_Find"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ReplaceSubMenu", "ct_find_replace", _("_Replace"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "RowSubMenu", "ct_edit", _("Ro_w"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "FormattingSubMenu", "ct_fmt-txt", _("F_ormat"), None, None, sigc::signal<void>()});

    // main actions
    const char* file_cat = _("File");
    _actions.push_back(CtMenuAction{file_cat, "ct_new_inst", "ct_new-instance", _("_New Instance"), None,
        _("Start a New Instance of CherryTree"), sigc::mem_fun(*pActions, &CtActions::file_new)});
    _actions.push_back(CtMenuAction{file_cat, "ct_open_file", "ct_open", _("_Open File"), KB_CONTROL+"O",
        _("Open a CherryTree Document"), sigc::mem_fun(*pActions, &CtActions::file_open)});
    _actions.push_back(CtMenuAction{file_cat, "ct_save", "ct_save", _("_Save"), KB_CONTROL+"S",
        _("Save File"), sigc::mem_fun(*pActions, &CtActions::file_save)});
    _actions.push_back(CtMenuAction{file_cat, "ct_vacuum", "ct_clear", _("Save and _Vacuum"), None,
        _("Save File and Vacuum"), sigc::mem_fun(*pActions, &CtActions::file_vacuum)});
    _actions.push_back(CtMenuAction{file_cat, "ct_save_as", "ct_save-as", _("Save _As"), KB_CONTROL+KB_SHIFT+"S",
        _("Save File As"), sigc::mem_fun(*pActions, &CtActions::file_save_as)});
    _actions.push_back(CtMenuAction{file_cat, "print_page_setup", "ct_print", _("Pa_ge Setup"), None,
        _("Set up the Page for Printing"), sigc::mem_fun(*pActions, &CtActions::export_print_page_setup)});
    _actions.push_back(CtMenuAction{file_cat, "do_print", "ct_print", _("P_rint"), KB_CONTROL+"P",
        _("Print"), sigc::mem_fun(*pActions, &CtActions::export_print)});
    _actions.push_back(CtMenuAction{file_cat, "preferences_dlg", "ct_preferences", _("_Preferences"), KB_CONTROL+KB_ALT+"P",
        _("Preferences"), sigc::mem_fun(*pActions, &CtActions::dialog_preferences) });
    _actions.push_back(CtMenuAction{file_cat, "pref_import", CtConst::STR_STOCK_CT_IMP, _("_Import Preferences"), None,
        _("Import Preferences"), sigc::mem_fun(*pActions, &CtActions::preferences_import) });
    _actions.push_back(CtMenuAction{file_cat, "pref_export", "ct_export_from_cherrytree", _("_Export Preferences"), None,
        _("Export Preferences"), sigc::mem_fun(*pActions, &CtActions::preferences_export) });
    _actions.push_back(CtMenuAction{file_cat, "tree_parse_info", "ct_info", _("Tree In_fo"), None,
        _("Tree Summary Information"), sigc::mem_fun(*pActions, &CtActions::tree_info)});
    _actions.push_back(CtMenuAction{file_cat, "quit_app", "ct_quit-app", _("_Quit"), KB_CONTROL+"Q",
        _("Quit the Application"), sigc::mem_fun(*pActions, &CtActions::quit_or_hide_window)});
    _actions.push_back(CtMenuAction{file_cat, "exit_app", "ct_quit-app", _("E_xit CherryTree"), KB_CONTROL+KB_SHIFT+"Q",
        _("Exit from CherryTree"), sigc::mem_fun(*pActions, &CtActions::quit_window)});

    const char* editor_cat = _("Edit/Insert");
    _actions.push_back(CtMenuAction{editor_cat, "act_undo", "ct_undo", _("_Undo"), KB_CONTROL+"Z",
        _("Undo Last Operation"), sigc::mem_fun(*pActions, &CtActions::requested_step_back)});
    _actions.push_back(CtMenuAction{editor_cat, "act_redo", "ct_redo", _("_Redo"), KB_CONTROL+"Y",
        _("Redo Previously Discarded Operation"), sigc::mem_fun(*pActions, &CtActions::requested_step_ahead)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_image", "ct_image_insert", _("Insert I_mage"), KB_CONTROL+KB_ALT+"I",
        _("Insert an Image"), sigc::mem_fun(*pActions, &CtActions::image_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_latex", "ct_latex_insert", _("Insert Late_x"), KB_CONTROL+KB_ALT+"X",
        _("Insert LatexBox"), sigc::mem_fun(*pActions, &CtActions::latex_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_table", "ct_table_insert", _("Insert _Table"), KB_CONTROL+KB_ALT+"B",
        _("Insert a Table"), sigc::mem_fun(*pActions, &CtActions::table_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_codebox", "ct_codebox_insert", _("Insert _CodeBox"), KB_CONTROL+KB_ALT+"C",
        _("Insert a CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_embfile", "ct_file_icon", _("Insert _File"), KB_CONTROL+KB_ALT+"E",
        _("Insert File"), sigc::mem_fun(*pActions, &CtActions::embfile_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_link", "ct_link_handle", _("Insert/Edit _Link"), KB_CONTROL+"L",
        _("Insert a Link/Edit the Underlying Link"), sigc::mem_fun(*pActions, &CtActions::apply_tag_link)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_anchor", "ct_anchor_insert", _("Insert _Anchor"), KB_CONTROL+KB_ALT+"A",
        _("Insert an Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_handle)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_toc", "ct_index", _("Insert T_OC"), None,
        _("Insert Table of Contents"), sigc::mem_fun(*pActions, &CtActions::toc_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_timestamp", "ct_timestamp", _("Insert Timestam_p"), KB_CONTROL+KB_ALT+"M",
        _("Insert Timestamp"), sigc::mem_fun(*pActions, &CtActions::timestamp_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_special_char", "ct_insert", _("Insert _Special Character"), None,
        _("Insert a Special Character"), sigc::mem_fun(*pActions, &CtActions::special_char_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_horiz_rule", "ct_horiz_rule", _("Insert _Horizontal Rule"), KB_CONTROL+"R",
        _("Insert Horizontal Rule"), sigc::mem_fun(*pActions, &CtActions::horizontal_rule_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "case_down", "ct_case_lower", _("_Lower Case of Selection/Word"), KB_CONTROL+"W",
        _("Lower the Case of the Selection/the Underlying Word"), sigc::mem_fun(*pActions, &CtActions::text_selection_lower_case)});
    _actions.push_back(CtMenuAction{editor_cat, "case_up", "ct_case_upper", _("_Upper Case of Selection/Word"), KB_CONTROL+KB_SHIFT+"W",
        _("Upper the Case of the Selection/the Underlying Word"), sigc::mem_fun(*pActions, &CtActions::text_selection_upper_case)});
    _actions.push_back(CtMenuAction{editor_cat, "case_tggl", "ct_case_toggle", _("_Toggle Case of Selection/Word"), KB_CONTROL+"G",
        _("Toggle the Case of the Selection/the Underlying Word"), sigc::mem_fun(*pActions, &CtActions::text_selection_toggle_case)});
    _actions.push_back(CtMenuAction{editor_cat, "cut_plain", "ct_edit_cut", _("Cu_t as Plain Text"), KB_CONTROL+KB_SHIFT+"X",
        _("Cut as Plain Text, Discard the Rich Text Formatting"), sigc::mem_fun(*pActions, &CtActions::cut_as_plain_text)});
    _actions.push_back(CtMenuAction{editor_cat, "copy_plain", "ct_edit_copy", _("_Copy as Plain Text"), KB_CONTROL+KB_SHIFT+"C",
        _("Copy as Plain Text, Discard the Rich Text Formatting"), sigc::mem_fun(*pActions, &CtActions::copy_as_plain_text)});
    _actions.push_back(CtMenuAction{editor_cat, "paste_plain", "ct_edit_paste", _("_Paste as Plain Text"), KB_CONTROL+KB_SHIFT+"V",
        _("Paste as Plain Text, Discard the Rich Text Formatting"), sigc::mem_fun(*pActions, &CtActions::paste_as_plain_text)});
    _actions.push_back(CtMenuAction{editor_cat, "cut_row", "ct_edit_cut", _("C_ut Row"), KB_SHIFT+KB_ALT+"X",
        _("Cut the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_cut)});
    _actions.push_back(CtMenuAction{editor_cat, "copy_row", "ct_edit_copy", _("C_opy Row"), KB_SHIFT+KB_ALT+"C",
        _("Copy the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_copy)});
    _actions.push_back(CtMenuAction{editor_cat, "del_row", "ct_edit_delete", _("De_lete Row"), KB_CONTROL+"K",
        _("Delete the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_delete)});
    _actions.push_back(CtMenuAction{editor_cat, "dup_row", "ct_add", _("_Duplicate Row"), KB_CONTROL+"D",
        _("Duplicate the Current Row/Selection"), sigc::mem_fun(*pActions, &CtActions::text_row_selection_duplicate)});
    _actions.push_back(CtMenuAction{editor_cat, "mv_up_row", "ct_go-up", _("_Move Up Row"), KB_ALT+CtConst::STR_KEY_UP,
        _("Move Up the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_up)});
    _actions.push_back(CtMenuAction{editor_cat, "mv_down_row", "ct_go-down", _("Mo_ve Down Row"), KB_ALT+CtConst::STR_KEY_DOWN,
        _("Move Down the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_down)});

    const char* fmt_cat = _("Format");
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_clone", "ct_fmt-txt-clone", _("Format Clo_ne"), None,
        _("Clone the Text Format Type at Cursor"), sigc::mem_fun(*pActions, &CtActions::save_tags_at_cursor_as_latest)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_latest", "ct_fmt-txt-latest", _("Format _Latest"), "F7",
        _("Memory of Latest Text Format Type"), sigc::mem_fun(*pActions, &CtActions::apply_tags_latest)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_rm", "ct_fmt-txt-clear", _("_Remove Formatting"), KB_CONTROL+KB_SHIFT+"R",
        _("Remove the Formatting from the Selected Text"), sigc::mem_fun(*pActions, &CtActions::remove_text_formatting)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_color_fg", "ct_color_fg", _("Text _Color Foreground"), KB_SHIFT+KB_ALT+"F",
        _("Change the Color of the Selected Text Foreground"), sigc::mem_fun(*pActions, &CtActions::apply_tag_foreground)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_color_bg", "ct_color_bg", _("Text C_olor Background"), KB_SHIFT+KB_ALT+"B",
        _("Change the Color of the Selected Text Background"), sigc::mem_fun(*pActions, &CtActions::apply_tag_background)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_bold", "ct_fmt-txt-bold", _("Toggle _Bold Property"), KB_CONTROL+"B",
        _("Toggle Bold Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_bold)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_italic", "ct_fmt-txt-italic", _("Toggle _Italic Property"), KB_CONTROL+"I",
        _("Toggle Italic Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_italic)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_underline", "ct_fmt-txt-underline", _("Toggle _Underline Property"), KB_CONTROL+"U",
        _("Toggle Underline Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_underline)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_strikethrough", "ct_fmt-txt-strikethrough", _("Toggle Stri_kethrough Property"), KB_CONTROL+"E",
        _("Toggle Strikethrough Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_strikethrough)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h1", "ct_fmt-txt-h1", _("Toggle h_1 Property"), KB_CONTROL+"1",
        _("Toggle h1 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h1)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h2", "ct_fmt-txt-h2", _("Toggle h_2 Property"), KB_CONTROL+"2",
        _("Toggle h2 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h2)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h3", "ct_fmt-txt-h3", _("Toggle h_3 Property"), KB_CONTROL+"3",
        _("Toggle h3 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h3)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h4", "ct_fmt-txt-h4", _("Toggle h_4 Property"), KB_CONTROL+"4",
        _("Toggle h4 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h4)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h5", "ct_fmt-txt-h5", _("Toggle h_5 Property"), KB_CONTROL+"5",
        _("Toggle h5 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h5)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h6", "ct_fmt-txt-h6", _("Toggle h_6 Property"), KB_CONTROL+"6",
        _("Toggle h6 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h6)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_small", "ct_fmt-txt-small", _("Toggle _Small Property"), KB_CONTROL+"0",
        _("Toggle Small Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_small)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_superscript", "ct_fmt-txt-superscript", _("Toggle Su_perscript Property"), None,
        _("Toggle Superscript Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_superscript)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_subscript", "ct_fmt-txt-subscript", _("Toggle Su_bscript Property"), None,
        _("Toggle Subscript Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_subscript)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_monospace", "ct_fmt-txt-monospace", _("Toggle _Monospace Property"), KB_CONTROL+"M",
        _("Toggle Monospace Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_monospace)});
    _actions.push_back(CtMenuAction{fmt_cat, "handle_bull_list", "ct_list_bulleted", _("Set/Unset _Bulleted List"), KB_CONTROL+KB_ALT+"1",
        _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), sigc::mem_fun(*pActions, &CtActions::list_bulleted_handler)});
    _actions.push_back(CtMenuAction{fmt_cat, "handle_num_list", "ct_list_numbered", _("Set/Unset _Numbered List"), KB_CONTROL+KB_ALT+"2",
        _("Set/Unset the Current Paragraph/Selection as a Numbered List"), sigc::mem_fun(*pActions, &CtActions::list_numbered_handler)});
    _actions.push_back(CtMenuAction{fmt_cat, "handle_todo_list", "ct_list_todo", _("Set/Unset _To-Do List"), KB_CONTROL+KB_ALT+"3",
        _("Set/Unset the Current Paragraph/Selection as a To-Do List"), sigc::mem_fun(*pActions, &CtActions::list_todo_handler)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_left", "ct_justify-left", _("Justify _Left"), None,
        _("Justify Left the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_left)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_center", "ct_justify-center", _("Justify _Center"), None,
        _("Justify Center the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_center)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_right", "ct_justify-right", _("Justify _Right"), None,
        _("Justify Right the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_right)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_fill", "ct_justify-fill", _("Justify _Fill"), None,
        _("Justify Fill the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_fill)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_indent", "ct_fmt-indent", _("_Indent Paragraph"), KB_CONTROL+"greater",
        _("Indent the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_indent)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_unindent", "ct_fmt-unindent", _("_Unindent Paragraph"), KB_CONTROL+"less",
        _("Unindent the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::reduce_tag_indent)});

    const char* tools_cat = _("Tools");
    _actions.push_back(CtMenuAction{tools_cat, "spellcheck_toggle", "ct_spell-check", _("Enable/Disable _Spell Check"), KB_CONTROL+KB_ALT+"S",
        _("Toggle Enable/Disable Spell Check"), sigc::mem_fun(*pActions, &CtActions::toggle_ena_dis_spellcheck)});
    _actions.push_back(CtMenuAction{tools_cat, "strip_trail_spaces", "ct_clear", _("Stri_p Trailing Spaces"), None,
        _("Strip Trailing Spaces"), sigc::mem_fun(*pActions, &CtActions::strip_trailing_spaces)});
    _actions.push_back(CtMenuAction{tools_cat, "repl_tabs_spaces", "ct_find_replace", _("_Replace Tabs with Spaces"), None,
        _("Replace Tabs with Spaces"), sigc::mem_fun(*pActions, &CtActions::replace_tabs_with_spaces)});
    _actions.push_back(CtMenuAction{tools_cat, "command_palette", "ct_execute", _("_Command Palette"), KB_CONTROL+KB_SHIFT+"P",
        _("Command Palette"), sigc::mem_fun(*pActions, &CtActions::command_palette)});
    _actions.push_back(CtMenuAction{tools_cat, "exec_code_all", "ct_play", _("_Execute Code All"), "F5",
        _("Execute All Code in CodeBox or Node"), sigc::mem_fun(*pActions, &CtActions::exec_code_all)});
    _actions.push_back(CtMenuAction{tools_cat, "exec_code_los", "ct_play", _("E_xecute Code Line or Selection"), KB_CONTROL+"F5",
        _("Execute Code from Current Line or Selected Text"), sigc::mem_fun(*pActions, &CtActions::exec_code_line_or_selection)});

    const char* tree_cat = _("Tree");
    _actions.push_back(CtMenuAction{tree_cat, "go_node_prev", "ct_go-back", _("Go _Back"), KB_ALT+CtConst::STR_KEY_LEFT,
        _("Go to the Previous Visited Node"), sigc::mem_fun(*pActions, &CtActions::node_go_back)});
    _actions.push_back(CtMenuAction{tree_cat, "go_node_next", "ct_go-forward", _("Go _Forward"), KB_ALT+CtConst::STR_KEY_RIGHT,
        _("Go to the Next Visited Node"), sigc::mem_fun(*pActions, &CtActions::node_go_forward)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_add_node", "ct_tree-node-add", _("Add _Node"), KB_CONTROL+"N",
        _("Add a Node having the same Parent of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_add)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_add_subnode", "ct_tree-subnode-add", _("Add _Subnode"), KB_CONTROL+KB_SHIFT+"N",
        _("Add a Child Node to the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_child_add)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_dup_node", "ct_tree-node-dupl", _("_Duplicate Node"), KB_CONTROL+KB_SHIFT+"D",
        _("Duplicate the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_duplicate)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_dup_node_subnodes", "ct_tree-nodesub-dupl", _("Duplicate Node _and Subnodes"), None,
        _("Duplicate the Selected Node and the Subnodes"), sigc::mem_fun(*pActions, &CtActions::node_subnodes_duplicate)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_copy_node_subnodes", "ct_edit_copy", _("Copy Node and S_ubnodes"), None,
        _("Copy the Selected Node and the Subnodes"), sigc::mem_fun(*pActions, &CtActions::node_subnodes_copy)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_paste_node_subnodes", "ct_edit_paste", _("_Paste Node and Subnodes"), None,
        _("Paste the Copied Node and Subnodes"), sigc::mem_fun(*pActions, &CtActions::node_subnodes_paste)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_date", "ct_calendar", _("Insert _Today's Node"), "F8",
        _("Insert a Node with Hierarchy Year/Month/Day"), sigc::mem_fun(*pActions, &CtActions::node_date)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_prop", "ct_cherry_edit", _("C_hange Node Properties"), "F2",
        _("Edit the Properties of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_edit)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_toggle_ro", "ct_locked", _("Toggle _Read Only"), KB_CONTROL+KB_ALT+"R",
        _("Toggle the Read Only Property of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_toggle_read_only)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_link", "ct_node_link", _("Cop_y Link to Node"), None,
        _("Copy Link to the Selected Node to Clipboard"), sigc::mem_fun(*pActions, &CtActions::node_link_to_clipboard)});
    _actions.push_back(CtMenuAction{tree_cat, "child_nodes_inherit_syntax", "ct_execute", _("Children _Inherit Syntax"), None,
        _("Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting"),
        sigc::mem_fun(*pActions, &CtActions::node_inherit_syntax)});
    _actions.push_back(CtMenuAction{tree_cat, "handle_bookmarks", "ct_edit", _("_Handle Bookmarks"), None,
        _("Handle the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmarks_handle)});
    _actions.push_back(CtMenuAction{tree_cat, "node_bookmark", "ct_pin-add", _("Add to Boo_kmarks"), KB_CONTROL+KB_SHIFT+"B",
        _("Add the Current Node to the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmark_curr_node)});
    _actions.push_back(CtMenuAction{tree_cat, "node_unbookmark", "ct_pin-remove", _("_Remove from Bookmarks"), KB_CONTROL+KB_ALT+"B",
        _("Remove the Current Node from the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmark_curr_node_remove)});
    _actions.push_back(CtMenuAction{tree_cat, "nodes_all_expand", "ct_zoom-in", _("E_xpand All Nodes"), KB_CONTROL+KB_SHIFT+"E",
        _("Expand All the Tree Nodes"), sigc::mem_fun(*pActions, &CtActions::nodes_expand_all)});
    _actions.push_back(CtMenuAction{tree_cat, "nodes_all_collapse", "ct_zoom-out", _("_Collapse All Nodes"), KB_CONTROL+KB_SHIFT+"L",
        _("Collapse All the Tree Nodes"), sigc::mem_fun(*pActions, &CtActions::nodes_collapse_all)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_up", "ct_go-up", _("Node _Up"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_UP,
        _("Move the Selected Node Up"), sigc::mem_fun(*pActions, &CtActions::node_up)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_down", "ct_go-down", _("Node _Down"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_DOWN,
        _("Move the Selected Node Down"), sigc::mem_fun(*pActions, &CtActions::node_down)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_left", "ct_go-back", _("Node _Left"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_LEFT,
        _("Move the Selected Node Left"), sigc::mem_fun(*pActions, &CtActions::node_left)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_right", "ct_go-forward", _("Node _Right"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_RIGHT,
        _("Move the Selected Node Right"), sigc::mem_fun(*pActions, &CtActions::node_right)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_new_father", "ct_go-jump", _("Node Change _Parent"), KB_SHIFT+KB_ALT+"P",
        _("Change the Selected Node's Parent"), sigc::mem_fun(*pActions, &CtActions::node_change_father)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_all_sort_asc", "ct_sort-asc", _("Sort Tree _Ascending"), None,
        _("Sort the Tree Ascending"), sigc::mem_fun(*pActions, &CtActions::tree_sort_ascending)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_all_sort_desc", "ct_sort-desc", _("Sort Tree _Descending"), None,
        _("Sort the Tree Descending"), sigc::mem_fun(*pActions, &CtActions::tree_sort_descending)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_sibl_sort_asc", "ct_sort-asc", _("Sort Siblings A_scending"), None,
        _("Sort all the Siblings of the Selected Node Ascending"), sigc::mem_fun(*pActions, &CtActions::node_siblings_sort_ascending)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_sibl_sort_desc", "ct_sort-desc", _("Sort Siblings D_escending"), None,
        _("Sort all the Siblings of the Selected Node Descending"), sigc::mem_fun(*pActions, &CtActions::node_siblings_sort_descending)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_del", "ct_edit_delete", _("De_lete Node"), None,
        _("Delete the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_delete)});

    const char* find_cat = _("Find/Replace");
    _actions.push_back(CtMenuAction{find_cat, "find_in_node", "ct_find_sel", _("_Find in Node Content"), KB_CONTROL+"F",
        _("Find into the Selected Node Content"), sigc::mem_fun(*pActions, &CtActions::find_in_selected_node)});
    _actions.push_back(CtMenuAction{find_cat, "find_in_allnodes", "ct_find_all", _("Find _in Multiple Nodes"), KB_CONTROL+KB_SHIFT+"F",
        _("Find in Multiple Nodes"), sigc::mem_fun(*pActions, &CtActions::find_in_multiple_nodes_act)});
    _actions.push_back(CtMenuAction{find_cat, "find_in_node_names", "ct_find", _("Find in _Nodes Names and Tags"), KB_CONTROL+"T",
        _("Find in Nodes Names and Tags"), sigc::mem_fun(*pActions, &CtActions::find_a_node)});
    _actions.push_back(CtMenuAction{find_cat, "find_iter_fw", "ct_find_again", _("Find _Again"), "F3",
        _("Iterate the Last Find Operation"), sigc::mem_fun(*pActions, &CtActions::find_again)});
    _actions.push_back(CtMenuAction{find_cat, "find_iter_bw", "ct_find_back", _("Find _Back"), "F4",
        _("Iterate the Last Find Operation in Opposite Direction"), sigc::mem_fun(*pActions, &CtActions::find_back)});
    _actions.push_back(CtMenuAction{find_cat, "replace_in_node", "ct_replace_sel", _("_Replace in Node Content"), KB_CONTROL+"H",
        _("Replace into the Selected Node Content"), sigc::mem_fun(*pActions, &CtActions::replace_in_selected_node)});
    _actions.push_back(CtMenuAction{find_cat, "replace_in_allnodes", "ct_replace_all",
        _("Replace in _Multiple Nodes"), KB_CONTROL+KB_SHIFT+"H",
        _("Replace in Multiple Nodes"), sigc::mem_fun(*pActions, &CtActions::replace_in_multiple_nodes)});
    _actions.push_back(CtMenuAction{find_cat, "replace_iter_fw", "ct_replace_again", _("Replace A_gain"), "F6",
        _("Iterate the Last Replace Operation"), sigc::mem_fun(*pActions, &CtActions::replace_again)});
    _actions.push_back(CtMenuAction{find_cat, "toggle_show_allmatches_dlg", "ct_find",
        _("Show All Matches _Dialog"), KB_CONTROL+KB_SHIFT+"A",
        _("Show Search All Matches Dialog"), sigc::mem_fun(*pActions, &CtActions::find_allmatchesdialog_restore)});
    _actions.push_back(CtMenuAction{find_cat, "tree_clear_exclude_from_search", "ct_clear", _("_Clear All Exclusions From Search"), None,
        _("Clear All Tree Nodes Properties of Exclusions From Search"), sigc::mem_fun(*pActions, &CtActions::tree_clear_property_exclude_from_search)});

    const char* view_cat = _("View");
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_tree", "ct_cherries", _("Show/Hide _Tree Explorer"), "F9",
        _("Toggle Show/Hide Tree"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_tree)});
#if defined(HAVE_VTE)
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_vte", "ct_term", _("Show/Hide Te_rminal"), KB_CONTROL+"F9",
        _("Toggle Show/Hide Terminal"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_vte)});
#endif // HAVE_VTE
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_menubar", "ct_menubar", _("Show/Hide _Menubar"), "F12",
        _("Toggle Show/Hide Menubar"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_menubar)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_toolbar", "ct_toolbar", _("Show/Hide Tool_bar"), None,
        _("Toggle Show/Hide Toolbar"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_toolbars)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_statusbar", "ct_statusbar", _("Show/Hide _Statusbar"), None,
        _("Toggle Show/Hide Statusbar"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_statusbar)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_treelines", "ct_treelines", _("Show/Hide _Lines Node Parent to Children"), None,
        _("Toggle Show/Hide Lines Between Node Parent and Children"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_tree_lines)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_node_name_head", "ct_node_name_header", _("Show/Hide Node Name _Header"), None,
        _("Toggle Show/Hide Node Name Header"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_node_name_header)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_focus_tree_text", "ct_go-jump", _("Toggle _Focus Tree/Text"), KB_CONTROL+"Tab",
        _("Toggle Focus Between Tree and Text"), sigc::mem_fun(*pActions, &CtActions::toggle_tree_text)});
    _actions.push_back(CtMenuAction{view_cat, "toolbar_icons_size_p", "ct_add", _("_Increase Toolbar Icons Size"), None,
        _("Increase the Size of the Toolbar Icons"), sigc::mem_fun(*pActions, &CtActions::toolbar_icons_size_increase)});
    _actions.push_back(CtMenuAction{view_cat, "toolbar_icons_size_m", "ct_remove", _("_Decrease Toolbar Icons Size"), None,
        _("Decrease the Size of the Toolbar Icons"), sigc::mem_fun(*pActions, &CtActions::toolbar_icons_size_decrease)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_fullscreen", "ct_fullscreen", _("Full Screen _On/Off"), "F11",
        _("Toggle Full Screen On/Off"), sigc::mem_fun(*pActions, &CtActions::fullscreen_toggle)});
    if (_pCtConfig->menubarInTitlebar) {
        _actions.push_back(CtMenuAction{view_cat, "menubar_in_titlebar", "ct_mb_in_tb_no", _("Disable Menubar i_n Titlebar"), None,
            _("Do Not Place the Menubar in the Titlebar"), sigc::mem_fun(*pActions, &CtActions::disable_menubar_in_titlebar)});
    }
    else {
        _actions.push_back(CtMenuAction{view_cat, "menubar_in_titlebar", "ct_mb_in_tb_yes", _("Enable Menubar i_n Titlebar"), None,
            _("Place the Menubar in the Titlebar"), sigc::mem_fun(*pActions, &CtActions::enable_menubar_in_titlebar)});
    }

    const char* import_cat = _("Import");
    _actions.push_back(CtMenuAction{import_cat, "import_cherrytree", "ct_from_cherrytree", _("From _CherryTree File"), None,
        _("Add Nodes of a CherryTree File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_ct_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_txt_file", "ct_from_txt", _("From _Plain Text File"), None,
        _("Add Node from a Plain Text File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_node_from_plaintext_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_txt_folder", "ct_from_txt", _("From _Folder of Plain Text Files"), None,
        _("Add Nodes from a Folder of Plain Text Files to the Current Tree"),
        sigc::mem_fun(*pActions, &CtActions::import_nodes_from_plaintext_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_html_file", "ct_from_html", _("From _HTML File"), None,
        _("Add Node from an HTML File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_node_from_html_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_html_folder", "ct_from_html", _("From _Folder of HTML Files"), None,
        _("Add Nodes from a Folder of HTML Files to the Current Tree"),
        sigc::mem_fun(*pActions, &CtActions::import_node_from_html_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_md_file",  CtConst::STR_STOCK_CT_IMP, _("From _Markdown File"), None,
        _("Add a node from a Markdown File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_node_from_md_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_md_folder",  CtConst::STR_STOCK_CT_IMP, _("From _Folder of Markdown Files"), None,
        _("Add Nodes from a Folder of Markdown Files to the Current Tree"),
        sigc::mem_fun(*pActions, &CtActions::import_nodes_from_md_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_gnote", CtConst::STR_STOCK_CT_IMP, _("From _Gnote Folder"), None,
        _("Add Nodes of a Gnote Folder to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_gnote_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_keepnote", CtConst::STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None,
        _("Add Nodes of a KeepNote Folder to the Current Tree"),
        sigc::mem_fun(*pActions, &CtActions::import_nodes_from_keepnote_directory) /* dad.nodes_add_from_keepnote_folder */});
    _actions.push_back(CtMenuAction{import_cat, "import_leo", CtConst::STR_STOCK_CT_IMP, _("From _Leo File"), None,
        _("Add Nodes of a Leo File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_leo_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_mempad", CtConst::STR_STOCK_CT_IMP, _("From _Mempad File"), None,
        _("Add Nodes of a Mempad File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_mempad_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_notecase", CtConst::STR_STOCK_CT_IMP, _("From _NoteCase HTML File"), None,
        _("Add Nodes of a NoteCase HTML File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_notecase_html)});
    _actions.push_back(CtMenuAction{import_cat, "import_rednotebook", CtConst::STR_STOCK_CT_IMP, _("From _RedNotebook HTML"), None,
        _("Add Nodes of a RedNotebook HTML file to the Current Tree"),
        sigc::mem_fun(*pActions, &CtActions::import_nodes_from_rednotebook_html)});
    _actions.push_back(CtMenuAction{import_cat, "import_tomboy", CtConst::STR_STOCK_CT_IMP, _("From T_omboy Folder"), None,
        _("Add Nodes of a Tomboy Folder to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_tomboy_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_treepad", CtConst::STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None,
        _("Add Nodes of a Treepad Lite File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_treepad_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_zim", CtConst::STR_STOCK_CT_IMP, _("From _Zim Folder"), None,
        _("Add Nodes of a Zim Folder to the Current Tree"),
        sigc::mem_fun(*pActions, &CtActions::import_nodes_from_zim_directory) /* dad.nodes_add_from_zim_folder */});
    _actions.push_back(CtMenuAction{import_cat, "import_pandoc_file", CtConst::STR_STOCK_CT_IMP, _("From File using _Pandoc"), None,
        _("Add a node to the current tree using Pandoc"), sigc::mem_fun(*pActions, &CtActions::import_node_from_pandoc) });

    const char* export_cat = _("Export");
    _actions.push_back(CtMenuAction{export_cat, "export_pdf", "ct_to_pdf", _("Export To _PDF"), None,
        _("Export To PDF"), sigc::mem_fun(*pActions, &CtActions::export_to_pdf)});
    _actions.push_back(CtMenuAction{export_cat, "export_html", "ct_to_html", _("Export To _HTML"), None,
        _("Export To HTML"), sigc::mem_fun(*pActions, &CtActions::export_to_html)});
    _actions.push_back(CtMenuAction{export_cat, "export_txt", "ct_to_txt", _("Export to Plain _Text"), None,
        _("Export to Plain Text"), sigc::mem_fun(*pActions, &CtActions::export_to_txt)});
    _actions.push_back(CtMenuAction{export_cat, "export_ctd", "ct_to_cherrytree", _("_Export To CherryTree Document"), None,
        _("Export To CherryTree Document"), sigc::mem_fun(*pActions, &CtActions::export_to_ctd)});

    const char* help_cat = _("Help");
    _actions.push_back(CtMenuAction{help_cat, "ct_check_newer", "ct_network", _("_Check Newer Version"), None,
        _("Check for a Newer Version"), sigc::mem_fun(*pActions, &CtActions::check_for_newer_version)});
    _actions.push_back(CtMenuAction{help_cat, "ct_help", "ct_help", _("Online _Manual"), "F1",
        _("Application's Online Manual"), sigc::mem_fun(*pActions, &CtActions::online_help)});
    _actions.push_back(CtMenuAction{help_cat, "ct_about", "ct_about", _("_About"), None,
        _("About CherryTree"), sigc::mem_fun(*pActions, &CtActions::dialog_about)});
    _actions.push_back(CtMenuAction{help_cat, "open_cfg_folder", "ct_directory", _("_Open Preferences Directory"), None,
        _("Open the Directory with Preferences Files"), sigc::mem_fun(*pActions, &CtActions::folder_cfg_open)});

    // add actions in the Windows for the toolbar
    // by default actions will have prefix 'win.'
    // (the menu uses not actions, but accelerators)
    for (const CtMenuAction& action : _actions) {
        pActions->getCtMainWin()->add_action(action.id, action.run_action);
    }

    const char* others_cat = "";
    _actions.push_back(CtMenuAction{others_cat, "anch_cut", "ct_edit_cut", _("C_ut Anchor"), None,
        _("Cut the Selected Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_cut)});
    _actions.push_back(CtMenuAction{others_cat, "anch_copy", "ct_edit_copy", _("_Copy Anchor"), None,
        _("Copy the Selected Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_copy)});
    _actions.push_back(CtMenuAction{others_cat, "anch_del", "ct_edit_delete", _("_Delete Anchor"), None,
        _("Delete the Selected Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_delete)});
    _actions.push_back(CtMenuAction{others_cat, "anch_edit", "ct_anchor_edit", _("Edit _Anchor"), None,
        _("Edit the Underlying Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_edit)});
    _actions.push_back(CtMenuAction{others_cat, "anch_link", "ct_anchor_link", _("Copy Anchor Link"), None,
        _("Copy Link to the Underlying Anchor to Clipboard"), sigc::mem_fun(*pActions, &CtActions::anchor_link_to_clipboard)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_cut", "ct_edit_cut", _("C_ut Embedded File"), None,
        _("Cut the Selected Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_cut)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_copy", "ct_edit_copy", _("_Copy Embedded File"), None,
        _("Copy the Selected Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_copy)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_del", "ct_edit_delete", _("_Delete Embedded File"), None,
        _("Delete the Selected Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_delete)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_save", "ct_save-as", _("Save _As"), None,
        _("Save File As"), sigc::mem_fun(*pActions, &CtActions::embfile_save)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_open", "ct_open", _("_Open File"), None,
        _("Open Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_open)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_rename", "ct_edit", _("_Rename"), None,
        _("Rename Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_rename)});
    _actions.push_back(CtMenuAction{others_cat, "tex_save", "ct_latex_save", _("_Save LatexBox as PNG"), None,
        _("Save the Selected LatexBox as a PNG file"), sigc::mem_fun(*pActions, &CtActions::latex_save)});
    _actions.push_back(CtMenuAction{others_cat, "tex_edit", "ct_latex_edit", _("_Edit LatexBox"), None,
        _("Edit the Selected LatexBox"), sigc::mem_fun(*pActions, &CtActions::latex_edit)});
    _actions.push_back(CtMenuAction{others_cat, "tex_cut", "ct_edit_cut", _("C_ut LatexBox"), None,
        _("Cut the Selected LatexBox"), sigc::mem_fun(*pActions, &CtActions::latex_cut)});
    _actions.push_back(CtMenuAction{others_cat, "tex_copy", "ct_edit_copy", _("_Copy LatexBox"), None,
        _("Copy the Selected LatexBox"), sigc::mem_fun(*pActions, &CtActions::latex_copy)});
    _actions.push_back(CtMenuAction{others_cat, "tex_del", "ct_edit_delete", _("_Delete LatexBox"), None,
        _("Delete the Selected LatexBox"), sigc::mem_fun(*pActions, &CtActions::latex_delete)});
    _actions.push_back(CtMenuAction{others_cat, "img_save", "ct_image_save", _("_Save Image as PNG"), None,
        _("Save the Selected Image as a PNG file"), sigc::mem_fun(*pActions, &CtActions::image_save)});
    _actions.push_back(CtMenuAction{others_cat, "img_edit", "ct_image_edit", _("_Edit Image"), None,
        _("Edit the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_edit)});
    _actions.push_back(CtMenuAction{others_cat, "img_cut", "ct_edit_cut", _("C_ut Image"), None,
        _("Cut the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_cut)});
    _actions.push_back(CtMenuAction{others_cat, "img_copy", "ct_edit_copy", _("_Copy Image"), None,
        _("Copy the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_copy)});
    _actions.push_back(CtMenuAction{others_cat, "img_del", "ct_edit_delete", _("_Delete Image"), None,
        _("Delete the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_delete)});
    _actions.push_back(CtMenuAction{others_cat, "img_link_edit", "ct_link_handle", _("Edit _Link"), None,
        _("Edit the Link Associated to the Image"), sigc::mem_fun(*pActions, &CtActions::image_link_edit)});
    _actions.push_back(CtMenuAction{others_cat, "img_link_dismiss", "ct_clear", _("D_ismiss Link"), None,
        _("Dismiss the Link Associated to the Image"), sigc::mem_fun(*pActions, &CtActions::image_link_dismiss)});
    _actions.push_back(CtMenuAction{others_cat, "toggle_show_mainwin", CtConst::APP_NAME, _("Show/Hide _CherryTree"), None,
        _("Toggle Show/Hide CherryTree"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_main_window)});

    const char* link_cat = "";
    _actions.push_back(CtMenuAction{link_cat, "apply_tag_link", "ct_link_handle", _("Edit _Link"), None,
        _("Edit the Underlying Link"), sigc::mem_fun(*pActions, &CtActions::apply_tag_link)});
    _actions.push_back(CtMenuAction{link_cat, "link_cut", "ct_edit_cut", _("C_ut Link"), None,
        _("Cut the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_cut)});
    _actions.push_back(CtMenuAction{link_cat, "link_copy", "ct_edit_copy", _("_Copy Link"), None,
        _("Copy the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_copy)});
    _actions.push_back(CtMenuAction{link_cat, "link_dismiss", "ct_clear", _("D_ismiss Link"), None,
        _("Dismiss the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_dismiss)});
    _actions.push_back(CtMenuAction{link_cat, "link_delete", "ct_edit_delete", _("_Delete Link"), None,
        _("Delete the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_delete)});

    const char* table_cat = "";
    _actions.push_back(CtMenuAction{table_cat, "table_cut", "ct_edit_cut", _("C_ut Table"), None,
        _("Cut the Selected Table"), sigc::mem_fun(*pActions, &CtActions::table_cut)});
    _actions.push_back(CtMenuAction{table_cat, "table_copy", "ct_edit_copy", _("_Copy Table"), None,
        _("Copy the Selected Table"), sigc::mem_fun(*pActions, &CtActions::table_copy)});
    _actions.push_back(CtMenuAction{table_cat, "table_delete", "ct_edit_delete", _("_Delete Table"), None,
        _("Delete the Selected Table"), sigc::mem_fun(*pActions, &CtActions::table_delete)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_add", "ct_add", _("_Add Column"), None,
        _("Add a Table Column"), sigc::mem_fun(*pActions, &CtActions::table_column_add)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_delete", "ct_edit_delete", _("De_lete Column"), None,
        _("Delete the Selected Table Column"), sigc::mem_fun(*pActions, &CtActions::table_column_delete)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_left", "ct_go-back", _("Move Column _Left"), KB_CONTROL+"braceleft",
        _("Move the Selected Column Left"), sigc::mem_fun(*pActions, &CtActions::table_column_left)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_right", "ct_go-forward", _("Move Column _Right"), KB_CONTROL+"braceright",
        _("Move the Selected Column Right"), sigc::mem_fun(*pActions, &CtActions::table_column_right)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_increase_width", "ct_go-forward",
        _("Increase Column Width"), KB_CONTROL+"parenleft",
        _("Increase the Width of the Column"), sigc::mem_fun(*pActions, &CtActions::table_column_increase_width)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_decrease_width", "ct_go-back",
        _("Decrease Column Width"), KB_CONTROL+KB_ALT+"parenleft",
        _("Decrease the Width of the Column"), sigc::mem_fun(*pActions, &CtActions::table_column_decrease_width)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_add", "ct_add", _("_Add Row"), KB_CONTROL+"comma",
        _("Add a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_add)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_cut", "ct_edit_cut", _("Cu_t Row"), None,
        _("Cut a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_cut)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_copy", "ct_edit_copy", _("_Copy Row"), None,
        _("Copy a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_copy)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_paste", "ct_edit_paste", _("_Paste Row"), None,
        _("Paste a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_paste)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_delete", "ct_edit_delete", _("De_lete Row"), KB_CONTROL+KB_ALT+"comma",
        _("Delete the Selected Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_delete)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_up", "ct_go-up", _("Move Row _Up"), KB_CONTROL+"bracketleft",
        _("Move the Selected Row Up"), sigc::mem_fun(*pActions, &CtActions::table_row_up)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_down", "ct_go-down", _("Move Row _Down"), KB_CONTROL+"bracketright",
        _("Move the Selected Row Down"), sigc::mem_fun(*pActions, &CtActions::table_row_down)});
    _actions.push_back(CtMenuAction{table_cat, "table_rows_sort_descending", "ct_sort-desc", _("Sort Rows De_scending"), None,
        _("Sort all the Rows Descending"), sigc::mem_fun(*pActions, &CtActions::table_rows_sort_descending)});
    _actions.push_back(CtMenuAction{table_cat, "table_rows_sort_ascending", "ct_sort-asc", _("Sort Rows As_cending"), None,
        _("Sort all the Rows Ascending"), sigc::mem_fun(*pActions, &CtActions::table_rows_sort_ascending)});
    _actions.push_back(CtMenuAction{table_cat, "table_edit_properties", "ct_table_edit", _("_Edit Table Properties"), None,
        _("Edit the Table Properties"), sigc::mem_fun(*pActions, &CtActions::table_edit_properties)});
    _actions.push_back(CtMenuAction{table_cat, "table_export", "ct_table_save", _("_Table Export"), None,
        _("Export Table as CSV File"), sigc::mem_fun(*pActions, &CtActions::table_export)});

    const char* codebox_cat = "";
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_change_properties", "ct_codebox_edit", _("Change CodeBox _Properties"), None,
        _("Edit the Properties of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_change_properties)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_load_from_file", "ct_from_txt", _("CodeBox _Load From Text File"), None,
        _("Load the CodeBox Content From a Text File"), sigc::mem_fun(*pActions, &CtActions::codebox_load_from_file)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_save_to_file", "ct_to_txt", _("CodeBox _Save To Text File"), None,
        _("Save the CodeBox Content To a Text File"), sigc::mem_fun(*pActions, &CtActions::codebox_save_to_file)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_cut", "ct_edit_cut", _("C_ut CodeBox"), None,
        _("Cut the Selected CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_cut)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_copy", "ct_edit_copy", _("_Copy CodeBox"), None,
        _("Copy the Selected CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_copy)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_delete", "ct_edit_delete", _("_Delete CodeBox"), None,
        _("Delete the Selected CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_delete)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_delete_keeping_text", "ct_edit_delete", _("Delete CodeBox _Keep Content"), None,
        _("Delete the Selected CodeBox But Keep Its Content"), sigc::mem_fun(*pActions, &CtActions::codebox_delete_keeping_text)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_increase_width", "ct_go-forward",
        _("Increase CodeBox Width"), KB_CONTROL+"parenleft",
        _("Increase the Width of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_increase_width)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_decrease_width", "ct_go-back",
        _("Decrease CodeBox Width"), KB_CONTROL+KB_ALT+"parenleft",
        _("Decrease the Width of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_decrease_width)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_increase_height", "ct_go-down",
        _("Increase CodeBox Height"), KB_CONTROL+"comma",
        _("Increase the Height of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_increase_height)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_decrease_height", "ct_go-up",
        _("Decrease CodeBox Height"), KB_CONTROL+KB_ALT+"comma",
        _("Decrease the Height of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_decrease_height)});

    const char* terminal_cat = "";
    _actions.push_back(CtMenuAction{terminal_cat, "term_copy", "ct_edit_copy", _("Copy Selection or All"), KB_CONTROL+KB_SHIFT+"C",
        _("Copy Selection or All"), sigc::mem_fun(*pActions, &CtActions::terminal_copy)});
    _actions.push_back(CtMenuAction{terminal_cat, "term_paste", "ct_edit_paste", _("Paste"), KB_CONTROL+KB_SHIFT+"V",
        _("Paste"), sigc::mem_fun(*pActions, &CtActions::terminal_paste)});
    _actions.push_back(CtMenuAction{terminal_cat, "term_reset", "ct_clear", _("Reset"), None,
        _("Reset"), sigc::mem_fun(*pActions, &CtActions::terminal_reset)});
}
