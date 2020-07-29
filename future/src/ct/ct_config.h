/*
 * ct_config.h
 *
 * Copyright 2009-2020
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

#pragma once

#include <unordered_map>
#include <vector>
#include <glibmm.h>
#include "ct_const.h"
#include "ct_types.h"
#include "ct_splittable.h"
#include "ct_filesystem.h"

class CtConfig
{
    
public:
    CtConfig();
    virtual ~CtConfig();

    bool load_from_file(const fs::path& filepath = _defaultFilepath);
    bool write_to_file(const fs::path& filepath = _defaultFilepath);

    // [state]
    CtRecentDocsRestore                         recentDocsRestore;
    bool                                        toolbarVisible{true};
    bool                                        winIsMaximised{false};
    int                                         winRect[4]{10, 10, 963, 630};
    int                                         hpanedPos{170};
    bool                                        treeVisible{true};
    CtRecentDocsFilepaths                       recentDocsFilepaths;
    std::string                                 pickDirImport;
    std::string                                 pickDirExport;
    std::string                                 pickDirFile;
    std::string                                 pickDirImg;
    std::string                                 pickDirCsv;
    std::string                                 pickDirCbox;
    std::string                                 linkType{CtConst::LINK_TYPE_WEBS};
    bool                                        showNodeNameHeader{true};
    int                                         nodesOnNodeNameHeader{3};
    int                                         toolbarIconSize{1};
    std::unordered_map<gchar, std::string>      currColors{{'f', ""}, {'b', ""}, {'n', ""}};

    // [tree]
    CtRestoreExpColl                            restoreExpColl{CtRestoreExpColl::FROM_STR};
    bool                                        nodesBookmExp{false};
    std::string                                 nodesIcons{CtConst::NODE_ICON_TYPE_CHERRY};
    bool                                        auxIconHide{false};
    int                                         defaultIconText{CtConst::NODE_ICON_BULLET_ID};
    bool                                        treeRightSide{false};
    int                                         cherryWrapWidth{130};
    bool                                        treeClickFocusText{false};
    bool                                        treeClickExpand{false};

    // [editor]
    std::string                                 syntaxHighlighting{CtConst::RICH_TEXT_ID};
    std::string                                 autoSynHighl{CtConst::SYN_HIGHL_BASH};
    std::string                                 styleSchemeId{CtConst::STYLE_SCHEME_DARK};
    bool                                        enableSpellCheck{false};
    std::string                                 spellCheckLang;
    bool                                        showLineNumbers{false};
    bool                                        spacesInsteadTabs{false};
    int                                         tabsWidth{4};
    int                                         anchorSize{16};
    int                                         embfileSize{48};
    bool                                        embfileShowFileName{true};
    int                                         embfileMaxSize{10};
    bool                                        lineWrapping{true};
    bool                                        autoSmartQuotes{true};
    bool                                        tripleClickParagraph{false};
    bool                                        enableSymbolAutoreplace{true};
    bool                                        enableMdFormatting{false};
    int                                         wrappingIndent{-14};
    bool                                        autoIndent{true};
    bool                                        rtShowWhiteSpaces{false};
    bool                                        ptShowWhiteSpaces{true};
    bool                                        rtHighlCurrLine{true};
    bool                                        ptHighlCurrLine{true};
    int                                         spaceAroundLines{0};
    int                                         relativeWrappedSpace{50};
    Glib::ustring                               hRule{"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"};
    CtStringSplittable                          specialChars{CtConst::SPECIAL_CHARS_DEFAULT};
    CtStringSplittable                          selwordChars{CtConst::SELWORD_CHARS_DEFAULT};
    CtStringSplittable                          charsListbul{CtConst::CHARS_LISTBUL_DEFAULT};
    CtStringSplittable                          charsToc{CtConst::CHARS_TOC_DEFAULT};
    CtStringSplittable                          charsTodo{CtConst::CHARS_TODO_DEFAULT};
    CtStringSplittable                          chars_smart_dquote{CtConst::CHARS_SMART_DQUOTE_DEFAULT};
    CtStringSplittable                          chars_smart_squote{CtConst::CHARS_SMART_SQUOTE_DEFAULT};
    std::string                                 latestTagProp;
    std::string                                 latestTagVal;
    Glib::ustring                               timestampFormat{CtConst::TIMESTAMP_FORMAT_DEFAULT};
    bool                                        linksUnderline{true};
    bool                                        linksRelative{false};
    bool                                        weblinkCustomOn{false};
    bool                                        filelinkCustomOn{false};
    bool                                        folderlinkCustomOn{false};
#if defined(_WIN32) || defined(_WIN64)
    std::string                                 weblinkCustomAct{"explorer %s &"};
    std::string                                 filelinkCustomAct{"explorer %s &"};
    std::string                                 folderlinkCustomAct{"explorer %s &"};
#elif __APPLE__
    std::string                                 weblinkCustomAct{"open %s &"};
    std::string                                 filelinkCustomAct{"open %s &"};
    std::string                                 folderlinkCustomAct{"open %s &"};
#else
    std::string                                 weblinkCustomAct{"firefox %s &"};
    std::string                                 filelinkCustomAct{"xdg-open %s &"};
    std::string                                 folderlinkCustomAct{"xdg-open %s &"};
#endif
    // [codebox]
    double                                      codeboxWidth{500};
    double                                      codeboxHeight{100};
    bool                                        codeboxWidthPixels{true};
    bool                                        codeboxLineNum{false};
    bool                                        codeboxMatchBra{true};
    std::string                                 codeboxSynHighl{CtConst::PLAIN_TEXT_ID};
    bool                                        codeboxAutoResize{false};

    // [table]
    int                                         tableRows{3};
    int                                         tableColumns{3};
    CtTableColMode                              tableColMode{CtTableColMode::RENAME};
    int                                         tableColMin{40};
    int                                         tableColMax{60};

    // [fonts]
    std::string                                 rtFont{"Sans 9"};
    std::string                                 ptFont{"Sans 9"};
    std::string                                 treeFont{"Sans 8"};
    std::string                                 codeFont{"Monospace 9"};
    std::string                                 fallbackFontFamily{"Sans"};

    // [colors]
    std::string                                 rtDefFg{CtConst::RICH_TEXT_DARK_FG};
    std::string                                 rtDefBg{CtConst::RICH_TEXT_DARK_BG};
    std::string                                 ttDefFg{CtConst::TREE_TEXT_LIGHT_FG};
    std::string                                 ttDefBg{CtConst::TREE_TEXT_LIGHT_BG};
    std::string                                 monospaceBg{CtConst::DEFAULT_MONOSPACE_BG};
    std::string                                 colorPalette{"#000000:#ffffff:#7f7f7f:#ff0000:#a020f0:"
                                                             "#0000ff:#add8e6:#00ff00:#ffff00:#ffa500:"
                                                             "#e6e6fa:#a52a2a:#8b6914:#1e90ff:#ffc0cb:"
                                                             "#90ee90:#1a1a1a:#4d4d4d:#bfbfbf:#e5e5e5"};
    std::string                                 colLinkWebs{CtConst::COLOR_48_LINK_WEBS};
    std::string                                 colLinkNode{CtConst::COLOR_48_LINK_NODE};
    std::string                                 colLinkFile{CtConst::COLOR_48_LINK_FILE};
    std::string                                 colLinkFold{CtConst::COLOR_48_LINK_FOLD};

    // [misc]
    std::string                                 toolbarUiList{CtConst::TOOLBAR_VEC_DEFAULT};
    bool                                        systrayOn{false};
    bool                                        startOnSystray{false};
    bool                                        useAppInd{false};
    bool                                        autosaveOn{false};
    int                                         autosaveVal{5};
    bool                                        checkVersion{false};
    bool                                        wordCountOn{false};
    bool                                        reloadDocLast{true};
    bool                                        modTimeSentinel{false};
    bool                                        backupCopy{true};
    int                                         backupNum{3};
    bool                                        autosaveOnQuit{false};
    int                                         limitUndoableSteps{20};
    bool                                        usePandoc{true}; // Whether to use Pandoc for exporting

    // [keyboard]
    std::map<std::string, std::string>          customKbShortcuts;

    // [codexec_term]
    std::string                                 customCodexecTerm;

    // [codexec_type]
    std::map<std::string, std::string>          customCodexecType;

    // [codexec_ext]
    std::map<std::string, std::string>          customCodexecExt;

protected:
    template<class String> bool _populate_string_from_keyfile(const gchar* key, String* pTarget)
    {
        bool gotIt{false};
        if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key))
        {
            try
            {
                *pTarget = _uKeyFile->get_string(_currentGroup, key);
                gotIt = true;
            }
            catch (Glib::KeyFileError& kferror)
            {
                _unexpected_keyfile_error(key, kferror);
            }
        }
        return gotIt;
    }
    bool _populate_bool_from_keyfile(const gchar* key, bool* pTarget);
    bool _populate_int_from_keyfile(const gchar* key, int* pTarget);
    bool _populate_double_from_keyfile(const gchar* key, double* pTarget);
    void _populate_map_from_current_group(std::map<std::string, std::string>* pTarget);
    void _populate_current_group_from_map(const std::map<std::string, std::string>& map);
    void _populate_data_from_keyfile();
    void _populate_keyfile_from_data();
    void _unexpected_keyfile_error(const gchar* key, const Glib::KeyFileError& kferror);

    static const size_t _maxTempKeySize{20};
    static const fs::path _defaultFilepath;

    gchar _tempKey[_maxTempKeySize];
    std::unique_ptr<Glib::KeyFile> _uKeyFile;
    std::string _currentGroup;
};
