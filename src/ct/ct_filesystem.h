/*
 * ct_filesystem.h
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

#pragma once

#include <string>
#include <list>
#include <vector>
#include "ct_types.h"
#include "ct_logging.h"
#include <glibmm/miscutils.h>

class CtConfig;

namespace fs {

class path;

void register_exe_path_detect_if_portable(const char* exe_path);
bool alter_locale_env_var(const std::string& key, const std::string& val);

bool copy_file(const path& from, const path& to);

bool move_file(const path& from, const path& to);

bool is_regular_file(const path& file);

bool is_directory(const path& p);

path absolute(const path& p);

path canonical(const path& p, const bool resolveSymlink = true);

path canonical(const path& p, const path& base);

path relative(const path& p, const path& base);

bool exists(const path& filepath);

time_t getmtime(const path& p);

std::uintmax_t file_size(const path& p);

std::list<path> get_dir_entries(const path& dir);

void open_weblink(const std::string& link);
void open_filepath(const path& filepath, bool open_folder_if_file_not_exists, CtConfig* config);
void open_folderpath(const path& folderpath, CtConfig* config);

std::string get_content(const path& filepath);

path prepare_export_folder(const path& dir_place, path new_folder, bool overwrite_existing);

CtDocType get_doc_type(const path& fileName);
CtDocEncrypt get_doc_encrypt(const path& fileName);

/**
* @brief Remove a directory and all its children
* @param dir
* @return
*/
std::uintmax_t remove_all(const path& dir);
/**
* @brief Remove a file or an empty directory
* @param path
* @return
*/
bool remove(const path& p);

path get_cherrytree_datadir();
path get_cherrytree_localedir();
path get_cherrytree_configdir();
path get_cherrytree_lang_filepath();
path get_cherrytree_config_filepath();
path get_cherrytree_config_language_specs_dirpath();
path get_cherrytree_config_styles_dirpath();
path get_cherrytree_config_user_style_filepath(const unsigned num);
// Filepath is a url so not an fs::path
std::string download_file(const std::string& filepath);

/**
* @class path
* @brief An object representing a filepath
*/
class path
{
public:
    friend void swap(path& lhs, path& rhs) noexcept {
        using std::swap;
        swap(lhs._path, rhs._path);
    }

    path() = default;
    path(std::string p) : _path(std::move(p)) {}
    path(const char* cpath) : _path(cpath) {}
    template<typename ITERATOR_T>
    path(ITERATOR_T begin, ITERATOR_T end) : _path(begin, end) {}
    ~path() = default;

    void swap(path& other) noexcept {
        using std::swap;
        swap(*this, other);
    }

    path& operator=(std::string other) { _path = other; return *this; }
    path& operator=(const char* other) { return operator=(std::string(other)); }

    friend path operator/(const path& lhs, const path& rhs) {
        return path(Glib::build_filename(lhs._path, rhs._path));
    }

    friend path operator/(const path& lhs, const char* rhs) {
        return path(Glib::build_filename(lhs._path, rhs));
    }

    friend path operator/(const path& lhs, const std::string& rhs) {
        return path(Glib::build_filename(lhs._path, rhs));
    }

    friend bool operator==(const path& lhs, const path& rhs) { return lhs._path == rhs._path; }
    friend bool operator!=(const path& lhs, const path& rhs) { return !(lhs == rhs); }
    friend bool operator<(const path& lhs, const path& rhs) { return lhs._path < rhs._path; }
    friend bool operator>(const path& lhs, const path& rhs) { return lhs._path > rhs._path; }
    friend bool operator>=(const path& lhs, const path& rhs) { return lhs._path >= rhs._path; }
    friend bool operator<=(const path& lhs, const path& rhs) { return lhs._path <= rhs._path; }

    friend void operator+=(path& lhs, const path& rhs) { lhs._path += rhs._path; }
    friend void operator+=(path& lhs, const std::string& rhs) { lhs._path += rhs; }
    friend void operator+=(path& lhs, const char* rhs) { lhs._path += rhs; }

    [[nodiscard]] const char* c_str() const { return _path.c_str(); }
    [[nodiscard]] std::string string() const { return _path; }
    [[nodiscard]] bool is_absolute() const { return Glib::path_is_absolute(_path); }
    [[nodiscard]] bool is_relative() const { return !is_absolute(); }
    [[nodiscard]] bool empty() const noexcept { return _path.empty(); }
    [[nodiscard]] path filename() const { return Glib::path_get_basename(_path); }
    [[nodiscard]] path parent_path() const { return Glib::path_get_dirname(_path); }
    [[nodiscard]] path extension() const;
    [[nodiscard]] path stem() const;
    [[nodiscard]] std::string string_native() const;
    [[nodiscard]] std::string string_unix() const;

private:
    std::string _path;
};

} // namespace fs

template<>
struct fmt::formatter<fs::path> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const fs::path& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", p.string());
    }
};
