/*
 * ct_menu.cc
 *
 * Copyright 2009-2021
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

#include "ct_menu.h"
#include "ct_const.h"
#include "ct_config.h"
#include "ct_actions.h"
#include "ct_app.h"
#include "ct_storage_xml.h"
#include <glibmm/i18n.h>
#include <sigc++/sigc++.h>
#include <gtk/gtk.h>
#include <glib-object.h>

static xmlpp::Attribute* get_attribute(xmlpp::Node* pNode, char const* name)
{
    xmlpp::Element* pElement = static_cast<xmlpp::Element*>(pNode);
    return pElement->get_attribute(name);
}

static void on_menu_activate(void* /*pObject*/, CtMenuAction* pAction)
{
    if (pAction) {
        pAction->run_action();
    }
}

const std::string& CtMenuAction::get_shortcut(CtConfig* pCtConfig) const
{
    auto it = pCtConfig->customKbShortcuts.find(id);
    return it != pCtConfig->customKbShortcuts.end() ? it->second : built_in_shortcut;
}


CtMenu::CtMenu(CtConfig* pCtConfig, CtActions* pActions)
 : _pCtConfig{pCtConfig}
{
    _pAccelGroup = Gtk::AccelGroup::create();
    _rGtkBuilder = Gtk::Builder::create();
    init_actions(pActions);
}

/*static*/ Gtk::MenuItem* CtMenu::create_menu_item(Gtk::Menu* pMenu, const char* name, const char* image, const char* desc)
{
    return _add_menu_item(pMenu, name, image, nullptr, Glib::RefPtr<Gtk::AccelGroup>{}, desc, nullptr, nullptr, nullptr);
}

void CtMenu::init_actions(CtActions* pActions)
{
    // stubs for menu bar
    _actions.push_back(CtMenuAction{"", "FileMenu", None, _("_File"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "EditMenu", None, _("_Edit"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "FormattingMenu", None, _("For_matting"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "TreeMenu", None, _("_Tree"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "TreeMoveMenu", "ct_go-jump", _("Node _Move"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "TreeSortMenu", "ct_sort-asc", _("Nodes _Sort"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "TreeImportMenu", CtConst::STR_STOCK_CT_IMP, _("Nodes _Import"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "TreeExportMenu", "ct_export_from_cherrytree", _("Nodes E_xport"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "RecentDocsMenu", "ct_open", _("_Recent Documents"), None, _("Open a Recent CherryTree Document"), sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ChangeCaseMenu", "ct_case_toggle", _("C_hange Case"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "SearchMenu", None, _("_Search"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ViewMenu", None, _("_View"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "BookmarksMenu", None, _("_Bookmarks"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ImportMenu", None, _("_Import"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "ExportMenu", None, _("E_xport"), None, None, sigc::signal<void>()});
    _actions.push_back(CtMenuAction{"", "HelpMenu", None, _("_Help"), None, None, sigc::signal<void>()});

    // main actions
    const char* file_cat = _("File");
    _actions.push_back(CtMenuAction{file_cat, "ct_new_inst", "ct_new-instance", _("New _Instance"), None, _("Start a New Instance of CherryTree"), sigc::mem_fun(*pActions, &CtActions::file_new)});
    _actions.push_back(CtMenuAction{file_cat, "ct_open_file", "ct_open", _("_Open File"), KB_CONTROL+"O", _("Open a CherryTree Document"), sigc::mem_fun(*pActions, &CtActions::file_open)});
    _actions.push_back(CtMenuAction{file_cat, "ct_save", "ct_save", _("_Save"), KB_CONTROL+"S", _("Save File"), sigc::mem_fun(*pActions, &CtActions::file_save)});
    _actions.push_back(CtMenuAction{file_cat, "ct_vacuum", "ct_clear", _("Save and _Vacuum"), None, _("Save File and Vacuum"), sigc::mem_fun(*pActions, &CtActions::file_vacuum)});
    _actions.push_back(CtMenuAction{file_cat, "ct_save_as", "ct_save-as", _("Save _As"), KB_CONTROL+KB_SHIFT+"S", _("Save File As"), sigc::mem_fun(*pActions, &CtActions::file_save_as)});
    _actions.push_back(CtMenuAction{file_cat, "command_palette", "ct_execute", _("Command Palette"), KB_CONTROL+KB_SHIFT+"P", _("Command Palette"), sigc::mem_fun(*pActions, &CtActions::command_palette)});
    _actions.push_back(CtMenuAction{file_cat, "exec_code", "ct_execute", _("_Execute Code"), "F5", _("Execute Code"), sigc::mem_fun(*pActions, &CtActions::exec_code)});
    _actions.push_back(CtMenuAction{file_cat, "open_cfg_folder", "ct_directory", _("Open Preferences _Directory"), None, _("Open the Directory with Preferences Files"), sigc::mem_fun(*pActions, &CtActions::folder_cfg_open)});
    _actions.push_back(CtMenuAction{file_cat, "print_page_setup", "ct_print", _("Pa_ge Setup"), None, _("Set up the Page for Printing"), sigc::mem_fun(*pActions, &CtActions::export_print_page_setup)});
    _actions.push_back(CtMenuAction{file_cat, "do_print", "ct_print", _("_Print"), KB_CONTROL+"P", _("Print"), sigc::mem_fun(*pActions, &CtActions::export_print)});
    _actions.push_back(CtMenuAction{file_cat, "quit_app", "ct_quit-app", _("_Quit"), KB_CONTROL+"Q", _("Quit the Application"), sigc::mem_fun(*pActions, &CtActions::quit_or_hide_window)});
    _actions.push_back(CtMenuAction{file_cat, "exit_app", "ct_quit-app", _("_Exit CherryTree"), KB_CONTROL+KB_SHIFT+"Q", _("Exit from CherryTree"), sigc::mem_fun(*pActions, &CtActions::quit_window)});
    _actions.push_back(CtMenuAction{file_cat, "preferences_dlg", "ct_preferences", _("_Preferences"), KB_CONTROL+KB_ALT+"P", _("Preferences"), sigc::mem_fun(*pActions, &CtActions::dialog_preferences) });
    _actions.push_back(CtMenuAction{file_cat, "ct_check_newer", "ct_network", _("_Check Newer Version"), None, _("Check for a Newer Version"), sigc::mem_fun(*pActions, &CtActions::check_for_newer_version)});
    _actions.push_back(CtMenuAction{file_cat, "ct_help", "ct_help", _("Online _Manual"), "F1", _("Application's Online Manual"), sigc::mem_fun(*pActions, &CtActions::online_help)});
    _actions.push_back(CtMenuAction{file_cat, "ct_about", "ct_about", _("_About"), None, _("About CherryTree"), sigc::mem_fun(*pActions, &CtActions::dialog_about)});
    const char* editor_cat = _("Editor");
    _actions.push_back(CtMenuAction{editor_cat, "act_undo", "ct_undo", _("_Undo"), KB_CONTROL+"Z", _("Undo Last Operation"), sigc::mem_fun(*pActions, &CtActions::requested_step_back)});
    _actions.push_back(CtMenuAction{editor_cat, "act_redo", "ct_redo", _("_Redo"), KB_CONTROL+"Y", _("Redo Previously Discarded Operation"), sigc::mem_fun(*pActions, &CtActions::requested_step_ahead)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_image", "ct_image_insert", _("Insert I_mage"), KB_CONTROL+KB_ALT+"I", _("Insert an Image"), sigc::mem_fun(*pActions, &CtActions::image_handle)});
    //_actions.push_back(CtAction{"handle_screenshot", "screenshot_insert", _("Insert _Screenshot"), KB_CONTROL+KB_SHIFT+KB_ALT+"S", _("Insert a Screenshot"), sigc::signal<void>() /* dad.screenshot_handle */});
    _actions.push_back(CtMenuAction{editor_cat, "handle_table", "ct_table_insert", _("Insert _Table"), KB_CONTROL+KB_ALT+"T", _("Insert a Table"), sigc::mem_fun(*pActions, &CtActions::table_handle)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_codebox", "ct_codebox_insert", _("Insert _CodeBox"), KB_CONTROL+KB_ALT+"C", _("Insert a CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_handle)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_embfile", "ct_file_icon", _("Insert _File"), KB_CONTROL+KB_ALT+"E", _("Insert File"), sigc::mem_fun(*pActions, &CtActions::embfile_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_link", "ct_link_handle", _("Insert/Edit _Link"), KB_CONTROL+"L", _("Insert a Link/Edit the Underlying Link"), sigc::mem_fun(*pActions, &CtActions::apply_tag_link)});
    _actions.push_back(CtMenuAction{editor_cat, "handle_anchor", "ct_anchor_insert", _("Insert _Anchor"), KB_CONTROL+KB_ALT+"A", _("Insert an Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_handle)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_toc", "ct_index", _("Insert T_OC"), None, _("Insert Table of Contents"), sigc::mem_fun(*pActions, &CtActions::toc_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_timestamp", "ct_timestamp", _("Insert Ti_mestamp"), KB_CONTROL+KB_ALT+"M", _("Insert Timestamp"), sigc::mem_fun(*pActions, &CtActions::timestamp_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_special_char", "ct_insert", _("Insert _Special Character"), None, _("Insert a Special Character"), sigc::mem_fun(*pActions, &CtActions::special_char_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "insert_horiz_rule", "ct_horiz_rule", _("Insert _Horizontal Rule"), KB_CONTROL+"R", _("Insert Horizontal Rule"), sigc::mem_fun(*pActions, &CtActions::horizontal_rule_insert)});
    _actions.push_back(CtMenuAction{editor_cat, "case_down", "ct_case_lower", _("_Lower Case of Selection/Word"), KB_CONTROL+"W", _("Lower the Case of the Selection/the Underlying Word"), sigc::mem_fun(*pActions, &CtActions::text_selection_lower_case)});
    _actions.push_back(CtMenuAction{editor_cat, "case_up", "ct_case_upper", _("_Upper Case of Selection/Word"), KB_CONTROL+KB_SHIFT+"W", _("Upper the Case of the Selection/the Underlying Word"), sigc::mem_fun(*pActions, &CtActions::text_selection_upper_case)});
    _actions.push_back(CtMenuAction{editor_cat, "case_tggl", "ct_case_toggle", _("_Toggle Case of Selection/Word"), KB_CONTROL+"G", _("Toggle the Case of the Selection/the Underlying Word"), sigc::mem_fun(*pActions, &CtActions::text_selection_toggle_case)});
    _actions.push_back(CtMenuAction{editor_cat, "spellcheck_toggle", "ct_spell-check", _("Enable/Disable _Spell Check"), KB_CONTROL+KB_ALT+"S", _("Toggle Enable/Disable Spell Check"), sigc::mem_fun(*pActions, &CtActions::toggle_ena_dis_spellcheck)});
    _actions.push_back(CtMenuAction{editor_cat, "cut_plain", "ct_edit_cut", _("Cu_t as Plain Text"), KB_CONTROL+KB_SHIFT+"X", _("Cut as Plain Text, Discard the Rich Text Formatting"), sigc::mem_fun(*pActions, &CtActions::cut_as_plain_text)});
    _actions.push_back(CtMenuAction{editor_cat, "copy_plain", "ct_edit_copy", _("_Copy as Plain Text"), KB_CONTROL+KB_SHIFT+"C", _("Copy as Plain Text, Discard the Rich Text Formatting"), sigc::mem_fun(*pActions, &CtActions::copy_as_plain_text)});
    _actions.push_back(CtMenuAction{editor_cat, "paste_plain", "ct_edit_paste", _("_Paste as Plain Text"), KB_CONTROL+KB_SHIFT+"V", _("Paste as Plain Text, Discard the Rich Text Formatting"), sigc::mem_fun(*pActions, &CtActions::paste_as_plain_text)});
    _actions.push_back(CtMenuAction{editor_cat, "cut_row", "ct_edit_cut", _("Cu_t Row"), KB_SHIFT+KB_ALT+"X", _("Cut the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_cut)});
    _actions.push_back(CtMenuAction{editor_cat, "copy_row", "ct_edit_copy", _("_Copy Row"), KB_SHIFT+KB_ALT+"C", _("Copy the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_copy)});
    _actions.push_back(CtMenuAction{editor_cat, "del_row", "ct_edit_delete", _("De_lete Row"), KB_CONTROL+"K", _("Delete the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_delete)});
    _actions.push_back(CtMenuAction{editor_cat, "dup_row", "ct_add", _("_Duplicate Row"), KB_CONTROL+"D", _("Duplicate the Current Row/Selection"), sigc::mem_fun(*pActions, &CtActions::text_row_selection_duplicate)});
    _actions.push_back(CtMenuAction{editor_cat, "mv_up_row", "ct_go-up", _("Move _Up Row"), KB_ALT+CtConst::STR_KEY_UP, _("Move Up the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_up)});
    _actions.push_back(CtMenuAction{editor_cat, "mv_down_row", "ct_go-down", _("Move _Down Row"), KB_ALT+CtConst::STR_KEY_DOWN, _("Move Down the Current Row/Selected Rows"), sigc::mem_fun(*pActions, &CtActions::text_row_down)});
    _actions.push_back(CtMenuAction{editor_cat, "strip_trail_spaces", "ct_clear", _("Stri_p Trailing Spaces"), None, _("Strip Trailing Spaces"), sigc::mem_fun(*pActions, &CtActions::strip_trailing_spaces)});
    const char* fmt_cat = _("Format");
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_clone", "ct_fmt-txt-clone", _("Format C_lone"), None, _("Clone the Text Format Type at Cursor"), sigc::mem_fun(*pActions, &CtActions::save_tags_at_cursor_as_latest)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_latest", "ct_fmt-txt-latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), sigc::mem_fun(*pActions, &CtActions::apply_tags_latest)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_rm", "ct_fmt-txt-clear", _("_Remove Formatting"), KB_CONTROL+KB_SHIFT+"R", _("Remove the Formatting from the Selected Text"), sigc::mem_fun(*pActions, &CtActions::remove_text_formatting)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_color_fg", "ct_color_fg", _("Text _Color Foreground"), KB_SHIFT+KB_ALT+"F", _("Change the Color of the Selected Text Foreground"), sigc::mem_fun(*pActions, &CtActions::apply_tag_foreground)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_color_bg", "ct_color_bg", _("Text C_olor Background"), KB_SHIFT+KB_ALT+"B", _("Change the Color of the Selected Text Background"), sigc::mem_fun(*pActions, &CtActions::apply_tag_background)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_bold", "ct_fmt-txt-bold", _("Toggle _Bold Property"), KB_CONTROL+"B", _("Toggle Bold Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_bold)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_italic", "ct_fmt-txt-italic", _("Toggle _Italic Property"), KB_CONTROL+"I", _("Toggle Italic Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_italic)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_underline", "ct_fmt-txt-underline", _("Toggle _Underline Property"), KB_CONTROL+"U", _("Toggle Underline Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_underline)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_strikethrough", "ct_fmt-txt-strikethrough", _("Toggle Stri_kethrough Property"), KB_CONTROL+"E", _("Toggle Strikethrough Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_strikethrough)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h1", "ct_fmt-txt-large", _("Toggle h_1 Property"), KB_CONTROL+"1", _("Toggle h1 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h1)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h2", "ct_fmt-txt-large2", _("Toggle h_2 Property"), KB_CONTROL+"2", _("Toggle h2 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h2)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_h3", "ct_fmt-txt-large3", _("Toggle h_3 Property"), KB_CONTROL+"3", _("Toggle h3 Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_h3)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_small", "ct_fmt-txt-small", _("Toggle _Small Property"), KB_CONTROL+"0", _("Toggle Small Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_small)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_superscript", "ct_fmt-txt-superscript", _("Toggle Su_perscript Property"), None, _("Toggle Superscript Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_superscript)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_subscript", "ct_fmt-txt-subscript", _("Toggle Su_bscript Property"), None, _("Toggle Subscript Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_subscript)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_monospace", "ct_fmt-txt-monospace", _("Toggle _Monospace Property"), KB_CONTROL+"M", _("Toggle Monospace Property of the Selected Text"), sigc::mem_fun(*pActions, &CtActions::apply_tag_monospace)});
    _actions.push_back(CtMenuAction{fmt_cat, "handle_bull_list", "ct_list_bulleted", _("Set/Unset _Bulleted List"), KB_CONTROL+KB_ALT+"1", _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), sigc::mem_fun(*pActions, &CtActions::list_bulleted_handler)});
    _actions.push_back(CtMenuAction{fmt_cat, "handle_num_list", "ct_list_numbered", _("Set/Unset _Numbered List"), KB_CONTROL+KB_ALT+"2", _("Set/Unset the Current Paragraph/Selection as a Numbered List"), sigc::mem_fun(*pActions, &CtActions::list_numbered_handler)});
    _actions.push_back(CtMenuAction{fmt_cat, "handle_todo_list", "ct_list_todo", _("Set/Unset _To-Do List"), KB_CONTROL+KB_ALT+"3", _("Set/Unset the Current Paragraph/Selection as a To-Do List"), sigc::mem_fun(*pActions, &CtActions::list_todo_handler)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_left", "ct_justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_left)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_center", "ct_justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_center)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_right", "ct_justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_right)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_justify_fill", "ct_justify-fill", _("Justify _Fill"), None, _("Justify Fill the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_justify_fill)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_indent", "ct_fmt-indent", _("Indent Paragraph"), KB_CONTROL+KB_SHIFT+"greater", _("Indent the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::apply_tag_indent)});
    _actions.push_back(CtMenuAction{fmt_cat, "fmt_unindent", "ct_fmt-unindent", _("Unindent Paragraph"), KB_CONTROL+KB_SHIFT+"less", _("Unindent the Current Paragraph"), sigc::mem_fun(*pActions, &CtActions::reduce_tag_indent)});
    const char* tree_cat = _("Tree");
    _actions.push_back(CtMenuAction{tree_cat, "tree_add_node", "ct_tree-node-add", _("Add _Node"), KB_CONTROL+"N", _("Add a Node having the same Parent of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_add)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_add_subnode", "ct_tree-subnode-add", _("Add _SubNode"), KB_CONTROL+KB_SHIFT+"N", _("Add a Child Node to the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_child_add)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_dup_node", "ct_tree-node-dupl", _("_Duplicate Node"), KB_CONTROL+KB_SHIFT+"D", _("Duplicate the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_duplicate)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_dup_node_subnodes", "ct_tree-nodesub-dupl", _("_Duplicate Node and Sub Nodes"), None, _("Duplicate the Selected Node With SubNodes"), sigc::mem_fun(*pActions, &CtActions::node_subnodes_duplicate)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_prop", "ct_cherry_edit", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_edit)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_toggle_ro", "ct_locked", _("Toggle _Read Only"), KB_CONTROL+KB_ALT+"R", _("Toggle the Read Only Property of the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_toggle_read_only)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_date", "ct_calendar", _("Insert Today's Node"), "F8", _("Insert a Node with Hierarchy Year/Month/Day"), sigc::mem_fun(*pActions, &CtActions::node_date)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_parse_info", "ct_info", _("Tree _Info"), None, _("Tree Summary Information"), sigc::mem_fun(*pActions, &CtActions::tree_info)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_up", "ct_go-up", _("Node _Up"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_UP, _("Move the Selected Node Up"), sigc::mem_fun(*pActions, &CtActions::node_up)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_down", "ct_go-down", _("Node _Down"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_DOWN, _("Move the Selected Node Down"), sigc::mem_fun(*pActions, &CtActions::node_down)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_left", "ct_go-back", _("Node _Left"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_LEFT, _("Move the Selected Node Left"), sigc::mem_fun(*pActions, &CtActions::node_left)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_right", "ct_go-forward", _("Node _Right"), KB_SHIFT+KB_ALT+CtConst::STR_KEY_RIGHT, _("Move the Selected Node Right"), sigc::mem_fun(*pActions, &CtActions::node_right)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_new_father", "ct_go-jump", _("Node Change _Parent"), KB_SHIFT+KB_ALT+"P", _("Change the Selected Node's Parent"), sigc::mem_fun(*pActions, &CtActions::node_change_father)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_all_sort_asc", "ct_sort-asc", _("Sort Tree _Ascending"), None, _("Sort the Tree Ascending"), sigc::mem_fun(*pActions, &CtActions::tree_sort_ascending)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_all_sort_desc", "ct_sort-desc", _("Sort Tree _Descending"), None, _("Sort the Tree Descending"), sigc::mem_fun(*pActions, &CtActions::tree_sort_descending)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_sibl_sort_asc", "ct_sort-asc", _("Sort Siblings A_scending"), None, _("Sort all the Siblings of the Selected Node Ascending"), sigc::mem_fun(*pActions, &CtActions::node_siblings_sort_ascending)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_sibl_sort_desc", "ct_sort-desc", _("Sort Siblings D_escending"), None, _("Sort all the Siblings of the Selected Node Descending"), sigc::mem_fun(*pActions, &CtActions::node_siblings_sort_descending)});
    _actions.push_back(CtMenuAction{tree_cat, "child_nodes_inherit_syntax", "ct_execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting"), sigc::mem_fun(*pActions, &CtActions::node_inherit_syntax)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_del", "ct_edit_delete", _("De_lete Node"), None, _("Delete the Selected Node"), sigc::mem_fun(*pActions, &CtActions::node_delete)});
    _actions.push_back(CtMenuAction{tree_cat, "node_bookmark", "ct_pin-add", _("Add to _Bookmarks"), KB_CONTROL+KB_SHIFT+"B", _("Add the Current Node to the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmark_curr_node)});
    _actions.push_back(CtMenuAction{tree_cat, "node_unbookmark", "ct_pin-remove", _("_Remove from Bookmarks"), KB_CONTROL+KB_ALT+"B", _("Remove the Current Node from the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmark_curr_node_remove)});
    _actions.push_back(CtMenuAction{tree_cat, "handle_bookmarks", "ct_edit", _("_Handle Bookmarks"), None, _("Handle the Bookmarks List"), sigc::mem_fun(*pActions, &CtActions::bookmarks_handle)});
    _actions.push_back(CtMenuAction{tree_cat, "go_node_prev", "ct_go-back", _("Go _Back"), KB_ALT+CtConst::STR_KEY_LEFT, _("Go to the Previous Visited Node"), sigc::mem_fun(*pActions, &CtActions::node_go_back)});
    _actions.push_back(CtMenuAction{tree_cat, "go_node_next", "ct_go-forward", _("Go _Forward"), KB_ALT+CtConst::STR_KEY_RIGHT, _("Go to the Next Visited Node"), sigc::mem_fun(*pActions, &CtActions::node_go_forward)});
    _actions.push_back(CtMenuAction{tree_cat, "tree_node_link", "ct_node_link", _("Copy Link to Node"), None, _("Copy Link to the Selected Node to Clipboard"), sigc::mem_fun(*pActions, &CtActions::node_link_to_clipboard)});
    const char* find_cat = _("Find/Replace");
    _actions.push_back(CtMenuAction{find_cat, "find_in_node", "ct_find_sel", _("_Find in Node Content"), KB_CONTROL+"F", _("Find into the Selected Node Content"), sigc::mem_fun(*pActions, &CtActions::find_in_selected_node)});
    _actions.push_back(CtMenuAction{find_cat, "find_in_allnodes", "ct_find_all", _("Find in _All Nodes Contents"), KB_CONTROL+KB_SHIFT+"F", _("Find into All the Tree Nodes Contents"), sigc::mem_fun(*pActions, &CtActions::find_in_all_nodes)});
    _actions.push_back(CtMenuAction{find_cat, "find_in_node_n_sub", "ct_find_selnsub", _("Find in _Selected Node and Subnodes Contents"), KB_CONTROL+KB_ALT+"F", _("Find into the Selected Node and Subnodes Contents"), sigc::mem_fun(*pActions, &CtActions::find_in_sel_node_and_subnodes)});
    _actions.push_back(CtMenuAction{find_cat, "find_in_node_names", "ct_find", _("Find in _Nodes Names and Tags"), KB_CONTROL+"T", _("Find in Nodes Names and Tags"), sigc::mem_fun(*pActions, &CtActions::find_a_node)});
    _actions.push_back(CtMenuAction{find_cat, "find_iter_fw", "ct_find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), sigc::mem_fun(*pActions, &CtActions::find_again)});
    _actions.push_back(CtMenuAction{find_cat, "find_iter_bw", "ct_find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), sigc::mem_fun(*pActions, &CtActions::find_back)});
    _actions.push_back(CtMenuAction{find_cat, "replace_in_node", "ct_replace_sel", _("_Replace in Node Content"), KB_CONTROL+"H", _("Replace into the Selected Node Content"), sigc::mem_fun(*pActions, &CtActions::replace_in_selected_node)});
    _actions.push_back(CtMenuAction{find_cat, "replace_in_allnodes", "ct_replace_all", _("Replace in _All Nodes Contents"), KB_CONTROL+KB_SHIFT+"H", _("Replace into All the Tree Nodes Contents"), sigc::mem_fun(*pActions, &CtActions::replace_in_all_nodes)});
    _actions.push_back(CtMenuAction{find_cat, "replace_in_node_n_sub", "ct_replace_selnsub", _("Replace in _Selected Node and Subnodes Contents"), KB_CONTROL+KB_ALT+"H", _("Replace into the Selected Node and Subnodes Contents"), sigc::mem_fun(*pActions, &CtActions::replace_in_sel_node_and_subnodes)});
    _actions.push_back(CtMenuAction{find_cat, "replace_in_node_names", "ct_find_replace", _("Replace in Nodes _Names"), KB_CONTROL+KB_SHIFT+"T", _("Replace in Nodes Names"), sigc::mem_fun(*pActions, &CtActions::replace_in_nodes_names)});
    _actions.push_back(CtMenuAction{find_cat, "replace_iter_fw", "ct_replace_again", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), sigc::mem_fun(*pActions, &CtActions::replace_again)});
    _actions.push_back(CtMenuAction{find_cat, "toggle_show_allmatches_dlg", "ct_find", _("Show _All Matches Dialog"), KB_CONTROL+KB_SHIFT+"A", _("Show Search All Matches Dialog"), sigc::mem_fun(*pActions, &CtActions::find_allmatchesdialog_restore)});
    const char* view_cat = _("View");
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_tree", "ct_cherries", _("Show/Hide _Tree"), "F9", _("Toggle Show/Hide Tree"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_tree)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_toolbar", "ct_toolbar", _("Show/Hide Tool_bar"), None, _("Toggle Show/Hide Toolbar"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_toolbars)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_show_node_name_head", "ct_node_name_header", _("Show/Hide Node Name _Header"), None, _("Toggle Show/Hide Node Name Header"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_node_name_header)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_focus_tree_text", "ct_go-jump", _("Toggle _Focus Tree/Text"), KB_CONTROL+"Tab", _("Toggle Focus Between Tree and Text"), sigc::mem_fun(*pActions, &CtActions::toggle_tree_text)});
    _actions.push_back(CtMenuAction{view_cat, "nodes_all_expand", "ct_zoom-in", _("E_xpand All Nodes"), KB_CONTROL+KB_SHIFT+"E", _("Expand All the Tree Nodes"), sigc::mem_fun(*pActions, &CtActions::nodes_expand_all)});
    _actions.push_back(CtMenuAction{view_cat, "nodes_all_collapse", "ct_zoom-out", _("_Collapse All Nodes"), KB_CONTROL+KB_SHIFT+"L", _("Collapse All the Tree Nodes"), sigc::mem_fun(*pActions, &CtActions::nodes_collapse_all)});
    _actions.push_back(CtMenuAction{view_cat, "toolbar_icons_size_p", "ct_add", _("_Increase Toolbar Icons Size"), None, _("Increase the Size of the Toolbar Icons"), sigc::mem_fun(*pActions, &CtActions::toolbar_icons_size_increase)});
    _actions.push_back(CtMenuAction{view_cat, "toolbar_icons_size_m", "ct_remove", _("_Decrease Toolbar Icons Size"), None, _("Decrease the Size of the Toolbar Icons"), sigc::mem_fun(*pActions, &CtActions::toolbar_icons_size_decrease)});
    _actions.push_back(CtMenuAction{view_cat, "toggle_fullscreen", "ct_fullscreen", _("_Full Screen On/Off"), "F11", _("Toggle Full Screen On/Off"), sigc::mem_fun(*pActions, &CtActions::fullscreen_toggle)});
    const char* export_cat = _("Export");
    _actions.push_back(CtMenuAction{export_cat, "export_pdf", "ct_to_pdf", _("Export To _PDF"), None, _("Export To PDF"), sigc::mem_fun(*pActions, &CtActions::export_to_pdf)});
    _actions.push_back(CtMenuAction{export_cat, "export_html", "ct_to_html", _("Export To _HTML"), None, _("Export To HTML"), sigc::mem_fun(*pActions, &CtActions::export_to_html)});
    _actions.push_back(CtMenuAction{export_cat, "export_txt", "ct_to_txt", _("Export to Plain _Text"), None, _("Export to Plain Text"), sigc::mem_fun(*pActions, &CtActions::export_to_txt)});
    _actions.push_back(CtMenuAction{export_cat, "export_ctd", "ct_to_cherrytree", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), sigc::mem_fun(*pActions, &CtActions::export_to_ctd)});
    const char* import_cat = _("Import");
    _actions.push_back(CtMenuAction{import_cat, "import_cherrytree", "ct_from_cherrytree", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_ct_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_txt_file", "ct_from_txt", _("From _Plain Text File"), None, _("Add Node from a Plain Text File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_node_from_plaintext_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_txt_folder", "ct_from_txt", _("From _Folder of Plain Text Files"), None, _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_plaintext_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_html_file", "ct_from_html", _("From _HTML File"), None, _("Add Node from an HTML File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_node_from_html_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_html_folder", "ct_from_html", _("From _Folder of HTML Files"), None, _("Add Nodes from a Folder of HTML Files to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_node_from_html_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_md_file",  CtConst::STR_STOCK_CT_IMP, _("From _Markdown File"), None, _("Add a node from a Markdown File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_node_from_md_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_md_folder",  CtConst::STR_STOCK_CT_IMP, _("From _Folder of Markdown Files"), None, _("Add Nodes from a Folder of Markdown Files to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_md_directory)});
    //_actions.push_back(CtMenuAction{import_cat, "import_basket", CtConst::STR_STOCK_CT_IMP, _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_basket_folder */});
    //_actions.push_back(CtMenuAction{import_cat, "import_epim_html", CtConst::STR_STOCK_CT_IMP, _("From _EssentialPIM HTML File"), None, _("Add Node from an EssentialPIM HTML File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_epim_html_file */});
    _actions.push_back(CtMenuAction{import_cat, "import_gnote", CtConst::STR_STOCK_CT_IMP, _("From _Gnote Folder"), None, _("Add Nodes of a Gnote Folder to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_gnote_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_keepnote", CtConst::STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_keepnote_directory) /* dad.nodes_add_from_keepnote_folder */});
    //_actions.push_back(CtMenuAction{import_cat, "import_keynote", CtConst::STR_STOCK_CT_IMP, _("From K_eyNote File"), None, _("Add Nodes of a KeyNote File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_keynote_file */});
    //_actions.push_back(CtMenuAction{import_cat, "import_knowit", CtConst::STR_STOCK_CT_IMP, _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_knowit_file */});
    _actions.push_back(CtMenuAction{import_cat, "import_leo", CtConst::STR_STOCK_CT_IMP, _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_leo_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_mempad", CtConst::STR_STOCK_CT_IMP, _("From _Mempad File"), None, _("Add Nodes of a Mempad File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_mempad_file)});
    _actions.push_back(CtMenuAction{import_cat, "import_notecase", CtConst::STR_STOCK_CT_IMP, _("From _NoteCase HTML File"), None, _("Add Nodes of a NoteCase HTML File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_notecase_html)});
    _actions.push_back(CtMenuAction{import_cat, "import_rednotebook", CtConst::STR_STOCK_CT_IMP, _("From _RedNotebook HTML"), None, _("Add Nodes of a RedNotebook HTML file to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_rednotebook_html)});
    _actions.push_back(CtMenuAction{import_cat, "import_tomboy", CtConst::STR_STOCK_CT_IMP, _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_tomboy_directory)});
    _actions.push_back(CtMenuAction{import_cat, "import_treepad", CtConst::STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_treepad_file)});
    //_actions.push_back(CtMenuAction{import_cat, "import_tuxcards", CtConst::STR_STOCK_CT_IMP, _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_tuxcards_file */});
    _actions.push_back(CtMenuAction{import_cat, "import_zim", CtConst::STR_STOCK_CT_IMP, _("From _Zim Folder"), None, _("Add Nodes of a Zim Folder to the Current Tree"), sigc::mem_fun(*pActions, &CtActions::import_nodes_from_zim_directory) /* dad.nodes_add_from_zim_folder */});
    _actions.push_back(CtMenuAction{import_cat, "import_pandoc_file", CtConst::STR_STOCK_CT_IMP, _("From File using _Pandoc"), None, _("Add a node to the current tree using Pandoc"), sigc::mem_fun(*pActions, &CtActions::import_node_from_pandoc) });
    const char* others_cat = "";
    _actions.push_back(CtMenuAction{others_cat, "anch_cut", "ct_edit_cut", _("C_ut Anchor"), None, _("Cut the Selected Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_cut)});
    _actions.push_back(CtMenuAction{others_cat, "anch_copy", "ct_edit_copy", _("_Copy Anchor"), None, _("Copy the Selected Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_copy)});
    _actions.push_back(CtMenuAction{others_cat, "anch_del", "ct_edit_delete", _("_Delete Anchor"), None, _("Delete the Selected Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_delete)});
    _actions.push_back(CtMenuAction{others_cat, "anch_edit", "ct_anchor_edit", _("Edit _Anchor"), None, _("Edit the Underlying Anchor"), sigc::mem_fun(*pActions, &CtActions::anchor_edit)});
    _actions.push_back(CtMenuAction{others_cat, "anch_link", "ct_anchor_link", _("Copy Anchor Link"), None, _("Copy Link to the Underlying Anchor to Clipboard"), sigc::mem_fun(*pActions, &CtActions::anchor_link_to_clipboard)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_cut", "ct_edit_cut", _("C_ut Embedded File"), None, _("Cut the Selected Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_cut)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_copy", "ct_edit_copy", _("_Copy Embedded File"), None, _("Copy the Selected Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_copy)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_del", "ct_edit_delete", _("_Delete Embedded File"), None, _("Delete the Selected Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_delete)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_save", "ct_save-as", _("Save _As"), None, _("Save File As"), sigc::mem_fun(*pActions, &CtActions::embfile_save)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_open", "ct_open", _("_Open File"), None, _("Open Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_open)});
    _actions.push_back(CtMenuAction{others_cat, "emb_file_rename", "ct_edit", _("_Rename"), None, _("Rename Embedded File"), sigc::mem_fun(*pActions, &CtActions::embfile_rename)});
    _actions.push_back(CtMenuAction{others_cat, "img_save", "ct_image_save", _("_Save Image as PNG"), None, _("Save the Selected Image as a PNG file"), sigc::mem_fun(*pActions, &CtActions::image_save)});
    _actions.push_back(CtMenuAction{others_cat, "img_edit", "ct_image_edit", _("_Edit Image"), None, _("Edit the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_edit)});
    _actions.push_back(CtMenuAction{others_cat, "img_cut", "ct_edit_cut", _("C_ut Image"), None, _("Cut the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_cut)});
    _actions.push_back(CtMenuAction{others_cat, "img_copy", "ct_edit_copy", _("_Copy Image"), None, _("Copy the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_copy)});
    _actions.push_back(CtMenuAction{others_cat, "img_del", "ct_edit_delete", _("_Delete Image"), None, _("Delete the Selected Image"), sigc::mem_fun(*pActions, &CtActions::image_delete)});
    _actions.push_back(CtMenuAction{others_cat, "img_link_edit", "ct_link_handle", _("Edit _Link"), None, _("Edit the Link Associated to the Image"), sigc::mem_fun(*pActions, &CtActions::image_link_edit)});
    _actions.push_back(CtMenuAction{others_cat, "img_link_dismiss", "ct_clear", _("D_ismiss Link"), None, _("Dismiss the Link Associated to the Image"), sigc::mem_fun(*pActions, &CtActions::image_link_dismiss)});
    _actions.push_back(CtMenuAction{others_cat, "toggle_show_mainwin", CtConst::APP_NAME, _("Show/Hide _CherryTree"), None, _("Toggle Show/Hide CherryTree"), sigc::mem_fun(*pActions, &CtActions::toggle_show_hide_main_window)});

    // add actions in the Windows for the toolbar
    // by default actions will have prefix 'win.'
    // (the menu uses not actions, but accelerators)
    for (const CtMenuAction& action : _actions) {
        pActions->getCtMainWin()->add_action(action.id, action.run_action);
    }

    // for popup menus
    const char* link_cat = "";
    _actions.push_back(CtMenuAction{link_cat, "apply_tag_link", "ct_link_handle", _("Edit _Link"), None, _("Edit the Underlying Link"), sigc::mem_fun(*pActions, &CtActions::apply_tag_link)});
    _actions.push_back(CtMenuAction{link_cat, "link_cut", "ct_edit_cut", _("C_ut Link"), None, _("Cut the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_cut)});
    _actions.push_back(CtMenuAction{link_cat, "link_copy", "ct_edit_copy", _("_Copy Link"), None, _("Copy the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_copy)});
    _actions.push_back(CtMenuAction{link_cat, "link_dismiss", "ct_clear", _("D_ismiss Link"), None, _("Dismiss the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_dismiss)});
    _actions.push_back(CtMenuAction{link_cat, "link_delete", "ct_edit_delete", _("_Delete Link"), None, _("Delete the Selected Link"), sigc::mem_fun(*pActions, &CtActions::link_delete)});
    const char* table_cat = "";
    _actions.push_back(CtMenuAction{table_cat, "table_cut", "ct_edit_cut", _("C_ut Table"), None, _("Cut the Selected Table"), sigc::mem_fun(*pActions, &CtActions::table_cut)});
    _actions.push_back(CtMenuAction{table_cat, "table_copy", "ct_edit_copy", _("_Copy Table"), None, _("Copy the Selected Table"), sigc::mem_fun(*pActions, &CtActions::table_copy)});
    _actions.push_back(CtMenuAction{table_cat, "table_delete", "ct_edit_delete", _("_Delete Table"), None, _("Delete the Selected Table"), sigc::mem_fun(*pActions, &CtActions::table_delete)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_add", "ct_add", _("_Add Column"), None, _("Add a Table Column"), sigc::mem_fun(*pActions, &CtActions::table_column_add)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_delete", "ct_edit_delete", _("De_lete Column"), None, _("Delete the Selected Table Column"), sigc::mem_fun(*pActions, &CtActions::table_column_delete)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_left", "ct_go-back", _("Move Column _Left"), None, _("Move the Selected Column Left"), sigc::mem_fun(*pActions, &CtActions::table_column_left)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_right", "ct_go-forward", _("Move Column _Right"), None, _("Move the Selected Column Right"), sigc::mem_fun(*pActions, &CtActions::table_column_right)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_increase_width", "ct_go-forward", _("Increase Column Width"), KB_CONTROL+"period", _("Increase the Width of the Column"), sigc::mem_fun(*pActions, &CtActions::table_column_increase_width)});
    _actions.push_back(CtMenuAction{table_cat, "table_column_decrease_width", "ct_go-back", _("Decrease Column Width"), KB_CONTROL+KB_ALT+"period", _("Decrease the Width of the Column"), sigc::mem_fun(*pActions, &CtActions::table_column_decrease_width)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_add", "ct_add", _("_Add Row"), KB_CONTROL+"comma", _("Add a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_add)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_cut", "ct_edit_cut", _("Cu_t Row"), None, _("Cut a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_cut)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_copy", "ct_edit_copy", _("_Copy Row"), None, _("Copy a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_copy)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_paste", "ct_edit_paste", _("_Paste Row"), None, _("Paste a Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_paste)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_delete", "ct_edit_delete", _("De_lete Row"), KB_CONTROL+KB_ALT+"comma", _("Delete the Selected Table Row"), sigc::mem_fun(*pActions, &CtActions::table_row_delete)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_up", "ct_go-up", _("Move Row _Up"), None, _("Move the Selected Row Up"), sigc::mem_fun(*pActions, &CtActions::table_row_up)});
    _actions.push_back(CtMenuAction{table_cat, "table_row_down", "ct_go-down", _("Move Row _Down"), None, _("Move the Selected Row Down"), sigc::mem_fun(*pActions, &CtActions::table_row_down)});
    _actions.push_back(CtMenuAction{table_cat, "table_rows_sort_descending", "ct_sort-desc", _("Sort Rows De_scending"), None, _("Sort all the Rows Descending"), sigc::mem_fun(*pActions, &CtActions::table_rows_sort_descending)});
    _actions.push_back(CtMenuAction{table_cat, "table_rows_sort_ascending", "ct_sort-asc", _("Sort Rows As_cending"), None, _("Sort all the Rows Ascending"), sigc::mem_fun(*pActions, &CtActions::table_rows_sort_ascending)});
    _actions.push_back(CtMenuAction{table_cat, "table_edit_properties", "ct_table_edit", _("_Edit Table Properties"), None, _("Edit the Table Properties"), sigc::mem_fun(*pActions, &CtActions::table_edit_properties)});
    _actions.push_back(CtMenuAction{table_cat, "table_export", "ct_table_save", _("_Table Export"), None, _("Export Table as CSV File"), sigc::mem_fun(*pActions, &CtActions::table_export)});
    const char* codebox_cat = "";
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_change_properties", "ct_codebox_edit", _("Change CodeBox _Properties"), None, _("Edit the Properties of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_change_properties)});
    _actions.push_back(CtMenuAction{codebox_cat, "exec_code", "ct_execute", _("_Execute CodeBox Code"), None, _("Execute CodeBox Code"), sigc::mem_fun(*pActions, &CtActions::exec_code)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_load_from_file", "ct_from_txt", _("CodeBox _Load From Text File"), None, _("Load the CodeBox Content From a Text File"), sigc::mem_fun(*pActions, &CtActions::codebox_load_from_file)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_save_to_file", "ct_to_txt", _("CodeBox _Save To Text File"), None, _("Save the CodeBox Content To a Text File"), sigc::mem_fun(*pActions, &CtActions::codebox_save_to_file)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_cut", "ct_edit_cut", _("C_ut CodeBox"), None, _("Cut the Selected CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_cut)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_copy", "ct_edit_copy", _("_Copy CodeBox"), None, _("Copy the Selected CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_copy)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_delete", "ct_edit_delete", _("_Delete CodeBox"), None, _("Delete the Selected CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_delete)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_delete_keeping_text", "ct_edit_delete", _("Delete CodeBox _Keep Content"), None, _("Delete the Selected CodeBox But Keep Its Content"), sigc::mem_fun(*pActions, &CtActions::codebox_delete_keeping_text)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_increase_width", "ct_go-forward", _("Increase CodeBox Width"), KB_CONTROL+"period", _("Increase the Width of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_increase_width)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_decrease_width", "ct_go-back", _("Decrease CodeBox Width"), KB_CONTROL+KB_ALT+"period", _("Decrease the Width of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_decrease_width)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_increase_height", "ct_go-down", _("Increase CodeBox Height"), KB_CONTROL+"comma", _("Increase the Height of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_increase_height)});
    _actions.push_back(CtMenuAction{codebox_cat, "codebox_decrease_height", "ct_go-up", _("Decrease CodeBox Height"), KB_CONTROL+KB_ALT+"comma", _("Decrease the Height of the CodeBox"), sigc::mem_fun(*pActions, &CtActions::codebox_decrease_height)});
}

CtMenuAction* CtMenu::find_action(const std::string& id)
{
    for (CtMenuAction& action : _actions) {
        if (action.id == id) {
            return &action;
        }
    }
    return nullptr;
}

/*static*/ Gtk::MenuItem* CtMenu::find_menu_item(Gtk::MenuBar* menuBar, std::string name)
{
    for (Gtk::Widget* child : menuBar->get_children())
        if (auto menuItem = dynamic_cast<Gtk::MenuItem*>(child))
            if (menuItem->get_name() == name)
                return menuItem;

    // check first level menu items, these menu items have complicated structure
    for (Gtk::Widget* child : menuBar->get_children())
        if (auto menuItem = dynamic_cast<Gtk::MenuItem*>(child))
            if (Gtk::Menu* subMenu = menuItem->get_submenu())
                for (Gtk::Widget* subChild : subMenu->get_children())
                    if (auto subItem = dynamic_cast<Gtk::MenuItem*>(subChild))
                        if (Gtk::Widget* subItemChild = subItem->get_child())
                            if (subItemChild->get_name() == name)
                                return subItem; // it's right, not a subItemChild

    return nullptr;
}

/*static*/ Gtk::AccelLabel* CtMenu::get_accel_label(Gtk::MenuItem* item)
{
    if (auto box = dynamic_cast<Gtk::Box*>(item->get_child()))
        if (auto label = dynamic_cast<Gtk::AccelLabel*>(box->get_children().back()))
            return label;
    return nullptr;
}

/*static*/ int CtMenu::calculate_image_shift(Gtk::MenuItem* menuItem)
{
    if (menuItem)
        if (auto box = dynamic_cast<Gtk::Box *>(menuItem->get_child()))
            if (auto image = dynamic_cast<Gtk::Image *>(box->get_children()[0])) {
                auto allocation_menuitem = menuItem->get_allocation();
                auto allocation_image = image->get_allocation();

                int shift = -allocation_image.get_x();
                if (menuItem->get_direction() == Gtk::TEXT_DIR_RTL) {
                    shift += (allocation_menuitem.get_width() - allocation_image.get_width());
                }
                return shift;
            }
    return 0;
}

std::vector<Gtk::Toolbar*> CtMenu::build_toolbars(Gtk::MenuToolButton*& pRecentDocsMenuToolButton)
{
    pRecentDocsMenuToolButton = nullptr;
    std::vector<Gtk::Toolbar*> toolbars;
    for (const auto& toolbar_str: _get_ui_str_toolbars()) {
        Gtk::Toolbar* pToolbar = nullptr;
        _rGtkBuilder->add_from_string(toolbar_str);
        _rGtkBuilder->get_widget("ToolBar" + std::to_string(toolbars.size()), pToolbar);
        toolbars.push_back(pToolbar);
        if (!pRecentDocsMenuToolButton)
            _rGtkBuilder->get_widget("RecentDocs", pRecentDocsMenuToolButton);
    }
    return toolbars;
}

Gtk::MenuBar* CtMenu::build_menubar()
{
    Gtk::MenuBar* pMenuBar = Gtk::manage(new Gtk::MenuBar());
    _walk_menu_xml(pMenuBar, _get_ui_str_menu(), nullptr);
    return pMenuBar;
}

Gtk::Menu* CtMenu::build_bookmarks_menu(std::list<std::pair<gint64, std::string>>& bookmarks,
                                        sigc::slot<void, gint64>& bookmark_action)
{
    Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu());
    _add_menu_item(pMenu, find_action("handle_bookmarks"));
    _add_menu_separator(pMenu);
    for (const auto& bookmark : bookmarks) {
        const gint64& node_id = bookmark.first;
        const std::string& node_name = bookmark.second;
        Gtk::MenuItem* pMenuItem = _add_menu_item(pMenu, node_name.c_str(), "ct_pin", nullptr, _pAccelGroup, node_name.c_str(), nullptr, nullptr, nullptr);
        pMenuItem->signal_activate().connect(sigc::bind(bookmark_action, node_id));
    }
    return pMenu;
}

Gtk::Menu* CtMenu::build_recent_docs_menu(const CtRecentDocsFilepaths& recentDocsFilepaths,
                                          sigc::slot<void, const std::string&>& recent_doc_open_action,
                                          sigc::slot<void, const std::string&>& recent_doc_rm_action)
{
    Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu());
    for (const fs::path& filepath : recentDocsFilepaths) {
        bool file_exists = fs::exists(filepath);
        Gtk::MenuItem* pMenuItem = _add_menu_item(pMenu, filepath.c_str(), file_exists ? "ct_open" : "ct_urgent", nullptr, _pAccelGroup, filepath.c_str(), nullptr, nullptr, nullptr, false/*use_underline*/);
        pMenuItem->signal_activate().connect(sigc::bind(recent_doc_open_action, filepath.string()));
    }
    Gtk::MenuItem* pMenuItemRm = _add_menu_item(pMenu, _("Remove from list"), "ct_edit_delete", nullptr, _pAccelGroup, _("Remove from list"), nullptr, nullptr, nullptr);
    Gtk::Menu* pMenuRm = Gtk::manage(new Gtk::Menu());
    pMenuItemRm->set_submenu(*pMenuRm);
    for (const fs::path& filepath : recentDocsFilepaths) {
        bool file_exists = fs::exists(filepath);
        Gtk::MenuItem* pMenuItem = _add_menu_item(pMenuRm, filepath.c_str(), file_exists ? "ct_edit_delete" : "ct_urgent", nullptr, _pAccelGroup, filepath.c_str(), nullptr, nullptr, nullptr);
        pMenuItem->signal_activate().connect(sigc::bind(recent_doc_rm_action, filepath.string()));
    }
    return pMenu;
}

Gtk::Menu* CtMenu::get_popup_menu(POPUP_MENU_TYPE popupMenuType)
{
    if (_popupMenus[popupMenuType] == nullptr) {
        Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu{});
        build_popup_menu(pMenu, popupMenuType);
        _popupMenus[popupMenuType] = pMenu;
    }
    return _popupMenus[popupMenuType];
}

void CtMenu::build_popup_menu(Gtk::Menu* pMenu, POPUP_MENU_TYPE popupMenuType)
{
    switch (popupMenuType) {
        case CtMenu::POPUP_MENU_TYPE::Node: _walk_menu_xml(pMenu, _get_ui_str_menu(), "/menubar/menu[@action='TreeMenu']/*"); break;
        case CtMenu::POPUP_MENU_TYPE::Text: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_text(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Code: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_code(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Image: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_image(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Anchor: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_anchor(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::EmbFile: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_embfile(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Link: {
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("apply_tag_link"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("link_cut"));
            _add_menu_item(pMenu, find_action("link_copy"));
            _add_menu_item(pMenu, find_action("link_dismiss"));
            _add_menu_item(pMenu, find_action("link_delete"));
        } break;
        case CtMenu::POPUP_MENU_TYPE::Codebox: {
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("cut_plain"));
            _add_menu_item(pMenu, find_action("copy_plain"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("codebox_change_properties"));
            _add_menu_item(pMenu, find_action("exec_code"));
            _add_menu_item(pMenu, find_action("codebox_load_from_file"));
            _add_menu_item(pMenu, find_action("codebox_save_to_file"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("codebox_cut"));
            _add_menu_item(pMenu, find_action("codebox_copy"));
            _add_menu_item(pMenu, find_action("codebox_delete"));
            _add_menu_item(pMenu, find_action("codebox_delete_keeping_text"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("codebox_increase_width"));
            _add_menu_item(pMenu, find_action("codebox_decrease_width"));
            _add_menu_item(pMenu, find_action("codebox_increase_height"));
            _add_menu_item(pMenu, find_action("codebox_decrease_height"));
        } break;
        case CtMenu::POPUP_MENU_TYPE::PopupMenuNum: {
        } break;
    }
}

void CtMenu::build_popup_menu_table_cell(Gtk::Menu* pMenu,
                                         const bool first_row,
                                         const bool first_col,
                                         const bool last_row,
                                         const bool last_col)
{
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_cut"));
    _add_menu_item(pMenu, find_action("table_copy"));
    _add_menu_item(pMenu, find_action("table_delete"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_column_add"));
    _add_menu_item(pMenu, find_action("table_column_delete"));
    _add_menu_separator(pMenu);
    if (not first_col) _add_menu_item(pMenu, find_action("table_column_left"));
    if (not last_col) _add_menu_item(pMenu, find_action("table_column_right"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_column_increase_width"));
    _add_menu_item(pMenu, find_action("table_column_decrease_width"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_row_add"));
    _add_menu_item(pMenu, find_action("table_row_delete"));
    _add_menu_item(pMenu, find_action("table_row_cut"));
    _add_menu_item(pMenu, find_action("table_row_copy"));
    _add_menu_item(pMenu, find_action("table_row_paste"));
    _add_menu_separator(pMenu);
    if (not first_row) _add_menu_item(pMenu, find_action("table_row_up"));
    if (not last_row) _add_menu_item(pMenu, find_action("table_row_down"));
    _add_menu_item(pMenu, find_action("table_rows_sort_descending"));
    _add_menu_item(pMenu, find_action("table_rows_sort_ascending"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_edit_properties"));
    _add_menu_item(pMenu, find_action("table_export"));
}

void CtMenu::_walk_menu_xml(Gtk::MenuShell* pMenuShell, const char* document, const char* xpath)
{
    xmlpp::DomParser parser;
    if (not CtXmlHelper::safe_parse_memory(parser, document)) {
        return;
    }
    if (xpath) {
        _walk_menu_xml(pMenuShell, parser.get_document()->get_root_node()->find(xpath)[0]);
    }
    else {
        _walk_menu_xml(pMenuShell, parser.get_document()->get_root_node());
    }
}

void CtMenu::_walk_menu_xml(Gtk::MenuShell* pMenuShell, xmlpp::Node* pNode)
{
    for (xmlpp::Node* pNodeIter = pNode; pNodeIter; pNodeIter = pNodeIter->get_next_sibling()) {
        if (pNodeIter->get_name() == "menubar" || pNodeIter->get_name() == "popup") {
            _walk_menu_xml(pMenuShell, pNodeIter->get_first_child());
        }
        else if (pNodeIter->get_name() == "menu") {
            if (xmlpp::Attribute* pAttrName = get_attribute(pNodeIter, "_name")) // menu name which need to be translated
            {
                xmlpp::Attribute* pAttrImage = get_attribute(pNodeIter, "image");
                Gtk::Menu* pSubmenu = _add_menu_submenu(pMenuShell, pAttrName->get_value().c_str(), _(pAttrName->get_value().c_str()), pAttrImage->get_value().c_str());
                _walk_menu_xml(pSubmenu, pNodeIter->get_first_child());
            }
            else { // otherwise it is an action id
                CtMenuAction const* pAction = find_action(get_attribute(pNodeIter, "action")->get_value());
                Gtk::Menu* pSubmenu = _add_menu_submenu(pMenuShell, pAction->id.c_str(), pAction->name.c_str(), pAction->image.c_str());
                _walk_menu_xml(pSubmenu, pNodeIter->get_first_child());
            }
        }
        else if (pNodeIter->get_name() == "menuitem") {
            CtMenuAction* pAction = find_action(get_attribute(pNodeIter, "action")->get_value());
            _add_menu_item(pMenuShell, pAction);
        }
        else if (pNodeIter->get_name() == "separator") {
            _add_menu_separator(pMenuShell);
        }
    }
}

Gtk::Menu* CtMenu::_add_menu_submenu(Gtk::MenuShell* pMenuShell, const char* id, const char* name, const char* image)
{
    Gtk::MenuItem* pMenuItem = Gtk::manage(new Gtk::MenuItem{});
    pMenuItem->set_name(id);
    Gtk::AccelLabel* pLabel = Gtk::manage(new Gtk::AccelLabel{name, true});
    pLabel->set_xalign(0.0);
    pLabel->set_accel_widget(*pMenuItem);

    _add_menu_item_image_or_label(pMenuItem, image, pLabel);
    pMenuItem->get_child()->set_name(id); // for find_menu_item()
    pMenuItem->show_all();

    Gtk::Menu* pSubMenu = Gtk::manage(new Gtk::Menu{});
    pMenuItem->set_submenu(*pSubMenu);
    pMenuShell->append(*pMenuItem);
    return pSubMenu;
}

Gtk::MenuItem* CtMenu::_add_menu_item(Gtk::MenuShell* pMenuShell, CtMenuAction* pAction)
{
    std::string shortcut = pAction->get_shortcut(_pCtConfig);
    Gtk::MenuItem* pMenuItem = _add_menu_item(pMenuShell,
                                              pAction->name.c_str(),
                                              pAction->image.c_str(),
                                              shortcut.c_str(),
                                              _pAccelGroup,
                                              pAction->desc.c_str(),
                                              (gpointer)pAction,
                                              &pAction->signal_set_sensitive,
                                              &pAction->signal_set_visible);
    pMenuItem->get_child()->set_name(pAction->id); // for find_menu_item();
    return pMenuItem;
}

// based on inkscape/src/ui/desktop/menubar.cpp
/*static*/ Gtk::MenuItem* CtMenu::_add_menu_item(Gtk::MenuShell* pMenuShell,
                                                 const char* name,
                                                 const char* image,
                                                 const char* shortcut,
                                                 Glib::RefPtr<Gtk::AccelGroup> accelGroup,
                                                 const char* desc,
                                                 gpointer action_data,
                                                 sigc::signal<void, bool>* signal_set_sensitive,
                                                 sigc::signal<void, bool>* signal_set_visible,
                                                 const bool use_underline/*= true*/)
{
    Gtk::MenuItem* pMenuItem = Gtk::manage(new Gtk::MenuItem{});

    if (desc && strlen(desc)) {
        pMenuItem->set_tooltip_text(desc);
    }
    // Now create the label and add it to the menu item
    Gtk::AccelLabel* pLabel = Gtk::manage(new Gtk::AccelLabel{name, use_underline});
    pLabel->set_xalign(0.0);
    pLabel->set_accel_widget(*pMenuItem);
    if (shortcut && strlen(shortcut)) {
        Gtk::AccelKey accel_key(shortcut);
        pMenuItem->add_accelerator("activate", accelGroup, accel_key.get_key(), accel_key.get_mod(), Gtk::ACCEL_VISIBLE);
    }

    _add_menu_item_image_or_label(pMenuItem, image, pLabel);

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

    if (action_data) {
        gtk_widget_set_events(GTK_WIDGET(pMenuItem->gobj()), GDK_KEY_PRESS_MASK);
        g_signal_connect(G_OBJECT(pMenuItem->gobj()), "activate", G_CALLBACK(on_menu_activate), action_data);
    }

    pMenuItem->show_all();
    pMenuShell->append(*pMenuItem);

    return pMenuItem;
}

/*static*/ void CtMenu::_add_menu_item_image_or_label(Gtk::MenuItem* pMenuItem, const char* image, Gtk::AccelLabel* pLabel)
{
    if (image && strlen(image)) {
        pMenuItem->set_name("ImageMenuItem");  // custom name to identify our "ImageMenuItems"
        Gtk::Image *pIcon = Gtk::manage(new Gtk::Image{});
        pIcon->set_from_icon_name(image, Gtk::ICON_SIZE_MENU);

        // create a box to hold icon and label as GtkMenuItem derives from GtkBin and can only hold one child
        auto const box = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
        box->pack_start(*pIcon, false, false, 0);
        box->pack_start(*pLabel, true, true, 0);

        pMenuItem->add(*box);

        // to fix image placement in MenuBar /context menu
        // based on inkscape: src/ui/desktop/menu-icon-shift.cpp
        // we don't know which MenuItem will be first mapped, so connect  all of them
        static std::list<sigc::connection>* static_map_connectons = new std::list<sigc::connection>();
        if (static_map_connectons != nullptr) { // if null then the fix was applied
            static_map_connectons->push_back(pMenuItem->signal_map().connect([pMenuItem]
            {
                spdlog::debug("shift images in MenuBar/context menu");
                int shift = CtMenu::calculate_image_shift(pMenuItem);
                if (shift != 0) {
                    auto provider = Gtk::CssProvider::create();
                    auto const screen = Gdk::Screen::get_default();
                    Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                    if (pMenuItem->get_direction() == Gtk::TEXT_DIR_RTL) {
                        provider->load_from_data("menuitem box image {margin-right:" + std::to_string(shift) + "px;}");
                    } else {
                        provider->load_from_data("menuitem box image {margin-left:" + std::to_string(shift) + "px;}");
                    }
                }
                // we don't need to call this again, so kill all map connections
                for (auto& connections: *static_map_connectons)
                    connections.disconnect();
                delete static_map_connectons;
                static_map_connectons = nullptr;
            }));
        }
    }
    else {
        pMenuItem->add(*pLabel);
    }
}

Gtk::SeparatorMenuItem* CtMenu::_add_menu_separator(Gtk::MenuShell* pMenuShell)
{
    Gtk::SeparatorMenuItem* pSeparatorItem = Gtk::manage(new Gtk::SeparatorMenuItem());
    pSeparatorItem->show_all();
    pMenuShell->append(*pSeparatorItem);
    return pSeparatorItem;
}

std::vector<std::string> CtMenu::_get_ui_str_toolbars()
{
    auto generate_ui = [&](size_t id, const std::vector<std::string>& items)
    {
        std::string str;
        for (const std::string& element: items) {
            if (element == CtConst::TAG_SEPARATOR) {
                str += "<child><object class='GtkSeparatorToolItem'/></child>";
            }
            else {
                const bool isOpenRecent{element == CtConst::CHAR_STAR};
                CtMenuAction const* pAction = isOpenRecent ? find_action("ct_open_file") : find_action(element);
                if (pAction)
                {
                    if (isOpenRecent) str += "<child><object class='GtkMenuToolButton' id='RecentDocs'>";
                    else str += "<child><object class='GtkToolButton'>";
                    str += "<property name='action-name'>win." + pAction->id + "</property>"; // 'win.' is a default action group in Window
                    str += "<property name='icon-name'>" + pAction->image + "</property>";
                    str += "<property name='label'>" + pAction->name + "</property>";
                    str += "<property name='tooltip-text'>" + pAction->desc + "</property>";
                    str += "<property name='visible'>True</property>";
                    str += "<property name='use_underline'>True</property>";
                    str += "</object></child>";
                }
            }
        }
        str = "<interface><object class='GtkToolbar' id='ToolBar" + std::to_string(id) + "'>"
                "<property name='visible'>True</property>"
                "<property name='can_focus'>False</property>"
                + str +
                "</object></interface>";
        return str;
    };

    std::vector<std::string> toolbarUIstr;
    std::vector<std::string> vecToolbarElements = str::split(_pCtConfig->toolbarUiList, ",");
    std::vector<std::string> toolbar_accumulator;
    for (const std::string& element : vecToolbarElements) {
        if (element != CtConst::TOOLBAR_SPLIT)
            toolbar_accumulator.push_back(element);
        else if (!toolbar_accumulator.empty()) {
            toolbarUIstr.push_back(generate_ui(toolbarUIstr.size(), toolbar_accumulator));
            toolbar_accumulator.clear();
        }
    }

    if (!toolbar_accumulator.empty()) {
        toolbarUIstr.push_back(generate_ui(toolbarUIstr.size(), toolbar_accumulator));
        toolbar_accumulator.clear();
    }

    return toolbarUIstr;
}

const char* CtMenu::_get_ui_str_menu()
{
    return R"MARKUP(
<menubar name='MenuBar'>
  <menu action='FileMenu'>
    <menuitem action='ct_new_inst'/>
    <menuitem action='ct_open_file'/>
    <menu action='RecentDocsMenu'>
    </menu>
    <separator/>
    <menuitem action='ct_vacuum'/>
    <menuitem action='ct_save'/>
    <menuitem action='ct_save_as'/>
    <separator/>
    <menuitem action='print_page_setup'/>
    <menuitem action='do_print'/>
    <separator/>
    <menuitem action='preferences_dlg'/>
    <menuitem action='tree_parse_info'/>
    <separator/>
    <menuitem action='command_palette'/>
    <menuitem action='exec_code'/>
    <separator/>
    <menuitem action='quit_app'/>
    <menuitem action='exit_app'/>
  </menu>

  <menu action='EditMenu'>
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
    <menuitem action='insert_special_char'/>
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
    <menuitem action='fmt_clone'/>
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
    <menuitem action='fmt_indent'/>
    <menuitem action='fmt_unindent'/>
    <separator/>
    <menuitem action='fmt_justify_left'/>
    <menuitem action='fmt_justify_center'/>
    <menuitem action='fmt_justify_right'/>
    <menuitem action='fmt_justify_fill'/>
  </menu>

  <menu action='TreeMenu'>
    <menuitem action='tree_add_node'/>
    <menuitem action='tree_add_subnode'/>
    <menuitem action='tree_dup_node'/>
    <menuitem action='tree_dup_node_subnodes'/>
    <separator/>
    <menuitem action='tree_node_prop'/>
    <menuitem action='tree_node_toggle_ro'/>
    <menuitem action='node_bookmark'/>
    <menuitem action='node_unbookmark'/>
    <menuitem action='tree_node_link'/>
    <menuitem action='tree_node_date'/>
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
      <menuitem action='import_md_file'/>
      <menuitem action='import_md_folder'/>
      <menuitem action='import_gnote'/>
      <menuitem action='import_keepnote'/>
      <menuitem action='import_leo'/>
      <menuitem action='import_mempad'/>
      <menuitem action='import_notecase'/>
      <menuitem action='import_rednotebook'/>
      <menuitem action='import_tomboy'/>
      <menuitem action='import_treepad'/>
      <menuitem action='import_zim'/>
      <menuitem action='import_pandoc_file'/>
    </menu>
    <separator/>
    <menu action='TreeExportMenu'>
      <menuitem action='export_pdf'/>
      <menuitem action='export_html'/>
      <menuitem action='export_txt'/>
      <menuitem action='export_ctd'/>
    </menu>
    <separator/>
    <menuitem action='child_nodes_inherit_syntax'/>
    <separator/>
    <menuitem action='tree_node_del'/>
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
    <menuitem action='import_md_file'/>
    <menuitem action='import_md_folder'/>
    <menuitem action='import_gnote'/>
    <menuitem action='import_keepnote'/>
    <menuitem action='import_leo'/>
    <menuitem action='import_mempad'/>
    <menuitem action='import_notecase'/>
    <menuitem action='import_rednotebook'/>
    <menuitem action='import_tomboy'/>
    <menuitem action='import_treepad'/>
    <menuitem action='import_zim'/>
    <menuitem action='import_pandoc_file'/>
  </menu>

  <menu action='ExportMenu'>
    <menuitem action='export_pdf'/>
    <menuitem action='export_html'/>
    <menuitem action='export_txt'/>
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

const char* CtMenu::_get_popup_menu_ui_str_text()
{
    return R"MARKUP(
<popup>
  <separator/>
  <menuitem action='cut_plain'/>
  <menuitem action='copy_plain'/>
  <menuitem action='paste_plain'/>
  <separator/>
  <menu _name='For_matting' image='ct_fmt-txt'>
    <menuitem action='fmt_clone'/>
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
  <menu _name='_List' image='ct_list_bulleted'>
    <menuitem action='handle_bull_list'/>
    <menuitem action='handle_num_list'/>
    <menuitem action='handle_todo_list'/>
  </menu>
  <menuitem action='fmt_indent'/>
  <menuitem action='fmt_unindent'/>
  <menu _name='_Justify' image='ct_justify-center'>
    <menuitem action='fmt_justify_left'/>
    <menuitem action='fmt_justify_center'/>
    <menuitem action='fmt_justify_right'/>
    <menuitem action='fmt_justify_fill'/>
  </menu>
  <separator/>
  <menu _name='_Insert' image='ct_insert'>
    <menuitem action='handle_image'/>
    <menuitem action='handle_table'/>
    <menuitem action='handle_codebox'/>
    <menuitem action='handle_embfile'/>
    <menuitem action='handle_link'/>
    <menuitem action='handle_anchor'/>
    <menuitem action='insert_toc'/>
    <menuitem action='insert_timestamp'/>
    <menuitem action='insert_special_char'/>
    <menuitem action='insert_horiz_rule'/>
  </menu>
  <menu _name='C_hange Case' image='ct_case_toggle'>
    <menuitem action='case_down'/>
    <menuitem action='case_up'/>
    <menuitem action='case_tggl'/>
  </menu>
  <menu _name='_Row' image='ct_edit'>
    <menuitem action='cut_row'/>
    <menuitem action='copy_row'/>
    <menuitem action='del_row'/>
    <menuitem action='dup_row'/>
    <menuitem action='mv_up_row'/>
    <menuitem action='mv_down_row'/>
  </menu>
  <menuitem action='strip_trail_spaces'/>
  <separator/>
  <menu _name='_Search' image='ct_find'>
    <menuitem action='find_in_node'/>
    <menuitem action='find_in_allnodes'/>
    <menuitem action='find_in_node_n_sub'/>
    <menuitem action='find_in_node_names'/>
    <menuitem action='find_iter_fw'/>
    <menuitem action='find_iter_bw'/>
  </menu>
  <menu _name='_Replace' image='ct_find_replace'>
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
#if 0
// the following is to have xgettext add <menu _name='THIS_STRING' to the strings to be translated
_("For_matting"),_("_List"),_("_Justify"),_("_Insert"),_("C_hange Case"),_("_Row"),_("_Search"),_("_Replace")
#endif

const char* CtMenu::_get_popup_menu_ui_str_code()
{
    return R"MARKUP(
<popup>
  <separator/>
  <menuitem action='cut_plain'/>
  <menuitem action='copy_plain'/>
  <separator/>
  <menuitem action='exec_code'/>
  <menu _name='_Insert' image='ct_insert'>
    <menuitem action='insert_timestamp'/>
    <menuitem action='insert_special_char'/>
    <menuitem action='insert_horiz_rule'/>
  </menu>
  <menuitem action='strip_trail_spaces'/>
  <menu _name='C_hange Case' image='ct_case_toggle'>
    <menuitem action='case_down'/>
    <menuitem action='case_up'/>
    <menuitem action='case_tggl'/>
  </menu>
  <menu _name='_Row' image='ct_edit'>
    <menuitem action='cut_row'/>
    <menuitem action='copy_row'/>
    <menuitem action='del_row'/>
    <menuitem action='dup_row'/>
    <menuitem action='mv_up_row'/>
    <menuitem action='mv_down_row'/>
  </menu>
  <separator/>
  <menu _name='_Search' image='ct_find'>
    <menuitem action='find_in_node'/>
    <menuitem action='find_in_allnodes'/>
    <menuitem action='find_in_node_n_sub'/>
    <menuitem action='find_in_node_names'/>
    <menuitem action='find_iter_fw'/>
    <menuitem action='find_iter_bw'/>
  </menu>
  <menu _name='_Replace' image='ct_find_replace'>
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
#if 0
// the following is to have xgettext add <menu _name='THIS_STRING' to the strings to be translated
_("_Insert"),_("C_hange Case"),_("_Row"),_("_Search"),_("_Replace")
#endif

const char* CtMenu::_get_popup_menu_ui_str_image()
{
    return R"MARKUP(
<popup>
  <menuitem action='img_cut'/>
  <menuitem action='img_copy'/>
  <menuitem action='img_del'/>
  <separator/>
  <menuitem action='img_edit'/>
  <menuitem action='img_save'/>
  <separator/>
  <menuitem action='img_link_edit'/>
  <menuitem action='img_link_dismiss'/>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_anchor()
{
    return R"MARKUP(
<popup>
  <menuitem action='anch_cut'/>
  <menuitem action='anch_copy'/>
  <menuitem action='anch_del'/>
  <separator/>
  <menuitem action='anch_link'/>
  <menuitem action='anch_edit'/>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_embfile()
{
    return R"MARKUP(
<popup>
  <menuitem action='emb_file_cut'/>
  <menuitem action='emb_file_copy'/>
  <menuitem action='emb_file_del'/>
  <separator/>
  <menuitem action='emb_file_open'/>
  <menuitem action='emb_file_save'/>
  <separator/>
  <menuitem action='emb_file_rename'/>
</popup>
    )MARKUP";
}
