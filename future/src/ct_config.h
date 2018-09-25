/*
 * ct_config.h
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

#pragma once

#include <unordered_map>
#include <glibmm.h>
#include "ct_const.h"


enum class RestoreExpColl : int {FROM_STR=0, ALL_EXP=1, ALL_COLL=2};

enum class TableColMode : int {RENAME=0, ADD=1, DELETE=2, RIGHT=3, LEFT=4};


struct t_ct_recent_docs_restore
{
    Glib::ustring   doc_name;
    Glib::ustring   exp_coll_str;
    Glib::ustring   node_path;
    int             cursor_pos;
};


class CTConfig
{
public:
    CTConfig();
    virtual ~CTConfig();

    // [state]
    Glib::ustring                               fileDir;
    Glib::ustring                               fileName;
    bool                                        toolbarVisible{true};
    bool                                        winIsMaximised{false};
    int                                         winRect[4]{10, 10, 963, 630};
    int                                         hpanedPos{170};
    bool                                        treeVisible{true};
    Glib::ustring                               nodePath;
    int                                         cursorPosition;
    std::list<Glib::ustring>                    recentDocs;
    Glib::ustring                               pickDirImport;
    Glib::ustring                               pickDirExport;
    Glib::ustring                               pickDirFile;
    Glib::ustring                               pickDirImg;
    Glib::ustring                               pickDirCsv;
    Glib::ustring                               pickDirCbox;
    Glib::ustring                               linkType{LINK_TYPE_WEBS};
    bool                                        showNodeNameHeader{true};
    int                                         nodesOnNodeNameHeader{3};
    int                                         toolbarIconSize{Gtk::BuiltinIconSize::ICON_SIZE_MENU};
    std::unordered_map<gchar, Glib::ustring>    currColors{{'f', ""}, {'b', ""}, {'n', ""}};

    // [tree]
    RestoreExpColl                              restoreExpColl{RestoreExpColl::FROM_STR};
    Glib::ustring                               expandedCollapsedString;
    t_ct_recent_docs_restore                    recentDocsRestore[MAX_RECENT_DOCS_RESTORE];
    bool                                        nodesBookmExp{false};
    Glib::ustring                               nodesIcons{NODE_ICON_TYPE_CHERRY};
    bool                                        auxIconHide{false};
    int                                         defaultIconText{NODE_ICON_BULLET_ID};
    bool                                        treeRightSide{false};
    int                                         cherryWrapWidth{130};
    bool                                        treeClickFocusText{false};
    bool                                        treeClickExpand{false};

    // [editor]
    Glib::ustring                               syntaxHighlighting{RICH_TEXT_ID};
    Glib::ustring                               autoSynHighl{SYN_HIGHL_BASH};
    Glib::ustring                               styleScheme{STYLE_SCHEME_DARK};
    bool                                        enableSpellCheck{false};
    Glib::ustring                               spellCheckLang;
    bool                                        showLineNumbers{false};
    bool                                        spacesInsteadTabs{false};
    int                                         tabsWidth{4};
    int                                         anchorSize{16};
    int                                         embfileSize{48};
    bool                                        embfileShowFileName{true};
    int                                         embfileMaxSize{10};
    bool                                        lineWrapping{true};
    bool                                        autoSmartQuotes{true};
    int                                         wrappingIndent{-14};
    bool                                        autoIndent{true};
    bool                                        rtShowWhiteSpaces{false};
    bool                                        ptShowWhiteSpaces{true};
    bool                                        rtHighlCurrLine{true};
    bool                                        ptHighlCurrLine{true};
    int                                         spaceAroundLines{0};
    int                                         relativeWrappedSpace{50};
    Glib::ustring                               hRule{"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"};
    Glib::ustring                               specialChars{SPECIAL_CHARS_DEFAULT};
    Glib::ustring                               selwordChars{SELWORD_CHARS_DEFAULT};
    Glib::ustring                               charsListbul{CHARS_LISTBUL_DEFAULT};
    Glib::ustring                               charsToc{CHARS_TOC_DEFAULT};
    Glib::ustring                               latestTagProp;
    Glib::ustring                               latestTagVal;
    Glib::ustring                               timestampFormat{TIMESTAMP_FORMAT_DEFAULT};
    bool                                        linksUnderline{true};
    bool                                        linksRelative{false};
    bool                                        weblinkCustomOn{false};
    bool                                        filelinkCustomOn{false};
    bool                                        folderlinkCustomOn{false};
    Glib::ustring                               weblinkCustomAct{"firefox %s &"};
    Glib::ustring                               filelinkCustomAct{"xdg-open %s &"};
    Glib::ustring                               folderlinkCustomAct{"xdg-open %s &"};

    // [codebox]
    double                                      codeboxWidth{500};
    double                                      codeboxHeight{100};
    bool                                        codeboxWidthPixels{true};
    bool                                        codeboxLineNum{false};
    bool                                        codeboxMatchBra{true};
    Glib::ustring                               codeboxSynHighl{PLAIN_TEXT_ID};
    bool                                        codeboxAutoResize{false};

    // [table]
    int                                         tableRows{3};
    int                                         tableColumns{3};
    TableColMode                                tableColMode{TableColMode::RENAME};
    int                                         tableColMin{40};
    int                                         tableColMax{60};

    // [fonts]
    Glib::ustring                               rtFont{"Sans 9"};
    Glib::ustring                               ptFont{"Sans 9"};
    Glib::ustring                               treeFont{"Sans 8"};
    Glib::ustring                               codeFont{"Monospace 9"};

    // [colors]
    Glib::ustring                               rtDefFg{RICH_TEXT_DARK_FG};
    Glib::ustring                               rtDefBg{RICH_TEXT_DARK_BG};
    Glib::ustring                               ttDefFg{TREE_TEXT_LIGHT_FG};
    Glib::ustring                               ttDefBg{TREE_TEXT_LIGHT_BG};
    Glib::ustring                               monospaceBg{DEFAULT_MONOSPACE_BG};
    Glib::ustring                               colorPalette{"#000000:#ffffff:#7f7f7f:#ff0000:#a020f0:"
                                                             "#0000ff:#add8e6:#00ff00:#ffff00:#ffa500:"
                                                             "#e6e6fa:#a52a2a:#8b6914:#1e90ff:#ffc0cb:"
                                                             "#90ee90:#1a1a1a:#4d4d4d:#bfbfbf:#e5e5e5"};
    Glib::ustring                               m_col_link_webs{COLOR_48_LINK_WEBS};
    Glib::ustring                               m_col_link_node{COLOR_48_LINK_NODE};
    Glib::ustring                               m_col_link_file{COLOR_48_LINK_FILE};
    Glib::ustring                               m_col_link_fold{COLOR_48_LINK_FOLD};

    // [misc]
    Glib::ustring                               toolbarUiList{TOOLBAR_VEC_DEFAULT};
    bool                                        systrayOn{false};
    bool                                        startOnSystray{false};
    bool                                        useAppInd{false};
    bool                                        autosaveOn{false};
    int                                         autosaveVal{5};
    bool                                        checkVersion{false};
    bool                                        wordCountOn{false};
    bool                                        reloadDocLast{true};
    bool                                        modTimeSentinel{true};
    bool                                        backupCopy{true};
    int                                         backupNum{3};
    bool                                        autosaveOnQuit{false};
    int                                         limitUndoableSteps{20};

    // [keyboard]
    std::map<Glib::ustring, Glib::ustring>     customKbShortcuts;

    // [codexec_term]
    Glib::ustring                              customCodexecTerm;

    // [codexec_type]
    std::map<Glib::ustring, Glib::ustring>     customCodexecType;

    // [codexec_ext]
    std::map<Glib::ustring, Glib::ustring>     customCodexecExt;

protected:
    bool _populateStringFromKeyfile(const gchar *key, Glib::ustring *p_target);
    bool _populateBoolFromKeyfile(const gchar *key, bool *p_target);
    bool _populateIntFromKeyfile(const gchar *key, int *p_target);
    bool _populateDoubleFromKeyfile(const gchar *key, double *p_target);
    void _populateMapFromCurrentGroup(std::map<Glib::ustring, Glib::ustring> *p_target);
    void _populateFromKeyfile();
    bool _checkLoadFromFile();
    void _unexpectedKeyfileError(const gchar *key, const Glib::KeyFileError &kferror);

    Glib::ustring _filepath;
    Glib::KeyFile* _pKeyFile{nullptr};
    Glib::ustring _currentGroup;
};
