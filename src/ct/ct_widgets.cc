/*
 * ct_widgets.cc
 *
 * Copyright 2009-2025
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

#include "ct_widgets.h"
#include "ct_main_win.h"
#include <glib/gstdio.h>
#include "ct_app.h"

CtTmp::~CtTmp()
{
    //std::cout << "~CtTmp()" << std::endl;
    for (const auto& currPair : _mapHiddenFiles) {
        if ( Glib::file_test(currPair.second, Glib::FILE_TEST_IS_REGULAR) and
             0 != g_remove(currPair.second) )
        {
            spdlog::error("!! g_remove");
        }
        g_free(currPair.second);
    }
    for (const auto& currPair : _mapHiddenDirs) {
        fs::remove_all(currPair.second);
        g_free(currPair.second);
    }
}

fs::path CtTmp::getHiddenDirPath(const fs::path& visiblePath)
{
    if (not _mapHiddenDirs.count(visiblePath.string())) {
        _mapHiddenDirs[visiblePath.string()] = g_dir_make_tmp(nullptr, nullptr);
    }
    return _mapHiddenDirs.at(visiblePath.string());
}

fs::path CtTmp::getHiddenFilePath(const fs::path& visiblePath)
{
    if (not _mapHiddenFiles.count(visiblePath.string())) {
        fs::path tempDir = getHiddenDirPath(visiblePath);
        fs::path basename = visiblePath.filename();
        if (basename.extension() == ".ctx") {
            basename = basename.stem();
            basename += ".ctb";
        }
        else if (basename.extension() == ".ctz") {
            basename = basename.stem();
            basename += ".ctd";
        }
        _mapHiddenFiles[visiblePath.string()] = g_build_filename(tempDir.c_str(), basename.c_str(), nullptr);
    }
    return _mapHiddenFiles.at(visiblePath.string());
}

const double CtTextView::TEXT_SCROLL_MARGIN{0.1}; // margin as a [0.0,0.5] fraction of screen size

CtAnchoredWidget::CtAnchoredWidget(CtMainWin* pCtMainWin, const int charOffset, const std::string& justification)
 : _pCtMainWin{pCtMainWin}
 , _pCtConfig{pCtMainWin->get_ct_config()}
 , _charOffset{charOffset}
 , _justification{justification}
{
    _frame.set_shadow_type(Gtk::ShadowType::SHADOW_NONE);
    signal_button_press_event().connect([this](GdkEventButton* /*pEvent*/){
        _pCtMainWin->curr_buffer()->place_cursor(_pCtMainWin->curr_buffer()->get_iter_at_child_anchor(_rTextChildAnchor));
        return true; // we need to block this or the focus will go to the text buffer below
    });
    add(_frame);
}

void CtAnchoredWidget::updateJustification(const Gtk::TextIter& textIter)
{
    updateJustification(CtTextIterUtil::get_text_iter_alignment(textIter, _pCtMainWin));
}

void CtAnchoredWidget::insertInTextBuffer(Glib::RefPtr<Gtk::TextBuffer> rTextBuffer)
{
    _rTextChildAnchor = rTextBuffer->create_child_anchor(rTextBuffer->get_iter_at_offset(_charOffset));
    if (not _justification.empty()) {
        Gtk::TextIter textIterStart = rTextBuffer->get_iter_at_child_anchor(_rTextChildAnchor);
        Gtk::TextIter textIterEnd = textIterStart;
        textIterEnd.forward_char();
        Glib::ustring tagName = _pCtMainWin->get_text_tag_name_exist_or_create(CtConst::TAG_JUSTIFICATION, _justification);
        rTextBuffer->apply_tag_by_name(tagName, textIterStart, textIterEnd);
        std::optional<Glib::ustring> optTag = CtTextIterUtil::iter_get_tag_startingwith(textIterStart, CtConst::TAG_INVISIBLE_PREFIX);
        if (optTag.has_value()) {
            //spdlog::debug("{}", optTag.value().c_str());
            set_hidden(true);
        }
    }
}

void CtAnchoredWidget::_on_frame_size_allocate(Gtk::Allocation& allocation)
{
    if (allocation == _lastAllocation) {
        return;
    }
    const bool needWorkaround = _lastAllocation.get_height() != allocation.get_height();
    _lastAllocation = allocation;
    if (not needWorkaround) {
        return;
    }
    Glib::signal_idle().connect_once([&](){
        CtTextView& textView = _pCtMainWin->get_text_view();
        textView.mm().set_wrap_mode(_pCtMainWin->get_ct_config()->lineWrapping ? Gtk::WrapMode::WRAP_NONE : Gtk::WrapMode::WRAP_WORD_CHAR);
        textView.mm().set_wrap_mode(_pCtMainWin->get_ct_config()->lineWrapping ? Gtk::WrapMode::WRAP_WORD_CHAR : Gtk::WrapMode::WRAP_NONE);
    });
}

CtTreeView::CtTreeView(CtConfig* pCtConfig)
 : _pCtConfig{pCtConfig}
{
    set_headers_visible(false);
    set_enable_search(false);
    if (_pCtConfig->treeTooltips) {
        set_tooltips_enable(true/*on*/);
    }
}

void CtTreeView::set_tooltips_enable(const bool on)
{
    if (on) set_tooltip_column(1); // node name
    else set_tooltip_column(-1);
}

void CtTreeView::set_cursor_safe(const Gtk::TreeIter& treeIter)
{
    Gtk::TreeRow row = *treeIter;
    Gtk::TreeIter iterParent = row.parent();
    if (iterParent) {
        expand_to_path(get_model()->get_path(iterParent));
    }
    set_cursor(get_model()->get_path(treeIter));
}

void CtTreeView::set_tree_node_name_wrap_width(const bool wrap_enabled, const int wrap_width)
{
    Gtk::TreeViewColumn* pTVCol0 = get_column(CtTreeView::TITLE_COL_NUM);
    std::vector<Gtk::CellRenderer*> cellRenderers0 = pTVCol0->get_cells();
    if (cellRenderers0.size() > 1) {
        Gtk::CellRendererText *pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(cellRenderers0[1]);
        if (pCellRendererText) {
            pCellRendererText->property_wrap_mode().set_value(Pango::WRAP_CHAR);
            pCellRendererText->property_wrap_width().set_value(wrap_enabled ? wrap_width : -1);
        }
    }
}

CtStatusIcon::CtStatusIcon(CtApp& ctApp, CtConfig* pCtConfig)
 : _ctApp{ctApp}
 , _pCtConfig{pCtConfig}
{
}

Gtk::StatusIcon* CtStatusIcon::get()
{
    if (not _rStatusIcon) {
        _rStatusIcon = Gtk::StatusIcon::create(CtConst::APP_NAME);
        _rStatusIcon->set_title(CtConst::APP_NAME);
        _rStatusIcon->set_tooltip_markup(_("CherryTree Hierarchical Note Taking"));
        _rStatusIcon->signal_button_press_event().connect([&](GdkEventButton* event) {
            if (event->button == 1) { _ctApp.systray_show_hide_windows(); }
            return false;
        });
        _rStatusIcon->signal_popup_menu().connect([&](guint button, guint32 activate_time){
            if (not _uStatusIconMenu) {
                _uStatusIconMenu = std::make_unique<Gtk::Menu>();
                auto item1 = CtMenu::create_menu_item(_uStatusIconMenu.get(),
                                                      _("Show/Hide _CherryTree"),
                                                      CtConst::APP_NAME,
                                                      _pCtConfig->menusTooltips ? _("Toggle Show/Hide CherryTree") : nullptr);
                item1->signal_activate().connect([&](){ _ctApp.systray_show_hide_windows(); });
                auto item2 = CtMenu::create_menu_item(_uStatusIconMenu.get(),
                                                      _("_Exit CherryTree"),
                                                      "ct_quit-app",
                                                      _pCtConfig->menusTooltips ? _("Exit from CherryTree") : nullptr);
                item2->signal_activate().connect([&](){ _ctApp.close_all_windows(false/*fromKillCallback*/); });
            }
            _uStatusIconMenu->show_all();
            _uStatusIconMenu->popup(button, activate_time);
        });
    }
    return _rStatusIcon.get();
}

void CtStatusIcon::ensure_menu_hidden()
{
    if (_uStatusIconMenu) {
        _uStatusIconMenu->hide();
    }
}
