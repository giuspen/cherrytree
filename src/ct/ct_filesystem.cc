/*
  ct_filesystem.cc
 *
 * Copyright 2009-2026
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

#include <glibmm/miscutils.h>
#include <giomm/file.h>
#include <glib/gstdio.h>
#include <curl/curl.h>
#include <system_error>
#include <utility>
#include <unordered_map>

#include "ct_filesystem.h"
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_logging.h"
#include "ct_config.h"

namespace fs {

static fs::path _exePath;
static fs::path _portableConfigDir;
static std::unordered_map<std::string, std::pair<std::string,std::string>> _alteredLocaleEnvVars;
#if defined(_WIN32)
static fs::path _mingw64Dir;
#else
static fs::path _AppImageUsrDir;
#endif // _WIN32

// replacement of Glib::canonicalize_filename for Glibmm < 2.64
std::string legacy_canonicalize_filename(const std::string& filename, const std::string& relative_to/*= ""*/)
{
    // manage case where filename is an empty string -- just send it back unchanged
    if (filename == "")
      return "";

    std::string retFilepath;
    GFile* pGFile{nullptr};
    if (not Glib::path_is_absolute(filename) and not relative_to.empty()) {
        pGFile = g_file_new_for_path(Glib::build_filename(relative_to, filename).c_str());
    }
    else {
        pGFile = g_file_new_for_path(filename.c_str());
    }
    g_autofree gchar* pAbsPath = g_file_get_path(pGFile);
    g_object_unref(pGFile);
    retFilepath = pAbsPath;
    return retFilepath;
}

void _locale_env_vars_set_for_external_cmd(const bool isPre)
{
    for (const auto& currPair : _alteredLocaleEnvVars) {
        (void)Glib::setenv(currPair.first, isPre ? currPair.second.first : currPair.second.second, true/*overwrite*/);
    }
}

bool alter_locale_env_var(const std::string& key, const std::string& val)
{
    _alteredLocaleEnvVars[key] = std::make_pair(Glib::getenv(key), val);
    return Glib::setenv(key, val, true/*overwrite*/);
}

#if defined(_WIN32)
bool alter_TEXMFROOT_env_var()
{
    if (not _mingw64Dir.empty()) {
        const fs::path TEXMFROOT = _mingw64Dir / "share";
        const bool retVal = Glib::setenv("TEXMFROOT", TEXMFROOT.string_unix(), true/*overwrite*/);
        return retVal;
    }
    return false;
}
bool alter_PATH_env_var()
{
    if (not _mingw64Dir.empty()) {
        const fs::path PATH = _mingw64Dir / "bin";
        const bool retVal = Glib::setenv("PATH", Glib::getenv("PATH") + ";" + PATH.string_native(), true/*overwrite*/);
        return retVal;
    }
    return false;
}
#endif /* _WIN32 */

const char* get_latex_dvipng_console_bin_prefix()
{
    auto _get_latex_dvipng_console_bin_prefix = []()->const char*{
#if defined(_WIN32)
        if (not _mingw64Dir.empty()) {
            const fs::path binDir = _mingw64Dir / "bin";
            const fs::path latexBin = binDir / "latex.exe";
            const fs::path dvipngBin = binDir / "dvipng.exe";
            if (fs::is_regular_file(latexBin) and fs::is_regular_file(dvipngBin)) {
                spdlog::debug("found latex and dvipng in {}", binDir.c_str());
                return g_strdup(binDir.c_str());
            }
            spdlog::debug("?? NOT FOUND latex and dvipng in {}", binDir.c_str());
        }
#else /* !_WIN32 */
        const fs::path tinytex_bin_x86_64_linux = _exePath.parent_path() / "x86_64-linux";
        if (fs::is_directory(tinytex_bin_x86_64_linux)) {
            const fs::path tinytex_latex = tinytex_bin_x86_64_linux / "latex";
            const fs::path tinytex_dvipng = tinytex_bin_x86_64_linux / "dvipng";
            if (fs::is_regular_file(tinytex_latex) and fs::is_regular_file(tinytex_dvipng)) {
                spdlog::debug("found latex and dvipng in {}", tinytex_bin_x86_64_linux.c_str());
                return g_strdup_printf("cd %s && ./", tinytex_bin_x86_64_linux.c_str());
            }
            spdlog::debug("?? NOT FOUND latex and dvipng in {}", tinytex_bin_x86_64_linux.c_str());
        }
#endif /* !_WIN32 */
        return g_strdup("");
    };
    static const char* latex_dvipng_console_bin_prefix = _get_latex_dvipng_console_bin_prefix();
    return latex_dvipng_console_bin_prefix;
}

void register_exe_path_detect_if_portable(const char* exe_path)
{
    _exePath = fs::canonical(exe_path);
    // spdlog is not up yet here!
    //printf("exePath: %s\nAPPIMAGE=%s\n", _exePath.c_str(), Glib::getenv("APPIMAGE").c_str());
#if defined(_WIN32)
    // e.g. cherrytree_1.5.0.0_win64_portable\ucrt64\bin\cherrytree.exe
    //      cherrytree_1.5.0.0_win64_portable\config.cfg
    _mingw64Dir = _exePath.parent_path().parent_path();
    const fs::path dirname = _mingw64Dir.filename();
    if (dirname != "ucrt64" and dirname != "mingw64") {
        _mingw64Dir.clear();
    }
    const fs::path portableConfigDir = _mingw64Dir.parent_path();
#else // !_WIN32
    const fs::path portableConfigDir = _exePath.parent_path() / "config";
    if (not Glib::getenv("APPIMAGE").empty()) {
        // APPIMAGE=/home/giuspen/git/cherrytree/build/CherryTree-825b1d77-x86_64.AppImage
        // exePath: /tmp/.mount_CherryaUjoTO/usr/bin/cherrytree
        _AppImageUsrDir = _exePath.parent_path().parent_path();
    }
#endif // !_WIN32
    const fs::path portableConfigFile = portableConfigDir / CtConfig::ConfigFilename;
    if (is_regular_file(portableConfigFile)) {
        _portableConfigDir = portableConfigDir;
    }
}

bool remove(const fs::path& path2rm)
{
    if (fs::is_directory(path2rm)) {
        if (g_rmdir(path2rm.c_str()) != 0) {
            spdlog::error("fs::remove: g_rmdir failed to remove {}", path2rm.string());
            return false;
        }
    }
    else if (fs::exists(path2rm)) {
        if (::g_remove(path2rm.c_str()) != 0) {
            spdlog::error("fs::remove: g_remove failed to remove {}", path2rm.string());
            return false;
        }
    }
    else {
        return false;
    }
    return true;
}

bool exists(const path& filepath)
{
    return Glib::file_test(filepath.string(), Glib::FILE_TEST_EXISTS);
}

bool is_regular_file(const path& file)
{
    return Glib::file_test(file.string(), Glib::FILE_TEST_IS_REGULAR);
}

bool is_directory(const fs::path& p)
{
    return Glib::file_test(p.string(), Glib::FILE_TEST_IS_DIR);
}

bool is_file_image(const path& file_path)
{
#if defined(_WIN32)
    const Glib::ustring ext = Glib::ustring{file_path.extension()}.lowercase();
    return vec::exists(std::vector<Glib::ustring>{".png", ".jpg", ".jpeg", ".gif", ".bmp", ".svg"}, ext);
#else
    g_autofree gchar* mimetype = g_content_type_guess(file_path.c_str(), nullptr, 0, nullptr);
    return mimetype and str::startswith(mimetype, "image/");
#endif
}

bool copy_file(const path& from, const path& to)
{
    try {
        Glib::RefPtr<Gio::File> rFileFrom = Gio::File::create_for_path(from.string());
        Glib::RefPtr<Gio::File> rFileTo = Gio::File::create_for_path(to.string());
        return rFileFrom->copy(rFileTo, Gio::FILE_COPY_OVERWRITE);
    }
    catch (Gio::Error& error) {
        spdlog::debug("fs::copy_file, error: {}, from: {}, to: {}", error.what().raw(), from.string(), to.string());
        return false;
    }
}

bool move_file(const path& from, const path& to)
{
    GFile* pGFile_from = g_file_new_for_path(from.c_str());
    GFile* pGFile_to = g_file_new_for_path(to.c_str());
    GError* pError = NULL;
    bool retSuccess = g_file_move(pGFile_from,
                                  pGFile_to,
                                  G_FILE_COPY_OVERWRITE,
                                  NULL,
                                  NULL,  // GFileProgressCallback
                                  NULL,  // data
                                  &pError);
    if (pError) {
        spdlog::warn("{}, error: {}, from: {}, to: {}", __FUNCTION__, pError->message, from.string(), to.string());
        g_error_free(pError);
    }
    g_object_unref(pGFile_from);
    g_object_unref(pGFile_to);
    return retSuccess;
}

path absolute(const path& p)
{
    GFile* pGFile = g_file_new_for_path(p.c_str());
    g_autofree gchar* pAbsPath = g_file_get_path(pGFile);
    path retPath{pAbsPath};
    g_object_unref(pGFile);
    return retPath;
}

time_t getmtime(const path& p)
{
    time_t time = 0;
    GStatBuf st;
    if (g_stat(p.c_str(), &st) == 0)
        time = st.st_mtime;
    return time;
}

std::uintmax_t file_size(const path& p)
{
    if (fs::is_directory(p)) {
        spdlog::error("fs::file_size: path is a directory, {}", p.string());
        return 0;
    }

    GStatBuf st;
    if (g_stat(p.c_str(), &st) != 0) {
        spdlog::error("fs::file_size: g_stat failed, {}", p.string());
        return 0;
    }

    return st.st_size;
}

std::list<fs::path> get_dir_entries(const path& dir)
{
    Glib::Dir gdir(dir.string());
    std::list<fs::path> entries(gdir.begin(), gdir.end());
    for (auto& entry : entries)
        entry = dir / entry;
    return entries;
}

void open_weblink(const std::string& link)
{
#if defined(_WIN32)
    glong utf16text_len = 0;
    g_autofree gunichar2* utf16text = g_utf8_to_utf16(link.c_str(), (glong)Glib::ustring(link.c_str()).bytes(), nullptr, &utf16text_len, nullptr);
    ShellExecuteW(GetActiveWindow(), L"open", (LPCWSTR)utf16text, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    GError *error = nullptr;
    if (!g_app_info_launch_default_for_uri(link.c_str(), nullptr, &error))
    {
        spdlog::debug("fs::open_weblink failed to open link: {}, error: {}", link, error->message);
        g_error_free(error);
    }
#else
    std::vector<std::string> argv = { "xdg-open", link};
    Glib::spawn_async("", argv, Glib::SpawnFlags::SPAWN_SEARCH_PATH);
    // g_app_info_launch_default_for_uri(link.c_str(), nullptr, nullptr); // doesn't work on KDE
#endif
}

void _open_path_with_default_app(const fs::path& file_or_folder_path)
{
    _locale_env_vars_set_for_external_cmd(true/*isPre*/);
#ifdef _WIN32
    // https://stackoverflow.com/questions/42442189/how-to-open-spawn-a-file-with-glib-gtkmm-in-windows
    glong utf16text_len = 0;
    g_autofree gunichar2* utf16text = g_utf8_to_utf16(file_or_folder_path.c_str(),
                                                      (glong)Glib::ustring(file_or_folder_path.c_str()).bytes(),
                                                      nullptr,
                                                      &utf16text_len,
                                                      nullptr);
    ShellExecuteW(GetActiveWindow(), L"open", (LPCWSTR)utf16text, NULL, NULL, SW_SHOWDEFAULT);
#elif defined(__APPLE__)
    Glib::RefPtr<Gio::File> gioFile = Gio::File::create_for_path(file_or_folder_path.string());
    std::vector<std::string> argv = { "open", gioFile->get_uri() };
    Glib::spawn_async("", argv, Glib::SpawnFlags::SPAWN_SEARCH_PATH);
#else
    Glib::RefPtr<Gio::File> gioFile = Gio::File::create_for_path(file_or_folder_path.string());
    std::vector<std::string> argv = { "xdg-open", gioFile->get_uri() };
    Glib::spawn_async("", argv, Glib::SpawnFlags::SPAWN_SEARCH_PATH);
    // g_app_info_launch_default_for_uri(f_path.c_str(), nullptr, nullptr); // doesn't work on KDE
#endif
    _locale_env_vars_set_for_external_cmd(false/*isPre*/);
}

// Open Filepath with External App
void open_filepath(const fs::path& filepath, bool open_folder_if_file_not_exists, CtConfig* config)
{
    spdlog::debug("fs::open_filepath {}", filepath.string());
    if (config->filelinkCustomOn) {
        std::string cmd = config->filelinkCustomAct;
        g_autofree gchar* quoted = g_shell_quote(filepath.string().c_str());
        std::string quotedPath(quoted);
        
        // Replace "file://%s" with just %s (without the double quotes from template)
        size_t filePos = cmd.find("\"file://%s\"");
        if (filePos != std::string::npos) {
            cmd.replace(filePos, 11, quotedPath);  // Replace the entire "file://%s" with quoted path
        }
        else {
            // Fallback: just replace %s if "file://%s" pattern not found
            size_t pos = 0;
            while ((pos = cmd.find("%s", pos)) != std::string::npos) {
                cmd.replace(pos, 2, quotedPath);
                pos += quotedPath.length();
            }
        }
        
        _locale_env_vars_set_for_external_cmd(true/*isPre*/);
        const int retVal = std::system(cmd.c_str());
        _locale_env_vars_set_for_external_cmd(false/*isPre*/);
        if (retVal != 0) {
            spdlog::error("system({}) returned {}", cmd, retVal);
        }
    }
    else {
        if (open_folder_if_file_not_exists && !fs::exists(filepath)) {
            open_folderpath(filepath, config);
        }
        else if (!fs::exists(filepath)) {
            spdlog::error("fs::open_filepath: file doesn't exist, {}", filepath.string());
            return;
        }
        else {
            _open_path_with_default_app(filepath);
        }
    }
}

// Open Folderpath with External App
void open_folderpath(const fs::path& folderpath, CtConfig* config)
{
    spdlog::debug("fs::open_folderpath {}", folderpath.string());
    if (config->folderlinkCustomOn) {
        std::string cmd = config->folderlinkCustomAct;
        g_autofree gchar* quoted = g_shell_quote(folderpath.string().c_str());
        std::string quotedPath(quoted);
        
        // Replace "file://%s" with just %s (without the double quotes from template)
        size_t filePos = cmd.find("\"file://%s\"");
        if (filePos != std::string::npos) {
            cmd.replace(filePos, 11, quotedPath);  // Replace the entire "file://%s" with quoted path
        }
        else {
            // Fallback: just replace %s if "file://%s" pattern not found
            size_t pos = 0;
            while ((pos = cmd.find("%s", pos)) != std::string::npos) {
                cmd.replace(pos, 2, quotedPath);
                pos += quotedPath.length();
            }
        }
        
        _locale_env_vars_set_for_external_cmd(true/*isPre*/);
        const int retVal = std::system(cmd.c_str());
        _locale_env_vars_set_for_external_cmd(false/*isPre*/);
        if (retVal != 0) {
            spdlog::error("system({}) returned {}", cmd, retVal);
        }
    }
    else {
        _open_path_with_default_app(folderpath);
    }
}

path prepare_export_folder(const path& dir_place, path new_folder, bool overwrite_existing)
{
    const fs::path dir_place_new_folder = dir_place / new_folder;
    if (fs::is_directory(dir_place_new_folder)) {
        // todo:
        if (overwrite_existing) {
            spdlog::debug("fs::prepare_export_folder: removing dir {}", dir_place_new_folder.string());
            remove_all(dir_place_new_folder);
        }
        else {
            int n = 2;
            while (fs::is_directory(dir_place / (new_folder.string() + fmt::format("{:03d}", n))))
                n += 1;
            new_folder += fmt::format("{:03d}", n);
        }
    }
    return new_folder;
}

std::uintmax_t remove_all(const path& dir)
{
    std::uintmax_t count{0u};
    if (fs::is_directory(dir)) {
        for (const auto& file : get_dir_entries(dir)) {
            if (is_directory(file)) {
                count += remove_all(file);
            }
            else {
                ++count;
                remove(file);
            }
        }
    }
    if (fs::exists(dir)) {
        remove(dir);
        ++count;
    }
    return count;
}

fs::path get_cherrytree_datadir()
{
    if (_exePath.parent_path() == fs::canonical(_CMAKE_BINARY_DIR)) {
        // we're running from the build sources
        return _CMAKE_SOURCE_DIR;
    }
#ifdef _WIN32
    // e.g. cherrytree_1.5.0.0_win64_portable\ucrt64\bin\cherrytree.exe
    //      cherrytree_1.5.0.0_win64_portable\ucrt64\usr\share\cherrytree\language-specs
    //      cherrytree_1.5.0.0_win64_portable\ucrt64\usr\share\cherrytree\styles
    //      cherrytree_1.5.0.0_win64_portable\ucrt64\usr\share\cherrytree\data
    //      cherrytree_1.5.0.0_win64_portable\ucrt64\usr\share\cherrytree\icons
    return _mingw64Dir / "usr" / "share" / "cherrytree";
#else
    if (not _AppImageUsrDir.empty()) {
        return _AppImageUsrDir / "share" / "cherrytree";
    }
    return CHERRYTREE_DATADIR;
#endif // _WIN32
}

fs::path get_cherrytree_localedir()
{
    if (_exePath.parent_path() == fs::canonical(_CMAKE_BINARY_DIR)) {
        // we're running from the build sources
        return fs_canonicalize_filename(Glib::build_filename(_CMAKE_SOURCE_DIR, "po"));
    }
#if defined(_WIN32)
    // e.g. cherrytree_1.5.0.0_win64_portable\ucrt64\bin\cherrytree.exe
    //      cherrytree_1.5.0.0_win64_portable\ucrt64\share\locale
    return _mingw64Dir / "share" / "locale";
#elif defined(_FLATPAK_BUILD)
   return CHERRYTREE_DATADIR "/locale";
#else
    if (not _AppImageUsrDir.empty()) {
        return _AppImageUsrDir / "share" / "locale";
    }
    return CHERRYTREE_LOCALEDIR;
#endif // _WIN32
}

fs::path get_cherrytree_configdir()
{
    if (not _portableConfigDir.empty()) {
        return _portableConfigDir;
    }
    return Glib::build_filename(Glib::get_user_config_dir(), CtConst::APP_NAME);
}

std::optional<fs::path> get_cherrytree_logdir()
{
    const fs::path logcfgFilepath = fs::get_cherrytree_logcfg_filepath();
    if (not fs::is_regular_file(logcfgFilepath)) {
        return std::nullopt; // file missing => no log
    }
    const std::string logDirpath = str::trim(Glib::file_get_contents(logcfgFilepath.string()));
    if (logDirpath.empty()) {
        return get_cherrytree_configdir(); // file empty => log in config dir
    }
    if (fs::is_directory(logDirpath)) {
        return fs::path{logDirpath}; // valid directory => OK return it
    }
    return std::nullopt; // invalid directory => no log
}

fs::path get_cherrytree_print_page_setup_cfg_filepath()
{
    return fs::canonical(get_cherrytree_configdir() / CtConfig::PrintPageSetupFilename);
}

fs::path get_cherrytree_langcfg_filepath()
{
    return fs::canonical(get_cherrytree_configdir() / CtConfig::LangFilename);
}

fs::path get_cherrytree_logcfg_filepath()
{
    return fs::canonical(get_cherrytree_configdir() / CtConfig::LogFilename);
}

fs::path get_cherrytree_config_filepath()
{
    return fs::canonical(get_cherrytree_configdir() / CtConfig::ConfigFilename);
}

fs::path get_cherrytree_config_language_specs_dirpath()
{
    return get_cherrytree_configdir() / CtConfig::ConfigLanguageSpecsDirname;
}

fs::path get_cherrytree_config_styles_dirpath()
{
    return get_cherrytree_configdir() / CtConfig::ConfigStylesDirname;
}

fs::path get_cherrytree_config_icons_dirpath()
{
    return get_cherrytree_configdir() / CtConfig::ConfigIconsDirname;
}

fs::path get_cherrytree_config_user_style_filepath(const unsigned num)
{
    return fs::canonical(get_cherrytree_config_styles_dirpath() / ("user-style-" + std::to_string(num) + ".xml"));
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
    CtConfig* pCtConfig = CtConfig::GetCtConfig();

    spdlog::debug("fs::download_file: start downloading {}", filepath);

    std::string buffer;
    buffer.reserve(3 * 1024 * 1024); // preallocate 3mb

    // from https://curl.haxx.se/libcurl/c/getinmemory.html
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* pCurlHandle = curl_easy_init();

    curl_easy_setopt(pCurlHandle, CURLOPT_URL, filepath.c_str());
    if (not pCtConfig->proxyUrlColonPort.empty()) {
        curl_easy_setopt(pCurlHandle, CURLOPT_PROXY, pCtConfig->proxyUrlColonPort.c_str());
        if (not pCtConfig->proxyUsername.empty()) {
            curl_easy_setopt(pCurlHandle, CURLOPT_PROXYUSERNAME, pCtConfig->proxyUsername.c_str());
            if (not pCtConfig->proxyPassword.empty()) {
                curl_easy_setopt(pCurlHandle, CURLOPT_PROXYPASSWORD, pCtConfig->proxyPassword.c_str());
            }
        }
    }
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, local::write_memory_callback);
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, (void*)&buffer);
    curl_easy_setopt(pCurlHandle, CURLOPT_TIMEOUT, 3);
    curl_easy_setopt(pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    const CURLcode res = curl_easy_perform(pCurlHandle);
    curl_easy_cleanup(pCurlHandle);
    curl_global_cleanup();

    if (res != CURLE_OK) {
        spdlog::error("fs::download_file: curl_easy_perform() failed, {}", curl_easy_strerror(res));
        return "";
    }

    return buffer;
}

CtDocType get_doc_type_from_file_ext(const fs::path& filename)
{
    const std::string file_ext = filename.extension();
    if (CtConst::CTDOC_XML_NOENC == file_ext or
        CtConst::CTDOC_XML_ENC == file_ext)
    {
        return CtDocType::XML;
    }
    if (CtConst::CTDOC_SQLITE_NOENC == file_ext or
        CtConst::CTDOC_SQLITE_ENC == file_ext)
    {
        return CtDocType::SQLite;
    }
    return CtDocType::None;
}

CtDocEncrypt get_doc_encrypt_from_file_ext(const fs::path& filename)
{
    const std::string file_ext = filename.extension();
    if (CtConst::CTDOC_XML_NOENC == file_ext or
        CtConst::CTDOC_SQLITE_NOENC == file_ext)
    {
        return CtDocEncrypt::False;
    }
    if (CtConst::CTDOC_XML_ENC == file_ext or
        CtConst::CTDOC_SQLITE_ENC == file_ext)
    {
        return CtDocEncrypt::True;
    }
    return CtDocEncrypt::None;
}

path canonical(const path& p, const bool resolveSymlink)
{
    if (resolveSymlink and Glib::file_test(p.string(), Glib::FILE_TEST_IS_SYMLINK)) {
        g_autoptr(GError) pError{nullptr};
        g_autofree gchar* pOutStr = g_file_read_link(p.c_str(), &pError);
        if (pOutStr) {
            if (g_path_is_absolute(pOutStr)) {
                return fs_canonicalize_filename(pOutStr);
            }
            return fs_canonicalize_filename(pOutStr, Glib::path_get_dirname(p.string()));
        }
    }
    return fs_canonicalize_filename(p.string());
}

path canonical(const path& p, const path& base)
{
    return fs_canonicalize_filename(p.string(), base.string());
}

path relative(const path& p, const path& base)
{
    GFile* pGFile_File = g_file_new_for_path(p.c_str());
    GFile* pGFile_Dir = g_file_new_for_path(base.c_str());
    auto on_scope_exit = scope_guard([&](void*) {
        g_object_unref(pGFile_File);
        g_object_unref(pGFile_Dir);
    });
    {
        g_autofree gchar* pRelPath = g_file_get_relative_path(pGFile_Dir, pGFile_File);
        if (pRelPath) {
            return path{pRelPath};
        }
    }
    unsigned countUp{0};
    while (g_file_has_parent(pGFile_Dir, NULL)) {
        GFile* pGFile_TmpDir = g_file_get_parent(pGFile_Dir);
        g_object_unref(pGFile_Dir);
        pGFile_Dir = pGFile_TmpDir;
        ++countUp;
        g_autofree gchar* pRelPath = g_file_get_relative_path(pGFile_Dir, pGFile_File);
        if (pRelPath) {
            path retPath{pRelPath};
            for (unsigned i = 0; i < countUp; ++i) {
                retPath = ".." / retPath;
            }
            return retPath;
        }
    }
    return p;
}

std::string path::extension() const
{
    std::string name = filename().string();
    auto last_pos = name.find_last_of('.');
    if (last_pos == std::string::npos || last_pos == name.size() - 1 || last_pos == 0) {
        return "";
    }
    return name.substr(last_pos);
}

std::string path::stem() const
{
    if (empty()) return "";
    std::string name = filename().string();
    size_t dot_pos = name.find_last_of('.');
    if (dot_pos == std::string::npos || dot_pos == 0)
        return name;
    return name.substr(0, dot_pos);
}

std::string path::string_native() const
{
#ifdef _WIN32
    return str::replace(_path, CtConst::CHAR_SLASH, CtConst::CHAR_BSLASH);
#else
    return str::replace(_path, CtConst::CHAR_BSLASH, CtConst::CHAR_SLASH);
#endif
}

std::string path::string_unix() const
{
    return str::replace(_path, CtConst::CHAR_BSLASH, CtConst::CHAR_SLASH);
}

#if !defined(_WIN32)
void path::_replaceStartingTilde()
{
    if (str::startswith(_path, "~/")) {
        _path = Glib::get_home_dir() + _path.substr(1);
    }
}
#endif // !_WIN32

} // namespace fs
