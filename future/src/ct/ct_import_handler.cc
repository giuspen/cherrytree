/*
 * ct_import_handler.cc
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
#include "ct_imports.h"

namespace fs = std::filesystem;


void CtImportHandler::_process_files(const std::filesystem::path &path) {
    if (!_processed_files) {
        auto &accepted_extensions = _get_accepted_file_extensions();
        if (!fs::is_directory(path)) {
            if (accepted_extensions.find(path.extension().string()) != accepted_extensions.end()) _import_files.emplace_back(path);
        } else {
            fs::recursive_directory_iterator rec_dir_iter(path);
            for (const auto &dir_entry : rec_dir_iter) {
                auto &fpath = dir_entry.path();
            
                if (accepted_extensions.find(fpath.extension().string()) != accepted_extensions.end()) {
                    ImportFile file(fpath, rec_dir_iter.depth());
                    _import_files.emplace_back(file);
                }
            
            }
        
        
        }
        _processed_files = true;
    }
    
    
    
}
