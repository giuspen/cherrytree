/*
 * tests_multi_node.cpp
 *
 * Copyright 2009-2026
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "tests_multi_node.h"
#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_storage_control.h"
#include "tests_common.h"
namespace {
void process_gtk_events()
{
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    while (gtk_events_pending()) gtk_main_iteration();
#else
    while (g_main_context_pending(nullptr)) g_main_context_iteration(nullptr, false);
#endif
}
void collect_text_views(GtkWidget* widget, std::vector<GtkWidget*>& text_views)
{
    const bool is_main_text_view = GTK_IS_TEXT_VIEW(widget) and
#if GTKMM_MAJOR_VERSION >= 4
        gtk_widget_has_css_class(widget, "ct-view-panel");
#else
        gtk_style_context_has_class(gtk_widget_get_style_context(widget), "ct-view-panel");
#endif
    if (is_main_text_view) text_views.push_back(widget);
#if GTKMM_MAJOR_VERSION >= 4
    for (GtkWidget* child = gtk_widget_get_first_child(widget); child; child = gtk_widget_get_next_sibling(child)) {
        collect_text_views(child, text_views);
    }
#else
    if (GTK_IS_CONTAINER(widget)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(widget));
        for (GList* child = children; child; child = child->next) {
            collect_text_views(GTK_WIDGET(child->data), text_views);
        }
        g_list_free(children);
    }
#endif
}
GtkWidget* find_label_with_prefix(GtkWidget* widget, const char* prefix)
{
    if (GTK_IS_LABEL(widget)) {
        const char* text = gtk_label_get_text(GTK_LABEL(widget));
        if (text and g_str_has_prefix(text, prefix)) return widget;
    }
#if GTKMM_MAJOR_VERSION >= 4
    for (GtkWidget* child = gtk_widget_get_first_child(widget); child; child = gtk_widget_get_next_sibling(child)) {
        if (GtkWidget* match = find_label_with_prefix(child, prefix)) return match;
    }
#else
    if (GTK_IS_CONTAINER(widget)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(widget));
        for (GList* child = children; child; child = child->next) {
            if (GtkWidget* match = find_label_with_prefix(GTK_WIDGET(child->data), prefix)) {
                g_list_free(children);
                return match;
            }
        }
        g_list_free(children);
    }
#endif
    return nullptr;
}
void select_primary_nodes(CtMainWin* pWin,
                          const Gtk::TreeModel::iterator& first,
                          const Gtk::TreeModel::iterator& second)
{
    auto selection = pWin->get_tree_view().get_selection();
    selection->unselect_all();
    pWin->get_tree_view().set_cursor_safe(second);
    selection->select(pWin->get_tree_store().get_path(first));
    pWin->show();
    process_gtk_events();
}
void assert_single_editor_transition(CtMainWin* pWin,
                                     const Gtk::TreeModel::iterator& first,
                                     const Gtk::TreeModel::iterator& second)
{
    for (const bool keep_temporary_view : {true, false}) {
        pWin->get_tree_view().set_cursor_safe(second);
        process_gtk_events();
        GtkWidget* permanent_view = GTK_WIDGET(pWin->get_text_view().mm().gobj());
        auto selection = pWin->get_tree_view().get_selection();
        selection->select(pWin->get_tree_store().get_path(first));
        pWin->show();
        process_gtk_events();
        CtTreeIter remaining = pWin->get_tree_store().to_ct_tree_iter(keep_temporary_view ? first : second);
        auto expected_buffer = remaining.get_node_text_buffer();
        std::vector<GtkWidget*> views;
        collect_text_views(GTK_WIDGET(pWin->getScrolledwindowText().gobj()), views);
        GtkWidget* remaining_view{nullptr};
        for (GtkWidget* view : views) {
            if (gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)) == expected_buffer->gobj()) remaining_view = view;
        }
        ASSERT_NE(nullptr, remaining_view);
        EXPECT_EQ(not keep_temporary_view, remaining_view == permanent_view);
        gtk_widget_grab_focus(remaining_view);
        process_gtk_events();
        ASSERT_EQ(remaining.get_node_id(), pWin->curr_tree_iter().get_node_id());
        selection->unselect(pWin->get_tree_store().get_path(keep_temporary_view ? second : first));
        process_gtk_events();
        ASSERT_EQ(1u, pWin->selected_tree_iters().size());
        EXPECT_EQ(permanent_view, GTK_WIDGET(pWin->get_text_view().mm().gobj()));
        ASSERT_EQ(expected_buffer, pWin->curr_buffer());
        EXPECT_EQ(remaining.get_node_id(), pWin->curr_tree_iter().get_node_id());
        int requested_width, requested_height;
        pWin->get_text_view().mm().get_size_request(requested_width, requested_height);
        EXPECT_EQ(-1, requested_height);
        expected_buffer->set_modified(false);
        expected_buffer->insert(expected_buffer->end(), "x");
        const auto& pending = pWin->get_ct_storage()->get_storage_sync_pending()->nodes_to_write_dict;
        const auto found = pending.find(remaining.get_node_id_data_holder());
        EXPECT_TRUE(found != pending.end() and found->second.buff);
        auto erase_from = expected_buffer->end();
        erase_from.backward_char();
        expected_buffer->erase(erase_from, expected_buffer->end());
    }
}
void assert_selection_order_and_shared_deduplication(CtMainWin* pWin,
                                                      const Gtk::TreeModel::iterator& first,
                                                      const Gtk::TreeModel::iterator& second)
{
    const auto selected = pWin->selected_tree_iters();
    ASSERT_EQ(2u, selected.size());
    EXPECT_EQ(pWin->get_tree_store().get_path(first), pWin->get_tree_store().get_path(selected[0]));
    EXPECT_EQ(pWin->get_tree_store().get_path(second), pWin->get_tree_store().get_path(selected[1]));
    CtNodeData shared_data{};
    pWin->get_tree_store().get_node_data(first, shared_data, true);
    shared_data.nodeId = -800;
    shared_data.sharedNodesMasterId = selected[0].get_node_id();
    auto shared_iter = pWin->get_tree_store().append_node(&shared_data);
    auto selection = pWin->get_tree_view().get_selection();
    selection->select(pWin->get_tree_store().get_path(shared_iter));
    process_gtk_events();
    const auto selected_with_alias = pWin->selected_tree_iters();
    ASSERT_EQ(2u, selected_with_alias.size());
    EXPECT_EQ(selected[0].get_node_id_data_holder(), selected_with_alias[0].get_node_id_data_holder());
    EXPECT_EQ(selected[1].get_node_id_data_holder(), selected_with_alias[1].get_node_id_data_holder());
    selection->unselect(pWin->get_tree_store().get_path(shared_iter));
    process_gtk_events();
    pWin->get_tree_store().get_store()->erase(shared_iter);
}
void assert_focused_editor_and_tree_targeting(CtMainWin* pWin,
                                               const Gtk::TreeModel::iterator& second)
{
    std::vector<GtkWidget*> text_views;
    collect_text_views(GTK_WIDGET(pWin->getScrolledwindowText().gobj()), text_views);
    ASSERT_GE(text_views.size(), 2u);
    gtk_widget_grab_focus(text_views.back());
    process_gtk_events();
    EXPECT_EQ(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_views.back())), pWin->curr_buffer()->gobj());
    EXPECT_EQ(pWin->curr_tree_iter().get_node_text_buffer(), pWin->curr_buffer());
    EXPECT_EQ(pWin->get_tree_store().to_ct_tree_iter(second).get_node_id(), pWin->tree_cursor_iter().get_node_id());
    auto active_buffer = pWin->curr_buffer();
    active_buffer->insert(active_buffer->end(), "x");
    auto erase_from = active_buffer->end();
    erase_from.backward_char();
    active_buffer->erase(erase_from, active_buffer->end());
}
void assert_editor_preferences(CtMainWin* pWin)
{
    std::vector<GtkWidget*> text_views;
    collect_text_views(GTK_WIDGET(pWin->getScrolledwindowText().gobj()), text_views);
    ASSERT_GE(text_views.size(), 2u);
    for (GtkWidget* text_view : text_views) {
        EXPECT_TRUE(gtk_source_view_get_show_line_numbers(GTK_SOURCE_VIEW(text_view)));
    }
    for (size_t i = 0; i < text_views.size(); ++i) {
#if GTKMM_MAJOR_VERSION >= 4
        EXPECT_EQ(i + 1 == text_views.size() ? 400 : 0,
                  gtk_text_view_get_bottom_margin(GTK_TEXT_VIEW(text_views[i])));
#else
        EXPECT_EQ(i + 1 != text_views.size(),
                  gtk_style_context_has_class(gtk_widget_get_style_context(text_views[i]),
                                              "ct-view-no-scroll-beyond"));
#endif
    }
}
void assert_repeated_focus_rebuild_and_teardown(CtMainWin* pWin,
                                                 const Gtk::TreeModel::iterator& second)
{
    auto selection = pWin->get_tree_view().get_selection();
    const auto second_path = pWin->get_tree_store().get_path(second);
    for (unsigned round = 0; round < 5; ++round) {
        std::vector<GtkWidget*> text_views;
        collect_text_views(GTK_WIDGET(pWin->getScrolledwindowText().gobj()), text_views);
        ASSERT_GE(text_views.size(), 2u);
        for (GtkWidget* text_view : text_views) {
            gtk_widget_grab_focus(text_view);
            process_gtk_events();
        }
        gtk_widget_grab_focus(GTK_WIDGET(pWin->get_tree_view().gobj()));
        process_gtk_events();
        selection->unselect(second_path);
        process_gtk_events();
        ASSERT_EQ(1u, pWin->selected_tree_iters().size());
        selection->select(second_path);
        process_gtk_events();
        ASSERT_EQ(2u, pWin->selected_tree_iters().size());
    }
}
void assert_large_documents_use_inner_scrollers(CtMainWin* pWin)
{
    Glib::ustring large_text;
    for (int i = 0; i < 1500; ++i) large_text += "line\n";
    std::vector<Gtk::TreeModel::iterator> large_nodes;
    for (int i = 0; i < 2; ++i) {
        CtNodeData node_data{};
        node_data.nodeId = -900 - i;
        node_data.name = "multi-node-large-" + std::to_string(i);
        node_data.syntax = CtConst::PLAIN_TEXT_ID;
        node_data.pTextBuffer = pWin->get_new_text_buffer(i == 0 ? large_text : Glib::ustring{"line\n"});
        large_nodes.push_back(pWin->get_tree_store().append_node(&node_data));
    }
    auto selection = pWin->get_tree_view().get_selection();
    selection->unselect_all();
    for (const auto& iter : large_nodes) selection->select(pWin->get_tree_store().get_path(iter));
    process_gtk_events();
    std::vector<GtkWidget*> text_views;
    collect_text_views(GTK_WIDGET(pWin->getScrolledwindowText().gobj()), text_views);
    ASSERT_EQ(2u, text_views.size());
    EXPECT_FALSE(gtk_scrolled_window_get_overlay_scrolling(pWin->getScrolledwindowText().gobj()));
    for (GtkWidget* text_view : text_views) {
        EXPECT_TRUE(GTK_IS_SCROLLED_WINDOW(gtk_widget_get_parent(text_view)));
        EXPECT_FALSE(gtk_scrolled_window_get_overlay_scrolling(
            GTK_SCROLLED_WINDOW(gtk_widget_get_parent(text_view))));
    }
    selection->unselect_all();
    process_gtk_events();
    EXPECT_EQ(static_cast<bool>(pWin->get_ct_config()->overlayScroll),
              gtk_scrolled_window_get_overlay_scrolling(pWin->getScrolledwindowText().gobj()));
    for (auto iter = large_nodes.rbegin(); iter != large_nodes.rend(); ++iter) {
        pWin->get_tree_store().get_store()->erase(*iter);
    }
}
void assert_large_selection_is_paged(CtMainWin* pWin)
{
    std::vector<Gtk::TreeModel::iterator> temporary_nodes;
    std::vector<Gtk::TreeModel::Path> selected_paths;
    for (int i = 0; i < 30; ++i) {
        CtNodeData node_data{};
        node_data.nodeId = -1000 - i;
        node_data.name = "multi-node-stress-" + std::to_string(i);
        node_data.syntax = CtConst::PLAIN_TEXT_ID;
        node_data.pTextBuffer = pWin->get_new_text_buffer("line\n");
        auto iter = pWin->get_tree_store().append_node(&node_data);
        temporary_nodes.push_back(iter);
        selected_paths.push_back(pWin->get_tree_store().get_path(iter));
    }
    auto selection = pWin->get_tree_view().get_selection();
    selection->unselect_all();
    for (const auto& path : selected_paths) selection->select(path);
    process_gtk_events();
    std::vector<GtkWidget*> text_views;
    collect_text_views(GTK_WIDGET(pWin->getScrolledwindowText().gobj()), text_views);
    EXPECT_EQ(25u, text_views.size());
    EXPECT_EQ(selected_paths.size(), pWin->selected_tree_iters().size());
    GtkWidget* page_label = find_label_with_prefix(GTK_WIDGET(pWin->gobj()), "Showing selected nodes");
    ASSERT_NE(nullptr, page_label);
    EXPECT_TRUE(gtk_widget_get_visible(page_label));
    EXPECT_FALSE(gtk_widget_is_ancestor(page_label, GTK_WIDGET(pWin->getScrolledwindowText().gobj())));
    auto outer_adjustment = pWin->getScrolledwindowText().get_vadjustment();
    outer_adjustment->set_value(outer_adjustment->get_upper() - outer_adjustment->get_page_size());
    process_gtk_events();
    EXPECT_TRUE(gtk_widget_get_visible(page_label));
    for (GtkWidget* text_view : text_views) {
        gtk_widget_grab_focus(text_view);
        process_gtk_events();
    }
    gtk_widget_grab_focus(GTK_WIDGET(pWin->get_tree_view().gobj()));
    process_gtk_events();
    selection->unselect_all();
    process_gtk_events();
    for (auto iter = temporary_nodes.rbegin(); iter != temporary_nodes.rend(); ++iter) {
        pWin->get_tree_store().get_store()->erase(*iter);
    }
}


void assert_multi_node_movement(CtMainWin* pWin, Gtk::TreeModel::iterator original_cursor)
{
    auto& store = pWin->get_tree_store();
    auto& view = pWin->get_tree_view();
    auto& actions = *pWin->get_ct_actions();
    std::vector<gint64> temporary_ids;
    auto cleanup = scope_guard([&](void*) {
        pWin->resetPrevTreeIter();
        view.set_cursor_safe(original_cursor);
        process_gtk_events();
        for (auto id = temporary_ids.rbegin(); id != temporary_ids.rend(); ++id) {
            if (auto iter = store.get_node_from_node_id(*id)) store.get_store()->erase(iter);
        }
        store.pending_rm_db_nodes(temporary_ids);
        store.nodes_sequences_fix({}, true);
    });
    auto add_node = [&](gint64 parent_id = -1, gint64 master_id = 0) {
        CtNodeData data{};
        data.nodeId = store.node_id_get();
        data.name = "movement-test-" + std::to_string(data.nodeId);
        data.syntax = CtConst::PLAIN_TEXT_ID;
        data.pTextBuffer = pWin->get_new_text_buffer("movement\n");
        data.sharedNodesMasterId = master_id;
        Gtk::TreeModel::iterator parent = store.get_node_from_node_id(parent_id);
        data.sequence = (parent ? parent->children().size() : store.get_store()->children().size()) + 1;
        store.append_node(&data, parent ? &parent : nullptr);
        temporary_ids.push_back(data.nodeId);
        return data.nodeId;
    };
    auto children = [&](gint64 parent_id) {
        auto parent = store.get_node_from_node_id(parent_id);
        std::vector<gint64> ids;
        auto rows = parent ? parent->children() : store.get_store()->children();
        for (auto iter = rows.begin(); iter != rows.end(); ++iter) {
            ids.push_back(store.to_ct_tree_iter(iter).get_node_id());
        }
        return ids;
    };
    auto select_nodes = [&](const std::vector<gint64>& ids) {
        pWin->restore_tree_selection(ids, ids.front());
        process_gtk_events();
    };
    const gint64 root = add_node();
    const gint64 a = add_node(root), b = add_node(root), c = add_node(root);
    const gint64 d = add_node(root), e = add_node(root), child = add_node(b);
    const gint64 existing_child = add_node(e);
    const std::vector<gint64> initial{a, b, c, d, e};
    const auto b_buffer = store.get_node_from_node_id(b).get_node_text_buffer();
    store.nodes_sequences_fix({}, true);
    pWin->show();
    // Disjoint roots move one position without changing their relative order.
    select_nodes({b, d});
    const auto& pending = pWin->get_ct_storage()->get_storage_sync_pending()->nodes_to_write_dict;
    for (const auto id : {b, d}) {
        const auto entry = pending.find(id);
        EXPECT_TRUE(entry == pending.end() or not entry->second.hier);
    }
    actions.node_up();
    ASSERT_NE(pending.end(), pending.find(b));
    ASSERT_NE(pending.end(), pending.find(d));
    EXPECT_TRUE(pending.at(b).hier);
    EXPECT_TRUE(pending.at(d).hier);
    EXPECT_EQ((std::vector<gint64>{b, a, d, c, e}), children(root));
    EXPECT_EQ((std::vector<gint64>{b, d}), pWin->selected_tree_node_ids());
    actions.node_down();
    EXPECT_EQ(initial, children(root));
    select_nodes({b, child, c});
    const auto roots = pWin->selected_tree_root_iters();
    ASSERT_EQ(2u, roots.size());
    EXPECT_EQ(b, roots[0].get_node_id());
    EXPECT_EQ(c, roots[1].get_node_id());
    actions.node_up();
    EXPECT_EQ((std::vector<gint64>{b, c, a, d, e}), children(root));
    actions.node_up(); // Already at the upper boundary: the group stays put.
    EXPECT_EQ((std::vector<gint64>{b, c, a, d, e}), children(root));
    actions.node_down();
    EXPECT_EQ(initial, children(root));
    EXPECT_EQ((std::vector<gint64>{b, child, c}), pWin->selected_tree_node_ids());
    actions.node_right();
    EXPECT_EQ((std::vector<gint64>{a, d, e}), children(root));
    EXPECT_EQ((std::vector<gint64>{b, c}), children(a));
    EXPECT_EQ((std::vector<gint64>{child}), children(b));
    EXPECT_EQ(b_buffer, store.get_node_from_node_id(b).get_node_text_buffer());
    EXPECT_EQ((std::vector<gint64>{b, child, c}), pWin->selected_tree_node_ids());
    ASSERT_NE(pending.end(), pending.find(b));
    ASSERT_NE(pending.end(), pending.find(c));
    EXPECT_TRUE(pending.at(b).hier);
    EXPECT_TRUE(pending.at(c).hier);
    actions.node_left();
    EXPECT_EQ(initial, children(root));
    // The same batch operation is used by drag and drop.
    actions.nodes_move_after({b, c}, store.get_node_from_node_id(e), {}, true);
    EXPECT_EQ((std::vector<gint64>{b, c, existing_child}), children(e));
    actions.nodes_move_after({b, c}, store.get_node_from_node_id(root), store.get_node_from_node_id(a));
    EXPECT_EQ(initial, children(root));
    EXPECT_EQ((std::vector<gint64>{existing_child}), children(e));
    // Inserting at root level must not dereference a missing parent iterator.
    actions.nodes_move_after({b, c}, {}, {}, true);
    const auto top_level = children(-1);
    ASSERT_GE(top_level.size(), 2u);
    EXPECT_EQ(b, top_level[0]);
    EXPECT_EQ(c, top_level[1]);
    actions.nodes_move_after({b, c}, store.get_node_from_node_id(root), store.get_node_from_node_id(a));
    EXPECT_EQ(initial, children(root));
    // Shift leaves a collapsed destination closed and does not relocate the cursor.
    view.set_cursor_safe(store.get_node_from_node_id(d));
    auto selection = view.get_selection();
    selection->select(store.get_path(store.get_node_from_node_id(b)));
    selection->select(store.get_path(store.get_node_from_node_id(c)));
    selection->unselect(store.get_path(store.get_node_from_node_id(d)));
    view.collapse_row(store.get_path(store.get_node_from_node_id(e)));
    ASSERT_EQ(d, pWin->tree_cursor_iter().get_node_id());
    actions.nodes_move_after({b, c}, store.get_node_from_node_id(e), {}, false, true);
    process_gtk_events();
    EXPECT_FALSE(view.row_expanded(store.get_path(store.get_node_from_node_id(e))));
    EXPECT_EQ(d, pWin->tree_cursor_iter().get_node_id());
    EXPECT_EQ((std::vector<gint64>{existing_child, b, c}), children(e));
    select_nodes({b, c});
    actions.nodes_move_after({b, c}, store.get_node_from_node_id(root), store.get_node_from_node_id(a));
    EXPECT_EQ(initial, children(root));
    select_nodes({d, e});
    actions.node_down(); // Lower boundary.
    EXPECT_EQ(initial, children(root));
    const gint64 alias = add_node(root, b);
    select_nodes({b, alias});
    EXPECT_EQ(1u, pWin->selected_tree_iters().size());
    EXPECT_EQ(2u, pWin->selected_tree_root_iters().size());
    actions.node_right();
    EXPECT_EQ((std::vector<gint64>{b, alias}), children(a));
    EXPECT_EQ((std::vector<gint64>{child}), children(b));
}

} // namespace

void assert_multi_node_selection(CtMainWin* pWin)
{
    auto first = pWin->get_tree_store().get_iter_first();
    if (not first) return;
    auto second = first;
    ++second;
    if (not second) return;
    const bool show_line_numbers = pWin->get_ct_config()->showLineNumbers;
    const bool scroll_beyond_last_line = pWin->get_ct_config()->scrollBeyondLastLine;
    pWin->get_ct_config()->showLineNumbers = true;
    pWin->set_show_line_numbers(true);
    pWin->get_ct_config()->scrollBeyondLastLine = true;
    pWin->update_theme();
    pWin->set_scroll_beyond_last_line(true);
    static bool movement_scenarios_done{false};
    if (not movement_scenarios_done) {
        movement_scenarios_done = true;
        assert_multi_node_movement(pWin, first);
    }
    assert_single_editor_transition(pWin, first, second);
    select_primary_nodes(pWin, first, second);
    assert_selection_order_and_shared_deduplication(pWin, first, second);
    assert_focused_editor_and_tree_targeting(pWin, second);
    assert_editor_preferences(pWin);
    assert_repeated_focus_rebuild_and_teardown(pWin, second);
    static bool large_selection_stress_done{false};
    if (not large_selection_stress_done) {
        large_selection_stress_done = true;
        assert_large_documents_use_inner_scrollers(pWin);
        assert_large_selection_is_paged(pWin);
    }
    pWin->get_tree_view().set_cursor_safe(first);
    process_gtk_events();
    pWin->get_ct_config()->showLineNumbers = show_line_numbers;
    pWin->set_show_line_numbers(show_line_numbers);
    pWin->get_ct_config()->scrollBeyondLastLine = scroll_beyond_last_line;
    pWin->update_theme();
    pWin->set_scroll_beyond_last_line(scroll_beyond_last_line);
    pWin->hide();
}