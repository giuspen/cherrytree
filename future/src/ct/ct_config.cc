/*
 * ct_config.cc
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
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

#include <iostream>
#include <gtkmm.h>
#include "ct_config.h"
#include "ct_misc_utils.h"

const std::string CtConfig::_defaultFilepath{Glib::build_filename(Glib::get_user_config_dir(), CtConst::APP_NAME, "config.cfg")};

CtConfig::CtConfig()
{
    (void)load_from_file();
}

CtConfig::~CtConfig()
{
    //std::cout << "~CtConfig()" << std::endl;
}

bool CtConfig::load_from_file(const std::string& filepath)
{
    if (Glib::file_test(filepath, Glib::FILE_TEST_EXISTS))
    {
        _uKeyFile.reset(new Glib::KeyFile);
        _uKeyFile->load_from_file(filepath);
        _populate_data_from_keyfile();
        _uKeyFile.reset(nullptr);
        std::cout << filepath << " parsed" << std::endl;
        return true;
    }
    std::cout << filepath << " missing" << std::endl;
    return false;
}

bool CtConfig::write_to_file(const std::string& filepath)
{
    _uKeyFile.reset(new Glib::KeyFile);
    _populate_keyfile_from_data();
    const bool writeSucceeded = _uKeyFile->save_to_file(filepath+"todo");
    _uKeyFile.reset(nullptr);
    return writeSucceeded;
}

bool CtConfig::_populate_bool_from_keyfile(const gchar* key, bool* pTarget)
{
    bool gotIt{false};
    if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key))
    {
        try
        {
            *pTarget = _uKeyFile->get_boolean(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror)
        {
            if (kferror.code() == Glib::KeyFileError::Code::INVALID_VALUE)
            {
                // booleans from python ConfigParser
                Glib::ustring bool_str = _uKeyFile->get_value(_currentGroup, key);
                *pTarget = (bool_str == "True");
                gotIt = true;
            }
            else
            {
                _unexpected_keyfile_error(key, kferror);
            }
        }
    }
    return gotIt;
}

bool CtConfig::_populate_int_from_keyfile(const gchar* key, int* pTarget)
{
    bool gotIt{false};
    if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key))
    {
        try
        {
            *pTarget = _uKeyFile->get_integer(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror)
        {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return gotIt;
}

bool CtConfig::_populate_double_from_key_file(const gchar* key, double* pTarget)
{
    bool gotIt{false};
    if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key))
    {
        try
        {
            *pTarget = _uKeyFile->get_double(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror)
        {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return gotIt;
}

void CtConfig::_populate_map_from_current_group(std::map<std::string, std::string> *p_map)
{
    if (_uKeyFile->has_group(_currentGroup))
    {
        for (std::string key : _uKeyFile->get_keys(_currentGroup))
        {
            (*p_map)[key] = _uKeyFile->get_value(_currentGroup, key);
        }
    }
}

void CtConfig::_unexpected_keyfile_error(const gchar* key, const Glib::KeyFileError& kferror)
{
    std::cerr << "!! " << key << " error code " << kferror.code() << std::endl;
}

void CtConfig::_populate_keyfile_from_data()
{
    // [state]
    _currentGroup = "state";
    guint i{0};
    for (const std::string& filepath : recentDocsFilepaths)
    {
        snprintf(_tempKey, _maxTempKeySize, "doc_%d", i);
        _uKeyFile->set_string(_currentGroup, _tempKey, filepath);
        const CtRecentDocsRestore::iterator mapIt = recentDocsRestore.find(filepath);
        if (mapIt != recentDocsRestore.end())
        {
            snprintf(_tempKey, _maxTempKeySize, "expcol_%d", i);
            _uKeyFile->set_string(_currentGroup, _tempKey, mapIt->second.exp_coll_str);
            snprintf(_tempKey, _maxTempKeySize, "nodep_%d", i);
            _uKeyFile->set_string(_currentGroup, _tempKey, mapIt->second.node_path);
            snprintf(_tempKey, _maxTempKeySize, "curs_%d", i);
            _uKeyFile->set_integer(_currentGroup, _tempKey, mapIt->second.cursor_pos);
        }
        ++i;
    }
    _uKeyFile->set_boolean(_currentGroup, "toolbar_visible", toolbarVisible);
    _uKeyFile->set_boolean(_currentGroup, "win_is_maximized", winIsMaximised);
    _uKeyFile->set_integer(_currentGroup, "win_position_x", winRect[0]);
    _uKeyFile->set_integer(_currentGroup, "win_position_y", winRect[1]);
    _uKeyFile->set_integer(_currentGroup, "win_size_w", winRect[2]);
    _uKeyFile->set_integer(_currentGroup, "win_size_h", winRect[3]);
    _uKeyFile->set_integer(_currentGroup, "hpaned_pos", hpanedPos);
    _uKeyFile->set_boolean(_currentGroup, "tree_visible", treeVisible);
    _uKeyFile->set_string(_currentGroup, "pick_dir_import", pickDirImport);
    _uKeyFile->set_string(_currentGroup, "pick_dir_export", pickDirExport);
    _uKeyFile->set_string(_currentGroup, "pick_dir_file", pickDirFile);
    _uKeyFile->set_string(_currentGroup, "pick_dir_img", pickDirImg);
    _uKeyFile->set_string(_currentGroup, "pick_dir_csv", pickDirCsv);
    _uKeyFile->set_string(_currentGroup, "pick_dir_cbox", pickDirCbox);
    _uKeyFile->set_string(_currentGroup, "link_type", linkType);
    _uKeyFile->set_boolean(_currentGroup, "show_node_name_header", showNodeNameHeader);
    _uKeyFile->set_integer(_currentGroup, "nodes_on_node_name_header", nodesOnNodeNameHeader);
    _uKeyFile->set_integer(_currentGroup, "toolbar_icon_size", toolbarIconSize);
    _uKeyFile->set_string(_currentGroup, "fg", currColors['f']);
    _uKeyFile->set_string(_currentGroup, "bg", currColors['b']);
    _uKeyFile->set_string(_currentGroup, "nn", currColors['n']);

    // [tree]
    _currentGroup = "tree";
    _uKeyFile->set_integer(_currentGroup, "rest_exp_coll", static_cast<int>(restoreExpColl));
    _uKeyFile->set_boolean(_currentGroup, "nodes_bookm_exp", nodesBookmExp);
    _uKeyFile->set_string(_currentGroup, "nodes_icons", nodesIcons);
    _uKeyFile->set_boolean(_currentGroup, "aux_icon_hide", auxIconHide);
    _uKeyFile->set_integer(_currentGroup, "default_icon_text", defaultIconText);
    _uKeyFile->set_boolean(_currentGroup, "tree_right_side", treeRightSide);
    _uKeyFile->set_integer(_currentGroup, "cherry_wrap_width", cherryWrapWidth);
    _uKeyFile->set_boolean(_currentGroup, "tree_click_focus_text", treeClickFocusText);
    _uKeyFile->set_boolean(_currentGroup, "tree_click_expand", treeClickExpand);
}

void CtConfig::_populate_data_from_keyfile()
{
    // [state]
    _currentGroup = "state";
    bool savedFromPyGtk{false};
    {
        std::string fileName, fileDir;
        if ( _populate_string_from_keyfile("file_name", &fileName) and
             _populate_string_from_keyfile("file_dir", &fileDir) )
        {
            const std::string filePath = Glib::build_filename(fileDir, fileName);
            recentDocsFilepaths.move_or_push_front(filePath);
            savedFromPyGtk = true;
        }
    }
    for (guint i=0; i<recentDocsFilepaths.maxSize; ++i)
    {
        snprintf(_tempKey, _maxTempKeySize, "doc_%d", i);
        std::string filepath;
        if (not _populate_string_from_keyfile(_tempKey, &filepath))
        {
            break;
        }
        recentDocsFilepaths.push_back(filepath);
        if (not savedFromPyGtk)
        {
            CtRecentDocRestore recentDocRestore;
            snprintf(_tempKey, _maxTempKeySize, "nodep_%d", i);
            if (_populate_string_from_keyfile(_tempKey, &recentDocRestore.node_path))
            {
                snprintf(_tempKey, _maxTempKeySize, "expcol_%d", i);
                _populate_string_from_keyfile(_tempKey, &recentDocRestore.exp_coll_str);
                snprintf(_tempKey, _maxTempKeySize, "curs_%d", i);
                _populate_int_from_keyfile(_tempKey, &recentDocRestore.cursor_pos);
                recentDocsRestore[filepath] = recentDocRestore;
            }
        }
    }
    _populate_bool_from_keyfile("toolbar_visible", &toolbarVisible);
    _populate_bool_from_keyfile("win_is_maximized", &winIsMaximised);
    _populate_int_from_keyfile("win_position_x", &winRect[0]);
    _populate_int_from_keyfile("win_position_y", &winRect[1]);
    _populate_int_from_keyfile("win_size_w", &winRect[2]);
    _populate_int_from_keyfile("win_size_h", &winRect[3]);
    _populate_int_from_keyfile("hpaned_pos", &hpanedPos);
    _populate_bool_from_keyfile("tree_visible", &treeVisible);
    if (savedFromPyGtk)
    {
        CtRecentDocRestore recentDocRestore;
        if (_populate_string_from_keyfile("node_path", &recentDocRestore.node_path))
        {
            str::replace(recentDocRestore.node_path, " ", ":");
            _populate_int_from_keyfile("cursor_position", &recentDocRestore.cursor_pos);
            recentDocsRestore[recentDocsFilepaths.front()] = recentDocRestore;
        }
    }
    _populate_string_from_keyfile("pick_dir_import", &pickDirImport);
    _populate_string_from_keyfile("pick_dir_export", &pickDirExport);
    _populate_string_from_keyfile("pick_dir_file", &pickDirFile);
    _populate_string_from_keyfile("pick_dir_img", &pickDirImg);
    _populate_string_from_keyfile("pick_dir_csv", &pickDirCsv);
    _populate_string_from_keyfile("pick_dir_cbox", &pickDirCbox);
    _populate_string_from_keyfile("link_type", &linkType);
    _populate_bool_from_keyfile("show_node_name_header", &showNodeNameHeader);
    _populate_int_from_keyfile("nodes_on_node_name_header", &nodesOnNodeNameHeader);
    _populate_int_from_keyfile("toolbar_icon_size", &toolbarIconSize);
    _populate_string_from_keyfile("fg", &currColors['f']);
    _populate_string_from_keyfile("bg", &currColors['b']);
    _populate_string_from_keyfile("nn", &currColors['n']);

    // [tree]
    _currentGroup = "tree";
    int rest_exp_coll;
    if (_populate_int_from_keyfile("rest_exp_coll", &rest_exp_coll))
    {
        restoreExpColl = static_cast<CtRestoreExpColl>(rest_exp_coll);
    }
    if (savedFromPyGtk)
    {
        std::string exp_coll_str;
        if (_populate_string_from_keyfile("expanded_collapsed_string", &exp_coll_str))
        {
            recentDocsRestore[recentDocsFilepaths.front()].exp_coll_str = exp_coll_str;
        }
        for (guint i=1; i<=3; ++i)
        {
            std::string docName;
            snprintf(_tempKey, _maxTempKeySize, "expcollnam%d", i);
            if (_populate_string_from_keyfile(_tempKey, &docName))
            {
                for (const std::string& filepath : recentDocsFilepaths)
                {
                    if (Glib::path_get_basename(filepath) == docName)
                    {
                        CtRecentDocRestore recentDocRestore;
                        snprintf(_tempKey, _maxTempKeySize, "expcollstr%d", i);
                        _populate_string_from_keyfile(_tempKey, &recentDocRestore.exp_coll_str);
                        snprintf(_tempKey, _maxTempKeySize, "expcollsel%d", i);
                        if (_populate_string_from_keyfile(_tempKey, &recentDocRestore.node_path))
                        {
                            str::replace(recentDocRestore.node_path, " ", ":");
                            snprintf(_tempKey, _maxTempKeySize, "expcollcur%d", i);
                            _populate_int_from_keyfile(_tempKey, &recentDocRestore.cursor_pos);
                        }
                        recentDocsRestore[filepath] = recentDocRestore;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
    _populate_bool_from_keyfile("nodes_bookm_exp", &nodesBookmExp);
    _populate_string_from_keyfile("nodes_icons", &nodesIcons);
    _populate_bool_from_keyfile("aux_icon_hide", &auxIconHide);
    _populate_int_from_keyfile("default_icon_text", &defaultIconText);
    _populate_bool_from_keyfile("tree_right_side", &treeRightSide);
    _populate_int_from_keyfile("cherry_wrap_width", &cherryWrapWidth);
    _populate_bool_from_keyfile("tree_click_focus_text", &treeClickFocusText);
    _populate_bool_from_keyfile("tree_click_expand", &treeClickExpand);

    // [editor]
    _currentGroup = "editor";
    _populate_string_from_keyfile("syntax_highlighting", &syntaxHighlighting);
    _populate_string_from_keyfile("auto_syn_highl", &autoSynHighl);
    _populate_string_from_keyfile("style_scheme", &styleSchemeId);
    if (_populate_bool_from_keyfile("enable_spell_check", &enableSpellCheck))
    {
        _populate_string_from_keyfile("spell_check_lang", &spellCheckLang);
    }
    _populate_bool_from_keyfile("show_line_numbers", &showLineNumbers);
    _populate_bool_from_keyfile("spaces_instead_tabs", &spacesInsteadTabs);
    _populate_int_from_keyfile("tabs_width", &tabsWidth);
    _populate_int_from_keyfile("anchor_size", &anchorSize);
    _populate_int_from_keyfile("embfile_size", &embfileSize);
    _populate_bool_from_keyfile("embfile_show_filename", &embfileShowFileName);
    _populate_int_from_keyfile("embfile_max_size", &embfileMaxSize);
    _populate_bool_from_keyfile("line_wrapping", &lineWrapping);
    _populate_bool_from_keyfile("auto_smart_quotes", &autoSmartQuotes);
    _populate_bool_from_keyfile("enable_symbol_autoreplace", &enableSymbolAutoreplace);
    _populate_int_from_keyfile("wrapping_indent", &wrappingIndent);
    _populate_bool_from_keyfile("auto_indent", &autoIndent);
    _populate_bool_from_keyfile("rt_show_white_spaces", &rtShowWhiteSpaces);
    _populate_bool_from_keyfile("pt_show_white_spaces", &ptShowWhiteSpaces);
    _populate_bool_from_keyfile("rt_highl_curr_line", &rtHighlCurrLine);
    _populate_bool_from_keyfile("pt_highl_curr_line", &ptHighlCurrLine);
    _populate_int_from_keyfile("space_around_lines", &spaceAroundLines);
    _populate_int_from_keyfile("relative_wrapped_space", &relativeWrappedSpace);
    _populate_string_from_keyfile("h_rule", &hRule);
    _populate_string_from_keyfile("special_chars", &specialChars);
    _populate_string_from_keyfile("selword_chars", &selwordChars);
    _populate_string_from_keyfile("chars_listbul", &charsListbul);
    _populate_string_from_keyfile("chars_toc", &charsToc);
    _populate_string_from_keyfile("chars_todo", &charsTodo);
    _populate_string_from_keyfile("chars_smart_dquote", &chars_smart_dquote);
    _populate_string_from_keyfile("chars_smart_squote", &chars_smart_squote);
    _populate_string_from_keyfile("latest_tag_prop", &latestTagProp);
    _populate_string_from_keyfile("latest_tag_val", &latestTagVal);
    _populate_string_from_keyfile("timestamp_format", &timestampFormat);
    _populate_bool_from_keyfile("links_underline", &linksUnderline);
    _populate_bool_from_keyfile("links_relative", &linksRelative);
    _populate_bool_from_keyfile("weblink_custom_on", &weblinkCustomOn);
    _populate_bool_from_keyfile("filelink_custom_on", &filelinkCustomOn);
    _populate_bool_from_keyfile("folderlink_custom_on", &folderlinkCustomOn);
    _populate_string_from_keyfile("weblink_custom_act", &weblinkCustomAct);
    _populate_string_from_keyfile("filelink_custom_act", &filelinkCustomAct);
    _populate_string_from_keyfile("folderlink_custom_act", &folderlinkCustomAct);

    // [codebox]
    _currentGroup = "codebox";
    _populate_double_from_key_file("codebox_width", &codeboxWidth);
    _populate_double_from_key_file("codebox_height", &codeboxHeight);
    _populate_bool_from_keyfile("codebox_width_pixels", &codeboxWidthPixels);
    _populate_bool_from_keyfile("codebox_line_num", &codeboxLineNum);
    _populate_bool_from_keyfile("codebox_match_bra", &codeboxMatchBra);
    _populate_string_from_keyfile("codebox_syn_highl", &codeboxSynHighl);
    _populate_bool_from_keyfile("codebox_auto_resize", &codeboxAutoResize);

    // [table]
    _currentGroup = "table";
    _populate_int_from_keyfile("table_rows", &tableRows);
    _populate_int_from_keyfile("table_columns", &tableColumns);
    int table_col_mode;
    if (_populate_int_from_keyfile("table_col_mode", &table_col_mode))
    {
        tableColMode = static_cast<CtTableColMode>(table_col_mode);
    }
    _populate_int_from_keyfile("table_col_min", &tableColMin);
    _populate_int_from_keyfile("table_col_max", &tableColMax);

    // [fonts]
    _currentGroup = "fonts";
    _populate_string_from_keyfile("rt_font", &rtFont);
    _populate_string_from_keyfile("pt_font", &ptFont);
    _populate_string_from_keyfile("tree_font", &treeFont);
    _populate_string_from_keyfile("code_font", &codeFont);

    // [colors]
    _currentGroup = "colors";
    _populate_string_from_keyfile("rt_def_fg", &rtDefFg);
    _populate_string_from_keyfile("rt_def_bg", &rtDefBg);
    _populate_string_from_keyfile("tt_def_fg", &ttDefFg);
    _populate_string_from_keyfile("tt_def_bg", &ttDefBg);
    _populate_string_from_keyfile("monospace_bg", &monospaceBg);
    _populate_string_from_keyfile("color_palette", &colorPalette);
    _populate_string_from_keyfile("col_link_webs", &colLinkWebs);
    _populate_string_from_keyfile("col_link_node", &colLinkNode);
    _populate_string_from_keyfile("col_link_file", &colLinkFile);
    _populate_string_from_keyfile("col_link_fold", &colLinkFold);

    // [misc]
    _currentGroup = "misc";
    _populate_string_from_keyfile("toolbar_ui_list", &toolbarUiList);
    _populate_bool_from_keyfile("systray", &systrayOn);
    _populate_bool_from_keyfile("start_on_systray", &startOnSystray);
    _populate_bool_from_keyfile("use_appind", &useAppInd);
    _populate_bool_from_keyfile("autosave_on", &autosaveOn);
    _populate_int_from_keyfile("autosave_val", &autosaveVal);
    _populate_bool_from_keyfile("check_version", &checkVersion);
    _populate_bool_from_keyfile("word_count", &wordCountOn);
    _populate_bool_from_keyfile("reload_doc_last", &reloadDocLast);
    _populate_bool_from_keyfile("mod_time_sentinel", &modTimeSentinel);
    _populate_bool_from_keyfile("backup_copy", &backupCopy);
    _populate_int_from_keyfile("backup_num", &backupNum);
    _populate_bool_from_keyfile("autosave_on_quit", &autosaveOnQuit);
    _populate_int_from_keyfile("limit_undoable_steps", &limitUndoableSteps);

    // [keyboard]
    _currentGroup = "keyboard";
    _populate_map_from_current_group(&customKbShortcuts);

    // [codexec_term]
    _currentGroup = "codexec_term";
    _populate_string_from_keyfile("custom_codexec_term", &customCodexecTerm);

    // [codexec_type]
    _currentGroup = "codexec_type";
    _populate_map_from_current_group(&customCodexecType);

    // [codexec_ext]
    _currentGroup = "codexec_ext";
    _populate_map_from_current_group(&customCodexecExt);
}
