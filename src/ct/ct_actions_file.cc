/*
 * ct_actions_file.cc
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
    if (not _is_tree_not_empty_or_error()) {
        return;
    }
    CtDialogs::CtStorageSelectArgs storageSelArgs{};
    storageSelArgs.showAutosaveOptions = true;
    fs::path currDocFilepath = _pCtMainWin->get_ct_storage()->get_file_path();
    if (not currDocFilepath.empty()) {
        storageSelArgs.ctDocType = fs::get_doc_type_from_file_ext(currDocFilepath);
        storageSelArgs.ctDocEncrypt = fs::get_doc_encrypt_from_file_ext(currDocFilepath);
    }
    if (not CtDialogs::choose_data_storage_dialog(_pCtMainWin, storageSelArgs)) {
        return;
    }
    CtDialogs::CtFileSelectArgs fileSelArgs{};
    if (not currDocFilepath.empty()) {
        fileSelArgs.curr_folder = currDocFilepath.parent_path();
        fs::path suggested_basename = currDocFilepath.filename();
        fileSelArgs.curr_file_name = suggested_basename.stem() + CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
    }
    std::string filepath;
    if (CtDocType::MultiFile == storageSelArgs.ctDocType) {
        filepath = CtDialogs::folder_save_as_dialog(_pCtMainWin, fileSelArgs);
    }
    else {
        fileSelArgs.filter_name = _("CherryTree File");
        std::string fileExtension = CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
        fileSelArgs.filter_pattern.push_back(std::string{CtConst::CHAR_STAR}+fileExtension);
        filepath = CtDialogs::file_save_as_dialog(_pCtMainWin, fileSelArgs);
    }
    if (filepath.empty()) {
        return;
    }
    CtMiscUtil::filepath_extension_fix(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt, filepath);
    _pCtMainWin->file_save_as(filepath, storageSelArgs.ctDocType, storageSelArgs.password);
}

void CtActions::folder_open()
{
    const std::string folder_path = CtDialogs::folder_select_dialog(_pCtMainWin, _pCtMainWin->get_ct_storage()->get_file_dir().string());

    if (folder_path.empty()) return;

    _pCtMainWin->file_open(folder_path, ""/*node*/, ""/*anchor*/);
}

void CtActions::file_open()
{
    CtDialogs::CtFileSelectArgs args{};
    args.curr_folder = _pCtMainWin->get_ct_storage()->get_file_dir();
    args.filter_name = _("CherryTree File");
    args.filter_pattern.push_back("*.ctb"); // macos doesn't understand *.ct*
    args.filter_pattern.push_back("*.ctx");
    args.filter_pattern.push_back("*.ctd");
    args.filter_pattern.push_back("*.ctz");

    const std::string file_path = CtDialogs::file_select_dialog(_pCtMainWin, args);

    if (file_path.empty()) return;

    _pCtMainWin->file_open(file_path, ""/*node*/, ""/*anchor*/);
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
    CtDialogs::CtFileSelectArgs args{};
    args.filter_name = _("Preferences File");
    args.filter_pattern.push_back("*.cfg");
    const std::string filepath = CtDialogs::file_select_dialog(_pCtMainWin, args);
    if (filepath.empty()) return;
    CtConfig ctConfigImported{filepath};
    if (not ctConfigImported.getInitLoadFromFileOk()) return;

    _pCtConfig->toolbarVisible = ctConfigImported.toolbarVisible;
    _pCtConfig->statusbarVisible = ctConfigImported.statusbarVisible;
    _pCtConfig->treeLinesVisible = ctConfigImported.treeLinesVisible;
    _pCtConfig->hpanedPos = ctConfigImported.hpanedPos;
    _pCtConfig->vpanedPos = ctConfigImported.vpanedPos;
    _pCtConfig->treeVisible = ctConfigImported.treeVisible;
    _pCtConfig->vteVisible = ctConfigImported.vteVisible;
    _pCtConfig->menubarVisible = ctConfigImported.menubarVisible;
    _pCtConfig->linkType = ctConfigImported.linkType;
    _pCtConfig->menubarInTitlebar = ctConfigImported.menubarInTitlebar;
    _pCtConfig->showNodeNameHeader = ctConfigImported.showNodeNameHeader;
    _pCtConfig->nodesOnNodeNameHeader = ctConfigImported.nodesOnNodeNameHeader;
    _pCtConfig->toolbarIconSize = ctConfigImported.toolbarIconSize;
    _pCtConfig->currColors['f'] = ctConfigImported.currColors['f'];
    _pCtConfig->currColors['b'] = ctConfigImported.currColors['b'];
    _pCtConfig->currColors['n'] = ctConfigImported.currColors['n'];
    _pCtConfig->restoreExpColl = ctConfigImported.restoreExpColl;
    _pCtConfig->nodesBookmExp = ctConfigImported.nodesBookmExp;
    _pCtConfig->nodesIcons = ctConfigImported.nodesIcons;
    _pCtConfig->auxIconHide = ctConfigImported.auxIconHide;
    _pCtConfig->defaultIconText = ctConfigImported.defaultIconText;
    _pCtConfig->lastIconSel = ctConfigImported.lastIconSel;
    _pCtConfig->treeRightSide = ctConfigImported.treeRightSide;
    _pCtConfig->cherryWrapEnabled = ctConfigImported.cherryWrapEnabled;
    _pCtConfig->cherryWrapWidth = ctConfigImported.cherryWrapWidth;
    _pCtConfig->treeClickFocusText = ctConfigImported.treeClickFocusText;
    _pCtConfig->treeClickExpand = ctConfigImported.treeClickExpand;
    _pCtConfig->syntaxHighlighting = ctConfigImported.syntaxHighlighting;
    _pCtConfig->autoSynHighl = ctConfigImported.autoSynHighl;
    _pCtConfig->rtStyleScheme = ctConfigImported.rtStyleScheme;
    _pCtConfig->ptStyleScheme = ctConfigImported.ptStyleScheme;
    _pCtConfig->taStyleScheme = ctConfigImported.taStyleScheme;
    _pCtConfig->coStyleScheme = ctConfigImported.coStyleScheme;
    _pCtConfig->enableSpellCheck = ctConfigImported.enableSpellCheck;
    _pCtConfig->spellCheckLang = ctConfigImported.spellCheckLang;
    _pCtConfig->showLineNumbers = ctConfigImported.showLineNumbers;
    _pCtConfig->scrollBeyondLastLine = ctConfigImported.scrollBeyondLastLine;
    _pCtConfig->spacesInsteadTabs = ctConfigImported.spacesInsteadTabs;
    _pCtConfig->tabsWidth = ctConfigImported.tabsWidth;
    _pCtConfig->anchorSize = ctConfigImported.anchorSize;
    _pCtConfig->latexSizeDpi = ctConfigImported.latexSizeDpi;
    _pCtConfig->embfileIconSize = ctConfigImported.embfileIconSize;
    _pCtConfig->embfileShowFileName = ctConfigImported.embfileShowFileName;
    _pCtConfig->embfileMaxSize = ctConfigImported.embfileMaxSize;
    _pCtConfig->lineWrapping = ctConfigImported.lineWrapping;
    _pCtConfig->autoSmartQuotes = ctConfigImported.autoSmartQuotes;
    _pCtConfig->urlAutoLink = ctConfigImported.urlAutoLink;
    _pCtConfig->camelCaseAutoLink = ctConfigImported.camelCaseAutoLink;
    _pCtConfig->tripleClickParagraph = ctConfigImported.tripleClickParagraph;
    _pCtConfig->enableSymbolAutoreplace = ctConfigImported.enableSymbolAutoreplace;
    _pCtConfig->wrappingIndent = ctConfigImported.wrappingIndent;
    _pCtConfig->autoIndent = ctConfigImported.autoIndent;
    _pCtConfig->codeExecConfirm = ctConfigImported.codeExecConfirm;
    _pCtConfig->codeExecVte = ctConfigImported.codeExecVte;
    _pCtConfig->vteShell = ctConfigImported.vteShell;
    _pCtConfig->rtShowWhiteSpaces = ctConfigImported.rtShowWhiteSpaces;
    _pCtConfig->ptShowWhiteSpaces = ctConfigImported.ptShowWhiteSpaces;
    _pCtConfig->rtHighlCurrLine = ctConfigImported.rtHighlCurrLine;
    _pCtConfig->ptHighlCurrLine = ctConfigImported.ptHighlCurrLine;
    _pCtConfig->rtHighlMatchBra = ctConfigImported.rtHighlMatchBra;
    _pCtConfig->ptHighlMatchBra = ctConfigImported.ptHighlMatchBra;
    _pCtConfig->spaceAroundLines = ctConfigImported.spaceAroundLines;
    _pCtConfig->relativeWrappedSpace = ctConfigImported.relativeWrappedSpace;
    _pCtConfig->hRule = ctConfigImported.hRule;
    _pCtConfig->specialChars = ctConfigImported.specialChars;
    _pCtConfig->lastSpecialChar = ctConfigImported.lastSpecialChar;
    _pCtConfig->selwordChars = ctConfigImported.selwordChars;
    _pCtConfig->charsListbul = ctConfigImported.charsListbul;
    _pCtConfig->charsToc = ctConfigImported.charsToc;
    _pCtConfig->charsTodo = ctConfigImported.charsTodo;
    _pCtConfig->chars_smart_dquote = ctConfigImported.chars_smart_dquote;
    _pCtConfig->chars_smart_squote = ctConfigImported.chars_smart_squote;
    _pCtConfig->latestTagProp = ctConfigImported.latestTagProp;
    _pCtConfig->latestTagVal = ctConfigImported.latestTagVal;
    _pCtConfig->timestampFormat = ctConfigImported.timestampFormat;
    _pCtConfig->linksUnderline = ctConfigImported.linksUnderline;
    _pCtConfig->linksRelative = ctConfigImported.linksRelative;
    _pCtConfig->weblinkCustomOn = ctConfigImported.weblinkCustomOn;
    _pCtConfig->filelinkCustomOn = ctConfigImported.filelinkCustomOn;
    _pCtConfig->folderlinkCustomOn = ctConfigImported.folderlinkCustomOn;
    _pCtConfig->weblinkCustomAct = ctConfigImported.weblinkCustomAct;
    _pCtConfig->filelinkCustomAct = ctConfigImported.filelinkCustomAct;
    _pCtConfig->folderlinkCustomAct = ctConfigImported.folderlinkCustomAct;
    _pCtConfig->codeboxWidth = ctConfigImported.codeboxWidth;
    _pCtConfig->codeboxHeight = ctConfigImported.codeboxHeight;
    _pCtConfig->codeboxWidthPixels = ctConfigImported.codeboxWidthPixels;
    _pCtConfig->codeboxLineNum = ctConfigImported.codeboxLineNum;
    _pCtConfig->codeboxMatchBra = ctConfigImported.codeboxMatchBra;
    _pCtConfig->codeboxSynHighl = ctConfigImported.codeboxSynHighl;
    _pCtConfig->codeboxAutoResize = ctConfigImported.codeboxAutoResize;
    _pCtConfig->tableRows = ctConfigImported.tableRows;
    _pCtConfig->tableColumns = ctConfigImported.tableColumns;
    _pCtConfig->tableColWidthDefault = ctConfigImported.tableColWidthDefault;
    _pCtConfig->tableCellsGoLight = ctConfigImported.tableCellsGoLight;
    _pCtConfig->rtFont = ctConfigImported.rtFont;
    _pCtConfig->ptFont = ctConfigImported.ptFont;
    _pCtConfig->treeFont = ctConfigImported.treeFont;
    _pCtConfig->codeFont = ctConfigImported.codeFont;
    _pCtConfig->vteFont = ctConfigImported.vteFont;
    _pCtConfig->ttDefFg = ctConfigImported.ttDefFg;
    _pCtConfig->ttDefBg = ctConfigImported.ttDefBg;
    _pCtConfig->ttSelFg = ctConfigImported.ttSelFg;
    _pCtConfig->ttSelBg = ctConfigImported.ttSelBg;
    _pCtConfig->scalableH1.deserialise(ctConfigImported.scalableH1.serialise().c_str());
    _pCtConfig->scalableH2.deserialise(ctConfigImported.scalableH2.serialise().c_str());
    _pCtConfig->scalableH3.deserialise(ctConfigImported.scalableH3.serialise().c_str());
    _pCtConfig->scalableH4.deserialise(ctConfigImported.scalableH4.serialise().c_str());
    _pCtConfig->scalableH5.deserialise(ctConfigImported.scalableH5.serialise().c_str());
    _pCtConfig->scalableH6.deserialise(ctConfigImported.scalableH6.serialise().c_str());
    _pCtConfig->scalableSmall.deserialise(ctConfigImported.scalableSmall.serialise().c_str());
    _pCtConfig->monospaceFg = ctConfigImported.monospaceFg;
    _pCtConfig->monospaceBg = ctConfigImported.monospaceBg;
    _pCtConfig->msDedicatedFont = ctConfigImported.msDedicatedFont;
    _pCtConfig->monospaceFont = ctConfigImported.monospaceFont;
    _pCtConfig->colLinkWebs = ctConfigImported.colLinkWebs;
    _pCtConfig->colLinkNode = ctConfigImported.colLinkNode;
    _pCtConfig->colLinkFile = ctConfigImported.colLinkFile;
    _pCtConfig->colLinkFold = ctConfigImported.colLinkFold;
    for (unsigned n = 1; n <= CtConst::NUM_USER_STYLES; ++n) {
        const unsigned i = n-1;
        _pCtConfig->userStyleTextFg[i] = ctConfigImported.userStyleTextFg[i];
        _pCtConfig->userStyleTextBg[i] = ctConfigImported.userStyleTextBg[i];
        _pCtConfig->userStyleSelectionFg[i] = ctConfigImported.userStyleSelectionFg[i];
        _pCtConfig->userStyleSelectionBg[i] = ctConfigImported.userStyleSelectionBg[i];
        _pCtConfig->userStyleCursor[i] = ctConfigImported.userStyleCursor[i];
        _pCtConfig->userStyleCurrentLineBg[i] = ctConfigImported.userStyleCurrentLineBg[i];
        _pCtConfig->userStyleLineNumbersFg[i] = ctConfigImported.userStyleLineNumbersFg[i];
        _pCtConfig->userStyleLineNumbersBg[i] = ctConfigImported.userStyleLineNumbersBg[i];
        _pCtConfig->update_user_style(n);
    }
    _pCtConfig->toolbarUiList = ctConfigImported.toolbarUiList;
    _pCtConfig->systrayOn = ctConfigImported.systrayOn;
    _pCtConfig->startOnSystray = ctConfigImported.startOnSystray;
    _pCtConfig->useAppInd = ctConfigImported.useAppInd;
    _pCtConfig->autosaveOn = ctConfigImported.autosaveOn;
    _pCtConfig->autosaveVal = ctConfigImported.autosaveVal;
    _pCtConfig->bookmarksInTopMenu = ctConfigImported.bookmarksInTopMenu;
    _pCtConfig->checkVersion = ctConfigImported.checkVersion;
    _pCtConfig->wordCountOn = ctConfigImported.wordCountOn;
    _pCtConfig->reloadDocLast = ctConfigImported.reloadDocLast;
    _pCtConfig->winTitleShowDocDir = ctConfigImported.winTitleShowDocDir;
    _pCtConfig->nodeNameHeaderShowFullPath = ctConfigImported.nodeNameHeaderShowFullPath;
    _pCtConfig->modTimeSentinel = ctConfigImported.modTimeSentinel;
    _pCtConfig->backupCopy = ctConfigImported.backupCopy;
    _pCtConfig->backupNum = ctConfigImported.backupNum;
    _pCtConfig->autosaveOnQuit = ctConfigImported.autosaveOnQuit;
    _pCtConfig->customBackupDirOn = ctConfigImported.customBackupDirOn;
    _pCtConfig->customBackupDir = ctConfigImported.customBackupDir;
    _pCtConfig->limitUndoableSteps = ctConfigImported.limitUndoableSteps;
    for (const auto& currPair : ctConfigImported.customKbShortcuts) {
        _pCtConfig->customKbShortcuts[currPair.first] = currPair.second;
    }
    _pCtConfig->customCodexecTerm = ctConfigImported.customCodexecTerm;
    for (const auto& currPair : ctConfigImported.customCodexecType) {
        _pCtConfig->customCodexecType[currPair.first] = currPair.second;
    }
    for (const auto& currPair : ctConfigImported.customCodexecExt) {
        _pCtConfig->customCodexecExt[currPair.first] = currPair.second;
    }

    CtDialogs::info_dialog(_("This Change will have Effect Only After Restarting CherryTree."), *_pCtMainWin);
}

void CtActions::preferences_export()
{
    CtDialogs::CtFileSelectArgs args{};
    const time_t time = std::time(nullptr);
    args.curr_file_name = std::string{"config_"} + str::time_format("%Y.%m.%d_%H.%M.%S", time) + ".cfg";
    args.filter_name = _("Preferences File");
    args.filter_pattern.push_back("*.cfg");
    const std::string filepath = CtDialogs::file_save_as_dialog(_pCtMainWin, args);
    _pCtMainWin->config_update_data_from_curr_status();
    _pCtConfig->write_to_file(filepath);
}

void CtActions::command_palette()
{
    std::string id = CtDialogs::dialog_palette(_pCtMainWin);
    if (CtMenuAction* action = _pCtMainWin->get_ct_menu().find_action(id))
        action->run_action();
}
