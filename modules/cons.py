# -*- coding: UTF-8 -*-
#
#       cons.py
#       
#       Copyright 2009-2011 Giuseppe Penone <giuspen@gmail.com>
#       
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
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

import os, sys, gtk


VERSION = "0.18.1"
APP_NAME = "cherrytree"
NEWER_VERSION_URL = "http://www.giuspen.com/software/version_cherrytree"
if sys.platform[0:3] == "win":
   CONFIG_PATH = 'config.xml'
   LANG_PATH = 'lang'
   IMG_PATH = 'glade/img_tmp.png'
   TMP_FOLDER = os.path.join(os.getcwd(), 'glade/ct_tmp/')
   GLADE_PATH = 'glade/'
   LOCALE_PATH = 'locale/'
else:
   CONFIG_PATH = os.path.join(os.path.expanduser('~'), '.config/cherrytree/config.xml')
   LANG_PATH = os.path.join(os.path.expanduser('~'), '.config/cherrytree/lang')
   IMG_PATH = os.path.join(os.path.expanduser('~'), '.config/cherrytree/img_tmp.png')
   TMP_FOLDER = '/tmp/ct_tmp/'
   if os.path.isdir('modules'):
      GLADE_PATH = 'glade/'
      LOCALE_PATH = 'locale/'
   else:
      GLADE_PATH = '/usr/share/cherrytree/glade/'
      LOCALE_PATH = '/usr/share/locale/'
AVAILABLE_LANGS = ['default', 'cs', 'de', 'en', 'es', 'fr', 'gl', 'it', 'pl', 'ru', 'uk']
SHOW_MENU_ICONS = "gconftool-2 --set /desktop/gnome/interface/menus_have_icons --type bool 1"
COLOR_WHITE = "#ffffffffffff"
COLOR_BLACK = "#000000000000"
BAD_CHARS = "[\x00-\x08\x0b-\x1f]"
TAG_PROPERTIES = ["weight", "foreground", "background", "style", "underline", "strikethrough",
                  "scale", "justification", "link"]
CUSTOM_COLORS_ID = "custom-colors"
ICONS_SIZE = {1: gtk.ICON_SIZE_MENU, 2: gtk.ICON_SIZE_SMALL_TOOLBAR, 3: gtk.ICON_SIZE_LARGE_TOOLBAR,
              4: gtk.ICON_SIZE_DND, 5: gtk.ICON_SIZE_DIALOG}
ANCHOR_CHAR = GLADE_PATH + 'anchor_char.png'
NODES_ICONS = {0:'Red Cherry', 1:'Blue Cherry', 2:'Orange Cherry', 3:'Cyan Cherry',
               4:'Orange Dark Cherry', 5:'Sherbert Cherry', 6:'Yellow Cherry'}
CODE_ICONS = {"python": 'Green Cherry', "sh":'Purple Cherry',
              "c":'Black Cherry', "cpp":'Black Cherry', "chdr":'Black Cherry'}

WHITE_SPACE_BETW_PIXB_AND_TEXT = 3
GRID_SLIP_OFFSET = 3
MAIN_WIN_TO_TEXT_WIN_NORMALIZER = 50
MAX_NODES_STATES_NUM = 30

CHAR_SPACE = " "
CHAR_NEWLINE = "\n"
CHAR_CR = "\r"
CHAR_TAB = "\t"
CHAR_LISTBUL = "•"
CHAR_TILDE = "~"

HTML_HEADER = '<!doctype html><html><head><meta http-equiv="content-type" content="text/html; charset=utf-8"><title>%s</title><meta name="generator" content="CherryTree"></head><body>'
HTML_FOOTER = '</body></html>'

STOCKS_N_FILES = {'Node Bullet':'node_bullet.png',
                  'Node NoIcon': 'node_no_icon.png',
                  'Black Cherry':'cherry_black.png',
                  'Blue Cherry':'cherry_blue.png',
                  'Cyan Cherry':'cherry_cyan.png',
                  'Green Cherry':'cherry_green.png',
                  'Gray Cherry':'cherry_gray.png',
                  'Orange Cherry':'cherry_orange.png',
                  'Orange Dark Cherry':'cherry_orange_dark.png',
                  'Purple Cherry':'cherry_purple.png',
                  'Red Cherry':'cherry_red.png',
                  'Sherbert Cherry':'cherry_sherbert.png',
                  'Yellow Cherry':'cherry_yellow.png',
                  'Insert Image':'image_insert.png',
                  'Edit Image':'image_edit.png',
                  'Save Image':'image_save.png',
                  'Insert Table':'table_insert.png',
                  'Edit Table':'table_edit.png',
                  'Save Table':'table_save.png',
                  'Insert CodeBox':'codebox_insert.png',
                  'Edit CodeBox':'codebox_edit.png',
                  'Insert Anchor':'anchor_insert.png',
                  'Edit Anchor':'anchor_edit.png',
                  'Anchor':'anchor.png',
                  'Insert':'insert.png',
                  'Handle Link':'link_handle.png',
                  'WebSite Link':'link_website.png',
                  'Edit Node':'cherry_edit.png',
                  'Bulleted List':'list_bulleted.png',
                  'Numbered List':'list_numbered.png',
                  'Node Name Header':'node_name_header.png',
                  'Delete':'edit-delete.png',
                  'Copy':'edit-copy.png',
                  'Cut':'edit-cut.png',
                  'Find':'find.png',
                  'Find Again':'find_again.png',
                  'Find Back':'find_back.png',
                  'Find Replace':'find_replace.png',
                  'Change Color Background':'color_background.png',
                  'Change Color Foreground':'color_foreground.png',
                  'Format Text Large':'format-text-large.png',
                  'Format Text Large2':'format-text-large2.png',
                  'Format Text Small':'format-text-small.png',
                  'Format Text Latest':'format_text_latest.png',
                  'Format Text':'format_text.png',
                  'Rotate Left':'object-rotate-left.png',
                  'Rotate Right':'object-rotate-right.png',
                  'To Txt':'to_txt.png',
                  'To HTML':'to_html.png',
                  'CherryTree Export':'export_from_cherrytree.png',
                  'CherryTree Import':'import_in_cherrytree.png',
                  'CherryTree':'cherrytree.png',
                  'Quit App':'quit-app.png',
                  'New Instance':'new-instance.png',
                  'Toolbar':'toolbar.png',
                  'Tree':'cherries.png',
                  'Help Contents':'help-contents.png',
                  'Index':'index.png',
                  'Horizontal Rule':'horizontal_rule.png'}

NODES_STOCKS = ['Node Bullet', 'Node NoIcon', 'Black Cherry',
                'Blue Cherry', 'Cyan Cherry', 'Green Cherry',
                'Gray Cherry', 'Orange Cherry', 'Orange Dark Cherry',
                'Purple Cherry', 'Red Cherry', 'Sherbert Cherry',
                'Yellow Cherry']

HORIZONTAL_RULE = "\n=============================================\n"

UI_INFO = """
<ui>
   <menubar name='MenuBar'>
      <menu action='FileMenu'>
         <menuitem action='NewInstance'/>
         <menuitem action='OpenFile'/>
         <separator/>
         <menuitem action='CTDProtection'/>
         <separator/>
         <menuitem action='Save'/>
         <menuitem action='SaveAs'/>
         <separator/>
         <menuitem action='PageSetup'/>
         <menuitem action='NodePrint'/>
         <separator/>
         <menuitem action='QuitApp'/>
         <menuitem action='ExitApp'/>
      </menu>
      
      <menu action='EditMenu'>
         <menuitem action='Preferences'/>
         <separator/>
         <menuitem action='HandleImage'/>
         <menuitem action='HandleTable'/>
         <menuitem action='HandleCodeBox'/>
         <menuitem action='HandleLink'/>
         <menuitem action='HandleAnchor'/>
         <menuitem action='InsertTOC'/>
         <menuitem action='HorizontalRule'/>
         <separator/>
         <menuitem action='DuplicateRow'/>
         <menuitem action='DeleteRow'/>
         <separator/>
         <menuitem action='BulletedList'/>
         <menuitem action='NumberedList'/>
         <separator/>
         <menuitem action='Undo'/>
         <menuitem action='Redo'/>
      </menu>
      
      <menu action='FormattingMenu'>
         <menuitem action='FormatLatest'/>
         <menuitem action='RemoveFormatting'/>
         <separator/>
         <menuitem action='ColorForeground'/>
         <menuitem action='ColorBackground'/>
         <menuitem action='Bold'/>
         <menuitem action='Italic'/>
         <menuitem action='Underline'/>
         <menuitem action='Strikethrough'/>
         <menuitem action='H1'/>
         <menuitem action='H2'/>
         <menuitem action='Small'/>
         <separator/>
         <menuitem action='JustifyLeft'/>
         <menuitem action='JustifyCenter'/>
         <menuitem action='JustifyRight'/>
      </menu>
      
      <menu action='TreeMenu'>
         <menuitem action='AddNode'/>
         <menuitem action='AddSubNode'/>
         <separator/>
         <menuitem action='ChangeNodeProp'/>
         <menuitem action='TreeInfo'/>
         <separator/>
         <menuitem action='NodeUp'/>
         <menuitem action='NodeDown'/>
         <menuitem action='NodeLeft'/>
         <menuitem action='NodeChangeFather'/>
         <separator/>
         <menuitem action='NodesExpandAll'/>
         <menuitem action='NodesCollapseAll'/>
         <separator/>
         <menuitem action='SortSiblingsDescending'/>
         <menuitem action='SortSiblingsAscending'/>
         <separator/>
         <menuitem action='InheritSyntax'/>
         <separator/>
         <menuitem action='DeleteNode'/>
      </menu>
      
      <menu action='SearchMenu'>
         <menuitem action='FindInNode'/>
         <menuitem action='FindInNodes'/>
         <menuitem action='FindNode'/>
         <menuitem action='FindAgain'/>
         <menuitem action='FindBack'/>
      </menu>
      
      <menu action='ReplaceMenu'>
         <menuitem action='ReplaceInNode'/>
         <menuitem action='ReplaceInNodes'/>
         <menuitem action='ReplaceInNodesNames'/>
         <menuitem action='ReplaceAgain'/>
      </menu>
      
      <menu action='ViewMenu'>
         <menuitem action='ShowHideNodeNameHeader'/>
         <menuitem action='ShowHideTree'/>
         <menuitem action='ShowHideToolbar'/>
         <menuitem action='IncreaseToolbarIconsSize'/>
         <menuitem action='DecreaseToolbarIconsSize'/>
      </menu>
      
      <menu action='ImportMenu'>
         <menuitem action='FromBasket'/>
         <menuitem action='FromCherryTree'/>
         <menuitem action='FromKeepNote'/>
         <menuitem action='FromNoteCase'/>
         <menuitem action='FromTreepad'/>
         <menuitem action='FromTuxCards'/>
      </menu>
      
      <menu action='ExportMenu'>
         <menuitem action='NodePrint'/>
         <menuitem action='SelNode2Txt'/>
         <menuitem action='SelNode2HTML'/>
         <menuitem action='AllNodes2HTML'/>
         <menuitem action='SelNode2CTD'/>
      </menu>
      
      <menu action='HelpMenu'>
         <menuitem action='CheckNewer'/>
         <separator/>
         <menuitem action='Help'/>
         <separator/>
         <menuitem action='About'/>
      </menu>
   </menubar>

   <toolbar name='ToolBar'>
      <toolitem action='ShowHideTree'/>
      <separator/>
      <toolitem action='NewInstance'/>
      <toolitem action='OpenFile'/>
      <toolitem action='Save'/>
      <separator/>
      <toolitem action='Undo'/>
      <toolitem action='Redo'/>
      <separator/>
      <toolitem action='JustifyLeft'/>
      <toolitem action='JustifyCenter'/>
      <toolitem action='JustifyRight'/>
      <separator/>
      <toolitem action='BulletedList'/>
      <toolitem action='NumberedList'/>
      <separator/>
      <toolitem action='HandleImage'/>
      <toolitem action='HandleTable'/>
      <toolitem action='HandleCodeBox'/>
      <toolitem action='HandleLink'/>
      <toolitem action='HandleAnchor'/>
      <separator/>
      <toolitem action='ColorForeground'/>
      <toolitem action='ColorBackground'/>
      <toolitem action='Bold'/>
      <toolitem action='Italic'/>
      <toolitem action='Underline'/>
      <toolitem action='Strikethrough'/>
      <toolitem action='H1'/>
      <toolitem action='H2'/>
      <toolitem action='Small'/>
      <toolitem action='FormatLatest'/>
      <separator/>
      <toolitem action='FindAgain'/>
      <toolitem action='FindBack'/>
      <toolitem action='ReplaceAgain'/>
      <separator/>
      <toolitem action='QuitApp'/>
   </toolbar>
   
   <popup name='TreeMenu'>
      <menuitem action='AddNode'/>
      <menuitem action='AddSubNode'/>
      <separator/>
      <menuitem action='ChangeNodeProp'/>
      <menuitem action='TreeInfo'/>
      <separator/>
      <menu action='TreeMoveMenu'>
         <menuitem action='NodeUp'/>
         <menuitem action='NodeDown'/>
         <menuitem action='NodeLeft'/>
         <menuitem action='NodeChangeFather'/>
      </menu>
      <separator/>
      <menuitem action='NodesExpandAll'/>
      <menuitem action='NodesCollapseAll'/>
      <separator/>
      <menuitem action='FindNode'/>
      <menuitem action='ReplaceInNodesNames'/>
      <separator/>
      <menu action='TreeImportMenu'>
         <menuitem action='FromBasket'/>
         <menuitem action='FromCherryTree'/>
         <menuitem action='FromKeepNote'/>
         <menuitem action='FromNoteCase'/>
         <menuitem action='FromTreepad'/>
         <menuitem action='FromTuxCards'/>
      </menu>
      <menu action='TreeExportMenu'>
         <menuitem action='NodePrint'/>
         <menuitem action='SelNode2Txt'/>
         <menuitem action='SelNode2HTML'/>
         <menuitem action='AllNodes2HTML'/>
         <menuitem action='SelNode2CTD'/>
      </menu>
      <separator/>
      <menuitem action='SortSiblingsDescending'/>
      <menuitem action='SortSiblingsAscending'/>
      <separator/>
      <menuitem action='InheritSyntax'/>
      <separator/>
      <menuitem action='DeleteNode'/>
   </popup>
   
   <popup name='SysTrayMenu'>
      <menuitem action='ExitApp'/>
   </popup>
   
   <popup name='ImageMenu'>
      <menuitem action='CutImage'/>
      <menuitem action='CopyImage'/>
      <menuitem action='DeleteImage'/>
      <separator/>
      <menuitem action='EditImage'/>
      <menuitem action='SaveImage'/>
   </popup>
   
   <popup name='AnchorMenu'>
      <menuitem action='EditAnchor'/>
   </popup>
   
   <popup name='TableMenu'>
      <menuitem action='CutTable'/>
      <menuitem action='CopyTable'/>
      <menuitem action='DeleteTable'/>
      <separator/>
      <menuitem action='TableRowAdd'/>
      <separator/>
      <menuitem action='TableRowUp'/>
      <menuitem action='TableRowDown'/>
      <menuitem action='TableSortRowsDesc'/>
      <menuitem action='TableSortRowsAsc'/>
      <separator/>
      <menuitem action='TableRowDelete'/>
      <separator/>
      <menuitem action='TableEditProp'/>
      <menuitem action='TableExport'/>
      <separator/>
   </popup>
</ui>
"""

def get_entries(inst):
   """Returns the Menu Entries Given the Class Instance"""
   return [
   # name, stock id, label
   ( "FileMenu", None, _("_File") ),
   ( "EditMenu", None, _("_Edit") ),
   ( "FormattingMenu", None, _("For_matting") ),
   ( "TreeMenu", None, _("_Tree") ),
   ( "TreeMoveMenu", "gtk-jump-to", _("Node _Move") ),
   ( "TreeImportMenu", "CherryTree Import", _("Nodes _Import") ),
   ( "TreeExportMenu", "CherryTree Export", _("Nodes E_xport") ),
   ( "SearchMenu", None, _("_Search") ),
   ( "ReplaceMenu", None, _("_Replace") ),
   ( "ViewMenu", None, _("_View") ),
   ( "ImportMenu", None, _("_Import") ),
   ( "ExportMenu", None, _("E_xport") ),
   ( "HelpMenu", None, _("_Help") ),
   # name, stock id, label, accelerator, tooltip, callback
   ( "NewInstance", "New Instance", _("New _Instance"), None, _("Start a New Instance of CherryTree"), inst.file_new),
   ( "OpenFile", "gtk-open", _("_Open File"), "<control>O", _("Open a .ct* File"), inst.file_open),
   ( "CTDProtection", "gtk-dialog-authentication", _("Set _Document Protection"), None, _("Set Protection of the Current Document"), inst.dialog_edit_protection),
   ( "Save", "gtk-save", _("_Save"), "<control>S", _("Save File"), inst.file_save),
   ( "SaveAs", "gtk-save-as", _("Save _As"), "<control><shift>S", _("Save File As"), inst.file_save_as),
   ( "AllNodes2HTML", "To HTML", _("Export _Tree To HTML"), None, _("Export the Tree To HTML"), inst.nodes_all_export_to_html),
   ( "SelNode2HTML", "To HTML", _("Export Node To _HTML"), None, _("Export the Selected Node To HTML"), inst.node_export_to_html),
   ( "SelNode2Txt", "To Txt", _("Save Node as Plain _Text"), None, _("Save the Selected Node as Plain Text"), inst.node_export_to_plain_text),
   ( "SelNode2CTD", "CherryTree Export", _("_Export Node and Subnodes"), None, _("Export the Selected Node and its Subnodes"), inst.node_and_subnodes_export),
   ( "PageSetup", "gtk-print", _("Pa_ge Setup"), None, _("Set up the Page for Printing"), inst.node_print_page_setup),
   ( "NodePrint", "gtk-print", _("Node _Print / Save as PDF, PS, SVG"), None, _("Print the Current Node / Save the Current Node as PDF, PS, SVG"), inst.node_print),
   ( "QuitApp", "Quit App", _("_Quit"), "<control>Q", _("Quit the Application"), inst.quit_application),
   ( "ExitApp", "Quit App", _("_Exit CherryTree"), "<control><shift>Q", _("Exit from CherryTree"), inst.quit_application_totally),
   ( "Preferences", "gtk-preferences", _("_Preferences"), "<control>P", _("Preferences"), inst.dialog_preferences),
   ( "HorizontalRule", "Horizontal Rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), inst.horizontal_rule_insert),
   ( "BulletedList", "Bulleted List", _("Set/Unset _Bulleted List"), None, _("Set/Unset the Current Paragraph as a Bulleted List Element"), inst.list_bulleted_handler),
   ( "NumberedList", "Numbered List", _("Set/Unset _Numbered List"), None, _("Set/Unset the Current Paragraph as a Numbered List Element"), inst.list_numbered_handler),
   ( "HandleLink", "Handle Link", _("Insert/Edit _Link"), "<control>L", _("Insert a Link/Edit the Underlying Link"), inst.apply_tag_link),
   ( "InsertTOC", "Index", _("Insert T_OC"), None, _("Insert Table of Contents"), inst.toc_insert),
   ( "HandleAnchor", "Insert Anchor", _("Insert _Anchor"), None, _("Insert an Anchor"), inst.anchor_handle),
   ( "EditAnchor", "Edit Anchor", _("Edit _Anchor"), None, _("Edit the Underlying Anchor"), inst.anchor_edit),
   ( "HandleImage", "Insert Image", _("Insert I_mage"), None, _("Insert an Image"), inst.image_handle),
   ( "SaveImage", "Save Image", _("_Save Image as PNG"), None, _("Save the Selected Image as a PNG file"), inst.image_save),
   ( "EditImage", "Edit Image", _("_Edit Image"), None, _("Edit the Selected Image"), inst.image_edit),
   ( "CutImage", "Cut", _("C_ut Image"), None, _("Cut the Selected Image"), inst.image_cut),
   ( "CopyImage", "Copy", _("_Copy Image"), None, _("Copy the Selected Image"), inst.image_copy),
   ( "DeleteImage", "Delete", _("_Delete Image"), None, _("Delete the Selected Image"), inst.image_delete),
   ( "CutTable", "Cut", _("C_ut Table"), None, _("Cut the Selected Table"), inst.tables_handler.table_cut),
   ( "CopyTable", "Copy", _("_Copy Table"), None, _("Copy the Selected Table"), inst.tables_handler.table_copy),
   ( "DeleteTable", "Delete", _("_Delete Table"), None, _("Delete the Selected Table"), inst.tables_handler.table_delete),
   ( "HandleTable", "Insert Table", _("Insert _Table"), None, _("Insert a Table"), inst.table_handle),
   ( "HandleCodeBox", "Insert CodeBox", _("Insert _CodeBox"), None, _("Insert a CodeBox"), inst.codebox_handle),
   ( "DuplicateRow", "gtk-add", _("_Duplicate Row/Selection"), "<control>D", _("Duplicate the Whole Row/a Selection"), inst.text_row_selection_duplicate),
   ( "DeleteRow", "gtk-clear", _("De_lete Row"), "<control><shift>D", _("Delete the Whole Row"), inst.text_row_delete),
   ( "Undo", "gtk-undo", _("_Undo"), "<control>Z", _("Undo Last Operation"), inst.requested_step_back),
   ( "Redo", "gtk-redo", _("_Redo"), "<control>Y", _("Redo Previously Discarded Operation"), inst.requested_step_ahead),
   ( "InheritSyntax", "gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"), inst.node_inherit_syntax),
   ( "FormatLatest", "Format Text Latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), inst.apply_tag_latest),
   ( "RemoveFormatting", "gtk-clear", _("_Remove Formatting"), None, _("Remove the Formatting from the Selected Text"), inst.remove_text_formatting),
   ( "ColorForeground", "Change Color Foreground", _("Text _Color Foreground"), None, _("Change the Color of the Selected Text Foreground"), inst.apply_tag_foreground),
   ( "ColorBackground", "Change Color Background", _("Text C_olor Background"), None, _("Change the Color of the Selected Text Background"), inst.apply_tag_background),
   ( "Bold", "gtk-bold", _("Toggle _Bold Property"), "<control>B", _("Toggle Bold Property of the Selected Text"), inst.apply_tag_bold),
   ( "Italic", "gtk-italic", _("Toggle _Italic Property"), "<control>I", _("Toggle Italic Property of the Selected Text"), inst.apply_tag_italic),
   ( "Underline", "gtk-underline", _("Toggle _Underline Property"), "<control>U", _("Toggle Underline Property of the Selected Text"), inst.apply_tag_underline),
   ( "Strikethrough", "gtk-strikethrough", _("Toggle Stri_kethrough Property"), "<control>K", _("Toggle Strikethrough Property of the Selected Text"), inst.apply_tag_strikethrough),
   ( "H1", "Format Text Large", _("Toggle h_1 Property"), "<control>1", _("Toggle h1 Property of the Selected Text"), inst.apply_tag_large),
   ( "H2", "Format Text Large2", _("Toggle h_2 Property"), "<control>2", _("Toggle h2 Property of the Selected Text"), inst.apply_tag_large2),
   ( "Small", "Format Text Small", _("Toggle _Small Property"), "<control><shift>G", _("Toggle Small Property of the Selected Text"), inst.apply_tag_small),
   ( "JustifyLeft", "gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), inst.apply_tag_justify_left),
   ( "JustifyCenter", "gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), inst.apply_tag_justify_center),
   ( "JustifyRight", "gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), inst.apply_tag_justify_right),
   ( "FindInNode", "Find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), inst.find_in_selected_node),
   ( "FindInNodes", "Find", _("Find in Node_s"), "<control><shift>F", _("Find into all the Tree Nodes"), inst.find_in_all_nodes),
   ( "FindNode", "Find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
   ( "FindAgain", "Find Again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), inst.find_again),
   ( "FindBack", "Find Back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), inst.find_back),
   ( "ReplaceInNode", "Find Replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), inst.replace_in_selected_node),
   ( "ReplaceInNodes", "Find Replace", _("Replace in Node_s"), "<control><shift>H", _("Replace into all the Tree Nodes"), inst.replace_in_all_nodes),
   ( "ReplaceInNodesNames", "Find Replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
   ( "ReplaceAgain", "Find Replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), inst.replace_again),
   ( "ShowHideNodeNameHeader", "Node Name Header", _("Show/Hide Node Name _Header"), None, _("Toggle Show/Hide Node Name Header"), inst.toggle_show_hide_node_name_header),
   ( "ShowHideTree", "Tree", _("Show/Hide _Tree"), "F9", _("Toggle Show/Hide Tree"), inst.toggle_show_hide_tree),
   ( "ShowHideToolbar", "Toolbar", _("Show/Hide Tool_bar"), None, _("Toggle Show/Hide Toolbar"), inst.toggle_show_hide_toolbar),
   ( "IncreaseToolbarIconsSize", "gtk-add", _("_Increase Toolbar Icons Size"), None, _("Increase the Size of the Toolbar Icons"), inst.toolbar_icons_size_increase),
   ( "DecreaseToolbarIconsSize", "gtk-remove", _("_Decrease Toolbar Icons Size"), None, _("Decrease the Size of the Toolbar Icons"), inst.toolbar_icons_size_decrease),
   ( "FromBasket", "CherryTree Import", _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), inst.nodes_add_from_basket_folder),
   ( "FromCherryTree", "CherryTree Import", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), inst.nodes_add_from_cherrytree_file),
   ( "FromNoteCase", "CherryTree Import", _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), inst.nodes_add_from_notecase_file),
   ( "FromKeepNote", "CherryTree Import", _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), inst.nodes_add_from_keepnote_folder),
   ( "FromTuxCards", "CherryTree Import", _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), inst.nodes_add_from_tuxcards_file),
   ( "FromTreepad", "CherryTree Import", _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), inst.nodes_add_from_treepad_file),
   ( "Help", "Help Contents", _("_Help"), None, _("Application's Home Page"), inst.on_help_menu_item_activated),
   ( "CheckNewer", "gtk-network", _("_Check Newer Version"), None, _("Check for a Newer Version"), inst.check_for_newer_version),
   ( "About", "gtk-about", _("_About"), None, _("About CherryTree"), inst.dialog_about),
   ( "TreeInfo", "gtk-info", _("Tree _Info"), None, _("Tree Summary Information"), inst.tree_info),
   ( "AddNode", "gtk-add", _("Add _Node"), "<control>N", _("Add a Node having the same Father of the Selected Node"), inst.node_add),
   ( "AddSubNode", "gtk-add", _("Add _SubNode"), "<control><shift>N", _("Add a Child Node to the Selected Node"), inst.node_child_add),
   ( "ChangeNodeProp", "Edit Node", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), inst.node_edit),
   ( "NodeUp", "gtk-go-up", _("Node _Up"), "<alt>Up", _("Move the Selected Node Up"), inst.node_up),
   ( "NodeDown", "gtk-go-down", _("Node _Down"), "<alt>Down", _("Move the Selected Node Down"), inst.node_down),
   ( "NodeLeft", "gtk-go-back", _("Node _Left"), "<alt>Left", _("Move the Selected Node Left"), inst.node_left),
   ( "NodeChangeFather", "gtk-jump-to", _("Node Change _Father"), "<alt>Right", _("Change the Selected Node's Father"), inst.node_change_father),
   ( "NodesExpandAll", "gtk-zoom-in", _("E_xpand All Nodes"), None, _("Expand All the Tree Nodes"), inst.nodes_expand_all),
   ( "NodesCollapseAll", "gtk-zoom-out", _("_Collapse All Nodes"), None, _("Collapse All the Tree Nodes"), inst.nodes_collapse_all),
   ( "SortSiblingsDescending", "gtk-sort-descending", _("Sort Siblings _Descending"), None, _("Sort all the Siblings of the Selected Node Descending"), inst.node_siblings_sort_descending),
   ( "SortSiblingsAscending", "gtk-sort-ascending", _("Sort Siblings _Ascending"), None, _("Sort all the Siblings of the Selected Node Ascending"), inst.node_siblings_sort_ascending),
   ( "DeleteNode", "gtk-clear", _("De_lete Node"), "<alt>Delete", _("Delete the Selected Node"), inst.node_delete),
   ( "TableEditProp", "Edit Table", _("_Edit Table Properties"), None, _("Edit the Table Properties"), inst.tables_handler.table_edit_properties),
   ( "TableRowAdd", "gtk-add", _("_Add Row"), None, _("Add a Table Row"), inst.tables_handler.table_row_add),
   ( "TableRowDelete", "gtk-clear", _("De_lete Row"), None, _("Delete the Selected Table Row"), inst.tables_handler.table_row_delete),
   ( "TableRowUp", "gtk-go-up", _("Move Row _Up"), None, _("Move the Selected Row Up"), inst.tables_handler.table_row_up),
   ( "TableRowDown", "gtk-go-down", _("Move Row _Down"), None, _("Move the Selected Row Down"), inst.tables_handler.table_row_down),
   ( "TableSortRowsDesc", "gtk-sort-descending", _("Sort Rows De_scending"), None, _("Sort all the Rows Descending"), inst.tables_handler.table_rows_sort_descending),
   ( "TableSortRowsAsc", "gtk-sort-ascending", _("Sort Rows As_cending"), None, _("Sort all the Rows Ascending"), inst.tables_handler.table_rows_sort_ascending),
   ( "TableExport", "Save Table", _("_Table Export"), None, _("Export Table as CSV File"), inst.tables_handler.table_export),
   ]

def get_popup_menu_entries_text(inst):
   """Returns the Menu Entries Given the Class Instance"""
   # stock id, label, tooltip, callback | "separator", None, None, None |
   # "submenu-start", label, stock id, None | "submenu-end", None, None, None
   return [
   ("separator", None, None, None),
   ("submenu-start", _("For_matting") , "Format Text", None),
   ("Format Text Latest", _("Format _Latest"), _("Memory of Latest Text Format Type"), inst.apply_tag_latest),
   ("gtk-clear", _("_Remove Formatting"), _("Remove the Formatting from the Selected Text"), inst.remove_text_formatting),
   ("separator", None, None, None),
   ("Change Color Foreground", _("Text _Color Foreground"), _("Change the Color of the Selected Text Foreground"), inst.apply_tag_foreground),
   ("Change Color Background", _("Text C_olor Background"), _("Change the Color of the Selected Text Background"), inst.apply_tag_background),
   ("gtk-bold", _("Toggle _Bold Property"), _("Toggle Bold Property of the Selected Text"), inst.apply_tag_bold),
   ("gtk-italic", _("Toggle _Italic Property"), _("Toggle Italic Property of the Selected Text"), inst.apply_tag_italic),
   ("gtk-underline", _("Toggle _Underline Property"), _("Toggle Underline Property of the Selected Text"), inst.apply_tag_underline),
   ("gtk-strikethrough", _("Toggle Stri_kethrough Property"), _("Toggle Strikethrough Property of the Selected Text"), inst.apply_tag_strikethrough),
   ("Format Text Large", _("Toggle h_1 Property"), _("Toggle h1 Property of the Selected Text"), inst.apply_tag_large),
   ("Format Text Large2", _("Toggle h_2 Property"), _("Toggle h2 Property of the Selected Text"), inst.apply_tag_large2),
   ("Format Text Small", _("Toggle _Small Property"), _("Toggle Small Property of the Selected Text"), inst.apply_tag_small),
   ("submenu-end", None, None, None),
   ("separator", None, None, None),
   ("submenu-start", _("_Insert") , "Insert", None),
   ("Insert Image", _("Insert I_mage"), _("Insert an Image"), inst.image_handle),
   ("Insert Table", _("Insert _Table"), _("Insert a Table"), inst.table_handle),
   ("Insert CodeBox", _("Insert _CodeBox"), _("Insert a CodeBox"), inst.codebox_handle),
   ("Handle Link", _("Insert/Edit _Link"), _("Insert a Link/Edit the Underlying Link"), inst.apply_tag_link),
   ("Insert Anchor", _("Insert _Anchor"), _("Insert an Anchor"), inst.anchor_handle),
   ("Index", _("Insert T_OC"), _("Insert Table of Contents"), inst.toc_insert),
   ("Horizontal Rule", _("Insert _Horizontal Rule"), _("Insert Horizontal Rule"), inst.horizontal_rule_insert),
   ("submenu-end", None, None, None),
   ("gtk-add", _("_Duplicate Row/Selection"), _("Duplicate the Whole Row/a Selection"), inst.text_row_selection_duplicate),
   ("gtk-clear", _("De_lete Row"), _("Delete the Whole Row"), inst.text_row_delete),
   ("separator", None, None, None),
   ("submenu-start", _("_Search") , "Find", None),
   ("Find", _("_Find in Node"), _("Find into the Selected Node"), inst.find_in_selected_node),
   ("Find", _("Find in Node_s"), _("Find into all the Tree Nodes"), inst.find_in_all_nodes),
   ("Find", _("Find a _Node"), _("Find a Node from its Name"), inst.find_a_node),
   ("Find Again", _("Find _Again"), _("Iterate the Last Find Operation"), inst.find_again),
   ("Find Back", _("Find _Back"), _("Iterate the Last Find Operation in Opposite Direction"), inst.find_back),
   ("submenu-end", None, None, None),
   ("submenu-start", _("_Replace") , "Find Replace", None),
   ("Find Replace", _("_Replace in Node"), _("Replace into the Selected Node"), inst.replace_in_selected_node),
   ("Find Replace", _("Replace in Node_s"), _("Replace into all the Tree Nodes"), inst.replace_in_all_nodes),
   ("Find Replace", _("Replace in Nodes _Names"), _("Replace in Nodes Names"), inst.replace_in_nodes_names),
   ("Find Replace", _("Replace _Again"), _("Iterate the Last Replace Operation"), inst.replace_again),
   ("submenu-end", None, None, None),
   ("separator", None, None, None),
   ("Bulleted List", _("Set/Unset _Bulleted List"), _("Set/Unset the Current Paragraph as a Bulleted List Element"), inst.list_bulleted_handler),
   ("Numbered List", _("Set/Unset _Numbered List"), _("Set/Unset the Current Paragraph as a Numbered List Element"), inst.list_numbered_handler),
   ]

def get_popup_menu_entries_code(inst):
   """Returns the Menu Entries Given the Class Instance"""
   # stock id, label, tooltip, callback | "separator", None, None, None |
   # "submenu-start", label, stock id, None | "submenu-end", None, None, None
   return [
   ("separator",None,None,None),
   ("gtk-add", _("_Duplicate Row/Selection"), _("Duplicate the Whole Row/a Selection"), inst.text_row_selection_duplicate),
   ("gtk-clear", _("De_lete Row"), _("Delete the Whole Row"), inst.text_row_delete),
   ]

def get_popup_menu_entries_codebox(inst):
   """Returns the Menu Entries Given the Class Instance"""
   # stock id, label, tooltip, callback | "separator", None, None, None |
   # "submenu-start", label, stock id, None | "submenu-end", None, None, None
   return [
   ("separator",None,None,None),
   ("Cut", _("C_ut CodeBox"), _("Cut the Selected CodeBox"), inst.codebox_cut),
   ("Copy", _("_Copy CodeBox"), _("Copy the Selected CodeBox"), inst.codebox_copy),
   ("Delete", _("_Delete CodeBox"), _("Delete the Selected CodeBox"), inst.codebox_delete),
   ("separator",None,None,None),
   ("Edit CodeBox", _("Change CodeBox _Properties"), _("Edit the Properties of the CodeBox"), inst.codebox_change_properties),
   ("separator",None,None,None),
   ("gtk-go-forward", _("Increase CodeBox Width") + 20*CHAR_SPACE + "Ctrl+.", _("Increase the Width of the CodeBox"), inst.codebox_increase_width),
   ("gtk-go-back", _("Decrease CodeBox Width") + 10*CHAR_SPACE + "Ctrl+Alt+.", _("Decrease the Width of the CodeBox"), inst.codebox_decrease_width),
   ("gtk-go-down", _("Increase CodeBox Height") + 20*CHAR_SPACE + "Ctrl+,", _("Increase the Height of the CodeBox"), inst.codebox_increase_height),
   ("gtk-go-up", _("Decrease CodeBox Height") + 10*CHAR_SPACE + "Ctrl+Alt+,", _("Decrease the Height of the CodeBox"), inst.codebox_decrease_height),
   ]
