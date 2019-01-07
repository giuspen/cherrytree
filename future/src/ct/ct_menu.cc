/*
 * ct_menu.cc
 *
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

static xmlpp::Attribute*
get_attribute(xmlpp::Node* node, char const *name)
{
    xmlpp::Element* element = static_cast<xmlpp::Element*>(node);
    return element->get_attribute(name);
}

static void
on_menu_activate(void */*object*/, CtAction *action)
{
    action->run_action();
}

CtMenu::CtMenu()
{

}


void CtMenu::init_actions(CtApp *app)
{
    const std::string
            None = "",
            KB_CONTROL = "<control>",
            KB_SHIFT = "<shift>",
            KB_ALT = "<alt>";

    // CtAction::run_action constructor cannnot take sigc::mem_fun as a parameter
    // thus create a signal via this function
    typedef void (CtApp::*fun_ptr)(void);
    auto mem_fun = [](CtApp *app, fun_ptr fun) -> sigc::signal<void> {
        sigc::signal<void> c;
        c.connect(sigc::mem_fun(*app, fun));
        return c;
    };

    // for menu bar
    _actions.push_back(CtAction{"FileMenu", None, _("_File"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"EditMenu", None, _("_Edit"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"FormattingMenu", None, _("For_matting"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"TreeMenu", None, _("_Tree"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"TreeMoveMenu", "gtk-jump-to", ("Node _Move"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"TreeImportMenu", CtConst::STR_STOCK_CT_IMP, _("Nodes _Import"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"TreeExportMenu", "export_from_cherrytree", _("Nodes E_xport"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"ChangeCaseMenu", "case_toggle", _("C_hange Case"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"SearchMenu", None, _("_Search"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"ViewMenu", None, _("_View"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"BookmarksMenu", None, _("_Bookmarks"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"ImportMenu", None, _("_Import"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"ExportMenu", None, _("E_xport"), None, None, sigc::signal<void>()});
    _actions.push_back(CtAction{"HelpMenu", None, _("_Help"), None, None, sigc::signal<void>()});

    _actions.push_back(CtAction{"ct_new_inst", "new-instance", _("New _Instance"), None, _("Start a New Instance of CherryTree"), sigc::signal<void>() /* dad.file_new */});
    _actions.push_back(CtAction{"ct_open_file", "gtk-open", _("_Open File"), KB_CONTROL+"O", _("Open a CherryTree Document"), sigc::signal<void>() /* dad.file_open */});
    _actions.push_back(CtAction{"ct_save", "gtk-save", _("_Save"), KB_CONTROL+"S", _("Save File"), sigc::signal<void>() /* dad.file_save */});
    _actions.push_back(CtAction{"ct_vacuum", "gtk-clear", _("Save and _Vacuum"), None, _("Save File and Vacuum"), sigc::signal<void>() /* dad.file_vacuum */});
    _actions.push_back(CtAction{"ct_save_as", "gtk-save-as", _("Save _As"), KB_CONTROL+KB_SHIFT+"S", _("Save File As"), sigc::signal<void>() /* dad.file_save_as */});
    _actions.push_back(CtAction{"exec_code", "gtk-execute", _("_Execute Code"), "F5", _("Execute Code"), sigc::signal<void>() /* dad.exec_code */});
    _actions.push_back(CtAction{"open_cfg_folder", "gtk-directory", _("Open Preferences _Directory"), None, _("Open the Directory with Preferences Files"), sigc::signal<void>() /* dad.folder_cfg_open */});
    _actions.push_back(CtAction{"print_page_setup", "gtk-print", _("Pa_ge Setup"), KB_CONTROL+KB_SHIFT+"P", _("Set up the Page for Printing"), sigc::signal<void>() /* dad.export_print_page_setup */});
    _actions.push_back(CtAction{"do_print", "gtk-print", _("_Print"), KB_CONTROL+"P", _("Print"), sigc::signal<void>() /* dad.export_print */});
    _actions.push_back(CtAction{"quit_app", "quit-app", _("_Quit"), KB_CONTROL+"Q", _("Quit the Application"), mem_fun(app, &CtApp::quit_application) /* dad.quit_application */});
    _actions.push_back(CtAction{"exit_app", "quit-app", _("_Exit CherryTree"), KB_CONTROL+KB_SHIFT+"Q", _("Exit from CherryTree"), sigc::signal<void>() /* dad.quit_application_totally */});
    _actions.push_back(CtAction{"preferences_dlg", "gtk-preferences", _("_Preferences"), KB_CONTROL+KB_ALT+"P", _("Preferences"), sigc::signal<void>() /* dad.dialog_preferences */});
    _actions.push_back(CtAction{"act_undo", "gtk-undo", _("_Undo"), KB_CONTROL+"Z", _("Undo Last Operation"), sigc::signal<void>() /* dad.requested_step_back */});
    _actions.push_back(CtAction{"act_redo", "gtk-redo", _("_Redo"), KB_CONTROL+"Y", _("Redo Previously Discarded Operation"), sigc::signal<void>() /* dad.requested_step_ahead */});
    _actions.push_back(CtAction{"handle_image", "image_insert", _("Insert I_mage"), KB_CONTROL+KB_ALT+"I", _("Insert an Image"), sigc::signal<void>() /* dad.image_handle */});
    //_actions.push_back(CtAction{"handle_screenshot", "screenshot_insert", _("Insert _Screenshot"), KB_CONTROL+KB_SHIFT+KB_ALT+"S", _("Insert a Screenshot"), sigc::signal<void>() /* dad.screenshot_handle */});
    _actions.push_back(CtAction{"handle_table", "table_insert", _("Insert _Table"), KB_CONTROL+KB_ALT+"T", _("Insert a Table"), sigc::signal<void>() /* dad.table_handle */});
    _actions.push_back(CtAction{"handle_codebox", "codebox_insert", _("Insert _CodeBox"), KB_CONTROL+KB_ALT+"C", _("Insert a CodeBox"), sigc::signal<void>() /* dad.codebox_handle */});
    _actions.push_back(CtAction{"handle_embfile", "file_icon", _("Insert _File"), KB_CONTROL+KB_ALT+"E", _("Insert File"), sigc::signal<void>() /* dad.embfile_insert */});
    _actions.push_back(CtAction{"handle_link", "link_handle", _("Insert/Edit _Link"), KB_CONTROL+"L", _("Insert a Link/Edit the Underlying Link"), sigc::signal<void>() /* dad.apply_tag_link */});
    _actions.push_back(CtAction{"handle_anchor", "anchor_insert", _("Insert _Anchor"), KB_CONTROL+KB_ALT+"A", _("Insert an Anchor"), sigc::signal<void>() /* dad.anchor_handle */});
    _actions.push_back(CtAction{"insert_toc", "index", _("Insert T_OC"), None, _("Insert Table of Contents"), sigc::signal<void>() /* dad.toc_insert */});
    _actions.push_back(CtAction{"insert_timestamp", "timestamp", _("Insert Ti_mestamp"), KB_CONTROL+KB_ALT+"M", _("Insert Timestamp"), sigc::signal<void>() /* dad.timestamp_insert */});
    _actions.push_back(CtAction{"insert_horiz_rule", "horizontal_rule", _("Insert _Horizontal Rule"), KB_CONTROL+"R", _("Insert Horizontal Rule"), sigc::signal<void>() /* dad.horizontal_rule_insert */});
    _actions.push_back(CtAction{"case_down", "case_lower", _("_Lower Case of Selection/Word"), KB_CONTROL+"W", _("Lower the Case of the Selection/the Underlying Word"), sigc::signal<void>() /* dad.text_selection_lower_case */});
    _actions.push_back(CtAction{"case_up", "case_upper", _("_Upper Case of Selection/Word"), KB_CONTROL+KB_SHIFT+"W", _("Upper the Case of the Selection/the Underlying Word"), sigc::signal<void>() /* dad.text_selection_upper_case */});
    _actions.push_back(CtAction{"case_tggl", "case_toggle", _("_Toggle Case of Selection/Word"), KB_CONTROL+"G", _("Toggle the Case of the Selection/the Underlying Word"), sigc::signal<void>() /* dad.text_selection_toggle_case */});
    _actions.push_back(CtAction{"spellcheck_toggle", "gtk-spell-check", _("Enable/Disable _Spell Check"), KB_CONTROL+KB_ALT+"S", _("Toggle Enable/Disable Spell Check"), sigc::signal<void>() /* dad.toggle_ena_dis_spellcheck */});
    _actions.push_back(CtAction{"cut_plain", "edit-cut", _("Cu_t as Plain Text"), KB_CONTROL+KB_SHIFT+"X", _("Cut as Plain Text, Discard the Rich Text Formatting"), sigc::signal<void>() /* dad.cut_as_plain_text */});
    _actions.push_back(CtAction{"copy_plain", "edit-copy", _("_Copy as Plain Text"), KB_CONTROL+KB_SHIFT+"C", _("Copy as Plain Text, Discard the Rich Text Formatting"), sigc::signal<void>() /* dad.copy_as_plain_text */});
    _actions.push_back(CtAction{"paste_plain", "edit-paste", _("_Paste as Plain Text"), KB_CONTROL+KB_SHIFT+"V", _("Paste as Plain Text, Discard the Rich Text Formatting"), sigc::signal<void>() /* dad.paste_as_plain_text */});
    _actions.push_back(CtAction{"cut_row", "edit-cut", _("Cu_t Row"), KB_SHIFT+KB_ALT+"X", _("Cut the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_cut */});
    _actions.push_back(CtAction{"copy_row", "edit-copy", _("_Copy Row"), KB_SHIFT+KB_ALT+"C", _("Copy the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_copy */});
    _actions.push_back(CtAction{"del_row", "edit-delete", _("De_lete Row"), KB_CONTROL+"K", _("Delete the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_delete */});
    _actions.push_back(CtAction{"dup_row", "gtk-add", _("_Duplicate Row"), KB_CONTROL+"D", _("Duplicate the Current Row/Selection"), sigc::signal<void>() /* dad.text_row_selection_duplicate */});
    _actions.push_back(CtAction{"mv_up_row", "gtk-go-up", _("Move _Up Row"), KB_ALT+CtConst::STR_KEY_UP, _("Move Up the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_up */});
    _actions.push_back(CtAction{"mv_down_row", "gtk-go-down", _("Move _Down Row"), KB_ALT+CtConst::STR_KEY_DOWN, _("Move Down the Current Row/Selected Rows"), sigc::signal<void>() /* dad.text_row_down */});
    _actions.push_back(CtAction{"fmt_latest", "format_text_latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), sigc::signal<void>() /* dad.apply_tag_latest */});
    _actions.push_back(CtAction{"fmt_rm", "format_text_clear", _("_Remove Formatting"), KB_CONTROL+KB_SHIFT+"R", _("Remove the Formatting from the Selected Text"), sigc::signal<void>() /* dad.remove_text_formatting */});
    _actions.push_back(CtAction{"fmt_color_fg", "color_foreground", _("Text _Color Foreground"), KB_SHIFT+KB_ALT+"F", _("Change the Color of the Selected Text Foreground"), sigc::signal<void>() /* dad.apply_tag_foreground */});
    _actions.push_back(CtAction{"fmt_color_bg", "color_background", _("Text C_olor Background"), KB_SHIFT+KB_ALT+"B", _("Change the Color of the Selected Text Background"), sigc::signal<void>() /* dad.apply_tag_background */});
    _actions.push_back(CtAction{"fmt_bold", "format-text-bold", _("Toggle _Bold Property"), KB_CONTROL+"B", _("Toggle Bold Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_bold */});
    _actions.push_back(CtAction{"fmt_italic", "format-text-italic", _("Toggle _Italic Property"), KB_CONTROL+"I", _("Toggle Italic Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_italic */});
    _actions.push_back(CtAction{"fmt_underline", "format-text-underline", _("Toggle _Underline Property"), KB_CONTROL+"U", _("Toggle Underline Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_underline */});
    _actions.push_back(CtAction{"fmt_strikethrough", "format-text-strikethrough", _("Toggle Stri_kethrough Property"), KB_CONTROL+"E", _("Toggle Strikethrough Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_strikethrough */});
    _actions.push_back(CtAction{"fmt_h1", "format-text-large", _("Toggle h_1 Property"), KB_CONTROL+"1", _("Toggle h1 Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_h1 */});
    _actions.push_back(CtAction{"fmt_h2", "format-text-large2", _("Toggle h_2 Property"), KB_CONTROL+"2", _("Toggle h2 Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_h2 */});
    _actions.push_back(CtAction{"fmt_h3", "format-text-large3", _("Toggle h_3 Property"), KB_CONTROL+"3", _("Toggle h3 Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_h3 */});
    _actions.push_back(CtAction{"fmt_small", "format-text-small", _("Toggle _Small Property"), KB_CONTROL+"0", _("Toggle Small Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_small */});
    _actions.push_back(CtAction{"fmt_superscript", "format-text-superscript", _("Toggle Su_perscript Property"), None, _("Toggle Superscript Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_superscript */});
    _actions.push_back(CtAction{"fmt_subscript", "format-text-subscript", _("Toggle Su_bscript Property"), None, _("Toggle Subscript Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_subscript */});
    _actions.push_back(CtAction{"fmt_monospace", "format-text-monospace", _("Toggle _Monospace Property"), KB_CONTROL+"M", _("Toggle Monospace Property of the Selected Text"), sigc::signal<void>() /* dad.apply_tag_monospace */});
    _actions.push_back(CtAction{"handle_bull_list", "list_bulleted", _("Set/Unset _Bulleted List"), KB_CONTROL+KB_ALT+"1", _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), sigc::signal<void>() /* dad.list_bulleted_handler */});
    _actions.push_back(CtAction{"handle_num_list", "list_numbered", _("Set/Unset _Numbered List"), KB_CONTROL+KB_ALT+"2", _("Set/Unset the Current Paragraph/Selection as a Numbered List"), sigc::signal<void>() /* dad.list_numbered_handler */});
    _actions.push_back(CtAction{"handle_todo_list", "list_todo", _("Set/Unset _To-Do List"), KB_CONTROL+KB_ALT+"3", _("Set/Unset the Current Paragraph/Selection as a To-Do List"), sigc::signal<void>() /* dad.list_todo_handler */});
    _actions.push_back(CtAction{"fmt_justify_left", "gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), sigc::signal<void>() /* dad.apply_tag_justify_left */});
    _actions.push_back(CtAction{"fmt_justify_center", "gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), sigc::signal<void>() /* dad.apply_tag_justify_center */});
    _actions.push_back(CtAction{"fmt_justify_right", "gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), sigc::signal<void>() /* dad.apply_tag_justify_right */});
    _actions.push_back(CtAction{"fmt_justify_fill", "gtk-justify-fill", _("Justify _Fill"), None, _("Justify Fill the Current Paragraph"), sigc::signal<void>() /* dad.apply_tag_justify_fill */});
    _actions.push_back(CtAction{"tree_add_node", "tree-node-add", _("Add _Node"), KB_CONTROL+"N", _("Add a Node having the same Parent of the Selected Node"), sigc::signal<void>() /* dad.node_add */});
    _actions.push_back(CtAction{"tree_add_subnode", "tree-subnode-add", _("Add _SubNode"), KB_CONTROL+KB_SHIFT+"N", _("Add a Child Node to the Selected Node"), sigc::signal<void>() /* dad.node_child_add */});
    _actions.push_back(CtAction{"tree_dup_node", "tree-node-dupl", _("_Duplicate Node"), KB_CONTROL+KB_SHIFT+"D", _("Duplicate the Selected Node"), sigc::signal<void>() /* dad.node_duplicate */});
    _actions.push_back(CtAction{"tree_node_prop", "cherry_edit", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), sigc::signal<void>() /* dad.node_edit */});
    _actions.push_back(CtAction{"tree_node_toggle_ro", "locked", _("Toggle _Read Only"), KB_CONTROL+KB_ALT+"R", _("Toggle the Read Only Property of the Selected Node"), sigc::signal<void>() /* dad.node_toggle_read_only */});
    _actions.push_back(CtAction{"tree_node_date", "calendar", _("Insert Today's Node"), "F8", _("Insert a Node with Hierarchy Year/Month/Day"), sigc::signal<void>() /* dad.node_date */});
    _actions.push_back(CtAction{"tree_parse_info", "gtk-info", _("Tree _Info"), None, _("Tree Summary Information"), sigc::signal<void>() /* dad.tree_info */});
    _actions.push_back(CtAction{"tree_node_up", "gtk-go-up", _("Node _Up"), KB_SHIFT+CtConst::STR_KEY_UP, _("Move the Selected Node Up"), sigc::signal<void>() /* dad.node_up */});
    _actions.push_back(CtAction{"tree_node_down", "gtk-go-down", _("Node _Down"), KB_SHIFT+CtConst::STR_KEY_DOWN, _("Move the Selected Node Down"), sigc::signal<void>() /* dad.node_down */});
    _actions.push_back(CtAction{"tree_node_left", "gtk-go-back", _("Node _Left"), KB_SHIFT+CtConst::STR_KEY_LEFT, _("Move the Selected Node Left"), sigc::signal<void>() /* dad.node_left */});
    _actions.push_back(CtAction{"tree_node_right", "gtk-go-forward", _("Node _Right"), KB_SHIFT+CtConst::STR_KEY_RIGHT, _("Move the Selected Node Right"), sigc::signal<void>() /* dad.node_right */});
    _actions.push_back(CtAction{"tree_node_new_father", "gtk-jump-to", _("Node Change _Parent"), KB_CONTROL+KB_SHIFT+CtConst::STR_KEY_RIGHT, _("Change the Selected Node's Parent"), sigc::signal<void>() /* dad.node_change_father */});
    _actions.push_back(CtAction{"tree_all_sort_asc", "gtk-sort-ascending", _("Sort Tree _Ascending"), None, _("Sort the Tree Ascending"), sigc::signal<void>() /* dad.tree_sort_ascending */});
    _actions.push_back(CtAction{"tree_all_sort_desc", "gtk-sort-descending", _("Sort Tree _Descending"), None, _("Sort the Tree Descending"), sigc::signal<void>() /* dad.tree_sort_descending */});
    _actions.push_back(CtAction{"tree_sibl_sort_asc", "gtk-sort-ascending", _("Sort Siblings A_scending"), None, _("Sort all the Siblings of the Selected Node Ascending"), sigc::signal<void>() /* dad.node_siblings_sort_ascending */});
    _actions.push_back(CtAction{"tree_sibl_sort_desc", "gtk-sort-descending", _("Sort Siblings D_escending"), None, _("Sort all the Siblings of the Selected Node Descending"), sigc::signal<void>() /* dad.node_siblings_sort_descending */});
    _actions.push_back(CtAction{"child_nodes_inherit_syntax", "gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting"), sigc::signal<void>() /* dad.node_inherit_syntax */});
    _actions.push_back(CtAction{"tree_node_del", "edit-delete", _("De_lete Node"), "Delete", _("Delete the Selected Node"), sigc::signal<void>() /* dad.node_delete */});
    _actions.push_back(CtAction{"go_node_prev", "gtk-go-back", _("Go _Back"), KB_ALT+CtConst::STR_KEY_LEFT, _("Go to the Previous Visited Node"), sigc::signal<void>() /* dad.go_back */});
    _actions.push_back(CtAction{"go_node_next", "gtk-go-forward", _("Go _Forward"), KB_ALT+CtConst::STR_KEY_RIGHT, _("Go to the Next Visited Node"), sigc::signal<void>() /* dad.go_forward */});
    _actions.push_back(CtAction{"find_in_node", "find_sel", _("_Find in Node Content"), KB_CONTROL+"F", _("Find into the Selected Node Content"), sigc::signal<void>() /* dad.find_in_selected_node */});
    _actions.push_back(CtAction{"find_in_allnodes", "find_all", _("Find in _All Nodes Contents"), KB_CONTROL+KB_SHIFT+"F", _("Find into All the Tree Nodes Contents"), sigc::signal<void>() /* dad.find_in_all_nodes */});
    _actions.push_back(CtAction{"find_in_node_n_sub", "find_selnsub", _("Find in _Selected Node and Subnodes Contents"), KB_CONTROL+KB_ALT+"F", _("Find into the Selected Node and Subnodes Contents"), sigc::signal<void>() /* dad.find_in_sel_node_and_subnodes */});
    _actions.push_back(CtAction{"find_in_node_names", "find", _("Find in _Nodes Names and Tags"), KB_CONTROL+"T", _("Find in Nodes Names and Tags"), sigc::signal<void>() /* dad.find_a_node */});
    _actions.push_back(CtAction{"find_iter_fw", "find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), sigc::signal<void>() /* dad.find_again */});
    _actions.push_back(CtAction{"find_iter_bw", "find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), sigc::signal<void>() /* dad.find_back */});
    _actions.push_back(CtAction{"replace_in_node", "replace_sel", _("_Replace in Node Content"), KB_CONTROL+"H", _("Replace into the Selected Node Content"), sigc::signal<void>() /* dad.replace_in_selected_node */});
    _actions.push_back(CtAction{"replace_in_allnodes", "replace_all", _("Replace in _All Nodes Contents"), KB_CONTROL+KB_SHIFT+"H", _("Replace into All the Tree Nodes Contents"), sigc::signal<void>() /* dad.replace_in_all_nodes */});
    _actions.push_back(CtAction{"replace_in_node_n_sub", "replace_selnsub", _("Replace in _Selected Node and Subnodes Contents"), KB_CONTROL+KB_ALT+"H", _("Replace into the Selected Node and Subnodes Contents"), sigc::signal<void>() /* dad.replace_in_sel_node_and_subnodes */});
    _actions.push_back(CtAction{"replace_in_node_names", "find_replace", _("Replace in Nodes _Names"), KB_CONTROL+KB_SHIFT+"T", _("Replace in Nodes Names"), sigc::signal<void>() /* dad.replace_in_nodes_names */});
    _actions.push_back(CtAction{"replace_iter_fw", "replace_again", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), sigc::signal<void>() /* dad.replace_again */});
    _actions.push_back(CtAction{"toggle_show_tree", "cherries", _("Show/Hide _Tree"), "F9", _("Toggle Show/Hide Tree"), sigc::signal<void>() /* dad.toggle_show_hide_tree */});
    _actions.push_back(CtAction{"toggle_show_toolbar", "toolbar", _("Show/Hide Tool_bar"), None, _("Toggle Show/Hide Toolbar"), sigc::signal<void>() /* dad.toggle_show_hide_toolbar */});
    _actions.push_back(CtAction{"toggle_show_node_name_head", "node_name_header", _("Show/Hide Node Name _Header"), None, _("Toggle Show/Hide Node Name Header"), sigc::signal<void>() /* dad.toggle_show_hide_node_name_header */});
    _actions.push_back(CtAction{"toggle_show_allmatches_dlg", "find", _("Show _All Matches Dialog"), KB_CONTROL+KB_SHIFT+"A", _("Show Search All Matches Dialog"), sigc::signal<void>() /* dad.find_allmatchesdialog_restore */});
    _actions.push_back(CtAction{"toggle_focus_tree_text", "gtk-jump-to", _("Toggle _Focus Tree/Text"), KB_CONTROL+"Tab", _("Toggle Focus Between Tree and Text"), sigc::signal<void>() /* dad.toggle_tree_text */});
    _actions.push_back(CtAction{"nodes_all_expand", "gtk-zoom-in", _("E_xpand All Nodes"), KB_CONTROL+KB_SHIFT+"E", _("Expand All the Tree Nodes"), sigc::signal<void>() /* dad.nodes_expand_all */});
    _actions.push_back(CtAction{"nodes_all_collapse", "gtk-zoom-out", _("_Collapse All Nodes"), KB_CONTROL+KB_SHIFT+"L", _("Collapse All the Tree Nodes"), sigc::signal<void>() /* dad.nodes_collapse_all */});
    _actions.push_back(CtAction{"toolbar_icons_size_p", "gtk-add", _("_Increase Toolbar Icons Size"), None, _("Increase the Size of the Toolbar Icons"), sigc::signal<void>() /* dad.toolbar_icons_size_increase */});
    _actions.push_back(CtAction{"toolbar_icons_size_m", "gtk-remove", _("_Decrease Toolbar Icons Size"), None, _("Decrease the Size of the Toolbar Icons"), sigc::signal<void>() /* dad.toolbar_icons_size_decrease */});
    _actions.push_back(CtAction{"toggle_fullscreen", "gtk-fullscreen", _("_Full Screen On/Off"), "F11", _("Toggle Full Screen On/Off"), sigc::signal<void>() /* dad.fullscreen_toggle */});
    _actions.push_back(CtAction{"node_bookmark", "pin-add", _("Add to _Bookmarks"), KB_CONTROL+KB_SHIFT+"B", _("Add the Current Node to the Bookmarks List"), sigc::signal<void>() /* dad.bookmark_curr_node */});
    _actions.push_back(CtAction{"node_unbookmark", "pin-remove", _("_Remove from Bookmarks"), KB_CONTROL+KB_ALT+"B", _("Remove the Current Node from the Bookmarks List"), sigc::signal<void>() /* dad.bookmark_curr_node_remove */});
    _actions.push_back(CtAction{"handle_bookmarks", "gtk-edit", _("_Handle Bookmarks"), None, _("Handle the Bookmarks List"), sigc::signal<void>() /* dad.bookmarks_handle */});
    _actions.push_back(CtAction{"import_cherrytree", "from_cherrytree", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_cherrytree_file */});
    _actions.push_back(CtAction{"import_txt_file", "from_txt", _("From _Plain Text File"), None, _("Add Node from a Plain Text File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_plain_text_file */});
    _actions.push_back(CtAction{"import_txt_folder", "from_txt", _("From _Folder of Plain Text Files"), None, _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_plain_text_folder */});
    _actions.push_back(CtAction{"import_html_file", "from_html", _("From _HTML File"), None, _("Add Node from an HTML File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_html_file */});
    _actions.push_back(CtAction{"import_html_folder", "from_html", _("From _Folder of HTML Files"), None, _("Add Nodes from a Folder of HTML Files to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_html_folder */});
    _actions.push_back(CtAction{"import_basket", CtConst::STR_STOCK_CT_IMP, _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_basket_folder */});
    _actions.push_back(CtAction{"import_epim_html", CtConst::STR_STOCK_CT_IMP, _("From _EssentialPIM HTML File"), None, _("Add Node from an EssentialPIM HTML File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_epim_html_file */});
    _actions.push_back(CtAction{"import_gnote", CtConst::STR_STOCK_CT_IMP, _("From _Gnote Folder"), None, _("Add Nodes of a Gnote Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_gnote_folder */});
    _actions.push_back(CtAction{"import_keepnote", CtConst::STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_keepnote_folder */});
    _actions.push_back(CtAction{"import_keynote", CtConst::STR_STOCK_CT_IMP, _("From K_eyNote File"), None, _("Add Nodes of a KeyNote File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_keynote_file */});
    _actions.push_back(CtAction{"import_knowit", CtConst::STR_STOCK_CT_IMP, _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_knowit_file */});
    _actions.push_back(CtAction{"import_leo", CtConst::STR_STOCK_CT_IMP, _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_leo_file */});
    _actions.push_back(CtAction{"import_mempad", CtConst::STR_STOCK_CT_IMP, _("From _Mempad File"), None, _("Add Nodes of a Mempad File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_mempad_file */});
    _actions.push_back(CtAction{"import_notecase", CtConst::STR_STOCK_CT_IMP, _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_notecase_file */});
    _actions.push_back(CtAction{"import_rednotebook", CtConst::STR_STOCK_CT_IMP, _("From _RedNotebook Folder"), None, _("Add Nodes of a RedNotebook Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_rednotebook_folder */});
    _actions.push_back(CtAction{"import_tomboy", CtConst::STR_STOCK_CT_IMP, _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_tomboy_folder */});
    _actions.push_back(CtAction{"import_treepad", CtConst::STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_treepad_file */});
    _actions.push_back(CtAction{"import_tuxcards", CtConst::STR_STOCK_CT_IMP, _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_tuxcards_file */});
    _actions.push_back(CtAction{"import_zim", CtConst::STR_STOCK_CT_IMP, _("From _Zim Folder"), None, _("Add Nodes of a Zim Folder to the Current Tree"), sigc::signal<void>() /* dad.nodes_add_from_zim_folder */});
    _actions.push_back(CtAction{"export_pdf", "to_pdf", _("Export To _PDF"), None, _("Export To PDF"), sigc::signal<void>() /* dad.export_to_pdf */});
    _actions.push_back(CtAction{"export_html", "to_html", _("Export To _HTML"), None, _("Export To HTML"), sigc::signal<void>() /* dad.export_to_html */});
    _actions.push_back(CtAction{"export_txt_multiple", "to_txt", _("Export to Multiple Plain _Text Files"), None, _("Export to Multiple Plain Text Files"), sigc::signal<void>() /* dad.export_to_txt_multiple */});
    _actions.push_back(CtAction{"export_txt_single", "to_txt", _("Export to _Single Plain Text File"), None, _("Export to Single Plain Text File"), sigc::signal<void>() /* dad.export_to_txt_single */});
    _actions.push_back(CtAction{"export_ctd", "to_cherrytree", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), sigc::signal<void>() /* dad.export_to_ctd */});
    _actions.push_back(CtAction{"ct_check_newer", "gtk-network", _("_Check Newer Version"), None, _("Check for a Newer Version"), sigc::signal<void>() /* dad.check_for_newer_version */});
    _actions.push_back(CtAction{"ct_help", "help-contents", _("Online _Manual"), "F1", _("Application's Online Manual"), sigc::signal<void>() /* dad.on_help_menu_item_activated */});
    _actions.push_back(CtAction{"ct_about", "gtk-about", _("_About"), None, _("About CherryTree"), sigc::signal<void>() /* dad.dialog_about */});
    _actions.push_back(CtAction{"anch_cut", "edit-cut", _("C_ut Anchor"), None, _("Cut the Selected Anchor"), sigc::signal<void>() /* dad.anchor_cut */});
    _actions.push_back(CtAction{"anch_copy", "edit-copy", _("_Copy Anchor"), None, _("Copy the Selected Anchor"), sigc::signal<void>() /* dad.anchor_copy */});
    _actions.push_back(CtAction{"anch_del", "edit-delete", _("_Delete Anchor"), None, _("Delete the Selected Anchor"), sigc::signal<void>() /* dad.anchor_delete */});
    _actions.push_back(CtAction{"anch_edit", "anchor_edit", _("Edit _Anchor"), None, _("Edit the Underlying Anchor"), sigc::signal<void>() /* dad.anchor_edit */});
    _actions.push_back(CtAction{"emb_file_cut", "edit-cut", _("C_ut Embedded File"), None, _("Cut the Selected Embedded File"), sigc::signal<void>() /* dad.embfile_cut */});
    _actions.push_back(CtAction{"emb_file_copy", "edit-copy", _("_Copy Embedded File"), None, _("Copy the Selected Embedded File"), sigc::signal<void>() /* dad.embfile_copy */});
    _actions.push_back(CtAction{"emb_file_del", "edit-delete", _("_Delete Embedded File"), None, _("Delete the Selected Embedded File"), sigc::signal<void>() /* dad.embfile_delete */});
    _actions.push_back(CtAction{"emb_file_save", "gtk-save-as", _("Save _As"), None, _("Save File As"), sigc::signal<void>() /* dad.embfile_save */});
    _actions.push_back(CtAction{"emb_file_open", "gtk-open", _("_Open File"), None, _("Open Embedded File"), sigc::signal<void>() /* dad.embfile_open */});
    _actions.push_back(CtAction{"img_save", "image_save", _("_Save Image as PNG"), None, _("Save the Selected Image as a PNG file"), sigc::signal<void>() /* dad.image_save */});
    _actions.push_back(CtAction{"img_edit", "image_edit", _("_Edit Image"), None, _("Edit the Selected Image"), sigc::signal<void>() /* dad.image_edit */});
    _actions.push_back(CtAction{"img_cut", "edit-cut", _("C_ut Image"), None, _("Cut the Selected Image"), sigc::signal<void>() /* dad.image_cut */});
    _actions.push_back(CtAction{"img_copy", "edit-copy", _("_Copy Image"), None, _("Copy the Selected Image"), sigc::signal<void>() /* dad.image_copy */});
    _actions.push_back(CtAction{"img_del", "edit-delete", _("_Delete Image"), None, _("Delete the Selected Image"), sigc::signal<void>() /* dad.image_delete */});
    _actions.push_back(CtAction{"img_link_edit", "link_handle", _("Edit _Link"), None, _("Edit the Link Associated to the Image"), sigc::signal<void>() /* dad.image_link_edit */});
    _actions.push_back(CtAction{"img_link_dismiss", "gtk-clear", _("D_ismiss Link"), None, _("Dismiss the Link Associated to the Image"), sigc::signal<void>() /* dad.image_link_dismiss */});
    _actions.push_back(CtAction{"toggle_show_mainwin", CtConst::APP_NAME, _("Show/Hide _CherryTree"), None, _("Toggle Show/Hide CherryTree"), sigc::signal<void>() /* dad.toggle_show_hide_main_window */});
    _actions.push_back(CtAction{"strip_trail_spaces", "gtk-clear", _("Stri_p Trailing Spaces"), None, _("Strip Trailing Spaces"), sigc::signal<void>() /* dad.strip_trailing_spaces */});

    _actions.push_back(CtAction{});
}

CtAction const * CtMenu::find_action(std::string id)
{
    for (CtAction& action: _actions)
        if (action.id == id)
            return &action;
    return nullptr;
}

GtkAccelGroup* CtMenu::default_accel_group()
{
    static GtkAccelGroup *accel_group = nullptr;
    if (!accel_group)
        accel_group = gtk_accel_group_new ();
    return accel_group;
}

GtkWidget* CtMenu::build_menubar()
{
    xmlpp::DomParser parser;
    parser.parse_memory(get_menu_markup());

    GtkWidget *mbar = gtk_menu_bar_new();
    build_menus(parser.get_document()->get_root_node(), mbar);
    return mbar;
}

void CtMenu::build_menus(xmlpp::Node* node, GtkWidget *menu)
{
    for (xmlpp::Node* nodeIter = node; nodeIter; nodeIter = nodeIter->get_next_sibling()) {
        if (nodeIter->get_name() == "menubar") {
            build_menus(nodeIter->get_first_child(), menu);
        }
        if (nodeIter->get_name() == "menu") {
            CtAction const *action = find_action(get_attribute(nodeIter, "name")->get_value());
            GtkWidget *mitem = gtk_menu_item_new_with_mnemonic(action->name.c_str());
            GtkWidget *submenu = gtk_menu_new();
            build_menus(nodeIter->get_first_child(), submenu);
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), GTK_WIDGET(submenu));
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
            continue;
        }
        if (nodeIter->get_name() == "menuitem") {
            CtAction const *action = find_action(get_attribute(nodeIter, "action")->get_value());
            build_menu_item(GTK_MENU(menu), action);
            continue;
        }
        if (nodeIter->get_name() == "separator") {
            build_menu_item(GTK_MENU(menu), nullptr);
            continue;
        }
    }
}

// based on inkscape/src/ui/interface.cpp
GtkWidget* CtMenu::build_menu_item(GtkMenu *menu, CtAction const *action)
{
    Gtk::Widget *item;
    if (action == nullptr) {
        item = new Gtk::SeparatorMenuItem();
    } else {
        item = new Gtk::MenuItem();

        // Now create the label and add it to the menu item
        GtkWidget *label = gtk_accel_label_new(action->name.c_str());
        gtk_label_set_markup_with_mnemonic(GTK_LABEL(label), action->name.c_str());

   #if GTK_CHECK_VERSION(3,16,0)
        gtk_label_set_xalign(GTK_LABEL(label), 0.0);
   #else
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
   #endif

        if (!action->shortcut.empty()) {
            guint key; GdkModifierType mod;
            gtk_accelerator_parse(action->shortcut.c_str(), &key, &mod);
            gtk_widget_add_accelerator (item->gobj(),
                            "activate",
                            default_accel_group(),
                            key,
                            mod,
                            GTK_ACCEL_VISIBLE);
        }
        gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(label), item->gobj());

        // If there is an image associated with the action, then we can add it as an icon for the menu item.
        if (!action->stock.empty()) {
            item->set_name("ImageMenuItem");  // custom name to identify our "ImageMenuItems"

            GtkWidget *icon = gtk_image_new_from_icon_name(action->stock.c_str(), GTK_ICON_SIZE_MENU);
            //GtkWidget *icon = gtk_image_new();
            //Glib::RefPtr<Gdk::Pixbuf> pixbuf = CtApp::R_icontheme->load_icon(action->stock, GTK_ICON_SIZE_MENU);
            //gtk_image_set_from_pixbuf(GTK_IMAGE(icon), pixbuf->gobj());


            // create a box to hold icon and label as GtkMenuItem derives from GtkBin and can only hold one child
            GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 5);
            gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

            gtk_container_add(GTK_CONTAINER(item->gobj()), box);
        } else {
            gtk_container_add(GTK_CONTAINER(item->gobj()), label);
        }


        gtk_widget_set_events(item->gobj(), GDK_KEY_PRESS_MASK);
        g_signal_connect(G_OBJECT(item->gobj()), "activate", G_CALLBACK(on_menu_activate), (gpointer)action );
    }

    item->show_all();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item->gobj());

    return item->gobj();
}

const char* CtMenu::get_menu_markup()
{
    return R"MARKUP(
             <menubar name='MenuBar'>
               <menu name='FileMenu'>
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

               <menu name='EditMenu'>
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
                 <menu name='ChangeCaseMenu'>
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

               <menu name='FormattingMenu'>
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

               <menu name='TreeMenu'>
               </menu>

               <menu name='SearchMenu'>
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

               <menu name='ViewMenu'>
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

               <menu name='BookmarksMenu'>
                 <menuitem action='handle_bookmarks'/>
               </menu>

               <menu name='ImportMenu'>
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

               <menu name='ExportMenu'>
                 <menuitem action='export_pdf'/>
                 <menuitem action='export_html'/>
                 <menuitem action='export_txt_multiple'/>
                 <menuitem action='export_txt_single'/>
                 <menuitem action='export_ctd'/>
               </menu>

               <menu name='HelpMenu'>
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
