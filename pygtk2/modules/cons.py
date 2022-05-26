# -*- coding: UTF-8 -*-
#
#       cons.py
#
#       Copyright 2009-2020
#       Giuseppe Penone <giuspen@gmail.com>
#       Evgenii Gurianov <https://github.com/txe>
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

import os
import sys
import tempfile


VERSION = "0.39.4"
APP_NAME = "cherrytree"
NEWER_VERSION_URL = "http://www.giuspen.net/software/version_cherrytree"
IS_MAC_OS = False

if sys.platform == 'darwin':
    IS_MAC_OS = True

if sys.platform.startswith("win"):
    IS_WIN_OS = True
    SZA_PATH = '"'+os.path.join(SHARE_PATH, "7za.exe")+'"'
    GLADE_PATH = os.path.join(SHARE_PATH, 'glade/')
    SPECS_PATH = os.path.join(SHARE_PATH, 'language-specs/')
    LOCALE_PATH = os.path.join(SHARE_PATH, 'locale/')
    if not os.path.isfile(os.path.join(SHARE_PATH, 'config.cfg')):
        CONFIG_DIR = os.path.join(os.environ['APPDATA'], APP_NAME)
    else:
        CONFIG_DIR = SHARE_PATH
else:
    IS_WIN_OS = False
    SZA_PATH = "7za"
    MODULES_PATH = os.path.dirname(os.path.realpath(__file__))
    CONFIG_DIR = os.path.join(os.path.expanduser('~'), '.config', 'cherrytree')
    if SHARE_PATH == os.path.dirname(MODULES_PATH):
        GLADE_PATH = os.path.join(SHARE_PATH, "glade/")
        SPECS_PATH = os.path.join(SHARE_PATH, 'language-specs/')
        LOCALE_PATH = os.path.join(SHARE_PATH, 'locale/')
        if os.path.isfile(os.path.join(SHARE_PATH, 'config.cfg')):
            CONFIG_DIR = SHARE_PATH
    else:
        GLADE_PATH = os.path.join(SHARE_PATH, 'cherrytree', 'glade/')
        SPECS_PATH = os.path.join(SHARE_PATH, 'cherrytree', 'language-specs')
        LOCALE_PATH = os.path.join(SHARE_PATH, 'locale')
CONFIG_PATH = os.path.join(CONFIG_DIR, 'config.cfg')
LANG_PATH = os.path.join(CONFIG_DIR, 'lang')
IMG_PATH = os.path.join(CONFIG_DIR, 'img_tmp.png')
TMP_FOLDER = tempfile.mkdtemp()

try:
    import appindicator
    HAS_APPINDICATOR = True
except: HAS_APPINDICATOR = False
XDG_CURRENT_DESKTOP = 'XDG_CURRENT_DESKTOP'
HAS_SYSTRAY = not (XDG_CURRENT_DESKTOP in os.environ and os.environ[XDG_CURRENT_DESKTOP] == "Unity")
DISABLE_SYSTRAY = IS_MAC_OS

AVAILABLE_LANGS = ['default', 'cs', 'de', 'el', 'en', 'es', 'fi', 'fr', 'hy', 'it', 'ja', 'lt', 'nl', 'pl', 'pt_BR', 'ru', 'sl', 'sv', 'tr', 'uk', 'zh_CN']
COLOR_48_LINK_WEBS = "#00008989ffff"
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
TAG_PROP_SMALL = "small"
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
TAG_PROP_FILL = "fill"
TAG_PROPERTIES = [TAG_WEIGHT, TAG_FOREGROUND, TAG_BACKGROUND, TAG_STYLE, TAG_UNDERLINE,
                  TAG_STRIKETHROUGH, TAG_SCALE, TAG_FAMILY, TAG_JUSTIFICATION, TAG_LINK]
TAG_SEPARATOR = "separator"
LINK_TYPE_WEBS = "webs"
LINK_TYPE_FILE = "file"
LINK_TYPE_FOLD = "fold"
LINK_TYPE_NODE = "node"
RICH_TEXT_ID = "custom-colors"
PLAIN_TEXT_ID = "plain-text"
STYLE_SCHEME_LIGHT = "classic"
STYLE_SCHEME_DARK = "cobalt"
STYLE_SCHEME_GRAY = "oblivion"
ANCHOR_CHAR = GLADE_PATH + 'anchor.png'
FILE_CHAR = GLADE_PATH + 'file_icon.png'

MIN_CT_DOC_SIZE = 10
MAX_FILE_NAME_LEN = 142
WHITE_SPACE_BETW_PIXB_AND_TEXT = 3
GRID_SLIP_OFFSET = 3
MAIN_WIN_TO_TEXT_WIN_NORMALIZER = 50
MAX_RECENT_DOCS = 10
MAX_TOOLTIP_LINK_CHARS = 150
TABLE_DEFAULT_COL_MIN = 40
TABLE_DEFAULT_COL_MAX = 400
SCROLL_MARGIN = 0.3
TREE_DRAG_EDGE_PROX = 10
TREE_DRAG_EDGE_SCROLL = 15

CHAR_SPACE = " "
CHAR_NEWLINE = "\n"
CHAR_NEWPAGE = "\x0c"
CHAR_CR = "\r"
CHAR_TAB = "\t"
CHARS_LISTNUM = ".)->"
NUM_CHARS_LISTNUM = len(CHARS_LISTNUM)
CHAR_TILDE = "~"
CHAR_MINUS = "-"
CHAR_DQUOTE = '"'
CHAR_SQUOTE = "'"
CHAR_GRAVE = "`"
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
CHAR_DOLLAR = "$"
CHARS_NOT_FOR_PASSWD = [CHAR_SQUOTE, CHAR_DQUOTE, CHAR_BSLASH, CHAR_SEMICOLON, CHAR_SPACE, CHAR_PARENTH_OPEN, CHAR_PARENTH_CLOSE, CHAR_PIPE, CHAR_AMPERSAND, CHAR_CARET, CHAR_DOLLAR, CHAR_LESSER, CHAR_GREATER]

SPECIAL_CHAR_ARROW_RIGHT = "→"
SPECIAL_CHAR_ARROW_RIGHT2 = "⇒"
SPECIAL_CHAR_ARROW_LEFT = "←"
SPECIAL_CHAR_ARROW_LEFT2 = "⇐"
SPECIAL_CHAR_ARROW_DOUBLE = "↔"
SPECIAL_CHAR_ARROW_DOUBLE2 = "⇔"
SPECIAL_CHAR_COPYRIGHT = "©"
SPECIAL_CHAR_UNREGISTERED_TRADEMARK = "™"
SPECIAL_CHAR_REGISTERED_TRADEMARK = "®"

WEB_LINK_STARTERS = ["http://", "https://", "www.", "ftp://"]
WEB_LINK_SEPARATORS = [CHAR_SPACE, CHAR_NEWLINE, CHAR_CR, CHAR_TAB]

STR_CURSOR_POSITION = "cursor-position"
STR_STOCK_CT_IMP = "import_in_cherrytree"
STR_VISIBLE = "visible"
STR_UTF8 = "utf-8"
STR_UTF16 = "utf-16"
STR_ISO_8859 = "iso-8859-1"
STR_IGNORE = "ignore"
STR_KEY_RETURN = "Return"
STR_KEY_DELETE = "Delete"
STR_KEY_TAB = "Tab"
STR_KEY_SHIFT_TAB = "ISO_Left_Tab"
STR_KEY_SPACE = "space"
STR_KEY_MENU = "Menu"
STR_KEY_UP = "Up"
STR_KEY_DOWN = "Down"
STR_KEY_LEFT = "Left"
STR_KEY_RIGHT = "Right"
STR_KEY_DQUOTE = "quotedbl"
STR_KEY_SQUOTE = "apostrophe"
STR_KEYS_CONTROL = ["Control_L", "Control_R"]
STR_KEYS_LAYOUT_GROUP = ["ISO_Prev_Group", "ISO_Next_Group"]
STR_PYGTK_222_REQUIRED = "PyGTK 2.22 required"

CHERRY_RED = 'cherry_red'
CHERRY_BLUE = 'cherry_blue'
CHERRY_ORANGE = 'cherry_orange'
CHERRY_CYAN = 'cherry_cyan'
CHERRY_ORANGE_DARK = 'cherry_orange_dark'
CHERRY_SHERBERT = 'cherry_sherbert'
CHERRY_YELLOW = 'cherry_yellow'
CHERRY_GREEN = 'cherry_green'
CHERRY_PURPLE = 'cherry_purple'
CHERRY_BLACK = 'cherry_black'
CHERRY_GRAY = 'cherry_gray'

HTML_HEADER = '''<!doctype html><html>
<head>
  <meta http-equiv="content-type" content="text/html; charset=utf-8">
  <title>%s</title>
  <meta name="generator" content="CherryTree">
  <link rel="stylesheet" href="res/styles3.css" type="text/css" />
  <script></script>
</head>
<body>'''
HTML_FOOTER = '</body></html>'

STOCKS_N_FILES = [
'node_bullet', 'node_no_icon', CHERRY_BLACK, CHERRY_BLUE,
CHERRY_CYAN, CHERRY_GREEN, CHERRY_GRAY, CHERRY_ORANGE,
CHERRY_ORANGE_DARK, CHERRY_PURPLE, CHERRY_RED, CHERRY_SHERBERT,
CHERRY_YELLOW, 'image_insert', 'screenshot_insert', 'image_edit', 'image_save',
'table_insert', 'table_edit', 'table_save', 'codebox_insert',
'codebox_edit', 'anchor_insert', 'anchor_edit', 'anchor',
'insert', 'link_handle', 'link_website', 'cherry_edit',
'list_bulleted', 'list_numbered', 'list_todo', 'node_name_header',
'case_toggle', 'case_upper', 'case_lower', 'edit-delete',
'edit-copy', 'edit-cut', 'edit-paste',
'find', 'find_sel', 'find_selnsub', 'find_all','find_again', 'find_back', 
'replace_sel', 'replace_selnsub', 'replace_all', 'find_replace', 'replace_again',
'color_background', 'color_foreground', 'format-text-large', 'format-text-large2', 'format-text-large3',
'format-text-small', 'format-text-subscript', 'format-text-superscript', 'format-text-monospace',
'format-text-strikethrough', 'format-text-underline', 'format-text-bold', 'format-text-italic',
'format_text_latest', 'format_text_clear', 'format_text',
'object-rotate-left', 'object-rotate-right',
'to_pdf', 'to_txt', 'to_html', 'to_cherrytree', 'export_from_cherrytree', STR_STOCK_CT_IMP,
'from_cherrytree', 'from_txt', 'from_html', 'cherrytree', 'quit-app',
'new-instance', 'toolbar', 'cherries', 'tree-node-dupl', 'tree-node-add',
'tree-subnode-add', 'information', 'help-contents', 'index', 'timestamp',
'calendar', 'horizontal_rule', 'file_icon', 'pin', 'pin-add', 'pin-remove',
'add', 'cancel', 'done', 'java', 'mail', 'notes', 'python', 'remove', 'star',
'terminal', 'terminal-red', 'warning', 'home', 'code', 'html',
'circle-green', 'circle-grey', 'circle-red', 'circle-yellow',
'locked', 'unlocked', 'lockpin', 'people', 'urgent', 'folder', 'leaf',
'xml', 'c', 'cpp', 'perl',
]
NODES_STOCKS_KEYS = [1,2,3,4,5,6,7,8,9,40,41,42,10,43,11,12,13,14,15,16,44,18,19,20,39,38,21,22,23,24,46,47,48,17,25,26,45,27,28,29,30,31,32,33,34,35,36,37]
NODES_STOCKS = {
 1: 'circle-green',
 2: 'circle-yellow',
 3: 'circle-red',
 4: 'circle-grey',
 5: 'add',
 6: 'remove',
 7: 'done',
 8: 'cancel',
 9: 'edit-delete',
10: 'warning',
11: 'star',
12: 'information',
13: 'help-contents',
14: 'home',
15: 'index',
16: 'mail',
17: 'html',
18: 'notes',
19: 'timestamp',
20: 'calendar',
21: 'terminal',
22: 'terminal-red',
23: 'python',
24: 'java',
25: 'node_bullet',
26: 'node_no_icon',
27: CHERRY_BLACK,
28: CHERRY_BLUE,
29: CHERRY_CYAN,
30: CHERRY_GREEN,
31: CHERRY_GRAY,
32: CHERRY_ORANGE,
33: CHERRY_ORANGE_DARK,
34: CHERRY_PURPLE,
35: CHERRY_RED,
36: CHERRY_SHERBERT,
37: CHERRY_YELLOW,
38: 'code',
39: 'find',
40: 'locked',
41: 'unlocked',
42: 'people',
43: 'urgent',
44: 'folder',
45: 'leaf',
46: 'xml',
47: 'c',
48: 'cpp',
}
NODES_ICONS = {
0: CHERRY_RED,
1: CHERRY_BLUE,
2: CHERRY_ORANGE,
3: CHERRY_CYAN,
4: CHERRY_ORANGE_DARK,
5: CHERRY_SHERBERT,
6: CHERRY_YELLOW,
7: CHERRY_GREEN,
8: CHERRY_PURPLE,
9: CHERRY_BLACK,
10: CHERRY_GRAY,
-1: CHERRY_GRAY,
}
CODE_ICONS = {
"python": 'python',
"python3": 'python',
"perl": 'perl',
"sh": 'terminal',
"dosbatch": 'terminal-red',
"powershell": 'terminal-red',
"java": 'java',
"html": 'html',
"xml": 'xml',
"c": 'c',
"cpp": 'cpp',
}
NODE_ICON_CODE_ID = 38
NODE_ICON_BULLET_ID = 25
NODE_ICON_NO_ICON_ID = 26

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
level INTEGER,
ts_creation INTEGER,
ts_lastsave INTEGER
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
png BLOB,
filename TEXT,
link TEXT,
time INTEGER
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

# http://www.w3schools.com/html/html_colornames.asp
HTML_COLOR_NAMES = {
'aliceblue': "#f0f8ff",
'antiquewhite': "#faebd7",
'aqua': "#00ffff",
'aquamarine': "#7fffd4",
'azure': "#f0ffff",
'beige': "#f5f5dc",
'bisque': "#ffe4c4",
'black': "#000000",
'blanchedalmond': "#ffebcd",
'blue': "#0000ff",
'blueviolet': "#8a2be2",
'brown': "#a52a2a",
'burlywood': "#deb887",
'cadetblue': "#5f9ea0",
'chartreuse': "#7fff00",
'chocolate': "#d2691e",
'coral': "#ff7f50",
'cornflowerblue': "#6495ed",
'cornsilk': "#fff8dc",
'crimson': "#dc143c",
'cyan': "#00ffff",
'darkblue': "#00008b",
'darkcyan': "#008b8b",
'darkgoldenrod': "#b8860b",
'darkgray': "#a9a9a9",
'darkgreen': "#006400",
'darkkhaki': "#bdb76b",
'darkmagenta': "#8b008b",
'darkolivegreen': "#556b2f",
'darkorange': "#ff8c00",
'darkorchid': "#9932cc",
'darkred': "#8b0000",
'darksalmon': "#e9967a",
'darkseagreen': "#8fbc8f",
'darkslateblue': "#483d8b",
'darkslategray': "#2f4f4f",
'darkturquoise': "#00ced1",
'darkviolet': "#9400d3",
'deeppink': "#ff1493",
'deepskyblue': "#00bfff",
'dimgray': "#696969",
'dodgerblue': "#1e90ff",
'firebrick': "#b22222",
'floralwhite': "#fffaf0",
'forestgreen': "#228b22",
'fuchsia': "#ff00ff",
'gainsboro': "#dcdcdc",
'ghostwhite': "#f8f8ff",
'gold': "#ffd700",
'goldenrod': "#daa520",
'gray': "#808080",
'green': "#008000",
'greenyellow': "#adff2f",
'honeydew': "#f0fff0",
'hotpink': "#ff69b4",
'indianred ': "#cd5c5c",
'indigo ': "#4b0082",
'ivory': "#fffff0",
'khaki': "#f0e68c",
'lavender': "#e6e6fa",
'lavenderblush': "#fff0f5",
'lawngreen': "#7cfc00",
'lemonchiffon': "#fffacd",
'lightblue': "#add8e6",
'lightcoral': "#f08080",
'lightcyan': "#e0ffff",
'lightgoldenrodyellow': "#fafad2",
'lightgray': "#d3d3d3",
'lightgreen': "#90ee90",
'lightpink': "#ffb6c1",
'lightsalmon': "#ffa07a",
'lightseagreen': "#20b2aa",
'lightskyblue': "#87cefa",
'lightslategray': "#778899",
'lightsteelblue': "#b0c4de",
'lightyellow': "#ffffe0",
'lime': "#00ff00",
'limegreen': "#32cd32",
'linen': "#faf0e6",
'magenta': "#ff00ff",
'maroon': "#800000",
'mediumaquamarine': "#66cdaa",
'mediumblue': "#0000cd",
'mediumorchid': "#ba55d3",
'mediumpurple': "#9370db",
'mediumseagreen': "#3cb371",
'mediumslateblue': "#7b68ee",
'mediumspringgreen': "#00fa9a",
'mediumturquoise': "#48d1cc",
'mediumvioletred': "#c71585",
'midnightblue': "#191970",
'mintcream': "#f5fffa",
'mistyrose': "#ffe4e1",
'moccasin': "#ffe4b5",
'navajowhite': "#ffdead",
'navy': "#000080",
'oldlace': "#fdf5e6",
'olive': "#808000",
'olivedrab': "#6b8e23",
'orange': "#ffa500",
'orangered': "#ff4500",
'orchid': "#da70d6",
'palegoldenrod': "#eee8aa",
'palegreen': "#98fb98",
'paleturquoise': "#afeeee",
'palevioletred': "#db7093",
'papayawhip': "#ffefd5",
'peachpuff': "#ffdab9",
'peru': "#cd853f",
'pink': "#ffc0cb",
'plum': "#dda0dd",
'powderblue': "#b0e0e6",
'purple': "#800080",
'red': "#ff0000",
'rosybrown': "#bc8f8f",
'royalblue': "#4169e1",
'saddlebrown': "#8b4513",
'salmon': "#fa8072",
'sandybrown': "#f4a460",
'seagreen': "#2e8b57",
'seashell': "#fff5ee",
'sienna': "#a0522d",
'silver': "#c0c0c0",
'skyblue': "#87ceeb",
'slateblue': "#6a5acd",
'slategray': "#708090",
'snow': "#fffafa",
'springgreen': "#00ff7f",
'steelblue': "#4682b4",
'tan': "#d2b48c",
'teal': "#008080",
'thistle': "#d8bfd8",
'tomato': "#ff6347",
'turquoise': "#40e0d0",
'violet': "#ee82ee",
'wheat': "#f5deb3",
'white': "#ffffff",
'whitesmoke': "#f5f5f5",
'yellow': "#ffff00",
'yellowgreen': "#9acd32",
}

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
