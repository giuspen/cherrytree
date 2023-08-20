/*
 * ct_main_win_vte.cc
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

#include "ct_main_win.h"
#include "ct_actions.h"
#if defined(HAVE_VTE)
#include <vte/vte.h>
#endif // HAVE_VTE

// The following macro is copied from Geany https://github.com/geany/geany/blob/master/src/vte.c
static const gchar VTE_ADDITIONAL_WORDCHARS[] = "-,./?%&#:_";

struct CtVteSpawnAsyncData {
    GPid* pPid;
    char* pCmd;
};

#if defined(HAVE_VTE)
static void _vteTerminalSpawnAsyncCallback(VteTerminal* pVteTerminal,
                                           GPid pid,
                                           GError* error,
                                           gpointer user_data)
{
    auto pSpawnAsyncData = (CtVteSpawnAsyncData*)user_data;
    *(pSpawnAsyncData->pPid) = pid;
    auto f_freemem = [pSpawnAsyncData](){
        if (pSpawnAsyncData->pCmd) {
            g_free(pSpawnAsyncData->pCmd);
        }
        delete pSpawnAsyncData;
    };
    if (-1 != pid) {
        if (pSpawnAsyncData->pCmd) {
            spdlog::debug("+VTE cmd");
            Glib::signal_idle().connect_once([pVteTerminal, pSpawnAsyncData, f_freemem](){
                vte_terminal_feed_child(pVteTerminal, pSpawnAsyncData->pCmd, strlen(pSpawnAsyncData->pCmd));
                f_freemem();
            });
        }
        else {
            spdlog::debug("+VTE");
            f_freemem();
        }
    }
    else {
        spdlog::error("!! VTE");
        f_freemem();
    }
    if (NULL != error) {
        spdlog::error("{}", error->message);
        g_clear_error(&error);
    }
}
#endif // HAVE_VTE

bool CtMainWin::show_hide_vte_cmd_passed_as_first_in_session(bool visible, const char* first_cmd_passed)
{
    bool ret_val{false};
    if (visible and not _pVte) {
        restart_vte(first_cmd_passed);
        ret_val = true;
    }
    _hBoxVte.property_visible() = visible;
    return ret_val;
}

void CtMainWin::restart_vte(const char* first_cmd_passed)
{
#if defined(HAVE_VTE)
    if (_pVte) {
        vte_terminal_reset(VTE_TERMINAL(_pVte->gobj()), true/*clear_tabstops*/, true/*clear_history*/);
    }
    for (Gtk::Widget* pWidget : _hBoxVte.get_children()) {
        _hBoxVte.remove(*pWidget);
    }
    if (_vtePid > 0) {
        std::string shell_cmd = fmt::format("kill {}", _vtePid);
        int retVal = std::system(shell_cmd.c_str());
        if (retVal != 0) {
            spdlog::debug("!! system({})", shell_cmd);
            shell_cmd = fmt::format("kill -9 {}", _vtePid);
            retVal = std::system(shell_cmd.c_str());
            if (retVal != 0) {
                spdlog::debug("!! system({})", shell_cmd);
            }
        }
        _vtePid = 0;
    }
    GtkWidget* pTermWidget = vte_terminal_new();
    _pVte = Gtk::manage(Glib::wrap(pTermWidget));
    update_vte_settings();

    //g_autofree gchar* user_shell = vte_get_user_shell(); (#2199) issue when using zsh as default shell
    char* startterm[2] = {0, 0};
    startterm[0] = (char*)_pCtConfig->vteShell.c_str();
    auto pSpawnAsyncData = new CtVteSpawnAsyncData; // will be deleted inside the async callback after being used
    pSpawnAsyncData->pPid = &_vtePid;
    pSpawnAsyncData->pCmd = g_strdup(first_cmd_passed); // must be freed!
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
                             pSpawnAsyncData/*user_data*/);

    auto button_copy = Gtk::manage(new Gtk::Button{});
    button_copy->set_image(*new_managed_image_from_stock("ct_edit_copy", Gtk::ICON_SIZE_MENU));
    button_copy->set_tooltip_text(_("Copy Selection or All"));

    auto button_paste = Gtk::manage(new Gtk::Button{});
    button_paste->set_image(*new_managed_image_from_stock("ct_edit_paste", Gtk::ICON_SIZE_MENU));
    button_paste->set_tooltip_text(_("Paste"));

    auto button_reset = Gtk::manage(new Gtk::Button{});
    button_reset->set_image(*new_managed_image_from_stock("ct_clear", Gtk::ICON_SIZE_MENU));
    button_reset->set_tooltip_text(_("Reset"));

    button_copy->signal_clicked().connect([this](){
        get_ct_actions()->terminal_copy();
    });
    button_paste->signal_clicked().connect([this](){
        get_ct_actions()->terminal_paste();
    });
    button_reset->signal_clicked().connect([this](){
        get_ct_actions()->terminal_reset();
    });

    _pVte->signal_button_press_event().connect([this](GdkEventButton* event){
        if (3 == event->button) {
            get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Terminal)->popup(event->button, event->time);
            return true; // do not propagate the event
        }
        return false; // propagate the event
    }, false);

    auto pButtonsBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 0/*spacing*/});
    pButtonsBox->pack_start(*button_copy, false, false);
    pButtonsBox->pack_start(*button_paste, false, false);
    pButtonsBox->pack_start(*button_reset, false, false);

    _hBoxVte.pack_start(*pButtonsBox, false, false);
    _hBoxVte.pack_start(*_pVte);
    if (GTK_IS_SCROLLABLE(pTermWidget)) {
        GtkWidget* pScrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,
                gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(pTermWidget)));
        gtk_widget_set_can_focus(pScrollbar, FALSE);
        Gtk::Widget* pGtkmmScrollbar = Gtk::manage(Glib::wrap(pScrollbar));
        _hBoxVte.pack_start(*pGtkmmScrollbar, false, false);
    }
    else {
        spdlog::warn("!! GTK_IS_SCROLLABLE(pTermWidget)");
    }
    _hBoxVte.show_all();
#else // !HAVE_VTE
    spdlog::warn("!! noVte {}", first_cmd_passed);
#endif // !HAVE_VTE
}

void CtMainWin::update_vte_settings()
{
#if defined(HAVE_VTE)
    if (not _pVte) {
        return;
    }
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(_pVte->gobj()), -1/*infinite*/);
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(_pVte->gobj()), true);
    vte_terminal_set_word_char_exceptions(VTE_TERMINAL(_pVte->gobj()), VTE_ADDITIONAL_WORDCHARS);
    vte_terminal_set_font(VTE_TERMINAL(_pVte->gobj()),
                          Pango::FontDescription{_pCtConfig->vteFont}.gobj());
#endif // HAVE_VTE
}

void CtMainWin::exec_in_vte(const std::string& shell_cmd)
{
    if (not show_hide_vte_cmd_passed_as_first_in_session(true/*visible*/, shell_cmd.c_str())) {
#if defined(HAVE_VTE)
        vte_terminal_feed_child(VTE_TERMINAL(_pVte->gobj()), shell_cmd.c_str(), shell_cmd.size());
#else // !HAVE_VTE
        spdlog::warn("!! noVte {}", shell_cmd);
#endif // !HAVE_VTE
    }
    else {
    }
}
