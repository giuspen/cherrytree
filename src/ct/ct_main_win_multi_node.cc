/*
 * ct_main_win_multi_node.cc
 *
 * Copyright 2009-2026
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "ct_main_win.h"

void CtMainWin::_setup_multi_node_editor()
{
    _multiNodePrevButton.set_label(_("Previous"));
    _multiNodeNextButton.set_label(_("Next"));
#if GTKMM_MAJOR_VERSION >= 4
    _multiNodePageBar.append(_multiNodePrevButton);
    _multiNodePageBar.append(_multiNodePageLabel);
    _multiNodePageBar.append(_multiNodeNextButton);
    _multiNodePageLabel.set_hexpand(true);
#else
    _multiNodePageBar.pack_start(_multiNodePrevButton, false, false);
    _multiNodePageBar.pack_start(_multiNodePageLabel, true, true);
    _multiNodePageBar.pack_start(_multiNodeNextButton, false, false);
#endif
    _multiNodePageLabel.set_xalign(0.5f);
    _multiNodePrevButton.signal_clicked().connect([this](){
        const auto selected = selected_tree_iters();
        if (selected.size() > 1 and _multiNodePageStart >= MULTI_NODE_PAGE_SIZE) {
            _show_multi_node_editor(selected, _multiNodePageStart - MULTI_NODE_PAGE_SIZE);
        }
    });
    _multiNodeNextButton.signal_clicked().connect([this](){
        const auto selected = selected_tree_iters();
        if (selected.size() > _multiNodePageStart + MULTI_NODE_PAGE_SIZE) {
            _show_multi_node_editor(selected, _multiNodePageStart + MULTI_NODE_PAGE_SIZE);
        }
    });
}

CtMainWin::CtMultiNodeSection* CtMainWin::_find_multi_node_section(const gint64 node_id, CtTextView* pTextView)
{
    for (const auto& section : _multiNodeSections) {
        if (section->textView == pTextView and section->treeIter.get_node_id() == node_id) {
            return section.get();
        }
    }
    return nullptr;
}

void CtMainWin::_update_multi_node_section_height(CtTextView& text_view)
{
    auto buffer = text_view.get_buffer();
    if (not buffer) return;
    const int line_count = std::max(1, buffer->get_line_count());
    text_view.mm().set_size_request(-1, std::max(80, line_count * 24 + 24));
}

void CtMainWin::_clear_multi_node_editor()
{
    if (not _multiNodeMode and _multiNodeSections.empty()) return;
    _multiNodeEditorRebuilding = true;
    ++_multiNodeEditorGeneration;

    // Event handlers can run synchronously while focused widgets are removed.
    // Stop exposing a temporary text view before disconnecting or destroying it.
    _pActiveTextview = &_ctTextview;
    _uCtTreestore->disconnect_text_view_connections();

    for (auto& section : _multiNodeSections) {
        section->focusConnection.disconnect();
        section->heightConnection.disconnect();
    }
    for (auto& section : _multiNodeSections) {
        if (section->usesScrolledWindow) {
#if GTKMM_MAJOR_VERSION >= 4
            section->scrolledWindow.set_child(nullptr);
#else
            section->scrolledWindow.remove();
#endif
            _multiNodeBox.remove(section->scrolledWindow);
        }
        else {
            _multiNodeBox.remove(section->textView->mm());
        }
        if (section->titleAdded) _multiNodeBox.remove(section->title);
        if (section->separatorAdded) _multiNodeBox.remove(section->separator);
    }
    if (_multiNodePageBarVisible) {
        _multiNodePageBar.hide();
        _multiNodePageBarVisible = false;
    }
    _multiNodeSections.clear();

#if GTKMM_MAJOR_VERSION >= 4
    _scrolledwindowText.set_child(_ctTextview.mm());
    _scrolledwindowText.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
#else
    _scrolledwindowText.remove();
    _scrolledwindowText.add(_ctTextview.mm());
    _scrolledwindowText.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _ctTextview.mm().show();
#endif
    _scrolledwindowText.set_overlay_scrolling(static_cast<bool>(_pCtConfig->overlayScroll));
    _multiNodeMode = false;
    _ctTextview.set_scroll_beyond_last_line(_pCtConfig->scrollBeyondLastLine);
    _multiNodeEditorRebuilding = false;
}

void CtMainWin::_show_multi_node_editor(const std::vector<CtTreeIter>& tree_iters, size_t requested_page_start)
{
    if (tree_iters.size() < 2) return;
    const gint64 active_node_id = _activeTreeIter ? _activeTreeIter.get_node_id() : -1;
    const CtTreeIter cursor_iter = tree_cursor_iter();
    const gint64 cursor_node_id = cursor_iter ? cursor_iter.get_node_id() : -1;
    _clear_multi_node_editor();
    _multiNodeEditorRebuilding = true;
    const guint64 generation = ++_multiNodeEditorGeneration;
    _uCtTreestore->disconnect_text_view_connections();

#if GTKMM_MAJOR_VERSION >= 4
    _scrolledwindowText.set_child(_multiNodeBox);
#else
    _scrolledwindowText.remove();
    _scrolledwindowText.add(_multiNodeBox);
#endif
    _multiNodeMode = true;

    size_t page_start = requested_page_start;
    if (page_start == static_cast<size_t>(-1)) {
        auto selected_cursor = std::find_if(tree_iters.begin(), tree_iters.end(), [cursor_node_id](const CtTreeIter& iter){
            return iter.get_node_id() == cursor_node_id;
        });
        const size_t cursor_index = selected_cursor == tree_iters.end() ? 0 : std::distance(tree_iters.begin(), selected_cursor);
        page_start = (cursor_index / MULTI_NODE_PAGE_SIZE) * MULTI_NODE_PAGE_SIZE;
    }
    if (page_start >= tree_iters.size()) {
        page_start = ((tree_iters.size() - 1) / MULTI_NODE_PAGE_SIZE) * MULTI_NODE_PAGE_SIZE;
    }
    _multiNodePageStart = page_start;
    const size_t page_end = std::min(tree_iters.size(), page_start + MULTI_NODE_PAGE_SIZE);

    gint64 estimated_page_height{0};
    for (size_t i = page_start; i < page_end; ++i) {
        const auto buffer = tree_iters[i].get_node_text_buffer();
        const int line_count = buffer ? std::max(1, buffer->get_line_count()) : 1;
        estimated_page_height += std::min<gint64>(MULTI_NODE_SAFE_TOTAL_HEIGHT,
                                                 static_cast<gint64>(line_count) * 24 + 24);
    }
    const bool compact_sections = tree_iters.size() > MULTI_NODE_PAGE_SIZE or
                                  estimated_page_height > MULTI_NODE_SAFE_TOTAL_HEIGHT;

    if (compact_sections) {
        // Leave room for section-local scrollbars beside the outer scrollbar.
#if GTKMM_MAJOR_VERSION >= 4
        _scrolledwindowText.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
#else
        _scrolledwindowText.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
#endif
        _scrolledwindowText.set_overlay_scrolling(false);
    }

    if (tree_iters.size() > MULTI_NODE_PAGE_SIZE) {
        _multiNodePageLabel.set_text(str::format(_("Showing selected nodes %s-%s of %s"),
                                                std::to_string(page_start + 1),
                                                std::to_string(page_end),
                                                std::to_string(tree_iters.size())));
        _multiNodePrevButton.set_sensitive(page_start > 0);
        _multiNodeNextButton.set_sensitive(page_end < tree_iters.size());
#if GTKMM_MAJOR_VERSION >= 4
        _multiNodePageBar.show();
#else
        _multiNodePrevButton.show();
        _multiNodePageLabel.show();
        _multiNodeNextButton.show();
        _multiNodePageBar.show();
#endif
        _multiNodePageBarVisible = true;
    }

    gint64 main_view_node_id = active_node_id;
    if (std::none_of(tree_iters.begin() + page_start, tree_iters.begin() + page_end, [main_view_node_id](const CtTreeIter& iter){
        return iter.get_node_id() == main_view_node_id;
    })) {
        main_view_node_id = cursor_node_id;
    }
    if (std::none_of(tree_iters.begin() + page_start, tree_iters.begin() + page_end, [main_view_node_id](const CtTreeIter& iter){
        return iter.get_node_id() == main_view_node_id;
    })) {
        main_view_node_id = tree_iters[page_start].get_node_id();
    }

    CtMultiNodeSection* pActiveSection{nullptr};
    bool main_view_used{false};
    for (size_t i = page_start; i < page_end; ++i) {
        auto section = std::make_unique<CtMultiNodeSection>();
        section->treeIter = tree_iters[i];
        if (not main_view_used and tree_iters[i].get_node_id() == main_view_node_id) {
            section->textView = &_ctTextview;
            main_view_used = true;
        }
        else {
            section->ownedTextView = std::make_unique<CtTextView>(this);
            section->textView = section->ownedTextView.get();
            _connect_text_view_events(*section->textView);
        }

        if (i > page_start) {
#if GTKMM_MAJOR_VERSION >= 4
            _multiNodeBox.append(section->separator);
#else
            _multiNodeBox.pack_start(section->separator, false, false);
#endif
            section->separatorAdded = true;
        }
        if (_pCtConfig->multiNodeShowTitles) {
            const Glib::ustring title = _pCtConfig->nodeNameHeaderShowFullPath ?
                Glib::ustring{CtMiscUtil::get_node_hierarchical_name(tree_iters[i], " / ", false)} : tree_iters[i].get_node_name();
            section->title.set_markup("<b>" + str::xml_escape(title) + "</b>");
            section->title.set_xalign(0.0f);
            section->title.set_margin_start(8);
            section->title.set_margin_top(6);
            section->title.set_margin_bottom(4);
#if GTKMM_MAJOR_VERSION >= 4
            _multiNodeBox.append(section->title);
#else
            _multiNodeBox.pack_start(section->title, false, false);
#endif
            section->titleAdded = true;
        }

        CtTreeIter iter_for_binding = section->treeIter;
        _uCtTreestore->text_view_apply_textbuffer(iter_for_binding, section->textView, false, false);
        section->textView->set_scroll_beyond_last_line(_pCtConfig->scrollBeyondLastLine and i + 1 == page_end);
        if (compact_sections) {
            section->usesScrolledWindow = true;
#if GTKMM_MAJOR_VERSION >= 4
            section->scrolledWindow.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
#else
            section->scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
#endif
            section->scrolledWindow.set_overlay_scrolling(false);
            section->scrolledWindow.set_size_request(-1, MULTI_NODE_SECTION_MAX_HEIGHT);
#if GTKMM_MAJOR_VERSION >= 4
            section->scrolledWindow.set_child(section->textView->mm());
            section->textView->mm().set_hexpand(true);
            _multiNodeBox.append(section->scrolledWindow);
#else
            section->scrolledWindow.add(section->textView->mm());
            _multiNodeBox.pack_start(section->scrolledWindow, false, false);
#endif
        }
        else {
#if GTKMM_MAJOR_VERSION >= 4
            _multiNodeBox.append(section->textView->mm());
            section->textView->mm().set_hexpand(true);
#else
            _multiNodeBox.pack_start(section->textView->mm(), false, false);
#endif
            _update_multi_node_section_height(*section->textView);
        }

        CtMultiNodeSection* pSection = section.get();
        const gint64 node_id = section->treeIter.get_node_id();
        CtTextView* pTextView = section->textView;
        section->focusConnection = section->textView->mm().property_has_focus().signal_changed().connect([this, generation, node_id, pTextView](){
            if (_multiNodeEditorRebuilding or generation != _multiNodeEditorGeneration) return;
            CtMultiNodeSection* section = _find_multi_node_section(node_id, pTextView);
            if (section and section->textView->mm().has_focus()) {
                _set_active_editor(section->treeIter, section->textView);
            }
        });
        section->heightConnection = section->textView->get_buffer()->signal_changed().connect([this, generation, node_id, pTextView, compact_sections](){
            if (_multiNodeEditorRebuilding or generation != _multiNodeEditorGeneration or compact_sections) return;
            CtMultiNodeSection* section = _find_multi_node_section(node_id, pTextView);
            if (section) _update_multi_node_section_height(*section->textView);
        });
        if (tree_iters[i].get_node_id() == main_view_node_id) pActiveSection = pSection;
        _multiNodeSections.push_back(std::move(section));
    }

#if GTKMM_MAJOR_VERSION >= 4
    _multiNodeBox.show();
#else
    _multiNodeBox.show_all();
#endif
    _multiNodeEditorRebuilding = false;
    if (pActiveSection) {
        _set_active_editor(pActiveSection->treeIter, pActiveSection->textView);
    }
}

void CtMainWin::refresh_multi_node_editor()
{
    if (_multiNodeEditorRebuilding) return;
    const auto selected = selected_tree_iters();
    if (selected.size() > 1) _show_multi_node_editor(selected);
}

void CtMainWin::set_show_line_numbers(const bool show)
{
    _ctTextview.set_show_line_numbers(show);
    for (const auto& section : _multiNodeSections) {
        if (section->textView != &_ctTextview) {
            section->textView->set_show_line_numbers(show);
        }
    }
}

void CtMainWin::set_scroll_beyond_last_line(const bool enabled)
{
    if (_multiNodeMode) {
        for (size_t i = 0; i < _multiNodeSections.size(); ++i) {
            _multiNodeSections[i]->textView->set_scroll_beyond_last_line(enabled and i + 1 == _multiNodeSections.size());
        }
    }
    else {
        _ctTextview.set_scroll_beyond_last_line(enabled);
    }
}

void CtMainWin::activate_editor_for_widget(CtAnchoredWidget* pWidget)
{
    if (_multiNodeEditorRebuilding or not _multiNodeMode or not pWidget) return;
    for (const auto& section : _multiNodeSections) {
        const auto widgets = section->treeIter.get_anchored_widgets_fast();
        if (std::find(widgets.begin(), widgets.end(), pWidget) != widgets.end()) {
            _set_active_editor(section->treeIter, section->textView);
            return;
        }
    }
}
