/*
 * ct_main_win_events.cc
 *
 * Copyright 2009-2021
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
#include "ct_list.h"

void CtMainWin::_on_treeview_cursor_changed()
{
    CtTreeIter treeIter = curr_tree_iter();
    if (not treeIter) {
        // just removed the last node on the tree?
        _prevTreeIter = treeIter;
        return;
    }
    const gint64 nodeId = treeIter.get_node_id();
    if (_prevTreeIter) {
        const gint64 prevNodeId = _prevTreeIter.get_node_id();
        if (prevNodeId == nodeId) {
            return;
        }
        Glib::RefPtr<Gsv::Buffer> rTextBuffer = _prevTreeIter.get_node_text_buffer();
        if (rTextBuffer->get_modified()) {
            _fileSaveNeeded = true;
            rTextBuffer->set_modified(false);
            _ctStateMachine.update_state(_prevTreeIter);
        }
        _nodesCursorPos[prevNodeId] = rTextBuffer->property_cursor_position();
        _nodesVScrollPos[prevNodeId] = round(_scrolledwindowText.get_vadjustment()->get_value());
    }

    _uCtTreestore->text_view_apply_textbuffer(treeIter, &_ctTextview);

    if (user_active()) {
        auto mapIter = _nodesCursorPos.find(nodeId);
        if (mapIter != _nodesCursorPos.end() and mapIter->second > 0) {
            text_view_apply_cursor_position(treeIter, mapIter->second, _nodesVScrollPos.at(nodeId));
        }
        else {
            text_view_apply_cursor_position(treeIter, 0, 0);
        }

        menu_update_bookmark_menu_item(_uCtTreestore->is_node_bookmarked(nodeId));
        window_header_update();
        window_header_update_lock_icon(treeIter.get_node_read_only());
        window_header_update_bookmark_icon(_uCtTreestore->is_node_bookmarked(nodeId));
        update_selected_node_statusbar_info();
    }

    _ctStateMachine.node_selected_changed(nodeId);

    _prevTreeIter = treeIter;
}

bool CtMainWin::_on_treeview_button_release_event(GdkEventButton* event)
{
    if (event->button == 3) {
        _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(event->button, event->time);
        return true;
    }
    return false;
}

bool CtMainWin::_on_window_key_press_event(GdkEventKey* event)
{
    if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_Tab) {
            _uCtActions->toggle_tree_text();
            return true;
        }
    }
    return false;
}

void CtMainWin::_on_treeview_event_after(GdkEvent* event)
{
    if (event->type == GDK_BUTTON_PRESS and event->button.button == 1) {
        if (_pCtConfig->treeClickFocusText) {
            _ctTextview.grab_focus();
        }
        if (_pCtConfig->treeClickExpand) {
            _tree_just_auto_expanded = false;
            Gtk::TreePath path_at_click;
            if (get_tree_view().get_path_at_pos((int)event->button.x, (int)event->button.y, path_at_click)) {
                if (!get_tree_view().row_expanded(path_at_click)) {
                    get_tree_view().expand_row(path_at_click, false);
                    _tree_just_auto_expanded = true;
                }
            }
        }
    }
    if (event->type == GDK_BUTTON_PRESS && event->button.button == 2 /* wheel click */) {
        auto path = get_tree_store().get_path(curr_tree_iter());
        if (_uCtTreeview->row_expanded(path))
            _uCtTreeview->collapse_row(path);
        else
            _uCtTreeview->expand_row(path, true);
    }
    else if (event->type == GDK_2BUTTON_PRESS and event->button.button == 1) {
        // _on_treeview_row_activated works better for double-click
        // but it doesn't work with one click
        // in this case use real double click
        if (_pCtConfig->treeClickExpand) {
            Gtk::TreePath path_at_click;
            if (get_tree_view().get_path_at_pos((int)event->button.x, (int)event->button.y, path_at_click)) {
                if (path_at_click == get_tree_store().get_path(curr_tree_iter()))
                {
                    if (_uCtTreeview->row_expanded(path_at_click))
                        _uCtTreeview->collapse_row(path_at_click);
                    else
                        _uCtTreeview->expand_row(path_at_click, false);
                }
            }
        }
    }
}

void CtMainWin::_on_treeview_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*)
{
    // _on_treeview_row_activated works better for double-click
    // but it doesn't work with one click
    // in this case use real double click
    if (_pCtConfig->treeClickExpand)
        return;
    if (_uCtTreeview->row_expanded(path))
        _uCtTreeview->collapse_row(path);
    else
        _uCtTreeview->expand_row(path, false);
}

bool CtMainWin::_on_treeview_test_collapse_row(const Gtk::TreeModel::iterator&,const Gtk::TreeModel::Path&)
{
    // to fix one click
    if (_pCtConfig->treeClickExpand) {
        if (_tree_just_auto_expanded) {
            _tree_just_auto_expanded = false;
            return true;
        }
    }
    return false;
}

bool CtMainWin::_on_treeview_key_press_event(GdkEventKey* event)
{
    if (not curr_tree_iter()) return false;
    if (event->state & GDK_SHIFT_MASK) {
        if (event->state & GDK_CONTROL_MASK && event->keyval == GDK_KEY_Right) {
            _uCtActions->node_change_father();
            return true;
        }
        else if (event->keyval == GDK_KEY_Up) {
            _uCtActions->node_up();
            return true;
        }
        else if (event->keyval == GDK_KEY_Down) {
            _uCtActions->node_down();
            return true;
        }
        else if (event->keyval == GDK_KEY_Left) {
            _uCtActions->node_left();
            return true;
        }
        else if (event->keyval == GDK_KEY_Right) {
            _uCtActions->node_right();
            return true;
        }
    }
    else if (event->state & GDK_MOD1_MASK) {
    }
    else if (event->state & GDK_CONTROL_MASK) {
        auto reduce = [](Gtk::TreeIter first, std::function<Gtk::TreeIter(Gtk::TreeIter)> operatr)->Gtk::TreeIter{
            Gtk::TreeIter result;
            for (auto next = operatr(first); next; next = operatr(next))
                result = next;
            return result;
        };
        if (event->keyval == GDK_KEY_Up) {
            auto fist_sibling = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return --iter;});
            if (fist_sibling)
                get_tree_view().set_cursor_safe(fist_sibling);
            return true;
        }
        else if (event->keyval == GDK_KEY_Down) {
            auto last_sibling = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return ++iter;});
            if (last_sibling)
                get_tree_view().set_cursor_safe(last_sibling);
            return true;
        }
        else if (event->keyval == GDK_KEY_Left) {
            auto fist_parent = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return iter->parent();});
            if (fist_parent)
                get_tree_view().set_cursor_safe(fist_parent);
            return true;
        }
        else if (event->keyval == GDK_KEY_Right) {
            auto last_child = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return iter->children().begin();});
            if (last_child)
                get_tree_view().set_cursor_safe(last_child);
            return true;
        }
        else {
            if (event->keyval == GDK_KEY_plus || event->keyval == GDK_KEY_KP_Add || event->keyval == GDK_KEY_equal) {
                _zoom_tree(true);
                return true;
            }
            else if (event->keyval == GDK_KEY_minus|| event->keyval == GDK_KEY_KP_Subtract) {
                _zoom_tree(false);
                return true;
            }
        }
    }
    else {
        if (event->keyval == GDK_KEY_Left) {
            if (_uCtTreeview->row_expanded(_uCtTreestore->get_path(curr_tree_iter())))
                _uCtTreeview->collapse_row(_uCtTreestore->get_path(curr_tree_iter()));
            else if (curr_tree_iter().parent())
                get_tree_view().set_cursor_safe(curr_tree_iter().parent());
            return true;
        }
        else if (event->keyval == GDK_KEY_Right) {
            get_tree_view().expand_row(get_tree_store().get_path(curr_tree_iter()), false);
            return true;
        }
        else if (GDK_KEY_Return == event->keyval or GDK_KEY_KP_Enter == event->keyval) {
            auto path = get_tree_store().get_path(curr_tree_iter());
            if (_uCtTreeview->row_expanded(path))
                _uCtTreeview->collapse_row(path);
            else
                _uCtTreeview->expand_row(path, false);
            return true;
        }
        else if (event->keyval == GDK_KEY_Menu) {
            _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(0, event->time);
            return true;
        }
        else if (event->keyval == GDK_KEY_Tab) {
            _uCtActions->toggle_tree_text();
            return true;
        }
        else if (event->keyval == GDK_KEY_Delete) {
            _uCtActions->node_delete();
            return true;
        }
    }
    return false;
}

bool CtMainWin::_on_treeview_popup_menu()
{
    _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(0, 0);
    return true;
}

bool CtMainWin::_on_treeview_scroll_event(GdkEventScroll* event)
{
    if (!(event->state & GDK_CONTROL_MASK))
        return false;
    if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
        _zoom_tree(event->direction == GDK_SCROLL_DOWN);
    if (event->direction == GDK_SCROLL_SMOOTH && event->delta_y != 0)
        _zoom_tree(event->delta_y < 0);
    return true;
}

// Extend the Default Right-Click Menu
void CtMainWin::_on_textview_populate_popup(Gtk::Menu* menu)
{
    if (curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID) {
        for (auto child: menu->get_children()) {
            if (auto menuItem = dynamic_cast<Gtk::MenuItem*>(child)) {
                if (menuItem->get_label() == "_Paste") {
                    menuItem->set_sensitive(true);
                    break;
                }
            }
        }

        if (hovering_link_iter_offset() >= 0) {
            Gtk::TextIter target_iter = curr_buffer()->get_iter_at_offset(hovering_link_iter_offset());
            if (target_iter) {
                bool do_set_cursor = true;
                if (curr_buffer()->get_has_selection()) {
                    Gtk::TextIter iter_sel_start, iter_sel_end;
                    curr_buffer()->get_selection_bounds(iter_sel_start, iter_sel_end);
                    if (hovering_link_iter_offset() >= iter_sel_start.get_offset()
                        and hovering_link_iter_offset() <= iter_sel_end.get_offset())
                    {
                        do_set_cursor = false;
                    }
                }
                if (do_set_cursor) curr_buffer()->place_cursor(target_iter);
            }
            //for (auto iter : menu->get_children()) menu->remove(*iter);
            get_ct_menu().build_popup_menu(menu, CtMenu::POPUP_MENU_TYPE::Link);
        }
        else {
            //for (auto iter : menu->get_children()) menu->remove(*iter);
            get_ct_menu().build_popup_menu(menu, CtMenu::POPUP_MENU_TYPE::Text);
        }
    }
    else {
        //for (auto iter : menu->get_children()) menu->remove(*iter);
        _uCtActions->getCtMainWin()->get_ct_menu().build_popup_menu(menu, CtMenu::POPUP_MENU_TYPE::Code);
    }
}

// Update the cursor image if the pointer moved
bool CtMainWin::_on_textview_motion_notify_event(GdkEventMotion* event)
{
    if (not _ctTextview.get_cursor_visible())
        _ctTextview.set_cursor_visible(true);
    if (curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID
        and curr_tree_iter().get_node_syntax_highlighting() != CtConst::PLAIN_TEXT_ID)
    {
        _ctTextview.get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::XTERM));
        return false;
    }
    int x, y;
    _ctTextview.window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, (int)event->x, (int)event->y, x, y);
    _ctTextview.cursor_and_tooltips_handler(x, y);
    return false;
}

// Update the cursor image if the window becomes visible (e.g. when a window covering it got iconified)
bool CtMainWin::_on_textview_visibility_notify_event(GdkEventVisibility*)
{
    if (curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID and
        curr_tree_iter().get_node_syntax_highlighting() != CtConst::PLAIN_TEXT_ID)
    {
        _ctTextview.get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::XTERM));
        return false;
    }
    int x,y, bx, by;
    Gdk::ModifierType mask;
    _ctTextview.get_window(Gtk::TEXT_WINDOW_TEXT)->get_pointer(x, y, mask);
    _ctTextview.window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, x, y, bx, by);
    _ctTextview.cursor_and_tooltips_handler(bx, by);
    return false;
}

bool CtMainWin::_on_window_configure_event(GdkEventConfigure*/*configure_event*/)
{
    auto f_update_configs = [this](){
        _pCtConfig->winIsMaximised = is_maximized();
        if (not _pCtConfig->winIsMaximised) {
            get_position(_pCtConfig->winRect[0], _pCtConfig->winRect[1]);
            get_size(_pCtConfig->winRect[2], _pCtConfig->winRect[3]);
        }
    };
    const bool prevWinIsMaximised = _pCtConfig->winIsMaximised;
    f_update_configs();
    if (prevWinIsMaximised and _pCtConfig->winIsMaximised) {
        // when unmaximising, the maximised flag takes a moment to go down
        Glib::signal_idle().connect_once([f_update_configs](){
            f_update_configs();
        });
    }
    return false;
}

void CtMainWin::_on_textview_size_allocate(Gtk::Allocation& allocation)
{
    _pCtConfig->hpanedPos = _hPaned.property_position();
    if (_prevTextviewWidth == 0) {
        _prevTextviewWidth = allocation.get_width();
    }
    else if (_prevTextviewWidth != allocation.get_width()) {
        _prevTextviewWidth = allocation.get_width();
        auto widgets = curr_tree_iter().get_anchored_widgets_fast();
        for (auto& widget: widgets) {
            if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widget)) {
                if (not codebox->get_width_in_pixels()) {
                    codebox->apply_width_height(allocation.get_width());
                }
            }
        }
    }
}

bool CtMainWin::_on_textview_event(GdkEvent* event)
{
    if (event->type != GDK_KEY_PRESS)
        return false;

    auto curr_buffer = _ctTextview.get_buffer();
    if (event->key.state & Gdk::SHIFT_MASK) {
        if (event->key.keyval == GDK_KEY_ISO_Left_Tab and !curr_buffer->get_has_selection()) {
            auto iter_insert = curr_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(this, curr_buffer).get_paragraph_list_info(iter_insert);
            if (list_info and list_info.level) {
                _ctTextview.list_change_level(iter_insert, list_info, false);
                return true;
            }
        }
    }
    else if (event->key.state & Gdk::CONTROL_MASK and event->key.keyval == GDK_KEY_space) {
        if (_try_move_focus_to_anchored_widget_if_on_it()) {
            return true;
        }
        auto iter_insert = _ctTextview.get_buffer()->get_insert()->get_iter();
        CtListInfo list_info = CtList(this, curr_buffer).get_paragraph_list_info(iter_insert);
        if (list_info and list_info.type == CtListType::Todo)
            if (_uCtActions->_is_curr_node_not_read_only_or_error()) {
                auto iter_start_list = curr_buffer->get_iter_at_offset(list_info.startoffs + 3*list_info.level);
                CtList(this, curr_buffer).todo_list_rotate_status(iter_start_list);
                return true;
            }
    }
    else if (GDK_KEY_Return == event->key.keyval or GDK_KEY_KP_Enter == event->key.keyval) {
        auto iter_insert = curr_buffer->get_insert()->get_iter();
        if (iter_insert)
            cursor_key_press() = iter_insert.get_offset();
        else
            cursor_key_press() = -1;
        // print "self.cursor_key_press", self.cursor_key_press
    }
    else if (event->key.keyval == GDK_KEY_Menu) {
        if (curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID) {
            if (not curr_buffer->get_has_selection()) return false;
            Gtk::TextIter iter_sel_start, iter_sel_end;
            curr_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
            int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
            if (num_chars != 1) return false;
            auto widgets = curr_tree_iter().get_anchored_widgets(iter_sel_start.get_offset(), iter_sel_start.get_offset());
            if (widgets.empty()) return false;
            if (CtImageAnchor* anchor = dynamic_cast<CtImageAnchor*>(widgets.front())) {
                _uCtActions->curr_anchor_anchor = anchor;
                _uCtActions->object_set_selection(anchor);
                _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Anchor)->popup(3, event->button.time);
            }
            else if (CtImagePng* image = dynamic_cast<CtImagePng*>(widgets.front())) {
                _uCtActions->curr_image_anchor = image;
                _uCtActions->object_set_selection(image);
                _uCtMenu->find_action("img_link_dismiss")->signal_set_visible.emit(not image->get_link().empty());
                _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Image)->popup(3, event->button.time);
            }
            return true;
        }
    }
    else if (event->key.keyval == GDK_KEY_Tab) {
        if (not curr_buffer->get_has_selection()) {
            auto iter_insert = curr_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(this, curr_buffer).get_paragraph_list_info(iter_insert);
            if (list_info) {
                _ctTextview.list_change_level(iter_insert, list_info, true);
                return true;
            }
        }
        else if (curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID) {
            Gtk::TextIter iter_sel_start, iter_sel_end;
            curr_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
            const int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
            if (num_chars != 1) {
                return false;
            }
            auto widgets = curr_tree_iter().get_anchored_widgets(iter_sel_start.get_offset(), iter_sel_start.get_offset());
            if (widgets.empty()) {
                return false;
            }
            if (dynamic_cast<CtTable*>(widgets.front())) {
                curr_buffer->place_cursor(iter_sel_end);
                _ctTextview.grab_focus();
                return true;
            }
            return false;
        }
    }
    else if (event->key.state & Gdk::CONTROL_MASK) {
        if (event->key.keyval == GDK_KEY_plus || event->key.keyval == GDK_KEY_KP_Add || event->key.keyval == GDK_KEY_equal) {
            _ctTextview.zoom_text(true, curr_tree_iter().get_node_syntax_highlighting());
            return true;
        }
        else if (event->key.keyval == GDK_KEY_minus|| event->key.keyval == GDK_KEY_KP_Subtract) {
            _ctTextview.zoom_text(false, curr_tree_iter().get_node_syntax_highlighting());
            return true;
        }
    }
    return false;
}

// Called after every event on the SourceView
void CtMainWin::_on_textview_event_after(GdkEvent* event)
{
    if (event->type == GDK_2BUTTON_PRESS and event->button.button == 1) {
        _ctTextview.for_event_after_double_click_button1(event);
    }
    if (event->type == GDK_3BUTTON_PRESS and event->button.button == 1) {
        if (curr_tree_iter().get_node_is_rich_text() and _pCtConfig->tripleClickParagraph) {
            if (_ctTextview.get_todo_rotate_time() != event->button.time) {
                _ctTextview.for_event_after_triple_click_button1(event);
            }
        }
    }
    else if (event->type == GDK_BUTTON_PRESS or event->type == GDK_KEY_PRESS) {
        if (event->type == GDK_BUTTON_PRESS) {
            _ctTextview.for_event_after_button_press(event);
        }
        if (event->type == GDK_KEY_PRESS) {
            _ctTextview.for_event_after_key_press(event, curr_tree_iter().get_node_syntax_highlighting());
        }
    }
    else if (event->type == GDK_KEY_RELEASE) {
        if (GDK_KEY_Return == event->key.keyval or GDK_KEY_KP_Enter == event->key.keyval or event->key.keyval == GDK_KEY_space) {
            if (_pCtConfig->wordCountOn) {
                update_selected_node_statusbar_info();
            }
        }
    }
}

bool CtMainWin::_on_textview_scroll_event(GdkEventScroll* event)
{
    if (!(event->state & GDK_CONTROL_MASK))
        return false;
    if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
        _ctTextview.zoom_text(event->direction == GDK_SCROLL_DOWN, curr_tree_iter().get_node_syntax_highlighting());
    if (event->direction == GDK_SCROLL_SMOOTH && event->delta_y != 0)
        _ctTextview.zoom_text(event->delta_y < 0, curr_tree_iter().get_node_syntax_highlighting());
    return true;
}

bool CtMainWin::_on_treeview_drag_motion(const Glib::RefPtr<Gdk::DragContext>& /*context*/,
                                         int x,
                                         int y,
                                         guint /*time*/)
{
    if (y < CtConst::TREE_DRAG_EDGE_PROX or y > (_uCtTreeview->get_allocation().get_height() - CtConst::TREE_DRAG_EDGE_PROX)) {
        const int delta = y < CtConst::TREE_DRAG_EDGE_PROX ? -CtConst::TREE_DRAG_EDGE_SCROLL : CtConst::TREE_DRAG_EDGE_SCROLL;
        Gtk::Scrollbar* vscroll_obj = _scrolledwindowTree.get_vscrollbar();
        vscroll_obj->set_value(vscroll_obj->get_value() + delta);
    }
    if (x < CtConst::TREE_DRAG_EDGE_PROX or x > (_uCtTreeview->get_allocation().get_width() - CtConst::TREE_DRAG_EDGE_PROX)) {
        const int delta = x < CtConst::TREE_DRAG_EDGE_PROX ? -CtConst::TREE_DRAG_EDGE_SCROLL : CtConst::TREE_DRAG_EDGE_SCROLL;
        Gtk::Scrollbar* hscroll_obj = _scrolledwindowTree.get_hscrollbar();
        hscroll_obj->set_value(hscroll_obj->get_value() + delta);
    }
    Gtk::TreePath treePath;
    Gtk::TreeViewDropPosition treeDropPos{Gtk::TREE_VIEW_DROP_BEFORE};
    if (_uCtTreeview->get_dest_row_at_pos(x, y, treePath, treeDropPos)) {
        _uCtTreeview->set_drag_dest_row(treePath, treeDropPos);
    }
    return true;
}

void CtMainWin::_on_treeview_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context,
                                                int x,
                                                int y,
                                                const Gtk::SelectionData& selection_data,
                                                guint /*info*/,
                                                guint time)
{
    auto on_scope_exit = scope_guard([&](void*) { context->drag_finish(false, false, time); });
    Gtk::TreePath treePathDest;
    Gtk::TreeViewDropPosition treeDropPos{Gtk::TREE_VIEW_DROP_BEFORE};
    if (not _uCtTreeview->get_dest_row_at_pos(x, y, treePathDest, treeDropPos)) {
        return;
    }
    const std::string treePathSrcStr = selection_data.get_data_as_string();
    if (treePathSrcStr.empty()) {
        return;
    }
    Gtk::TreePath treePathSrc{treePathSrcStr};
    if (treePathDest == treePathSrc) {
        return;
    }
    CtTreeIter drag_iter = _uCtTreestore->get_iter(treePathSrc);
    if (not drag_iter) {
        return;
    }
    CtTreeIter drop_iter = _uCtTreestore->get_iter(treePathDest);
    if (not drop_iter) {
        return;
    }
    CtTreeIter move_towards_top_iter = drop_iter.parent();
    while (move_towards_top_iter) {
        if (move_towards_top_iter == drag_iter) {
            CtDialogs::error_dialog(_("The new parent can't be one of his children!"), *this);
            return;
        }
        move_towards_top_iter = move_towards_top_iter.parent();
    }
    if (treeDropPos == Gtk::TREE_VIEW_DROP_BEFORE) {
        auto prev_iter = drop_iter;
        --prev_iter;
        // note: prev_iter could be None, use drop_iter to retrieve the parent
        _uCtActions->node_move_after(drag_iter, drop_iter.parent(), prev_iter, true/*set_first*/);
    }
    else if (treeDropPos == Gtk::TREE_VIEW_DROP_AFTER) {
        _uCtActions->node_move_after(drag_iter, drop_iter.parent(), drop_iter);
    }
    else {
        _uCtActions->node_move_after(drag_iter, drop_iter);
    }
}

void CtMainWin::_on_treeview_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& /*context*/,
                                           Gtk::SelectionData& selection_data,
                                           guint /*info*/,
                                           guint /*time*/)
{
    Gtk::TreeIter sel_iter = _uCtTreeview->get_selection()->get_selected();
    const Glib::ustring treePathStr = _uCtTreeview->get_model()->get_path(sel_iter).to_string();
    selection_data.set("UTF8_STRING", 8, (const guint8*)treePathStr.c_str(), (int)treePathStr.size());
}
