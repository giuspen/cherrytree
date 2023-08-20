/*
 * ct_actions_help.cc
 *
 * Copyright 2009-2023
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

void CtActions::online_help()
{
    fs::open_weblink("https://giuspen.net/cherrytreemanual/");
}

void CtActions::online_home()
{
    fs::open_weblink("https://www.giuspen.net/cherrytree/");
}

void CtActions::online_code()
{
    fs::open_weblink("https://github.com/giuspen/cherrytree");
}

void CtActions::online_issues()
{
    fs::open_weblink("https://github.com/giuspen/cherrytree/issues");
}

void CtActions::dialog_about()
{
    CtDialogs::dialog_about(*_pCtMainWin, _pCtMainWin->get_icon_theme()->load_icon(CtConst::APP_NAME, 128));
}

void CtActions::folder_cfg_open()
{
    fs::open_folderpath(fs::get_cherrytree_configdir(), _pCtConfig);
}

void CtActions::check_for_newer_version()
{
    auto& statusbar = _pCtMainWin->get_status_bar();
    statusbar.update_status(_("Checking for Newer Version..."));
    while (gtk_events_pending()) gtk_main_iteration();

    std::string latest_debian_changelog_from_server = fs::download_file("https://raw.githubusercontent.com/giuspen/cherrytree/master/debian/changelog");
    std::size_t openp = latest_debian_changelog_from_server.find("(");
    std::size_t closep = latest_debian_changelog_from_server.find(")");
    if (std::string::npos == openp or std::string::npos == closep or closep < openp) {
        statusbar.update_status(_("Failed to Retrieve Latest Version Information - Try Again Later."));
        return;
    }
    std::string latest_version_from_server = latest_debian_changelog_from_server.substr(openp + 1, closep - openp - 1);
    auto re = Glib::Regex::create("(\\d+)\\.(\\d+)\\.(\\d+)-\\d+");
    Glib::MatchInfo match;
    if (not re->match(latest_version_from_server, match)) {
        statusbar.update_status(_("Failed to Retrieve Latest Version Information - Try Again Later."));
        return;
    }
    std::vector<gint64> splitted_latest_v = {atoi(match.fetch(1).c_str()), atoi(match.fetch(2).c_str()), atoi(match.fetch(3).c_str())};
    std::vector<gint64> splitted_local_v = CtStrUtil::gstring_split_to_int64(PACKAGE_VERSION, ".");
    if (splitted_local_v.size() != 3) {
        spdlog::error("unexpected version {}", PACKAGE_VERSION);
    }
    else {
        gint64 weighted_latest_v = splitted_latest_v[0]*10000 + splitted_latest_v[1]*100 + splitted_latest_v[2];
        gint64 weighted_local_v = splitted_local_v[0]*10000 + splitted_local_v[1]*100 + splitted_local_v[2];
        std::string trail_latest_from_srv = fmt::format(" ({}.{}.{})", splitted_latest_v[0], splitted_latest_v[1], splitted_latest_v[2]);
        if (weighted_latest_v > weighted_local_v) {
            CtDialogs::info_dialog(Glib::ustring{_("A Newer Version Is Available!")} + trail_latest_from_srv, *_pCtMainWin);
            _pCtMainWin->update_selected_node_statusbar_info();
        }
        else {
            if (weighted_latest_v == weighted_local_v) {
                statusbar.update_status(Glib::ustring{_("You Are Using the Latest Version Available.")} + trail_latest_from_srv);
            }
            else {
                statusbar.update_status(_("You Are Using a Development Version."));
                spdlog::debug("latest from server {}.{}.{}", splitted_latest_v[0], splitted_latest_v[1], splitted_latest_v[2]);
            }
        }
    }
}
