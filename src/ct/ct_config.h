/*
 * ct_config.h
 *
 * Copyright 2009-2023
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
#include "ct_filesystem.h"

class CtConfig
{
public:
    CtConfig();
    CtConfig(const std::string filepath);

    static const fs::path ConfigFilename;
    static const fs::path PrintPageSetupFilename;
    static const fs::path LangFilename;
    static const fs::path LogFilename;
    static const fs::path ConfigLanguageSpecsDirname;
    static const fs::path ConfigStylesDirname;
    static const fs::path ConfigIconsDirname;
    static const fs::path UserStyleTemplate;

    bool getInitLoadFromFileOk() { return _initLoadFromFileOk; }
    bool write_to_file(const std::string filepath = "");
    void move_from_tmp();

    void update_user_style(const unsigned num);
    static std::string get_user_style_id(const unsigned num);

    // [state]
    CtRecentDocsRestore                         recentDocsRestore;
    bool                                        toolbarVisible{true};
    bool                                        statusbarVisible{true};
    bool                                        treeLinesVisible{false};
    bool                                        winIsMaximised{false};
    int                                         winRect[4]{10, 10, 963, 630};
    int                                         hpanedPos{170};
    int                                         vpanedPos{450};
    bool                                        treeVisible{true};
    bool                                        vteVisible{false};
    bool                                        menubarVisible{true};
    CtRecentDocsFilepaths                       recentDocsFilepaths;
    std::string                                 pickDirImport;
    std::string                                 pickDirExport;
    std::string                                 pickDirFile;
    std::string                                 pickDirImg;
    std::string                                 pickDirCsv;
    std::string                                 pickDirCbox;
    std::string                                 linkType{CtConst::LINK_TYPE_WEBS};
    bool                                        menubarInTitlebar{false};
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
    int                                         lastIconSel{CtConst::NODE_ICON_SEL_DEFAULT};
    bool                                        treeRightSide{false};
    bool                                        cherryWrapEnabled{false};
    int                                         cherryWrapWidth{130};
    bool                                        treeClickFocusText{false};
    bool                                        treeClickExpand{false};

    // [editor]
    std::string                                 syntaxHighlighting{CtConst::RICH_TEXT_ID};
    std::string                                 autoSynHighl{CtConst::SYN_HIGHL_SHELL};
    std::string                                 rtStyleScheme{"user-1"};
    std::string                                 ptStyleScheme{"user-1"};
    std::string                                 taStyleScheme{"user-2"};
    std::string                                 coStyleScheme{"cobalt-darkened"};
    bool                                        enableSpellCheck{false};
    std::string                                 spellCheckLang;
    bool                                        showLineNumbers{false};
    bool                                        scrollBeyondLastLine{true};
    bool                                        spacesInsteadTabs{false};
    int                                         tabsWidth{4};
    int                                         anchorSize{16};
    int                                         latexSizeDpi{140};
    int                                         embfileIconSize{48};
    bool                                        embfileShowFileName{true};
    int                                         embfileMaxSize{10};
    bool                                        lineWrapping{true};
    bool                                        autoSmartQuotes{true};
    bool                                        urlAutoLink{true};
    bool                                        camelCaseAutoLink{true};
    bool                                        tripleClickParagraph{false};
    bool                                        enableSymbolAutoreplace{true};
#ifdef MD_AUTO_REPLACEMENT
    bool                                        enableMdFormatting{false};
#endif // MD_AUTO_REPLACEMENT
    int                                         wrappingIndent{-14};
    bool                                        autoIndent{true};
    bool                                        codeExecConfirm{true};
    bool                                        codeExecVte{true};
    std::string                                 vteShell{CtConst::VTE_SHELL_DEFAULT};
    bool                                        rtShowWhiteSpaces{false};
    bool                                        ptShowWhiteSpaces{true};
    bool                                        rtHighlCurrLine{true};
    bool                                        ptHighlCurrLine{true};
    bool                                        rtHighlMatchBra{false};
    bool                                        ptHighlMatchBra{true};
    int                                         spaceAroundLines{0};
    int                                         relativeWrappedSpace{50};
    Glib::ustring                               hRule{"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"};
    CtStringSplittable                          specialChars{CtConst::SPECIAL_CHARS_DEFAULT};
    Glib::ustring                               lastSpecialChar{};
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
#if defined(_WIN32)
    std::string                                 weblinkCustomAct{"explorer %s &"};
    std::string                                 filelinkCustomAct{"explorer %s &"};
    std::string                                 folderlinkCustomAct{"explorer %s &"};
#elif defined(__APPLE__)
    std::string                                 weblinkCustomAct{"open %s &"};
    std::string                                 filelinkCustomAct{"open %s &"};
    std::string                                 folderlinkCustomAct{"open %s &"};
#else
    std::string                                 weblinkCustomAct{"firefox %s &"};
    std::string                                 filelinkCustomAct{"xdg-open \"file://%s\" &"};
    std::string                                 folderlinkCustomAct{"xdg-open \"file://%s\" &"};
#endif
    // [codebox]
    double                                      codeboxWidth{500};
    double                                      codeboxHeight{100};
    bool                                        codeboxWidthPixels{true};
    bool                                        codeboxLineNum{false};
    bool                                        codeboxMatchBra{true};
    std::string                                 codeboxSynHighl{CtConst::PLAIN_TEXT_ID};
    bool                                        codeboxAutoResize{true};

    // [table]
    int                                         tableRows{3};
    int                                         tableColumns{3};
    int                                         tableColWidthDefault{60};
    int                                         tableCellsGoLight{CtConst::ADVISED_TABLE_LIGHT_HEAVY};

    // [fonts]
    Glib::ustring                               rtFont{CtConst::FONT_RT_DEFAULT};
    Glib::ustring                               ptFont{CtConst::FONT_PT_DEFAULT};
    Glib::ustring                               treeFont{CtConst::FONT_TREE_DEFAULT};
    Glib::ustring                               codeFont{CtConst::FONT_CODE_DEFAULT};
    Glib::ustring                               vteFont{CtConst::FONT_VTE_DEFAULT};
    Glib::ustring                               fallbackFontFamily{"Sans"};

    // [colors]
    std::string                                 ttDefFg{CtConst::TREE_TEXT_LIGHT_FG};
    std::string                                 ttDefBg{CtConst::TREE_TEXT_LIGHT_BG};
    std::string                                 ttSelFg{CtConst::TREE_TEXT_LIGHT_BG};
    std::string                                 ttSelBg{CtConst::TREE_TEXT_SEL_BG};
    std::string                                 monospaceFg{CtConst::DEFAULT_MONOSPACE_FG};
    std::string                                 monospaceBg{CtConst::DEFAULT_MONOSPACE_BG};
    bool                                        msDedicatedFont{false};
    Glib::ustring                               monospaceFont{CtConst::FONT_MS_DEFAULT};
    CtScalableTag                               scalableH1{CtConst::SCALABLE_H1_DEFAULT};
    CtScalableTag                               scalableH2{CtConst::SCALABLE_H2_DEFAULT};
    CtScalableTag                               scalableH3{CtConst::SCALABLE_H3_DEFAULT};
    CtScalableTag                               scalableH4{CtConst::SCALABLE_H4_DEFAULT};
    CtScalableTag                               scalableH5{CtConst::SCALABLE_H5_DEFAULT};
    CtScalableTag                               scalableH6{CtConst::SCALABLE_H6_DEFAULT};
    CtScalableTag                               scalableSmall{CtConst::SCALABLE_SMALL_DEFAULT};
    std::string                                 colLinkWebs{CtConst::COLOR_48_LINK_WEBS};
    std::string                                 colLinkNode{CtConst::COLOR_48_LINK_NODE};
    std::string                                 colLinkFile{CtConst::COLOR_48_LINK_FILE};
    std::string                                 colLinkFold{CtConst::COLOR_48_LINK_FOLD};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleTextFg{CtConst::USER_STYLE_TEXT_FG};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleTextBg{CtConst::USER_STYLE_TEXT_BG};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleSelectionFg{CtConst::USER_STYLE_SELECTION_FG};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleSelectionBg{CtConst::USER_STYLE_SELECTION_BG};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleCursor{CtConst::USER_STYLE_CURSOR};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleCurrentLineBg{CtConst::USER_STYLE_CURRENT_LINE_BG};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleLineNumbersFg{CtConst::USER_STYLE_LINE_NUMBERS_FG};
    std::array<std::string,CtConst::NUM_USER_STYLES> userStyleLineNumbersBg{CtConst::USER_STYLE_LINE_NUMBERS_BG};

    // [misc]
    std::string                                 toolbarUiList{CtConst::TOOLBAR_VEC_DEFAULT};
    bool                                        bookmarksInTopMenu{true};
    bool                                        systrayOn{false};
    bool                                        startOnSystray{false};
    bool                                        useAppInd{false};
    bool                                        autosaveOn{true};
    int                                         autosaveVal{1};
    bool                                        checkVersion{false};
    bool                                        wordCountOn{false};
    bool                                        reloadDocLast{true};
    bool                                        winTitleShowDocDir{true};
    bool                                        nodeNameHeaderShowFullPath{true};
    bool                                        modTimeSentinel{false};
    bool                                        backupCopy{true};
    int                                         backupNum{3};
    bool                                        autosaveOnQuit{false};
    bool                                        customBackupDirOn{false};
    std::string                                 customBackupDir{""};
    int                                         limitUndoableSteps{10};

    // [keyboard]
    std::map<std::string, std::string>          customKbShortcuts;

    // [codexec_term]
    std::string                                 customCodexecTerm;

    // [codexec_type]
    std::map<std::string, std::string>          customCodexecType;

    // [codexec_ext]
    std::map<std::string, std::string>          customCodexecExt;

    // helpers
    const std::array<CtScalableTag*,7>          scalablesTags{&scalableH1, &scalableH2, &scalableH3,
                                                              &scalableH4, &scalableH5, &scalableH6,
                                                              &scalableSmall};

protected:
    template<class String> bool _populate_string_from_keyfile(std::string key, String* pTarget) {
        bool gotIt{false};
        if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key)) {
            try {
                *pTarget = _uKeyFile->get_string(_currentGroup, key);
                gotIt = true;
            }
            catch (Glib::KeyFileError& kferror) {
                _unexpected_keyfile_error(key.c_str(), kferror);
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

    bool _load_from_file();
    void _ensure_user_styles_exist();

    static const size_t _maxTempKeySize{20};

    gchar _tempKey[_maxTempKeySize];
    std::unique_ptr<Glib::KeyFile> _uKeyFile;
    std::string _currentGroup;
    bool _initLoadFromFileOk{false};

    const fs::path _configFilepath;
    const fs::path _configFilepathTmp;
};
