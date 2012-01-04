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

import os, sys


VERSION = "0.24"
APP_NAME = "cherrytree"
NEWER_VERSION_URL = "http://www.giuspen.com/software/version_cherrytree"
if sys.platform[0:3] == "win":
    EXE_DIR = os.getcwd()
    CONFIG_PATH = os.path.join(EXE_DIR, 'config.xml')
    LANG_PATH = os.path.join(EXE_DIR, 'lang')
    IMG_PATH = os.path.join(EXE_DIR, 'glade/img_tmp.png')
    TMP_FOLDER = os.path.join(EXE_DIR, 'glade/ct_tmp/')
    GLADE_PATH = os.path.join(EXE_DIR, 'glade/')
    LOCALE_PATH = os.path.join(EXE_DIR, 'locale/')
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
AVAILABLE_LANGS = ['default', 'cs', 'de', 'en', 'es', 'fr', 'it', 'pl', 'ru', 'uk']
SHOW_MENU_ICONS = "gconftool-2 --set /desktop/gnome/interface/menus_have_icons --type bool 1"
COLOR_YELLOW = "#ffffffff0000"
COLOR_WHITE = "#ffffffffffff"
COLOR_BLACK = "#000000000000"
BAD_CHARS = "[\x00-\x08\x0b-\x1f]"
TAG_PROPERTIES = ["weight", "foreground", "background", "style", "underline", "strikethrough",
                  "scale", "justification", "link"]
CUSTOM_COLORS_ID = "custom-colors"
STYLE_SCHEME_DEFAULT = "cobalt"
ANCHOR_CHAR = GLADE_PATH + 'anchor_char.png'
NODES_ICONS = {0:'Red Cherry', 1:'Blue Cherry', 2:'Orange Cherry', 3:'Cyan Cherry',
               4:'Orange Dark Cherry', 5:'Sherbert Cherry', 6:'Yellow Cherry'}
CODE_ICONS = {"python": 'Green Cherry', "sh":'Purple Cherry',
              "c":'Black Cherry', "cpp":'Black Cherry', "chdr":'Black Cherry'}

WHITE_SPACE_BETW_PIXB_AND_TEXT = 3
GRID_SLIP_OFFSET = 3
MAIN_WIN_TO_TEXT_WIN_NORMALIZER = 50

CHAR_SPACE = " "
CHAR_NEWLINE = "\n"
CHAR_CR = "\r"
CHAR_TAB = "\t"
CHAR_LISTBUL = "•"
CHAR_TILDE = "~"
CHAR_DQUOTE = '"'
CHAR_SQUOTE = "'"
CHAR_SQ_BR_OPEN = "["
CHAR_X = "X"
CHAR_SQ_BR_CLOSE = "]"

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
                  'ToDo List':'list_todo.png',
                  'Node Name Header':'node_name_header.png',
                  'Case Toggle':'case_toggle.png',
                  'Case Up':'case_upper.png',
                  'Case Down':'case_lower.png',
                  'Delete':'edit-delete.png',
                  'Copy':'edit-copy.png',
                  'Cut':'edit-cut.png',
                  'Paste':'edit-paste.png',
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
                  'Add Node':'tree-node-add.png',
                  'Add SubNode':'tree-subnode-add.png',
                  'Help Contents':'help-contents.png',
                  'Index':'index.png',
                  'Timestamp':'timestamp.png',
                  'Horizontal Rule':'horizontal_rule.png'}

NODES_STOCKS = ['Node Bullet', 'Node NoIcon', 'Black Cherry',
                'Blue Cherry', 'Cyan Cherry', 'Green Cherry',
                'Gray Cherry', 'Orange Cherry', 'Orange Dark Cherry',
                'Purple Cherry', 'Red Cherry', 'Sherbert Cherry',
                'Yellow Cherry']

HORIZONTAL_RULE = "\n=============================================\n"

TABLE_NODE_CREATE = """CREATE TABLE node (
node_id INTEGER UNIQUE,
name TEXT,
txt TEXT,
syntax TEXT,
tags TEXT,
is_ro INTEGER,
is_richtxt INTEGER,
has_codebox INTEGER,
has_table INTEGER,
has_image INTEGER,
has_children INTEGER,
level INTEGER,
)"""

TABLE_CODEBOX_CREATE = """CREATE TABLE codebox (
node_id INTEGER,
offset INTEGER,
justification TEXT,
txt TEXT,
syntax TEXT,
width INTEGER,
height INTEGER,
is_width_pix INTEGER,
do_highl_bra INTEGER,
do_show_linenum INTEGER
)"""

TABLE_TABLE_CREATE = """CREATE TABLE table (
node_id INTEGER,
offset INTEGER,
justification TEXT,
txt TEXT,
col_min INTEGER,
col_max INTEGER
)"""

TABLE_IMAGE_CREATE = """CREATE TABLE image (
node_id INTEGER,
offset INTEGER,
justification TEXT,
anchor TEXT,
png BLOB
)"""

TABLE_CHILDREN_CREATE = """CREATE TABLE children (
node_id INTEGER UNIQUE,
father_id INTEGER,
sequence INTEGER
)"""

TABLE_BOOKMARK_CREATE = """CREATE TABLE bookmark (
node_id INTEGER UNIQUE,
sequence INTEGER
)"""

UI_INFO = """
<ui>
    <menubar name='MenuBar'>
        <menu action='FileMenu'>
            <menuitem action='NewInstance'/>
            <menuitem action='OpenFile'/>
            <separator/>
            <menuitem action='CTDStorage'/>
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
            <menuitem action='Undo'/>
            <menuitem action='Redo'/>
            <separator/>
            <menuitem action='HandleImage'/>
            <menuitem action='HandleTable'/>
            <menuitem action='HandleCodeBox'/>
            <menuitem action='HandleLink'/>
            <menuitem action='HandleAnchor'/>
            <menuitem action='InsertTOC'/>
            <menuitem action='Timestamp'/>
            <menuitem action='HorizontalRule'/>
            <separator/>
            <menu action='ChangeCaseMenu'>
                <menuitem action='DownCase'/>
                <menuitem action='UpCase'/>
                <menuitem action='ToggleCase'/>
            </menu>
            <separator/>
            <menuitem action='CutRow'/>
            <menuitem action='CopyRow'/>
            <menuitem action='DeleteRow'/>
            <menuitem action='DuplicateRow'/>
            <menuitem action='MoveRowUp'/>
            <menuitem action='MoveRowDown'/>
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
            <menuitem action='BulletedList'/>
            <menuitem action='NumberedList'/>
            <menuitem action='ToDoList'/>
            <separator/>
            <menuitem action='JustifyLeft'/>
            <menuitem action='JustifyCenter'/>
            <menuitem action='JustifyRight'/>
        </menu>

        <menu action='TreeMenu'>
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
            <menuitem action='ShowHideTree'/>
            <menuitem action='ShowHideToolbar'/>
            <menuitem action='ShowHideNodeNameHeader'/>
            <separator/>
            <menuitem action='IncreaseToolbarIconsSize'/>
            <menuitem action='DecreaseToolbarIconsSize'/>
            <separator/>
            <menuitem action='Fullscreen'/>
        </menu>

        <menu action='BookmarksMenu'>
            <menuitem action='BookmarkThisNode'/>
            <menuitem action='BookmarksHandle'/>
        </menu>

        <menu action='ImportMenu'>
            <menuitem action='FromBasket'/>
            <menuitem action='FromCherryTree'/>
            <menuitem action='FromKeepNote'/>
            <menuitem action='FromKnowit'/>
            <menuitem action='FromLeo'/>
            <menuitem action='FromNoteCase'/>
            <menuitem action='FromTomboy'/>
            <menuitem action='FromTreepad'/>
            <menuitem action='FromTuxCards'/>
        </menu>

        <menu action='ExportMenu'>
            <menuitem action='NodePrint'/>
            <menuitem action='Export2Txt'/>
            <menuitem action='Export2HTML'/>
            <menuitem action='Export2CTD'/>
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
        <toolitem action='TreeAddNode'/>
        <toolitem action='TreeAddSubNode'/>
        <separator/>
        <toolitem action='GoBack'/>
        <toolitem action='GoForward'/>
        <separator/>
        <toolitem action='NewInstance'/>
        <toolitem action='Save'/>
        <separator/>
        <toolitem action='Undo'/>
        <toolitem action='Redo'/>
        <separator/>
        <toolitem action='FindInNodes'/>
        <separator/>
        <toolitem action='BulletedList'/>
        <toolitem action='NumberedList'/>
        <toolitem action='ToDoList'/>
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
    </toolbar>

    <popup name='SysTrayMenu'>
        <menuitem action='ShowHideMainWin'/>
        <separator/>
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
        <menuitem action='TableRowCut'/>
        <menuitem action='TableRowCopy'/>
        <menuitem action='TableRowPaste'/>
        <menuitem action='TableRowDelete'/>
        <separator/>
        <menuitem action='TableRowUp'/>
        <menuitem action='TableRowDown'/>
        <menuitem action='TableSortRowsDesc'/>
        <menuitem action='TableSortRowsAsc'/>
        <separator/>
        <menuitem action='TableEditProp'/>
        <menuitem action='TableExport'/>
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
    ( "ChangeCaseMenu", "Case Toggle", _("C_hange Case") ),
    ( "SearchMenu", None, _("_Search") ),
    ( "ReplaceMenu", None, _("_Replace") ),
    ( "ViewMenu", None, _("_View") ),
    ( "BookmarksMenu", None, _("_Bookmarks") ),
    ( "ImportMenu", None, _("_Import") ),
    ( "ExportMenu", None, _("E_xport") ),
    ( "HelpMenu", None, _("_Help") ),
    # name, stock id, label, accelerator, tooltip, callback
    ( "NewInstance", "New Instance", _("New _Instance"), None, _("Start a New Instance of CherryTree"), inst.file_new),
    ( "OpenFile", "gtk-open", _("_Open File"), "<control>O", _("Open a CherryTree Document"), inst.file_open),
    ( "CTDProtection", "gtk-dialog-authentication", _("Set _Document Protection"), None, _("Set Protection of the Current Document"), inst.dialog_edit_protection),
    ( "CTDStorage", "gtk-harddisk", _("Set Data Storage _Type"), None, _("Set Data Storage Type of the Current Document"), inst.dialog_edit_data_storage_type),
    ( "Save", "gtk-save", _("_Save"), "<control>S", _("Save File"), inst.file_save),
    ( "SaveAs", "gtk-save-as", _("Save _As"), "<control><shift>S", _("Save File As"), inst.file_save_as),
    ( "Export2HTML", "To HTML", _("Export To _HTML"), None, _("Export To HTML"), inst.export_to_html),
    ( "Export2Txt", "To Txt", _("Export to Plain _Text"), None, _("Export to Plain Text"), inst.export_to_txt),
    ( "Export2CTD", "CherryTree Export", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), inst.export_to_ctd),
    ( "PageSetup", "gtk-print", _("Pa_ge Setup"), "<control><shift>P", _("Set up the Page for Printing"), inst.export_print_page_setup),
    ( "NodePrint", "gtk-print", _("_Print / Export To PDF"), "<control>P", _("Print / Export To PDF"), inst.export_print),
    ( "QuitApp", "Quit App", _("_Quit"), "<control>Q", _("Quit the Application"), inst.quit_application),
    ( "ExitApp", "Quit App", _("_Exit CherryTree"), "<control><shift>Q", _("Exit from CherryTree"), inst.quit_application_totally),
    ( "Preferences", "gtk-preferences", _("_Preferences"), "<control><alt>P", _("Preferences"), inst.dialog_preferences),
    ( "HorizontalRule", "Horizontal Rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), inst.horizontal_rule_insert),
    ( "Timestamp", "Timestamp", _("Insert Ti_mestamp"), "<control>M", _("Insert Timestamp"), inst.timestamp_insert),
    ( "BulletedList", "Bulleted List", _("Set/Unset _Bulleted List"), None, _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), inst.list_bulleted_handler),
    ( "NumberedList", "Numbered List", _("Set/Unset _Numbered List"), None, _("Set/Unset the Current Paragraph/Selection as a Numbered List"), inst.list_numbered_handler),
    ( "ToDoList", "ToDo List", _("Set/Unset _To-Do List"), None, _("Set/Unset the Current Paragraph/Selection as a To-Do List"), inst.list_todo_handler),
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
    ( "DownCase", "Case Down", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), inst.text_selection_lower_case),
    ( "UpCase", "Case Up", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), inst.text_selection_upper_case),
    ( "ToggleCase", "Case Toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), inst.text_selection_toggle_case),
    ( "DuplicateRow", "gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), inst.text_row_selection_duplicate),
    ( "MoveRowUp", "gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), inst.text_row_up),
    ( "MoveRowDown", "gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), inst.text_row_down),
    ( "DeleteRow", "Delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), inst.text_row_delete),
    ( "CutRow", "Cut", _("Cu_t Row"), "<control><shift>X", _("Cut the Current Row/Selected Rows"), inst.text_row_cut),
    ( "CopyRow", "Copy", _("_Copy Row"), "<control><shift>C", _("Copy the Current Row/Selected Rows"), inst.text_row_copy),
    ( "GoBack", "gtk-go-back", _("Go _Back"), "<alt>Left", _("Go to the Previous Visited Node"), inst.go_back),
    ( "GoForward", "gtk-go-forward", _("Go _Forward"), "<alt>Right", _("Go to the Next Visited Node"), inst.go_forward),
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
    ( "Strikethrough", "gtk-strikethrough", _("Toggle Stri_kethrough Property"), "<control>E", _("Toggle Strikethrough Property of the Selected Text"), inst.apply_tag_strikethrough),
    ( "H1", "Format Text Large", _("Toggle h_1 Property"), "<control>1", _("Toggle h1 Property of the Selected Text"), inst.apply_tag_h1),
    ( "H2", "Format Text Large2", _("Toggle h_2 Property"), "<control>2", _("Toggle h2 Property of the Selected Text"), inst.apply_tag_h2),
    ( "Small", "Format Text Small", _("Toggle _Small Property"), "<control>0", _("Toggle Small Property of the Selected Text"), inst.apply_tag_small),
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
    ( "ShowHideMainWin", "CherryTree", _("Show/Hide _CherryTree"), None, _("Toggle Show/Hide CherryTree"), inst.toggle_show_hide_main_window),
    ( "ShowHideToolbar", "Toolbar", _("Show/Hide Tool_bar"), None, _("Toggle Show/Hide Toolbar"), inst.toggle_show_hide_toolbar),
    ( "IncreaseToolbarIconsSize", "gtk-add", _("_Increase Toolbar Icons Size"), None, _("Increase the Size of the Toolbar Icons"), inst.toolbar_icons_size_increase),
    ( "DecreaseToolbarIconsSize", "gtk-remove", _("_Decrease Toolbar Icons Size"), None, _("Decrease the Size of the Toolbar Icons"), inst.toolbar_icons_size_decrease),
    ( "Fullscreen", "gtk-fullscreen", _("_Full Screen On/Off"), "F11", _("Toggle Full Screen On/Off"), inst.fullscreen_toggle),
    ( "FromBasket", "CherryTree Import", _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), inst.nodes_add_from_basket_folder),
    ( "FromCherryTree", "CherryTree Import", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), inst.nodes_add_from_cherrytree_file),
    ( "FromNoteCase", "CherryTree Import", _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), inst.nodes_add_from_notecase_file),
    ( "FromKeepNote", "CherryTree Import", _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), inst.nodes_add_from_keepnote_folder),
    ( "FromKnowit", "CherryTree Import", _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), inst.nodes_add_from_knowit_file),
    ( "FromTomboy", "CherryTree Import", _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), inst.nodes_add_from_tomboy_folder),
    ( "FromTuxCards", "CherryTree Import", _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), inst.nodes_add_from_tuxcards_file),
    ( "FromTreepad", "CherryTree Import", _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), inst.nodes_add_from_treepad_file),
    ( "FromLeo", "CherryTree Import", _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), inst.nodes_add_from_leo_file),
    ( "Help", "Help Contents", _("_Help"), None, _("Application's Home Page"), inst.on_help_menu_item_activated),
    ( "CheckNewer", "gtk-network", _("_Check Newer Version"), None, _("Check for a Newer Version"), inst.check_for_newer_version),
    ( "About", "gtk-about", _("_About"), None, _("About CherryTree"), inst.dialog_about),
    ( "TableEditProp", "Edit Table", _("_Edit Table Properties"), None, _("Edit the Table Properties"), inst.tables_handler.table_edit_properties),
    ( "TableRowAdd", "gtk-add", _("_Add Row"), None, _("Add a Table Row"), inst.tables_handler.table_row_add),
    ( "TableRowCut", "Cut", _("Cu_t Row"), None, _("Cut a Table Row"), inst.tables_handler.table_row_cut),
    ( "TableRowCopy", "Copy", _("_Copy Row"), None, _("Copy a Table Row"), inst.tables_handler.table_row_copy),
    ( "TableRowPaste", "Paste", _("_Paste Row"), None, _("Paste a Table Row"), inst.tables_handler.table_row_paste),
    ( "TableRowDelete", "Delete", _("De_lete Row"), None, _("Delete the Selected Table Row"), inst.tables_handler.table_row_delete),
    ( "TableRowUp", "gtk-go-up", _("Move Row _Up"), None, _("Move the Selected Row Up"), inst.tables_handler.table_row_up),
    ( "TableRowDown", "gtk-go-down", _("Move Row _Down"), None, _("Move the Selected Row Down"), inst.tables_handler.table_row_down),
    ( "TableSortRowsDesc", "gtk-sort-descending", _("Sort Rows De_scending"), None, _("Sort all the Rows Descending"), inst.tables_handler.table_rows_sort_descending),
    ( "TableSortRowsAsc", "gtk-sort-ascending", _("Sort Rows As_cending"), None, _("Sort all the Rows Ascending"), inst.tables_handler.table_rows_sort_ascending),
    ( "TableExport", "Save Table", _("_Table Export"), None, _("Export Table as CSV File"), inst.tables_handler.table_export),
    ( "BookmarkThisNode", "gtk-add", _("_Bookmark This Node"), None, _("Add the Current Node to the Bookmarks List"), inst.bookmark_curr_node),
    ( "BookmarksHandle", "gtk-edit", _("_Handle Bookmarks"), None, _("Handle the Bookmarks List"), inst.bookmarks_handle),
    ( "TreeAddNode", "Add Node", _("Add _Node"), "<control>N", _("Add a Node having the same Father of the Selected Node"), inst.node_add),
    ( "TreeAddSubNode", "Add SubNode", _("Add _SubNode"), "<control><shift>N", _("Add a Child Node to the Selected Node"), inst.node_child_add),
    ]

def get_popup_menu_tree(inst):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
    ("Add Node", _("Add _Node"), "<control>N", _("Add a Node having the same Father of the Selected Node"), inst.node_add),
    ("Add SubNode", _("Add _SubNode"), "<control><shift>N", _("Add a Child Node to the Selected Node"), inst.node_child_add),
    ("separator", None, None, None, None),
    ("Edit Node", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), inst.node_edit),
    ("gtk-info", _("Tree _Info"), None, _("Tree Summary Information"), inst.tree_info),
    ("separator", None, None, None, None),
    ("submenu-start", _("Node _Move"), "gtk-jump-to", None, None),
    ("gtk-go-up", _("Node _Up"), "<shift>Up", _("Move the Selected Node Up"), inst.node_up),
    ("gtk-go-down", _("Node _Down"), "<shift>Down", _("Move the Selected Node Down"), inst.node_down),
    ("gtk-go-back", _("Node _Left"), "<shift>Left", _("Move the Selected Node Left"), inst.node_left),
    ("gtk-jump-to", _("Node Change _Father"), "<shift>Right", _("Change the Selected Node's Father"), inst.node_change_father),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("submenu-start", _("Nodes _Sort"), "gtk-sort-ascending", None, None),
    ("gtk-sort-ascending", _("Sort Tree _Ascending"), None, _("Sort the Tree Ascending"), inst.tree_sort_ascending),
    ("gtk-sort-descending", _("Sort Tree _Descending"), None, _("Sort the Tree Descending"), inst.tree_sort_descending),
    ("gtk-sort-ascending", _("Sort Siblings A_scending"), None, _("Sort all the Siblings of the Selected Node Ascending"), inst.node_siblings_sort_ascending),
    ("gtk-sort-descending", _("Sort Siblings D_escending"), None, _("Sort all the Siblings of the Selected Node Descending"), inst.node_siblings_sort_descending),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("gtk-zoom-in", _("E_xpand All Nodes"), None, _("Expand All the Tree Nodes"), inst.nodes_expand_all),
    ("gtk-zoom-out", _("_Collapse All Nodes"), None, _("Collapse All the Tree Nodes"), inst.nodes_collapse_all),
    ("separator", None, None, None, None),
    ("Find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
    ("Find Replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
    ("separator", None, None, None, None),
    ("submenu-start", _("Nodes _Import"), "CherryTree Import", None, None),
    ("CherryTree Import", _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), inst.nodes_add_from_basket_folder),
    ("CherryTree Import", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), inst.nodes_add_from_cherrytree_file),
    ("CherryTree Import", _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), inst.nodes_add_from_keepnote_folder),
    ("CherryTree Import", _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), inst.nodes_add_from_knowit_file),
    ("CherryTree Import", _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), inst.nodes_add_from_leo_file),
    ("CherryTree Import", _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), inst.nodes_add_from_notecase_file),
    ("CherryTree Import", _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), inst.nodes_add_from_tomboy_folder),
    ("CherryTree Import", _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), inst.nodes_add_from_treepad_file),
    ("CherryTree Import", _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), inst.nodes_add_from_tuxcards_file),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("Nodes E_xport"), "CherryTree Export", None, None),
    ("gtk-print", _("_Print / Export To PDF"), "<control>P", _("Print / Export To PDF"), inst.export_print),
    ("To Txt", _("Export to Plain _Text"), None, _("Export to Plain Text"), inst.export_to_txt),
    ("To HTML", _("Export To _HTML"), None, _("Export To HTML"), inst.export_to_html),
    ("CherryTree Export", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), inst.export_to_ctd),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"), inst.node_inherit_syntax),
    ("separator", None, None, None, None),
    ("Delete", _("De_lete Node"), "Delete", _("Delete the Selected Node"), inst.node_delete),
    ("separator", None, None, None, None),
    ("gtk-go-back", _("Go _Back"), "<alt>Left", _("Go to the Previous Visited Node"), inst.go_back),
    ("gtk-go-forward", _("Go _Forward"), "<alt>Right", _("Go to the Next Visited Node"), inst.go_forward),
    ]

def get_popup_menu_entries_text(inst):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
    ("separator", None, None, None, None),
    ("Paste", _("_Paste as Plain Text"), None, _("Paste as Plain Text, Discard the Rich Text Formatting"), inst.paste_as_plain_text),
    ("separator", None, None, None, None),
    ("submenu-start", _("For_matting") , "Format Text", None, None),
    ("Format Text Latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), inst.apply_tag_latest),
    ("gtk-clear", _("_Remove Formatting"), None, _("Remove the Formatting from the Selected Text"), inst.remove_text_formatting),
    ("separator", None, None, None, None),
    ("Change Color Foreground", _("Text _Color Foreground"), None, _("Change the Color of the Selected Text Foreground"), inst.apply_tag_foreground),
    ("Change Color Background", _("Text C_olor Background"), None, _("Change the Color of the Selected Text Background"), inst.apply_tag_background),
    ("gtk-bold", _("Toggle _Bold Property"), "<control>B", _("Toggle Bold Property of the Selected Text"), inst.apply_tag_bold),
    ("gtk-italic", _("Toggle _Italic Property"), "<control>I", _("Toggle Italic Property of the Selected Text"), inst.apply_tag_italic),
    ("gtk-underline", _("Toggle _Underline Property"), "<control>U", _("Toggle Underline Property of the Selected Text"), inst.apply_tag_underline),
    ("gtk-strikethrough", _("Toggle Stri_kethrough Property"), "<control>E", _("Toggle Strikethrough Property of the Selected Text"), inst.apply_tag_strikethrough),
    ("Format Text Large", _("Toggle h_1 Property"), "<control>1", _("Toggle h1 Property of the Selected Text"), inst.apply_tag_h1),
    ("Format Text Large2", _("Toggle h_2 Property"), "<control>2", _("Toggle h2 Property of the Selected Text"), inst.apply_tag_h2),
    ("Format Text Small", _("Toggle _Small Property"), "<control>0", _("Toggle Small Property of the Selected Text"), inst.apply_tag_small),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Justify") , "gtk-justify-center", None, None),
    ("gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), inst.apply_tag_justify_left),
    ("gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), inst.apply_tag_justify_center),
    ("gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), inst.apply_tag_justify_right),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_List") , "Bulleted List", None, None),
    ("Bulleted List", _("Set/Unset _Bulleted List"), None, _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), inst.list_bulleted_handler),
    ("Numbered List", _("Set/Unset _Numbered List"), None, _("Set/Unset the Current Paragraph/Selection as a Numbered List"), inst.list_numbered_handler),
    ("ToDo List", _("Set/Unset _To-Do List"), None, _("Set/Unset the Current Paragraph/Selection as a To-Do List"), inst.list_todo_handler),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("submenu-start", _("_Insert") , "Insert", None, None),
    ("Insert Image", _("Insert I_mage"), None, _("Insert an Image"), inst.image_handle),
    ("Insert Table", _("Insert _Table"), None, _("Insert a Table"), inst.table_handle),
    ("Insert CodeBox", _("Insert _CodeBox"), None, _("Insert a CodeBox"), inst.codebox_handle),
    ("Handle Link", _("Insert/Edit _Link"), "<control>L", _("Insert a Link/Edit the Underlying Link"), inst.apply_tag_link),
    ("Insert Anchor", _("Insert _Anchor"), None, _("Insert an Anchor"), inst.anchor_handle),
    ("Index", _("Insert T_OC"), None, _("Insert Table of Contents"), inst.toc_insert),
    ("Timestamp", _("Insert Ti_mestamp"), "<control>M", _("Insert Timestamp"), inst.timestamp_insert),
    ("Horizontal Rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), inst.horizontal_rule_insert),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("C_hange Case") , "Case Toggle", None, None),
    ("Case Down", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), inst.text_selection_lower_case),
    ("Case Up", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), inst.text_selection_upper_case),
    ("Case Toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), inst.text_selection_toggle_case),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Row") , "gtk-edit", None, None),
    ("Cut", _("Cu_t Row"), "<control><shift>X", _("Cut the Current Row/Selected Rows"), inst.text_row_cut),
    ("Copy", _("C_opy Row"), "<control><shift>C", _("Copy the Current Row/Selected Rows"), inst.text_row_copy),
    ("Delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), inst.text_row_delete),
    ("gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), inst.text_row_selection_duplicate),
    ("gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), inst.text_row_up),
    ("gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), inst.text_row_down),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("submenu-start", _("_Search") , "Find", None, None),
    ("Find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), inst.find_in_selected_node),
    ("Find", _("Find in Node_s"), "<control><shift>F", _("Find into all the Tree Nodes"), inst.find_in_all_nodes),
    ("Find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
    ("Find Again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), inst.find_again),
    ("Find Back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), inst.find_back),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Replace") , "Find Replace", None, None),
    ("Find Replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), inst.replace_in_selected_node),
    ("Find Replace", _("Replace in Node_s"), "<control><shift>H", _("Replace into all the Tree Nodes"), inst.replace_in_all_nodes),
    ("Find Replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
    ("Find Replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), inst.replace_again),
    ("submenu-end", None, None, None, None),
    ]

def get_popup_menu_entries_code(inst):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
    ("separator", None, None, None, None),
    ("Timestamp", _("Insert Ti_mestamp"), "<control>M", _("Insert Timestamp"), inst.timestamp_insert),
    ("submenu-start", _("C_hange Case") , "Case Toggle", None, None),
    ("Case Down", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), inst.text_selection_lower_case),
    ("Case Up", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), inst.text_selection_upper_case),
    ("Case Toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), inst.text_selection_toggle_case),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Row") , "gtk-edit", None, None),
    ("Cut", _("Cu_t Row"), "<control><shift>X", _("Cut the Current Row/Selected Rows"), inst.text_row_cut),
    ("Copy", _("C_opy Row"), "<control><shift>C", _("Copy the Current Row/Selected Rows"), inst.text_row_copy),
    ("Delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), inst.text_row_delete),
    ("gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), inst.text_row_selection_duplicate),
    ("gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), inst.text_row_up),
    ("gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), inst.text_row_down),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("submenu-start", _("_Search") , "Find", None, None),
    ("Find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), inst.find_in_selected_node),
    ("Find", _("Find in Node_s"), "<control><shift>F", _("Find into all the Tree Nodes"), inst.find_in_all_nodes),
    ("Find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
    ("Find Again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), inst.find_again),
    ("Find Back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), inst.find_back),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Replace") , "Find Replace", None, None),
    ("Find Replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), inst.replace_in_selected_node),
    ("Find Replace", _("Replace in Node_s"), "<control><shift>H", _("Replace into all the Tree Nodes"), inst.replace_in_all_nodes),
    ("Find Replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
    ("Find Replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), inst.replace_again),
    ("submenu-end", None, None, None, None),
    ]

def get_popup_menu_entries_codebox(inst):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
    ("separator", None, None, None, None),
    ("Cut", _("C_ut CodeBox"), None, _("Cut the Selected CodeBox"), inst.codebox_cut),
    ("Copy", _("_Copy CodeBox"), None, _("Copy the Selected CodeBox"), inst.codebox_copy),
    ("Delete", _("_Delete CodeBox"), None, _("Delete the Selected CodeBox"), inst.codebox_delete),
    ("separator", None, None, None, None),
    ("Edit CodeBox", _("Change CodeBox _Properties"), None, _("Edit the Properties of the CodeBox"), inst.codebox_change_properties),
    ("separator", None, None, None, None),
    ("gtk-go-forward", _("Increase CodeBox Width"), "<control>period", _("Increase the Width of the CodeBox"), inst.codebox_increase_width),
    ("gtk-go-back", _("Decrease CodeBox Width"), "<control><alt>period", _("Decrease the Width of the CodeBox"), inst.codebox_decrease_width),
    ("gtk-go-down", _("Increase CodeBox Height"), "<control>comma", _("Increase the Height of the CodeBox"), inst.codebox_increase_height),
    ("gtk-go-up", _("Decrease CodeBox Height"), "<control><alt>comma", _("Decrease the Height of the CodeBox"), inst.codebox_decrease_height),
    ]
