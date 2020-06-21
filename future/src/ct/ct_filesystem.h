/*
 * ct_filesystem.h
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

#pragma once
#include <string>
#include <list>
#include <vector>
#include "ct_types.h"
#include "ct_splittable.h"
#include "ct_logging.h"
#include <glibmm/miscutils.h>

class CtConfig;

namespace fs {
class path;
#include "ct_splittable.h"

bool copy_file(const path& from, const path& to);

bool move_file(const path& from, const path& to);

bool is_regular_file(const path& file);

bool is_directory(const path& path);

path absolute(const path& path);

path canonical(const path& path);

bool exists(const path& filepath);

time_t getmtime(const path& path);

std::uintmax_t file_size(const path& path);

std::list<path> get_dir_entries(const path& dir);

void external_filepath_open(const path& filepath, bool open_folder_if_file_not_exists, CtConfig* config);
void external_folderpath_open(const path& folderpath, CtConfig* config);

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
bool remove(const path& path);

path get_cherrytree_datadir();
path get_cherrytree_localedir();
path get_cherrytree_configdir();
path get_cherrytree_lang_filepath();
// Filepath is a url so not an fs::path
std::string download_file(const std::string& filepath);

/**
* @class path
* @brief An object representing a filepath
*/
class path {
    using value_type = std::string::value_type;
    using string_type = std::basic_string<value_type>;

#ifdef _WIN32
    static const value_type path_sep = '\\';
#else
    static const value_type path_sep = '/';
#endif

public:
    friend void swap(path& lhs, path& rhs) noexcept {
        using std::swap;
        swap(lhs._path, rhs._path);
    }

    path() = default;
    path(string_type path) : _path(std::move(path)) {}
    path(const value_type* cpath) : _path(cpath) {}
    template<typename ITERATOR_T>
    path(ITERATOR_T begin, ITERATOR_T end) : _path(begin, end) {}
    ~path() = default;

    void swap(path& other) noexcept {
        using std::swap;
        swap(*this, other);
    }


    path& operator=(string_type other) { _path = other; return *this;};
    path& operator=(const value_type* other) { return operator=(string_type(other)); };


    friend path operator/(const path& lhs, const path& rhs)
    {
        return path(Glib::build_filename(lhs._path, rhs._path));
    }
    friend path operator/(const path& lhs, const value_type* rhs) {
        return path(Glib::build_filename(lhs._path, rhs));
    }

    friend path operator/(const path& lhs, const string_type& rhs)
    {
        return path(Glib::build_filename(lhs._path, rhs));
    }

    friend bool operator==(const path& lhs, const path& rhs) { return lhs._path == rhs._path; }
    friend bool operator!=(const path& lhs, const path& rhs) { return !(lhs == rhs); }
    friend bool operator<(const path& lhs, const path& rhs) { return lhs._path < rhs._path; }
    friend bool operator>(const path& lhs, const path& rhs) { return lhs._path > rhs._path; }
    friend bool operator>=(const path& lhs, const path& rhs) { return lhs._path >= rhs._path; }
    friend bool operator<=(const path& lhs, const path& rhs) { return lhs._path <= rhs._path; }

    friend void operator+=(path& lhs, const path& rhs) { lhs._path += rhs._path; }
    friend void operator+=(path& lhs, const string_type& rhs) { lhs._path += rhs; }
    friend void operator+=(path& lhs, const value_type* rhs) { lhs._path += rhs; }

    [[nodiscard]] const char* c_str() const { return _path.c_str(); };
    [[nodiscard]] std::string string() const { return _path; }
    [[nodiscard]] bool is_absolute() const { return Glib::path_is_absolute(_path); }
    [[nodiscard]] bool is_relative() const { return !is_absolute(); }
    [[nodiscard]] bool empty() const noexcept { return _path.empty(); }
    [[nodiscard]] path filename() const { return Glib::path_get_basename(_path); }
    [[nodiscard]] path parent_path() const { return Glib::path_get_dirname(_path); }
    [[nodiscard]] path extension() const;
    [[nodiscard]] path stem() const;
    [[nodiscard]] std::string native() const;
private:
    string_type _path;

    /// From Slash to Backslash when needed
    static std::string _get_platform_path(std::string filepath);

};

} // namespace CtFileSystem


template<>
struct fmt::formatter<fs::path> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const fs::path& path, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", path.string());
    }
};
