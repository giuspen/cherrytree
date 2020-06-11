/*
 * ct_pandoc.cc
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

#include <sstream>
#include "ct_pandoc.h"
#include "ct_process.h"
#include "ct_logging.h"

#include <filesystem>

// Decide on path seperate ; on windows, : otherwise
#if defined(__WIN32)
constexpr auto path_delim = ';';
#else
constexpr auto path_delim = ':';
#endif
namespace fs = std::filesystem;

std::unique_ptr<CtProcess> pandoc_process() {
    auto p = std::make_unique<CtProcess>("pandoc");
    p->append_arg("-t");
    p->append_arg("html");
    return p;
}

namespace CtPandoc {

bool dir_contains_file(const std::filesystem::path& path, std::string_view file) 
{
    for (auto& dir_entry : fs::directory_iterator(path)) {
        auto p = dir_entry.path().stem();
        if (p == file) {
            return true;
        }
    }
    return false;
}

// Checks if the specified file is in the PATH environment variable
bool in_path(std::string_view file) 
{
    std::stringstream env_path(getenv("PATH"));
    std::vector<fs::path> path_dirs;

    // Split into search paths
    std::string str;
    while(std::getline(env_path, str, path_delim)) {
        path_dirs.emplace_back(str);
    }
    
    // Search for the file
    for (const auto& path : path_dirs) {
        if (fs::exists(path)) { // Sanity check
            if (dir_contains_file(path, file)) {
                return true;
            }
        }
    }
    
    return false;
}


bool has_pandoc() 
{
    return in_path("pandoc");
}


void to_html(std::istream& input, std::ostream& output) 
{
    auto process = pandoc_process();
    try {
        process->input(&input);
        process->run(output);
    } catch(std::exception& e) {
        spdlog::error("Exception in to_html: {}", e.what());
        throw;
    }
}


void to_html(const std::filesystem::path& file, std::ostream& output) {
    auto process = pandoc_process();
    process->append_arg(file.string());

    try {
        process->run(output);
    } catch(std::exception& e) {
        spdlog::error("Exception in to_html with filepath: {}; message: {}", file.string(), e.what());
        throw;
    }
}

}
