# -*- coding: UTF-8 -*-
#
#       menus.py
#
#       Copyright 2009-2019 Giuseppe Penone <giuspen@gmail.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.

import cons

KB_CONTROL = "<control>"
KB_SHIFT = "<shift>"
KB_ALT = "<alt>"

CONFIG_ACTIONS_DICT = {
"file": [
"ct_new_inst",
"ct_open_file",
"ct_save",
"ct_vacuum",
"ct_save_as",
"print_page_setup",
"do_print",
"command_palette",
"exec_code",
"quit_app",
"exit_app",
"preferences_dlg",
"open_cfg_folder",
"ct_check_newer",
"ct_help",
"ct_about",
],
"tree": [
"tree_add_node",
"tree_add_subnode",
"tree_dup_node",
"tree_node_date",
"tree_all_sort_asc",
"tree_all_sort_desc",
"tree_sibl_sort_asc",
"tree_sibl_sort_desc",
"tree_node_prop",
"tree_node_toggle_ro",
"child_nodes_inherit_syntax",
"tree_parse_info",
"node_bookmark",
"node_unbookmark",
"handle_bookmarks",
],
"editor": [
"act_undo",
"act_redo",
"handle_image",
#"handle_screenshot",
"handle_table",
"handle_codebox",
"handle_embfile",
"handle_link",
"handle_anchor",
"insert_toc",
"insert_timestamp",
"insert_horiz_rule",
"case_down",
"case_up",
"case_tggl",
"strip_trail_spaces",
"spellcheck_toggle",
"cut_plain",
"copy_plain",
"paste_plain",
"cut_row",
"copy_row",
"del_row",
"dup_row",
"mv_up_row",
"mv_down_row",
],
"fmt": [
"fmt_color_fg",
"fmt_color_bg",
"fmt_bold",
"fmt_italic",
"fmt_underline",
"fmt_strikethrough",
"fmt_h1",
"fmt_h2",
"fmt_h3",
"fmt_small",
"fmt_superscript",
"fmt_subscript",
"fmt_monospace",
"fmt_justify_left",
"fmt_justify_center",
"fmt_justify_right",
"fmt_justify_fill",
"handle_bull_list",
"handle_num_list",
"handle_todo_list",
"fmt_latest",
"fmt_rm",
],
"findrepl": [
"find_in_node",
"find_in_allnodes",
"find_in_node_n_sub",
"find_in_node_names",
"find_iter_fw",
"find_iter_bw",
"replace_in_node",
"replace_in_allnodes",
"replace_in_node_n_sub",
"replace_in_node_names",
"replace_iter_fw",
"toggle_show_allmatches_dlg",
],
"view": [
"toggle_show_tree",
"toggle_show_toolbar",
"toggle_show_node_name_head",
"toggle_focus_tree_text",
"nodes_all_expand",
"nodes_all_collapse",
"toolbar_icons_size_p",
"toolbar_icons_size_m",
"toggle_fullscreen",
],
"export": [
"export_pdf",
"export_html",
"export_txt_multiple",
"export_txt_single",
"export_ctd",
],
"import": [
"import_cherrytree",
"import_txt_file",
"import_txt_folder",
"import_html_file",
"import_html_folder",
"import_basket",
"import_epim_html",
"import_gnote",
"import_keepnote",
"import_keynote",
"import_knowit",
"import_leo",
"import_mempad",
"import_notecase",
"import_rednotebook",
"import_tomboy",
"import_treepad",
"import_tuxcards",
"import_zim",
],
}

def load_menudict(dad):
    """Loads the Menus Dictionary"""
    dad.menudict = {
# sk = stock; sd = short description; kb = keyboard shortcut, dn = description, cb = callback
"ct_new_inst": {"sk": "new-instance", "sd": _("New _Instance"), "kb": None, "dn": _("Start a New Instance of CherryTree"), "cb": dad.file_new},
"ct_open_file": {"sk": "gtk-open", "sd": _("_Open File"), "kb": KB_CONTROL+"o", "dn": _("Open a CherryTree Document"), "cb": dad.file_open},
"ct_save": {"sk": "gtk-save", "sd": _("_Save"), "kb": KB_CONTROL+"s", "dn": _("Save File"), "cb": dad.file_save},
"ct_vacuum": {"sk": "gtk-clear", "sd": _("Save and _Vacuum"), "kb": None, "dn": _("Save File and Vacuum"), "cb": dad.file_vacuum},
"ct_save_as": {"sk": "gtk-save-as", "sd": _("Save _As"), "kb": KB_CONTROL+KB_SHIFT+"s", "dn": _("Save File As"), "cb": dad.file_save_as},
"exec_code": {"sk": "gtk-execute", "sd": _("_Execute Code"), "kb": "F5", "dn": _("Execute Code"), "cb": dad.exec_code},
"open_cfg_folder": {"sk": "gtk-directory", "sd": _("Open Preferences _Directory"), "kb": None, "dn": _("Open the Directory with Preferences Files"), "cb": dad.folder_cfg_open},
"print_page_setup": {"sk": "gtk-print", "sd": _("Pa_ge Setup"), "kb": None, "dn": _("Set up the Page for Printing"), "cb": dad.export_print_page_setup},
"do_print": {"sk": "gtk-print", "sd": _("_Print"), "kb": KB_CONTROL+"p", "dn": _("Print"), "cb": dad.export_print},
"quit_app": {"sk": "quit-app", "sd": _("_Quit"), "kb": KB_CONTROL+"q", "dn": _("Quit the Application"), "cb": dad.quit_application},
"exit_app": {"sk": "quit-app", "sd": _("_Exit CherryTree"), "kb": KB_CONTROL+KB_SHIFT+"q", "dn": _("Exit from CherryTree"), "cb": dad.quit_application_totally},
"preferences_dlg": {"sk": "gtk-preferences", "sd": _("_Preferences"), "kb": KB_CONTROL+KB_ALT+"p", "dn": _("Preferences"), "cb": dad.dialog_preferences},
"act_undo": {"sk": "gtk-undo", "sd": _("_Undo"), "kb": KB_CONTROL+"z", "dn": _("Undo Last Operation"), "cb": dad.requested_step_back},
"act_redo": {"sk": "gtk-redo", "sd": _("_Redo"), "kb": KB_CONTROL+"y", "dn": _("Redo Previously Discarded Operation"), "cb": dad.requested_step_ahead},
"handle_image": {"sk": "image_insert", "sd": _("Insert I_mage"), "kb": KB_CONTROL+KB_ALT+"i", "dn": _("Insert an Image"), "cb": dad.image_handle},
#"handle_screenshot": {"sk": "screenshot_insert", "sd": _("Insert _Screenshot"), "kb": KB_CONTROL+KB_SHIFT+KB_ALT+"s", "dn": _("Insert a Screenshot"), "cb": dad.screenshot_handle},
"handle_table": {"sk": "table_insert", "sd": _("Insert _Table"), "kb": KB_CONTROL+KB_ALT+"t", "dn": _("Insert a Table"), "cb": dad.table_handle},
"handle_codebox": {"sk": "codebox_insert", "sd": _("Insert _CodeBox"), "kb": KB_CONTROL+KB_ALT+"c", "dn": _("Insert a CodeBox"), "cb": dad.codebox_handle},
"handle_embfile": {"sk": "file_icon", "sd": _("Insert _File"), "kb": KB_CONTROL+KB_ALT+"e", "dn": _("Insert File"), "cb": dad.embfile_insert},
"handle_link": {"sk": "link_handle", "sd": _("Insert/Edit _Link"), "kb": KB_CONTROL+"l", "dn": _("Insert a Link/Edit the Underlying Link"), "cb": dad.apply_tag_link},
"handle_anchor": {"sk": "anchor_insert", "sd": _("Insert _Anchor"), "kb": KB_CONTROL+KB_ALT+"a", "dn": _("Insert an Anchor"), "cb": dad.anchor_handle},
"insert_toc": {"sk": "index", "sd": _("Insert T_OC"), "kb": None, "dn": _("Insert Table of Contents"), "cb": dad.toc_insert},
"insert_timestamp": {"sk": "timestamp", "sd": _("Insert Ti_mestamp"), "kb": KB_CONTROL+KB_ALT+"m", "dn": _("Insert Timestamp"), "cb": dad.timestamp_insert},
"insert_horiz_rule": {"sk": "horizontal_rule", "sd": _("Insert _Horizontal Rule"), "kb": KB_CONTROL+"r", "dn": _("Insert Horizontal Rule"), "cb": dad.horizontal_rule_insert},
"case_down": {"sk": "case_lower", "sd": _("_Lower Case of Selection/Word"), "kb": KB_CONTROL+"w", "dn": _("Lower the Case of the Selection/the Underlying Word"), "cb": dad.text_selection_lower_case},
"case_up": {"sk": "case_upper", "sd": _("_Upper Case of Selection/Word"), "kb": KB_CONTROL+KB_SHIFT+"w", "dn": _("Upper the Case of the Selection/the Underlying Word"), "cb": dad.text_selection_upper_case},
"case_tggl": {"sk": "case_toggle", "sd": _("_Toggle Case of Selection/Word"), "kb": KB_CONTROL+"g", "dn": _("Toggle the Case of the Selection/the Underlying Word"), "cb": dad.text_selection_toggle_case},
"spellcheck_toggle": {"sk": "gtk-spell-check", "sd": _("Enable/Disable _Spell Check"), "kb": KB_CONTROL+KB_ALT+"s", "dn": _("Toggle Enable/Disable Spell Check"), "cb": dad.toggle_ena_dis_spellcheck},
"cut_plain": {"sk": "edit-cut", "sd": _("Cu_t as Plain Text"), "kb": KB_CONTROL+KB_SHIFT+"x", "dn": _("Cut as Plain Text, Discard the Rich Text Formatting"), "cb": dad.cut_as_plain_text},
"copy_plain": {"sk": "edit-copy", "sd": _("_Copy as Plain Text"), "kb": KB_CONTROL+KB_SHIFT+"c", "dn": _("Copy as Plain Text, Discard the Rich Text Formatting"), "cb": dad.copy_as_plain_text},
"paste_plain": {"sk": "edit-paste", "sd": _("_Paste as Plain Text"), "kb": KB_CONTROL+KB_SHIFT+"v", "dn": _("Paste as Plain Text, Discard the Rich Text Formatting"), "cb": dad.paste_as_plain_text},
"cut_row": {"sk": "edit-cut", "sd": _("Cu_t Row"), "kb": KB_SHIFT+KB_ALT+"x", "dn": _("Cut the Current Row/Selected Rows"), "cb": dad.text_row_cut},
"copy_row": {"sk": "edit-copy", "sd": _("_Copy Row"), "kb": KB_SHIFT+KB_ALT+"c", "dn": _("Copy the Current Row/Selected Rows"), "cb": dad.text_row_copy},
"del_row": {"sk": "edit-delete", "sd": _("De_lete Row"), "kb": KB_CONTROL+"k", "dn": _("Delete the Current Row/Selected Rows"), "cb": dad.text_row_delete},
"dup_row": {"sk": "gtk-add", "sd": _("_Duplicate Row"), "kb": KB_CONTROL+"d", "dn": _("Duplicate the Current Row/Selection"), "cb": dad.text_row_selection_duplicate},
"mv_up_row": {"sk": "gtk-go-up", "sd": _("Move _Up Row"), "kb": KB_ALT+cons.STR_KEY_UP, "dn": _("Move Up the Current Row/Selected Rows"), "cb": dad.text_row_up},
"mv_down_row": {"sk": "gtk-go-down", "sd": _("Move _Down Row"), "kb": KB_ALT+cons.STR_KEY_DOWN, "dn": _("Move Down the Current Row/Selected Rows"), "cb": dad.text_row_down},
"fmt_latest": {"sk": "format_text_latest", "sd": _("Format _Latest"), "kb": "F7", "dn": _("Memory of Latest Text Format Type"), "cb": dad.apply_tag_latest},
"fmt_rm": {"sk": "format_text_clear", "sd": _("_Remove Formatting"), "kb": KB_CONTROL+KB_SHIFT+"r", "dn": _("Remove the Formatting from the Selected Text"), "cb": dad.remove_text_formatting},
"fmt_color_fg": {"sk": "color_foreground", "sd": _("Text _Color Foreground"), "kb": KB_SHIFT+KB_ALT+"f", "dn": _("Change the Color of the Selected Text Foreground"), "cb": dad.apply_tag_foreground},
"fmt_color_bg": {"sk": "color_background", "sd": _("Text C_olor Background"), "kb": KB_SHIFT+KB_ALT+"b", "dn": _("Change the Color of the Selected Text Background"), "cb": dad.apply_tag_background},
"fmt_bold": {"sk": "format-text-bold", "sd": _("Toggle _Bold Property"), "kb": KB_CONTROL+"b", "dn": _("Toggle Bold Property of the Selected Text"), "cb": dad.apply_tag_bold},
"fmt_italic": {"sk": "format-text-italic", "sd": _("Toggle _Italic Property"), "kb": KB_CONTROL+"i", "dn": _("Toggle Italic Property of the Selected Text"), "cb": dad.apply_tag_italic},
"fmt_underline": {"sk": "format-text-underline", "sd": _("Toggle _Underline Property"), "kb": KB_CONTROL+"u", "dn": _("Toggle Underline Property of the Selected Text"), "cb": dad.apply_tag_underline},
"fmt_strikethrough": {"sk": "format-text-strikethrough", "sd": _("Toggle Stri_kethrough Property"), "kb": KB_CONTROL+"e", "dn": _("Toggle Strikethrough Property of the Selected Text"), "cb": dad.apply_tag_strikethrough},
"fmt_h1": {"sk": "format-text-large", "sd": _("Toggle h_1 Property"), "kb": KB_CONTROL+"1", "dn": _("Toggle h1 Property of the Selected Text"), "cb": dad.apply_tag_h1},
"fmt_h2": {"sk": "format-text-large2", "sd": _("Toggle h_2 Property"), "kb": KB_CONTROL+"2", "dn": _("Toggle h2 Property of the Selected Text"), "cb": dad.apply_tag_h2},
"fmt_h3": {"sk": "format-text-large3", "sd": _("Toggle h_3 Property"), "kb": KB_CONTROL+"3", "dn": _("Toggle h3 Property of the Selected Text"), "cb": dad.apply_tag_h3},
"fmt_small": {"sk": "format-text-small", "sd": _("Toggle _Small Property"), "kb": KB_CONTROL+"0", "dn": _("Toggle Small Property of the Selected Text"), "cb": dad.apply_tag_small},
"fmt_superscript": {"sk": "format-text-superscript", "sd": _("Toggle Su_perscript Property"), "kb": None, "dn": _("Toggle Superscript Property of the Selected Text"), "cb": dad.apply_tag_superscript},
"fmt_subscript": {"sk": "format-text-subscript", "sd": _("Toggle Su_bscript Property"), "kb": None, "dn": _("Toggle Subscript Property of the Selected Text"), "cb": dad.apply_tag_subscript},
"fmt_monospace": {"sk": "format-text-monospace", "sd": _("Toggle _Monospace Property"), "kb": KB_CONTROL+"m", "dn": _("Toggle Monospace Property of the Selected Text"), "cb": dad.apply_tag_monospace},
"handle_bull_list": {"sk": "list_bulleted", "sd": _("Set/Unset _Bulleted List"), "kb": KB_CONTROL+KB_ALT+"1", "dn": _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), "cb": dad.list_bulleted_handler},
"handle_num_list": {"sk": "list_numbered", "sd": _("Set/Unset _Numbered List"), "kb": KB_CONTROL+KB_ALT+"2", "dn": _("Set/Unset the Current Paragraph/Selection as a Numbered List"), "cb": dad.list_numbered_handler},
"handle_todo_list": {"sk": "list_todo", "sd": _("Set/Unset _To-Do List"), "kb": KB_CONTROL+KB_ALT+"3", "dn": _("Set/Unset the Current Paragraph/Selection as a To-Do List"), "cb": dad.list_todo_handler},
"fmt_justify_left": {"sk": "gtk-justify-left", "sd": _("Justify _Left"), "kb": None, "dn": _("Justify Left the Current Paragraph"), "cb": dad.apply_tag_justify_left},
"fmt_justify_center": {"sk": "gtk-justify-center", "sd": _("Justify _Center"), "kb": None, "dn": _("Justify Center the Current Paragraph"), "cb": dad.apply_tag_justify_center},
"fmt_justify_right": {"sk": "gtk-justify-right", "sd": _("Justify _Right"), "kb": None, "dn": _("Justify Right the Current Paragraph"), "cb": dad.apply_tag_justify_right},
"fmt_justify_fill": {"sk": "gtk-justify-fill", "sd": _("Justify _Fill"), "kb": None, "dn": _("Justify Fill the Current Paragraph"), "cb": dad.apply_tag_justify_fill},
"tree_add_node": {"sk": "tree-node-add", "sd": _("Add _Node"), "kb": KB_CONTROL+"n", "dn": _("Add a Node having the same Parent of the Selected Node"), "cb": dad.node_add},
"tree_add_subnode": {"sk": "tree-subnode-add", "sd": _("Add _SubNode"), "kb": KB_CONTROL+KB_SHIFT+"n", "dn": _("Add a Child Node to the Selected Node"), "cb": dad.node_child_add},
"tree_dup_node": {"sk": "tree-node-dupl", "sd": _("_Duplicate Node"), "kb": KB_CONTROL+KB_SHIFT+"d", "dn": _("Duplicate the Selected Node"), "cb": dad.node_duplicate},
"tree_node_prop": {"sk": "cherry_edit", "sd": _("Change Node _Properties"), "kb": "F2", "dn": _("Edit the Properties of the Selected Node"), "cb": dad.node_edit},
"tree_node_toggle_ro": {"sk": "locked", "sd": _("Toggle _Read Only"), "kb": KB_CONTROL+KB_ALT+"r", "dn": _("Toggle the Read Only Property of the Selected Node"), "cb": dad.node_toggle_read_only},
"tree_node_date": {"sk": "calendar", "sd": _("Insert Today's Node"), "kb": "F8", "dn": _("Insert a Node with Hierarchy Year/Month/Day"), "cb": dad.node_date},
"tree_parse_info": {"sk": "gtk-info", "sd": _("Tree _Info"), "kb": None, "dn": _("Tree Summary Information"), "cb": dad.tree_info},
"tree_node_up": {"sk": "gtk-go-up", "sd": _("Node _Up"), "kb": KB_SHIFT+cons.STR_KEY_UP, "dn": _("Move the Selected Node Up"), "cb": dad.node_up},
"tree_node_down": {"sk": "gtk-go-down", "sd": _("Node _Down"), "kb": KB_SHIFT+cons.STR_KEY_DOWN, "dn": _("Move the Selected Node Down"), "cb": dad.node_down},
"tree_node_left": {"sk": "gtk-go-back", "sd": _("Node _Left"), "kb": KB_SHIFT+cons.STR_KEY_LEFT, "dn": _("Move the Selected Node Left"), "cb": dad.node_left},
"tree_node_right": {"sk": "gtk-go-forward", "sd": _("Node _Right"), "kb": KB_SHIFT+cons.STR_KEY_RIGHT, "dn": _("Move the Selected Node Right"), "cb": dad.node_right},
"tree_node_new_father": {"sk": "gtk-jump-to", "sd": _("Node Change _Parent"), "kb": KB_CONTROL+KB_SHIFT+cons.STR_KEY_RIGHT, "dn": _("Change the Selected Node's Parent"), "cb": dad.node_change_father},
"tree_all_sort_asc": {"sk": "gtk-sort-ascending", "sd": _("Sort Tree _Ascending"), "kb": None, "dn": _("Sort the Tree Ascending"), "cb": dad.tree_sort_ascending},
"tree_all_sort_desc": {"sk": "gtk-sort-descending", "sd": _("Sort Tree _Descending"), "kb": None, "dn": _("Sort the Tree Descending"), "cb": dad.tree_sort_descending},
"tree_sibl_sort_asc": {"sk": "gtk-sort-ascending", "sd": _("Sort Siblings A_scending"), "kb": None, "dn": _("Sort all the Siblings of the Selected Node Ascending"), "cb": dad.node_siblings_sort_ascending},
"tree_sibl_sort_desc": {"sk": "gtk-sort-descending", "sd": _("Sort Siblings D_escending"), "kb": None, "dn": _("Sort all the Siblings of the Selected Node Descending"), "cb": dad.node_siblings_sort_descending},
"child_nodes_inherit_syntax": {"sk": "gtk-execute", "sd": _("_Inherit Syntax"), "kb": None, "dn": _("Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting"), "cb": dad.node_inherit_syntax},
"tree_node_del": {"sk": "edit-delete", "sd": _("De_lete Node"), "kb": "Delete", "dn": _("Delete the Selected Node"), "cb": dad.node_delete},
"go_node_prev": {"sk": "gtk-go-back", "sd": _("Go _Back"), "kb": KB_ALT+cons.STR_KEY_LEFT, "dn": _("Go to the Previous Visited Node"), "cb": dad.go_back},
"go_node_next": {"sk": "gtk-go-forward", "sd": _("Go _Forward"), "kb": KB_ALT+cons.STR_KEY_RIGHT, "dn": _("Go to the Next Visited Node"), "cb": dad.go_forward},
"find_in_node": {"sk": "find_sel", "sd": _("_Find in Node Content"), "kb": KB_CONTROL+"f", "dn": _("Find into the Selected Node Content"), "cb": dad.find_in_selected_node},
"find_in_allnodes": {"sk": "find_all", "sd": _("Find in _All Nodes Contents"), "kb": KB_CONTROL+KB_SHIFT+"f", "dn": _("Find into All the Tree Nodes Contents"), "cb": dad.find_in_all_nodes},
"find_in_node_n_sub": {"sk": "find_selnsub", "sd": _("Find in _Selected Node and Subnodes Contents"), "kb": KB_CONTROL+KB_ALT+"f", "dn": _("Find into the Selected Node and Subnodes Contents"), "cb": dad.find_in_sel_node_and_subnodes},
"find_in_node_names": {"sk": "find", "sd": _("Find in _Nodes Names and Tags"), "kb": KB_CONTROL+"t", "dn": _("Find in Nodes Names and Tags"), "cb": dad.find_a_node},
"find_iter_fw": {"sk": "find_again", "sd": _("Find _Again"), "kb": "F3", "dn": _("Iterate the Last Find Operation"), "cb": dad.find_again},
"find_iter_bw": {"sk": "find_back", "sd": _("Find _Back"), "kb": "F4", "dn": _("Iterate the Last Find Operation in Opposite Direction"), "cb": dad.find_back},
"replace_in_node": {"sk": "replace_sel", "sd": _("_Replace in Node Content"), "kb": KB_CONTROL+"h", "dn": _("Replace into the Selected Node Content"), "cb": dad.replace_in_selected_node},
"replace_in_allnodes": {"sk": "replace_all", "sd": _("Replace in _All Nodes Contents"), "kb": KB_CONTROL+KB_SHIFT+"h", "dn": _("Replace into All the Tree Nodes Contents"), "cb": dad.replace_in_all_nodes},
"replace_in_node_n_sub": {"sk": "replace_selnsub", "sd": _("Replace in _Selected Node and Subnodes Contents"), "kb": KB_CONTROL+KB_ALT+"h", "dn": _("Replace into the Selected Node and Subnodes Contents"), "cb": dad.replace_in_sel_node_and_subnodes},
"replace_in_node_names": {"sk": "find_replace", "sd": _("Replace in Nodes _Names"), "kb": KB_CONTROL+KB_SHIFT+"t", "dn": _("Replace in Nodes Names"), "cb": dad.replace_in_nodes_names},
"replace_iter_fw": {"sk": "replace_again", "sd": _("Replace _Again"), "kb": "F6", "dn": _("Iterate the Last Replace Operation"), "cb": dad.replace_again},
"toggle_show_tree": {"sk": "cherries", "sd": _("Show/Hide _Tree"), "kb": "F9", "dn": _("Toggle Show/Hide Tree"), "cb": dad.toggle_show_hide_tree},
"toggle_show_toolbar": {"sk": "toolbar", "sd": _("Show/Hide Tool_bar"), "kb": None, "dn": _("Toggle Show/Hide Toolbar"), "cb": dad.toggle_show_hide_toolbar},
"toggle_show_node_name_head": {"sk": "node_name_header", "sd": _("Show/Hide Node Name _Header"), "kb": None, "dn": _("Toggle Show/Hide Node Name Header"), "cb": dad.toggle_show_hide_node_name_header},
"toggle_show_allmatches_dlg": {"sk": "find", "sd": _("Show _All Matches Dialog"), "kb": KB_CONTROL+KB_SHIFT+"a", "dn": _("Show Search All Matches Dialog"), "cb": dad.find_allmatchesdialog_restore},
"toggle_focus_tree_text": {"sk": "gtk-jump-to", "sd": _("Toggle _Focus Tree/Text"), "kb": KB_CONTROL+"Tab", "dn": _("Toggle Focus Between Tree and Text"), "cb": dad.toggle_tree_text},
"nodes_all_expand": {"sk": "gtk-zoom-in", "sd": _("E_xpand All Nodes"), "kb": KB_CONTROL+KB_SHIFT+"e", "dn": _("Expand All the Tree Nodes"), "cb": dad.nodes_expand_all},
"nodes_all_collapse": {"sk": "gtk-zoom-out", "sd": _("_Collapse All Nodes"), "kb": KB_CONTROL+KB_SHIFT+"l", "dn": _("Collapse All the Tree Nodes"), "cb": dad.nodes_collapse_all},
"toolbar_icons_size_p": {"sk": "gtk-add", "sd": _("_Increase Toolbar Icons Size"), "kb": None, "dn": _("Increase the Size of the Toolbar Icons"), "cb": dad.toolbar_icons_size_increase},
"toolbar_icons_size_m": {"sk": "gtk-remove", "sd": _("_Decrease Toolbar Icons Size"), "kb": None, "dn": _("Decrease the Size of the Toolbar Icons"), "cb": dad.toolbar_icons_size_decrease},
"toggle_fullscreen": {"sk": "gtk-fullscreen", "sd": _("_Full Screen On/Off"), "kb": "F11", "dn": _("Toggle Full Screen On/Off"), "cb": dad.fullscreen_toggle},
"node_bookmark": {"sk": "pin-add", "sd": _("Add to _Bookmarks"), "kb": KB_CONTROL+KB_SHIFT+"b", "dn": _("Add the Current Node to the Bookmarks List"), "cb": dad.bookmark_curr_node},
"node_unbookmark": {"sk": "pin-remove", "sd": _("_Remove from Bookmarks"), "kb": KB_CONTROL+KB_ALT+"b", "dn": _("Remove the Current Node from the Bookmarks List"), "cb": dad.bookmark_curr_node_remove},
"handle_bookmarks": {"sk": "gtk-edit", "sd": _("_Handle Bookmarks"), "kb": None, "dn": _("Handle the Bookmarks List"), "cb": dad.bookmarks_handle},
"import_cherrytree": {"sk": "from_cherrytree", "sd": _("From _CherryTree File"), "kb": None, "dn": _("Add Nodes of a CherryTree File to the Current Tree"), "cb": dad.nodes_add_from_cherrytree_file},
"import_txt_file": {"sk": "from_txt", "sd": _("From _Plain Text File"), "kb": None, "dn": _("Add Node from a Plain Text File to the Current Tree"), "cb": dad.nodes_add_from_plain_text_file},
"import_txt_folder": {"sk": "from_txt", "sd": _("From _Folder of Plain Text Files"), "kb": None, "dn": _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), "cb": dad.nodes_add_from_plain_text_folder},
"import_html_file": {"sk": "from_html", "sd": _("From _HTML File"), "kb": None, "dn": _("Add Node from an HTML File to the Current Tree"), "cb": dad.nodes_add_from_html_file},
"import_html_folder": {"sk": "from_html", "sd": _("From _Folder of HTML Files"), "kb": None, "dn": _("Add Nodes from a Folder of HTML Files to the Current Tree"), "cb": dad.nodes_add_from_html_folder},
"import_basket": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _Basket Folder"), "kb": None, "dn": _("Add Nodes of a Basket Folder to the Current Tree"), "cb": dad.nodes_add_from_basket_folder},
"import_epim_html": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _EssentialPIM HTML File"), "kb": None, "dn": _("Add Node from an EssentialPIM HTML File to the Current Tree"), "cb": dad.nodes_add_from_epim_html_file},
"import_gnote": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _Gnote Folder"), "kb": None, "dn": _("Add Nodes of a Gnote Folder to the Current Tree"), "cb": dad.nodes_add_from_gnote_folder},
"import_keepnote": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _KeepNote Folder"), "kb": None, "dn": _("Add Nodes of a KeepNote Folder to the Current Tree"), "cb": dad.nodes_add_from_keepnote_folder},
"import_keynote": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From K_eyNote File"), "kb": None, "dn": _("Add Nodes of a KeyNote File to the Current Tree"), "cb": dad.nodes_add_from_keynote_file},
"import_knowit": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From K_nowit File"), "kb": None, "dn": _("Add Nodes of a Knowit File to the Current Tree"), "cb": dad.nodes_add_from_knowit_file},
"import_leo": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _Leo File"), "kb": None, "dn": _("Add Nodes of a Leo File to the Current Tree"), "cb": dad.nodes_add_from_leo_file},
"import_mempad": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _Mempad File"), "kb": None, "dn": _("Add Nodes of a Mempad File to the Current Tree"), "cb": dad.nodes_add_from_mempad_file},
"import_notecase": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _NoteCase File"), "kb": None, "dn": _("Add Nodes of a NoteCase File to the Current Tree"), "cb": dad.nodes_add_from_notecase_file},
"import_rednotebook": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _RedNotebook Folder"), "kb": None, "dn": _("Add Nodes of a RedNotebook Folder to the Current Tree"), "cb": dad.nodes_add_from_rednotebook_folder},
"import_tomboy": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From T_omboy Folder"), "kb": None, "dn": _("Add Nodes of a Tomboy Folder to the Current Tree"), "cb": dad.nodes_add_from_tomboy_folder},
"import_treepad": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From T_reepad Lite File"), "kb": None, "dn": _("Add Nodes of a Treepad Lite File to the Current Tree"), "cb": dad.nodes_add_from_treepad_file},
"import_tuxcards": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _TuxCards File"), "kb": None, "dn": _("Add Nodes of a TuxCards File to the Current Tree"), "cb": dad.nodes_add_from_tuxcards_file},
"import_zim": {"sk": cons.STR_STOCK_CT_IMP, "sd": _("From _Zim Folder"), "kb": None, "dn": _("Add Nodes of a Zim Folder to the Current Tree"), "cb": dad.nodes_add_from_zim_folder},
"export_pdf": {"sk": "to_pdf", "sd": _("Export To _PDF"), "kb": None, "dn": _("Export To PDF"), "cb": dad.export_to_pdf},
"command_palette": {"sk": "gtk-execute", "sd": _("Command Palette"), "kb": KB_CONTROL+KB_SHIFT+"p", "dn": _("Command Palette"), "cb": dad.show_command_palette},
"export_html": {"sk": "to_html", "sd": _("Export To _HTML"), "kb": None, "dn": _("Export To HTML"), "cb": dad.export_to_html},
"export_txt_multiple": {"sk": "to_txt", "sd": _("Export to Multiple Plain _Text Files"), "kb": None, "dn": _("Export to Multiple Plain Text Files"), "cb": dad.export_to_txt_multiple},
"export_txt_single": {"sk": "to_txt", "sd": _("Export to _Single Plain Text File"), "kb": None, "dn": _("Export to Single Plain Text File"), "cb": dad.export_to_txt_single},
"export_ctd": {"sk": "to_cherrytree", "sd": _("_Export To CherryTree Document"), "kb": None, "dn": _("Export To CherryTree Document"), "cb": dad.export_to_ctd},
"ct_check_newer": {"sk": "gtk-network", "sd": _("_Check Newer Version"), "kb": None, "dn": _("Check for a Newer Version"), "cb": dad.check_for_newer_version},
"ct_help": {"sk": "help-contents", "sd": _("Online _Manual"), "kb": "F1", "dn": _("Application's Online Manual"), "cb": dad.on_help_menu_item_activated},
"ct_about": {"sk": "gtk-about", "sd": _("_About"), "kb": None, "dn": _("About CherryTree"), "cb": dad.dialog_about},
"anch_cut": {"sk": "edit-cut", "sd": _("C_ut Anchor"), "kb": None, "dn": _("Cut the Selected Anchor"), "cb": dad.anchor_cut},
"anch_copy": {"sk": "edit-copy", "sd": _("_Copy Anchor"), "kb": None, "dn": _("Copy the Selected Anchor"), "cb": dad.anchor_copy},
"anch_del": {"sk": "edit-delete", "sd": _("_Delete Anchor"), "kb": None, "dn": _("Delete the Selected Anchor"), "cb": dad.anchor_delete},
"anch_edit": {"sk": "anchor_edit", "sd": _("Edit _Anchor"), "kb": None, "dn": _("Edit the Underlying Anchor"), "cb": dad.anchor_edit},
"emb_file_cut": {"sk": "edit-cut", "sd": _("C_ut Embedded File"), "kb": None, "dn": _("Cut the Selected Embedded File"), "cb": dad.embfile_cut},
"emb_file_copy": {"sk": "edit-copy", "sd": _("_Copy Embedded File"), "kb": None, "dn": _("Copy the Selected Embedded File"), "cb": dad.embfile_copy},
"emb_file_del": {"sk": "edit-delete", "sd": _("_Delete Embedded File"), "kb": None, "dn": _("Delete the Selected Embedded File"), "cb": dad.embfile_delete},
"emb_file_save": {"sk": "gtk-save-as", "sd": _("Save _As"), "kb": None, "dn": _("Save File As"), "cb": dad.embfile_save},
"emb_file_open": {"sk": "gtk-open", "sd": _("_Open File"), "kb": None, "dn": _("Open Embedded File"), "cb": dad.embfile_open},
"img_save": {"sk": "image_save", "sd": _("_Save Image as PNG"), "kb": None, "dn": _("Save the Selected Image as a PNG file"), "cb": dad.image_save},
"img_edit": {"sk": "image_edit", "sd": _("_Edit Image"), "kb": None, "dn": _("Edit the Selected Image"), "cb": dad.image_edit},
"img_cut": {"sk": "edit-cut", "sd": _("C_ut Image"), "kb": None, "dn": _("Cut the Selected Image"), "cb": dad.image_cut},
"img_copy": {"sk": "edit-copy", "sd": _("_Copy Image"), "kb": None, "dn": _("Copy the Selected Image"), "cb": dad.image_copy},
"img_del": {"sk": "edit-delete", "sd": _("_Delete Image"), "kb": None, "dn": _("Delete the Selected Image"), "cb": dad.image_delete},
"img_link_edit": {"sk": "link_handle", "sd": _("Edit _Link"), "kb": None, "dn": _("Edit the Link Associated to the Image"), "cb": dad.image_link_edit},
"img_link_dismiss": {"sk": "gtk-clear", "sd": _("D_ismiss Link"), "kb": None, "dn": _("Dismiss the Link Associated to the Image"), "cb": dad.image_link_dismiss},
"toggle_show_mainwin": {"sk": cons.APP_NAME, "sd": _("Show/Hide _CherryTree"), "kb": None, "dn": _("Toggle Show/Hide CherryTree"), "cb": dad.toggle_show_hide_main_window},
"strip_trail_spaces": {"sk": "gtk-clear", "sd": _("Stri_p Trailing Spaces"), "kb": None, "dn": _("Strip Trailing Spaces"), "cb": dad.strip_trailing_spaces},
}

def get_entries(dad):
    """Returns the Menu Entries Given the Class Instance"""
    if not hasattr(dad, "menudict"): load_menudict(dad)
    entries = [
    # name, stock id, label
    ("FileMenu", None, _("_File")),
    ("EditMenu", None, _("_Edit")),
    ("FormattingMenu", None, _("For_matting")),
    ("TreeMenu", None, _("_Tree")),
    ("TreeMoveMenu", "gtk-jump-to", _("Node _Move")),
    ("TreeImportMenu", cons.STR_STOCK_CT_IMP, _("Nodes _Import")),
    ("TreeExportMenu", "export_from_cherrytree", _("Nodes E_xport")),
    ("ChangeCaseMenu", "case_toggle", _("C_hange Case")),
    ("SearchMenu", None, _("_Search")),
    ("ViewMenu", None, _("_View")),
    ("BookmarksMenu", None, _("_Bookmarks")),
    ("ImportMenu", None, _("_Import")),
    ("ExportMenu", None, _("E_xport")),
    ("HelpMenu", None, _("_Help"))]
    for name in dad.menudict.keys():
        # name, stock id, label, accelerator, tooltip, callback
        mi_tuple = get_menu_item_tuple(dad, name)
        entries.append((name, mi_tuple[0], mi_tuple[1], mi_tuple[2], mi_tuple[3], mi_tuple[4]))
    return entries

def polish_overridden_keyboard_shortcuts(dad):
    """Polish the Overridded Keyboard Shortcuts"""
    for name in dad.custom_kb_shortcuts.keys():
        if dad.custom_kb_shortcuts[name] == dad.menudict[name]["kb"]:
            del dad.custom_kb_shortcuts[name]

def get_menu_item_name_from_shortcut(dad, kb_shortcut):
    """Returns a Menu Item name from a Keyboard Shortcut"""
    ret_name = ""
    for name in dad.menudict.keys():
        curr_kb_shortcut = get_menu_item_kb_shortcut(dad, name)
        if curr_kb_shortcut == kb_shortcut:
            ret_name = name
            break
    return ret_name

def get_menu_item_kb_shortcut(dad, name):
    """Returns the Keyboard Shortcut for a Menu Item"""
    return dad.menudict[name]["kb"] if not name in dad.custom_kb_shortcuts.keys() else dad.custom_kb_shortcuts[name]

def get_menu_item_tuple(dad, name):
    """Returns the Tuple for a Menu Item"""
    subdict = dad.menudict[name]
    kb_shortcut = get_menu_item_kb_shortcut(dad, name)
    return (subdict["sk"], subdict["sd"], kb_shortcut, subdict["dn"], subdict["cb"])

def get_popup_menu_tree(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
get_menu_item_tuple(dad, "tree_add_node"),
get_menu_item_tuple(dad, "tree_add_subnode"),
get_menu_item_tuple(dad, "tree_dup_node"),
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "tree_node_prop"),
get_menu_item_tuple(dad, "tree_node_toggle_ro"),
get_menu_item_tuple(dad, "node_bookmark"),
get_menu_item_tuple(dad, "node_unbookmark"),
get_menu_item_tuple(dad, "tree_node_date"),
get_menu_item_tuple(dad, "tree_parse_info"),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("Node _Move"), "gtk-jump-to", None, None),
get_menu_item_tuple(dad, "tree_node_up"),
get_menu_item_tuple(dad, "tree_node_down"),
get_menu_item_tuple(dad, "tree_node_left"),
get_menu_item_tuple(dad, "tree_node_right"),
get_menu_item_tuple(dad, "tree_node_new_father"),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("Nodes _Sort"), "gtk-sort-ascending", None, None),
get_menu_item_tuple(dad, "tree_all_sort_asc"),
get_menu_item_tuple(dad, "tree_all_sort_desc"),
get_menu_item_tuple(dad, "tree_sibl_sort_asc"),
get_menu_item_tuple(dad, "tree_sibl_sort_desc"),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "find_in_node_names"),
get_menu_item_tuple(dad, "replace_in_node_names"),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("Nodes _Import"), cons.STR_STOCK_CT_IMP, None, None),
get_menu_item_tuple(dad, "import_cherrytree"),
get_menu_item_tuple(dad, "import_txt_file"),
get_menu_item_tuple(dad, "import_txt_folder"),
get_menu_item_tuple(dad, "import_html_file"),
get_menu_item_tuple(dad, "import_html_folder"),
get_menu_item_tuple(dad, "import_basket"),
get_menu_item_tuple(dad, "import_epim_html"),
get_menu_item_tuple(dad, "import_gnote"),
get_menu_item_tuple(dad, "import_keepnote"),
get_menu_item_tuple(dad, "import_keynote"),
get_menu_item_tuple(dad, "import_knowit"),
get_menu_item_tuple(dad, "import_leo"),
get_menu_item_tuple(dad, "import_mempad"),
get_menu_item_tuple(dad, "import_notecase"),
get_menu_item_tuple(dad, "import_rednotebook"),
get_menu_item_tuple(dad, "import_tomboy"),
get_menu_item_tuple(dad, "import_treepad"),
get_menu_item_tuple(dad, "import_tuxcards"),
get_menu_item_tuple(dad, "import_zim"),
("submenu-end", None, None, None, None),
("submenu-start", _("Nodes E_xport"), "export_from_cherrytree", None, None),
get_menu_item_tuple(dad, "export_pdf"),
get_menu_item_tuple(dad, "export_html"),
get_menu_item_tuple(dad, "export_txt_multiple"),
get_menu_item_tuple(dad, "export_txt_single"),
get_menu_item_tuple(dad, "export_ctd"),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "child_nodes_inherit_syntax"),
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "tree_node_del"),
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "go_node_prev"),
get_menu_item_tuple(dad, "go_node_next"),
]

def get_popup_menu_entries_text(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "cut_plain"),
get_menu_item_tuple(dad, "copy_plain"),
get_menu_item_tuple(dad, "paste_plain"),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("For_matting") , "format_text", None, None),
get_menu_item_tuple(dad, "fmt_latest"),
get_menu_item_tuple(dad, "fmt_rm"),
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "fmt_color_fg"),
get_menu_item_tuple(dad, "fmt_color_bg"),
get_menu_item_tuple(dad, "fmt_bold"),
get_menu_item_tuple(dad, "fmt_italic"),
get_menu_item_tuple(dad, "fmt_underline"),
get_menu_item_tuple(dad, "fmt_strikethrough"),
get_menu_item_tuple(dad, "fmt_h1"),
get_menu_item_tuple(dad, "fmt_h2"),
get_menu_item_tuple(dad, "fmt_h3"),
get_menu_item_tuple(dad, "fmt_small"),
get_menu_item_tuple(dad, "fmt_superscript"),
get_menu_item_tuple(dad, "fmt_subscript"),
get_menu_item_tuple(dad, "fmt_monospace"),
("submenu-end", None, None, None, None),
("submenu-start", _("_Justify") , "gtk-justify-center", None, None),
get_menu_item_tuple(dad, "fmt_justify_left"),
get_menu_item_tuple(dad, "fmt_justify_center"),
get_menu_item_tuple(dad, "fmt_justify_right"),
get_menu_item_tuple(dad, "fmt_justify_fill"),
("submenu-end", None, None, None, None),
("submenu-start", _("_List") , "list_bulleted", None, None),
get_menu_item_tuple(dad, "handle_bull_list"),
get_menu_item_tuple(dad, "handle_num_list"),
get_menu_item_tuple(dad, "handle_todo_list"),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("_Insert") , "insert", None, None),
get_menu_item_tuple(dad, "handle_image"),
#get_menu_item_tuple(dad, "handle_screenshot"),
get_menu_item_tuple(dad, "handle_table"),
get_menu_item_tuple(dad, "handle_codebox"),
get_menu_item_tuple(dad, "handle_embfile"),
get_menu_item_tuple(dad, "handle_link"),
get_menu_item_tuple(dad, "handle_anchor"),
get_menu_item_tuple(dad, "insert_toc"),
get_menu_item_tuple(dad, "insert_timestamp"),
get_menu_item_tuple(dad, "insert_horiz_rule"),
("submenu-end", None, None, None, None),
("submenu-start", _("C_hange Case") , "case_toggle", None, None),
get_menu_item_tuple(dad, "case_down"),
get_menu_item_tuple(dad, "case_up"),
get_menu_item_tuple(dad, "case_tggl"),
("submenu-end", None, None, None, None),
("submenu-start", _("_Row") , "gtk-edit", None, None),
get_menu_item_tuple(dad, "cut_row"),
get_menu_item_tuple(dad, "copy_row"),
get_menu_item_tuple(dad, "del_row"),
get_menu_item_tuple(dad, "dup_row"),
get_menu_item_tuple(dad, "mv_up_row"),
get_menu_item_tuple(dad, "mv_down_row"),
("submenu-end", None, None, None, None),
get_menu_item_tuple(dad, "strip_trail_spaces"),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("_Search") , "find", None, None),
get_menu_item_tuple(dad, "find_in_node"),
get_menu_item_tuple(dad, "find_in_allnodes"),
get_menu_item_tuple(dad, "find_in_node_n_sub"),
get_menu_item_tuple(dad, "find_in_node_names"),
get_menu_item_tuple(dad, "find_iter_fw"),
get_menu_item_tuple(dad, "find_iter_bw"),
("submenu-end", None, None, None, None),
("submenu-start", _("_Replace") , "find_replace", None, None),
get_menu_item_tuple(dad, "replace_in_node"),
get_menu_item_tuple(dad, "replace_in_allnodes"),
get_menu_item_tuple(dad, "replace_in_node_n_sub"),
get_menu_item_tuple(dad, "replace_in_node_names"),
get_menu_item_tuple(dad, "replace_iter_fw"),
("submenu-end", None, None, None, None),
]

def get_popup_menu_entries_code(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "cut_plain"),
get_menu_item_tuple(dad, "copy_plain"),
(cons.TAG_SEPARATOR, None, None, None, None),
get_menu_item_tuple(dad, "exec_code"),
("submenu-start", _("_Insert") , "insert", None, None),
get_menu_item_tuple(dad, "insert_timestamp"),
get_menu_item_tuple(dad, "insert_horiz_rule"),
("submenu-end", None, None, None, None),
get_menu_item_tuple(dad, "strip_trail_spaces"),
("submenu-start", _("C_hange Case") , "case_toggle", None, None),
get_menu_item_tuple(dad, "case_down"),
get_menu_item_tuple(dad, "case_up"),
get_menu_item_tuple(dad, "case_tggl"),
("submenu-end", None, None, None, None),
("submenu-start", _("_Row") , "gtk-edit", None, None),
get_menu_item_tuple(dad, "cut_row"),
get_menu_item_tuple(dad, "copy_row"),
get_menu_item_tuple(dad, "del_row"),
get_menu_item_tuple(dad, "dup_row"),
get_menu_item_tuple(dad, "mv_up_row"),
get_menu_item_tuple(dad, "mv_down_row"),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("_Search") , "find", None, None),
get_menu_item_tuple(dad, "find_in_node"),
get_menu_item_tuple(dad, "find_in_allnodes"),
get_menu_item_tuple(dad, "find_in_node_n_sub"),
get_menu_item_tuple(dad, "find_in_node_names"),
get_menu_item_tuple(dad, "find_iter_fw"),
get_menu_item_tuple(dad, "find_iter_bw"),
("submenu-end", None, None, None, None),
("submenu-start", _("_Replace") , "find_replace", None, None),
get_menu_item_tuple(dad, "replace_in_node"),
get_menu_item_tuple(dad, "replace_in_allnodes"),
get_menu_item_tuple(dad, "replace_in_node_n_sub"),
get_menu_item_tuple(dad, "replace_in_node_names"),
get_menu_item_tuple(dad, "replace_iter_fw"),
("submenu-end", None, None, None, None),
]

def get_popup_menu_entries_link(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
("link_handle", _("Edit _Link"), None, _("Edit the Underlying Link"), dad.apply_tag_link),
(cons.TAG_SEPARATOR, None, None, None, None),
("edit-cut", _("C_ut Link"), None, _("Cut the Selected Link"), dad.link_cut),
("edit-copy", _("_Copy Link"), None, _("Copy the Selected Link"), dad.link_copy),
("gtk-clear", _("D_ismiss Link"), None, _("Dismiss the Selected Link"), dad.link_dismiss),
("edit-delete", _("_Delete Link"), None, _("Delete the Selected Link"), dad.link_delete),
]

def get_popup_menu_table(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
("edit-cut", _("C_ut Table"), None, _("Cut the Selected Table"), dad.tables_handler.table_cut),
("edit-copy", _("_Copy Table"), None, _("Copy the Selected Table"), dad.tables_handler.table_copy),
("edit-delete", _("_Delete Table"), None, _("Delete the Selected Table"), dad.tables_handler.table_delete),
(cons.TAG_SEPARATOR, None, None, None, None),
("gtk-add", _("_Add Row"), KB_CONTROL+"comma", _("Add a Table Row"), dad.tables_handler.table_row_add),
("edit-cut", _("Cu_t Row"), None, _("Cut a Table Row"), dad.tables_handler.table_row_cut),
("edit-copy", _("_Copy Row"), None, _("Copy a Table Row"), dad.tables_handler.table_row_copy),
("edit-paste", _("_Paste Row"), None, _("Paste a Table Row"), dad.tables_handler.table_row_paste),
("edit-delete", _("De_lete Row"), KB_CONTROL+KB_ALT+"comma", _("Delete the Selected Table Row"), dad.tables_handler.table_row_delete),
(cons.TAG_SEPARATOR, None, None, None, None),
("gtk-go-up", _("Move Row _Up"), KB_CONTROL+KB_ALT+"period", _("Move the Selected Row Up"), dad.tables_handler.table_row_up),
("gtk-go-down", _("Move Row _Down"), KB_CONTROL+"period", _("Move the Selected Row Down"), dad.tables_handler.table_row_down),
("gtk-sort-descending", _("Sort Rows De_scending"), None, _("Sort all the Rows Descending"), dad.tables_handler.table_rows_sort_descending),
("gtk-sort-ascending", _("Sort Rows As_cending"), None, _("Sort all the Rows Ascending"), dad.tables_handler.table_rows_sort_ascending),
(cons.TAG_SEPARATOR, None, None, None, None),
("table_edit", _("_Edit Table Properties"), None, _("Edit the Table Properties"), dad.tables_handler.table_edit_properties),
("table_save", _("_Table Export"), None, _("Export Table as CSV File"), dad.tables_handler.table_export),
]

def get_popup_menu_entries_table_cell(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
("insert", _("Insert _NewLine"), KB_CONTROL+"period", _("Insert NewLine Char"), dad.curr_table_cell_insert_newline),
]

def get_popup_menu_entries_codebox(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
("edit-cut", _("Cu_t as Plain Text"), KB_CONTROL+KB_SHIFT+"x", _("Cut as Plain Text, Discard the Rich Text Formatting"), dad.dad.cut_as_plain_text),
("edit-copy", _("_Copy as Plain Text"), KB_CONTROL+KB_SHIFT+"c", _("Copy as Plain Text, Discard the Rich Text Formatting"), dad.dad.copy_as_plain_text),
(cons.TAG_SEPARATOR, None, None, None, None),
("codebox_edit", _("Change CodeBox _Properties"), None, _("Edit the Properties of the CodeBox"), dad.codebox_change_properties),
("gtk-execute", _("_Execute CodeBox Code"), None, _("Execute CodeBox Code"), dad.dad.exec_code),
("from_txt", _("CodeBox _Load From Text File"), None, _("Load the CodeBox Content From a Text File"), dad.codebox_load_from_file),
("to_txt", _("CodeBox _Save To Text File"), None, _("Save the CodeBox Content To a Text File"), dad.codebox_save_to_file),
(cons.TAG_SEPARATOR, None, None, None, None),
("edit-cut", _("C_ut CodeBox"), None, _("Cut the Selected CodeBox"), dad.codebox_cut),
("edit-copy", _("_Copy CodeBox"), None, _("Copy the Selected CodeBox"), dad.codebox_copy),
("edit-delete", _("_Delete CodeBox"), None, _("Delete the Selected CodeBox"), dad.codebox_delete),
("edit-delete", _("Delete CodeBox _Keep Content"), None, _("Delete the Selected CodeBox But Keep Its Content"), dad.codebox_delete_keeping_text),
(cons.TAG_SEPARATOR, None, None, None, None),
("gtk-go-forward", _("Increase CodeBox Width"), KB_CONTROL+"period", _("Increase the Width of the CodeBox"), dad.codebox_increase_width),
("gtk-go-back", _("Decrease CodeBox Width"), KB_CONTROL+KB_ALT+"period", _("Decrease the Width of the CodeBox"), dad.codebox_decrease_width),
("gtk-go-down", _("Increase CodeBox Height"), KB_CONTROL+"comma", _("Increase the Height of the CodeBox"), dad.codebox_increase_height),
("gtk-go-up", _("Decrease CodeBox Height"), KB_CONTROL+KB_ALT+"comma", _("Decrease the Height of the CodeBox"), dad.codebox_decrease_height),
]

UI_INFO = """
<ui>
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
      <menuitem action='command_palette'/>
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
        <menuitem action='tree_add_node'/>
        <menuitem action='tree_add_subnode'/>
        <menuitem action='tree_dup_node'/>
        <menuitem action='tree_node_date'/>
        <menuitem action='tree_all_sort_asc'/>
        <menuitem action='tree_all_sort_desc'/>
        <menuitem action='tree_sibl_sort_asc'/>
        <menuitem action='tree_sibl_sort_desc'/>
        <menuitem action='tree_node_prop'/>
        <menuitem action='tree_node_toggle_ro'/>
        <menuitem action='child_nodes_inherit_syntax'/>
        <menuitem action='tree_parse_info'/>
        <menuitem action='node_bookmark'/>
        <menuitem action='node_unbookmark'/>
        <menuitem action='handle_bookmarks'/>
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

  <popup name='SysTrayMenu'>
    <menuitem action='toggle_show_mainwin'/>
    <separator/>
    <menuitem action='exit_app'/>
  </popup>

  <popup name='ImageMenu'>
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

  <popup name='AnchorMenu'>
    <menuitem action='anch_cut'/>
    <menuitem action='anch_copy'/>
    <menuitem action='anch_del'/>
    <separator/>
    <menuitem action='anch_edit'/>
  </popup>

  <popup name='EmbFileMenu'>
    <menuitem action='emb_file_cut'/>
    <menuitem action='emb_file_copy'/>
    <menuitem action='emb_file_del'/>
    <separator/>
    <menuitem action='emb_file_open'/>
    <menuitem action='emb_file_save'/>
  </popup>
</ui>
"""

TOOLBAR_SPLIT = "toolbar_split"
TOOLBAR_VEC_DEFAULT = ["tree_add_node", "tree_add_subnode", cons.TAG_SEPARATOR, "go_node_prev", "go_node_next", cons.TAG_SEPARATOR, cons.CHAR_STAR, "ct_save", "export_pdf", cons.TAG_SEPARATOR, "find_in_allnodes", cons.TAG_SEPARATOR, "handle_bull_list", "handle_num_list", "handle_todo_list", cons.TAG_SEPARATOR, "handle_image", "handle_table", "handle_codebox", "handle_embfile", "handle_link", "handle_anchor", cons.TAG_SEPARATOR, "fmt_rm", "fmt_color_fg", "fmt_color_bg", "fmt_bold", "fmt_italic", "fmt_underline", "fmt_strikethrough", "fmt_h1", "fmt_h2", "fmt_h3", "fmt_small", "fmt_superscript", "fmt_subscript", "fmt_monospace"]

TOOLBAR_VEC_BLACKLIST = [TOOLBAR_SPLIT, "anch_cut", "anch_copy", "anch_del", "anch_edit", "emb_file_cut", "emb_file_copy", "emb_file_del", "emb_file_save", "emb_file_open", "img_save", "img_edit", "img_cut", "img_copy", "img_del", "img_link_edit", "img_link_dismiss", "toggle_show_mainwin"]
