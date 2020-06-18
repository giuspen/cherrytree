/*
 * ct_filesystem.cc
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

#include <glibmm/miscutils.h>
#include <glib/gstdio.h>
#include <curl/curl.h>
#include <spdlog/fmt/bundled/printf.h>
#include <system_error>

#include "ct_filesystem.h"
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_logging.h"
#include "ct_config.h"

namespace CtFileSystem {
void path::append(const path& other)
{ 
    _path = Glib::build_filename(_path, other._path); 
}

path path::extension() const
{
    auto iter = _path.cend();
    while (iter != _path.cbegin()) {
        --iter;
        if (*iter == '.') break;
        if (*iter == path_sep) return path(""); // Cannot have an extension if not yet found
    }
    if (iter == _path.cend() || (iter == _path.cbegin() && _path.size() > 1)) return path("");

    return path(iter, _path.cend());
}

path& path::operator=(path::string_type other)
{
    _path = _get_platform_path(std::move(other));
    return *this;
}



bool remove(const CtFileSystem::path& path)
{
    if (fs::is_directory(path)) {
        // rmdir
        int retr = g_rmdir(path.c_str());
        if (retr != 0) {
            // error
            throw fs::filesystem_error("Error occurred during g_rmdir", path, std::error_code(retr, std::generic_category()));
        }
        g_unlink(path.c_str());
    }
    return true;
}

bool is_regular_file(const path& file)
{
    return Glib::file_test(file.string(), Glib::FILE_TEST_IS_REGULAR);
}


std::string path::_get_platform_path(std::string filepath)
{
#ifdef _WIN32
    filepath = str::replace(filepath, CtConst::CHAR_SLASH, CtConst::CHAR_BSLASH);
#else
    filepath = str::replace(filepath, CtConst::CHAR_BSLASH, CtConst::CHAR_SLASH);
#endif
    return filepath;
}

bool copy_file(const path& from, const path& to)
{
    Glib::RefPtr<Gio::File> rFileFrom = Gio::File::create_for_path(from.string());
    Glib::RefPtr<Gio::File> rFileTo = Gio::File::create_for_path(to.string());
    return rFileFrom->copy(rFileTo, Gio::FILE_COPY_OVERWRITE);
}

bool is_directory(const fs::path& path) {
    return Glib::file_test(path.string(), Glib::FILE_TEST_IS_DIR);
}

bool move_file(const path& from, const path& to)
{
    Glib::RefPtr<Gio::File> rFileFrom = Gio::File::create_for_path(from.string());
    Glib::RefPtr<Gio::File> rFileTo = Gio::File::create_for_path(to.string());
    return rFileFrom->move(rFileTo, Gio::FILE_COPY_OVERWRITE);
}

fs::path absolute(const fs::path& path)
{
    Glib::RefPtr<Gio::File> rFile = Gio::File::create_for_path(path.string());
    return rFile->get_path();
}

time_t getmtime(const path& path)
{
    time_t time = 0;
    GStatBuf st;
    if (g_stat(path.c_str(), &st) == 0)
        time = st.st_mtime;
    return time;
}

int getsize(const std::string& path)
{
    GStatBuf st;
    if (g_stat(path.c_str(), &st) == 0)
        return st.st_size;
    return 0;
}

std::list<CtFileSystem::path> get_dir_entries(const path& dir)
{
    Glib::Dir gdir(dir.string());
    std::list<CtFileSystem::path> entries(gdir.begin(), gdir.end());
    for (auto& entry: entries)
        entry = dir / entry;
    return entries;
}

path path::stem() const
{
    if (empty()) return "";
    std::string name = filename().string();
    size_t dot_pos = name.find_last_of('.');
    if (dot_pos == std::string::npos || dot_pos == 0)
        return name;
    return name.substr(0, dot_pos);
}

bool exists(const path& filepath) {
    return Glib::file_test(filepath.string(), Glib::FILE_TEST_EXISTS);
}

// Open Filepath with External App
void external_filepath_open(const fs::path& filepath, bool open_folder_if_file_not_exists, CtConfig* config)
{
    spdlog::debug("filepath to open: {}", filepath);
    if (config->filelinkCustomOn) {
        std::string cmd = fmt::sprintf(config->filelinkCustomAct, filepath.string());
        std::system(cmd.c_str());
    } else {
        if (open_folder_if_file_not_exists && !fs::exists(filepath)) {
            external_folderpath_open(filepath, config);
        } else if (!fs::exists(filepath)) {
            throw std::runtime_error(fmt::format("Filepath: {} does not exist and open_folder_if_not_exists is false", filepath.string()));
        } else {
#ifdef _WIN32
            ShellExecute(GetActiveWindow(), "open", filepath.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
            fs::path f_path("file://");
            f_path += filepath;
            g_app_info_launch_default_for_uri(f_path.c_str(), nullptr, nullptr);
#endif
        }
    }
}

// Open Folderpath with External App
void external_folderpath_open(const fs::path& folderpath, CtConfig* config)
{
    spdlog::debug("dir to open: {}", folderpath.string());
    if (config->folderlinkCustomOn) {
        std::string cmd = fmt::sprintf(config->filelinkCustomAct, folderpath.string());
        std::system(cmd.c_str());
    } else {

        // https://stackoverflow.com/questions/42442189/how-to-open-spawn-a-file-with-glib-gtkmm-in-windows
#ifdef _WIN32
        ShellExecute(NULL, "open", folderpath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif defined(__APPLE__)
        std::vector<std::string> argv = { "open", folderpath.string() };
Glib::spawn_async("", argv, Glib::SpawnFlags::SPAWN_SEARCH_PATH);
#else
        fs::path path("file://");
        path += folderpath;
        g_app_info_launch_default_for_uri(folderpath.c_str(), nullptr, nullptr);
#endif
    }
}

path prepare_export_folder(const path& dir_place, path new_folder, bool overwrite_existing)
{
    if (fs::is_directory(dir_place / new_folder))
    {
        // todo:
        if (overwrite_existing) {
            spdlog::debug("removing dir: {}", dir_place / new_folder);
            remove_all(dir_place / new_folder);
        }
        else {
            int n = 2;
            while (fs::is_directory(dir_place / (new_folder.string() + str::format("{:03d}", n))))
                n += 1;
            new_folder += str::format("{:03d}", n);
        }
    }
    return new_folder;
}

std::uintmax_t remove_all(const path& dir)
{
    std::uintmax_t count = 0;
    for (const auto& file : get_dir_entries(dir)) {
        ++count;
        if (is_directory(file)) {
            count += remove_all(file);
        }
        remove(file);
    }
    remove(dir);
    ++count;
    return count;
}

std::string get_cherrytree_datadir()
{
    if (Glib::file_test(_CMAKE_BINARY_DIR, Glib::FILE_TEST_IS_DIR)) {
        // we're running from the build sources
        return _CMAKE_SOURCE_DIR;
    }
    return CHERRYTREE_DATADIR;
}

fs::path get_cherrytree_localedir()
{
    std::string sources_po_dir = Glib::canonicalize_filename(Glib::build_filename(_CMAKE_SOURCE_DIR, "po"));
    if (Glib::file_test(sources_po_dir, Glib::FILE_TEST_IS_DIR)) {
        // we're running from the build sources
        return sources_po_dir;
    }
    return CHERRYTREE_LOCALEDIR;
}

std::string get_cherrytree_configdir()
{
    //TODO: define rule for local config.cfg/lang files at least for Windows portable
    return Glib::build_filename(Glib::get_user_config_dir(), CtConst::APP_NAME);
}

std::string get_cherrytree_lang_filepath()
{
    return Glib::build_filename(get_cherrytree_configdir(), "lang");
}

std::string download_file(const std::string& filepath)
{
    struct local {
        static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp)
        {
            const size_t realsize = size*nmemb;
            static_cast<std::string*>(userp)->append((char*)contents, realsize);
            return realsize;
        }
    };

    spdlog::debug("start downloading {}", filepath);

    std::string buffer;
    buffer.reserve(3 * 1024 * 1024); // preallocate 3mb

    // from https://curl.haxx.se/libcurl/c/getinmemory.html
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* pCurlHandle = curl_easy_init();

    curl_easy_setopt(pCurlHandle, CURLOPT_URL, filepath.c_str());
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, local::write_memory_callback);
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, (void*)&buffer);
    curl_easy_setopt(pCurlHandle, CURLOPT_TIMEOUT, 3);
    curl_easy_setopt(pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    const CURLcode res = curl_easy_perform(pCurlHandle);
    curl_easy_cleanup(pCurlHandle);
    curl_global_cleanup();

    if (res != CURLE_OK) {
        spdlog::error("curl_easy_perform() failed: {}", curl_easy_strerror(res));
        return "";
    }

    return buffer;
}


CtDocType get_doc_type(const fs::path& filename)
{
    CtDocType retDocType{CtDocType::None};
    if ((filename.extension() == CtConst::CTDOC_XML_NOENC) or
         (filename.extension() == CtConst::CTDOC_XML_ENC))
    {
        retDocType = CtDocType::XML;
    }
    else if ((filename.extension() == CtConst::CTDOC_SQLITE_NOENC) or
             (filename.extension() == CtConst::CTDOC_SQLITE_ENC))
    {
        retDocType = CtDocType::SQLite;
    }
    return retDocType;
}

CtDocEncrypt get_doc_encrypt(const fs::path& filename)
{
    CtDocEncrypt retDocEncrypt{CtDocEncrypt::None};
    if ( (filename.extension() == CtConst::CTDOC_XML_NOENC) or
         (filename.extension() == CtConst::CTDOC_SQLITE_NOENC))
    {
        retDocEncrypt = CtDocEncrypt::False;
    }
    else if ( (filename.extension() == CtConst::CTDOC_XML_ENC) or
              (filename.extension() == CtConst::CTDOC_SQLITE_ENC))
    {
        retDocEncrypt = CtDocEncrypt::True;
    }
    return retDocEncrypt;
}

path canonical(const path& path)
{
    return Glib::canonicalize_filename(path.string());
}

}
