/*
 * ct_config.cc
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

#include <memory>
#include <gtkmm.h>
#include "ct_config.h"
#include "ct_misc_utils.h"
#include "ct_logging.h"
#include "ct_filesystem.h"

const fs::path CtConfig::ConfigFilename{"config.cfg"};
const fs::path CtConfig::PrintPageSetupFilename{"print.cfg"};
const fs::path CtConfig::LangFilename{"lang"};
const fs::path CtConfig::LogFilename{"log"};
const fs::path CtConfig::ConfigLanguageSpecsDirname{"language-specs"};
const fs::path CtConfig::ConfigStylesDirname{"styles"};
const fs::path CtConfig::ConfigIconsDirname{"icons"};
const fs::path CtConfig::UserStyleTemplate{"user-style.xml"};

CtConfig::CtConfig()
 : _configFilepath{fs::get_cherrytree_config_filepath()}
 , _configFilepathTmp{_configFilepath.string() + ".tmp"}
{
    _initLoadFromFileOk = _load_from_file();
    _ensure_user_styles_exist();
}

CtConfig::CtConfig(const std::string filepath)
 : _configFilepath{filepath}
{
    _initLoadFromFileOk = _load_from_file();
}

void CtConfig::move_from_tmp()
{
    if (not _configFilepathTmp.string().empty() and fs::exists(_configFilepathTmp)) {
        if (not fs::move_file(_configFilepathTmp, _configFilepath)) {
            spdlog::error("{} -> {}", _configFilepathTmp, _configFilepath);
        }
    }
}

bool CtConfig::_load_from_file()
{
    move_from_tmp();
#ifdef _WIN32
    // compatibility with old pygtk2 version
    if (not fs::exists(_configFilepath)) {
        const std::string old_pygtk2_filepath = str::replace(_configFilepath.string(), "\\Local\\", "\\Roaming\\");
        if (fs::exists(old_pygtk2_filepath)) {
            fs::copy_file(old_pygtk2_filepath, _configFilepath);
        }
    }
#endif // _WIN32
    if (fs::exists(_configFilepath)) {
        _uKeyFile = std::make_unique<Glib::KeyFile>();
        try {
            _uKeyFile->load_from_file(_configFilepath.string());
        }
        catch (Glib::Error& error) {
            spdlog::error("CtConfig {}: {}", _configFilepath, error.what());
            return false;
        }
        _populate_data_from_keyfile();
        _uKeyFile.reset(nullptr);
        spdlog::debug("{} parsed", _configFilepath);
        return true;
    }
    spdlog::warn("{} missing", _configFilepath);
    return false;
}

bool CtConfig::write_to_file(const std::string filepath/*= ""*/)
{
    _uKeyFile = std::make_unique<Glib::KeyFile>();
    _populate_keyfile_from_data();
    const bool writeSucceeded = _uKeyFile->save_to_file(filepath.empty() ? _configFilepathTmp.string() : filepath);
    _uKeyFile.reset(nullptr);
    return writeSucceeded;
}

bool CtConfig::_populate_bool_from_keyfile(const gchar* key, bool* pTarget)
{
    bool gotIt{false};
    if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key)) {
        try {
            *pTarget = _uKeyFile->get_boolean(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror) {
            if (kferror.code() == Glib::KeyFileError::Code::INVALID_VALUE) {
                // booleans from python ConfigParser
                Glib::ustring bool_str = _uKeyFile->get_value(_currentGroup, key);
                *pTarget = (bool_str == "True");
                gotIt = true;
            }
            else {
                _unexpected_keyfile_error(key, kferror);
            }
        }
    }
    return gotIt;
}

bool CtConfig::_populate_int_from_keyfile(const gchar* key, int* pTarget)
{
    bool gotIt{false};
    if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key)) {
        try {
            *pTarget = _uKeyFile->get_integer(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror) {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return gotIt;
}

bool CtConfig::_populate_double_from_keyfile(const gchar* key, double* pTarget)
{
    bool gotIt{false};
    if (_uKeyFile->has_group(_currentGroup) && _uKeyFile->has_key(_currentGroup, key)) {
        try {
            *pTarget = _uKeyFile->get_double(_currentGroup, key);
            gotIt = true;
        }
        catch (Glib::KeyFileError& kferror) {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return gotIt;
}

void CtConfig::_populate_map_from_current_group(std::map<std::string, std::string> *p_map)
{
    if (_uKeyFile->has_group(_currentGroup)) {
        for (std::string key : _uKeyFile->get_keys(_currentGroup)) {
            (*p_map)[key] = _uKeyFile->get_value(_currentGroup, key);
        }
    }
}

void CtConfig::_populate_current_group_from_map(const std::map<std::string, std::string>& map)
{
    for (const auto& pair : map) {
        _uKeyFile->set_value(_currentGroup, pair.first, pair.second);
    }
}

void CtConfig::_unexpected_keyfile_error(const gchar* key, const Glib::KeyFileError& kferror)
{
    spdlog::error("!! {} error code {} ", key, kferror.code());
}

void CtConfig::_populate_keyfile_from_data()
{
    // [state]
    _currentGroup = "state";
    guint i{0};
    for (const fs::path& filepath : recentDocsFilepaths) {
        snprintf(_tempKey, _maxTempKeySize, "doc_%d", i);
        _uKeyFile->set_string(_currentGroup, _tempKey, filepath.string());
        const CtRecentDocsRestore::iterator mapIt = recentDocsRestore.find(filepath.string());
        if (mapIt != recentDocsRestore.end()) {
            snprintf(_tempKey, _maxTempKeySize, "visit_%d", i);
            _uKeyFile->set_string(_currentGroup, _tempKey, mapIt->second.visited_nodes);
            snprintf(_tempKey, _maxTempKeySize, "expcol_%d", i);
            _uKeyFile->set_string(_currentGroup, _tempKey, mapIt->second.exp_coll_str);
            snprintf(_tempKey, _maxTempKeySize, "nodep_%d", i);
            _uKeyFile->set_string(_currentGroup, _tempKey, mapIt->second.node_path);
            snprintf(_tempKey, _maxTempKeySize, "curs_%d", i);
            _uKeyFile->set_integer(_currentGroup, _tempKey, mapIt->second.cursor_pos);
            snprintf(_tempKey, _maxTempKeySize, "vscr_%d", i);
            _uKeyFile->set_integer(_currentGroup, _tempKey, mapIt->second.v_adj_val);
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
    if (not currColors['f'].empty()) _uKeyFile->set_string(_currentGroup, "fg", currColors['f']);
    if (not currColors['b'].empty()) _uKeyFile->set_string(_currentGroup, "bg", currColors['b']);
    if (not currColors['n'].empty()) _uKeyFile->set_string(_currentGroup, "nn", currColors['n']);

    // [tree]
    _currentGroup = "tree";
    _uKeyFile->set_integer(_currentGroup, "rest_exp_coll", static_cast<int>(restoreExpColl));
    _uKeyFile->set_boolean(_currentGroup, "nodes_bookm_exp", nodesBookmExp);
    _uKeyFile->set_string(_currentGroup, "nodes_icons", nodesIcons);
    _uKeyFile->set_boolean(_currentGroup, "aux_icon_hide", auxIconHide);
    _uKeyFile->set_integer(_currentGroup, "default_icon_text", defaultIconText);
    _uKeyFile->set_boolean(_currentGroup, "tree_right_side", treeRightSide);
    _uKeyFile->set_boolean(_currentGroup, "cherry_wrap_ena", cherryWrapEnabled);
    _uKeyFile->set_integer(_currentGroup, "cherry_wrap_width", cherryWrapWidth);
    _uKeyFile->set_boolean(_currentGroup, "tree_click_focus_text", treeClickFocusText);
    _uKeyFile->set_boolean(_currentGroup, "tree_click_expand", treeClickExpand);

    // [editor]
    _currentGroup = "editor";
    _uKeyFile->set_string(_currentGroup, "syntax_highlighting", syntaxHighlighting);
    _uKeyFile->set_string(_currentGroup, "auto_syn_highl", autoSynHighl);
    _uKeyFile->set_string(_currentGroup, "rt_style_scheme", rtStyleScheme);
    _uKeyFile->set_string(_currentGroup, "pt_style_scheme", ptStyleScheme);
    _uKeyFile->set_string(_currentGroup, "ta_style_scheme", taStyleScheme);
    _uKeyFile->set_string(_currentGroup, "co_style_scheme", coStyleScheme);
    _uKeyFile->set_boolean(_currentGroup, "enable_spell_check", enableSpellCheck);
    _uKeyFile->set_string(_currentGroup, "spell_check_lang", spellCheckLang);
    _uKeyFile->set_boolean(_currentGroup, "show_line_numbers", showLineNumbers);
    _uKeyFile->set_boolean(_currentGroup, "scroll_beyond_last_line", scrollBeyondLastLine);
    _uKeyFile->set_boolean(_currentGroup, "spaces_instead_tabs", spacesInsteadTabs);
    _uKeyFile->set_integer(_currentGroup, "tabs_width", tabsWidth);
    _uKeyFile->set_integer(_currentGroup, "anchor_size", anchorSize);
    _uKeyFile->set_integer(_currentGroup, "embfile_icon_size", embfileIconSize);
    _uKeyFile->set_boolean(_currentGroup, "embfile_show_filename", embfileShowFileName);
    _uKeyFile->set_integer(_currentGroup, "embfile_max_size", embfileMaxSize);
    _uKeyFile->set_boolean(_currentGroup, "line_wrapping", lineWrapping);
    _uKeyFile->set_boolean(_currentGroup, "auto_smart_quotes", autoSmartQuotes);
    _uKeyFile->set_boolean(_currentGroup, "triple_click_paragraph", tripleClickParagraph);
    _uKeyFile->set_boolean(_currentGroup, "enable_symbol_autoreplace", enableSymbolAutoreplace);
#ifdef MD_AUTO_REPLACEMENT
    _uKeyFile->set_boolean(_currentGroup, "enable_md_formatting", enableMdFormatting);
#endif // MD_AUTO_REPLACEMENT
    _uKeyFile->set_integer(_currentGroup, "wrapping_indent", wrappingIndent);
    _uKeyFile->set_boolean(_currentGroup, "auto_indent", autoIndent);
    _uKeyFile->set_boolean(_currentGroup, "rt_show_white_spaces", rtShowWhiteSpaces);
    _uKeyFile->set_boolean(_currentGroup, "pt_show_white_spaces", ptShowWhiteSpaces);
    _uKeyFile->set_boolean(_currentGroup, "rt_highl_curr_line", rtHighlCurrLine);
    _uKeyFile->set_boolean(_currentGroup, "pt_highl_curr_line", ptHighlCurrLine);
    _uKeyFile->set_boolean(_currentGroup, "rt_highl_match_bra", rtHighlMatchBra);
    _uKeyFile->set_boolean(_currentGroup, "pt_highl_match_bra", ptHighlMatchBra);
    _uKeyFile->set_integer(_currentGroup, "space_around_lines", spaceAroundLines);
    _uKeyFile->set_integer(_currentGroup, "relative_wrapped_space", relativeWrappedSpace);
    _uKeyFile->set_string(_currentGroup, "h_rule", hRule);
    _uKeyFile->set_string(_currentGroup, "special_chars", specialChars.item());
    _uKeyFile->set_string(_currentGroup, "last_special_char", lastSpecialChar);
    _uKeyFile->set_string(_currentGroup, "selword_chars", selwordChars.item());
    _uKeyFile->set_string(_currentGroup, "chars_listbul", charsListbul.item());
    _uKeyFile->set_string(_currentGroup, "chars_toc", charsToc.item());
    _uKeyFile->set_string(_currentGroup, "chars_todo", charsTodo.item());
    _uKeyFile->set_string(_currentGroup, "chars_smart_dquote", chars_smart_dquote.item());
    _uKeyFile->set_string(_currentGroup, "chars_smart_squote", chars_smart_squote.item());
    _uKeyFile->set_string(_currentGroup, "latest_tag_prop", latestTagProp);
    _uKeyFile->set_string(_currentGroup, "latest_tag_val", latestTagVal);
    _uKeyFile->set_string(_currentGroup, "timestamp_format", timestampFormat);
    _uKeyFile->set_boolean(_currentGroup, "links_underline", linksUnderline);
    _uKeyFile->set_boolean(_currentGroup, "links_relative", linksRelative);
    _uKeyFile->set_boolean(_currentGroup, "weblink_custom_on", weblinkCustomOn);
    _uKeyFile->set_boolean(_currentGroup, "filelink_custom_on", filelinkCustomOn);
    _uKeyFile->set_boolean(_currentGroup, "folderlink_custom_on", folderlinkCustomOn);
    _uKeyFile->set_string(_currentGroup, "weblink_custom_act", weblinkCustomAct);
    _uKeyFile->set_string(_currentGroup, "filelink_custom_act", filelinkCustomAct);
    _uKeyFile->set_string(_currentGroup, "folderlink_custom_act", folderlinkCustomAct);

    // [codebox]
    _currentGroup = "codebox";
    _uKeyFile->set_double(_currentGroup, "codebox_width", codeboxWidth);
    _uKeyFile->set_double(_currentGroup, "codebox_height", codeboxHeight);
    _uKeyFile->set_boolean(_currentGroup, "codebox_width_pixels", codeboxWidthPixels);
    _uKeyFile->set_boolean(_currentGroup, "codebox_line_num", codeboxLineNum);
    _uKeyFile->set_boolean(_currentGroup, "codebox_match_bra", codeboxMatchBra);
    _uKeyFile->set_string(_currentGroup, "codebox_syn_highl", codeboxSynHighl);
    _uKeyFile->set_boolean(_currentGroup, "codebox_auto_resize", codeboxAutoResize);

    // [table]
    _currentGroup = "table";
    _uKeyFile->set_integer(_currentGroup, "table_rows", tableRows);
    _uKeyFile->set_integer(_currentGroup, "table_columns", tableColumns);
    _uKeyFile->set_integer(_currentGroup, "table_col_width", tableColWidthDefault);

    // [fonts]
    _currentGroup = "fonts";
    _uKeyFile->set_string(_currentGroup, "rt_font", rtFont);
    _uKeyFile->set_string(_currentGroup, "pt_font", ptFont);
    _uKeyFile->set_string(_currentGroup, "tree_font", treeFont);
    _uKeyFile->set_string(_currentGroup, "code_font", codeFont);

    // [colors]
    _currentGroup = "colors";
    _uKeyFile->set_string(_currentGroup, "tt_def_fg", ttDefFg);
    _uKeyFile->set_string(_currentGroup, "tt_def_bg", ttDefBg);
    _uKeyFile->set_string(_currentGroup, "scalable_h1", scalableH1.serialise());
    _uKeyFile->set_string(_currentGroup, "scalable_h2", scalableH2.serialise());
    _uKeyFile->set_string(_currentGroup, "scalable_h3", scalableH3.serialise());
    _uKeyFile->set_string(_currentGroup, "scalable_h4", scalableH4.serialise());
    _uKeyFile->set_string(_currentGroup, "scalable_h5", scalableH5.serialise());
    _uKeyFile->set_string(_currentGroup, "scalable_h6", scalableH6.serialise());
    _uKeyFile->set_string(_currentGroup, "scalable_small", scalableSmall.serialise());
    _uKeyFile->set_string(_currentGroup, "monospace_fg", monospaceFg);
    _uKeyFile->set_string(_currentGroup, "monospace_bg", monospaceBg);
    _uKeyFile->set_boolean(_currentGroup, "ms_dedic_font", msDedicatedFont);
    _uKeyFile->set_string(_currentGroup, "monospace_font", monospaceFont);
    _uKeyFile->set_string(_currentGroup, "color_palette", colorPalette);
    _uKeyFile->set_string(_currentGroup, "col_link_webs", colLinkWebs);
    _uKeyFile->set_string(_currentGroup, "col_link_node", colLinkNode);
    _uKeyFile->set_string(_currentGroup, "col_link_file", colLinkFile);
    _uKeyFile->set_string(_currentGroup, "col_link_fold", colLinkFold);
    for (unsigned n = 1; n <= CtConst::NUM_USER_STYLES; ++n) {
        const unsigned i = n-1;
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_text_fg", userStyleTextFg[i]);
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_text_bg", userStyleTextBg[i]);
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_sel_fg", userStyleSelectionFg[i]);
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_sel_bg", userStyleSelectionBg[i]);
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_cusor", userStyleCursor[i]);
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_curr_line_bg", userStyleCurrentLineBg[i]);
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_linenum_fg", userStyleLineNumbersFg[i]);
        _uKeyFile->set_string(_currentGroup, "style_"+std::to_string(n)+"_linenum_bg", userStyleLineNumbersBg[i]);
    }

    // [misc]
    _currentGroup = "misc";
    _uKeyFile->set_string(_currentGroup, "toolbar_ui_list", toolbarUiList);
    _uKeyFile->set_boolean(_currentGroup, "systray", systrayOn);
    _uKeyFile->set_boolean(_currentGroup, "start_on_systray", startOnSystray);
    //_uKeyFile->set_boolean(_currentGroup, "use_appind", useAppInd);
    _uKeyFile->set_boolean(_currentGroup, "autosave_on", autosaveOn);
    _uKeyFile->set_integer(_currentGroup, "autosave_val", autosaveVal);
    _uKeyFile->set_boolean(_currentGroup, "bookm_top_menu", bookmarksInTopMenu);
    _uKeyFile->set_boolean(_currentGroup, "check_version", checkVersion);
    _uKeyFile->set_boolean(_currentGroup, "word_count", wordCountOn);
    _uKeyFile->set_boolean(_currentGroup, "reload_doc_last", reloadDocLast);
    _uKeyFile->set_boolean(_currentGroup, "win_title_doc_dir", winTitleShowDocDir);
    _uKeyFile->set_boolean(_currentGroup, "nn_header_full_path", nodeNameHeaderShowFullPath);
    _uKeyFile->set_boolean(_currentGroup, "mod_time_sentinel", modTimeSentinel);
    _uKeyFile->set_boolean(_currentGroup, "backup_copy", backupCopy);
    _uKeyFile->set_integer(_currentGroup, "backup_num", backupNum);
    _uKeyFile->set_boolean(_currentGroup, "autosave_on_quit", autosaveOnQuit);
    _uKeyFile->set_boolean(_currentGroup, "enable_custom_backup_dir", customBackupDirOn);
    _uKeyFile->set_string(_currentGroup, "custom_backup_dir", customBackupDir);
    _uKeyFile->set_integer(_currentGroup, "limit_undoable_steps", limitUndoableSteps);

    // [keyboard]
    _currentGroup = "keyboard";
    _populate_current_group_from_map(customKbShortcuts);

    // [codexec_term]
    _currentGroup = "codexec_term";
    _uKeyFile->set_string(_currentGroup, "custom_codexec_term", customCodexecTerm);

    // [codexec_type]
    _currentGroup = "codexec_type";
    _populate_current_group_from_map(customCodexecType);

    // [codexec_ext]
    _currentGroup = "codexec_ext";
    _populate_current_group_from_map(customCodexecExt);
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
            if (not fileName.empty()) {
                const std::string filePath = Glib::build_filename(fileDir, fileName);
                recentDocsFilepaths.move_or_push_front(fs_canonicalize_filename(filePath));
                savedFromPyGtk = true;
            }
        }
    }
    for (int i = 0; i < recentDocsFilepaths.maxSize; ++i) {
        snprintf(_tempKey, _maxTempKeySize, "doc_%d", i);
        std::string filepath;
        if (not _populate_string_from_keyfile(_tempKey, &filepath)) {
            break;
        }
        filepath = fs_canonicalize_filename(filepath);
        recentDocsFilepaths.push_back(filepath);
        if (not savedFromPyGtk) {
            CtRecentDocRestore recentDocRestore;
            snprintf(_tempKey, _maxTempKeySize, "nodep_%d", i);
            if (_populate_string_from_keyfile(_tempKey, &recentDocRestore.node_path)) {
                snprintf(_tempKey, _maxTempKeySize, "visit_%d", i);
                _populate_string_from_keyfile(_tempKey, &recentDocRestore.visited_nodes);
                snprintf(_tempKey, _maxTempKeySize, "expcol_%d", i);
                _populate_string_from_keyfile(_tempKey, &recentDocRestore.exp_coll_str);
                snprintf(_tempKey, _maxTempKeySize, "curs_%d", i);
                _populate_int_from_keyfile(_tempKey, &recentDocRestore.cursor_pos);
                snprintf(_tempKey, _maxTempKeySize, "vscr_%d", i);
                _populate_int_from_keyfile(_tempKey, &recentDocRestore.v_adj_val);
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
    if (savedFromPyGtk) {
        CtRecentDocRestore recentDocRestore;
        if (_populate_string_from_keyfile("node_path", &recentDocRestore.node_path)) {
            recentDocRestore.node_path = str::replace(recentDocRestore.node_path, " ", ":");
            _populate_int_from_keyfile("cursor_position", &recentDocRestore.cursor_pos);
            recentDocsRestore[recentDocsFilepaths.front().string()] = recentDocRestore;
        }
    }
    _populate_string_from_keyfile("pick_dir_import", &pickDirImport);
    pickDirImport = fs_canonicalize_filename(pickDirImport);
    _populate_string_from_keyfile("pick_dir_export", &pickDirExport);
    pickDirExport = fs_canonicalize_filename(pickDirExport);
    _populate_string_from_keyfile("pick_dir_file", &pickDirFile);
    pickDirFile = fs_canonicalize_filename(pickDirFile);
    _populate_string_from_keyfile("pick_dir_img", &pickDirImg);
    pickDirImg = fs_canonicalize_filename(pickDirImg);
    _populate_string_from_keyfile("pick_dir_csv", &pickDirCsv);
    pickDirCsv = fs_canonicalize_filename(pickDirCsv);
    _populate_string_from_keyfile("pick_dir_cbox", &pickDirCbox);
    pickDirCbox = fs_canonicalize_filename(pickDirCbox);
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
    if (_populate_int_from_keyfile("rest_exp_coll", &rest_exp_coll)) {
        restoreExpColl = static_cast<CtRestoreExpColl>(rest_exp_coll);
    }
    if (savedFromPyGtk) {
        std::string exp_coll_str;
        if (_populate_string_from_keyfile("expanded_collapsed_string", &exp_coll_str)) {
            recentDocsRestore[recentDocsFilepaths.front().string()].exp_coll_str = exp_coll_str;
        }
        for (guint i = 1; i <= 3; ++i) {
            std::string docName;
            snprintf(_tempKey, _maxTempKeySize, "expcollnam%d", i);
            if (_populate_string_from_keyfile(_tempKey, &docName)) {
                for (const fs::path& filepath : recentDocsFilepaths) {
                    if (filepath.filename() == docName) {
                        CtRecentDocRestore recentDocRestore;
                        snprintf(_tempKey, _maxTempKeySize, "expcollstr%d", i);
                        _populate_string_from_keyfile(_tempKey, &recentDocRestore.exp_coll_str);
                        snprintf(_tempKey, _maxTempKeySize, "expcollsel%d", i);
                        if (_populate_string_from_keyfile(_tempKey, &recentDocRestore.node_path)) {
                            recentDocRestore.node_path = str::replace(recentDocRestore.node_path, " ", ":");
                            snprintf(_tempKey, _maxTempKeySize, "expcollcur%d", i);
                            _populate_int_from_keyfile(_tempKey, &recentDocRestore.cursor_pos);
                        }
                        recentDocsRestore[filepath.string()] = recentDocRestore;
                    }
                }
            }
            else {
                break;
            }
        }
    }
    _populate_bool_from_keyfile("nodes_bookm_exp", &nodesBookmExp);
    _populate_string_from_keyfile("nodes_icons", &nodesIcons);
    _populate_bool_from_keyfile("aux_icon_hide", &auxIconHide);
    _populate_int_from_keyfile("default_icon_text", &defaultIconText);
    _populate_bool_from_keyfile("tree_right_side", &treeRightSide);
    _populate_bool_from_keyfile("cherry_wrap_ena", &cherryWrapEnabled);
    _populate_int_from_keyfile("cherry_wrap_width", &cherryWrapWidth);
    _populate_bool_from_keyfile("tree_click_focus_text", &treeClickFocusText);
    _populate_bool_from_keyfile("tree_click_expand", &treeClickExpand);

    // [editor]
    _currentGroup = "editor";
    _populate_string_from_keyfile("syntax_highlighting", &syntaxHighlighting);
    _populate_string_from_keyfile("auto_syn_highl", &autoSynHighl);
    _populate_string_from_keyfile("rt_style_scheme", &rtStyleScheme);
    _populate_string_from_keyfile("pt_style_scheme", &ptStyleScheme);
    _populate_string_from_keyfile("ta_style_scheme", &taStyleScheme);
    _populate_string_from_keyfile("co_style_scheme", &coStyleScheme);
    if (_populate_bool_from_keyfile("enable_spell_check", &enableSpellCheck))
    {
        _populate_string_from_keyfile("spell_check_lang", &spellCheckLang);
    }
    _populate_bool_from_keyfile("show_line_numbers", &showLineNumbers);
    _populate_bool_from_keyfile("scroll_beyond_last_line", &scrollBeyondLastLine);
    _populate_bool_from_keyfile("spaces_instead_tabs", &spacesInsteadTabs);
    _populate_int_from_keyfile("tabs_width", &tabsWidth);
    _populate_int_from_keyfile("anchor_size", &anchorSize);
    _populate_int_from_keyfile("embfile_icon_size", &embfileIconSize);
    _populate_bool_from_keyfile("embfile_show_filename", &embfileShowFileName);
    _populate_int_from_keyfile("embfile_max_size", &embfileMaxSize);
    _populate_bool_from_keyfile("line_wrapping", &lineWrapping);
    _populate_bool_from_keyfile("auto_smart_quotes", &autoSmartQuotes);
    _populate_bool_from_keyfile("triple_click_paragraph", &tripleClickParagraph);
    _populate_bool_from_keyfile("enable_symbol_autoreplace", &enableSymbolAutoreplace);
#ifdef MD_AUTO_REPLACEMENT
    _populate_bool_from_keyfile("enable_md_formatting", &enableMdFormatting);
#endif // MD_AUTO_REPLACEMENT
    _populate_int_from_keyfile("wrapping_indent", &wrappingIndent);
    _populate_bool_from_keyfile("auto_indent", &autoIndent);
    _populate_bool_from_keyfile("rt_show_white_spaces", &rtShowWhiteSpaces);
    _populate_bool_from_keyfile("pt_show_white_spaces", &ptShowWhiteSpaces);
    _populate_bool_from_keyfile("rt_highl_curr_line", &rtHighlCurrLine);
    _populate_bool_from_keyfile("pt_highl_curr_line", &ptHighlCurrLine);
    _populate_bool_from_keyfile("rt_highl_match_bra", &rtHighlMatchBra);
    _populate_bool_from_keyfile("pt_highl_match_bra", &ptHighlMatchBra);
    _populate_int_from_keyfile("space_around_lines", &spaceAroundLines);
    _populate_int_from_keyfile("relative_wrapped_space", &relativeWrappedSpace);
    _populate_string_from_keyfile("h_rule", &hRule);
    _populate_string_from_keyfile("special_chars", &specialChars);
    _populate_string_from_keyfile("last_special_char", &lastSpecialChar);
    _populate_string_from_keyfile("selword_chars", &selwordChars);
    _populate_string_from_keyfile("chars_listbul", &charsListbul);
    if (charsListbul.size() < CtConst::CHARS_LISTBUL_DEFAULT.size()) {
        charsListbul = CtConst::CHARS_LISTBUL_DEFAULT;
    }
    _populate_string_from_keyfile("chars_toc", &charsToc);
    if (charsToc.size() < CtConst::CHARS_TOC_DEFAULT.size()) {
        charsToc = CtConst::CHARS_TOC_DEFAULT;
    }
    _populate_string_from_keyfile("chars_todo", &charsTodo);
    if (charsTodo.size() != CtConst::CHARS_TODO_DEFAULT.size()) {
        charsTodo = CtConst::CHARS_TODO_DEFAULT;
    }
    _populate_string_from_keyfile("chars_smart_dquote", &chars_smart_dquote);
    if (chars_smart_dquote.size() != CtConst::CHARS_SMART_DQUOTE_DEFAULT.size()) {
        chars_smart_dquote = CtConst::CHARS_SMART_DQUOTE_DEFAULT;
    }
    _populate_string_from_keyfile("chars_smart_squote", &chars_smart_squote);
    if (chars_smart_squote.size() != CtConst::CHARS_SMART_SQUOTE_DEFAULT.size()) {
        chars_smart_squote = CtConst::CHARS_SMART_SQUOTE_DEFAULT;
    }
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
    _populate_double_from_keyfile("codebox_width", &codeboxWidth);
    _populate_double_from_keyfile("codebox_height", &codeboxHeight);
    _populate_bool_from_keyfile("codebox_width_pixels", &codeboxWidthPixels);
    _populate_bool_from_keyfile("codebox_line_num", &codeboxLineNum);
    _populate_bool_from_keyfile("codebox_match_bra", &codeboxMatchBra);
    _populate_string_from_keyfile("codebox_syn_highl", &codeboxSynHighl);
    _populate_bool_from_keyfile("codebox_auto_resize", &codeboxAutoResize);

    // [table]
    _currentGroup = "table";
    _populate_int_from_keyfile("table_rows", &tableRows);
    _populate_int_from_keyfile("table_columns", &tableColumns);
    _populate_int_from_keyfile("table_col_width", &tableColWidthDefault);

    // [fonts]
    _currentGroup = "fonts";
    _populate_string_from_keyfile("rt_font", &rtFont);
    _populate_string_from_keyfile("pt_font", &ptFont);
    _populate_string_from_keyfile("tree_font", &treeFont);
    _populate_string_from_keyfile("code_font", &codeFont);

    // [colors]
    _currentGroup = "colors";
    _populate_string_from_keyfile("tt_def_fg", &ttDefFg);
    _populate_string_from_keyfile("tt_def_bg", &ttDefBg);
    std::string tmpScalable;
    _populate_string_from_keyfile("scalable_h1", &tmpScalable);
    scalableH1.deserialise(tmpScalable.c_str(), CtConst::SCALABLE_H1_DEFAULT);
    _populate_string_from_keyfile("scalable_h2", &tmpScalable);
    scalableH2.deserialise(tmpScalable.c_str(), CtConst::SCALABLE_H2_DEFAULT);
    _populate_string_from_keyfile("scalable_h3", &tmpScalable);
    scalableH3.deserialise(tmpScalable.c_str(), CtConst::SCALABLE_H3_DEFAULT);
    _populate_string_from_keyfile("scalable_h4", &tmpScalable);
    scalableH4.deserialise(tmpScalable.c_str(), CtConst::SCALABLE_H4_DEFAULT);
    _populate_string_from_keyfile("scalable_h5", &tmpScalable);
    scalableH5.deserialise(tmpScalable.c_str(), CtConst::SCALABLE_H5_DEFAULT);
    _populate_string_from_keyfile("scalable_h6", &tmpScalable);
    scalableH6.deserialise(tmpScalable.c_str(), CtConst::SCALABLE_H6_DEFAULT);
    _populate_string_from_keyfile("scalable_small", &tmpScalable);
    scalableSmall.deserialise(tmpScalable.c_str(), CtConst::SCALABLE_SMALL_DEFAULT);
    _populate_string_from_keyfile("monospace_fg", &monospaceFg);
    _populate_string_from_keyfile("monospace_bg", &monospaceBg);
    _populate_bool_from_keyfile("ms_dedic_font", &msDedicatedFont);
    _populate_string_from_keyfile("monospace_font", &monospaceFont);
    _populate_string_from_keyfile("color_palette", &colorPalette);
    _populate_string_from_keyfile("col_link_webs", &colLinkWebs);
    _populate_string_from_keyfile("col_link_node", &colLinkNode);
    _populate_string_from_keyfile("col_link_file", &colLinkFile);
    _populate_string_from_keyfile("col_link_fold", &colLinkFold);
    for (unsigned n = 1; n <= CtConst::NUM_USER_STYLES; ++n) {
        const unsigned i = n-1;
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_text_fg", &userStyleTextFg[i]);
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_text_bg", &userStyleTextBg[i]);
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_sel_fg", &userStyleSelectionFg[i]);
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_sel_bg", &userStyleSelectionBg[i]);
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_cusor", &userStyleCursor[i]);
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_curr_line_bg", &userStyleCurrentLineBg[i]);
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_linenum_fg", &userStyleLineNumbersFg[i]);
        _populate_string_from_keyfile("style_"+std::to_string(n)+"_linenum_bg", &userStyleLineNumbersBg[i]);
    }

    // [misc]
    _currentGroup = "misc";
    _populate_string_from_keyfile("toolbar_ui_list", &toolbarUiList);
    _populate_bool_from_keyfile("systray", &systrayOn);
    _populate_bool_from_keyfile("start_on_systray", &startOnSystray);
    _populate_bool_from_keyfile("use_appind", &useAppInd);
    if (useAppInd) {
        // if coming from pygtk2 version that supports appindicator which we currently do not
        systrayOn = false;
        startOnSystray = false;
    }
    _populate_bool_from_keyfile("autosave_on", &autosaveOn);
    _populate_int_from_keyfile("autosave_val", &autosaveVal);
    _populate_bool_from_keyfile("bookm_top_menu", &bookmarksInTopMenu);
    _populate_bool_from_keyfile("check_version", &checkVersion);
    _populate_bool_from_keyfile("word_count", &wordCountOn);
    _populate_bool_from_keyfile("reload_doc_last", &reloadDocLast);
    _populate_bool_from_keyfile("win_title_doc_dir", &winTitleShowDocDir);
    _populate_bool_from_keyfile("nn_header_full_path", &nodeNameHeaderShowFullPath);
    _populate_bool_from_keyfile("mod_time_sentinel", &modTimeSentinel);
    _populate_bool_from_keyfile("backup_copy", &backupCopy);
    _populate_int_from_keyfile("backup_num", &backupNum);
    _populate_bool_from_keyfile("autosave_on_quit", &autosaveOnQuit);
    _populate_bool_from_keyfile("enable_custom_backup_dir", &customBackupDirOn);
    _populate_string_from_keyfile("custom_backup_dir", &customBackupDir);
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

void CtConfig::_ensure_user_styles_exist()
{
    for (unsigned n = 1; n <= CtConst::NUM_USER_STYLES; ++n) {
        const fs::path userStyleFilepath = fs::get_cherrytree_config_user_style_filepath(n);
        if (not fs::is_regular_file(userStyleFilepath)) {
            update_user_style(n);
        }
    }
}

std::string CtConfig::get_user_style_id(const unsigned num)
{
    return std::string{"user-"} + std::to_string(num);
}

void CtConfig::update_user_style(const unsigned num)
{
    const fs::path userStyleTemplateFilepath = fs::get_cherrytree_datadir() / "data" / CtConfig::UserStyleTemplate;
    if (not fs::is_regular_file(userStyleTemplateFilepath)) {
        spdlog::warn("Unexp missing {}", userStyleTemplateFilepath.c_str());
        return;
    }
    const fs::path userStyleDirpath = fs::get_cherrytree_config_styles_dirpath();
    if (not fs::is_directory(userStyleDirpath)) {
        if (g_mkdir_with_parents(userStyleDirpath.c_str(), 0755) < 0) {
            spdlog::warn("Could not create config dir {}", userStyleDirpath.c_str());
            return;
        }
    }
    std::string userStyleText = Glib::file_get_contents(userStyleTemplateFilepath.string());
    const unsigned i = num-1;
    userStyleText = str::replace(userStyleText, "_userStyleId_", CtConfig::get_user_style_id(num));
    userStyleText = str::replace(userStyleText, "_userStyleTextFg_", userStyleTextFg[i]);
    userStyleText = str::replace(userStyleText, "_userStyleTextBg_", userStyleTextBg[i]);
    userStyleText = str::replace(userStyleText, "_userStyleSelectionFg_", userStyleSelectionFg[i]);
    userStyleText = str::replace(userStyleText, "_userStyleSelectionBg_", userStyleSelectionBg[i]);
    userStyleText = str::replace(userStyleText, "_userStyleCursor_", userStyleCursor[i]);
    userStyleText = str::replace(userStyleText, "_userStyleCurrentLineBg_", userStyleCurrentLineBg[i]);
    userStyleText = str::replace(userStyleText, "_userStyleLineNumbersFg_", userStyleLineNumbersFg[i]);
    userStyleText = str::replace(userStyleText, "_userStyleLineNumbersBg_", userStyleLineNumbersBg[i]);
    const fs::path userStyleFilepath = fs::get_cherrytree_config_user_style_filepath(num);
    Glib::file_set_contents(userStyleFilepath.string(), userStyleText);
}
