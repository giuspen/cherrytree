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

}

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
