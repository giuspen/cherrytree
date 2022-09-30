/*
 * ct_main_win_vte.cc
 *
 * Copyright 2009-2022
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

#include "ct_main_win.h"
#if defined(HAVE_VTE)
#include <vte/vte.h>
#endif // HAVE_VTE

#if defined(HAVE_VTE)
static void _vteTerminalSpawnAsyncCallback(VteTerminal*/*terminal*/,
                                           GPid pid,
                                           GError* error,
                                           gpointer/*user_data*/)
{
    if (-1 != pid) {
        spdlog::debug("+VTE");
    }
    else {
        spdlog::error("!! VTE");
    }
    if (NULL != error) {
        spdlog::error("{}", error->message);
        g_clear_error(&error);
    }
}
#endif // HAVE_VTE

void CtMainWin::show_hide_vte(bool visible)
{
#if defined(HAVE_VTE)
    if (not _pVte) {
        GtkWidget* pTermWidget = vte_terminal_new();
        _pVte = Gtk::manage(Glib::wrap(pTermWidget));
        vte_terminal_set_scrollback_lines(VTE_TERMINAL(pTermWidget), -1/*infinite*/);

        char* startterm[2] = {(char*)"/bin/sh", 0};
        vte_terminal_spawn_async(VTE_TERMINAL(pTermWidget),
                                 VTE_PTY_DEFAULT,
                                 NULL/*working_directory*/,
                                 startterm/*argv*/,
                                 NULL/*envv*/,
                                 G_SPAWN_DEFAULT/*spawn_flags_*/,
                                 NULL/*child_setup*/,
                                 NULL/*child_setup_data*/,
                                 NULL/*child_setup_data_destroy*/,
                                 -1/*timeout*/,
                                 NULL/*cancellable*/,
                                 &_vteTerminalSpawnAsyncCallback,
                                 NULL/*user_data*/);

        _scrolledwindowVte.add(*_pVte);
        _pVte->show();
    }
#endif // HAVE_VTE
    _scrolledwindowVte.property_visible() = visible;
}

void CtMainWin::exec_in_vte(const std::string& shell_cmd)
{
    show_hide_vte(true/*visible*/);
#if defined(HAVE_VTE)
    vte_terminal_feed_child(VTE_TERMINAL(_pVte->gobj()), shell_cmd.c_str(), shell_cmd.size());
#else // !HAVE_VTE
    spdlog::warn("!! noVte {}", shell_cmd);
#endif // !HAVE_VTE
}
