#if GTKMM_MAJOR_VERSION >= 4
void CtMainWin::init_app_actions_gtk4()
{
    auto app = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
    if (!app) return;

    // Register application actions and accelerators for all actions
    // that define a shortcut (built-in or custom from settings)
    for (const auto& act : _uCtMenu->get_actions()) {
        if (act.id.empty()) continue;
        
        // Get the effective shortcut (custom from settings or built-in)
        const std::string& shortcut = act.get_shortcut(_pCtConfig);
        if (shortcut.empty()) continue;

        // If the action already exists, just refresh accelerators
        if (app->lookup_action(act.id)) {
            std::vector<Glib::ustring> accels{shortcut};
            app->set_accels_for_action(std::string("app.")+act.id, accels);
            continue;
        }

        auto simple = Gio::SimpleAction::create(act.id);
        simple->signal_activate().connect([this, action_id = act.id](const Glib::VariantBase&){
            if (auto a = _uCtMenu->find_action(action_id)) {
                if (a->run_action) a->run_action();
            }
        });
        app->add_action(simple);

        std::vector<Glib::ustring> accels{shortcut};
        app->set_accels_for_action(std::string("app.")+act.id, accels);
    }
}
#endif /* GTKMM_MAJOR_VERSION >= 4 */
/*
 * ct_main_win.cc
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

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>
#include <sigc++/functors/slot.h>
#if GTKMM_MAJOR_VERSION >= 4
#include <sigc++/signal.h>
#include <sigc++/functors/slot.h>
#endif
#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_storage_control.h"
#include "ct_clipboard.h"
#include "ct_dialogs.h"

void CtStatusBar::new_cursor_pos(const int r, const int c)
{
    if (_r != r or _c != c) {
#if GLIBMM_MAJOR_VERSION > 2 || (GLIBMM_MAJOR_VERSION == 2 && GLIBMM_MINOR_VERSION >= 62)
        cursorPos.set_text(Glib::ustring::sprintf(" (%d,%d)", r, c));
#else /* glibmm < 2.62 */
        cursorPos.set_text(fmt::format(" ({},{})", r, c));
#endif /* glibmm < 2.62 */
        _r = r;
        _c = c;
    }
}

CtMainWin::CtMainWin(bool                            no_gui,
                     CtConfig*                       pCtConfig,
                     CtTmp*                          pCtTmp,
                     Gtk::IconTheme*                 pGtkIconTheme,
                     Glib::RefPtr<Gtk::TextTagTable> rGtkTextTagTable,
                     Glib::RefPtr<Gtk::CssProvider>  rGtkCssProvider,
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
                     CtStatusIcon*                   pCtStatusIcon,
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
                     GtkSourceLanguageManager*       pGtkSourceLanguageManager)
 : Gtk::ApplicationWindow{}
 , _no_gui{no_gui}
 , _pCtConfig{pCtConfig}
 , _pCtTmp{pCtTmp}
 , _pGtkIconTheme{pGtkIconTheme}
 , _rGtkTextTagTable{rGtkTextTagTable}
 , _rGtkCssProvider{rGtkCssProvider}
 , _pGtkSourceLanguageManager{pGtkSourceLanguageManager}
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
 , _pCtStatusIcon{pCtStatusIcon}
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
 , _ctTextview{this}
 , _ctStateMachine{this}
{
#if GTKMM_MAJOR_VERSION >= 4
    // Initialize GTK4 signal callback arrays
    signal_app_new_instance.clear();
    signal_app_show_hide_main_win.clear();
    signal_app_tree_node_copy.clear();
    signal_app_tree_node_paste.clear();
    signal_app_apply_for_each_window.clear();
    signal_app_quit_or_hide_window.clear();
    signal_app_quit_window.clear();
#endif

    get_style_context()->add_class("ct-app-win");
#if GTKMM_MAJOR_VERSION < 4
    set_icon(_pGtkIconTheme->load_icon(CtConst::APP_NAME, 48));
#endif

    _uCtActions.reset(new CtActions{this});
    _uCtMenu.reset(new CtMenu{this});
    _pSaveMenuAction = _uCtMenu->find_action("ct_save");
    _uCtPrint.reset(new CtPrint{this});
    _uCtStorage.reset(CtStorageControl::create_dummy_storage(this));

    _scrolledwindowTree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowTree.get_style_context()->add_class("ct-tree-scroll-panel");
    _scrolledwindowText.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
#if GTKMM_MAJOR_VERSION >= 4
    _scrolledwindowText.set_child(_ctTextview.mm());
    _vboxText.append(_init_window_header());
    _vboxText.append(_scrolledwindowText);
    if (_pCtConfig->treeRightSide) {
        _hPaned.set_start_child(_vboxText);
        _hPaned.set_end_child(_scrolledwindowTree);
    }
    else {
        _hPaned.set_start_child(_scrolledwindowTree);
        _hPaned.set_end_child(_vboxText);
    }
    _vPaned.set_start_child(_hPaned);
#else
    _scrolledwindowText.add(_ctTextview.mm());
    _vboxText.pack_start(_init_window_header(), false, false);
    _vboxText.pack_start(_scrolledwindowText);
    if (_pCtConfig->treeRightSide) {
        _hPaned.pack1(_vboxText, Gtk::EXPAND);
        _hPaned.pack2(_scrolledwindowTree, Gtk::FILL);
    }
    else {
        _hPaned.pack1(_scrolledwindowTree, Gtk::FILL);
        _hPaned.pack2(_vboxText, Gtk::EXPAND);
    }
    _vPaned.pack1(_hPaned, Gtk::EXPAND);
#endif
    _hPaned.property_wide_handle() = true;
#if GTKMM_MAJOR_VERSION >= 4
    init_app_actions_gtk4();
#endif

    #if GTKMM_MAJOR_VERSION < 4
    _vPaned.pack2(_hBoxVte, Gtk::FILL);
    #else
    _vPaned.set_end_child(_hBoxVte);
    #endif
    _vPaned.property_wide_handle() = true;

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _pMenuBar = _uCtMenu->build_menubar();
    _pScrolledWindowMenuBar = Gtk::manage(new Gtk::ScrolledWindow{});
    _pScrolledWindowMenuBar->add(*_pMenuBar);
    _pMenuBar->set_name("MenuBar");
    _pBookmarksSubmenus[0] = CtMenu::find_menu_item(_pMenuBar, "BookmarksSubMenu");
    auto pPopumMenuTree = _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node);
    for (Gtk::Widget* child : pPopumMenuTree->get_children()) {
        if (auto menuItem = dynamic_cast<Gtk::MenuItem*>(child)) {
            if (menuItem->has_submenu()) {
                _pBookmarksSubmenus[1] = menuItem; // TODO FIND BETTER WAY TO IDENTIFY THAN FIRST WITH SUBMENU
                break;
            }
        }
    }
    _pBookmarksSubmenus[2] = CtMenu::find_menu_item(_pMenuBar, "BookmarksMenu");
    _pRecentDocsSubmenu = CtMenu::find_menu_item(_pMenuBar, "RecentDocsSubMenu");
    _pMenuBar->show_all();
    add_accel_group(_uCtMenu->get_accel_group());
    _pToolbars = _uCtMenu->build_toolbars(_pRecentDocsMenuToolButton, _pSaveToolButton);
#else
    // GTK4: use hierarchical Gio::MenuModel menubutton
    _pMenuButton4 = _uCtMenu->build_menubutton_model4();
#endif

        if (_pCtConfig->menubarInTitlebar) {
        _pHeaderBar = Gtk::manage(new Gtk::HeaderBar{});
        // GTK3-only APIs guarded
    #if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
        _pHeaderBar->set_has_subtitle(false);
        _pHeaderBar->set_show_close_button(true);
    #endif
        _pHeaderBar->pack_start(*Gtk::manage(new Gtk::Label{" "}));
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
        _pHeaderBar->pack_start(*_pScrolledWindowMenuBar);
#else
    _pHeaderBar->pack_start(*_pMenuButton4);
#endif
        _pHeaderBar->pack_start(*Gtk::manage(new Gtk::Label{" "}));
        // Ensure full visibility of header bar contents on GTK3
        #if GTKMM_MAJOR_VERSION < 4
        _pHeaderBar->show_all();
        #else
        _pHeaderBar->show();
        #endif
        set_titlebar(*_pHeaderBar);
    }
    else {
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
        _vboxMain.pack_start(*_pScrolledWindowMenuBar, false, false);
#else
#if GTKMM_MAJOR_VERSION >= 4
    _vboxMain.append(*_pMenuButton4);
#else
    _vboxMain.pack_start(*_pMenuButton4, false, false);
#endif
#endif
    }
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    for (auto pToolbar : _pToolbars) {
        _vboxMain.pack_start(*pToolbar, false, false);
    }
#else
    Gtk::MenuButton* recentBtn{}; Gtk::Button* saveBtn{};
    _gtk4Toolbars = _uCtMenu->build_toolbars4(recentBtn, saveBtn);
    _pRecentDocsMenuButton4 = recentBtn;
    for (auto pBox : _gtk4Toolbars) {
        _vboxMain.append(*pBox);
    }
    // Initial population of recent docs popover
    _uCtMenu->populate_recent_docs_menu4(_pRecentDocsMenuButton4, _pCtConfig->recentDocsFilepaths);
    // Create empty bookmarks button in primary toolbar (populated later)
    if (!_gtk4Toolbars.empty()) {
        std::list<std::tuple<gint64, Glib::ustring, const char*>> emptyBookmarks;
        auto bookmark_action_lambda = [this](gint64 node_id){
            CtTreeIter tree_iter = _uCtTreestore->get_node_from_node_id(node_id);
            if (tree_iter) _uCtTreeview->set_cursor_safe(tree_iter);
        };
        // GTK4: use std::function callback directly
        std::function<void(gint64)> bookmark_action = bookmark_action_lambda;
        _pBookmarksButton4 = _uCtMenu->build_bookmarks_button4(emptyBookmarks, bookmark_action, true);
        _gtk4Toolbars[0]->append(*_pBookmarksButton4);
    }
#endif
    // Main layout assembly
#if GTKMM_MAJOR_VERSION >= 4
    _vboxMain.append(_vPaned);
    _vboxMain.append(_init_status_bar());
    _vboxMain.show();
    set_child(_vboxMain);
#else
    _vboxMain.pack_start(_vPaned);
    _vboxMain.pack_start(_init_status_bar(), false, false);
    _vboxMain.show_all();
    add(_vboxMain);
#endif

    _reset_CtTreestore_CtTreeview();

    auto& textView = _ctTextview.mm();
    textView.get_style_context()->add_class("ct-view-panel");
    textView.set_sensitive(false);

    // GTK3 signal connections (popup/event/motion) not available in GTK4
#if GTKMM_MAJOR_VERSION < 4
    textView.signal_populate_popup().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_populate_popup));
    textView.signal_motion_notify_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_motion_notify_event));
#endif
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    textView.signal_visibility_notify_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_visibility_notify_event));
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
    #if GTKMM_MAJOR_VERSION < 4
    textView.signal_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_event));
    textView.signal_event_after().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_event_after));
    textView.signal_scroll_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_scroll_event));
    #endif

    _uCtPairCodeboxMainWin.reset(new CtPairCodeboxMainWin{nullptr, this});
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "cut-clipboard", G_CALLBACK(CtClipboard::on_cut_clipboard), _uCtPairCodeboxMainWin.get());
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "copy-clipboard", G_CALLBACK(CtClipboard::on_copy_clipboard), _uCtPairCodeboxMainWin.get());
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "paste-clipboard", G_CALLBACK(CtClipboard::on_paste_clipboard), _uCtPairCodeboxMainWin.get());

#if GTKMM_MAJOR_VERSION < 4
    signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_window_key_press_event), false);
    signal_show().connect([this](){
        auto rGdkWin = this->get_window();
        if (rGdkWin) {
            rGdkWin->set_events(rGdkWin->get_events() | Gdk::STRUCTURE_MASK);
        }
    });
#endif

    file_autosave_restart();
    mod_time_sentinel_restart();

    window_title_update(false/*saveNeeded*/);

    config_apply();

    menu_set_items_recent_documents();
    #if GTKMM_MAJOR_VERSION >= 4
    if (auto a = _uCtMenu->find_action("ct_vacuum")) {
        if (a->signal_set_visible) {
            a->signal_set_visible(false);
        }
    }
    #else
    if (auto a = _uCtMenu->find_action("ct_vacuum")) {
        if (a->signal_set_visible) {
            a->signal_set_visible->emit(false);
        }
    }
    #endif
    menu_update_doc_path_menu_item();
    menu_top_optional_bookmarks_enforce();

    if (_no_gui) {
        set_visible(false);
    }
    else {
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
        if (start_on_systray_is_active()) {
            /* Calling the 'present()' function apparently sets up selected
               node visibility within the TreeView panel, whereas this
               node setup is skipped when only calling 'set_visible(false)'.
               Calling 'present()' and then hiding the window with
               'set_visible(false)' works to place the selected node within
               the visible portion of the TreeView panel but does not seem
               like the correct way to achieve this. Is there a function
               that specifically sets the selected node to the top row of
               the visible area in the panel?
            */
            present();
            set_visible(false);
        }
        else
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
        {
            present();
        }
        Glib::signal_idle().connect_once([this](){
            _hPaned.property_position() = _pCtConfig->hpanedPos; // must be after present() + process events pending (#1534, #1918, #2126)
            _vPaned.property_position() = _pCtConfig->vpanedPos;
            #if GTKMM_MAJOR_VERSION < 4
            _ctTextview.mm().signal_size_allocate().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_size_allocate));
            signal_configure_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_window_configure_event), false);
            #endif

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
            // show status icon if needed
            if (_pCtConfig->systrayOn) {
                get_status_icon()->set_visible(true);
            }
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
        });
    }

    dispatcherErrorMsg.connect(sigc::mem_fun(*this, &CtMainWin::_on_dispatcher_error_msg));
}

// --- Signal emit/connect wrappers ------------------------------
void CtMainWin::emit_app_new_instance()
{
    #if GTKMM_MAJOR_VERSION >= 4
    for (auto& f : signal_app_new_instance) f();
    #else
    signal_app_new_instance.emit();
    #endif
}

void CtMainWin::emit_app_show_hide_main_win()
{
    #if GTKMM_MAJOR_VERSION >= 4
    for (auto& f : signal_app_show_hide_main_win) f();
    #else
    signal_app_show_hide_main_win.emit();
    #endif
}

void CtMainWin::emit_app_tree_node_copy()
{
    #if GTKMM_MAJOR_VERSION >= 4
    for (auto& f : signal_app_tree_node_copy) f();
    #else
    signal_app_tree_node_copy.emit();
    #endif
}

void CtMainWin::emit_app_tree_node_paste()
{
    #if GTKMM_MAJOR_VERSION >= 4
    for (auto& f : signal_app_tree_node_paste) f();
    #else
    signal_app_tree_node_paste.emit();
    #endif
}

void CtMainWin::connect_app_tree_node_copy(const std::function<void()>& cb)
{
    #if GTKMM_MAJOR_VERSION >= 4
    signal_app_tree_node_copy.push_back(cb);
    #else
    signal_app_tree_node_copy.connect(sigc::slot<void>(cb));
    #endif
}

void CtMainWin::connect_app_tree_node_paste(const std::function<void()>& cb)
{
    #if GTKMM_MAJOR_VERSION >= 4
    signal_app_tree_node_paste.push_back(cb);
    #else
    signal_app_tree_node_paste.connect(sigc::slot<void>(cb));
    #endif
}

void CtMainWin::emit_app_apply_for_each_window(const std::function<void(CtMainWin*)>& cb)
{
    #if GTKMM_MAJOR_VERSION >= 4
    for (auto& f : signal_app_apply_for_each_window) f(cb);
    #else
    signal_app_apply_for_each_window.emit(cb);
    #endif
}

void CtMainWin::emit_app_quit_or_hide_window(CtMainWin* pWin)
{
    #if GTKMM_MAJOR_VERSION >= 4
    for (auto& f : signal_app_quit_or_hide_window) f();
    #else
    signal_app_quit_or_hide_window.emit(pWin);
    #endif
}

void CtMainWin::emit_app_quit_window(CtMainWin* pWin)
{
    #if GTKMM_MAJOR_VERSION >= 4
    for (auto& f : signal_app_quit_window) f();
    #else
    signal_app_quit_window.emit(pWin);
    #endif
}

void CtMainWin::connect_app_new_instance(const std::function<void()>& cb)
{
    #if GTKMM_MAJOR_VERSION >= 4
    signal_app_new_instance.push_back(cb);
    #else
    signal_app_new_instance.connect(sigc::slot<void>(cb));
    #endif
}

void CtMainWin::connect_app_apply_for_each_window(const std::function<void(std::function<void(CtMainWin*)>)>& cb)
{
    #if GTKMM_MAJOR_VERSION >= 4
    signal_app_apply_for_each_window.push_back(cb);
    #else
    signal_app_apply_for_each_window.connect(sigc::slot<void, std::function<void(CtMainWin*)>>(cb));
    #endif
}

void CtMainWin::connect_app_quit_or_hide_window(const std::function<void(CtMainWin*)>& cb)
{
    #if GTKMM_MAJOR_VERSION >= 4
    signal_app_quit_or_hide_window.push_back([this, cb](){ cb(this); });
    #else
    signal_app_quit_or_hide_window.connect(sigc::slot<void, CtMainWin*>(cb));
    #endif
}

void CtMainWin::connect_app_quit_window(const std::function<void(CtMainWin*)>& cb)
{
    #if GTKMM_MAJOR_VERSION >= 4
    signal_app_quit_window.push_back([this, cb](){ cb(this); });
    #else
    signal_app_quit_window.connect(sigc::slot<void, CtMainWin*>(cb));
    #endif
}


CtMainWin::~CtMainWin()
{
    _autosave_timout_connection.disconnect();
    _mod_time_sentinel_timout_connection.disconnect();
    //std::cout << "~CtMainWin" << std::endl;
}

void CtMainWin::_on_dispatcher_error_msg()
{
    const std::string eroor_msg = errorsDEQueue.pop_front();
    CtDialogs::error_dialog(eroor_msg, *this);
}

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
bool CtMainWin::start_on_systray_is_active() const
{
    return _pCtConfig->systrayOn and _pCtConfig->startOnSystray;
}
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
void CtMainWin::start_on_systray_delayed_file_open_set(const std::string& filepath, const std::string& nodename, const std::string& anchorname)
{
    _startOnSystray_delayedFilepath = filepath;
    _startOnSystray_delayedNodeName = nodename;
    _startOnSystray_delayedAnchorName = anchorname;
}
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
bool CtMainWin::start_on_systray_delayed_file_open_kick()
{
    if (not _startOnSystray_delayedFilepath.empty()) {
        const std::string startOnSystray_delayedFilepath = _startOnSystray_delayedFilepath;
        const std::string startOnSystray_delayedNodeName = _startOnSystray_delayedNodeName;
        const std::string startOnSystray_delayedAnchorName = _startOnSystray_delayedAnchorName;
        _startOnSystray_delayedFilepath.clear();
        _startOnSystray_delayedNodeName.clear();
        _startOnSystray_delayedAnchorName.clear();
        if (file_open(startOnSystray_delayedFilepath, startOnSystray_delayedNodeName, startOnSystray_delayedAnchorName)) {
            return true;
        }
        spdlog::warn("%s Couldn't open file: %s", __FUNCTION__, _startOnSystray_delayedFilepath);
    }
    return false;
}
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */

const char* CtMainWin::get_code_icon_name(std::string code_type)
{
    for (const auto& iconPair : CtConst::NODE_CODE_ICONS) {
        if (0 == strcmp(iconPair.first, code_type.c_str())) {
            return iconPair.second;
        }
    }
    return CtStockIcon::at(CtConst::NODE_ICON_CODE_ID);
}

Gtk::Image* CtMainWin::new_managed_image_from_stock(const std::string& stockImage, Gtk::BuiltinIconSize size)
{
    auto pImage = Gtk::manage(new Gtk::Image{});
    #if GTKMM_MAJOR_VERSION >= 4
    pImage->set_from_icon_name(stockImage);
    #else
    pImage->set_from_icon_name(stockImage, size);
    #endif
    return pImage;
}

void CtMainWin::_reset_CtTreestore_CtTreeview()
{
    _prevTreeIter = CtTreeIter{};
    _nodesCursorPos.clear();
    _nodesVScrollPos.clear();
    _treeExpandedNodeIds.clear();
    _treeRestoreInProgress = false;

    #if GTKMM_MAJOR_VERSION >= 4
    _uCtTreeview.reset(new CtTreeView{_pCtConfig});
    _scrolledwindowTree.set_child(*_uCtTreeview);
    _uCtTreeview->show();
    #else
    _scrolledwindowTree.remove();
    _uCtTreeview.reset(new CtTreeView{_pCtConfig});
    _scrolledwindowTree.add(*_uCtTreeview);
    _uCtTreeview->show();
    #endif

    _uCtTreestore.reset(new CtTreeStore{this});
    _uCtTreestore->tree_view_connect(_uCtTreeview.get());
    _uCtTreeview->set_tree_node_name_wrap_width(_pCtConfig->cherryWrapEnabled, _pCtConfig->cherryWrapWidth);
    _uCtTreeview->get_column(CtTreeView::AUX_ICON_COL_NUM)->set_visible(!_pCtConfig->auxIconHide);
    show_hide_tree_lines(_pCtConfig->treeLinesVisible);

    _tree_just_auto_expanded = false;
    _uCtTreeview->signal_cursor_changed().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_cursor_changed));
    #if GTKMM_MAJOR_VERSION < 4
    _uCtTreeview->signal_button_release_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_button_release_event));
    _uCtTreeview->signal_event_after().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_event_after));
    _uCtTreeview->signal_row_activated().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_row_activated));
    _uCtTreeview->signal_row_expanded().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_row_expanded));
    _uCtTreeview->signal_row_collapsed().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_row_collapsed));
    _uCtTreeview->signal_test_collapse_row().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_test_collapse_row));
    _uCtTreeview->signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_key_press_event), false);
    _uCtTreeview->signal_scroll_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_scroll_event));
    _uCtTreeview->signal_popup_menu().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_popup_menu));
    #endif

    #if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _uCtTreeview->drag_source_set(std::vector<Gtk::TargetEntry>{Gtk::TargetEntry{"CT_DND", Gtk::TARGET_SAME_WIDGET, 0}},
                                  Gdk::BUTTON1_MASK,
                                  Gdk::ACTION_MOVE);
    _uCtTreeview->drag_dest_set(std::vector<Gtk::TargetEntry>{Gtk::TargetEntry{"CT_DND", Gtk::TARGET_SAME_WIDGET, 0}},
                                Gtk::DEST_DEFAULT_ALL,
                                Gdk::ACTION_MOVE);
    _uCtTreeview->signal_drag_motion().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_drag_motion));
    _uCtTreeview->signal_drag_data_received().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_drag_data_received));
    _uCtTreeview->signal_drag_data_get().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_drag_data_get));
    #else
    _setup_treeview_drag_and_drop_gtk4();
    #endif

    _uCtTreeview->get_style_context()->add_class("ct-tree-panel");
    _uCtTreeview->set_margin_bottom(10);  // so horiz scroll doens't prevent to select the bottom element
}

void CtMainWin::config_apply()
{
    #if GTKMM_MAJOR_VERSION < 4
    move(_pCtConfig->winRect[0], _pCtConfig->winRect[1]);
    #endif
    set_default_size(_pCtConfig->winRect[2], _pCtConfig->winRect[3]);
    if (_pCtConfig->winIsMaximised) {
        maximize();
    }

    show_hide_tree_view(_pCtConfig->treeVisible);
    (void)show_hide_vte_cmd_passed_as_first_in_session(_pCtConfig->vteVisible, nullptr/*first_cmd_passed*/);
    show_hide_menubar(_pCtConfig->menubarVisible);
    show_hide_win_header(_pCtConfig->showNodeNameHeader);
    _ctWinHeader.lockIcon.hide();
    _ctWinHeader.bookmarkIcon.hide();
    _ctWinHeader.ghostIcon.hide();

    menu_rebuild_toolbars(false);
    show_hide_statusbar(_pCtConfig->statusbarVisible);

    _ctStatusBar.progressBar.hide();
    _ctStatusBar.stopButton.hide();

    menu_set_visible_exit_app(_pCtConfig->systrayOn);

    auto pGtkSourceView = GTK_SOURCE_VIEW(_ctTextview.gobj());
    auto& textView = _ctTextview.mm();
    gtk_source_view_set_show_line_numbers(pGtkSourceView, _pCtConfig->showLineNumbers);
    gtk_source_view_set_insert_spaces_instead_of_tabs(pGtkSourceView, _pCtConfig->spacesInsteadTabs);
    gtk_source_view_set_tab_width(pGtkSourceView, _pCtConfig->tabsWidth);
    textView.set_indent(_pCtConfig->wrappingIndent);
    textView.set_pixels_above_lines(_pCtConfig->spaceAroundLines);
    textView.set_pixels_below_lines(_pCtConfig->spaceAroundLines);
    _ctTextview.set_pixels_inside_wrap(_pCtConfig->spaceAroundLines, _pCtConfig->relativeWrappedSpace);
    #if GTKMM_MAJOR_VERSION >= 4
    textView.set_wrap_mode(_pCtConfig->lineWrapping ? Gtk::WrapMode::WORD_CHAR : Gtk::WrapMode::NONE);
    #else
    textView.set_wrap_mode(_pCtConfig->lineWrapping ? Gtk::WrapMode::WRAP_WORD_CHAR : Gtk::WrapMode::WRAP_NONE);
    #endif

    Glib::RefPtr<Gtk::Settings> pSettings = Gtk::Settings::get_default();
    if (2 != _pCtConfig->cursorBlink) {
        pSettings->property_gtk_cursor_blink() = _pCtConfig->cursorBlink;
    }
    pSettings->property_gtk_cursor_blink_timeout() = INT_MAX; /*otherwise the cursor stops blinking after 10 sec of inactivity*/
    if (2 != _pCtConfig->overlayScroll) {
//        Gtk::Settings::get_default()->property_gtk_overlay_scrolling() = _pCtConfig->overlayScroll;
        _scrolledwindowText.set_overlay_scrolling(static_cast<bool>(_pCtConfig->overlayScroll));
    }
    update_theme();
#if GTKMM_MAJOR_VERSION >= 4
    // Refresh displayed shortcut labels (in case of changes)
    _uCtMenu->refresh_shortcuts_gtk4();
    // Repopulate recent docs and bookmarks after config apply
    _uCtMenu->populate_recent_docs_menu4(_pRecentDocsMenuButton4, _pCtConfig->recentDocsFilepaths);
    menu_set_bookmark_menu_items();
#endif
}

void CtMainWin::config_update_data_from_curr_status()
{
    _ensure_curr_doc_in_recent_docs();
    _ctTextview.synch_spell_check_change_from_gspell_right_click_menu();
}

void CtMainWin::update_theme()
{
    auto font_to_string = [](const Pango::FontDescription& font, const Glib::ustring&
#if defined(_WIN32)
                                                                                      fallbackFont
#endif /* _WIN32 */
                            )->std::string{
        // adds fallback font on Win32; on Linux, fonts work ok without explicit fallback
        // https://docs.gtk.org/gtk3/css-properties.html#font-properties
        // NOTE: {{ and }} are fmt::format escapes for { and }
        std::string retStr = fmt::format(" {{ font-family: \"{}\"", Glib::locale_from_utf8(font.get_family()));
#if defined(_WIN32)
        retStr += fmt::format(",\"{}\"", fallbackFont.raw());
#endif /* _WIN32 */
        const Pango::Style style_enum = font.get_style();
        const char* style_str = "normal";
        switch (style_enum) {
        #if GTKMM_MAJOR_VERSION >= 4
            case Pango::Style::OBLIQUE: style_str = "oblique"; break;
            case Pango::Style::ITALIC: style_str = "italic"; break;
        #else
            case Pango::Style::STYLE_OBLIQUE: style_str = "oblique"; break;
            case Pango::Style::STYLE_ITALIC: style_str = "italic"; break;
        #endif
            default: break;
        }
        const Pango::Stretch stretch_enum = font.get_stretch();
        const char* stretch_str = "normal";
        switch (stretch_enum) {
        #if GTKMM_MAJOR_VERSION >= 4
            case Pango::Stretch::ULTRA_CONDENSED: stretch_str = "ultra-condensed"; break;
            case Pango::Stretch::EXTRA_CONDENSED: stretch_str = "extra-condensed"; break;
            case Pango::Stretch::CONDENSED: stretch_str = "condensed"; break;
            case Pango::Stretch::SEMI_CONDENSED: stretch_str = "semi-condensed"; break;
            case Pango::Stretch::SEMI_EXPANDED: stretch_str = "semi-expanded"; break;
            case Pango::Stretch::EXPANDED: stretch_str = "expanded"; break;
            case Pango::Stretch::EXTRA_EXPANDED: stretch_str = "extra-expanded"; break;
            case Pango::Stretch::ULTRA_EXPANDED: stretch_str = "ultra-expanded"; break;
        #else
            case Pango::Stretch::STRETCH_ULTRA_CONDENSED: stretch_str = "ultra-condensed"; break;
            case Pango::Stretch::STRETCH_EXTRA_CONDENSED: stretch_str = "extra-condensed"; break;
            case Pango::Stretch::STRETCH_CONDENSED: stretch_str = "condensed"; break;
            case Pango::Stretch::STRETCH_SEMI_CONDENSED: stretch_str = "semi-condensed"; break;
            case Pango::Stretch::STRETCH_SEMI_EXPANDED: stretch_str = "semi-expanded"; break;
            case Pango::Stretch::STRETCH_EXPANDED: stretch_str = "expanded"; break;
            case Pango::Stretch::STRETCH_EXTRA_EXPANDED: stretch_str = "extra-expanded"; break;
            case Pango::Stretch::STRETCH_ULTRA_EXPANDED: stretch_str = "ultra-expanded"; break;
        #endif
            default: break;
        }
        const Pango::Variant variant_enum = font.get_variant();
        #if GTKMM_MAJOR_VERSION >= 4
        const char* variant_str = Pango::Variant::SMALL_CAPS == variant_enum ? "small-caps" : "normal";
        #else
        const char* variant_str = Pango::Variant::VARIANT_SMALL_CAPS == variant_enum ? "small-caps" : "normal";
        #endif
        retStr += fmt::format("; font-size: {}pt; font-weight: {}; font-style: {}; font-stretch: {}; font-variant: {}; }} ",
            std::to_string(font.get_size()/Pango::SCALE), std::to_string(font.get_weight()), style_str, stretch_str, variant_str);
        return retStr;
    };

    std::string rtFont = font_to_string(Pango::FontDescription(_pCtConfig->rtFont), _pCtConfig->fallbackFontFamily);
    std::string plFont = font_to_string(Pango::FontDescription(_pCtConfig->ptFont), _pCtConfig->fallbackFontFamily);
    std::string codeFont = font_to_string(Pango::FontDescription(_pCtConfig->codeFont), "monospace");
    std::string treeFont = font_to_string(Pango::FontDescription(_pCtConfig->treeFont), _pCtConfig->fallbackFontFamily);

    std::string css_str;
    css_str.reserve(1100);
    css_str += ".ct-view-panel.ct-view-rich-text" + rtFont;
    css_str += ".ct-view-panel.ct-view-plain-text" + plFont;
    css_str += ".ct-view-panel.ct-view-code" + codeFont;
    if (get_ct_config()->scrollBeyondLastLine) {
        css_str += ".ct-view-panel { padding-bottom: 400px } ";
    }
    css_str += ".ct-cboxtoolbar { background-color: transparent; } ";
    css_str += ".ct-codebox.ct-view-plain-text" + plFont;
    css_str += ".ct-codebox.ct-view-code" + codeFont;
    css_str += ".ct-tree-panel" + treeFont;
    css_str += ".ct-table-light" + rtFont;
    css_str += " ";
    css_str += ".ct-tree-panel { color: " + _pCtConfig->ttDefFg + "; background-color: " + _pCtConfig->ttDefBg + "; } ";
    css_str += ".ct-tree-panel:selected { color: " + _pCtConfig->ttSelFg + "; background: " + _pCtConfig->ttSelBg + "; } ";
    css_str += ".ct-tree-scroll-panel { background-color: " + _pCtConfig->ttDefBg + "; } ";
    css_str += ".ct-header-panel { background-color: " + _pCtConfig->ttDefBg + "; } ";
    css_str += ".ct-header-panel button { margin: 2px; padding: 0 4px 0 4px; } ";
    css_str += ".ct-status-bar bar { margin: 0px; } ";
#if defined(_WIN32)
    // without this the progress bar height is 1 or 2 px, hardly visible (#2373)
    css_str += ".ct-status-bar progressbar trough, progressbar progress { min-height: 16px; } ";
#endif // _WIN32
    if (_pCtConfig->scrollSliderMin) {
        // scrollbar too thin (#2427)
        css_str += "scrollbar slider { " + fmt::format("min-width: {0}px; min-height: {0}px;", _pCtConfig->scrollSliderMin) + " } ";
    }
    css_str += ".ct-table-header-cell { font-weight: bold; } ";
    css_str += ".ct-table grid { background: #cccccc; border-style:solid; border-width: 1px; border-color: gray; } ";
    css_str += "toolbar { padding: 2px 2px 2px 2px; } ";
    css_str += "toolbar button { padding: 0px; } ";
    css_str += "textview border { background-color: transparent; } "; // Loss of transparency with PNGs (#1402, #2132)
    //printf("css_str_len=%zu\n", css_str.size());

    if (_css_provider_theme) {
        #if GTKMM_MAJOR_VERSION >= 4
        Gtk::StyleContext::remove_provider_for_display(Gdk::Display::get_default(), _css_provider_theme);
        #else
        Gtk::StyleContext::remove_provider_for_screen(get_screen(), _css_provider_theme);
        #endif
    }
    _css_provider_theme = Gtk::CssProvider::create();
    #if GTKMM_MAJOR_VERSION >= 4
    gtk_css_provider_load_from_data(_css_provider_theme->gobj_copy(), css_str.c_str(), css_str.size());
    #else
    gtk_css_provider_load_from_data(_css_provider_theme->gobj_copy(), css_str.c_str(), css_str.size(), nullptr);
    #endif
    // _css_provider_theme->load_from_data(theme_css); second call of load_from_data erases css from the first call on mac
    #if GTKMM_MAJOR_VERSION >= 4
    Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), _css_provider_theme, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    #else
    get_style_context()->add_provider_for_screen(get_screen(), _css_provider_theme, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    #endif
}

Gtk::Box& CtMainWin::_init_status_bar()
{
    _ctStatusBar.statusId = _ctStatusBar.statusBar.get_context_id("");
    #if GTKMM_MAJOR_VERSION >= 4
    _ctStatusBar.frame.set_child(_ctStatusBar.progressBar);
    _ctStatusBar.stopButton.set_image_from_icon_name("ct_stop", Gtk::IconSize::INHERIT);
    _ctStatusBar.hbox.append(_ctStatusBar.cursorPos);
    _ctStatusBar.hbox.append(_ctStatusBar.statusBar);
    _ctStatusBar.hbox.append(_ctStatusBar.frame);
    _ctStatusBar.hbox.append(_ctStatusBar.stopButton);
    #else
    _ctStatusBar.frame.set_shadow_type(Gtk::SHADOW_NONE);
    _ctStatusBar.frame.add(_ctStatusBar.progressBar);
    _ctStatusBar.stopButton.set_image_from_icon_name("ct_stop", Gtk::ICON_SIZE_MENU);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.cursorPos, false, false);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.statusBar, true, true);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.frame, false, true);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.stopButton, false, true);
    #endif

    _ctStatusBar.hbox.get_style_context()->add_class("ct-status-bar");
    // todo: move to css
    #if GTKMM_MAJOR_VERSION < 4
    _ctStatusBar.frame.set_border_width(1);
    _ctStatusBar.statusBar.set_margin_top(0);
    _ctStatusBar.statusBar.set_margin_bottom(0);
    ((Gtk::Frame*)_ctStatusBar.statusBar.get_children()[0])->get_child()->set_margin_top(1);
    ((Gtk::Frame*)_ctStatusBar.statusBar.get_children()[0])->get_child()->set_margin_bottom(1);
    _ctStatusBar.hbox.set_border_width(0);
    #endif

    _ctStatusBar.stopButton.signal_clicked().connect([this](){
        _ctStatusBar.set_progress_stop(true);
        _ctStatusBar.stopButton.hide();
    });
    _ctStatusBar.set_progress_stop(false);
    return _ctStatusBar.hbox;
}

Gtk::EventBox& CtMainWin::_init_window_header()
{
    _ctWinHeader.nameLabel.set_margin_start(10);
    _ctWinHeader.nameLabel.set_margin_end(10);
    #if GTKMM_MAJOR_VERSION >= 4
    _ctWinHeader.nameLabel.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
    #else
    _ctWinHeader.nameLabel.set_ellipsize(Pango::EllipsizeMode::ELLIPSIZE_MIDDLE);
    #endif
#if GTKMM_MAJOR_VERSION >= 4
    _ctWinHeader.lockIcon.set_from_icon_name("ct_locked");
#else
    _ctWinHeader.lockIcon.set_from_icon_name("ct_locked", Gtk::ICON_SIZE_MENU);
#endif
    _ctWinHeader.lockIcon.hide();
#if GTKMM_MAJOR_VERSION >= 4
    _ctWinHeader.bookmarkIcon.set_from_icon_name("ct_pin");
#else
    _ctWinHeader.bookmarkIcon.set_from_icon_name("ct_pin", Gtk::ICON_SIZE_MENU);
#endif
    _ctWinHeader.bookmarkIcon.hide();
#if GTKMM_MAJOR_VERSION >= 4
    _ctWinHeader.ghostIcon.set_from_icon_name("ct_ghost");
#else
    _ctWinHeader.ghostIcon.set_from_icon_name("ct_ghost", Gtk::ICON_SIZE_MENU);
#endif
    _ctWinHeader.ghostIcon.hide();
#if GTKMM_MAJOR_VERSION >= 4
    _ctWinHeader.headerBox.append(_ctWinHeader.buttonBox);
    _ctWinHeader.headerBox.append(_ctWinHeader.nameLabel);
    _ctWinHeader.nameLabel.set_hexpand(true);
    _ctWinHeader.headerBox.append(_ctWinHeader.lockIcon);
    _ctWinHeader.headerBox.append(_ctWinHeader.bookmarkIcon);
    _ctWinHeader.headerBox.append(_ctWinHeader.ghostIcon);
    _ctWinHeader.eventBox.append(_ctWinHeader.headerBox);
#else
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.buttonBox, false, false);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.nameLabel, true, true);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.lockIcon, false, false);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.bookmarkIcon, false, false);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.ghostIcon, false, false);
    _ctWinHeader.eventBox.add(_ctWinHeader.headerBox);
#endif
    _ctWinHeader.eventBox.get_style_context()->add_class("ct-header-panel");
    return _ctWinHeader.eventBox;
}

void CtMainWin::window_header_update()
{
    if (_no_gui) return;

    // based on update_node_name_header
    std::string name = _pCtConfig->nodeNameHeaderShowFullPath ?
        CtMiscUtil::get_node_hierarchical_name(curr_tree_iter(), " / ", false).c_str() : curr_tree_iter().get_node_name().c_str();
    std::string foreground = curr_tree_iter().get_node_foreground();
    foreground = foreground.empty() ? _pCtConfig->ttDefFg : foreground;
    _ctWinHeader.nameLabel.set_markup(
        "<span foreground=\"" + foreground + "\" font_desc=\"" + _pCtConfig->treeFont + "\" font_weight=\"bold\">" + str::xml_escape(name) + "</span>");

    // update last visited buttons
    if (0 == _pCtConfig->nodesOnNodeNameHeader) {
#if GTKMM_MAJOR_VERSION >= 4
        while (auto pChild = _ctWinHeader.buttonBox.get_first_child()) {
            _ctWinHeader.buttonBox.remove(*pChild);
        }
#else
        for (auto pButton : _ctWinHeader.buttonBox.get_children()) {
            _ctWinHeader.buttonBox.remove(*pButton);
        }
#endif
    }
    else {
        // add more buttons if that is needed
#if GTKMM_MAJOR_VERSION >= 4
        int num_children = 0;
        for (auto pChild = _ctWinHeader.buttonBox.get_first_child(); pChild; pChild = pChild->get_next_sibling()) {
            ++num_children;
        }
        while (num_children < _pCtConfig->nodesOnNodeNameHeader) {
#else
        while ((int)_ctWinHeader.buttonBox.get_children().size() < _pCtConfig->nodesOnNodeNameHeader) {
#endif
            auto pButton = Gtk::manage(new Gtk::Button);
            auto pHBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 1});
            auto pImage = Gtk::manage(new Gtk::Image);
            auto pLabel = Gtk::manage(new Gtk::Label);
#if GTKMM_MAJOR_VERSION >= 4
            pHBox->append(*pImage);
            pHBox->append(*pLabel);
            pButton->set_child(*pHBox);
#else
            pHBox->pack_start(*pImage);
            pHBox->pack_start(*pLabel);
            pButton->add(*pHBox);
#endif
            auto f_on_click = [this](Gtk::Button* pButton) {
                auto node_id = _ctWinHeader.button_to_node_id.find(pButton);
                if (node_id != _ctWinHeader.button_to_node_id.end()) {
                    if (CtTreeIter tree_iter = get_tree_store().get_node_from_node_id(node_id->second)) {
                        _uCtTreeview->set_cursor_safe(tree_iter);
                    }
                    _ctTextview.mm().grab_focus();
                }
            };
            pButton->signal_clicked().connect(sigc::bind(f_on_click, pButton));
#if GTKMM_MAJOR_VERSION >= 4
            _ctWinHeader.buttonBox.append(*pButton);
            ++num_children;
#else
            _ctWinHeader.buttonBox.add(*pButton);
#endif
        }

        // update button labels and node_ids
        CtTreeStore& ctTreestore = get_tree_store();
        const gint64 curr_node_id = curr_tree_iter().get_node_id();
        int button_idx{0};
#if GTKMM_MAJOR_VERSION >= 4
        std::vector<Gtk::Widget*> buttons;
        for (auto pChild = _ctWinHeader.buttonBox.get_first_child(); pChild; pChild = pChild->get_next_sibling()) {
            buttons.push_back(pChild);
        }
#else
        std::vector<Gtk::Widget*> buttons = _ctWinHeader.buttonBox.get_children();
#endif
        const std::vector<gint64>& nodes = _ctStateMachine.get_visited_nodes_list();
        _ctWinHeader.button_to_node_id.clear();
        for (auto iter = nodes.rbegin(); iter != nodes.rend(); ++iter) {
            if (*iter == curr_node_id) continue;
            if (CtTreeIter ct_tree_iter = ctTreestore.get_node_from_node_id(*iter)) {
                Glib::ustring name = "<span font_desc=\"" + _pCtConfig->treeFont + "\">" + str::xml_escape(ct_tree_iter.get_node_name()) + "</span>";
                Glib::ustring tooltip = CtMiscUtil::get_node_hierarchical_name(ct_tree_iter, "/", false);
                if (auto pButton = dynamic_cast<Gtk::Button*>(buttons[buttons.size() - 1 - button_idx])) {
                    if (auto pHBox = dynamic_cast<Gtk::Box*>(pButton->get_child())) {
#if GTKMM_MAJOR_VERSION >= 4
                        std::vector<Gtk::Widget*> hbox_children;
                        for (auto pChild = pHBox->get_first_child(); pChild; pChild = pChild->get_next_sibling()) {
                            hbox_children.push_back(pChild);
                        }
#else
                        std::vector<Gtk::Widget*> hbox_children = pHBox->get_children();
#endif
                        for (auto pWidget : hbox_children) {
                            if (auto pLabel = dynamic_cast<Gtk::Label*>(pWidget)) {
                                pLabel->set_label(name);
                                pLabel->set_use_markup(true);
#if GTKMM_MAJOR_VERSION >= 4
                                pLabel->set_ellipsize(Pango::EllipsizeMode::END);
#else
                                pLabel->set_ellipsize(Pango::ELLIPSIZE_END);
#endif
                            }
                            else if (auto pImage = dynamic_cast<Gtk::Image*>(pWidget)) {
                                const char* node_icon = ctTreestore.get_node_icon(
                                    ctTreestore.get_store()->iter_depth(ct_tree_iter),
                                    ct_tree_iter.get_node_syntax_highlighting(),
                                    ct_tree_iter.get_node_custom_icon_id());
#if GTKMM_MAJOR_VERSION >= 4
                                pImage->set_from_icon_name(node_icon);
#else
                                pImage->set_from_icon_name(node_icon, Gtk::ICON_SIZE_MENU);
#endif
                            }
                            else {
                                spdlog::debug("? pLabel pImage");
                            }
                        }
                    }
                    else {
                        spdlog::debug("? pHBox");
                    }
                    pButton->set_tooltip_text(tooltip);
#if GTKMM_MAJOR_VERSION < 4
                    pButton->show_all();
#endif
                    _ctWinHeader.button_to_node_id[pButton] = *iter;
                }
                else {
                    spdlog::debug("? pButton");
                }
                ++button_idx;
                if (button_idx == _pCtConfig->nodesOnNodeNameHeader) {
                    break;
                }
            }
        }
        for (; button_idx < (int)buttons.size(); ++button_idx) {
            buttons[buttons.size() - 1 - button_idx]->hide();
        }
    }
}

void CtMainWin::window_header_update_lock_icon(const bool show)
{
    _ctWinHeader.lockIcon.set_visible(show);
}

void CtMainWin::window_header_update_ghost_icon(const bool show)
{
    _ctWinHeader.ghostIcon.set_visible(show);
}

void CtMainWin::window_header_update_bookmark_icon(const bool show)
{
    _ctWinHeader.bookmarkIcon.set_visible(show);
}

void CtMainWin::menu_top_optional_bookmarks_enforce()
{
#if GTKMM_MAJOR_VERSION < 4
    auto pBookmarksMenu = dynamic_cast<Gtk::Widget*>(_pBookmarksSubmenus[2]);
    if (pBookmarksMenu) {
        pBookmarksMenu->set_visible(_pCtConfig->bookmarksInTopMenu);
    }
#else
    // GTK4: bookmark visibility handled differently
#endif
}

void CtMainWin::menu_update_bookmark_menu_item(bool is_bookmarked)
{
#if GTKMM_MAJOR_VERSION >= 4
        {
        auto a1 = _uCtMenu->find_action("node_bookmark");
        auto a2 = _uCtMenu->find_action("node_unbookmark");
    #if GTKMM_MAJOR_VERSION >= 4
        if (a1 && a1->signal_set_visible) a1->signal_set_visible(!is_bookmarked);
        if (a2 && a2->signal_set_visible) a2->signal_set_visible(is_bookmarked);
    #else
        if (a1 && a1->signal_set_visible) a1->signal_set_visible->emit(!is_bookmarked);
        if (a2 && a2->signal_set_visible) a2->signal_set_visible->emit(is_bookmarked);
    #endif
        }
#else
    // GTK3: bookmark menu update handled differently
    (void)is_bookmarked; // silence unused warning in GTK3
#endif
}

void CtMainWin::menu_update_doc_path_menu_item()
{
    if (auto action = _uCtMenu->find_action("doc_path_clip")) {
#if GTKMM_MAJOR_VERSION >= 4
        if (action->signal_set_visible) {
            action->signal_set_visible(not _uCtStorage->get_file_path().empty());
        }
#else
        if (action->signal_set_visible) {
            action->signal_set_visible->emit(not _uCtStorage->get_file_path().empty());
        }
#endif
    }
}

void CtMainWin::maybe_show_start_dialog()
{
    if (_startDialogShown || _no_gui || not _pCtConfig->showStartDialog) {
        return;
    }
    if (not get_visible()) {
        if (not _startDialogShowConn.connected()) {
            _startDialogShowConn = signal_show().connect([this]() {
                if (_startDialogShowConn.connected()) {
                    _startDialogShowConn.disconnect();
                }
                maybe_show_start_dialog();
            });
        }
        return;
    }
    if (not _uCtStorage->get_file_path().empty()) {
        return;
    }
    if (get_tree_store().get_iter_first()) {
        return;
    }

    _startDialogShown = true;
    Glib::signal_idle().connect_once([this]() {
        bool dont_show_again{false};
        std::string recent_filepath;
        CtDialogs::CtStartDialogAction action = CtDialogs::start_dialog(
            this,
            _pCtConfig->recentDocsFilepaths,
            _pCtConfig->rememberRecentDocs,
            recent_filepath,
            dont_show_again);
        if (dont_show_again) {
            _pCtConfig->showStartDialog = false;
        }
        switch (action) {
            case CtDialogs::CtStartDialogAction::NewDoc:
                _uCtActions->node_add();
                break;
            case CtDialogs::CtStartDialogAction::OpenFile:
                _uCtActions->file_open();
                break;
            case CtDialogs::CtStartDialogAction::OpenFolder:
                _uCtActions->folder_open();
                break;
            case CtDialogs::CtStartDialogAction::OpenRecent:
                if (not recent_filepath.empty()) {
                    file_open(recent_filepath, ""/*node*/, ""/*anchor*/);
                }
                break;
            case CtDialogs::CtStartDialogAction::None:
            default:
                break;
        }
    });
}

void CtMainWin::menu_set_bookmark_menu_items()
{
    #if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    std::list<std::tuple<gint64, Glib::ustring, const char*>> bookmarks;
    for (const gint64& node_id : _uCtTreestore->bookmarks_get()) {
        CtTreeIter ct_tree_iter = _uCtTreestore->get_node_from_node_id(node_id);
        bookmarks.push_back(std::make_tuple(node_id,
            ct_tree_iter.get_node_name(),
            _uCtTreestore->get_node_icon(_uCtTreestore->get_store()->iter_depth(ct_tree_iter),
                ct_tree_iter.get_node_syntax_highlighting(),
                ct_tree_iter.get_node_custom_icon_id())));
    }

    sigc::slot<void, gint64> bookmark_action = [&](gint64 node_id) {
        CtTreeIter tree_iter = _uCtTreestore->get_node_from_node_id(node_id);
        if (tree_iter) {
            _uCtTreeview->set_cursor_safe(tree_iter);
        }
    };
    for (unsigned i = 0; i < _pBookmarksSubmenus.size(); ++i) {
        if (_pBookmarksSubmenus[i]) {
            _pBookmarksSubmenus[i]->set_submenu(*_uCtMenu->build_bookmarks_menu(bookmarks, bookmark_action, 2 == i/*isTopMenu*/));
        }
    }
    #else
    if (!_gtk4Toolbars.empty()) {
        std::list<std::tuple<gint64, Glib::ustring, const char*>> bookmarks;
        for (const gint64& node_id : _uCtTreestore->bookmarks_get()) {
            CtTreeIter ct_tree_iter = _uCtTreestore->get_node_from_node_id(node_id);
            bookmarks.push_back(std::make_tuple(node_id,
                ct_tree_iter.get_node_name(),
                _uCtTreestore->get_node_icon(_uCtTreestore->get_store()->iter_depth(ct_tree_iter),
                    ct_tree_iter.get_node_syntax_highlighting(),
                    ct_tree_iter.get_node_custom_icon_id())));
        }
        std::function<void(gint64)> bookmark_action = [this](gint64 node_id) {
            CtTreeIter tree_iter = _uCtTreestore->get_node_from_node_id(node_id);
            if (tree_iter) _uCtTreeview->set_cursor_safe(tree_iter);
        };
        if (_pBookmarksButton4) {
            _gtk4Toolbars[0]->remove(*_pBookmarksButton4);
        }
        _pBookmarksButton4 = _uCtMenu->build_bookmarks_button4(bookmarks, bookmark_action, true);
        _gtk4Toolbars[0]->append(*_pBookmarksButton4);
        _pBookmarksButton4->show();
    }
    #endif
}

void CtMainWin::menu_set_items_recent_documents()
{
    if (not _pCtConfig->rememberRecentDocs) return;
    #if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    sigc::slot<void, const std::string&> recent_doc_open_action = [&](const std::string& filepath){
        if (Glib::file_test(filepath, Glib::FILE_TEST_EXISTS)) {
            if (file_open(filepath, ""/*node*/, ""/*anchor*/)) {
                _pCtConfig->recentDocsFilepaths.move_or_push_front(fs_canonicalize_filename(filepath));
                menu_set_items_recent_documents();
            }
        }
        else {
            g_autofree gchar* title = g_strdup_printf(_("The Path %s does Not Exist"), str::xml_escape(filepath).c_str());
            CtDialogs::error_dialog(Glib::ustring{title}, *this);
            _pCtConfig->recentDocsFilepaths.move_or_push_back(fs_canonicalize_filename(filepath));
            menu_set_items_recent_documents();
        }
    };
    sigc::slot<void, const std::string&> recent_doc_rm_action = [&](const std::string& filepath){
        _pCtConfig->recentDocsFilepaths.remove(filepath);
        menu_set_items_recent_documents();
    };
    if (_pRecentDocsSubmenu) {
        Gtk::Menu* pMenu = _pRecentDocsSubmenu->get_submenu();
        delete pMenu;
        _pRecentDocsSubmenu->set_submenu(*_uCtMenu->build_recent_docs_menu(_pCtConfig->recentDocsFilepaths,
                                                                           recent_doc_open_action,
                                                                           recent_doc_rm_action));
    }
    if (_pRecentDocsMenuToolButton) {
        _pRecentDocsMenuToolButton->set_arrow_tooltip_text(_("Open a Recent CherryTree Document"));
        Gtk::Menu* pMenu = _pRecentDocsMenuToolButton->get_menu();
        delete pMenu;
        _pRecentDocsMenuToolButton->set_menu(*_uCtMenu->build_recent_docs_menu(_pCtConfig->recentDocsFilepaths,
                                                                               recent_doc_open_action,
                                                                               recent_doc_rm_action));
    }
    #else
    _uCtMenu->populate_recent_docs_menu4(_pRecentDocsMenuButton4, _pCtConfig->recentDocsFilepaths);
    #endif
}

void CtMainWin::menu_set_visible_exit_app(bool visible)
{
#if GTKMM_MAJOR_VERSION < 4
    if (auto quit_label = CtMenu::get_accel_label(CtMenu::find_menu_item(_pMenuBar, "quit_app"))) {
        quit_label->set_label(visible ? _("_Hide") : _("_Quit"));
        quit_label->set_tooltip_markup(visible ?  _("Hide the Window") : _("Quit the Application"));
    }
    CtMenu::find_menu_item(_pMenuBar, "exit_app")->set_visible(visible);
#else
    // GTK4: menu handling different
#endif
}

void CtMainWin::menu_rebuild_toolbars(bool new_toolbar)
{
    if (new_toolbar) {
        #if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
        _pRecentDocsMenuToolButton = nullptr;
        _pSaveToolButton = nullptr;
        for (auto pToolbar : _pToolbars) {
            _vboxMain.remove(*pToolbar);
        }
        _pToolbars = _uCtMenu->build_toolbars(_pRecentDocsMenuToolButton, _pSaveToolButton);
        for (auto toolbar = _pToolbars.rbegin(); toolbar != _pToolbars.rend(); ++toolbar) {
            _vboxMain.pack_start(*(*toolbar), false, false);
            if (not _pCtConfig->menubarInTitlebar) {
                _vboxMain.reorder_child(*(*toolbar), 1);
            }
            else {
                _vboxMain.reorder_child(*(*toolbar), 0);
            }
        }
        menu_set_items_recent_documents();
        window_title_update();
#if GTKMM_MAJOR_VERSION < 4
        for (auto pToolbar : _pToolbars) {
            pToolbar->show_all();
        }
#endif
        #else
        // For GTK4, remove existing Box toolbars and rebuild
        // Assuming they were added just before _vPaned
        // (we keep it simple: no reorder here)
        // A more advanced approach would track them explicitly.
        Gtk::MenuButton* recentBtn{}; Gtk::Button* saveBtn{};
        auto toolbars4 = _uCtMenu->build_toolbars4(recentBtn, saveBtn);
        for (auto pBox : toolbars4) {
#if GTKMM_MAJOR_VERSION >= 4
            _vboxMain.prepend(*pBox);
#else
            _vboxMain.pack_start(*pBox, false, false);
            pBox->show_all();
#endif
        }
        window_title_update();
        #endif
    }

    #if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    show_hide_toolbars(_pCtConfig->toolbarVisible);
    for (auto pToolbar : _pToolbars) {
        pToolbar->set_toolbar_style(Gtk::ToolbarStyle::TOOLBAR_ICONS);
    }
    set_toolbars_icon_size(_pCtConfig->toolbarIconSize);
    #endif
}

void CtMainWin::config_switch_tree_side()
{
    auto tree_width = _scrolledwindowTree.get_width();
    auto text_width = _vboxText.get_width();

#if GTKMM_MAJOR_VERSION >= 4
    if (_pCtConfig->treeRightSide) {
        _hPaned.set_start_child(_vboxText);
        _hPaned.set_end_child(_scrolledwindowTree);
        _hPaned.set_position(text_width);
    }
    else {
        _hPaned.set_start_child(_scrolledwindowTree);
        _hPaned.set_end_child(_vboxText);
        _hPaned.set_position(tree_width);
    }
#else
    _hPaned.remove(_scrolledwindowTree);
    _hPaned.remove(_vboxText);
    if (_pCtConfig->treeRightSide) {
        _hPaned.pack1(_vboxText, Gtk::EXPAND);
        _hPaned.pack2(_scrolledwindowTree, Gtk::FILL);
        _hPaned.property_position() = text_width;
    }
    else {
        _hPaned.pack1(_scrolledwindowTree, Gtk::FILL);
        _hPaned.pack2(_vboxText, Gtk::EXPAND);
        _hPaned.property_position() = tree_width;
    }
#endif
    _pCtConfig->hpanedPos = _hPaned.property_position();
}

void CtMainWin::_zoom_tree(const std::optional<bool> is_increase)
{
#if GTKMM_MAJOR_VERSION >= 4
    // GTK4: get font from config instead of style context
    Pango::FontDescription fontDesc(_pCtConfig->treeFont);
    const int size_pre = fontDesc.get_size() / Pango::SCALE;
#else
    Glib::RefPtr<Gtk::StyleContext> pContext = _uCtTreeview->get_style_context();
    const Pango::FontDescription fontDesc = pContext->get_font(pContext->get_state());
    const int size_pre = fontDesc.get_size() / Pango::SCALE;
#endif
    int size_new = 0;
    if (is_increase.has_value()) {
        if (0 == _pCtConfig->treeResetFontSize) {
            _pCtConfig->treeResetFontSize = size_pre;
            spdlog::debug("{} set reset to {}", __FUNCTION__, size_pre);
        }
        size_new = size_pre + (is_increase.value() ? 1 : -1);
    }
    else {
        // it's a reset
        if (0 == _pCtConfig->treeResetFontSize || size_pre == _pCtConfig->treeResetFontSize) {
            spdlog::debug("{} reset not necessary", __FUNCTION__);
        }
        else {
            size_new = _pCtConfig->treeResetFontSize;
        }
    }
    if (size_new > 0) {
        if (size_new < 6) size_new = 6;
        _pCtConfig->treeFont = CtFontUtil::get_font_str(CtFontUtil::get_font_family(_pCtConfig->treeFont), size_new);
#if GTKMM_MAJOR_VERSION >= 4
        emit_app_apply_for_each_window([](CtMainWin* win) { win->update_theme(); win->window_header_update(); });
#else
        signal_app_apply_for_each_window([](CtMainWin* win) { win->update_theme(); win->window_header_update(); });
#endif
    }
}

void CtMainWin::reset()
{
    auto on_scope_exit = scope_guard([&](void*) { user_active() = true; });
    user_active() = false;

    _ctStateMachine.reset();

    _uCtStorage.reset(CtStorageControl::create_dummy_storage(this));

    _reset_CtTreestore_CtTreeview();

    _latestStatusbarUpdateTime.clear();
#if GTKMM_MAJOR_VERSION < 4
    for (auto pButton : _ctWinHeader.buttonBox.get_children()) {
        pButton->hide();
    }
#endif
    _ctWinHeader.nameLabel.set_markup("");
    window_header_update_lock_icon(false);
    window_header_update_ghost_icon(false);
    window_header_update_bookmark_icon(false);
    menu_set_bookmark_menu_items();
    {
        auto a = _uCtMenu->find_action("ct_vacuum");
    #if GTKMM_MAJOR_VERSION >= 4
        if (a && a->signal_set_visible) a->signal_set_visible(false);
    #else
        if (a && a->signal_set_visible) a->signal_set_visible->emit(false);
    #endif
    }
    menu_update_doc_path_menu_item();
    menu_top_optional_bookmarks_enforce();

    update_window_save_not_needed();
    _ctTextview.set_buffer(Glib::RefPtr<Gtk::TextBuffer>{});
    _ctTextview.set_spell_check(false);
    if (not _no_gui) {
        _ctTextview.mm().set_sensitive(false);
    }
}

void CtMainWin::update_selected_node_statusbar_info()
{
    CtTreeIter treeIter = curr_tree_iter();
    Glib::ustring statusbar_text;
    if (not treeIter) {
        statusbar_text = _("No Node is Selected");
    }
    else {
        const std::string separator_text{"  -  "};
        statusbar_text = Glib::ustring{_("Node Type")} + _(": ");
        const std::string syntaxHighl = treeIter.get_node_syntax_highlighting();
        if (CtConst::RICH_TEXT_ID == syntaxHighl) {
            statusbar_text += _("Rich Text");
        }
        else if (CtConst::PLAIN_TEXT_ID == syntaxHighl) {
            statusbar_text += _("Plain Text");
        }
        else {
            statusbar_text += syntaxHighl;
        }
        if (not treeIter.get_node_tags().empty()) {
            statusbar_text += separator_text + _("Tags") + _(": ") + treeIter.get_node_tags();
        }
        if (_pCtConfig->enableSpellCheck && curr_tree_iter().get_node_is_text()) {
            statusbar_text += separator_text + _("Spell Check") + _(": ") + _pCtConfig->spellCheckLang;
        }
        if (_pCtConfig->wordCountOn) {
            statusbar_text += separator_text + _("Word Count") + _(": ") + std::to_string(CtTextIterUtil::get_words_count(_ctTextview.get_buffer()));
        }
        if (treeIter.get_node_creating_time() > 0) {
            const Glib::ustring timestamp_creation = str::time_format(_pCtConfig->timestampFormat, treeIter.get_node_creating_time());
            statusbar_text += separator_text + _("Date Created") + _(": ") + timestamp_creation;
        }
        if (treeIter.get_node_modification_time() > 0) {
            const Glib::ustring timestamp_lastsave = str::time_format(_pCtConfig->timestampFormat, treeIter.get_node_modification_time());
            statusbar_text += separator_text + _("Date Modified") + _(": ") + timestamp_lastsave;
        }
    }
    _ctStatusBar.update_status(statusbar_text);
}

void CtMainWin::tree_node_paste_from_other_window(CtMainWin* pWinToCopyFrom, gint64 nodeIdToCopyFrom)
{
    if (not pWinToCopyFrom) {
        CtDialogs::warning_dialog(_("No Previous Node Copy Was Performed During This Session or the Source Tree is No Longer Available."), *this);
        return;
    }
    CtTreeStore& other_ct_tree_store = pWinToCopyFrom->get_tree_store();
    CtTreeIter   other_ct_tree_iter = other_ct_tree_store.get_node_from_node_id(nodeIdToCopyFrom);
    if (not other_ct_tree_iter) {
        CtDialogs::warning_dialog(_("The Source Tree Node is No Longer Available."), *this);
        return;
    }
    _uCtActions->node_subnodes_paste2(other_ct_tree_iter, pWinToCopyFrom);
}
