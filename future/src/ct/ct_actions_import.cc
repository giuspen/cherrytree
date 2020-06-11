/*
 * ct_actions_import.cc
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

#include "ct_actions.h"
#include "ct_clipboard.h"
#include "ct_imports.h"
#include "ct_storage_control.h"
#include "ct_export2html.h"

#include "ct_logging.h"
#include <fstream>

namespace fs = std::filesystem;


struct File {
    fs::path path;
    unsigned int depth = 0;
    std::vector<File> children;
};

using file_valid_req = bool(*)(const fs::path&);
template<typename FILE_TYPE_VALID_T = file_valid_req>
void find_valid_files_in_dir(const fs::path& dir, std::vector<File>& files, unsigned int depth, std::optional<FILE_TYPE_VALID_T> file_valid_func) {
    for (const auto& dir_ent : fs::directory_iterator(dir)) {
        auto& path = dir_ent.path();
        
        if (file_valid_func) {
            if (!((*file_valid_func)(path))) continue; // Skip if invalid
        } 

        File file { path, depth, {} };
        if (fs::is_directory(path)) {
            find_valid_files_in_dir(path, file.children, depth + 1, file_valid_func);
        } else {
            files.emplace_back(std::move(file));
        }
    }
}

/*
 * NODE_ADD_T requires a signiture of 
 * (fs::path, CtTreeIter) -> CtTreeIter
 */

template<typename NODE_ADD_T>
void add_node_from_file(const File& file, const Gtk::TreeIter& parent, NODE_ADD_T node_add_func) {
    Gtk::TreeIter iter = node_add_func(file.path, parent);

    for (const auto& child : file.children) {
        add_node_from_file(child, iter, node_add_func);
    }
}

template<typename NODE_ADD_T, typename FILE_TYPE_VALID_T = file_valid_req>
void add_nodes_with_directory_hierarchy(NODE_ADD_T add_node_func, CtMainWin* main_win, bool recursive = true, std::optional<FILE_TYPE_VALID_T> path_valid_func = std::nullopt) {
    auto file_dir = CtDialogs::folder_select_dialog(main_win->get_ct_config()->pickDirImport, main_win);
    if (file_dir.empty()) return;

    main_win->get_ct_config()->pickDirImport = file_dir;


    std::vector<File> files;
    auto file_valid_func = path_valid_func;
    if (!recursive) {
        file_valid_func = [](const auto& path) { return !fs::is_directory(path); };
    }
    find_valid_files_in_dir(file_dir, files, 0, file_valid_func);

    auto parent = main_win->get_tree_store().to_ct_tree_iter(Gtk::TreeIter());
    for (const auto& file : files) {
        add_node_from_file(file, parent, add_node_func);
    }
}



// Todo: Add option to select whether parent is current node or top of tree

CtNodeData setup_node(CtMainWin* pWin, const std::filesystem::path& path)
{
    CtNodeData nodeData;
    nodeData.isBold = false;
    nodeData.customIconId = 0;
    nodeData.isRO = false;
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    nodeData.rTextBuffer = pWin->get_new_text_buffer();
    nodeData.name = path.stem().string();
    return nodeData;
}

// Ask the user what file to import for the node
void CtActions::import_node_from_html_file() noexcept 
{
    try {
        if(!_is_there_selected_node_or_error()) return;

        CtDialogs::file_select_args args(_pCtMainWin);
        args.curr_folder = _pCtMainWin->get_ct_config()->pickDirImport;
        args.filter_mime = {"text/html"};
        auto filepath = CtDialogs::file_select_dialog(args);
        if (filepath.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirImport = Glib::path_get_dirname(filepath);

        _import_node_from_html(filepath);

    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing node from file: {}", e.what());
    }
}

// Import a node from a html file
void CtActions::_import_node_from_html(const std::filesystem::path& filepath) 
{
    if (!_is_there_selected_node_or_error()) return;
    auto nodeData = setup_node(_pCtMainWin, filepath);
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    
    CtHtml2Xml parser(_pCtMainWin);

    try {
        parser.add_file(filepath);
        CtClipboard(_pCtMainWin).from_xml_string_to_buffer(nodeData.rTextBuffer, parser.to_string());
        auto iter = _pCtMainWin->curr_tree_iter();
        std::shared_ptr<CtNodeState> node_state;
        _node_add_with_data(iter, nodeData, false, node_state);
    } catch(std::exception& e) {
        spdlog::error("Exception caught while parsing the document: {}", e.what());
    }
}
// Import a directory of html files - non recursive
void CtActions::import_node_from_html_directory() noexcept 
{
    namespace fs = std::filesystem;
    
    try {
        if(!_is_there_selected_node_or_error()) return;

        std::string dirpath = CtDialogs::folder_select_dialog(_pCtMainWin->get_ct_config()->pickDirImport, _pCtMainWin);
        if (dirpath.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirImport = dirpath;

        for (const auto& file : fs::directory_iterator(dirpath)) {
            
            const auto& f_path = file.path();
            if (f_path.extension() == ".html" || f_path.extension() == ".htm") {
                _import_node_from_html(f_path);
            }
        }
    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing nodes from directory: {}", e.what());
    }
}

void CtActions::_import_node_from_plaintext(const std::filesystem::path &filepath)
{
    if (!_is_there_selected_node_or_error()) return;
    auto nodeData = setup_node(_pCtMainWin, filepath);
    nodeData.syntax = CtConst::PLAIN_TEXT_ID;
    
    try {

        std::ifstream infile;
        infile.exceptions(std::ios_base::failbit);
        infile.open(filepath);
        
        std::ostringstream data;
        data << infile.rdbuf();
        nodeData.rTextBuffer->insert_at_cursor(data.str());
        
        auto                         iter = _pCtMainWin->curr_tree_iter();
        std::shared_ptr<CtNodeState> node_state;
        
        _node_add_with_data(iter, nodeData, false, node_state);
    }
    catch (std::exception &e) {
        spdlog::error("Exception caught while importing plaintext file ({}): {}", filepath.string(), e.what());
    }
}

void CtActions::import_nodes_from_ct_file() noexcept
{
    try {
        if(!_is_there_selected_node_or_error()) return;
        CtDialogs::file_select_args args(_pCtMainWin);
        args.curr_folder = _pCtMainWin->get_ct_config()->pickDirImport;
        args.filter_pattern = CtConst::CT_FILE_EXTENSIONS_FILTER;
        
        auto fpath = CtDialogs::file_select_dialog(args);
        if (fpath.empty()) return; // No file selected
        _pCtMainWin->get_ct_config()->pickDirImport = Glib::path_get_dirname(fpath);

        
        // Add the nodes through the storage type
        _pCtMainWin->get_ct_storage()->add_nodes_from_storage(fpath);
        
    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing node from CT file: {}", e.what());
    }
}

void CtActions::import_node_from_plaintext_file() noexcept
{
    try {
        if(!_is_there_selected_node_or_error()) return;

        CtDialogs::file_select_args args(_pCtMainWin);
        args.curr_folder = _pCtMainWin->get_ct_config()->pickDirImport;
        args.filter_mime = {"text/plain"};
        auto fpath = CtDialogs::file_select_dialog(args);
        if (fpath.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirImport = Glib::path_get_dirname(fpath);
        
        _import_node_from_plaintext(fpath);
        
    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing plaintext file: {}", e.what());
    }
}

void CtActions::import_nodes_from_plaintext_directory() noexcept
{
    try {
        if(!_is_there_selected_node_or_error()) return;
        
        std::string fdir = CtDialogs::folder_select_dialog(_pCtMainWin->get_ct_config()->pickDirImport, _pCtMainWin);
        if (fdir.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirImport = fdir;
        
        for (const auto& pair : std::filesystem::directory_iterator(fdir)) {
            auto& filepath = pair.path();
            
            if (CtMiscUtil::mime_type_contains(filepath.string(), "text/")) {
                _import_node_from_plaintext(filepath);
            }
        }
        
    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing directory of plaintext files: {}", e.what());
    }
}

void CtActions::_import_node_from_md_file(const std::filesystem::path& filepath)
{
    std::ifstream infile(filepath);
    if (!infile) throw std::runtime_error(fmt::format("Failure while opening input file: {}: {}", filepath.string(), strerror(errno)));
    
    
    CtMDParser handler(_pCtMainWin->get_ct_config());
    handler.feed(infile);
    
    auto node = setup_node(_pCtMainWin, filepath);
    
    CtClipboard(_pCtMainWin).from_xml_string_to_buffer(node.rTextBuffer, handler.to_string());
    
    Gtk::TreeIter curr_iter;
    auto node_iter = _add_node_quick(curr_iter, node, false);
    _pCtMainWin->get_tree_store().nodes_sequences_fix(node_iter->parent(), false);
    _pCtMainWin->update_window_save_needed();
}

void CtActions::import_node_from_md_file() noexcept
{
    try {
        CtDialogs::file_select_args args(_pCtMainWin);
        args.curr_folder = _pCtMainWin->get_ct_config()->pickDirImport;
        args.filter_mime = {"text/plain"};
        args.filter_pattern = {"*.md"};
        auto fpath = CtDialogs::file_select_dialog(args);
        if (fpath.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirImport = Glib::path_get_dirname(fpath);
        
        _import_node_from_md_file(fpath);
    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing node from md file: ", e.what());
    }
}

void CtActions::_import_nodes_from_zim_directory(const std::filesystem::path& filepath)
{
    CtZimImportHandler handler(_pCtMainWin->get_ct_config());
    handler.add_directory(filepath);
    
    auto& files = handler.imported_files();
    
    CtClipboard clipboard(_pCtMainWin);
    std::vector<std::pair<CtImportFile*, CtTreeIter>> node_cache;
    
    auto& tree_store = _pCtMainWin->get_tree_store();
    auto add_node = [this, &tree_store, &node_cache, &clipboard](CtImportFile& file, const Gtk::TreeIter& parent_iter) mutable -> Gtk::TreeIter {
        auto node_data = setup_node(_pCtMainWin, file.path);
        node_data.syntax = CtConst::RICH_TEXT_ID;
        
        auto iter = _add_node_quick(parent_iter, node_data, true);
        
        node_cache.emplace_back(&file, tree_store.to_ct_tree_iter(iter));
        
        return iter;
    };
    
    std::function<void(const std::vector<std::shared_ptr<CtImportFile>>&, const Gtk::TreeIter&)> file_iter_func;
    file_iter_func = [&file_iter_func, &add_node](const std::vector<std::shared_ptr<CtImportFile>>& children, const Gtk::TreeIter& parent) {
        for (auto& child : children) {
            if (child) {
                auto iter = add_node(*child, parent);
                file_iter_func(child->children, iter);
            }
        }
    };
    
    for (auto& file : files) {
        if (file->depth == 0) {
            auto new_iter = add_node(*file, Gtk::TreeIter());
            file_iter_func(file->children, new_iter);
        }
    }
    
    // Update links
    for (auto& file : files) {
        for (auto& iter : node_cache) {
            file->fix_internal_links(iter.second.get_node_name(), iter.second.get_node_id());
        }
       
    }
    /* Populate the text buffers after fixing the links
     * This is needed because the link ids are only set properly
     * once the nodes have been added to the tree
     */
    for (auto& file_pair : node_cache) {
        auto text_buff = file_pair.second.get_node_text_buffer();
        auto data = file_pair.first->to_string();
        clipboard.from_xml_string_to_buffer(text_buff, data);
    }
    
    tree_store.nodes_sequences_fix(Gtk::TreeIter(), true);

    _pCtMainWin->update_window_save_needed();
    
}

void CtActions::import_nodes_from_zim_directory() noexcept
{
    try {
        if (!_is_there_selected_node_or_error()) return;
        
        auto fdir = CtDialogs::folder_select_dialog(_pCtMainWin->get_ct_config()->pickDirImport, _pCtMainWin);
        if (fdir.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirImport = fdir;
    
        _import_nodes_from_zim_directory(fdir);
        
    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing node from ZIM file: {}", e.what());
    }
}

void CtActions::import_nodes_from_md_directory() noexcept
{
    try {
        auto fdir = CtDialogs::folder_select_dialog(_pCtMainWin->get_ct_config()->pickDirImport, _pCtMainWin);
        if (fdir.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirImport = fdir;

        for (const auto& file : std::filesystem::directory_iterator(fdir)) {
            auto& file_path = file.path();
            if (file_path.extension() == ".md") {
                _import_node_from_md_file(file_path);
            }
        }
        
    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing files from MD directory: {}", e.what());

    }
}

bool pandoc_in_path(CtMainWin& main_win) 
{
    if (!CtPandoc::has_pandoc()) {
        CtDialogs::warning_dialog(_("Pandoc executable could not be found, please ensure it is in your path"), main_win);
        return false;
    }
    return true;
}



CtNodeData setup_pandoc_node(CtMainWin* main_win, CtClipboard& clipboard, const fs::path& filepath) {
    std::stringstream html_buff;
    CtPandoc::to_html(filepath, html_buff);
    CtHtml2Xml parser(main_win);
    parser.feed(html_buff.str());
    
    auto node = setup_node(main_win, filepath);
    clipboard.from_xml_string_to_buffer(node.rTextBuffer, parser.to_string());

    return node;
}

void CtActions::_import_through_pandoc(const std::filesystem::path& filepath) 
{
    if (!std::filesystem::exists(filepath)) throw std::runtime_error(fmt::format("Path does not exist: {}", filepath.string()));
    
    try {
        CtClipboard clip(_pCtMainWin);
        auto node = setup_pandoc_node(_pCtMainWin, clip, filepath);
        
        Gtk::TreeIter curr_iter;
        auto node_iter = _add_node_quick(curr_iter, node, false);
        _pCtMainWin->get_tree_store().nodes_sequences_fix(node_iter->parent(), false);
        _pCtMainWin->update_window_save_needed();
    } catch(std::exception& e) {
        spdlog::error("Exception in CtActions::_import_through_pandoc for path: {}; {}", filepath.string(), e.what());
        throw;
    }
}

void CtActions::import_node_from_pandoc() noexcept 
{
    try {
        if (!pandoc_in_path(*_pCtMainWin)) return;

        CtDialogs::file_select_args args(_pCtMainWin);
        auto path = CtDialogs::file_select_dialog(args);
        if (path.empty()) return;
        
        _import_through_pandoc(path);
        
    } catch(std::exception& e) {
        auto err_msg = fmt::format("Exception caught in CtActions::import_node_from_pandoc: {}", e.what());
        spdlog::error(err_msg);
        CtDialogs::error_dialog(err_msg, *_pCtMainWin);
    }
    
    
}


void CtActions::import_directory_from_pandoc() noexcept {
    try {
        if (!pandoc_in_path(*_pCtMainWin)) return;

        CtClipboard clipboard(_pCtMainWin);
        auto add_node_func = [this, &clipboard](const auto& path, const auto& iter){ 
            auto node = setup_pandoc_node(_pCtMainWin, clipboard, path);
            
            auto node_iter = _add_node_quick(iter, node, true);
            _pCtMainWin->get_tree_store().nodes_sequences_fix(node_iter->parent(), false);
            return node_iter;
        };
        add_nodes_with_directory_hierarchy(add_node_func, _pCtMainWin);

        _pCtMainWin->update_window_save_needed();

    } catch(std::exception& e) {
        spdlog::error("Exception caught in import_directory_from_pandoc: {}", e.what());
    }


};

