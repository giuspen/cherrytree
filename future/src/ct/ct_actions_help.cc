/*
 * ct_actions_help.cc
 *
 * Copyright 2009-2020
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

#include "ct_actions.h"
#include <glib/gstdio.h>
#include <curl/curl.h>

void CtActions::online_help()
{
    g_app_info_launch_default_for_uri("https://giuspen.com/cherrytreemanual/", nullptr, nullptr);
}

void CtActions::dialog_about()
{
    CtDialogs::dialog_about(*_pCtMainWin, _pCtMainWin->get_icon_theme()->load_icon(CtConst::APP_NAME, 128));
}

void CtActions::folder_cfg_open()
{
    CtFileSystem::external_folderpath_open(Glib::build_filename(Glib::get_user_config_dir(), CtConst::APP_NAME));
}

static size_t __write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    const size_t realsize = size*nmemb;
    *static_cast<std::string*>(userp) += std::string{static_cast<const char*>(contents), realsize};
    return realsize;
};

std::string CtActions::_get_latest_version_from_server()
{
    // from https://curl.haxx.se/libcurl/c/getinmemory.html

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* pCurlHandle = curl_easy_init();

    std::string ret_str;

    curl_easy_setopt(pCurlHandle, CURLOPT_URL, "https://www.giuspen.com/software/version_cherrytree");
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, __write_memory_callback);
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, (void*)&ret_str);
    curl_easy_setopt(pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    const CURLcode res = curl_easy_perform(pCurlHandle);
    if (res != CURLE_OK) {
        g_warning("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(pCurlHandle);
    curl_global_cleanup();

    return str::trim(ret_str);
}

void CtActions::check_for_newer_version()
{
    auto& statusbar = _pCtMainWin->get_status_bar();
    statusbar.update_status(_("Checking for Newer Version..."));
    while (gtk_events_pending()) gtk_main_iteration();

    const std::string latest_version_from_server = _get_latest_version_from_server();
    //g_print("v='%s'\n", latest_version_from_server.c_str());
    if (latest_version_from_server.empty() or latest_version_from_server.size() > 10) {
        statusbar.update_status(_("Failed to Retrieve Latest Version Information - Try Again Later"));
    }
    else {
        std::vector<gint64> splitted_latest_v = CtStrUtil::gstring_split_to_int64(latest_version_from_server.c_str(), ".");
        std::vector<gint64> splitted_local_v = CtStrUtil::gstring_split_to_int64(PACKAGE_VERSION, ".");
        if (splitted_latest_v.size() != 3 or splitted_local_v.size() != 3) {
            g_critical("unexpected versions %s, %s", latest_version_from_server.c_str(), PACKAGE_VERSION);
        }
        else {
            gint64 weighted_latest_v = splitted_latest_v[0]*10000 + splitted_latest_v[1]*100 + splitted_latest_v[2];
            gint64 weighted_local_v = splitted_local_v[0]*10000 + splitted_local_v[1]*100 + splitted_local_v[2];
            if (weighted_latest_v > weighted_local_v) {
                CtDialogs::info_dialog(Glib::ustring{_("A Newer Version Is Available!")} + " (" PACKAGE_VERSION ")", *_pCtMainWin);
                _pCtMainWin->update_selected_node_statusbar_info();
            }
            else {
                if (weighted_latest_v == weighted_local_v) {
                    statusbar.update_status(Glib::ustring{_("You Are Using the Latest Version Available")} + " (" PACKAGE_VERSION ")");
                }
                else {
                    statusbar.update_status(_("You Are Using a Development Version"));
                }
            }
        }
    }
}
