/*
 * ct_config.cc
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

#include <iostream>
#include <gtkmm.h>
#include "ct_config.h"
#include "ct_misc_utils.h"


CtConfig::CtConfig()
 : _filepath(Glib::build_filename(Glib::get_user_config_dir(), "cherrytree", "config.cfg"))
{
    bool config_found = _checkLoadFromFile();
    std::cout << _filepath << " " << (config_found ? "parsed":"missing") << std::endl;
}

CtConfig::~CtConfig()
{
    //std::cout << "~CtConfig()" << std::endl;
    if (_pKeyFile != nullptr)
    {
        delete _pKeyFile;
    }
}

bool CtConfig::_populateBoolFromKeyfile(const gchar* key, bool* pTarget)
{
    bool gotIt{false};
    if (_pKeyFile->has_key(_currentGroup, key))
    {
        try
        {
            *pTarget = _pKeyFile->get_boolean(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror)
        {
            if (kferror.code() == Glib::KeyFileError::Code::INVALID_VALUE)
            {
                // booleans from python ConfigParser
                Glib::ustring bool_str = _pKeyFile->get_value(_currentGroup, key);
                *pTarget = (bool_str == "True");
                gotIt = true;
            }
            else
            {
                _unexpectedKeyfileError(key, kferror);
            }
        }
    }
    return gotIt;
}

bool CtConfig::_populateIntFromKeyfile(const gchar* key, int* pTarget)
{
    bool gotIt{false};
    if (_pKeyFile->has_key(_currentGroup, key))
    {
        try
        {
            *pTarget = _pKeyFile->get_integer(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror)
        {
            _unexpectedKeyfileError(key, kferror);
        }
    }
    return gotIt;
}

bool CtConfig::_populateDoubleFromKeyfile(const gchar* key, double* pTarget)
{
    bool gotIt{false};
    if (_pKeyFile->has_key(_currentGroup, key))
    {
        try
        {
            *pTarget = _pKeyFile->get_double(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror)
        {
            _unexpectedKeyfileError(key, kferror);
        }
    }
    return gotIt;
}

void CtConfig::_populateMapFromCurrentGroup(std::map<std::string, std::string> *p_map)
{
    for (std::string key : _pKeyFile->get_keys(_currentGroup))
    {
        (*p_map)[key] = _pKeyFile->get_value(_currentGroup, key);
    }
}

void CtConfig::_unexpectedKeyfileError(const gchar* key, const Glib::KeyFileError& kferror)
{
    std::cerr << "!! " << key << " error code " << kferror.code() << std::endl;
}

void CtConfig::_populateFromKeyfile()
{
    guint8  i;
    const uint8_t MAX_TEMP_KEY_SIZE {16};
    gchar temp_key[MAX_TEMP_KEY_SIZE];
    // [state]
    _currentGroup = "state";
    _populateStringFromKeyfile("file_dir", &fileDir);
    _populateStringFromKeyfile("file_name", &fileName);
    _populateBoolFromKeyfile("toolbar_visible", &toolbarVisible);
    _populateBoolFromKeyfile("win_is_maximized", &winIsMaximised);
    _populateIntFromKeyfile("win_position_x", &winRect[0]);
    _populateIntFromKeyfile("win_position_y", &winRect[1]);
    _populateIntFromKeyfile("win_size_w", &winRect[2]);
    _populateIntFromKeyfile("win_size_h", &winRect[3]);
    _populateIntFromKeyfile("hpaned_pos", &hpanedPos);
    _populateBoolFromKeyfile("tree_visible", &treeVisible);
    if (_populateStringFromKeyfile("node_path", &nodePath))
    {
        CtStrUtil::replaceInString(nodePath, " ", ":");
        _populateIntFromKeyfile("cursor_position", &cursorPosition);
    }
    for (i=0; i<CtConst::MAX_RECENT_DOCS; i++)
    {
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "doc_%d", i);
        Glib::ustring recent_doc;
        if (!_populateStringFromKeyfile(temp_key, &recent_doc))
        {
            break;
        }
        recentDocs.push_back(recent_doc);
    }
    _populateStringFromKeyfile("pick_dir_import", &pickDirImport);
    _populateStringFromKeyfile("pick_dir_export", &pickDirExport);
    _populateStringFromKeyfile("pick_dir_file", &pickDirFile);
    _populateStringFromKeyfile("pick_dir_img", &pickDirImg);
    _populateStringFromKeyfile("pick_dir_csv", &pickDirCsv);
    _populateStringFromKeyfile("pick_dir_cbox", &pickDirCbox);
    _populateStringFromKeyfile("link_type", &linkType);
    _populateBoolFromKeyfile("show_node_name_header", &showNodeNameHeader);
    _populateIntFromKeyfile("nodes_on_node_name_header", &nodesOnNodeNameHeader);
    _populateIntFromKeyfile("toolbar_icon_size", &toolbarIconSize);
    _populateStringFromKeyfile("fg", &currColors['f']);
    _populateStringFromKeyfile("bg", &currColors['b']);
    _populateStringFromKeyfile("nn", &currColors['n']);

    // [tree]
    _currentGroup = "tree";
    int rest_exp_coll;
    if (_populateIntFromKeyfile("rest_exp_coll", &rest_exp_coll))
    {
        restoreExpColl = static_cast<CtRestoreExpColl>(rest_exp_coll);
    }
    _populateStringFromKeyfile("expanded_collapsed_string", &expandedCollapsedString);
    recentDocsRestore.resize(CtConst::MAX_RECENT_DOCS_RESTORE);
    for (i=0; i<=CtConst::MAX_RECENT_DOCS_RESTORE; i++)
    {
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollnam%d", i+1);
        if (!_populateStringFromKeyfile(temp_key, &recentDocsRestore[i].doc_name))
        {
            break;
        }
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollstr%d", i+1);
        _populateStringFromKeyfile(temp_key, &recentDocsRestore[i].exp_coll_str);
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollsel%d", i+1);
        _populateStringFromKeyfile(temp_key, &recentDocsRestore[i].node_path);
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollcur%d", i+1);
        _populateIntFromKeyfile(temp_key, &recentDocsRestore[i].cursor_pos);
    }
    _populateBoolFromKeyfile("nodes_bookm_exp", &nodesBookmExp);
    _populateStringFromKeyfile("nodes_icons", &nodesIcons);
    _populateBoolFromKeyfile("aux_icon_hide", &auxIconHide);
    _populateIntFromKeyfile("default_icon_text", &defaultIconText);
    _populateBoolFromKeyfile("tree_right_side", &treeRightSide);
    _populateIntFromKeyfile("cherry_wrap_width", &cherryWrapWidth);
    _populateBoolFromKeyfile("tree_click_focus_text", &treeClickFocusText);
    _populateBoolFromKeyfile("tree_click_expand", &treeClickExpand);

    // [editor]
    _currentGroup = "editor";
    _populateStringFromKeyfile("syntax_highlighting", &syntaxHighlighting);
    _populateStringFromKeyfile("auto_syn_highl", &autoSynHighl);
    _populateStringFromKeyfile("style_scheme", &styleSchemeId);
    if (_populateBoolFromKeyfile("enable_spell_check", &enableSpellCheck))
    {
        _populateStringFromKeyfile("spell_check_lang", &spellCheckLang);
    }
    _populateBoolFromKeyfile("show_line_numbers", &showLineNumbers);
    _populateBoolFromKeyfile("spaces_instead_tabs", &spacesInsteadTabs);
    _populateIntFromKeyfile("tabs_width", &tabsWidth);
    _populateIntFromKeyfile("anchor_size", &anchorSize);
    _populateIntFromKeyfile("embfile_size", &embfileSize);
    _populateBoolFromKeyfile("embfile_show_filename", &embfileShowFileName);
    _populateIntFromKeyfile("embfile_max_size", &embfileMaxSize);
    _populateBoolFromKeyfile("line_wrapping", &lineWrapping);
    _populateBoolFromKeyfile("auto_smart_quotes", &autoSmartQuotes);
    _populateIntFromKeyfile("wrapping_indent", &wrappingIndent);
    _populateBoolFromKeyfile("auto_indent", &autoIndent);
    _populateBoolFromKeyfile("rt_show_white_spaces", &rtShowWhiteSpaces);
    _populateBoolFromKeyfile("pt_show_white_spaces", &ptShowWhiteSpaces);
    _populateBoolFromKeyfile("rt_highl_curr_line", &rtHighlCurrLine);
    _populateBoolFromKeyfile("pt_highl_curr_line", &ptHighlCurrLine);
    _populateIntFromKeyfile("space_around_lines", &spaceAroundLines);
    _populateIntFromKeyfile("relative_wrapped_space", &relativeWrappedSpace);
    _populateStringFromKeyfile("h_rule", &hRule);
    _populateStringFromKeyfile("special_chars", &specialChars);
    _populateStringFromKeyfile("selword_chars", &selwordChars);
    _populateStringFromKeyfile("chars_listbul", &charsListbul);
    _populateStringFromKeyfile("chars_toc", &charsToc);
    _populateStringFromKeyfile("chars_todo", &charsTodo);
    _populateStringFromKeyfile("latest_tag_prop", &latestTagProp);
    _populateStringFromKeyfile("latest_tag_val", &latestTagVal);
    _populateStringFromKeyfile("timestamp_format", &timestampFormat);
    _populateBoolFromKeyfile("links_underline", &linksUnderline);
    _populateBoolFromKeyfile("links_relative", &linksRelative);
    _populateBoolFromKeyfile("weblink_custom_on", &weblinkCustomOn);
    _populateBoolFromKeyfile("filelink_custom_on", &filelinkCustomOn);
    _populateBoolFromKeyfile("folderlink_custom_on", &folderlinkCustomOn);
    _populateStringFromKeyfile("weblink_custom_act", &weblinkCustomAct);
    _populateStringFromKeyfile("filelink_custom_act", &filelinkCustomAct);
    _populateStringFromKeyfile("folderlink_custom_act", &folderlinkCustomAct);

    // [codebox]
    _currentGroup = "codebox";
    _populateDoubleFromKeyfile("codebox_width", &codeboxWidth);
    _populateDoubleFromKeyfile("codebox_height", &codeboxHeight);
    _populateBoolFromKeyfile("codebox_width_pixels", &codeboxWidthPixels);
    _populateBoolFromKeyfile("codebox_line_num", &codeboxLineNum);
    _populateBoolFromKeyfile("codebox_match_bra", &codeboxMatchBra);
    _populateStringFromKeyfile("codebox_syn_highl", &codeboxSynHighl);
    _populateBoolFromKeyfile("codebox_auto_resize", &codeboxAutoResize);

    // [table]
    _currentGroup = "table";
    _populateIntFromKeyfile("table_rows", &tableRows);
    _populateIntFromKeyfile("table_columns", &tableColumns);
    int table_col_mode;
    if (_populateIntFromKeyfile("table_col_mode", &table_col_mode))
    {
        tableColMode = static_cast<CtTableColMode>(table_col_mode);
    }
    _populateIntFromKeyfile("table_col_min", &tableColMin);
    _populateIntFromKeyfile("table_col_max", &tableColMax);

    // [fonts]
    _currentGroup = "fonts";
    _populateStringFromKeyfile("rt_font", &rtFont);
    _populateStringFromKeyfile("pt_font", &ptFont);
    _populateStringFromKeyfile("tree_font", &treeFont);
    _populateStringFromKeyfile("code_font", &codeFont);

    // [colors]
    _currentGroup = "colors";
    _populateStringFromKeyfile("rt_def_fg", &rtDefFg);
    _populateStringFromKeyfile("rt_def_bg", &rtDefBg);
    _populateStringFromKeyfile("tt_def_fg", &ttDefFg);
    _populateStringFromKeyfile("tt_def_bg", &ttDefBg);
    _populateStringFromKeyfile("monospace_bg", &monospaceBg);
    _populateStringFromKeyfile("color_palette", &colorPalette);
    _populateStringFromKeyfile("col_link_webs", &colLinkWebs);
    _populateStringFromKeyfile("col_link_node", &colLinkNode);
    _populateStringFromKeyfile("col_link_file", &colLinkFile);
    _populateStringFromKeyfile("col_link_fold", &colLinkFold);

    // [misc]
    _currentGroup = "misc";
    _populateStringFromKeyfile("toolbar_ui_list", &toolbarUiList);
    _populateBoolFromKeyfile("systray", &systrayOn);
    _populateBoolFromKeyfile("start_on_systray", &startOnSystray);
    _populateBoolFromKeyfile("use_appind", &useAppInd);
    _populateBoolFromKeyfile("autosave_on", &autosaveOn);
    _populateIntFromKeyfile("autosave_val", &autosaveVal);
    _populateBoolFromKeyfile("check_version", &checkVersion);
    _populateBoolFromKeyfile("word_count", &wordCountOn);
    _populateBoolFromKeyfile("reload_doc_last", &reloadDocLast);
    _populateBoolFromKeyfile("mod_time_sentinel", &modTimeSentinel);
    _populateBoolFromKeyfile("backup_copy", &backupCopy);
    _populateIntFromKeyfile("backup_num", &backupNum);
    _populateBoolFromKeyfile("autosave_on_quit", &autosaveOnQuit);
    _populateIntFromKeyfile("limit_undoable_steps", &limitUndoableSteps);

    // [keyboard]
    _currentGroup = "keyboard";
    _populateMapFromCurrentGroup(&customKbShortcuts);

    // [codexec_term]
    _currentGroup = "codexec_term";
    _populateStringFromKeyfile("custom_codexec_term", &customCodexecTerm);

    // [codexec_type]
    _currentGroup = "codexec_type";
    _populateMapFromCurrentGroup(&customCodexecType);

    // [codexec_ext]
    _currentGroup = "codexec_ext";
    _populateMapFromCurrentGroup(&customCodexecExt);
}

bool CtConfig::_checkLoadFromFile()
{
    if (Glib::file_test(_filepath, Glib::FILE_TEST_EXISTS))
    {
        _pKeyFile = new Glib::KeyFile();
        _pKeyFile->load_from_file(_filepath);
        _populateFromKeyfile();
        return true;
    }
    return false;
}
