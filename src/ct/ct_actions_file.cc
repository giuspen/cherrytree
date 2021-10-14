/*
 * ct_actions_file.cc
 *
 * Copyright 2009-2021
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

#include "ct_actions.h"
#include "ct_storage_control.h"
#include "ct_pref_dlg.h"

void CtActions::_file_save(bool need_vacuum)
{
    if (not _is_tree_not_empty_or_error())
        return;
    if (_pCtMainWin->get_ct_storage()->get_file_path().empty())
        file_save_as();
    else
        _pCtMainWin->file_save(need_vacuum);
}

void CtActions::file_new()
{
    _pCtMainWin->signal_app_new_instance();
}

// Save the file
void CtActions::file_save()
{
    _file_save(false);
}

// Save the file and vacuum the db
void CtActions::file_vacuum()
{
    _file_save(true);
}

// Save the file providing a new name
void CtActions::file_save_as()
{
    if (not _is_tree_not_empty_or_error())
    {
        return;
    }
    CtDialogs::storage_select_args storageSelArgs(_pCtMainWin);
    fs::path currDocFilepath = _pCtMainWin->get_ct_storage()->get_file_path();
    if (not currDocFilepath.empty())
    {
        storageSelArgs.ctDocType = fs::get_doc_type(currDocFilepath);
        storageSelArgs.ctDocEncrypt = fs::get_doc_encrypt(currDocFilepath);
    }
    if (not CtDialogs::choose_data_storage_dialog(storageSelArgs))
    {
        return;
    }
    CtDialogs::FileSelectArgs fileSelArgs{_pCtMainWin};
    if (not currDocFilepath.empty())
    {
        fileSelArgs.curr_folder = currDocFilepath.parent_path();
        fs::path suggested_basename = currDocFilepath.filename();
        fileSelArgs.curr_file_name = suggested_basename.stem().string() + CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
    }
    fileSelArgs.filter_name = _("CherryTree Document");
    std::string fileExtension = CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
    fileSelArgs.filter_pattern.push_back(std::string{CtConst::CHAR_STAR}+fileExtension);
    std::string filepath = CtDialogs::file_save_as_dialog(fileSelArgs);
    if (filepath.empty())
    {
        return;
    }

    CtMiscUtil::filepath_extension_fix(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt, filepath);
    _pCtMainWin->file_save_as(filepath, storageSelArgs.password);
}

void CtActions::file_open()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtMainWin->get_ct_storage()->get_file_dir();
    args.filter_name = _("CherryTree Document");
    args.filter_pattern.push_back("*.ctb"); // macos doesn't understand *.ct*
    args.filter_pattern.push_back("*.ctx");
    args.filter_pattern.push_back("*.ctd");
    args.filter_pattern.push_back("*.ctz");

    std::string filepath = CtDialogs::file_select_dialog(args);

    if (filepath.empty()) return;

    _pCtMainWin->file_open(filepath, "");
}

void CtActions::quit_or_hide_window()
{
    _pCtMainWin->signal_app_quit_or_hide_window(_pCtMainWin);
}

void CtActions::quit_window()
{
    _pCtMainWin->signal_app_quit_window(_pCtMainWin);
}

void CtActions::dialog_preferences()
{
    _pCtMainWin->get_text_view().synch_spell_check_change_from_gspell_right_click_menu();
    CtPrefDlg prefDlg(_pCtMainWin);
    prefDlg.show();
    prefDlg.run();
}

void CtActions::preferences_import()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.filter_name = _("Preferences File");
    args.filter_pattern.push_back("*.cfg");
    const std::string filepath = CtDialogs::file_select_dialog(args);
    if (filepath.empty()) return;
    CtConfig ctConfigImported{filepath};
    if (not ctConfigImported.getInitLoadFromFileOk()) return;
    auto pConfig = _pCtMainWin->get_ct_config();

    pConfig->toolbarVisible = ctConfigImported.toolbarVisible;
    pConfig->statusbarVisible = ctConfigImported.statusbarVisible;
    pConfig->treeLinesVisible = ctConfigImported.treeLinesVisible;
    pConfig->hpanedPos = ctConfigImported.hpanedPos;
    pConfig->treeVisible = ctConfigImported.treeVisible;
    pConfig->linkType = ctConfigImported.linkType;
    pConfig->menubarInTitlebar = ctConfigImported.menubarInTitlebar;
    pConfig->showNodeNameHeader = ctConfigImported.showNodeNameHeader;
    pConfig->nodesOnNodeNameHeader = ctConfigImported.nodesOnNodeNameHeader;
    pConfig->toolbarIconSize = ctConfigImported.toolbarIconSize;
    pConfig->currColors['f'] = ctConfigImported.currColors['f'];
    pConfig->currColors['b'] = ctConfigImported.currColors['b'];
    pConfig->currColors['n'] = ctConfigImported.currColors['n'];
    pConfig->restoreExpColl = ctConfigImported.restoreExpColl;
    pConfig->nodesBookmExp = ctConfigImported.nodesBookmExp;
    pConfig->nodesIcons = ctConfigImported.nodesIcons;
    pConfig->auxIconHide = ctConfigImported.auxIconHide;
    pConfig->defaultIconText = ctConfigImported.defaultIconText;
    pConfig->treeRightSide = ctConfigImported.treeRightSide;
    pConfig->cherryWrapEnabled = ctConfigImported.cherryWrapEnabled;
    pConfig->cherryWrapWidth = ctConfigImported.cherryWrapWidth;
    pConfig->treeClickFocusText = ctConfigImported.treeClickFocusText;
    pConfig->treeClickExpand = ctConfigImported.treeClickExpand;
    pConfig->syntaxHighlighting = ctConfigImported.syntaxHighlighting;
    pConfig->autoSynHighl = ctConfigImported.autoSynHighl;
    pConfig->rtStyleScheme = ctConfigImported.rtStyleScheme;
    pConfig->ptStyleScheme = ctConfigImported.ptStyleScheme;
    pConfig->taStyleScheme = ctConfigImported.taStyleScheme;
    pConfig->coStyleScheme = ctConfigImported.coStyleScheme;
    pConfig->enableSpellCheck = ctConfigImported.enableSpellCheck;
    pConfig->spellCheckLang = ctConfigImported.spellCheckLang;
    pConfig->showLineNumbers = ctConfigImported.showLineNumbers;
    pConfig->scrollBeyondLastLine = ctConfigImported.scrollBeyondLastLine;
    pConfig->spacesInsteadTabs = ctConfigImported.spacesInsteadTabs;
    pConfig->tabsWidth = ctConfigImported.tabsWidth;
    pConfig->anchorSize = ctConfigImported.anchorSize;
    pConfig->embfileIconSize = ctConfigImported.embfileIconSize;
    pConfig->embfileShowFileName = ctConfigImported.embfileShowFileName;
    pConfig->embfileMaxSize = ctConfigImported.embfileMaxSize;
    pConfig->lineWrapping = ctConfigImported.lineWrapping;
    pConfig->autoSmartQuotes = ctConfigImported.autoSmartQuotes;
    pConfig->tripleClickParagraph = ctConfigImported.tripleClickParagraph;
    pConfig->enableSymbolAutoreplace = ctConfigImported.enableSymbolAutoreplace;
    pConfig->wrappingIndent = ctConfigImported.wrappingIndent;
    pConfig->autoIndent = ctConfigImported.autoIndent;
    pConfig->rtShowWhiteSpaces = ctConfigImported.rtShowWhiteSpaces;
    pConfig->ptShowWhiteSpaces = ctConfigImported.ptShowWhiteSpaces;
    pConfig->rtHighlCurrLine = ctConfigImported.rtHighlCurrLine;
    pConfig->ptHighlCurrLine = ctConfigImported.ptHighlCurrLine;
    pConfig->rtHighlMatchBra = ctConfigImported.rtHighlMatchBra;
    pConfig->ptHighlMatchBra = ctConfigImported.ptHighlMatchBra;
    pConfig->spaceAroundLines = ctConfigImported.spaceAroundLines;
    pConfig->relativeWrappedSpace = ctConfigImported.relativeWrappedSpace;
    pConfig->hRule = ctConfigImported.hRule;
    pConfig->specialChars = ctConfigImported.specialChars;
    pConfig->lastSpecialChar = ctConfigImported.lastSpecialChar;
    pConfig->selwordChars = ctConfigImported.selwordChars;
    pConfig->charsListbul = ctConfigImported.charsListbul;
    pConfig->charsToc = ctConfigImported.charsToc;
    pConfig->charsTodo = ctConfigImported.charsTodo;
    pConfig->chars_smart_dquote = ctConfigImported.chars_smart_dquote;
    pConfig->chars_smart_squote = ctConfigImported.chars_smart_squote;
    pConfig->latestTagProp = ctConfigImported.latestTagProp;
    pConfig->latestTagVal = ctConfigImported.latestTagVal;
    pConfig->timestampFormat = ctConfigImported.timestampFormat;
    pConfig->linksUnderline = ctConfigImported.linksUnderline;
    pConfig->linksRelative = ctConfigImported.linksRelative;
    pConfig->weblinkCustomOn = ctConfigImported.weblinkCustomOn;
    pConfig->filelinkCustomOn = ctConfigImported.filelinkCustomOn;
    pConfig->folderlinkCustomOn = ctConfigImported.folderlinkCustomOn;
    pConfig->weblinkCustomAct = ctConfigImported.weblinkCustomAct;
    pConfig->filelinkCustomAct = ctConfigImported.filelinkCustomAct;
    pConfig->folderlinkCustomAct = ctConfigImported.folderlinkCustomAct;
    pConfig->codeboxWidth = ctConfigImported.codeboxWidth;
    pConfig->codeboxHeight = ctConfigImported.codeboxHeight;
    pConfig->codeboxWidthPixels = ctConfigImported.codeboxWidthPixels;
    pConfig->codeboxLineNum = ctConfigImported.codeboxLineNum;
    pConfig->codeboxMatchBra = ctConfigImported.codeboxMatchBra;
    pConfig->codeboxSynHighl = ctConfigImported.codeboxSynHighl;
    pConfig->codeboxAutoResize = ctConfigImported.codeboxAutoResize;
    pConfig->tableRows = ctConfigImported.tableRows;
    pConfig->tableColumns = ctConfigImported.tableColumns;
    pConfig->tableColWidthDefault = ctConfigImported.tableColWidthDefault;
    pConfig->rtFont = ctConfigImported.rtFont;
    pConfig->ptFont = ctConfigImported.ptFont;
    pConfig->treeFont = ctConfigImported.treeFont;
    pConfig->codeFont = ctConfigImported.codeFont;
    pConfig->ttDefFg = ctConfigImported.ttDefFg;
    pConfig->ttDefBg = ctConfigImported.ttDefBg;
    pConfig->ttSelFg = ctConfigImported.ttSelFg;
    pConfig->ttSelBg = ctConfigImported.ttSelBg;
    pConfig->scalableH1.deserialise(ctConfigImported.scalableH1.serialise().c_str());
    pConfig->scalableH2.deserialise(ctConfigImported.scalableH2.serialise().c_str());
    pConfig->scalableH3.deserialise(ctConfigImported.scalableH3.serialise().c_str());
    pConfig->scalableH4.deserialise(ctConfigImported.scalableH4.serialise().c_str());
    pConfig->scalableH5.deserialise(ctConfigImported.scalableH5.serialise().c_str());
    pConfig->scalableH6.deserialise(ctConfigImported.scalableH6.serialise().c_str());
    pConfig->scalableSmall.deserialise(ctConfigImported.scalableSmall.serialise().c_str());
    pConfig->monospaceFg = ctConfigImported.monospaceFg;
    pConfig->monospaceBg = ctConfigImported.monospaceBg;
    pConfig->msDedicatedFont = ctConfigImported.msDedicatedFont;
    pConfig->monospaceFont = ctConfigImported.monospaceFont;
    pConfig->colorPalette = ctConfigImported.colorPalette;
    pConfig->colLinkWebs = ctConfigImported.colLinkWebs;
    pConfig->colLinkNode = ctConfigImported.colLinkNode;
    pConfig->colLinkFile = ctConfigImported.colLinkFile;
    pConfig->colLinkFold = ctConfigImported.colLinkFold;
    for (unsigned n = 1; n <= CtConst::NUM_USER_STYLES; ++n) {
        const unsigned i = n-1;
        pConfig->userStyleTextFg[i] = ctConfigImported.userStyleTextFg[i];
        pConfig->userStyleTextBg[i] = ctConfigImported.userStyleTextBg[i];
        pConfig->userStyleSelectionFg[i] = ctConfigImported.userStyleSelectionFg[i];
        pConfig->userStyleSelectionBg[i] = ctConfigImported.userStyleSelectionBg[i];
        pConfig->userStyleCursor[i] = ctConfigImported.userStyleCursor[i];
        pConfig->userStyleCurrentLineBg[i] = ctConfigImported.userStyleCurrentLineBg[i];
        pConfig->userStyleLineNumbersFg[i] = ctConfigImported.userStyleLineNumbersFg[i];
        pConfig->userStyleLineNumbersBg[i] = ctConfigImported.userStyleLineNumbersBg[i];
        pConfig->update_user_style(n);
    }
    pConfig->toolbarUiList = ctConfigImported.toolbarUiList;
    pConfig->systrayOn = ctConfigImported.systrayOn;
    pConfig->startOnSystray = ctConfigImported.startOnSystray;
    pConfig->useAppInd = ctConfigImported.useAppInd;
    pConfig->autosaveOn = ctConfigImported.autosaveOn;
    pConfig->autosaveVal = ctConfigImported.autosaveVal;
    pConfig->bookmarksInTopMenu = ctConfigImported.bookmarksInTopMenu;
    pConfig->checkVersion = ctConfigImported.checkVersion;
    pConfig->wordCountOn = ctConfigImported.wordCountOn;
    pConfig->reloadDocLast = ctConfigImported.reloadDocLast;
    pConfig->winTitleShowDocDir = ctConfigImported.winTitleShowDocDir;
    pConfig->nodeNameHeaderShowFullPath = ctConfigImported.nodeNameHeaderShowFullPath;
    pConfig->modTimeSentinel = ctConfigImported.modTimeSentinel;
    pConfig->backupCopy = ctConfigImported.backupCopy;
    pConfig->backupNum = ctConfigImported.backupNum;
    pConfig->autosaveOnQuit = ctConfigImported.autosaveOnQuit;
    pConfig->customBackupDirOn = ctConfigImported.customBackupDirOn;
    pConfig->customBackupDir = ctConfigImported.customBackupDir;
    pConfig->limitUndoableSteps = ctConfigImported.limitUndoableSteps;
    for (const auto& currPair : ctConfigImported.customKbShortcuts) {
        pConfig->customKbShortcuts[currPair.first] = currPair.second;
    }
    pConfig->customCodexecTerm = ctConfigImported.customCodexecTerm;
    for (const auto& currPair : ctConfigImported.customCodexecType) {
        pConfig->customCodexecType[currPair.first] = currPair.second;
    }
    for (const auto& currPair : ctConfigImported.customCodexecExt) {
        pConfig->customCodexecExt[currPair.first] = currPair.second;
    }

    CtDialogs::info_dialog(_("This Change will have Effect Only After Restarting CherryTree"), *_pCtMainWin);
}

void CtActions::preferences_export()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    const time_t time = std::time(nullptr);
    args.curr_file_name = std::string{"config_"} + str::time_format("%Y.%m.%d_%H.%M.%S", time) + ".cfg";
    args.filter_name = _("Preferences File");
    args.filter_pattern.push_back("*.cfg");
    const std::string filepath = CtDialogs::file_save_as_dialog(args);
    _pCtMainWin->config_update_data_from_curr_status();
    _pCtMainWin->get_ct_config()->write_to_file(filepath);
}

void CtActions::command_palette()
{
    std::string id = CtDialogs::dialog_palette(_pCtMainWin);
    if (CtMenuAction* action = _pCtMainWin->get_ct_menu().find_action(id))
        action->run_action();
}
