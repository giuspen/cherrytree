# -*- coding: UTF-8 -*-
#
#       menus.py
#
#       Copyright 2009-2016 Giuseppe Penone <giuspen@gmail.com>
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


SHORTCUTS = {
"NewInstance": None,
"OpenFile": "<control>O",
"Save": "<control>S",
"SaveAs": "<control><shift>S",
"OpenCfgFolder": None,
}

def get_entries(dad):
    """Returns the Menu Entries Given the Class Instance"""
    return [
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
("HelpMenu", None, _("_Help")),

# name, stock id, label, accelerator, tooltip, callback
("NewInstance", "new-instance", _("New _Instance"), SHORTCUTS["NewInstance"], _("Start a New Instance of CherryTree"), dad.file_new),
("OpenFile", "gtk-open", _("_Open File"), SHORTCUTS["OpenFile"], _("Open a CherryTree Document"), dad.file_open),
("Save", "gtk-save", _("_Save"), SHORTCUTS["Save"], _("Save File"), dad.file_save),
("SaveAs", "gtk-save-as", _("Save _As"), SHORTCUTS["SaveAs"], _("Save File As"), dad.file_save_as),
("OpenCfgFolder", "gtk-directory", _("Open Preferences _Directory"), SHORTCUTS["OpenCfgFolder"], _("Open the Directory with Preferences Files"), dad.folder_cfg_open),
("PageSetup", "gtk-print", _("Pa_ge Setup"), "<control><shift>P", _("Set up the Page for Printing"), dad.export_print_page_setup),
("NodePrint", "gtk-print", _("_Print"), "<control>P", _("Print"), dad.export_print),
("QuitApp", "quit-app", _("_Quit"), "<control>Q", _("Quit the Application"), dad.quit_application),
("ExitApp", "quit-app", _("_Exit CherryTree"), "<control><shift>Q", _("Exit from CherryTree"), dad.quit_application_totally),
("Preferences", "gtk-preferences", _("_Preferences"), "<control><alt>P", _("Preferences"), dad.dialog_preferences),
("Undo", "gtk-undo", _("_Undo"), "<control>Z", _("Undo Last Operation"), dad.requested_step_back),
("Redo", "gtk-redo", _("_Redo"), "<control>Y", _("Redo Previously Discarded Operation"), dad.requested_step_ahead),
("HandleImage", "image_insert", _("Insert I_mage"), "<control><alt>I", _("Insert an Image"), dad.image_handle),
("HandleTable", "table_insert", _("Insert _Table"), "<control><alt>T", _("Insert a Table"), dad.table_handle),
("HandleCodeBox", "codebox_insert", _("Insert _CodeBox"), "<control><alt>C", _("Insert a CodeBox"), dad.codebox_handle),
("EmbFileInsert", "file_icon", _("Insert _File"), "<control><alt>E", _("Insert File"), dad.embfile_insert),
("HandleLink", "link_handle", _("Insert/Edit _Link"), "<control>L", _("Insert a Link/Edit the Underlying Link"), dad.apply_tag_link),
("HandleAnchor", "anchor_insert", _("Insert _Anchor"), "<control><alt>A", _("Insert an Anchor"), dad.anchor_handle),
("InsertTOC", "index", _("Insert T_OC"), None, _("Insert Table of Contents"), dad.toc_insert),
("Timestamp", "timestamp", _("Insert Ti_mestamp"), "<control><alt>M", _("Insert Timestamp"), dad.timestamp_insert),
("HorizontalRule", "horizontal_rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), dad.horizontal_rule_insert),
("DownCase", "case_lower", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), dad.text_selection_lower_case),
("UpCase", "case_upper", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), dad.text_selection_upper_case),
("ToggleCase", "case_toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), dad.text_selection_toggle_case),
("EnaDisSpellCheck", "gtk-spell-check", _("Enable/Disable _Spell Check"), "<control><alt>S", _("Toggle Enable/Disable Spell Check"), dad.toggle_ena_dis_spellcheck),
("CutPlainText", "edit-cut", _("Cu_t as Plain Text"), "<control><shift>X", _("Cut as Plain Text, Discard the Rich Text Formatting"), dad.cut_as_plain_text),
("CopyPlainText", "edit-copy", _("_Copy as Plain Text"), "<control><shift>C", _("Copy as Plain Text, Discard the Rich Text Formatting"), dad.copy_as_plain_text),
("PastePlainText", "edit-paste", _("_Paste as Plain Text"), "<control><shift>V", _("Paste as Plain Text, Discard the Rich Text Formatting"), dad.paste_as_plain_text),
("CutRow", "edit-cut", _("Cu_t Row"), "<shift><alt>X", _("Cut the Current Row/Selected Rows"), dad.text_row_cut),
("CopyRow", "edit-copy", _("_Copy Row"), "<shift><alt>C", _("Copy the Current Row/Selected Rows"), dad.text_row_copy),
("DeleteRow", "edit-delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), dad.text_row_delete),
("DuplicateRow", "gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), dad.text_row_selection_duplicate),
("MoveRowUp", "gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), dad.text_row_up),
("MoveRowDown", "gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), dad.text_row_down),
("FormatLatest", "format_text_latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), dad.apply_tag_latest),
("RemoveFormatting", "format_text_clear", _("_Remove Formatting"), "<control><shift>R", _("Remove the Formatting from the Selected Text"), dad.remove_text_formatting),
("ColorForeground", "color_foreground", _("Text _Color Foreground"), "<shift><alt>F", _("Change the Color of the Selected Text Foreground"), dad.apply_tag_foreground),
("ColorBackground", "color_background", _("Text C_olor Background"), "<shift><alt>B", _("Change the Color of the Selected Text Background"), dad.apply_tag_background),
("Bold", "format-text-bold", _("Toggle _Bold Property"), "<control>B", _("Toggle Bold Property of the Selected Text"), dad.apply_tag_bold),
("Italic", "format-text-italic", _("Toggle _Italic Property"), "<control>I", _("Toggle Italic Property of the Selected Text"), dad.apply_tag_italic),
("Underline", "format-text-underline", _("Toggle _Underline Property"), "<control>U", _("Toggle Underline Property of the Selected Text"), dad.apply_tag_underline),
("Strikethrough", "format-text-strikethrough", _("Toggle Stri_kethrough Property"), "<control>E", _("Toggle Strikethrough Property of the Selected Text"), dad.apply_tag_strikethrough),
("H1", "format-text-large", _("Toggle h_1 Property"), "<control>1", _("Toggle h1 Property of the Selected Text"), dad.apply_tag_h1),
("H2", "format-text-large2", _("Toggle h_2 Property"), "<control>2", _("Toggle h2 Property of the Selected Text"), dad.apply_tag_h2),
("H3", "format-text-large3", _("Toggle h_3 Property"), "<control>3", _("Toggle h3 Property of the Selected Text"), dad.apply_tag_h3),
("Small", "format-text-small", _("Toggle _Small Property"), "<control>0", _("Toggle Small Property of the Selected Text"), dad.apply_tag_small),
("Superscript", "format-text-superscript", _("Toggle Su_perscript Property"), None, _("Toggle Superscript Property of the Selected Text"), dad.apply_tag_superscript),
("Subscript", "format-text-subscript", _("Toggle Su_bscript Property"), None, _("Toggle Subscript Property of the Selected Text"), dad.apply_tag_subscript),
("Monospace", "format-text-monospace", _("Toggle _Monospace Property"), "<control>M", _("Toggle Monospace Property of the Selected Text"), dad.apply_tag_monospace),
("BulletedList", "list_bulleted", _("Set/Unset _Bulleted List"), "<control><alt>1", _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), dad.list_bulleted_handler),
("NumberedList", "list_numbered", _("Set/Unset _Numbered List"), "<control><alt>2", _("Set/Unset the Current Paragraph/Selection as a Numbered List"), dad.list_numbered_handler),
("ToDoList", "list_todo", _("Set/Unset _To-Do List"), "<control><alt>3", _("Set/Unset the Current Paragraph/Selection as a To-Do List"), dad.list_todo_handler),
("JustifyLeft", "gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), dad.apply_tag_justify_left),
("JustifyCenter", "gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), dad.apply_tag_justify_center),
("JustifyRight", "gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), dad.apply_tag_justify_right),
("JustifyFill", "gtk-justify-fill", _("Justify _Fill"), None, _("Justify Fill the Current Paragraph"), dad.apply_tag_justify_fill),
("TreeAddNode", "tree-node-add", _("Add _Node"), "<control>N", _("Add a Node having the same Father of the Selected Node"), dad.node_add),
("TreeAddSubNode", "tree-subnode-add", _("Add _SubNode"), "<control><shift>N", _("Add a Child Node to the Selected Node"), dad.node_child_add),
("TreeDuplicateNode", "tree-node-dupl", _("_Duplicate Node"), "<control><shift>D", _("Duplicate the Selected Node"), dad.node_duplicate),
("NodeEdit", "cherry_edit", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), dad.node_edit),
("NodeToggleRO", "cherry_edit", _("Toggle _Read Only"), "<Ctrl><Alt>R", _("Toggle the Read Only Property of the Selected Node"), dad.node_toggle_read_only),
("NodeDate", "calendar", _("Insert Today's Node"), "F8", _("Insert a Node with Hierarchy Year/Month/Day"), dad.node_date),
("TreeInfo", "gtk-info", _("Tree _Info"), None, _("Tree Summary Information"), dad.tree_info),
("NodeUp", "gtk-go-up", _("Node _Up"), "<shift>Up", _("Move the Selected Node Up"), dad.node_up),
("NodeDown", "gtk-go-down", _("Node _Down"), "<shift>Down", _("Move the Selected Node Down"), dad.node_down),
("NodeLeft", "gtk-go-back", _("Node _Left"), "<shift>Left", _("Move the Selected Node Left"), dad.node_left),
("NodeNewFather", "gtk-jump-to", _("Node Change _Father"), "<shift>Right", _("Change the Selected Node's Father"), dad.node_change_father),
("TreeSortAsc", "gtk-sort-ascending", _("Sort Tree _Ascending"), None, _("Sort the Tree Ascending"), dad.tree_sort_ascending),
("TreeSortDesc", "gtk-sort-descending", _("Sort Tree _Descending"), None, _("Sort the Tree Descending"), dad.tree_sort_descending),
("SiblSortAsc", "gtk-sort-ascending", _("Sort Siblings A_scending"), None, _("Sort all the Siblings of the Selected Node Ascending"), dad.node_siblings_sort_ascending),
("SiblSortDesc", "gtk-sort-descending", _("Sort Siblings D_escending"), None, _("Sort all the Siblings of the Selected Node Descending"), dad.node_siblings_sort_descending),
("InheritSyntax", "gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"), dad.node_inherit_syntax),
("NodeDel", "edit-delete", _("De_lete Node"), "Delete", _("Delete the Selected Node"), dad.node_delete),
("GoBack", "gtk-go-back", _("Go _Back"), "<alt>Left", _("Go to the Previous Visited Node"), dad.go_back),
("GoForward", "gtk-go-forward", _("Go _Forward"), "<alt>Right", _("Go to the Next Visited Node"), dad.go_forward),
("FindInNode", "find", _("_Find in Node Content"), "<control>F", _("Find into the Selected Node Content"), dad.find_in_selected_node),
("FindInNodes", "find", _("Find in _All Nodes Contents"), "<control><shift>F", _("Find into All the Tree Nodes Contents"), dad.find_in_all_nodes),
("FindInSelNSub", "find", _("Find in _Selected Node and Subnodes Contents"), "<control><alt>F", _("Find into the Selected Node and Subnodes Contents"), dad.find_in_sel_node_and_subnodes),
("FindNode", "find", _("Find in _Nodes Names and Tags"), "<control>T", _("Find in Nodes Names and Tags"), dad.find_a_node),
("FindAgain", "find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), dad.find_again),
("FindBack", "find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), dad.find_back),
("ReplaceInNode", "find_replace", _("_Replace in Node Content"), "<control>H", _("Replace into the Selected Node Content"), dad.replace_in_selected_node),
("ReplaceInNodes", "find_replace", _("Replace in _All Nodes Contents"), "<control><shift>H", _("Replace into All the Tree Nodes Contents"), dad.replace_in_all_nodes),
("ReplaceInSelNSub", "find_replace", _("Replace in _Selected Node and Subnodes Contents"), "<control><alt>H", _("Replace into the Selected Node and Subnodes Contents"), dad.replace_in_sel_node_and_subnodes),
("ReplaceInNodesNames", "find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), dad.replace_in_nodes_names),
("ReplaceAgain", "find_replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), dad.replace_again),
("ShowHideTree", "cherries", _("Show/Hide _Tree"), "F9", _("Toggle Show/Hide Tree"), dad.toggle_show_hide_tree),
("ShowHideToolbar", "toolbar", _("Show/Hide Tool_bar"), None, _("Toggle Show/Hide Toolbar"), dad.toggle_show_hide_toolbar),
("ShowHideNodeNameHeader", "node_name_header", _("Show/Hide Node Name _Header"), None, _("Toggle Show/Hide Node Name Header"), dad.toggle_show_hide_node_name_header),
("ShowAllMatchesDialog", "find", _("Show _All Matches Dialog"), "<control><shift>A", _("Show Search All Matches Dialog"), dad.find_allmatchesdialog_restore),
("ToggleTreeText", "gtk-jump-to", _("Toggle _Focus Tree/Text"), "<control>Tab", _("Toggle Focus Between Tree and Text"), dad.toggle_tree_text),
("ToggleNodeExpColl", "gtk-zoom-in", _("Toggle Node _Expanded/Collapsed"), "<control><shift>J", _("Toggle Expanded/Collapsed Status of the Selected Node"), dad.toggle_tree_node_expanded_collapsed),
("NodesExpAll", "gtk-zoom-in", _("E_xpand All Nodes"), "<control><shift>E", _("Expand All the Tree Nodes"), dad.nodes_expand_all),
("NodesCollAll", "gtk-zoom-out", _("_Collapse All Nodes"), "<control><shift>L", _("Collapse All the Tree Nodes"), dad.nodes_collapse_all),
("IncreaseToolbarIconsSize", "gtk-add", _("_Increase Toolbar Icons Size"), None, _("Increase the Size of the Toolbar Icons"), dad.toolbar_icons_size_increase),
("DecreaseToolbarIconsSize", "gtk-remove", _("_Decrease Toolbar Icons Size"), None, _("Decrease the Size of the Toolbar Icons"), dad.toolbar_icons_size_decrease),
("Fullscreen", "gtk-fullscreen", _("_Full Screen On/Off"), "F11", _("Toggle Full Screen On/Off"), dad.fullscreen_toggle),
("BookmarksHandle", "gtk-edit", _("_Handle Bookmarks"), None, _("Handle the Bookmarks List"), dad.bookmarks_handle),
("FromCherryTree", "from_cherrytree", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), dad.nodes_add_from_cherrytree_file),
("FromTxtFile", "from_txt", _("From _Plain Text File"), None, _("Add Node from a Plain Text File to the Current Tree"), dad.nodes_add_from_plain_text_file),
("FromTxtFolder", "from_txt", _("From _Folder of Plain Text Files"), None, _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), dad.nodes_add_from_plain_text_folder),
("FromHtmlFile", "from_html", _("From _HTML File"), None, _("Add Node from an HTML File to the Current Tree"), dad.nodes_add_from_html_file),
("FromHtmlFolder", "from_html", _("From _Folder of HTML Files"), None, _("Add Nodes from a Folder of HTML Files to the Current Tree"), dad.nodes_add_from_html_folder),
("FromBasket", cons.STR_STOCK_CT_IMP, _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), dad.nodes_add_from_basket_folder),
("FromEPIMHTML", cons.STR_STOCK_CT_IMP, _("From _EssentialPIM HTML File"), None, _("Add Node from an EssentialPIM HTML File to the Current Tree"), dad.nodes_add_from_epim_html_file),
("FromGnote", cons.STR_STOCK_CT_IMP, _("From _Gnote Folder"), None, _("Add Nodes of a Gnote Folder to the Current Tree"), dad.nodes_add_from_gnote_folder),
("FromKeepNote", cons.STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), dad.nodes_add_from_keepnote_folder),
("FromKeyNote", cons.STR_STOCK_CT_IMP, _("From K_eyNote File"), None, _("Add Nodes of a KeyNote File to the Current Tree"), dad.nodes_add_from_keynote_file),
("FromKnowit", cons.STR_STOCK_CT_IMP, _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), dad.nodes_add_from_knowit_file),
("FromLeo", cons.STR_STOCK_CT_IMP, _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), dad.nodes_add_from_leo_file),
("FromMempad", cons.STR_STOCK_CT_IMP, _("From _Mempad File"), None, _("Add Nodes of a Mempad File to the Current Tree"), dad.nodes_add_from_mempad_file),
("FromNoteCase", cons.STR_STOCK_CT_IMP, _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), dad.nodes_add_from_notecase_file),
("FromTomboy", cons.STR_STOCK_CT_IMP, _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), dad.nodes_add_from_tomboy_folder),
("FromTreepad", cons.STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), dad.nodes_add_from_treepad_file),
("FromTuxCards", cons.STR_STOCK_CT_IMP, _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), dad.nodes_add_from_tuxcards_file),
("FromZim", cons.STR_STOCK_CT_IMP, _("From _Zim Folder"), None, _("Add Nodes of a Zim Folder to the Current Tree"), dad.nodes_add_from_zim_folder),
("Export2PDF", "to_pdf", _("Export To _PDF"), None, _("Export To PDF"), dad.export_to_pdf),
("Export2HTML", "to_html", _("Export To _HTML"), None, _("Export To HTML"), dad.export_to_html),
("Export2TxtMultiple", "to_txt", _("Export to Multiple Plain _Text Files"), None, _("Export to Multiple Plain Text Files"), dad.export_to_txt_multiple),
("Export2TxtSingle", "to_txt", _("Export to _Single Plain Text File"), None, _("Export to Single Plain Text File"), dad.export_to_txt_single),
("Export2CTD", "to_cherrytree", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), dad.export_to_ctd),
("CheckNewer", "gtk-network", _("_Check Newer Version"), None, _("Check for a Newer Version"), dad.check_for_newer_version),
("Help", "help-contents", _("Online _Manual"), "F1", _("Application's Online Manual"), dad.on_help_menu_item_activated),
("About", "gtk-about", _("_About"), None, _("About CherryTree"), dad.dialog_about),
("CutAnchor", "edit-cut", _("C_ut Anchor"), None, _("Cut the Selected Anchor"), dad.anchor_cut),
("CopyAnchor", "edit-copy", _("_Copy Anchor"), None, _("Copy the Selected Anchor"), dad.anchor_copy),
("DeleteAnchor", "edit-delete", _("_Delete Anchor"), None, _("Delete the Selected Anchor"), dad.anchor_delete),
("EditAnchor", "anchor_edit", _("Edit _Anchor"), None, _("Edit the Underlying Anchor"), dad.anchor_edit),
("CutEmbFile", "edit-cut", _("C_ut Embedded File"), None, _("Cut the Selected Embedded File"), dad.embfile_cut),
("CopyEmbFile", "edit-copy", _("_Copy Embedded File"), None, _("Copy the Selected Embedded File"), dad.embfile_copy),
("DeleteEmbFile", "edit-delete", _("_Delete Embedded File"), None, _("Delete the Selected Embedded File"), dad.embfile_delete),
("EmbFileSave", "gtk-save-as", _("Save _As"), None, _("Save File As"), dad.embfile_save),
("EmbFileOpen", "gtk-open", _("_Open File"), None, _("Open Embedded File"), dad.embfile_open),
("SaveImage", "image_save", _("_Save Image as PNG"), None, _("Save the Selected Image as a PNG file"), dad.image_save),
("EditImage", "image_edit", _("_Edit Image"), None, _("Edit the Selected Image"), dad.image_edit),
("CutImage", "edit-cut", _("C_ut Image"), None, _("Cut the Selected Image"), dad.image_cut),
("CopyImage", "edit-copy", _("_Copy Image"), None, _("Copy the Selected Image"), dad.image_copy),
("DeleteImage", "edit-delete", _("_Delete Image"), None, _("Delete the Selected Image"), dad.image_delete),
("EditImageLink", "link_handle", _("Edit _Link"), None, _("Edit the Link Associated to the Image"), dad.image_link_edit),
("DismissImageLink", "gtk-clear", _("D_ismiss Link"), None, _("Dismiss the Link Associated to the Image"), dad.image_link_dismiss),
("ShowHideMainWin", cons.APP_NAME, _("Show/Hide _CherryTree"), None, _("Toggle Show/Hide CherryTree"), dad.toggle_show_hide_main_window),
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
("gtk-add", _("_Add Row"), "<control>comma", _("Add a Table Row"), dad.tables_handler.table_row_add),
("edit-cut", _("Cu_t Row"), None, _("Cut a Table Row"), dad.tables_handler.table_row_cut),
("edit-copy", _("_Copy Row"), None, _("Copy a Table Row"), dad.tables_handler.table_row_copy),
("edit-paste", _("_Paste Row"), None, _("Paste a Table Row"), dad.tables_handler.table_row_paste),
("edit-delete", _("De_lete Row"), "<control><alt>comma", _("Delete the Selected Table Row"), dad.tables_handler.table_row_delete),
(cons.TAG_SEPARATOR, None, None, None, None),
("gtk-go-up", _("Move Row _Up"), "<control><alt>period", _("Move the Selected Row Up"), dad.tables_handler.table_row_up),
("gtk-go-down", _("Move Row _Down"), "<control>period", _("Move the Selected Row Down"), dad.tables_handler.table_row_down),
("gtk-sort-descending", _("Sort Rows De_scending"), None, _("Sort all the Rows Descending"), dad.tables_handler.table_rows_sort_descending),
("gtk-sort-ascending", _("Sort Rows As_cending"), None, _("Sort all the Rows Ascending"), dad.tables_handler.table_rows_sort_ascending),
(cons.TAG_SEPARATOR, None, None, None, None),
("table_edit", _("_Edit Table Properties"), None, _("Edit the Table Properties"), dad.tables_handler.table_edit_properties),
("table_save", _("_Table Export"), None, _("Export Table as CSV File"), dad.tables_handler.table_export),
]

def get_popup_menu_tree(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
("tree-node-add", _("Add _Node"), "<control>N", _("Add a Node having the same Father of the Selected Node"), dad.node_add),
("tree-subnode-add", _("Add _SubNode"), "<control><shift>N", _("Add a Child Node to the Selected Node"), dad.node_child_add),
("tree-node-dupl", _("_Duplicate Node"), "<control><shift>D", _("Duplicate the Selected Node"), dad.node_duplicate),
(cons.TAG_SEPARATOR, None, None, None, None),
("cherry_edit", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), dad.node_edit),
("cherry_edit", _("Toggle _Read Only"), "<Ctrl><Alt>R", _("Toggle the Read Only Property of the Selected Node"), dad.node_toggle_read_only),
("pin-add", _("Add to _Bookmarks"), "<control><shift>B", _("Add the Current Node to the Bookmarks List"), dad.bookmark_curr_node),
("pin-remove", _("_Remove from Bookmarks"), "<control><alt>B", _("Remove the Current Node from the Bookmarks List"), dad.bookmark_curr_node_remove),
("calendar", _("Insert Today's Node"), "F8", _("Insert a Node with Hierarchy Year/Month/Day"), dad.node_date),
("gtk-info", _("Tree _Info"), None, _("Tree Summary Information"), dad.tree_info),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("Node _Move"), "gtk-jump-to", None, None),
("gtk-go-up", _("Node _Up"), "<shift>Up", _("Move the Selected Node Up"), dad.node_up),
("gtk-go-down", _("Node _Down"), "<shift>Down", _("Move the Selected Node Down"), dad.node_down),
("gtk-go-back", _("Node _Left"), "<shift>Left", _("Move the Selected Node Left"), dad.node_left),
("gtk-jump-to", _("Node Change _Father"), "<shift>Right", _("Change the Selected Node's Father"), dad.node_change_father),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("Nodes _Sort"), "gtk-sort-ascending", None, None),
("gtk-sort-ascending", _("Sort Tree _Ascending"), None, _("Sort the Tree Ascending"), dad.tree_sort_ascending),
("gtk-sort-descending", _("Sort Tree _Descending"), None, _("Sort the Tree Descending"), dad.tree_sort_descending),
("gtk-sort-ascending", _("Sort Siblings A_scending"), None, _("Sort all the Siblings of the Selected Node Ascending"), dad.node_siblings_sort_ascending),
("gtk-sort-descending", _("Sort Siblings D_escending"), None, _("Sort all the Siblings of the Selected Node Descending"), dad.node_siblings_sort_descending),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("find", _("Find in _Nodes Names and Tags"), "<control>T", _("Find in Nodes Names and Tags"), dad.find_a_node),
("find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), dad.replace_in_nodes_names),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("Nodes _Import"), cons.STR_STOCK_CT_IMP, None, None),
("from_cherrytree", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), dad.nodes_add_from_cherrytree_file),
("from_txt", _("From _Plain Text File"), None, _("Add Node from a Plain Text File to the Current Tree"), dad.nodes_add_from_plain_text_file),
("from_txt", _("From _Folder of Plain Text Files"), None, _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), dad.nodes_add_from_plain_text_folder),
("from_html", _("From _HTML File"), None, _("Add Node from an HTML File to the Current Tree"), dad.nodes_add_from_html_file),
("from_html", _("From _Folder of HTML Files"), None, _("Add Nodes from a Folder of HTML Files to the Current Tree"), dad.nodes_add_from_html_folder),
(cons.STR_STOCK_CT_IMP, _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), dad.nodes_add_from_basket_folder),
(cons.STR_STOCK_CT_IMP, _("From _Gnote Folder"), None, _("Add Nodes of a Gnote Folder to the Current Tree"), dad.nodes_add_from_gnote_folder),
(cons.STR_STOCK_CT_IMP, _("From _EssentialPIM HTML File"), None, _("Add Node from an EssentialPIM HTML File to the Current Tree"), dad.nodes_add_from_epim_html_file),
(cons.STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), dad.nodes_add_from_keepnote_folder),
(cons.STR_STOCK_CT_IMP, _("From K_eyNote File"), None, _("Add Nodes of a KeyNote File to the Current Tree"), dad.nodes_add_from_keynote_file),
(cons.STR_STOCK_CT_IMP, _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), dad.nodes_add_from_knowit_file),
(cons.STR_STOCK_CT_IMP, _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), dad.nodes_add_from_leo_file),
(cons.STR_STOCK_CT_IMP, _("From _Mempad File"), None, _("Add Nodes of a Mempad File to the Current Tree"), dad.nodes_add_from_mempad_file),
(cons.STR_STOCK_CT_IMP, _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), dad.nodes_add_from_notecase_file),
(cons.STR_STOCK_CT_IMP, _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), dad.nodes_add_from_tomboy_folder),
(cons.STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), dad.nodes_add_from_treepad_file),
(cons.STR_STOCK_CT_IMP, _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), dad.nodes_add_from_tuxcards_file),
(cons.STR_STOCK_CT_IMP, _("From _Zim Folder"), None, _("Add Nodes of a Zim Folder to the Current Tree"), dad.nodes_add_from_zim_folder),
("submenu-end", None, None, None, None),
("submenu-start", _("Nodes E_xport"), "export_from_cherrytree", None, None),
("to_pdf", _("Export To _PDF"), None, _("Export To PDF"), dad.export_to_pdf),
("to_txt", _("Export to Multiple Plain _Text Files"), None, _("Export to Multiple Plain Text Files"), dad.export_to_txt_multiple),
("to_txt", _("Export to _Single Plain Text File"), None, _("Export to Single Plain Text File"), dad.export_to_txt_single),
("to_html", _("Export To _HTML"), None, _("Export To HTML"), dad.export_to_html),
("to_cherrytree", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), dad.export_to_ctd),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"), dad.node_inherit_syntax),
(cons.TAG_SEPARATOR, None, None, None, None),
("edit-delete", _("De_lete Node"), "Delete", _("Delete the Selected Node"), dad.node_delete),
(cons.TAG_SEPARATOR, None, None, None, None),
("gtk-go-back", _("Go _Back"), "<alt>Left", _("Go to the Previous Visited Node"), dad.go_back),
("gtk-go-forward", _("Go _Forward"), "<alt>Right", _("Go to the Next Visited Node"), dad.go_forward),
]

def get_popup_menu_entries_text(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
("edit-cut", _("Cu_t as Plain Text"), "<control><shift>X", _("Cut as Plain Text, Discard the Rich Text Formatting"), dad.cut_as_plain_text),
("edit-copy", _("_Copy as Plain Text"), "<control><shift>C", _("Copy as Plain Text, Discard the Rich Text Formatting"), dad.copy_as_plain_text),
("edit-paste", _("_Paste as Plain Text"), "<control><shift>V", _("Paste as Plain Text, Discard the Rich Text Formatting"), dad.paste_as_plain_text),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("For_matting") , "format_text", None, None),
("format_text_latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), dad.apply_tag_latest),
("format_text_clear", _("_Remove Formatting"), "<control><shift>R", _("Remove the Formatting from the Selected Text"), dad.remove_text_formatting),
(cons.TAG_SEPARATOR, None, None, None, None),
("color_foreground", _("Text _Color Foreground"), None, _("Change the Color of the Selected Text Foreground"), dad.apply_tag_foreground),
("color_background", _("Text C_olor Background"), None, _("Change the Color of the Selected Text Background"), dad.apply_tag_background),
("format-text-bold", _("Toggle _Bold Property"), "<control>B", _("Toggle Bold Property of the Selected Text"), dad.apply_tag_bold),
("format-text-italic", _("Toggle _Italic Property"), "<control>I", _("Toggle Italic Property of the Selected Text"), dad.apply_tag_italic),
("format-text-underline", _("Toggle _Underline Property"), "<control>U", _("Toggle Underline Property of the Selected Text"), dad.apply_tag_underline),
("format-text-strikethrough", _("Toggle Stri_kethrough Property"), "<control>E", _("Toggle Strikethrough Property of the Selected Text"), dad.apply_tag_strikethrough),
("format-text-large", _("Toggle h_1 Property"), "<control>1", _("Toggle h1 Property of the Selected Text"), dad.apply_tag_h1),
("format-text-large2", _("Toggle h_2 Property"), "<control>2", _("Toggle h2 Property of the Selected Text"), dad.apply_tag_h2),
("format-text-large3", _("Toggle h_3 Property"), "<control>3", _("Toggle h3 Property of the Selected Text"), dad.apply_tag_h3),
("format-text-small", _("Toggle _Small Property"), "<control>0", _("Toggle Small Property of the Selected Text"), dad.apply_tag_small),
("format-text-superscript", _("Toggle Su_perscript Property"), None, _("Toggle Superscript Property of the Selected Text"), dad.apply_tag_superscript),
("format-text-subscript", _("Toggle Su_bscript Property"), None, _("Toggle Subscript Property of the Selected Text"), dad.apply_tag_subscript),
("format-text-monospace", _("Toggle _Monospace Property"), "<control>M", _("Toggle Monospace Property of the Selected Text"), dad.apply_tag_monospace),
("submenu-end", None, None, None, None),
("submenu-start", _("_Justify") , "gtk-justify-center", None, None),
("gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), dad.apply_tag_justify_left),
("gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), dad.apply_tag_justify_center),
("gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), dad.apply_tag_justify_right),
("gtk-justify-fill", _("Justify _Fill"), None, _("Justify Fill the Current Paragraph"), dad.apply_tag_justify_fill),
("submenu-end", None, None, None, None),
("submenu-start", _("_List") , "list_bulleted", None, None),
("list_bulleted", _("Set/Unset _Bulleted List"), None, _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), dad.list_bulleted_handler),
("list_numbered", _("Set/Unset _Numbered List"), None, _("Set/Unset the Current Paragraph/Selection as a Numbered List"), dad.list_numbered_handler),
("list_todo", _("Set/Unset _To-Do List"), None, _("Set/Unset the Current Paragraph/Selection as a To-Do List"), dad.list_todo_handler),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("_Insert") , "insert", None, None),
("image_insert", _("Insert I_mage"), "<control><alt>I", _("Insert an Image"), dad.image_handle),
("table_insert", _("Insert _Table"), "<control><alt>T", _("Insert a Table"), dad.table_handle),
("codebox_insert", _("Insert _CodeBox"), "<control><alt>C", _("Insert a CodeBox"), dad.codebox_handle),
("file_icon", _("Insert _File"), "<control><alt>E", _("Insert File"), dad.embfile_insert),
("link_handle", _("Insert/Edit _Link"), "<control>L", _("Insert a Link/Edit the Underlying Link"), dad.apply_tag_link),
("anchor_insert", _("Insert _Anchor"), "<control><alt>A", _("Insert an Anchor"), dad.anchor_handle),
("index", _("Insert T_OC"), None, _("Insert Table of Contents"), dad.toc_insert),
("timestamp", _("Insert Ti_mestamp"), "<control><alt>M", _("Insert Timestamp"), dad.timestamp_insert),
("horizontal_rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), dad.horizontal_rule_insert),
("submenu-end", None, None, None, None),
("submenu-start", _("C_hange Case") , "case_toggle", None, None),
("case_lower", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), dad.text_selection_lower_case),
("case_upper", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), dad.text_selection_upper_case),
("case_toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), dad.text_selection_toggle_case),
("submenu-end", None, None, None, None),
("submenu-start", _("_Row") , "gtk-edit", None, None),
("edit-cut", _("Cu_t Row"), "<shift><alt>X", _("Cut the Current Row/Selected Rows"), dad.text_row_cut),
("edit-copy", _("C_opy Row"), "<shift><alt>C", _("Copy the Current Row/Selected Rows"), dad.text_row_copy),
("edit-delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), dad.text_row_delete),
("gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), dad.text_row_selection_duplicate),
("gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), dad.text_row_up),
("gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), dad.text_row_down),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("_Search") , "find", None, None),
("find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), dad.find_in_selected_node),
("find", _("Find in Node_s"), "<control><shift>F", _("Find into all the Tree Nodes"), dad.find_in_all_nodes),
("find", _("Find in _Nodes Names and Tags"), "<control>T", _("Find in Nodes Names and Tags"), dad.find_a_node),
("find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), dad.find_again),
("find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), dad.find_back),
("submenu-end", None, None, None, None),
("submenu-start", _("_Replace") , "find_replace", None, None),
("find_replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), dad.replace_in_selected_node),
("find_replace", _("Replace in Node_s"), "<control><shift>H", _("Replace into all the Tree Nodes"), dad.replace_in_all_nodes),
("find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), dad.replace_in_nodes_names),
("find_replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), dad.replace_again),
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
("submenu-start", _("_Insert") , "insert", None, None),
("timestamp", _("Insert Ti_mestamp"), "<control><alt>M", _("Insert Timestamp"), dad.timestamp_insert),
("horizontal_rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), dad.horizontal_rule_insert),
("submenu-end", None, None, None, None),
("gtk-clear", _("Stri_p Trailing Spaces"), None, _("Strip Trailing Spaces"), dad.strip_trailing_spaces),
("submenu-start", _("C_hange Case") , "case_toggle", None, None),
("case_lower", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), dad.text_selection_lower_case),
("case_upper", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), dad.text_selection_upper_case),
("case_toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), dad.text_selection_toggle_case),
("submenu-end", None, None, None, None),
("submenu-start", _("_Row") , "gtk-edit", None, None),
("edit-cut", _("Cu_t Row"), "<shift><alt>X", _("Cut the Current Row/Selected Rows"), dad.text_row_cut),
("edit-copy", _("C_opy Row"), "<shift><alt>C", _("Copy the Current Row/Selected Rows"), dad.text_row_copy),
("edit-delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), dad.text_row_delete),
("gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), dad.text_row_selection_duplicate),
("gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), dad.text_row_up),
("gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), dad.text_row_down),
("submenu-end", None, None, None, None),
(cons.TAG_SEPARATOR, None, None, None, None),
("submenu-start", _("_Search") , "find", None, None),
("find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), dad.find_in_selected_node),
("find", _("Find in Node_s"), "<control><shift>F", _("Find into all the Tree Nodes"), dad.find_in_all_nodes),
("find", _("Find in _Nodes Names and Tags"), "<control>T", _("Find in Nodes Names and Tags"), dad.find_a_node),
("find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), dad.find_again),
("find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), dad.find_back),
("submenu-end", None, None, None, None),
("submenu-start", _("_Replace") , "find_replace", None, None),
("find_replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), dad.replace_in_selected_node),
("find_replace", _("Replace in Node_s"), "<control><shift>H", _("Replace into all the Tree Nodes"), dad.replace_in_all_nodes),
("find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), dad.replace_in_nodes_names),
("find_replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), dad.replace_again),
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

def get_popup_menu_entries_table_cell(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
("insert", _("Insert _NewLine"), "<control>period", _("Insert NewLine Char"), dad.curr_table_cell_insert_newline),
]

def get_popup_menu_entries_codebox(dad):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
(cons.TAG_SEPARATOR, None, None, None, None),
("codebox_edit", _("Change CodeBox _Properties"), None, _("Edit the Properties of the CodeBox"), dad.codebox_change_properties),
("from_txt", _("CodeBox _Load From Text File"), None, _("Load the CodeBox Content From a Text File"), dad.codebox_load_from_file),
("to_txt", _("CodeBox _Save To Text File"), None, _("Save the CodeBox Content To a Text File"), dad.codebox_save_to_file),
(cons.TAG_SEPARATOR, None, None, None, None),
("edit-cut", _("C_ut CodeBox"), None, _("Cut the Selected CodeBox"), dad.codebox_cut),
("edit-copy", _("_Copy CodeBox"), None, _("Copy the Selected CodeBox"), dad.codebox_copy),
("edit-delete", _("_Delete CodeBox"), None, _("Delete the Selected CodeBox"), dad.codebox_delete),
("edit-delete", _("Delete CodeBox _Keep Content"), None, _("Delete the Selected CodeBox But Keep Its Content"), dad.codebox_delete_keeping_text),
(cons.TAG_SEPARATOR, None, None, None, None),
("gtk-go-forward", _("Increase CodeBox Width"), "<control>period", _("Increase the Width of the CodeBox"), dad.codebox_increase_width),
("gtk-go-back", _("Decrease CodeBox Width"), "<control><alt>period", _("Decrease the Width of the CodeBox"), dad.codebox_decrease_width),
("gtk-go-down", _("Increase CodeBox Height"), "<control>comma", _("Increase the Height of the CodeBox"), dad.codebox_increase_height),
("gtk-go-up", _("Decrease CodeBox Height"), "<control><alt>comma", _("Decrease the Height of the CodeBox"), dad.codebox_decrease_height),
]
