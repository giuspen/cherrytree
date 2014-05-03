# -*- coding: UTF-8 -*-
#
#       cons.py
#
#       Copyright 2009-2014 Giuseppe Penone <giuspen@gmail.com>
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

import os, sys


VERSION = "0.33.0"
APP_NAME = "cherrytree"
NEWER_VERSION_URL = "http://www.giuspen.com/software/version_cherrytree"
if sys.platform.startswith("win"):
    IS_WIN_OS = True
    CONFIG_DIR = os.path.join(os.environ['APPDATA'], APP_NAME)
    if SHARE_PATH:
        EXE_DIR = SHARE_PATH
        os.chdir(EXE_DIR)
    else: EXE_DIR = os.getcwd()
    TMP_FOLDER = os.path.join(os.environ['TEMP'], 'ct_tmp/')
    GLADE_PATH = os.path.join(EXE_DIR, 'glade/')
    SPECS_PATH = os.path.join(EXE_DIR, 'language-specs/')
    LOCALE_PATH = os.path.join(EXE_DIR, 'locale/')
else:
    IS_WIN_OS = False
    CONFIG_DIR = os.path.join(os.path.expanduser('~'), '.config/cherrytree')
    TMP_FOLDER = '/tmp/ct_tmp/'
    if not SHARE_PATH:
        GLADE_PATH = os.path.join(os.getcwd(), "glade/")
        SPECS_PATH = os.path.join(os.getcwd(), 'language-specs/')
        LOCALE_PATH = os.path.join(os.getcwd(), 'locale/')
    else:
        GLADE_PATH = os.path.join(SHARE_PATH, 'cherrytree/glade/')
        SPECS_PATH = os.path.join(SHARE_PATH, 'cherrytree/language-specs')
        LOCALE_PATH = os.path.join(SHARE_PATH, 'locale')
CONFIG_PATH = os.path.join(CONFIG_DIR, 'config.cfg')
LANG_PATH = os.path.join(CONFIG_DIR, 'lang')
IMG_PATH = os.path.join(CONFIG_DIR, 'img_tmp.png')

try:
    import appindicator
    HAS_APPINDICATOR = True
except: HAS_APPINDICATOR = False
XDG_CURRENT_DESKTOP = 'XDG_CURRENT_DESKTOP'
HAS_SYSTRAY = not (XDG_CURRENT_DESKTOP in os.environ and os.environ[XDG_CURRENT_DESKTOP] == "Unity")

AVAILABLE_LANGS = ['default', 'cs', 'de', 'en', 'es', 'fr', 'hy', 'it', 'nl', 'pl', 'pt_BR', 'ru', 'uk', 'zh_CN']
COLOR_48_LINK_WEBS = "#00004444ffff"
COLOR_48_LINK_NODE = "#071c838e071c"
COLOR_48_LINK_FILE = "#8b8b69691414"
COLOR_48_LINK_FOLD = "#7f7f7f7f7f7f"
COLOR_48_YELLOW = "#bbbbbbbb0000"
COLOR_48_WHITE = "#ffffffffffff"
COLOR_48_BLACK = "#000000000000"
COLOR_24_BLACK = "#000000"
COLOR_24_WHITE = "#ffffff"
COLOR_24_BLUEBG = "#001b33"
COLOR_24_LBLACK = "#0b0c0c"
COLOR_24_GRAY = "#e0e0e0"
RICH_TEXT_DARK_FG = COLOR_24_WHITE
RICH_TEXT_DARK_BG = COLOR_24_BLUEBG
RICH_TEXT_LIGHT_FG = COLOR_24_BLACK
RICH_TEXT_LIGHT_BG = COLOR_24_WHITE
TREE_TEXT_DARK_FG = COLOR_24_WHITE
TREE_TEXT_DARK_BG = COLOR_24_BLUEBG
TREE_TEXT_LIGHT_FG = COLOR_24_LBLACK
TREE_TEXT_LIGHT_BG = COLOR_24_GRAY
BAD_CHARS = "[\x00-\x08\x0b-\x1f]"
GTKSPELLCHECK_TAG_NAME = "gtkspellchecker-misspelled"
TAG_WEIGHT = "weight"
TAG_FOREGROUND = "foreground"
TAG_BACKGROUND = "background"
TAG_STYLE = "style"
TAG_UNDERLINE = "underline"
TAG_STRIKETHROUGH = "strikethrough"
TAG_SCALE = "scale"
TAG_FAMILY = "family"
TAG_JUSTIFICATION = "justification"
TAG_LINK = "link"
TAG_PROP_HEAVY = "heavy"
TAG_PROP_ITALIC = "italic"
TAG_PROP_MONOSPACE = "monospace"
TAG_PROP_SINGLE = "single"
TAG_PROP_TRUE = "true"
TAG_PROP_H1 = "h1"
TAG_PROP_H2 = "h2"
TAG_PROP_H3 = "h3"
TAG_PROP_H4 = "h4"
TAG_PROP_H5 = "h5"
TAG_PROP_H6 = "h6"
TAG_PROP_SUP = "sup"
TAG_PROP_SUB = "sub"
TAG_PROP_LEFT = "left"
TAG_PROP_CENTER = "center"
TAG_PROP_RIGHT = "right"
TAG_PROPERTIES = [TAG_WEIGHT, TAG_FOREGROUND, TAG_BACKGROUND, TAG_STYLE, TAG_UNDERLINE,
                  TAG_STRIKETHROUGH, TAG_SCALE, TAG_FAMILY, TAG_JUSTIFICATION, TAG_LINK]
LINK_TYPE_WEBS = "webs"
LINK_TYPE_FILE = "file"
LINK_TYPE_FOLD = "fold"
LINK_TYPE_NODE = "node"
RICH_TEXT_ID = "custom-colors"
PLAIN_TEXT_ID = "plain-text"
STYLE_SCHEME_DEFAULT = "cobalt"
ANCHOR_CHAR = GLADE_PATH + 'anchor.png'
NODES_ICONS = {0:'cherry_red', 1:'cherry_blue', 2:'cherry_orange', 3:'cherry_cyan',
               4:'cherry_orange_dark', 5:'cherry_sherbert', 6:'cherry_yellow'}
CODE_ICONS = {"python": 'cherry_green', "sh":'cherry_purple',
              "c":'cherry_black', "cpp":'cherry_black', "chdr":'cherry_black'}

MAX_FILE_NAME_LEN = 150
WHITE_SPACE_BETW_PIXB_AND_TEXT = 3
GRID_SLIP_OFFSET = 3
MAIN_WIN_TO_TEXT_WIN_NORMALIZER = 50
MAX_RECENT_DOCS = 10
TABLE_DEFAULT_COL_MIN = 40
TABLE_DEFAULT_COL_MAX = 400

CHAR_SPACE = " "
CHAR_NEWLINE = "\n"
CHAR_NEWPAGE = "\x0c"
CHAR_CR = "\r"
CHAR_TAB = "\t"
CHAR_LISTBUL = "•"
CHAR_LISTARR = "►"
CHAR_LISTTODO = "☐"
CHAR_LISTDONEOK= "☑"
CHAR_LISTDONEFAIL= "☒"
CHAR_TILDE = "~"
CHAR_MINUS = "-"
CHAR_DQUOTE = '"'
CHAR_SQUOTE = "'"
CHAR_SLASH = "/"
CHAR_BSLASH = "\\"
CHAR_SQ_BR_OPEN = "["
CHAR_SQ_BR_CLOSE = "]"
CHAR_PARENTH_OPEN = "("
CHAR_PARENTH_CLOSE = ")"
CHAR_LESSER = "<"
CHAR_GREATER = ">"
CHAR_STAR = "*"
CHAR_QUESTION = "?"
CHAR_COMMA = ","
CHAR_COLON = ":"
CHAR_SEMICOLON = ";"
CHAR_USCORE = "_"
CHAR_EQUAL = "="
CHAR_BR_OPEN = "{"
CHAR_BR_CLOSE = "}"
CHAR_CARET = "^"
CHAR_PIPE = "|"
CHAR_AMPERSAND = "&"

SPECIAL_CHAR_ARROW_RIGHT = "→"
SPECIAL_CHAR_ARROW_LEFT = "←"
SPECIAL_CHAR_ARROW_DOUBLE = "↔"
SPECIAL_CHAR_COPYRIGHT = "©"
SPECIAL_CHAR_UNREGISTERED_TRADEMARK = "™"
SPECIAL_CHAR_REGISTERED_TRADEMARK = "®"

STR_CURSOR_POSITION = "cursor-position"
STR_STOCK_CT_IMP = "import_in_cherrytree"
STR_VISIBLE = "visible"
STR_UTF8 = "utf-8"
STR_UTF16 = "utf-16"
STR_IGNORE = "ignore"
STR_RETURN = "Return"
STR_PYGTK_222_REQUIRED = "PyGTK 2.22 required"

HTML_HEADER = '<!doctype html><html><head><meta http-equiv="content-type" content="text/html; charset=utf-8"><title>%s</title><meta name="generator" content="CherryTree"></head><body>'
HTML_FOOTER = '</body></html>'

STOCKS_N_FILES = [
'node_bullet.png', 'node_no_icon.png', 'cherry_black.png', 'cherry_blue.png',
'cherry_cyan.png', 'cherry_green.png', 'cherry_gray.png', 'cherry_orange.png',
'cherry_orange_dark.png', 'cherry_purple.png', 'cherry_red.png', 'cherry_sherbert.png',
'cherry_yellow.png', 'image_insert.png', 'image_edit.png', 'image_save.png',
'table_insert.png', 'table_edit.png', 'table_save.png', 'codebox_insert.png',
'codebox_edit.png', 'anchor_insert.png', 'anchor_edit.png', 'anchor.png',
'insert.png', 'link_handle.png', 'link_website.png', 'cherry_edit.png',
'list_bulleted.png', 'list_numbered.png', 'list_todo.png', 'node_name_header.png',
'case_toggle.png', 'case_upper.png', 'case_lower.png', 'edit-delete.png',
'edit-copy.png', 'edit-cut.png', 'edit-paste.png', 'find.png',
'find_again.png', 'find_back.png', 'find_replace.png', 'color_background.png',
'color_foreground.png', 'format-text-large.png', 'format-text-large2.png', 'format-text-large3.png',
'format-text-small.png', 'format-text-subscript.png', 'format-text-superscript.png', 'format-text-monospace.png',
'format-text-strikethrough.png', 'format-text-underline.png', 'format-text-bold.png', 'format-text-italic.png',
'format_text_latest.png', 'format_text_clear.png', 'format_text.png',
'object-rotate-left.png', 'object-rotate-right.png',
'to_pdf.png', 'to_txt.png', 'to_html.png', 'to_cherrytree.png', 'export_from_cherrytree.png', STR_STOCK_CT_IMP + '.png',
'from_cherrytree.png', 'from_txt.png', 'from_html.png', 'cherrytree.png', 'quit-app.png',
'new-instance.png', 'toolbar.png', 'cherries.png', 'tree-node-add.png',
'tree-subnode-add.png', 'help-contents.png', 'index.png', 'timestamp.png',
'calendar.png', 'horizontal_rule.png']

NODES_STOCKS = ['node_bullet', 'node_no_icon', 'cherry_black',
                'cherry_blue', 'cherry_cyan', 'cherry_green',
                'cherry_gray', 'cherry_orange', 'cherry_orange_dark',
                'cherry_purple', 'cherry_red', 'cherry_sherbert',
                'cherry_yellow']

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
level INTEGER
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

TABLE_TABLE_CREATE = """CREATE TABLE grid (
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
            <menuitem action='EnaDisSpellCheck'/>
            <separator/>
            <menuitem action='PastePlainText'/>
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
            <menuitem action='H3'/>
            <menuitem action='Small'/>
            <menuitem action='Superscript'/>
            <menuitem action='Subscript'/>
            <menuitem action='Monospace'/>
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
            <menuitem action='FindInSelNSub'/>
            <menuitem action='FindNode'/>
            <menuitem action='FindAgain'/>
            <menuitem action='FindBack'/>
        </menu>

        <menu action='ReplaceMenu'>
            <menuitem action='ReplaceInNode'/>
            <menuitem action='ReplaceInNodes'/>
            <menuitem action='ReplaceInSelNSub'/>
            <menuitem action='ReplaceInNodesNames'/>
            <menuitem action='ReplaceAgain'/>
        </menu>

        <menu action='ViewMenu'>
            <menuitem action='ShowHideTree'/>
            <menuitem action='ShowHideToolbar'/>
            <menuitem action='ShowHideNodeNameHeader'/>
            <menuitem action='ShowAllMatchesDialog'/>
            <separator/>
            <menuitem action='ToggleTreeText'/>
            <menuitem action='ToggleNodeExpColl'/>
            <menuitem action='NodesExpAll'/>
            <menuitem action='NodesCollAll'/>
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
            <menuitem action='FromCherryTree'/>
            <menuitem action='FromTxtFile'/>
            <menuitem action='FromTxtFolder'/>
            <menuitem action='FromHtmlFile'/>
            <menuitem action='FromHtmlFolder'/>
            <menuitem action='FromBasket'/>
            <menuitem action='FromGnote'/>
            <menuitem action='FromKeepNote'/>
            <menuitem action='FromKeyNote'/>
            <menuitem action='FromKnowit'/>
            <menuitem action='FromLeo'/>
            <menuitem action='FromMempad'/>
            <menuitem action='FromNoteCase'/>
            <menuitem action='FromTomboy'/>
            <menuitem action='FromTreepad'/>
            <menuitem action='FromTuxCards'/>
            <menuitem action='FromZim'/>
        </menu>

        <menu action='ExportMenu'>
            <menuitem action='Export2PDF'/>
            <menuitem action='Export2TxtMultiple'/>
            <menuitem action='Export2TxtSingle'/>
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
        <toolitem action='TreeAddNode'/>
        <toolitem action='TreeAddSubNode'/>
        <separator/>
        <toolitem action='GoBack'/>
        <toolitem action='GoForward'/>
        <separator/>
        <toolitem action='Save'/>
        <toolitem action='Export2PDF'/>
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
        <toolitem action='RemoveFormatting'/>
        <toolitem action='ColorForeground'/>
        <toolitem action='ColorBackground'/>
        <toolitem action='Bold'/>
        <toolitem action='Italic'/>
        <toolitem action='Underline'/>
        <toolitem action='Strikethrough'/>
        <toolitem action='H1'/>
        <toolitem action='H2'/>
        <toolitem action='H3'/>
        <toolitem action='Small'/>
        <toolitem action='Superscript'/>
        <toolitem action='Subscript'/>
        <toolitem action='Monospace'/>
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
    ( "TreeImportMenu", STR_STOCK_CT_IMP, _("Nodes _Import") ),
    ( "TreeExportMenu", "export_from_cherrytree", _("Nodes E_xport") ),
    ( "ChangeCaseMenu", "case_toggle", _("C_hange Case") ),
    ( "SearchMenu", None, _("_Search") ),
    ( "ReplaceMenu", None, _("_Replace") ),
    ( "ViewMenu", None, _("_View") ),
    ( "BookmarksMenu", None, _("_Bookmarks") ),
    ( "ImportMenu", None, _("_Import") ),
    ( "ExportMenu", None, _("E_xport") ),
    ( "HelpMenu", None, _("_Help") ),
    # name, stock id, label, accelerator, tooltip, callback
    ( "NewInstance", "new-instance", _("New _Instance"), None, _("Start a New Instance of CherryTree"), inst.file_new),
    ( "OpenFile", "gtk-open", _("_Open File"), "<control>O", _("Open a CherryTree Document"), inst.file_open),
    ( "Save", "gtk-save", _("_Save"), "<control>S", _("Save File"), inst.file_save),
    ( "SaveAs", "gtk-save-as", _("Save _As"), "<control><shift>S", _("Save File As"), inst.file_save_as),
    ( "Export2PDF", "to_pdf", _("Export To _PDF"), None, _("Export To PDF"), inst.export_to_pdf),
    ( "Export2HTML", "to_html", _("Export To _HTML"), None, _("Export To HTML"), inst.export_to_html),
    ( "Export2TxtMultiple", "to_txt", _("Export to Multiple Plain _Text Files"), None, _("Export to Multiple Plain Text Files"), inst.export_to_txt_multiple),
    ( "Export2TxtSingle", "to_txt", _("Export to _Single Plain Text File"), None, _("Export to Single Plain Text File"), inst.export_to_txt_single),
    ( "Export2CTD", "to_cherrytree", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), inst.export_to_ctd),
    ( "PageSetup", "gtk-print", _("Pa_ge Setup"), "<control><shift>P", _("Set up the Page for Printing"), inst.export_print_page_setup),
    ( "NodePrint", "gtk-print", _("_Print"), "<control>P", _("Print"), inst.export_print),
    ( "QuitApp", "quit-app", _("_Quit"), "<control>Q", _("Quit the Application"), inst.quit_application),
    ( "ExitApp", "quit-app", _("_Exit CherryTree"), "<control><shift>Q", _("Exit from CherryTree"), inst.quit_application_totally),
    ( "Preferences", "gtk-preferences", _("_Preferences"), "<control><alt>P", _("Preferences"), inst.dialog_preferences),
    ( "HorizontalRule", "horizontal_rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), inst.horizontal_rule_insert),
    ( "Timestamp", "timestamp", _("Insert Ti_mestamp"), "<control><alt>M", _("Insert Timestamp"), inst.timestamp_insert),
    ( "BulletedList", "list_bulleted", _("Set/Unset _Bulleted List"), None, _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), inst.list_bulleted_handler),
    ( "NumberedList", "list_numbered", _("Set/Unset _Numbered List"), None, _("Set/Unset the Current Paragraph/Selection as a Numbered List"), inst.list_numbered_handler),
    ( "ToDoList", "list_todo", _("Set/Unset _To-Do List"), None, _("Set/Unset the Current Paragraph/Selection as a To-Do List"), inst.list_todo_handler),
    ( "HandleLink", "link_handle", _("Insert/Edit _Link"), "<control>L", _("Insert a Link/Edit the Underlying Link"), inst.apply_tag_link),
    ( "InsertTOC", "index", _("Insert T_OC"), None, _("Insert Table of Contents"), inst.toc_insert),
    ( "HandleAnchor", "anchor_insert", _("Insert _Anchor"), None, _("Insert an Anchor"), inst.anchor_handle),
    ( "EditAnchor", "anchor_edit", _("Edit _Anchor"), None, _("Edit the Underlying Anchor"), inst.anchor_edit),
    ( "HandleImage", "image_insert", _("Insert I_mage"), "<control><alt>I", _("Insert an Image"), inst.image_handle),
    ( "SaveImage", "image_save", _("_Save Image as PNG"), None, _("Save the Selected Image as a PNG file"), inst.image_save),
    ( "EditImage", "image_edit", _("_Edit Image"), None, _("Edit the Selected Image"), inst.image_edit),
    ( "CutImage", "edit-cut", _("C_ut Image"), None, _("Cut the Selected Image"), inst.image_cut),
    ( "CopyImage", "edit-copy", _("_Copy Image"), None, _("Copy the Selected Image"), inst.image_copy),
    ( "DeleteImage", "edit-delete", _("_Delete Image"), None, _("Delete the Selected Image"), inst.image_delete),
    ( "CutTable", "edit-cut", _("C_ut Table"), None, _("Cut the Selected Table"), inst.tables_handler.table_cut),
    ( "CopyTable", "edit-copy", _("_Copy Table"), None, _("Copy the Selected Table"), inst.tables_handler.table_copy),
    ( "DeleteTable", "edit-delete", _("_Delete Table"), None, _("Delete the Selected Table"), inst.tables_handler.table_delete),
    ( "HandleTable", "table_insert", _("Insert _Table"), "<control><alt>T", _("Insert a Table"), inst.table_handle),
    ( "HandleCodeBox", "codebox_insert", _("Insert _CodeBox"), "<control><alt>C", _("Insert a CodeBox"), inst.codebox_handle),
    ( "DownCase", "case_lower", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), inst.text_selection_lower_case),
    ( "UpCase", "case_upper", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), inst.text_selection_upper_case),
    ( "ToggleCase", "case_toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), inst.text_selection_toggle_case),
    ( "DuplicateRow", "gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), inst.text_row_selection_duplicate),
    ( "MoveRowUp", "gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), inst.text_row_up),
    ( "MoveRowDown", "gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), inst.text_row_down),
    ( "DeleteRow", "edit-delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), inst.text_row_delete),
    ( "CutRow", "edit-cut", _("Cu_t Row"), "<control><shift>X", _("Cut the Current Row/Selected Rows"), inst.text_row_cut),
    ( "CopyRow", "edit-copy", _("_Copy Row"), "<control><shift>C", _("Copy the Current Row/Selected Rows"), inst.text_row_copy),
    ( "GoBack", "gtk-go-back", _("Go _Back"), "<alt>Left", _("Go to the Previous Visited Node"), inst.go_back),
    ( "GoForward", "gtk-go-forward", _("Go _Forward"), "<alt>Right", _("Go to the Next Visited Node"), inst.go_forward),
    ( "Undo", "gtk-undo", _("_Undo"), "<control>Z", _("Undo Last Operation"), inst.requested_step_back),
    ( "Redo", "gtk-redo", _("_Redo"), "<control>Y", _("Redo Previously Discarded Operation"), inst.requested_step_ahead),
    ( "InheritSyntax", "gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"), inst.node_inherit_syntax),
    ( "FormatLatest", "format_text_latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), inst.apply_tag_latest),
    ( "PastePlainText", "edit-paste", _("_Paste as Plain Text"), "<control><shift>V", _("Paste as Plain Text, Discard the Rich Text Formatting"), inst.paste_as_plain_text),
    ( "RemoveFormatting", "format_text_clear", _("_Remove Formatting"), "<control><shift>R", _("Remove the Formatting from the Selected Text"), inst.remove_text_formatting),
    ( "ColorForeground", "color_foreground", _("Text _Color Foreground"), None, _("Change the Color of the Selected Text Foreground"), inst.apply_tag_foreground),
    ( "ColorBackground", "color_background", _("Text C_olor Background"), None, _("Change the Color of the Selected Text Background"), inst.apply_tag_background),
    ( "Bold", "format-text-bold", _("Toggle _Bold Property"), "<control>B", _("Toggle Bold Property of the Selected Text"), inst.apply_tag_bold),
    ( "Italic", "format-text-italic", _("Toggle _Italic Property"), "<control>I", _("Toggle Italic Property of the Selected Text"), inst.apply_tag_italic),
    ( "Underline", "format-text-underline", _("Toggle _Underline Property"), "<control>U", _("Toggle Underline Property of the Selected Text"), inst.apply_tag_underline),
    ( "Strikethrough", "format-text-strikethrough", _("Toggle Stri_kethrough Property"), "<control>E", _("Toggle Strikethrough Property of the Selected Text"), inst.apply_tag_strikethrough),
    ( "H1", "format-text-large", _("Toggle h_1 Property"), "<control>1", _("Toggle h1 Property of the Selected Text"), inst.apply_tag_h1),
    ( "H2", "format-text-large2", _("Toggle h_2 Property"), "<control>2", _("Toggle h2 Property of the Selected Text"), inst.apply_tag_h2),
    ( "H3", "format-text-large3", _("Toggle h_3 Property"), "<control>3", _("Toggle h3 Property of the Selected Text"), inst.apply_tag_h3),
    ( "Small", "format-text-small", _("Toggle _Small Property"), "<control>0", _("Toggle Small Property of the Selected Text"), inst.apply_tag_small),
    ( "Superscript", "format-text-superscript", _("Toggle Su_perscript Property"), None, _("Toggle Superscript Property of the Selected Text"), inst.apply_tag_superscript),
    ( "Subscript", "format-text-subscript", _("Toggle Su_bscript Property"), None, _("Toggle Subscript Property of the Selected Text"), inst.apply_tag_subscript),
    ( "Monospace", "format-text-monospace", _("Toggle _Monospace Property"), "<control>M", _("Toggle Monospace Property of the Selected Text"), inst.apply_tag_monospace),
    ( "JustifyLeft", "gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), inst.apply_tag_justify_left),
    ( "JustifyCenter", "gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), inst.apply_tag_justify_center),
    ( "JustifyRight", "gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), inst.apply_tag_justify_right),
    ( "FindInNode", "find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), inst.find_in_selected_node),
    ( "FindInNodes", "find", _("Find in _All Nodes"), "<control><shift>F", _("Find into all the Tree Nodes"), inst.find_in_all_nodes),
    ( "FindInSelNSub", "find", _("Find in _Selected Node and Subnodes"), "<control><alt>F", _("Find into the Selected Node and Subnodes"), inst.find_in_sel_node_and_subnodes),
    ( "FindNode", "find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
    ( "FindAgain", "find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), inst.find_again),
    ( "FindBack", "find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), inst.find_back),
    ( "ReplaceInNode", "find_replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), inst.replace_in_selected_node),
    ( "ReplaceInNodes", "find_replace", _("Replace in _All Nodes"), "<control><shift>H", _("Replace into all the Tree Nodes"), inst.replace_in_all_nodes),
    ( "ReplaceInSelNSub", "find_replace", _("Replace in _Selected Node and Subnodes"), "<control><alt>H", _("Replace into the Selected Node and Subnodes"), inst.replace_in_sel_node_and_subnodes),
    ( "ReplaceInNodesNames", "find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
    ( "ReplaceAgain", "find_replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), inst.replace_again),
    ( "ShowHideNodeNameHeader", "node_name_header", _("Show/Hide Node Name _Header"), None, _("Toggle Show/Hide Node Name Header"), inst.toggle_show_hide_node_name_header),
    ( "EnaDisSpellCheck", "gtk-spell-check", _("Enable/Disable _Spell Check"), "<control><alt>S", _("Toggle Enable/Disable Spell Check"), inst.toggle_ena_dis_spellcheck),
    ( "ShowHideTree", "cherries", _("Show/Hide _Tree"), "F9", _("Toggle Show/Hide Tree"), inst.toggle_show_hide_tree),
    ( "ShowHideMainWin", APP_NAME, _("Show/Hide _CherryTree"), None, _("Toggle Show/Hide CherryTree"), inst.toggle_show_hide_main_window),
    ( "ShowHideToolbar", "toolbar", _("Show/Hide Tool_bar"), None, _("Toggle Show/Hide Toolbar"), inst.toggle_show_hide_toolbar),
    ( "ShowAllMatchesDialog", "find", _("Show _All Matches Dialog"), "<control><shift>A", _("Show Search All Matches Dialog"), inst.find_allmatchesdialog_restore),
    ( "IncreaseToolbarIconsSize", "gtk-add", _("_Increase Toolbar Icons Size"), None, _("Increase the Size of the Toolbar Icons"), inst.toolbar_icons_size_increase),
    ( "DecreaseToolbarIconsSize", "gtk-remove", _("_Decrease Toolbar Icons Size"), None, _("Decrease the Size of the Toolbar Icons"), inst.toolbar_icons_size_decrease),
    ( "ToggleTreeText", "gtk-jump-to", _("Toggle _Focus Tree/Text"), "<control>Tab", _("Toggle Focus Between Tree and Text"), inst.toggle_tree_text),
    ( "ToggleNodeExpColl", "gtk-zoom-in", _("Toggle Node _Expanded/Collapsed"), "<control><shift>J", _("Toggle Expanded/Collapsed Status of the Selected Node"), inst.toggle_tree_node_expanded_collapsed),
    ( "NodesExpAll", "gtk-zoom-in", _("E_xpand All Nodes"), "<control><shift>E", _("Expand All the Tree Nodes"), inst.nodes_expand_all),
    ( "NodesCollAll", "gtk-zoom-out", _("_Collapse All Nodes"), "<control><shift>L", _("Collapse All the Tree Nodes"), inst.nodes_collapse_all),
    ( "Fullscreen", "gtk-fullscreen", _("_Full Screen On/Off"), "F11", _("Toggle Full Screen On/Off"), inst.fullscreen_toggle),
    ( "FromCherryTree", "from_cherrytree", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), inst.nodes_add_from_cherrytree_file),
    ( "FromTxtFile", "from_txt", _("From _Plain Text File"), None, _("Add Node from a Plain Text File to the Current Tree"), inst.nodes_add_from_plain_text_file),
    ( "FromTxtFolder", "from_txt", _("From _Folder of Plain Text Files"), None, _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), inst.nodes_add_from_plain_text_folder),
    ( "FromHtmlFile", "from_html", _("From _HTML File"), None, _("Add Node from an HTML File to the Current Tree"), inst.nodes_add_from_html_file),
    ( "FromHtmlFolder", "from_html", _("From _Folder of HTML Files"), None, _("Add Nodes from a Folder of HTML Files to the Current Tree"), inst.nodes_add_from_html_folder),
    ( "FromBasket", STR_STOCK_CT_IMP, _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), inst.nodes_add_from_basket_folder),
    ( "FromGnote", STR_STOCK_CT_IMP, _("From _Gnote Folder"), None, _("Add Nodes of a Gnote Folder to the Current Tree"), inst.nodes_add_from_gnote_folder),
    ( "FromKeepNote", STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), inst.nodes_add_from_keepnote_folder),
    ( "FromKeyNote", STR_STOCK_CT_IMP, _("From K_eyNote File"), None, _("Add Nodes of a KeyNote File to the Current Tree"), inst.nodes_add_from_keynote_file),
    ( "FromKnowit", STR_STOCK_CT_IMP, _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), inst.nodes_add_from_knowit_file),
    ( "FromLeo", STR_STOCK_CT_IMP, _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), inst.nodes_add_from_leo_file),
    ( "FromMempad", STR_STOCK_CT_IMP, _("From _Mempad File"), None, _("Add Nodes of a Mempad File to the Current Tree"), inst.nodes_add_from_mempad_file),
    ( "FromNoteCase", STR_STOCK_CT_IMP, _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), inst.nodes_add_from_notecase_file),
    ( "FromTomboy", STR_STOCK_CT_IMP, _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), inst.nodes_add_from_tomboy_folder),
    ( "FromTreepad", STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), inst.nodes_add_from_treepad_file),
    ( "FromTuxCards", STR_STOCK_CT_IMP, _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), inst.nodes_add_from_tuxcards_file),
    ( "FromZim", STR_STOCK_CT_IMP, _("From _Zim Folder"), None, _("Add Nodes of a Zim Folder to the Current Tree"), inst.nodes_add_from_zim_folder),
    ( "Help", "help-contents", _("Online _Manual"), "F1", _("Application's Online Manual"), inst.on_help_menu_item_activated),
    ( "CheckNewer", "gtk-network", _("_Check Newer Version"), None, _("Check for a Newer Version"), inst.check_for_newer_version),
    ( "About", "gtk-about", _("_About"), None, _("About CherryTree"), inst.dialog_about),
    ( "TableEditProp", "table_edit", _("_Edit Table Properties"), None, _("Edit the Table Properties"), inst.tables_handler.table_edit_properties),
    ( "TableRowAdd", "gtk-add", _("_Add Row"), None, _("Add a Table Row"), inst.tables_handler.table_row_add),
    ( "TableRowCut", "edit-cut", _("Cu_t Row"), None, _("Cut a Table Row"), inst.tables_handler.table_row_cut),
    ( "TableRowCopy", "edit-copy", _("_Copy Row"), None, _("Copy a Table Row"), inst.tables_handler.table_row_copy),
    ( "TableRowPaste", "edit-paste", _("_Paste Row"), None, _("Paste a Table Row"), inst.tables_handler.table_row_paste),
    ( "TableRowDelete", "edit-delete", _("De_lete Row"), None, _("Delete the Selected Table Row"), inst.tables_handler.table_row_delete),
    ( "TableRowUp", "gtk-go-up", _("Move Row _Up"), None, _("Move the Selected Row Up"), inst.tables_handler.table_row_up),
    ( "TableRowDown", "gtk-go-down", _("Move Row _Down"), None, _("Move the Selected Row Down"), inst.tables_handler.table_row_down),
    ( "TableSortRowsDesc", "gtk-sort-descending", _("Sort Rows De_scending"), None, _("Sort all the Rows Descending"), inst.tables_handler.table_rows_sort_descending),
    ( "TableSortRowsAsc", "gtk-sort-ascending", _("Sort Rows As_cending"), None, _("Sort all the Rows Ascending"), inst.tables_handler.table_rows_sort_ascending),
    ( "TableExport", "table_save", _("_Table Export"), None, _("Export Table as CSV File"), inst.tables_handler.table_export),
    ( "BookmarkThisNode", "gtk-add", _("_Bookmark This Node"), None, _("Add the Current Node to the Bookmarks List"), inst.bookmark_curr_node),
    ( "BookmarksHandle", "gtk-edit", _("_Handle Bookmarks"), None, _("Handle the Bookmarks List"), inst.bookmarks_handle),
    ( "TreeAddNode", "tree-node-add", _("Add _Node"), "<control>N", _("Add a Node having the same Father of the Selected Node"), inst.node_add),
    ( "TreeAddSubNode", "tree-subnode-add", _("Add _SubNode"), "<control><shift>N", _("Add a Child Node to the Selected Node"), inst.node_child_add),
    ]

def get_popup_menu_tree(inst):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
    ("tree-node-add", _("Add _Node"), "<control>N", _("Add a Node having the same Father of the Selected Node"), inst.node_add),
    ("tree-subnode-add", _("Add _SubNode"), "<control><shift>N", _("Add a Child Node to the Selected Node"), inst.node_child_add),
    ("separator", None, None, None, None),
    ("cherry_edit", _("Change Node _Properties"), "F2", _("Edit the Properties of the Selected Node"), inst.node_edit),
    ("calendar", _("Insert Today's Node"), "F8", _("Insert a Node with Hierarchy Year/Month/Day"), inst.node_date),
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
    ("find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
    ("find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
    ("separator", None, None, None, None),
    ("submenu-start", _("Nodes _Import"), STR_STOCK_CT_IMP, None, None),
    ("from_cherrytree", _("From _CherryTree File"), None, _("Add Nodes of a CherryTree File to the Current Tree"), inst.nodes_add_from_cherrytree_file),
    ("from_txt", _("From _Plain Text File"), None, _("Add Node from a Plain Text File to the Current Tree"), inst.nodes_add_from_plain_text_file),
    ("from_txt", _("From _Folder of Plain Text Files"), None, _("Add Nodes from a Folder of Plain Text Files to the Current Tree"), inst.nodes_add_from_plain_text_folder),
    ("from_html", _("From _HTML File"), None, _("Add Node from an HTML File to the Current Tree"), inst.nodes_add_from_html_file),
    ("from_html", _("From _Folder of HTML Files"), None, _("Add Nodes from a Folder of HTML Files to the Current Tree"), inst.nodes_add_from_html_folder),
    (STR_STOCK_CT_IMP, _("From _Basket Folder"), None, _("Add Nodes of a Basket Folder to the Current Tree"), inst.nodes_add_from_basket_folder),
    (STR_STOCK_CT_IMP, _("From _Gnote Folder"), None, _("Add Nodes of a Gnote Folder to the Current Tree"), inst.nodes_add_from_gnote_folder),
    (STR_STOCK_CT_IMP, _("From _KeepNote Folder"), None, _("Add Nodes of a KeepNote Folder to the Current Tree"), inst.nodes_add_from_keepnote_folder),
    (STR_STOCK_CT_IMP, _("From K_eyNote File"), None, _("Add Nodes of a KeyNote File to the Current Tree"), inst.nodes_add_from_keynote_file),
    (STR_STOCK_CT_IMP, _("From K_nowit File"), None, _("Add Nodes of a Knowit File to the Current Tree"), inst.nodes_add_from_knowit_file),
    (STR_STOCK_CT_IMP, _("From _Leo File"), None, _("Add Nodes of a Leo File to the Current Tree"), inst.nodes_add_from_leo_file),
    (STR_STOCK_CT_IMP, _("From _Mempad File"), None, _("Add Nodes of a Mempad File to the Current Tree"), inst.nodes_add_from_mempad_file),
    (STR_STOCK_CT_IMP, _("From _NoteCase File"), None, _("Add Nodes of a NoteCase File to the Current Tree"), inst.nodes_add_from_notecase_file),
    (STR_STOCK_CT_IMP, _("From T_omboy Folder"), None, _("Add Nodes of a Tomboy Folder to the Current Tree"), inst.nodes_add_from_tomboy_folder),
    (STR_STOCK_CT_IMP, _("From T_reepad Lite File"), None, _("Add Nodes of a Treepad Lite File to the Current Tree"), inst.nodes_add_from_treepad_file),
    (STR_STOCK_CT_IMP, _("From _TuxCards File"), None, _("Add Nodes of a TuxCards File to the Current Tree"), inst.nodes_add_from_tuxcards_file),
    (STR_STOCK_CT_IMP, _("From _Zim Folder"), None, _("Add Nodes of a Zim Folder to the Current Tree"), inst.nodes_add_from_zim_folder),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("Nodes E_xport"), "export_from_cherrytree", None, None),
    ("to_pdf", _("Export To _PDF"), None, _("Export To PDF"), inst.export_to_pdf),
    ("to_txt", _("Export to Multiple Plain _Text Files"), None, _("Export to Multiple Plain Text Files"), inst.export_to_txt_multiple),
    ("to_txt", _("Export to _Single Plain Text File"), None, _("Export to Single Plain Text File"), inst.export_to_txt_single),
    ("to_html", _("Export To _HTML"), None, _("Export To HTML"), inst.export_to_html),
    ("to_cherrytree", _("_Export To CherryTree Document"), None, _("Export To CherryTree Document"), inst.export_to_ctd),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("gtk-execute", _("_Inherit Syntax"), None, _("Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"), inst.node_inherit_syntax),
    ("separator", None, None, None, None),
    ("edit-delete", _("De_lete Node"), "Delete", _("Delete the Selected Node"), inst.node_delete),
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
    ("edit-paste", _("_Paste as Plain Text"), "<control><shift>V", _("Paste as Plain Text, Discard the Rich Text Formatting"), inst.paste_as_plain_text),
    ("separator", None, None, None, None),
    ("submenu-start", _("For_matting") , "format_text", None, None),
    ("format_text_latest", _("Format _Latest"), "F7", _("Memory of Latest Text Format Type"), inst.apply_tag_latest),
    ("format_text_clear", _("_Remove Formatting"), "<control><shift>R", _("Remove the Formatting from the Selected Text"), inst.remove_text_formatting),
    ("separator", None, None, None, None),
    ("color_foreground", _("Text _Color Foreground"), None, _("Change the Color of the Selected Text Foreground"), inst.apply_tag_foreground),
    ("color_background", _("Text C_olor Background"), None, _("Change the Color of the Selected Text Background"), inst.apply_tag_background),
    ("format-text-bold", _("Toggle _Bold Property"), "<control>B", _("Toggle Bold Property of the Selected Text"), inst.apply_tag_bold),
    ("format-text-italic", _("Toggle _Italic Property"), "<control>I", _("Toggle Italic Property of the Selected Text"), inst.apply_tag_italic),
    ("format-text-underline", _("Toggle _Underline Property"), "<control>U", _("Toggle Underline Property of the Selected Text"), inst.apply_tag_underline),
    ("format-text-strikethrough", _("Toggle Stri_kethrough Property"), "<control>E", _("Toggle Strikethrough Property of the Selected Text"), inst.apply_tag_strikethrough),
    ("format-text-large", _("Toggle h_1 Property"), "<control>1", _("Toggle h1 Property of the Selected Text"), inst.apply_tag_h1),
    ("format-text-large2", _("Toggle h_2 Property"), "<control>2", _("Toggle h2 Property of the Selected Text"), inst.apply_tag_h2),
    ("format-text-large3", _("Toggle h_3 Property"), "<control>3", _("Toggle h3 Property of the Selected Text"), inst.apply_tag_h3),
    ("format-text-small", _("Toggle _Small Property"), "<control>0", _("Toggle Small Property of the Selected Text"), inst.apply_tag_small),
    ("format-text-superscript", _("Toggle Su_perscript Property"), None, _("Toggle Superscript Property of the Selected Text"), inst.apply_tag_superscript),
    ("format-text-subscript", _("Toggle Su_bscript Property"), None, _("Toggle Subscript Property of the Selected Text"), inst.apply_tag_subscript),
    ("format-text-monospace", _("Toggle _Monospace Property"), None, _("Toggle Monospace Property of the Selected Text"), inst.apply_tag_monospace),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Justify") , "gtk-justify-center", None, None),
    ("gtk-justify-left", _("Justify _Left"), None, _("Justify Left the Current Paragraph"), inst.apply_tag_justify_left),
    ("gtk-justify-center", _("Justify _Center"), None, _("Justify Center the Current Paragraph"), inst.apply_tag_justify_center),
    ("gtk-justify-right", _("Justify _Right"), None, _("Justify Right the Current Paragraph"), inst.apply_tag_justify_right),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_List") , "list_bulleted", None, None),
    ("list_bulleted", _("Set/Unset _Bulleted List"), None, _("Set/Unset the Current Paragraph/Selection as a Bulleted List"), inst.list_bulleted_handler),
    ("list_numbered", _("Set/Unset _Numbered List"), None, _("Set/Unset the Current Paragraph/Selection as a Numbered List"), inst.list_numbered_handler),
    ("list_todo", _("Set/Unset _To-Do List"), None, _("Set/Unset the Current Paragraph/Selection as a To-Do List"), inst.list_todo_handler),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("submenu-start", _("_Insert") , "insert", None, None),
    ("image_insert", _("Insert I_mage"), None, _("Insert an Image"), inst.image_handle),
    ("table_insert", _("Insert _Table"), None, _("Insert a Table"), inst.table_handle),
    ("codebox_insert", _("Insert _CodeBox"), None, _("Insert a CodeBox"), inst.codebox_handle),
    ("link_handle", _("Insert/Edit _Link"), "<control>L", _("Insert a Link/Edit the Underlying Link"), inst.apply_tag_link),
    ("anchor_insert", _("Insert _Anchor"), None, _("Insert an Anchor"), inst.anchor_handle),
    ("index", _("Insert T_OC"), None, _("Insert Table of Contents"), inst.toc_insert),
    ("timestamp", _("Insert Ti_mestamp"), "<control>M", _("Insert Timestamp"), inst.timestamp_insert),
    ("horizontal_rule", _("Insert _Horizontal Rule"), "<control>R", _("Insert Horizontal Rule"), inst.horizontal_rule_insert),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("C_hange Case") , "case_toggle", None, None),
    ("case_lower", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), inst.text_selection_lower_case),
    ("case_upper", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), inst.text_selection_upper_case),
    ("case_toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), inst.text_selection_toggle_case),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Row") , "gtk-edit", None, None),
    ("edit-cut", _("Cu_t Row"), "<control><shift>X", _("Cut the Current Row/Selected Rows"), inst.text_row_cut),
    ("edit-copy", _("C_opy Row"), "<control><shift>C", _("Copy the Current Row/Selected Rows"), inst.text_row_copy),
    ("edit-delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), inst.text_row_delete),
    ("gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), inst.text_row_selection_duplicate),
    ("gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), inst.text_row_up),
    ("gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), inst.text_row_down),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("submenu-start", _("_Search") , "find", None, None),
    ("find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), inst.find_in_selected_node),
    ("find", _("Find in Node_s"), "<control><shift>F", _("Find into all the Tree Nodes"), inst.find_in_all_nodes),
    ("find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
    ("find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), inst.find_again),
    ("find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), inst.find_back),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Replace") , "find_replace", None, None),
    ("find_replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), inst.replace_in_selected_node),
    ("find_replace", _("Replace in Node_s"), "<control><shift>H", _("Replace into all the Tree Nodes"), inst.replace_in_all_nodes),
    ("find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
    ("find_replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), inst.replace_again),
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
    ("timestamp", _("Insert Ti_mestamp"), "<control>M", _("Insert Timestamp"), inst.timestamp_insert),
    ("gtk-clear", _("Stri_p Trailing Spaces"), None, _("Strip Trailing Spaces"), inst.strip_trailing_spaces),
    ("submenu-start", _("C_hange Case") , "case_toggle", None, None),
    ("case_lower", _("_Lower Case of Selection/Word"), "<control>W", _("Lower the Case of the Selection/the Underlying Word"), inst.text_selection_lower_case),
    ("case_upper", _("_Upper Case of Selection/Word"), "<control><shift>W", _("Upper the Case of the Selection/the Underlying Word"), inst.text_selection_upper_case),
    ("case_toggle", _("_Toggle Case of Selection/Word"), "<control>G", _("Toggle the Case of the Selection/the Underlying Word"), inst.text_selection_toggle_case),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Row") , "gtk-edit", None, None),
    ("edit-cut", _("Cu_t Row"), "<control><shift>X", _("Cut the Current Row/Selected Rows"), inst.text_row_cut),
    ("edit-copy", _("C_opy Row"), "<control><shift>C", _("Copy the Current Row/Selected Rows"), inst.text_row_copy),
    ("edit-delete", _("De_lete Row"), "<control>K", _("Delete the Current Row/Selected Rows"), inst.text_row_delete),
    ("gtk-add", _("_Duplicate Row"), "<control>D", _("Duplicate the Current Row/Selection"), inst.text_row_selection_duplicate),
    ("gtk-go-up", _("Move _Up Row"), "<alt>Up", _("Move Up the Current Row/Selected Rows"), inst.text_row_up),
    ("gtk-go-down", _("Move _Down Row"), "<alt>Down", _("Move Down the Current Row/Selected Rows"), inst.text_row_down),
    ("submenu-end", None, None, None, None),
    ("separator", None, None, None, None),
    ("submenu-start", _("_Search") , "find", None, None),
    ("find", _("_Find in Node"), "<control>F", _("Find into the Selected Node"), inst.find_in_selected_node),
    ("find", _("Find in Node_s"), "<control><shift>F", _("Find into all the Tree Nodes"), inst.find_in_all_nodes),
    ("find", _("Find a _Node"), "<control>T", _("Find a Node from its Name"), inst.find_a_node),
    ("find_again", _("Find _Again"), "F3", _("Iterate the Last Find Operation"), inst.find_again),
    ("find_back", _("Find _Back"), "F4", _("Iterate the Last Find Operation in Opposite Direction"), inst.find_back),
    ("submenu-end", None, None, None, None),
    ("submenu-start", _("_Replace") , "find_replace", None, None),
    ("find_replace", _("_Replace in Node"), "<control>H", _("Replace into the Selected Node"), inst.replace_in_selected_node),
    ("find_replace", _("Replace in Node_s"), "<control><shift>H", _("Replace into all the Tree Nodes"), inst.replace_in_all_nodes),
    ("find_replace", _("Replace in Nodes _Names"), "<control><shift>T", _("Replace in Nodes Names"), inst.replace_in_nodes_names),
    ("find_replace", _("Replace _Again"), "F6", _("Iterate the Last Replace Operation"), inst.replace_again),
    ("submenu-end", None, None, None, None),
    ]

def get_popup_menu_entries_link(inst):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
    ("separator", None, None, None, None),
    ("link_handle", _("Edit _Link"), None, _("Edit the Underlying Link"), inst.apply_tag_link),
    ("separator", None, None, None, None),
    ("edit-cut", _("C_ut Link"), None, _("Cut the Selected CodeBox"), inst.link_cut),
    ("edit-copy", _("_Copy Link"), None, _("Copy the Selected CodeBox"), inst.link_copy),
    ("gtk-clear", _("D_ismiss Link"), None, _("Dismiss the Selected Link"), inst.link_dismiss),
    ("edit-delete", _("_Delete Link"), None, _("Delete the Selected Link"), inst.link_delete),
    ]

def get_popup_menu_entries_codebox(inst):
    """Returns the Menu Entries Given the Class Instance"""
    # stock id, label, accelerator, tooltip, callback |
    # "separator", None, None, None, None |
    # "submenu-start", label, stock id, None, None |
    # "submenu-end", None, None, None, None
    return [
    ("separator", None, None, None, None),
    ("codebox_edit", _("Change CodeBox _Properties"), None, _("Edit the Properties of the CodeBox"), inst.codebox_change_properties),
    ("from_txt", _("CodeBox _Load From Text File"), None, _("Load the CodeBox Content From a Text File"), inst.codebox_load_from_file),
    ("to_txt", _("CodeBox _Save To Text File"), None, _("Save the CodeBox Content To a Text File"), inst.codebox_save_to_file),
    ("separator", None, None, None, None),
    ("edit-cut", _("C_ut CodeBox"), None, _("Cut the Selected CodeBox"), inst.codebox_cut),
    ("edit-copy", _("_Copy CodeBox"), None, _("Copy the Selected CodeBox"), inst.codebox_copy),
    ("edit-delete", _("_Delete CodeBox"), None, _("Delete the Selected CodeBox"), inst.codebox_delete),
    ("edit-delete", _("Delete CodeBox _Keep Content"), None, _("Delete the Selected CodeBox But Keep Its Content"), inst.codebox_delete_keeping_text),
    ("separator", None, None, None, None),
    ("gtk-go-forward", _("Increase CodeBox Width"), "<control>period", _("Increase the Width of the CodeBox"), inst.codebox_increase_width),
    ("gtk-go-back", _("Decrease CodeBox Width"), "<control><alt>period", _("Decrease the Width of the CodeBox"), inst.codebox_decrease_width),
    ("gtk-go-down", _("Increase CodeBox Height"), "<control>comma", _("Increase the Height of the CodeBox"), inst.codebox_increase_height),
    ("gtk-go-up", _("Decrease CodeBox Height"), "<control><alt>comma", _("Decrease the Height of the CodeBox"), inst.codebox_decrease_height),
    ]

# Original from Dieter Verfaillie https://github.com/dieterv/elib.intl/blob/master/lib/elib/intl/__init__.py
# List of ISO 639-1 and ISO 639-2 language codes: http://www.loc.gov/standards/iso639-2/
# List of known lcid's: http://www.microsoft.com/globaldev/reference/lcid-all.mspx
# List of known MUI packs: http://www.microsoft.com/globaldev/reference/win2k/setup/Langid.mspx
MICROSOFT_WINDOWS_LCID_to_ISO_LANG =\
{
    1078:    'af',  # Afrikaans - South Africa
    1052:    'sq',  # Albanian - Albania
    1118:    'am',  # Amharic - Ethiopia
    1025:    'ar',  # Arabic - Saudi Arabia
    5121:    'ar',  # Arabic - Algeria
    15361:   'ar',  # Arabic - Bahrain
    3073:    'ar',  # Arabic - Egypt
    2049:    'ar',  # Arabic - Iraq
    11265:   'ar',  # Arabic - Jordan
    13313:   'ar',  # Arabic - Kuwait
    12289:   'ar',  # Arabic - Lebanon
    4097:    'ar',  # Arabic - Libya
    6145:    'ar',  # Arabic - Morocco
    8193:    'ar',  # Arabic - Oman
    16385:   'ar',  # Arabic - Qatar
    10241:   'ar',  # Arabic - Syria
    7169:    'ar',  # Arabic - Tunisia
    14337:   'ar',  # Arabic - U.A.E.
    9217:    'ar',  # Arabic - Yemen
    1067:    'hy',  # Armenian - Armenia
    1101:    'as',  # Assamese
    2092:    'az',  # Azeri (Cyrillic)
    1068:    'az',  # Azeri (Latin)
    1069:    'eu',  # Basque
    1059:    'be',  # Belarusian
    1093:    'bn',  # Bengali (India)
    2117:    'bn',  # Bengali (Bangladesh)
    5146:    'bs',  # Bosnian (Bosnia/Herzegovina)
    1026:    'bg',  # Bulgarian
    1109:    'my',  # Burmese
    1027:    'ca',  # Catalan
    1116:    'chr', # Cherokee - United States
    2052:    'zh',  # Chinese - People's Republic of China
    4100:    'zh',  # Chinese - Singapore
    1028:    'zh',  # Chinese - Taiwan
    3076:    'zh',  # Chinese - Hong Kong SAR
    5124:    'zh',  # Chinese - Macao SAR
    1050:    'hr',  # Croatian
    4122:    'hr',  # Croatian (Bosnia/Herzegovina)
    1029:    'cs',  # Czech
    1030:    'da',  # Danish
    1125:    'dv',  # Divehi
    1043:    'nl',  # Dutch - Netherlands
    2067:    'nl',  # Dutch - Belgium
    1126:    'bin', # Edo
    1033:    'en',  # English - United States
    2057:    'en',  # English - United Kingdom
    3081:    'en',  # English - Australia
    10249:   'en',  # English - Belize
    4105:    'en',  # English - Canada
    9225:    'en',  # English - Caribbean
    15369:   'en',  # English - Hong Kong SAR
    16393:   'en',  # English - India
    14345:   'en',  # English - Indonesia
    6153:    'en',  # English - Ireland
    8201:    'en',  # English - Jamaica
    17417:   'en',  # English - Malaysia
    5129:    'en',  # English - New Zealand
    13321:   'en',  # English - Philippines
    18441:   'en',  # English - Singapore
    7177:    'en',  # English - South Africa
    11273:   'en',  # English - Trinidad
    12297:   'en',  # English - Zimbabwe
    1061:    'et',  # Estonian
    1080:    'fo',  # Faroese
    1065:    None,  # TODO: Farsi
    1124:    'fil', # Filipino
    1035:    'fi',  # Finnish
    1036:    'fr',  # French - France
    2060:    'fr',  # French - Belgium
    11276:   'fr',  # French - Cameroon
    3084:    'fr',  # French - Canada
    9228:    'fr',  # French - Democratic Rep. of Congo
    12300:   'fr',  # French - Cote d'Ivoire
    15372:   'fr',  # French - Haiti
    5132:    'fr',  # French - Luxembourg
    13324:   'fr',  # French - Mali
    6156:    'fr',  # French - Monaco
    14348:   'fr',  # French - Morocco
    58380:   'fr',  # French - North Africa
    8204:    'fr',  # French - Reunion
    10252:   'fr',  # French - Senegal
    4108:    'fr',  # French - Switzerland
    7180:    'fr',  # French - West Indies
    1122:    'fy',  # Frisian - Netherlands
    1127:    None,  # TODO: Fulfulde - Nigeria
    1071:    'mk',  # FYRO Macedonian
    2108:    'ga',  # Gaelic (Ireland)
    1084:    'gd',  # Gaelic (Scotland)
    1110:    'gl',  # Galician
    1079:    'ka',  # Georgian
    1031:    'de',  # German - Germany
    3079:    'de',  # German - Austria
    5127:    'de',  # German - Liechtenstein
    4103:    'de',  # German - Luxembourg
    2055:    'de',  # German - Switzerland
    1032:    'el',  # Greek
    1140:    'gn',  # Guarani - Paraguay
    1095:    'gu',  # Gujarati
    1128:    'ha',  # Hausa - Nigeria
    1141:    'haw', # Hawaiian - United States
    1037:    'he',  # Hebrew
    1081:    'hi',  # Hindi
    1038:    'hu',  # Hungarian
    1129:    None,  # TODO: Ibibio - Nigeria
    1039:    'is',  # Icelandic
    1136:    'ig',  # Igbo - Nigeria
    1057:    'id',  # Indonesian
    1117:    'iu',  # Inuktitut
    1040:    'it',  # Italian - Italy
    2064:    'it',  # Italian - Switzerland
    1041:    'ja',  # Japanese
    1099:    'kn',  # Kannada
    1137:    'kr',  # Kanuri - Nigeria
    2144:    'ks',  # Kashmiri
    1120:    'ks',  # Kashmiri (Arabic)
    1087:    'kk',  # Kazakh
    1107:    'km',  # Khmer
    1111:    'kok', # Konkani
    1042:    'ko',  # Korean
    1088:    'ky',  # Kyrgyz (Cyrillic)
    1108:    'lo',  # Lao
    1142:    'la',  # Latin
    1062:    'lv',  # Latvian
    1063:    'lt',  # Lithuanian
    1086:    'ms',  # Malay - Malaysia
    2110:    'ms',  # Malay - Brunei Darussalam
    1100:    'ml',  # Malayalam
    1082:    'mt',  # Maltese
    1112:    'mni', # Manipuri
    1153:    'mi',  # Maori - New Zealand
    1102:    'mr',  # Marathi
    1104:    'mn',  # Mongolian (Cyrillic)
    2128:    'mn',  # Mongolian (Mongolian)
    1121:    'ne',  # Nepali
    2145:    'ne',  # Nepali - India
    1044:    'no',  # Norwegian (Bokmal)
    2068:    'no',  # Norwegian (Nynorsk)
    1096:    'or',  # Oriya
    1138:    'om',  # Oromo
    1145:    'pap', # Papiamentu
    1123:    'ps',  # Pashto
    1045:    'pl',  # Polish
    1046:    'pt',  # Portuguese - Brazil
    2070:    'pt',  # Portuguese - Portugal
    1094:    'pa',  # Punjabi
    2118:    'pa',  # Punjabi (Pakistan)
    1131:    'qu',  # Quecha - Bolivia
    2155:    'qu',  # Quecha - Ecuador
    3179:    'qu',  # Quecha - Peru
    1047:    'rm',  # Rhaeto-Romanic
    1048:    'ro',  # Romanian
    2072:    'ro',  # Romanian - Moldava
    1049:    'ru',  # Russian
    2073:    'ru',  # Russian - Moldava
    1083:    'se',  # Sami (Lappish)
    1103:    'sa',  # Sanskrit
    1132:    'nso', # Sepedi
    3098:    'sr',  # Serbian (Cyrillic)
    2074:    'sr',  # Serbian (Latin)
    1113:    'sd',  # Sindhi - India
    2137:    'sd',  # Sindhi - Pakistan
    1115:    'si',  # Sinhalese - Sri Lanka
    1051:    'sk',  # Slovak
    1060:    'sl',  # Slovenian
    1143:    'so',  # Somali
    1070:    'wen', # Sorbian
    3082:    'es',  # Spanish - Spain (Modern Sort)
    1034:    'es',  # Spanish - Spain (Traditional Sort)
    11274:   'es',  # Spanish - Argentina
    16394:   'es',  # Spanish - Bolivia
    13322:   'es',  # Spanish - Chile
    9226:    'es',  # Spanish - Colombia
    5130:    'es',  # Spanish - Costa Rica
    7178:    'es',  # Spanish - Dominican Republic
    12298:   'es',  # Spanish - Ecuador
    17418:   'es',  # Spanish - El Salvador
    4106:    'es',  # Spanish - Guatemala
    18442:   'es',  # Spanish - Honduras
    58378:   'es',  # Spanish - Latin America
    2058:    'es',  # Spanish - Mexico
    19466:   'es',  # Spanish - Nicaragua
    6154:    'es',  # Spanish - Panama
    15370:   'es',  # Spanish - Paraguay
    10250:   'es',  # Spanish - Peru
    20490:   'es',  # Spanish - Puerto Rico
    21514:   'es',  # Spanish - United States
    14346:   'es',  # Spanish - Uruguay
    8202:    'es',  # Spanish - Venezuela
    1072:    None,  # TODO: Sutu
    1089:    'sw',  # Swahili
    1053:    'sv',  # Swedish
    2077:    'sv',  # Swedish - Finland
    1114:    'syr', # Syriac
    1064:    'tg',  # Tajik
    1119:    None,  # TODO: Tamazight (Arabic)
    2143:    None,  # TODO: Tamazight (Latin)
    1097:    'ta',  # Tamil
    1092:    'tt',  # Tatar
    1098:    'te',  # Telugu
    1054:    'th',  # Thai
    2129:    'bo',  # Tibetan - Bhutan
    1105:    'bo',  # Tibetan - People's Republic of China
    2163:    'ti',  # Tigrigna - Eritrea
    1139:    'ti',  # Tigrigna - Ethiopia
    1073:    'ts',  # Tsonga
    1074:    'tn',  # Tswana
    1055:    'tr',  # Turkish
    1090:    'tk',  # Turkmen
    1152:    'ug',  # Uighur - China
    1058:    'uk',  # Ukrainian
    1056:    'ur',  # Urdu
    2080:    'ur',  # Urdu - India
    2115:    'uz',  # Uzbek (Cyrillic)
    1091:    'uz',  # Uzbek (Latin)
    1075:    've',  # Venda
    1066:    'vi',  # Vietnamese
    1106:    'cy',  # Welsh
    1076:    'xh',  # Xhosa
    1144:    'ii',  # Yi
    1085:    'yi',  # Yiddish
    1130:    'yo',  # Yoruba
    1077:    'zu'   # Zulu
}
