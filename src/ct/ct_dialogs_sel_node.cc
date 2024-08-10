/*
 * ct_dialogs_cmd_palette.cc
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

#include "ct_dialogs.h"
#include "ct_main_win.h"
#include "spdlog/spdlog.h"
#include <glibconfig.h>

gint64 CtDialogs::dialog_selnode(CtMainWin* pCtMainWin)
{
    // based on plotinus
    struct CtPaletteColumns : public Gtk::TreeModelColumnRecord
    {
        Gtk::TreeModelColumn<int>           order;
        Gtk::TreeModelColumn<gint64> id;
        Gtk::TreeModelColumn<Glib::ustring> path;
        Gtk::TreeModelColumn<Glib::ustring> label;
        CtPaletteColumns() { add(order); add(id); add(path); add(label); }
    } columns;

    Glib::ustring filter;
    std::vector<Glib::ustring> filter_words;

    auto get_command_score = [&](const Gtk::TreeIter& iter) -> int {
        auto label = iter->get_value(columns.label).lowercase();
        auto path = iter->get_value(columns.path).lowercase();
        int score = 0;
        if (str::startswith(label, filter)) return score;
        score++;
        if (label.find(filter) != Glib::ustring::npos) return score;
        score++;
        if (CtStrUtil::contains_words(label, filter_words)) return score;
        score++;
        if (CtStrUtil::contains_words(label, filter_words, false)) return score;
        score++;
        if (CtStrUtil::contains_words(path, filter_words)) return score;
        score++;
        if (CtStrUtil::contains_words(path, filter_words, false)) return score;
         return -1;
    };

    auto list_store = Gtk::ListStore::create(columns);
    auto& treeStore = pCtMainWin->get_tree_store();

    int order_cnt = 0;

    treeStore.get_store()->foreach_iter([&](const Gtk::TreeModel::iterator& iter)
    {
        auto ctit = treeStore.to_ct_tree_iter(iter);
        auto full_path = CtMiscUtil::get_node_hierarchical_name(ctit, " / ", false);
        auto listIter = *list_store->append();
        listIter[columns.order] = ++order_cnt;
        listIter[columns.id] = ctit.get_node_id();
        listIter[columns.path] = full_path;
        listIter[columns.label] = ctit.get_node_name();
        return false;
    });

    auto tree_model_filter = Gtk::TreeModelFilter::create(list_store);
    tree_model_filter->set_visible_func([&](const Gtk::TreeIter& iter) -> bool {
        if (filter.empty()) return true;
        return get_command_score(iter) >= 0;
    });
    auto tree_model_sort = Gtk::TreeModelSort::create(tree_model_filter);
    auto tree_view = Gtk::TreeView();
    tree_view.set_model(tree_model_sort);
    tree_view.set_headers_visible(false);

    // The theme's style context is reliably available only after the widget has been realized
    tree_view.signal_realize().connect([&](){
        auto style_context = tree_view.get_style_context();
        auto text_color = style_context->get_color(Gtk::StateFlags::STATE_FLAG_NORMAL);
        auto selection_color = style_context->get_background_color(Gtk::StateFlags::STATE_FLAG_SELECTED | Gtk::StateFlags::STATE_FLAG_FOCUSED);
        text_color.set_alpha(0.4);

        auto append_column = [&](std::function<Glib::ustring(const Gtk::TreeIter& iter)> markup_function, bool align_right, Gdk::RGBA* text_color, double font_scale = 1) {
            auto cell_renderer = Gtk::manage(new Gtk::CellRendererText());
            if (align_right) cell_renderer->property_xalign() = 1;
            if (text_color != nullptr) cell_renderer->property_foreground_rgba() = *text_color;
            cell_renderer->property_scale() = font_scale;
            auto column = Gtk::manage(new Gtk::TreeViewColumn());
            column->pack_start(*cell_renderer, true);
            column->set_cell_data_func(*cell_renderer, [markup_function](Gtk::CellRenderer* cell, const Gtk::TreeIter& iter){
                ((Gtk::CellRendererText*)cell)->property_markup() = markup_function(iter);
            });
            tree_view.append_column(*column);
        };
        append_column([&](const Gtk::TreeIter& iter) -> Glib::ustring {
            return "  " + CtStrUtil::highlight_words(iter->get_value(columns.path), filter_words) + "  ";
        }, true, &text_color);
        append_column([&](const Gtk::TreeIter& iter) -> Glib::ustring {
            return CtStrUtil::highlight_words(iter->get_value(columns.label), filter_words);
        }, false, nullptr, 1.4);
    });

    auto set_filter = [&] (const Glib::ustring& raw_filter) {
        filter = Glib::Regex::create("/\\s{2,}/")->replace(raw_filter.c_str(), -1, 0, " ");
        filter = str::trim(filter).lowercase();
        filter_words = str::split(filter, " ");

        tree_model_filter->refilter();

        // TreeModelSort has no "resort" method, but reassigning the comparison function forces a resort
        tree_model_sort->set_default_sort_func([&](const Gtk::TreeIter& iter_a, const Gtk::TreeIter& iter_b) {
          // "The sort function used by TreeModelSort is not guaranteed to be stable" (GTK+ documentation),
          // so the original order of commands is needed as a tie-breaker
          int id_difference = iter_a->get_value(columns.order) - iter_b->get_value(columns.order);
          if (filter.empty()) return id_difference;

          int score_difference = get_command_score(iter_a) - get_command_score(iter_b);
          return (score_difference != 0) ? score_difference : id_difference;
        });
    };
    auto scroll_to_selected_item = [&]() {
        if (Gtk::TreeIter selected_iter = tree_view.get_selection()->get_selected()) {
            auto selected_path = tree_view.get_model()->get_path(selected_iter);
            tree_view.scroll_to_row(selected_path);
        }
    };
    auto select_first_item = [&]() {
        if (Gtk::TreeIter iter = tree_view.get_model()->get_iter("0")) {
            tree_view.get_selection()->select(iter);
            scroll_to_selected_item();
        }
    };
    auto select_previous_item = [&]() {
        if (Gtk::TreeIter selected_iter = tree_view.get_selection()->get_selected())
            if (--selected_iter) {
                tree_view.get_selection()->select(selected_iter);
                scroll_to_selected_item();
            }
    };
    auto select_next_item = [&]() {
        if (Gtk::TreeIter selected_iter = tree_view.get_selection()->get_selected())
            if (++selected_iter) {
                tree_view.get_selection()->select(selected_iter);
                scroll_to_selected_item();
            }
    };

    Gtk::Dialog popup_dialog("", *pCtMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    popup_dialog.set_transient_for(*pCtMainWin);
    popup_dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    popup_dialog.set_skip_taskbar_hint(true);
    popup_dialog.set_default_size(-1, 200);
    popup_dialog.set_size_request(-1, 350);

    // Width is determined by the width of the search entry/command list
    auto scrolled_window = Gtk::ScrolledWindow();
    scrolled_window.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);
    popup_dialog.get_content_area()->pack_start(scrolled_window);

    select_first_item();
    tree_view.set_can_focus(false);
    scrolled_window.add(tree_view);

    auto header_bar = Gtk::HeaderBar();
    header_bar.property_spacing() = 0;
    popup_dialog.set_titlebar(header_bar);

    auto search_entry = Gtk::SearchEntry();
    search_entry.property_hexpand() = true;
    //if (Gtk.get_major_version() == 3 && Gtk.get_minor_version() < 22) {
      // GTK+ < 3.22 does not support expanding packed widgets
      // (see https://bugzilla.gnome.org/show_bug.cgi?id=724332)
    //  search_entry.set_size_request(600, -1);
    //}
    search_entry.property_margin() = 4;
    header_bar.set_custom_title(search_entry);
    search_entry.signal_changed().connect([&]() {
        set_filter(search_entry.get_text());
        select_first_item();
    });

    Gtk::TreeIter resulted_iter;
    auto run_command = [&] () {
        if (Gtk::TreeIter iter = tree_view.get_selection()->get_selected()) {
            resulted_iter = iter;
            popup_dialog.close();
        }
    };
    auto entry_activated = search_entry.signal_activate().connect([&]() {
        run_command();
    });
    tree_view.signal_row_activated().connect([&](const Gtk::TreeModel::Path&, Gtk::TreeViewColumn* ) {
        run_command();
    });
    popup_dialog.signal_show().connect([&]() {
        search_entry.grab_focus();
    });
    popup_dialog.signal_key_press_event().connect([&](GdkEventKey* key)->bool {
        if (key->keyval == GDK_KEY_Escape) {
            popup_dialog.close();
            return true;
        } else if (key->keyval == GDK_KEY_Tab || key->keyval == GDK_KEY_ISO_Left_Tab) {
            // Disable Tab and Shift+Tab to prevent navigating focus away from the search entry
            return true;
        } else if (key->keyval == GDK_KEY_Up) {
            select_previous_item();
            return true;
        } else if (key->keyval == GDK_KEY_Down) {
            select_next_item();
            return true;
        }
        return false;
    }, false);
    popup_dialog.show_all();

    // WIN_POS_CENTER_ON_PARENT works badly, so
    int root_x, root_y, height_1, width_1, height_2, width_2;
    pCtMainWin->get_position(root_x, root_y);
    pCtMainWin->get_size(width_1, height_1);
    popup_dialog.get_size(width_2, height_2);
    popup_dialog.move(root_x + (width_1 - width_2) / 2 - 150, root_y + (height_1 - height_2)/2 - 50);

    popup_dialog.run();

    if (resulted_iter)
        return resulted_iter->get_value(columns.id);
    return -1;
}
