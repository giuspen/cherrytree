/*
 * ct_menu.cc
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

#include "ct_menu.h"
#include "ct_const.h"
#include <sigc++/sigc++.h>
#include <gtk/gtk.h>
#include <glib-object.h>


static xmlpp::Attribute* get_attribute(xmlpp::Node* pNode, char const* name)
{
    xmlpp::Element* pElement = static_cast<xmlpp::Element*>(pNode);
    return pElement->get_attribute(name);
}

static void on_menu_activate(void* pObject, CtAction* pAction)
{
    if (pAction)
    {
        pAction->run_action();
    }
}

const std::string& CtAction::get_shortcut() const
{
    auto it = CtApp::P_ctCfg->customKbShortcuts.find(id);
    return it != CtApp::P_ctCfg->customKbShortcuts.end() ? it->second : built_in_shortcut;
}



CtMenu::CtMenu()
{
    _pAccelGroup = gtk_accel_group_new();
    _rGtkBuilder = Gtk::Builder::create();
}

void CtMenu::init_actions(CtApp *pApp, CtActions* pActions)
{
    // stubs for menu bar
    _actions.push_back(CtAction{"", "FileMenu", None, _("_File"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "EditMenu", None, _("_Edit"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "FormattingMenu", None, _("For_matting"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "TreeMenu", None, _("_Tree"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "TreeMoveMenu", "gtk-jump-to", ("Node _Move"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "TreeSortMenu", "gtk-sort-ascending", _("Nodes _Sort"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "TreeImportMenu", CtConst::STR_STOCK_CT_IMP, _("Nodes _Import"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "TreeExportMenu", "export_from_cherrytree", _("Nodes E_xport"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "ChangeCaseMenu", "case_toggle", _("C_hange Case"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "SearchMenu", None, _("_Search"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "ViewMenu", None, _("_View"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "BookmarksMenu", None, _("_Bookmarks"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "ImportMenu", None, _("_Import"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "ExportMenu", None, _("E_xport"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"", "HelpMenu", None, _("_Help"), None, None, sigc::signal<void>()});

    // main actions
    const char* file_cat = _("File");
    _actions.push_back(CtAction{file_cat, "ct_new_inst", "new-instance", _("New _Instance"), None, _("Start a New Instance of CherryTree"), sigc::signal<void>() /* dad.file_new */});
    _actions.push_back(CtAction{file_cat, "ct_open_file", "gtk-open", _("_Open File"), KB_CONTROL+"O", _("Open a CherryTree Document"), sigc::signal<void>() /* dad.file_open */});
    _actions.push_back(CtAction{file_cat, "ct_save", "gtk-save", _("_Save"), KB_CONTROL+"S", _("Save File"), sigc::signal<void>() /* dad.file_save */});
    _actions.push_back(CtAction{file_cat, "ct_vacuum", "gtk-clear", _("Save and _Vacuum"), None, _("Save File and Vacuum"), sigc::signal<void>() /* dad.file_vacuum */});
    _actions.push_back(CtAction{file_cat, "ct_save_as", "gtk-save-as", _("Save _As"), KB_CONTROL+KB_SHIFT+"S", _("Save File As"), sigc::signal<void>() /* dad.file_save_as */});
    _actions.push_back(CtAction{file_cat, "exec_code", "gtk-execute", _("_Execute Code"), "F5", _("Execute Code"), sigc::signal<void>() /* dad.exec_code */});
    _actions.push_back(CtAction{file_cat, "open_cfg_folder", "gtk-directory", _("Open Preferences _Directory"), None, _("Open the Directory with Preferences Files"), sigc::signal<void>() /* dad.folder_cfg_open */});
    _actions.push_back(CtAction{file_cat, "print_page_setup", "gtk-print", _("Pa_ge Setup"), KB_CONTROL+KB_SHIFT+"P", _("Set up the Page for Printing"), sigc::signal<void>() /* dad.export_print_page_setup */});
    _actions.push_back(CtAction{file_cat, "do_print", "gtk-print", _("_Print"), KB_CONTROL+"P", _("Print"), sigc::signal<void>() /* dad.export_print */});
    _actions.push_back(CtAction{file_cat, "quit_app", "quit-app", _("_Quit"), KB_CONTROL+"Q", _("Quit the Application"), sigc::mem_fun(*pApp, &CtApp::quit_application) /* dad.quit_application */});
    _actions.push_back(CtAction{file_cat, "exit_app", "quit-app", _("_Exit CherryTree"), KB_CONTROL+KB_SHIFT+"Q", _("Exit from CherryTree"), sigc::signal<void>() /* dad.quit_application_totally */});
    _actions.push_back(CtAction{file_cat, "preferences_dlg", "gtk-preferences", _("_Preferences"), KB_CONTROL+KB_ALT+"P", _("Preferences"), sigc::mem_fun(*pApp, &CtApp::dialog_preferences) });
    _actions.push_back(CtAction{file_cat, "ct_check_newer", "gtk-network", _("_Check Newer Version"), None, _("Check for a Newer Version"), sigc::signal<void>() /* dad.check_for_newer_version */});
    _actions.push_back(CtAction{file_cat, "ct_help", "help-contents", _("Online _Manual"), "F1", _("Application's Online Manual"), sigc::signal<void>() /* dad.on_help_menu_item_activated */});
    _actions.push_back(CtAction{file_cat, "ct_about", "gtk-about", _("_About"), None, _("About CherryTree"), sigc::signal<void>() /* dad.dialog_about */});
    const char* editor_cat = _("Editor");
    _actions.push_back(CtAction{editor_cat, "act_undo", "gtk-undo", _("_Undo"), KB_CONTROL+"Z", _("Undo Last Operation"), sigc::signal<void>() /* dad.requested_step_back */});
    _actions.push_back(CtAction{editor_cat, "act_redo", "gtk-redo", _("_Redo"), KB_CONTROL+"Y", _("Redo Previously Discarded Operation"), sigc::signal<void>() /* dad.requested_step_ahead */});
    _actions.push_back(CtAction{editor_cat, "handle_image", "image_insert", _("Insert I_mage"), KB_CONTROL+KB_ALT+"I", _("Insert an Image"), sigc::signal<void>() /* dad.image_handle */});
    //_actions.push_back(CtAction{"handle_screenshot", "screenshot_insert", _("Insert _Screenshot"), KB_CONTROL+KB_SHIFT+KB_ALT+"S", _("Insert a Screenshot"), sigc::signal<void>() /* dad.screenshot_handle */});
    _actions.push_back(CtAction{editor_cat, "handle_table", "table_insert", _("Insert _Table"), KB_CONTROL+KB_ALT+"T", _("Insert a Table"), sigc::signal<void>() /* dad.table_handle */});
    _actions.push_back(CtAction{editor_cat, "handle_codebox", "codebox_insert", _("Insert _CodeBox"), KB_CONTROL+KB_ALT+"C", _("Insert a CodeBox"), sigc::signal<void>() /* dad.codebox_handle */});
    _actions.push_back(CtAction{editor_cat, "handle_embfile", "file_icon", _("Insert _File"), KB_CONTROL+KB_ALT+"E", _("Insert File"), sigc::signal<void>() /* dad.embfile_insert */});
    _actions.push_back(CtAction{editor_cat, "handle_link", "link_handle", _("Insert/Edit _Link"), KB_CONTROL+"L", _("Insert a Link/Edit the Underlying Link"), sigc::signal<void>() /* dad.apply_tag_link */});
    _actions.push_back(CtAction{editor_cat, "handle_anchor", "anchor_insert", _("Insert _Anchor"), KB_CONTROL+KB_ALT+"A", _("Insert an Anchor"), sigc::signal<void>() /* dad.anchor_handle */});
    _actions.push_back(CtAction{editor_cat, "insert_toc", "index", _("Insert T_OC"), None, _("Insert Table of Contents"), sigc::signal<void>() /* dad.toc_insert */});
    _actions.push_back(CtAction{editor_cat, "insert_timestamp", "timestamp", _("Insert Ti_mestamp"), KB_CONTROL+KB_ALT+"M", _("Insert Timestamp"), sigc::signal<void>() /* dad.timestamp_insert */});
    _actions.push_back(CtAction{editor_cat, "insert_horiz_rule", "horizontal_rule", _("Insert _Horizontal Rule"), KB_CONTROL+"R", _("Insert Horizontal Rule"), sigc::signal<void>() /* dad.horizontal_rule_insert */});
    _actions.push_back(CtAction{editor_cat, "case_down", "case_lower", _("_Lower Case of Selection/Word"), KB_CONTROL+"W", _("Lower the Case of the Selection/the Underlying Word"), sigc::signal<void>() /* dad.text_selection_lower_case */});
    _actions.push_back(CtAction{editor_cat, "case_up", "case_upper", _("_Upper Case of Selection/Word"), KB_CONTROL+KB_SHIFT+"W", _("Upper the Case of the Selection/the Underlying Word"), sigc::signal<void>() /* dad.text_selection_upper_case */});
    _actions.push_back(CtAction{editor_cat, "case_tggl", "case_toggle", _("_Toggle Case of Selection/Word"), KB_CONTROL+"G", _("Toggle the Case of the Selection/the Underlying Word"), sigc::signal<void>() /* dad.text_selection_toggle_case */});
    _actions.push_back(CtAction{editor_cat, "spellcheck_toggle", "gtk-spell-check", _("Enable/Disable _Spell Check"), KB_CONTROL+KB_ALT+"S", _("Toggle Enable/Disable Spell Check"), sigc::signal<void>() /* dad.toggle_ena_dis_spellcheck */});
    _actions.push_back(CtAction{editor_cat, "cut_plain", "edit-cut", _("Cu_t as Plain Text"), KB_CONTROL+KB_SHIFT+"X", _("Cut as Plain Text, Discard the Rich Text Formatting"), sigc::signal<void>() /* dad.cut_as_plain_text */});
    _actions.push_back(CtAction{editor_cat, "copy_plain", "edit-copy", _("_Copy as Plain Text"), KB_CONTROL+KB_SHIFT+"C", _("Copy as Plain Text, Discard the Rich Text Formatting"), sigc::signal<void>() /* dad.copy_as_plain_text */});
    _actions.push_back(CtAction{editor_cat, "paste_plain", "edit-paste", _("_Paste as Plain Text"), KB_CONTROL+KB_SHIFT+"V", _("Paste as Plain Text, Discard the Rich Text Formatting"), sigc::signal<void>() /* dad.paste_as_plain_text */});
    _actions.push_back(CtAction{editor_cat, "cut_row", "edit-cut", _("Cu_t Row"), KB_SHIFT+KB_ALT+"X", _("Cut the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_cut */});
    _actions.push_back(CtAction{editor_cat, "copy_row", "edit-copy", _("_Copy Row"), KB_SHIFT+KB_ALT+"C", _("Copy the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_copy */});
    _actions.push_back(CtAction{editor_cat, "del_row", "edit-delete", _("De_lete Row"), KB_CONTROL+"K", _("Delete the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_delete */});
    _actions.push_back(CtAction{editor_cat, "dup_row", "gtk-add", _("_Duplicate Row"), KB_CONTROL+"D", _("Duplicate the Current Row/Selection"), sigc::signal<void>() /* dad.text_row_selection_duplicate */});
    _actions.push_back(CtAction{editor_cat, "mv_up_row", "gtk-go-up", _("Move _Up Row"), KB_ALT+CtConst::STR_KEY_UP, _("Move Up the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_up */});
    _actions.push_back(CtAction{editor_cat, "mv_down_row", "gtk-go-down", _("Move _Down Row"), KB_ALT+CtConst::STR_KEY_DOWN, _("Move Down the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_down */});
    _actions.push_back(CtAction{editor_cat, "strip_trail_spaces", "gtk-clear", _("Stri_p Trailing Spaces"), None, _("Strip Trailing Spaces"), sigc::signal<void>() /* dad.strip_trailing_spaces */});
    const char* fmt_cat = _("Format");
    _actions.push_back(CtAction{fmt_cat, "fmt_latest", "format_text_latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), sigc::mem_fun(*pActions, &CtActions::apply_tag_latest)});
    _actions.push_back(CtAction{fmt_cat, "fmt_rm", "format_text_clear", _("_Remove Formatting"), KB_CONTROL+KB_SHIFT+"R", _("Remove the Formatting from the Selected Text"), sigc::mem_fun(*pActions, &CtActions::remove_text_formatting)});
    _actions.push_back(CtAction{fmt_cat, "fmt_color_fg", "color_foreground", _("Text _Color Foreground"), KB_SHIFT+KB_ALT+"F", _("Change the Color of the Selected Text Foreground"), sigc::mem_fun(*pActions, &CtActions::apply_tag_foreground)});
    _actions.push_back(CtAction{fmt_cat, "fmt_color_bg", "color_background", _("Text C_olor Background"), KB_SHIFT+KB_ALT+"B", _("Change the Color of the Selected Text Background"), sigc::mem_fun(*pActions, &CtActions::apply_tag_background)});
    _actions.push_back(CtAction{fmt_cat, "fmt_bold", "format-text-bold", _("Toggle _Bold Property"), KB_CONTROL+"B", _("Toggle Bold Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_bold)});
    _actions.push_back(CtAction{fmt_cat, "fmt_italic", "format-text-italic", _("Toggle _Italic Property"), KB_CONTROL+"I", _("Toggle Italic Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_italic)});
    _actions.push_back(CtAction{fmt_cat, "fmt_underline", "format-text-underline", _("Toggle _Underline Property"), KB_CONTROL+"U", _("Toggle Underline Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_underline)});
    _actions.push_back(CtAction{fmt_cat, "fmt_strikethrough", "format-text-strikethrough", _("Toggle Stri_kethrough Property"), KB_CONTROL+"E", _("Toggle Strikethrough Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_strikethrough)});
    _actions.push_back(CtAction{fmt_cat, "fmt_h1", "format-text-large", _("Toggle h_1 Property"), KB_CONTROL+"1", _("Toggle h1 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h1)});
    _actions.push_back(CtAction{fmt_cat, "fmt_h2", "format-text-large2", _("Toggle h_2 Property"), KB_CONTROL+"2", _("Toggle h2 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h2)});
    _actions.push_back(CtAction{fmt_cat, "fmt_h3", "format-text-large3", _("Toggle h_3 Property"), KB_CONTROL+"3", _("Toggle h3 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h3)});
    _actions.push_back(CtAction{fmt_cat, "fmt_small", "format-text-small", _("Toggle _Small Property"), KB_CONTROL+"0", _("Toggle Small Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_small)});
    _actions.push_back(CtAction{fmt_cat, "fmt_superscript", "format-text-superscript", _("Toggle Su_perscript Property"), None, _("Toggle Superscript Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_superscript)});
    _actions.push_back(CtAction{fmt_cat, "fmt_subscript", "format-text-subscript", _("Toggle Su_bscript Property"), None, _("Toggle Subscript Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_subscript)});
    _actions.push_back(CtAction{fmt_cat, "fmt_monospace", "format-text-monospace", _("Toggle _Monospace Property"), KB_CONTROL+"M", _("Toggle Monospace Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_monospace)});
    _actions.push_back(CtAction{fmt_cat, "handle_bull_list", "list_bulleted", _("Set/Unset _Bulleted List"), KB_CONTROL+KB_ALT+"1", _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), sigc::mem_fun(*pActions, &CtActions::list_bulleted_handler)});
    _actions.push_back(CtAction{fmt_cat, "handle_num_list", "list_numbered", _("Set/Unset _Numbered List"), KB_CONTROL+KB_ALT+"2", _("Set/Unset the Current Paragraph/Selection as a Numbered List"), sigc::mem_fun(*pActions, &CtActions::list_numbered_handler)});
    _actions.push_back(CtAction{fmt_cat, "handle_todo_list", "list_todo", _("Set/Unset _To-Do List"), KB_CONTROL+KB_ALT+"3", _("Set/Unset the Current Paragraph/Selection as a To-Do List"), sigc::mem_fun(*pActions, &CtActions::list_todo_handler)});
    _actions.push_back(CtAction{fmt_cat, "fmt_justify_left", "gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_left)});
    _actions.push_back(CtAction{fmt_cat, "fmt_justify_center", "gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_center)});
    _actions.push_back(CtAction{fmt_cat, "fmt_justify_right", "gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_right)});
    _actions.push_back(CtAction{fmt_cat, "fmt_justify_fill", "gtk-justify-fill", _("Justify _Fill"), None, _("Justify Fill the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_fill)});
    const char* tree_cat = _("Tree");
    _actions.push_back(CtAction{tree_cat, "tree_add_node", "tree-node-add", _("Add _Node"), KB_CONTROL+"N", _("Add a Node having the same Parent of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_add)});
    _actions.push_back(CtAction{tree_cat, "tree_add_subnode", "tree-subnode-add", _("Add _SubNode"), KB_CONTROL+KB_SHIFT+"N", _("Add a Child Node to the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_child_add)});
    _actions.push_back(CtAction{tree_cat, "tree_dup_node", "tree-node-dupl", _("_Duplicate Node"), KB_CONTROL+KB_SHIFT+"D", _("Duplicate the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_dublicate)});
    _actions.push_back(CtAction{tree_cat, "tree_node_prop", "cherry_edit", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_edit)});
    _actions.push_back(CtAction{tree_cat, "tree_node_toggle_ro", "locked", _("Toggle _Read Only"), KB_CONTROL+KB_ALT+"R", _("Toggle the Read Only Property of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_toggle_read_only)});
    _actions.push_back(CtAction{tree_cat, "tree_node_date", "calendar", _("Insert Today's Node"), "F8", _("Insert a Node with Hierarchy Year/Month/Day"), sigc::mem_fun(*pActions, &CtActions::node_date)});
    _actions.push_back(CtAction{tree_cat, "tree_parse_info", "gtk-info", _("Tree _Info"), None, _("Tree Summary Information"), sigc::signal<void>() /* dad.tree_info */});
    _actions.push_back(CtAction{tree_cat, "tree_node_up", "gtk-go-up", _("Node _Up"), KB_SHIFT+CtConst::STR_KEY_UP, _("Move the Selected Node Up"), sigc::mem_fun(*pActions, &CtActions::node_up)});
    _actions.push_back(CtAction{tree_cat, "tree_node_down", "gtk-go-down", _("Node _Down"), KB_SHIFT+CtConst::STR_KEY_DOWN, _("Move the Selected Node Down"), sigc::mem_fun(*pActions, &CtActions::node_down)});
    _actions.push_back(CtAction{tree_cat, "tree_node_left", "gtk-go-back", _("Node _Left"), KB_SHIFT+CtConst::STR_KEY_LEFT, _("Move the Selected Node Left"), sigc::mem_fun(*pActions, &CtActions::node_left)});
    _actions.push_back(CtAction{tree_cat, "tree_node_right", "gtk-go-forward", _("Node _Right"), KB_SHIFT+CtConst::STR_KEY_RIGHT, _("Move the Selected Node Right"), sigc::mem_fun(*pActions, &CtActions::node_right)});
    _actions.push_back(CtAction{tree_cat, "tree_node_new_father", "gtk-jump-to", _("Node Change _Parent"), KB_CONTROL+KB_SHIFT+CtConst::STR_KEY_RIGHT, _("Change the Selected Node's Parent"), sigc::mem_fun(*pActions, &CtActions::node_change_father)});
    _actions.push_back(CtAction{tree_cat, "tree_all_sort_asc", "gtk-sort-ascending", _("Sort Tree _Ascending"), None, _("Sort the Tree Ascending"), sigc::mem_fun(*pActions, &CtActions::tree_sort_ascending)});
    _actions.push_back(CtAction{tree_cat, "tree_all_sort_desc", "gtk-sort-descending", _("Sort Tree _Descending"), None, _("Sort the Tree Descending"), sigc::mem_fun(*pActions, &CtActions::tree_sort_descending)});
    _actions.push_back(CtAction{tree_cat, "tree_sibl_sort_asc", "gtk-sort-ascending", _("Sort Siblings A_scending"), None, _("Sort all the Siblings of the Selected Node Ascending"), sigc::mem_fun(*pActions, &CtActions::node_siblings_sort_ascending)});
    _actions.push_back(CtAction{tree_cat, "tree_sibl_sort_desc", "gtk-sort-descending", _("Sort Siblings D_escending"), None, _("Sort all the Siblings of the Selected Node Descending"), sigc::mem_fun(*pActions, &CtActions::node_siblings_sort_descending)});
    _actions.push_back(CtAction{tree_cat, "child_nodes_inherit_syntax", "gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting"), sigc::signal<void>() /* dad.node_inherit_syntax */});
    _actions.push_back(CtAction{tree_cat, "tree_node_del", "edit-delete", _("De_lete Node"), "Delete", _("Delete the Selected Node"), sigc::signal<void>() /* dad.node_delete */});
    _actions.push_back(CtAction{tree_cat, "node_bookmark", "pin-add", _("Add to _Bookmarks"), KB_CONTROL+KB_SHIFT+"B", _("Add the Current Node to the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmark_curr_node)});
    _actions.push_back(CtAction{tree_cat, "node_unbookmark", "pin-remove", _("_Remove from Bookmarks"), KB_CONTROL+KB_ALT+"B", _("Remove the Current Node from the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmark_curr_node_remove)});
    _actions.push_back(CtAction{tree_cat, "handle_bookmarks", "gtk-edit", _("_Handle Bookmarks"), None, _("Handle the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmarks_handle)});
    _actions.push_back(CtAction{tree_cat, "go_node_prev", "gtk-go-back", _("Go _Back"), KB_ALT+CtConst::STR_KEY_LEFT, _("Go to the Previous Visited Node"), sigc::signal<void>() /* dad.go_back */});
    _actions.push_back(CtAction{tree_cat, "go_node_next", "gtk-go-forward", _("Go _Forward"), KB_ALT+CtConst::STR_KEY_RIGHT, _("Go to the Next Visited Node"), sigc::signal<void>() /* dad.go_forward */});
    const char* find_cat = _("Find/Replace");
    _actions.push_back(CtAction{find_cat, "find_in_node", "find_sel", _("_Find in Node Content"), KB_CONTROL+"F", _("Find into the Selected Node Content"), sigc::mem_fun(*pActions, &CtActions::find_in_selected_node)});
    _actions.push_back(CtAction{find_cat, "find_in_allnodes", "find_all", _("Find in _All Nodes Contents"), KB_CONTROL+KB_SHIFT+"F", _("Find into All the Tree Nodes Contents"), sigc::mem_fun(*pActions, &CtActions::find_in_all_nodes)});
    _actions.push_back(CtAction{find_cat, "find_in_node_n_sub", "find_selnsub", _("Find in _Selected Node and Subnodes Contents"), KB_CONTROL+KB_ALT+"F", _("Find into the Selected Node and Subnodes Contents"), sigc::mem_fun(*pActions, &CtActions::find_in_sel_node_and_subnodes)});
    _actions.push_back(CtAction{find_cat, "find_in_node_names", "find", _("Find in _Nodes Names and Tags"), KB_CONTROL+"T", _("Find in Nodes Names and Tags"), sigc::mem_fun(*pActions, &CtActions::find_a_node)});
    _actions.push_back(CtAction{find_cat, "find_iter_fw", "find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), sigc::mem_fun(*pActions, &CtActions::find_again)});
    _actions.push_back(CtAction{find_cat, "find_iter_bw", "find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), sigc::mem_fun(*pActions, &CtActions::find_back)});
    _actions.push_back(CtAction{find_cat, "replace_in_node", "replace_sel", _("_Replace in Node Content"), KB_CONTROL+"H", _("Replace into the Selected Node Content"), sigc::mem_fun(*pActions, &CtActions::replace_in_selected_node)});
    _actions.push_back(CtAction{find_cat, "replace_in_allnodes", "replace_all", _("Replace in _All Nodes Contents"), KB_CONTROL+KB_SHIFT+"H", _("Replace into All the Tree Nodes Contents"), sigc::mem_fun(*pActions, &CtActions::replace_in_all_nodes)});
    _actions.push_back(CtAction{find_cat, "replace_in_node_n_sub", "replace_selnsub", _("Replace in _Selected Node and Subnodes Contents"), KB_CONTROL+KB_ALT+"H", _("Replace into the Selected Node and Subnodes Contents"), sigc::mem_fun(*pActions, &CtActions::replace_in_sel_node_and_subnodes)});
    _actions.push_back(CtAction{find_cat, "replace_in_node_names", "find_replace", _("Replace in Nodes _Names"), KB_CONTROL+KB_SHIFT+"T", _("Replace in Nodes Names"), sigc::mem_fun(*pActions, &CtActions::replace_in_nodes_names)});
    _actions.push_back(CtAction{find_cat, "replace_iter_fw", "replace_again", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), sigc::mem_fun(*pActions, &CtActions::replace_again)});
    _actions.push_back(CtAction{find_cat, "toggle_show_allmatches_dlg", "find", _("Show _All Matches Dialog"), KB_CONTROL+KB_SHIFT+"A", _("Show Search All Matches Dialog"), sigc::mem_fun(*pActions, &CtActions::find_allmatchesdialog_restore)});
    const char* view_cat = _("View");
    _actions.push_back(CtAction{view_cat, "toggle_show_tree", "cherries", _("Show/Hide _Tree"), "F9", _("Toggle Show/Hide Tree"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_tree)});
    _actions.push_back(CtAction{view_cat, "toggle_show_toolbar", "toolbar", _("Show/Hide Tool_bar"), None, _("Toggle Show/Hide Toolbar"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_toolbar)});
    _actions.push_back(CtAction{view_cat, "toggle_show_node_name_head", "node_name_header", _("Show/Hide Node Name _Header"), None, _("Toggle Show/Hide Node Name Header"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_node_name_header)});
    _actions.push_back(CtAction{view_cat, "toggle_focus_tree_text", "gtk-jump-to", _("Toggle _Focus Tree/Text"), KB_CONTROL+"Tab", _("Toggle Focus Between Tree and Text"), sigc::mem_fun(*pActions, &CtActions::toggle_tree_text)});
    _actions.push_back(CtAction{view_cat, "nodes_all_expand", "gtk-zoom-in", _("E_xpand All Nodes"), KB_CONTROL+KB_SHIFT+"E", _("Expand All the Tree Nodes"), sigc::mem_fun(*pActions, &CtActions::nodes_expand_all)});
    _actions.push_back(CtAction{view_cat, "nodes_all_collapse", "gtk-zoom-out", _("_Collapse All Nodes"), KB_CONTROL+KB_SHIFT+"L", _("Collapse All the Tree Nodes"), sigc::mem_fun(*pActions, &CtActions::nodes_collapse_all)});
    _actions.push_back(CtAction{view_cat, "toolbar_icons_size_p", "gtk-add", _("_Increase Toolbar Icons Size"), None, _("Increase the Size of the Toolbar Icons"), sigc::mem_fun(*pActions, &CtActions::toolbar_icons_size_increase)});
    _actions.push_back(CtAction{view_cat, "toolbar_icons_size_m", "gtk-remove", _("_Decrease Toolbar Icons Size"), None, _("Decrease the Size of the Toolbar Icons"), sigc::mem_fun(*pActions, &CtActions::toolbar_icons_size_decrease)});
    _actions.push_back(CtAction{view_cat, "toggle_fullscreen", "gtk-fullscreen", _("_Full Screen On/Off"), "F11", _("Toggle Full Screen On/Off"), sigc::mem_fun(*pActions, &CtActions::fullscreen_toggle)});
    const char* export_cat = _("Export");
    _actions.push_back(CtAction{export_cat, "export_pdf", "to_pdf", _("Export To _PDF"), None, _("Export To PDF"), sigc::signal<void>() /* dad.export_to_pdf */});
    _actions.push_back(CtAction{export_cat, "export_html", "to_html", _("Export To _HTML"), None, _("Export To HTML"), sigc::signal<void>() /* dad.export_to_html */});
    _actions.push_back(CtAction{export_cat, "export_txt_multiple", "to_txt", _("Export to Multiple Plain _Text Files"), None, _("Export to Multiple Plain Text Files"), sigc::signal<void>() /* dad.export_to_txt_multiple */});
    _actions.push_back(CtAction{export_cat, "export_txt_single", "to_txt", _("Export to _Single Plain Text File"), None, _("Export to Single Plain Text File"), sigc::signal<void>() /* dad.export_to_txt_single */});
    _actions.push_back(CtAction{export_cat, "export_ctd", "to_cherrytree", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), sigc::signal<void>() /* dad.export_to_ctd */});
    const char* import_cat = _("Import");
    _actions.push_back(CtAction{import_cat, "import_cherrytree", "from_cherrytree", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_cherrytree_file */});
    _actions.push_back(CtAction{import_cat, "import_txt_file", "from_txt", _("From _Plain Text File"), None, _("Add Node from a Plain Text File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_plain_text_file */});
    _actions.push_back(CtAction{import_cat, "import_txt_folder", "from_txt", _("From _Folder of Plain Text Files"), None, _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_plain_text_folder */});
    _actions.push_back(CtAction{import_cat, "import_html_file", "from_html", _("From _HTML File"), None, _("Add Node from an HTML File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_html_file */});
    _actions.push_back(CtAction{import_cat, "import_html_folder", "from_html", _("From _Folder of HTML Files"), None, _("Add Nodes from a Folder of HTML Files to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_html_folder */});
    _actions.push_back(CtAction{import_cat, "import_basket", CtConst::STR_STOCK_CT_IMP, _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_basket_folder */});
    _actions.push_back(CtAction{import_cat, "import_epim_html", CtConst::STR_STOCK_CT_IMP, _("From _EssentialPIM HTML File"), None, _("Add Node from an EssentialPIM HTML File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_epim_html_file */});
    _actions.push_back(CtAction{import_cat, "import_gnote", CtConst::STR_STOCK_CT_IMP, _("From _Gnote Folder"), None, _("Add Nodes of a Gnote Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_gnote_folder */});
    _actions.push_back(CtAction{import_cat, "import_keepnote", CtConst::STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_keepnote_folder */});
    _actions.push_back(CtAction{import_cat, "import_keynote", CtConst::STR_STOCK_CT_IMP, _("From K_eyNote File"), None, _("Add Nodes of a KeyNote File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_keynote_file */});
    _actions.push_back(CtAction{import_cat, "import_knowit", CtConst::STR_STOCK_CT_IMP, _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_knowit_file */});
    _actions.push_back(CtAction{import_cat, "import_leo", CtConst::STR_STOCK_CT_IMP, _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_leo_file */});
    _actions.push_back(CtAction{import_cat, "import_mempad", CtConst::STR_STOCK_CT_IMP, _("From _Mempad File"), None, _("Add Nodes of a Mempad File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_mempad_file */});
    _actions.push_back(CtAction{import_cat, "import_notecase", CtConst::STR_STOCK_CT_IMP, _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_notecase_file */});
    _actions.push_back(CtAction{import_cat, "import_rednotebook", CtConst::STR_STOCK_CT_IMP, _("From _RedNotebook Folder"), None, _("Add Nodes of a RedNotebook Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_rednotebook_folder */});
    _actions.push_back(CtAction{import_cat, "import_tomboy", CtConst::STR_STOCK_CT_IMP, _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_tomboy_folder */});
    _actions.push_back(CtAction{import_cat, "import_treepad", CtConst::STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_treepad_file */});
    _actions.push_back(CtAction{import_cat, "import_tuxcards", CtConst::STR_STOCK_CT_IMP, _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_tuxcards_file */});
    _actions.push_back(CtAction{import_cat, "import_zim", CtConst::STR_STOCK_CT_IMP, _("From _Zim Folder"), None, _("Add Nodes of a Zim Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_zim_folder */});
    const char* others_cat = "";
    _actions.push_back(CtAction{others_cat, "anch_cut", "edit-cut", _("C_ut Anchor"), None, _("Cut the Selected Anchor"), sigc::signal<void>() /* dad.anchor_cut */});
    _actions.push_back(CtAction{others_cat, "anch_copy", "edit-copy", _("_Copy Anchor"), None, _("Copy the Selected Anchor"), sigc::signal<void>() /* dad.anchor_copy */});
    _actions.push_back(CtAction{others_cat, "anch_del", "edit-delete", _("_Delete Anchor"), None, _("Delete the Selected Anchor"), sigc::signal<void>() /* dad.anchor_delete */});
    _actions.push_back(CtAction{others_cat, "anch_edit", "anchor_edit", _("Edit _Anchor"), None, _("Edit the Underlying Anchor"), sigc::signal<void>() /* dad.anchor_edit */});
    _actions.push_back(CtAction{others_cat, "emb_file_cut", "edit-cut", _("C_ut Embedded File"), None, _("Cut the Selected Embedded File"), sigc::signal<void>() /* dad.embfile_cut */});
    _actions.push_back(CtAction{others_cat, "emb_file_copy", "edit-copy", _("_Copy Embedded File"), None, _("Copy the Selected Embedded File"), sigc::signal<void>() /* dad.embfile_copy */});
    _actions.push_back(CtAction{others_cat, "emb_file_del", "edit-delete", _("_Delete Embedded File"), None, _("Delete the Selected Embedded File"), sigc::signal<void>() /* dad.embfile_delete */});
    _actions.push_back(CtAction{others_cat, "emb_file_save", "gtk-save-as", _("Save _As"), None, _("Save File As"), sigc::signal<void>() /* dad.embfile_save */});
    _actions.push_back(CtAction{others_cat, "emb_file_open", "gtk-open", _("_Open File"), None, _("Open Embedded File"), sigc::signal<void>() /* dad.embfile_open */});
    _actions.push_back(CtAction{others_cat, "img_save", "image_save", _("_Save Image as PNG"), None, _("Save the Selected Image as a PNG file"), sigc::signal<void>() /* dad.image_save */});
    _actions.push_back(CtAction{others_cat, "img_edit", "image_edit", _("_Edit Image"), None, _("Edit the Selected Image"), sigc::signal<void>() /* dad.image_edit */});
    _actions.push_back(CtAction{others_cat, "img_cut", "edit-cut", _("C_ut Image"), None, _("Cut the Selected Image"), sigc::signal<void>() /* dad.image_cut */});
    _actions.push_back(CtAction{others_cat, "img_copy", "edit-copy", _("_Copy Image"), None, _("Copy the Selected Image"), sigc::signal<void>() /* dad.image_copy */});
    _actions.push_back(CtAction{others_cat, "img_del", "edit-delete", _("_Delete Image"), None, _("Delete the Selected Image"), sigc::signal<void>() /* dad.image_delete */});
    _actions.push_back(CtAction{others_cat, "img_link_edit", "link_handle", _("Edit _Link"), None, _("Edit the Link Associated to the Image"), sigc::signal<void>() /* dad.image_link_edit */});
    _actions.push_back(CtAction{others_cat, "img_link_dismiss", "gtk-clear", _("D_ismiss Link"), None, _("Dismiss the Link Associated to the Image"), sigc::signal<void>() /* dad.image_link_dismiss */});
    _actions.push_back(CtAction{others_cat, "toggle_show_mainwin", CtConst::APP_NAME, _("Show/Hide _CherryTree"), None, _("Toggle Show/Hide CherryTree"), sigc::signal<void>() /* dad.toggle_show_hide_main_window */});

    // add actions in the Applicaton for the toolbar
    // by default actions will have prefix 'app.'
    // (the menu uses not actions, but accelerators)
    for (const CtAction& action: _actions)
    {
        pApp->add_action(action.id, action.run_action);
    }
}

CtAction* CtMenu::find_action(const std::string& id)
{
    for (CtAction& action : _actions)
    {
        if (action.id == id)
        {
            return &action;
        }
    }
    return nullptr;
}

GtkAccelGroup* CtMenu::default_accel_group()
{
    return _pAccelGroup;
}

Gtk::MenuItem* CtMenu::find_menu_item(Gtk::MenuBar* menuBar, std::string name)
{
    for (Gtk::Widget* child: menuBar->get_children())
        if (auto menuItem = dynamic_cast<Gtk::MenuItem*>(child)){
            if (menuItem->get_name() == name)
                return menuItem;
        }
    return nullptr;
}

Gtk::Toolbar* CtMenu::build_toolbar()
{
    Gtk::Toolbar* pToolbar = nullptr;
    _rGtkBuilder->add_from_string(get_toolbar_ui_str());
    _rGtkBuilder->get_widget("ToolBar", pToolbar);
    return pToolbar;
}

Gtk::MenuBar* CtMenu::build_menubar()
{
    return Glib::wrap(GTK_MENU_BAR(walk_menu_xml(gtk_menu_bar_new(), get_menu_ui_str(), nullptr)));
}

Gtk::Menu* CtMenu::build_popup_menu_node()
{
    // look for the TreeMenu node in the menubar to show it in popup
    return Glib::wrap(GTK_MENU(walk_menu_xml(gtk_menu_new(), get_menu_ui_str(), "/menubar/menu[@action='TreeMenu']/*")));
}

Gtk::Menu* CtMenu::build_popup_menu_text()
{
    return Glib::wrap(GTK_MENU(walk_menu_xml(gtk_menu_new(), get_popup_menu_text_ui_str(), nullptr)));
}

Gtk::Menu* CtMenu::build_popup_menu_code()
{
    return Glib::wrap(GTK_MENU(walk_menu_xml(gtk_menu_new(), get_popup_menu_code_ui_str(), nullptr)));
}

Gtk::Menu* CtMenu::build_popup_menu_link()
{
    GtkWidget* pMenu = gtk_menu_new();
    add_separator(pMenu);
    add_menu_item(pMenu, _("Edit _Link"), "link_handle", nullptr, _("Edit the Underlying Link"), nullptr /*dad.apply_tag_link*/);
    add_separator(pMenu);
    add_menu_item(pMenu, _("C_ut Link"), "edit-cut", nullptr, _("Cut the Selected Link"), nullptr /*dad.link_cut*/);
    add_menu_item(pMenu, _("_Copy Link"), "edit-copy", nullptr, _("Copy the Selected Link"), nullptr /*dad.link_copy*/);
    add_menu_item(pMenu, _("D_ismiss Link"), "gtk-clear", nullptr, _("Dismiss the Selected Link"), nullptr /*dad.link_dismiss*/);
    add_menu_item(pMenu, _("_Delete Link"), "edit-delete", nullptr, _("Delete the Selected Link"), nullptr /*dad.link_delete*/);
    return Glib::wrap(GTK_MENU(pMenu));
}

Gtk::Menu* CtMenu::build_popup_menu_table()
{
    GtkWidget* pMenu = gtk_menu_new();
    add_menu_item(pMenu, _("C_ut Table"), "edit-cut", nullptr, _("Cut the Selected Table"), nullptr /*dad.tables_handler.table_cut*/);
    add_menu_item(pMenu, _("_Copy Table"), "edit-copy", nullptr, _("Copy the Selected Table"), nullptr /*dad.tables_handler.table_copy*/);
    add_menu_item(pMenu, _("_Delete Table"), "edit-delete", nullptr, _("Delete the Selected Table"), nullptr /*dad.tables_handler.table_delete*/);
    add_separator(pMenu);
    add_menu_item(pMenu, _("_Add Row"), "gtk-add", (KB_CONTROL+"comma").c_str(), _("Add a Table Row"), nullptr /*dad.tables_handler.table_row_add*/);
    add_menu_item(pMenu, _("Cu_t Row"), "edit-cut", nullptr, _("Cut a Table Row"), nullptr /*dad.tables_handler.table_row_cut*/);
    add_menu_item(pMenu, _("_Copy Row"), "edit-copy", nullptr, _("Copy a Table Row"), nullptr /*dad.tables_handler.table_row_copy*/);
    add_menu_item(pMenu, _("_Paste Row"), "edit-paste", nullptr, _("Paste a Table Row"), nullptr /*dad.tables_handler.table_row_paste*/);
    add_menu_item(pMenu, _("De_lete Row"), "edit-delete", (KB_CONTROL+KB_ALT+"comma").c_str(), _("Delete the Selected Table Row"), nullptr /*dad.tables_handler.table_row_delete*/);
    add_separator(pMenu);
    add_menu_item(pMenu, _("Move Row _Up"), "gtk-go-up", (KB_CONTROL+KB_ALT+"period").c_str(), _("Move the Selected Row Up"), nullptr /*dad.tables_handler.table_row_up*/);
    add_menu_item(pMenu, _("Move Row _Down"), "gtk-go-down", (KB_CONTROL+"period").c_str(), _("Move the Selected Row Down"), nullptr /*dad.tables_handler.table_row_down*/);
    add_menu_item(pMenu, _("Sort Rows De_scending"), "gtk-sort-descending", nullptr, _("Sort all the Rows Descending"), nullptr /*dad.tables_handler.table_rows_sort_descending*/);
    add_menu_item(pMenu, _("Sort Rows As_cending"), "gtk-sort-ascending", nullptr, _("Sort all the Rows Ascending"), nullptr /*dad.tables_handler.table_rows_sort_ascending*/);
    add_separator(pMenu);
    add_menu_item(pMenu, _("_Edit Table Properties"), "table_edit", nullptr, _("Edit the Table Properties"), nullptr /*dad.tables_handler.table_edit_properties*/);
    add_menu_item(pMenu, _("_Table Export"), "table_save", nullptr, _("Export Table as CSV File"), nullptr /*dad.tables_handler.table_export*/);
    return Glib::wrap(GTK_MENU(pMenu));
}

Gtk::Menu* CtMenu::build_popup_menu_table_cell()
{
    GtkWidget* pMenu = gtk_menu_new();
    add_separator(pMenu);
    add_menu_item(pMenu, _("Insert _NewLine"), "insert", (KB_CONTROL+"period").c_str(), _("Insert NewLine Char"), nullptr /*dad.curr_table_cell_insert_newline*/);
    return Glib::wrap(GTK_MENU(pMenu));
}

Gtk::Menu* CtMenu::build_popup_menu_table_codebox()
{
    GtkWidget* pMenu = gtk_menu_new();
    add_separator(pMenu);
    add_menu_item(pMenu, _("Cu_t as Plain Text"), "edit-cut", (KB_CONTROL+KB_SHIFT+"X").c_str(), _("Cut as Plain Text, Discard the Rich Text Formatting"), nullptr /*dad.dad.cut_as_plain_text*/);
    add_menu_item(pMenu, _("_Copy as Plain Text"), "edit-copy", (KB_CONTROL+KB_SHIFT+"C").c_str(), _("Copy as Plain Text, Discard the Rich Text Formatting"), nullptr /*dad.dad.copy_as_plain_text*/);
    add_separator(pMenu);
    add_menu_item(pMenu, _("Change CodeBox _Properties"), "codebox_edit", nullptr, _("Edit the Properties of the CodeBox"), nullptr /*dad.codebox_change_properties*/);
    add_menu_item(pMenu, _("_Execute CodeBox Code"), "gtk-execute", nullptr, _("Execute CodeBox Code"), nullptr /*dad.dad.exec_code*/);
    add_menu_item(pMenu, _("CodeBox _Load From Text File"), "from_txt", nullptr, _("Load the CodeBox Content From a Text File"), nullptr /*dad.codebox_load_from_file*/);
    add_menu_item(pMenu, _("CodeBox _Save To Text File"), "to_txt", nullptr, _("Save the CodeBox Content To a Text File"), nullptr /*dad.codebox_save_to_file*/);
    add_separator(pMenu);
    add_menu_item(pMenu, _("C_ut CodeBox"), "edit-cut", nullptr, _("Cut the Selected CodeBox"), nullptr /*dad.codebox_cut*/);
    add_menu_item(pMenu, _("_Copy CodeBox"), "edit-copy", nullptr, _("Copy the Selected CodeBox"), nullptr /*dad.codebox_copy*/);
    add_menu_item(pMenu, _("_Delete CodeBox"), "edit-delete", nullptr, _("Delete the Selected CodeBox"), nullptr /*dad.codebox_delete*/);
    add_menu_item(pMenu, _("Delete CodeBox _Keep Content"), "edit-delete", nullptr, _("Delete the Selected CodeBox But Keep Its Content"), nullptr /*dad.codebox_delete_keeping_text*/);
    add_separator(pMenu);
    add_menu_item(pMenu, _("Increase CodeBox Width"), "gtk-go-forward", (KB_CONTROL+"period").c_str(), _("Increase the Width of the CodeBox"), nullptr /*dad.codebox_increase_width*/);
    add_menu_item(pMenu, _("Decrease CodeBox Width"), "gtk-go-back", (KB_CONTROL+KB_ALT+"period").c_str(), _("Decrease the Width of the CodeBox"), nullptr /*dad.codebox_decrease_width*/);
    add_menu_item(pMenu, _("Increase CodeBox Height"), "gtk-go-down", (KB_CONTROL+"comma").c_str(), _("Increase the Height of the CodeBox"), nullptr /*dad.codebox_increase_height*/);
    add_menu_item(pMenu, _("Decrease CodeBox Height"), "gtk-go-up", (KB_CONTROL+KB_ALT+"comma").c_str(), _("Decrease the Height of the CodeBox"), nullptr /*dad.codebox_decrease_height*/);
    return Glib::wrap(GTK_MENU(pMenu));
}

Gtk::Menu* CtMenu::build_bookmarks_menu(std::list<std::tuple<gint64, std::string>>& bookmarks, sigc::slot<void, gint64>& bookmark_action)
{
    Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu());
    add_menu_item(GTK_WIDGET(pMenu->gobj()), find_action("handle_bookmarks"));
    add_separator(GTK_WIDGET(pMenu->gobj()));
    for (const auto& bookmark: bookmarks)
    {
        const gint64& node_id = std::get<0>(bookmark);
        const std::string& node_name = std::get<1>(bookmark);
        Gtk::MenuItem* menuItem = add_menu_item(GTK_WIDGET(pMenu->gobj()), node_name.c_str(), "pin", nullptr, node_name.c_str(), nullptr);
        menuItem->signal_activate().connect(sigc::bind(bookmark_action, node_id));
    }
    return pMenu;
}

GtkWidget* CtMenu::walk_menu_xml(GtkWidget* pMenu, const char* document, const char* xpath)
{
    xmlpp::DomParser parser;
    parser.parse_memory(document);
    if (xpath)
    {
        walk_menu_xml(pMenu, parser.get_document()->get_root_node()->find(xpath)[0]);
    }
    else
    {
        walk_menu_xml(pMenu, parser.get_document()->get_root_node());
    }
    return pMenu;
}

void CtMenu::walk_menu_xml(GtkWidget* pMenu, xmlpp::Node* pNode)
{
    for (xmlpp::Node* pNodeIter = pNode; pNodeIter; pNodeIter = pNodeIter->get_next_sibling())
    {
        if (pNodeIter->get_name() == "menubar" || pNodeIter->get_name() == "popup")
        {
            walk_menu_xml(pMenu, pNodeIter->get_first_child());
        }
        else if (pNodeIter->get_name() == "menu")
        {
            if (xmlpp::Attribute* pAttrName = get_attribute(pNodeIter, "_name")) // menu name which need to be translated
            {
                xmlpp::Attribute* pAttrImage = get_attribute(pNodeIter, "image");
                GtkWidget* pSubmenu = add_submenu(pMenu, pAttrName->get_value().c_str(), _(pAttrName->get_value().c_str()), pAttrImage->get_value().c_str());
                walk_menu_xml(pSubmenu, pNodeIter->get_first_child());
            }
            else // otherwise it is an action id
            {
                CtAction const* pAction = find_action(get_attribute(pNodeIter, "action")->get_value());
                GtkWidget* pSubmenu = add_submenu(pMenu, pAction->id.c_str(), pAction->name.c_str(), pAction->image.c_str());
                walk_menu_xml(pSubmenu, pNodeIter->get_first_child());
            }
        }
        else if (pNodeIter->get_name() == "menuitem")
        {
            CtAction* pAction = find_action(get_attribute(pNodeIter, "action")->get_value());
            add_menu_item(pMenu, pAction);
        }
        else if (pNodeIter->get_name() == "separator")
        {
            add_separator(pMenu);
        }
    }
}

GtkWidget* CtMenu::add_submenu(GtkWidget* pMenu, const char* id, const char* name, const char* image)
{
    Gtk::MenuItem* pMenuItem = Gtk::manage(new Gtk::MenuItem());
    pMenuItem->set_name(id);
    GtkWidget* pLabel = gtk_accel_label_new(name);
    gtk_label_set_markup_with_mnemonic(GTK_LABEL(pLabel), name);
#if GTK_CHECK_VERSION(3,16,0)
    gtk_label_set_xalign(GTK_LABEL(pLabel), 0.0);
#else
    gtk_misc_set_alignment(GTK_MISC(pLabel), 0.0, 0.5);
#endif
    add_menu_item_image_or_label(pMenuItem, image, pLabel);

    GtkWidget* pSubmenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem->gobj()), GTK_WIDGET(pSubmenu));
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), GTK_WIDGET(pMenuItem->gobj()));
    return pSubmenu;
}

Gtk::MenuItem* CtMenu::add_menu_item(GtkWidget* pMenu, CtAction* pAction)
{
    return add_menu_item(pMenu, pAction->name.c_str(), pAction->image.c_str(), pAction->get_shortcut().c_str(),
                  pAction->desc.c_str(), (gpointer)pAction, &pAction->signal_set_sensitive, &pAction->signal_set_visible);

}

// based on inkscape/src/ui/interface.cpp
Gtk::MenuItem* CtMenu::add_menu_item(GtkWidget* pMenu, const char* name, const char* image, const char* shortcut,
                                 const char* desc, gpointer action_data,
                                 sigc::signal<void, bool>* signal_set_sensitive /* = nullptr */,
                                 sigc::signal<void, bool>* signal_set_visible /* = nullptr */)
{
    Gtk::MenuItem* pMenuItem = Gtk::manage(new Gtk::MenuItem());

    if (desc && strlen(desc))
    {
        pMenuItem->set_tooltip_text(desc);
    }
    // Now create the label and add it to the menu item
    GtkWidget* pLabel = gtk_accel_label_new(name);
    gtk_label_set_markup_with_mnemonic(GTK_LABEL(pLabel), name);

#if GTK_CHECK_VERSION(3,16,0)
    gtk_label_set_xalign(GTK_LABEL(pLabel), 0.0);
#else
    gtk_misc_set_alignment(GTK_MISC(pLabel), 0.0, 0.5);
#endif

    if (shortcut && strlen(shortcut))
    {
        guint key;
        GdkModifierType mod;
        gtk_accelerator_parse(shortcut, &key, &mod);
        gtk_widget_add_accelerator(GTK_WIDGET(pMenuItem->gobj()),
                        "activate",
                        default_accel_group(),
                        key,
                        mod,
                        GTK_ACCEL_VISIBLE);
    }
    gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(pLabel), GTK_WIDGET(pMenuItem->gobj()));

    add_menu_item_image_or_label(pMenuItem, image, pLabel);

    if (signal_set_sensitive)
        signal_set_sensitive->connect(
            sigc::bind<0>(
                sigc::ptr_fun(&gtk_widget_set_sensitive),
                GTK_WIDGET(pMenuItem->gobj())));
    if (signal_set_visible)
        signal_set_visible->connect(
            sigc::bind<0>(
                sigc::ptr_fun(&gtk_widget_set_visible),
                GTK_WIDGET(pMenuItem->gobj())));

    gtk_widget_set_events(GTK_WIDGET(pMenuItem->gobj()), GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(pMenuItem->gobj()), "activate", G_CALLBACK(on_menu_activate), action_data);

    pMenuItem->show_all();
    gtk_menu_shell_append(GTK_MENU_SHELL(GTK_MENU(pMenu)), GTK_WIDGET(pMenuItem->gobj()));

    return pMenuItem;
}

void CtMenu::add_menu_item_image_or_label(Gtk::Widget* pMenuItem, const char* image, GtkWidget* pLabel)
{
    if (image && strlen(image))
    {
        pMenuItem->set_name("ImageMenuItem");  // custom name to identify our "ImageMenuItems"

        GtkWidget* pIcon = gtk_image_new_from_icon_name(image, GTK_ICON_SIZE_MENU);

        // create a box to hold icon and label as GtkMenuItem derives from GtkBin and can only hold one child
        GtkWidget* pBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(pBox), pIcon, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(pBox), pLabel, TRUE, TRUE, 0);

        gtk_container_add(GTK_CONTAINER(pMenuItem->gobj()), pBox);
    }
    else
    {
        gtk_container_add(GTK_CONTAINER(pMenuItem->gobj()), pLabel);
    }
}

GtkWidget* CtMenu::add_separator(GtkWidget* pMenu)
{
    Gtk::Widget* pSeparatorItem = Gtk::manage(new Gtk::SeparatorMenuItem());
    pSeparatorItem->show_all();
    gtk_menu_shell_append(GTK_MENU_SHELL(GTK_MENU(pMenu)), pSeparatorItem->gobj());
    return pSeparatorItem->gobj();
}

std::string CtMenu::get_toolbar_ui_str()
{
    std::vector<std::string> vecToolbarElements = str::split(CtApp::P_ctCfg->toolbarUiList, ",");
    std::string toolbarUIStr;
    for (const std::string& element: vecToolbarElements)
    {
        if (element == CtConst::TAG_SEPARATOR)
        {
            toolbarUIStr += "<child><object class='GtkSeparatorToolItem'/></child>";
        }
        else if (CtAction const* pAction = find_action(element))
        {
            toolbarUIStr += "<child><object class='GtkToolButton'>";
            toolbarUIStr += "<property name='action-name'>app." + pAction->id + "</property>"; // 'app.' is a default action group in Application
            toolbarUIStr += "<property name='icon-name'>" + pAction->image + "</property>";
            toolbarUIStr += "<property name='label'>" + pAction->name + "</property>";
            toolbarUIStr += "<property name='tooltip-text'>" + pAction->desc + "</property>";
            toolbarUIStr += "<property name='visible'>True</property>";
            toolbarUIStr += "</object></child>";
        }
    }
    toolbarUIStr = "<interface><object class='GtkToolbar' id='ToolBar'>"
            "<property name='visible'>True</property>"
            "<property name='can_focus'>False</property>"
            + toolbarUIStr +
            "</object></interface>";
    return toolbarUIStr;
}


const char* CtMenu::get_menu_ui_str()
{
    return R"MARKUP(
<menubar name='MenuBar'>
  <menu action='FileMenu'>
    <menuitem action='ct_new_inst'/>
    <menuitem action='ct_open_file'/>
    <separator/>
    <menuitem action='ct_vacuum'/>
    <menuitem action='ct_save'/>
    <menuitem action='ct_save_as'/>
    <separator/>
    <menuitem action='print_page_setup'/>
    <menuitem action='do_print'/>
    <separator/>
    <menuitem action='exec_code'/>
    <separator/>
    <menuitem action='quit_app'/>
    <menuitem action='exit_app'/>
  </menu>

  <menu action='EditMenu'>
    <menuitem action='preferences_dlg'/>
    <separator/>
    <menuitem action='act_undo'/>
    <menuitem action='act_redo'/>
    <separator/>
    <menuitem action='handle_image'/>
    <menuitem action='handle_table'/>
    <menuitem action='handle_codebox'/>
    <menuitem action='handle_embfile'/>
    <menuitem action='handle_link'/>
    <menuitem action='handle_anchor'/>
    <menuitem action='insert_toc'/>
    <menuitem action='insert_timestamp'/>
    <menuitem action='insert_horiz_rule'/>
    <menuitem action='strip_trail_spaces'/>
    <separator/>
    <menu action='ChangeCaseMenu'>
      <menuitem action='case_down'/>
      <menuitem action='case_up'/>
      <menuitem action='case_tggl'/>
    </menu>
    <separator/>
    <menuitem action='spellcheck_toggle'/>
    <separator/>
    <menuitem action='cut_plain'/>
    <menuitem action='copy_plain'/>
    <menuitem action='paste_plain'/>
    <separator/>
    <menuitem action='cut_row'/>
    <menuitem action='copy_row'/>
    <menuitem action='del_row'/>
    <menuitem action='dup_row'/>
    <menuitem action='mv_up_row'/>
    <menuitem action='mv_down_row'/>
  </menu>

  <menu action='FormattingMenu'>
    <menuitem action='fmt_latest'/>
    <menuitem action='fmt_rm'/>
    <separator/>
    <menuitem action='fmt_color_fg'/>
    <menuitem action='fmt_color_bg'/>
    <menuitem action='fmt_bold'/>
    <menuitem action='fmt_italic'/>
    <menuitem action='fmt_underline'/>
    <menuitem action='fmt_strikethrough'/>
    <menuitem action='fmt_h1'/>
    <menuitem action='fmt_h2'/>
    <menuitem action='fmt_h3'/>
    <menuitem action='fmt_small'/>
    <menuitem action='fmt_superscript'/>
    <menuitem action='fmt_subscript'/>
    <menuitem action='fmt_monospace'/>
    <separator/>
    <menuitem action='handle_bull_list'/>
    <menuitem action='handle_num_list'/>
    <menuitem action='handle_todo_list'/>
    <separator/>
    <menuitem action='fmt_justify_left'/>
    <menuitem action='fmt_justify_center'/>
    <menuitem action='fmt_justify_right'/>
    <menuitem action='fmt_justify_fill'/>
  </menu>

  <menu action='TreeMenu'>
    <menuitem action='fmt_justify_left'/>
    <menuitem action='fmt_justify_left'/>
    <menuitem action='tree_add_node'/>
    <menuitem action='tree_add_subnode'/>
    <menuitem action='tree_dup_node'/>
    <separator/>
    <menuitem action='tree_node_prop'/>
    <menuitem action='tree_node_toggle_ro'/>
    <menuitem action='node_bookmark'/>
    <menuitem action='node_unbookmark'/>
    <menuitem action='tree_node_date'/>
    <menuitem action='tree_parse_info'/>
    <separator/>
    <menu action='TreeMoveMenu'>
      <menuitem action='tree_node_up'/>
      <menuitem action='tree_node_down'/>
      <menuitem action='tree_node_left'/>
      <menuitem action='tree_node_right'/>
      <menuitem action='tree_node_new_father'/>
    </menu>
    <separator/>
    <menu action='TreeSortMenu'>
      <menuitem action='tree_all_sort_asc'/>
      <menuitem action='tree_all_sort_desc'/>
      <menuitem action='tree_sibl_sort_asc'/>
      <menuitem action='tree_sibl_sort_desc'/>
    </menu>
    <separator/>
    <menuitem action='find_in_node_names'/>
    <menuitem action='replace_in_node_names'/>
    <separator/>
    <menu action='TreeImportMenu'>
      <menuitem action='import_cherrytree'/>
      <menuitem action='import_txt_file'/>
      <menuitem action='import_txt_folder'/>
      <menuitem action='import_html_file'/>
      <menuitem action='import_html_folder'/>
      <menuitem action='import_basket'/>
      <menuitem action='import_epim_html'/>
      <menuitem action='import_gnote'/>
      <menuitem action='import_keepnote'/>
      <menuitem action='import_keynote'/>
      <menuitem action='import_knowit'/>
      <menuitem action='import_leo'/>
      <menuitem action='import_mempad'/>
      <menuitem action='import_notecase'/>
      <menuitem action='import_rednotebook'/>
      <menuitem action='import_tomboy'/>
      <menuitem action='import_treepad'/>
      <menuitem action='import_tuxcards'/>
      <menuitem action='import_zim'/>
    </menu>
    <separator/>
    <menu action='TreeExportMenu'>
      <menuitem action='export_pdf'/>
      <menuitem action='export_html'/>
      <menuitem action='export_txt_multiple'/>
      <menuitem action='export_txt_single'/>
      <menuitem action='export_ctd'/>
    </menu>
    <separator/>
    <menuitem action='go_node_prev'/>
    <menuitem action='go_node_next'/>
  </menu>

  <menu action='SearchMenu'>
    <menuitem action='find_in_node'/>
    <menuitem action='find_in_allnodes'/>
    <menuitem action='find_in_node_n_sub'/>
    <menuitem action='find_in_node_names'/>
    <menuitem action='find_iter_fw'/>
    <menuitem action='find_iter_bw'/>
    <separator/>
    <menuitem action='replace_in_node'/>
    <menuitem action='replace_in_allnodes'/>
    <menuitem action='replace_in_node_n_sub'/>
    <menuitem action='replace_in_node_names'/>
    <menuitem action='replace_iter_fw'/>
  </menu>

  <menu action='ViewMenu'>
    <menuitem action='toggle_show_tree'/>
    <menuitem action='toggle_show_toolbar'/>
    <menuitem action='toggle_show_node_name_head'/>
    <menuitem action='toggle_show_allmatches_dlg'/>
    <separator/>
    <menuitem action='toggle_focus_tree_text'/>
    <menuitem action='nodes_all_expand'/>
    <menuitem action='nodes_all_collapse'/>
    <separator/>
    <menuitem action='toolbar_icons_size_p'/>
    <menuitem action='toolbar_icons_size_m'/>
    <separator/>
    <menuitem action='toggle_fullscreen'/>
  </menu>

  <menu action='BookmarksMenu'>
    <menuitem action='handle_bookmarks'/>
  </menu>

  <menu action='ImportMenu'>
    <menuitem action='import_cherrytree'/>
    <menuitem action='import_txt_file'/>
    <menuitem action='import_txt_folder'/>
    <menuitem action='import_html_file'/>
    <menuitem action='import_html_folder'/>
    <menuitem action='import_basket'/>
    <menuitem action='import_epim_html'/>
    <menuitem action='import_gnote'/>
    <menuitem action='import_keepnote'/>
    <menuitem action='import_keynote'/>
    <menuitem action='import_knowit'/>
    <menuitem action='import_leo'/>
    <menuitem action='import_mempad'/>
    <menuitem action='import_notecase'/>
    <menuitem action='import_rednotebook'/>
    <menuitem action='import_tomboy'/>
    <menuitem action='import_treepad'/>
    <menuitem action='import_tuxcards'/>
    <menuitem action='import_zim'/>
  </menu>

  <menu action='ExportMenu'>
    <menuitem action='export_pdf'/>
    <menuitem action='export_html'/>
    <menuitem action='export_txt_multiple'/>
    <menuitem action='export_txt_single'/>
    <menuitem action='export_ctd'/>
  </menu>

  <menu action='HelpMenu'>
    <menuitem action='ct_check_newer'/>
    <separator/>
    <menuitem action='ct_help'/>
    <separator/>
    <menuitem action='open_cfg_folder'/>
    <separator/>
    <menuitem action='ct_about'/>
  </menu>
</menubar>
    )MARKUP";
}

const char* CtMenu::get_popup_menu_text_ui_str()
{
    return R"MARKUP(
<popup>
  <separator/>
  <menuitem action='cut_plain'/>
  <menuitem action='copy_plain'/>
  <menuitem action='paste_plain'/>
  <separator/>
  <menu _name='For_matting' image='format_text'>
      <menuitem action='fmt_latest'/>
      <menuitem action='fmt_rm'/>
      <separator/>
      <menuitem action='fmt_color_fg'/>
      <menuitem action='fmt_color_bg'/>
      <menuitem action='fmt_bold'/>
      <menuitem action='fmt_italic'/>
      <menuitem action='fmt_underline'/>
      <menuitem action='fmt_strikethrough'/>
      <menuitem action='fmt_h1'/>
      <menuitem action='fmt_h2'/>
      <menuitem action='fmt_h3'/>
      <menuitem action='fmt_small'/>
      <menuitem action='fmt_superscript'/>
      <menuitem action='fmt_subscript'/>
      <menuitem action='fmt_monospace'/>
  </menu>
  <menu _name='_Justify' image='gtk-justify-center'>
      <menuitem action='fmt_justify_left'/>
      <menuitem action='fmt_justify_center'/>
      <menuitem action='fmt_justify_right'/>
      <menuitem action='fmt_justify_fill'/>
  </menu>
  <menu _name='_List' image='list_bulleted'>
      <menuitem action='handle_bull_list'/>
      <menuitem action='handle_num_list'/>
      <menuitem action='handle_todo_list'/>
  </menu>
  <separator/>
  <menu _name='_Insert' image='insert'>
      <menuitem action='handle_image'/>
      <menuitem action='handle_table'/>
      <menuitem action='handle_codebox'/>
      <menuitem action='handle_embfile'/>
      <menuitem action='handle_link'/>
      <menuitem action='handle_anchor'/>
      <menuitem action='insert_toc'/>
      <menuitem action='insert_timestamp'/>
      <menuitem action='insert_horiz_rule'/>
  </menu>
  <menu _name='C_hange Case' image='case_toggle'>
      <menuitem action='case_down'/>
      <menuitem action='case_up'/>
      <menuitem action='case_tggl'/>
  </menu>
  <menu _name='_Row' image='gtk-edit'>
      <menuitem action='cut_row'/>
      <menuitem action='copy_row'/>
      <menuitem action='del_row'/>
      <menuitem action='dup_row'/>
      <menuitem action='mv_up_row'/>
      <menuitem action='mv_down_row'/>
  </menu>
  <menuitem action='strip_trail_spaces'/>
  <separator/>
  <menu _name='_Search' image='find'>
      <menuitem action='find_in_node'/>
      <menuitem action='find_in_allnodes'/>
      <menuitem action='find_in_node_n_sub'/>
      <menuitem action='find_in_node_names'/>
      <menuitem action='find_iter_fw'/>
      <menuitem action='find_iter_bw'/>
  </menu>
  <menu _name='_Replace' image='find_replace'>
      <menuitem action='replace_in_node'/>
      <menuitem action='replace_in_allnodes'/>
      <menuitem action='replace_in_node_n_sub'/>
      <menuitem action='replace_in_node_names'/>
      <menuitem action='replace_iter_fw'/>
      <menuitem action='find_iter_bw'/>
  </menu>
</popup>
    )MARKUP";
}

const char* CtMenu::get_popup_menu_code_ui_str()
{
    return R"MARKUP(
<popup>
  <separator/>
  <menuitem action='cut_plain'/>
  <menuitem action='copy_plain'/>
  <separator/>
  <menuitem action='exec_code'/>
  <menu _name='_Insert' image='insert'>
      <menuitem action='insert_timestamp'/>
      <menuitem action='insert_horiz_rule'/>
  </menu>
  <menuitem action='strip_trail_spaces'/>
  <menu _name='C_hange Case' image='case_toggle'>
      <menuitem action='case_down'/>
      <menuitem action='case_up'/>
      <menuitem action='case_tggl'/>
  </menu>
  <menu _name='_Row' image='gtk-edit'>
      <menuitem action='cut_row'/>
      <menuitem action='copy_row'/>
      <menuitem action='del_row'/>
      <menuitem action='dup_row'/>
      <menuitem action='mv_up_row'/>
      <menuitem action='mv_down_row'/>
  </menu>
  <separator/>
  <menu _name='_Search' image='find'>
      <menuitem action='find_in_node'/>
      <menuitem action='find_in_allnodes'/>
      <menuitem action='find_in_node_n_sub'/>
      <menuitem action='find_in_node_names'/>
      <menuitem action='find_iter_fw'/>
      <menuitem action='find_iter_bw'/>
  </menu>
  <menu _name='_Replace' image='find_replace'>
      <menuitem action='replace_in_node'/>
      <menuitem action='replace_in_allnodes'/>
      <menuitem action='replace_in_node_n_sub'/>
      <menuitem action='replace_in_node_names'/>
      <menuitem action='replace_iter_fw'/>
      <menuitem action='find_iter_bw'/>
  </menu>
</popup>
    )MARKUP";
}
