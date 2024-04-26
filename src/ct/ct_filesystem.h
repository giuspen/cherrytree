/*
 * ct_filesystem.h
 *
 * Copyright 2009-2024
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

#if GLIBMM_MAJOR_VERSION > 2 || (GLIBMM_MAJOR_VERSION == 2 && GLIBMM_MINOR_VERSION >= 64)
#define fs_canonicalize_filename Glib::canonicalize_filename
#else
#define fs_canonicalize_filename fs::legacy_canonicalize_filename
#endif

class CtConfig;

namespace fs {

class path;

std::string legacy_canonicalize_filename(const std::string& filename, const std::string& relative_to = "");

void register_exe_path_detect_if_portable(const char* exe_path);

bool alter_locale_env_var(const std::string& key, const std::string& val);

#if defined(_WIN32)
bool alter_TEXMFROOT_env_var();
bool alter_PATH_env_var();
#else // !_WIN32
const char* get_latex_dvipng_console_bin_prefix();
#endif // !_WIN32

bool copy_file(const path& from, const path& to);

bool move_file(const path& from, const path& to);

bool exists(const path& filepath);

bool is_regular_file(const path& file);

bool is_directory(const path& p);

path absolute(const path& p);

path canonical(const path& p, const bool resolveSymlink = true);

path canonical(const path& p, const path& base);

path relative(const path& p, const path& base);

time_t getmtime(const path& p);

std::uintmax_t file_size(const path& p);

std::list<path> get_dir_entries(const path& dir);

void open_weblink(const std::string& link);
void open_filepath(const path& filepath, bool open_folder_if_file_not_exists, CtConfig* config);
void open_folderpath(const path& folderpath, CtConfig* config);

path prepare_export_folder(const path& dir_place, path new_folder, bool overwrite_existing);

CtDocType get_doc_type_from_file_ext(const path& fileName);
CtDocEncrypt get_doc_encrypt_from_file_ext(const path& fileName);

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
path get_cherrytree_print_page_setup_cfg_filepath();
path get_cherrytree_langcfg_filepath();
path get_cherrytree_logcfg_filepath();
std::optional<path> get_cherrytree_logdir();
path get_cherrytree_config_filepath();
path get_cherrytree_config_language_specs_dirpath();
path get_cherrytree_config_styles_dirpath();
path get_cherrytree_config_icons_dirpath();
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
    friend void swap(path& lhs, path& rhs) {
        using std::swap;
        swap(lhs._path, rhs._path);
    }

    path() = default;
    path(std::string p) : _path{std::move(p)} {
#if !defined(_WIN32)
        _replaceStartingTilde();
#endif // !_WIN32
    }
    path(const char* cpath) : _path{cpath} {
#if !defined(_WIN32)
        _replaceStartingTilde();
#endif // !_WIN32
    }
    template<typename ITERATOR_T>
    path(ITERATOR_T begin, ITERATOR_T end) : _path{begin, end} {
#if !defined(_WIN32)
        _replaceStartingTilde();
#endif // !_WIN32
    }
    ~path() = default;

    void swap(path& other) {
        using std::swap;
        swap(*this, other);
    }

    path& operator=(std::string other) {
        _path = other;
#if !defined(_WIN32)
        _replaceStartingTilde();
#endif // !_WIN32
        return *this;
    }
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

    const char* c_str() const { return _path.c_str(); }
    std::string string() const { return _path; }
    bool is_absolute() const { return Glib::path_is_absolute(_path); }
    bool is_relative() const { return !is_absolute(); }
    void clear() { _path.clear(); }
    bool empty() const { return _path.empty(); }
    path filename() const { return Glib::path_get_basename(_path); }
    path parent_path() const { return Glib::path_get_dirname(_path); }
    std::string extension() const;
    std::string stem() const;
    std::string string_native() const;
    std::string string_unix() const;

private:
    std::string _path;
#if !defined(_WIN32)
    void _replaceStartingTilde();
#endif // !_WIN32
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
