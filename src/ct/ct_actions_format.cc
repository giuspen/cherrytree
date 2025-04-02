/*
 * ct_actions_format.cc
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

#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <glibmm/base64.h>
#include "ct_dialogs.h"
#include "ct_list.h"
#include <optional>

void CtActions::_save_tags_at_cursor_as_latest(Glib::RefPtr<Gtk::TextBuffer> rTextBuffer, int cursorOffset)
{
    std::list<std::string> tagProperties;
    std::list<std::string> tagValues;
    if (cursorOffset < 0) {
        cursorOffset = rTextBuffer->property_cursor_position();
    }
    Gtk::TextIter textIter = rTextBuffer->get_iter_at_offset(cursorOffset);
    std::vector<Glib::RefPtr<Gtk::TextTag>> curr_tags = textIter.get_tags();
    for (auto& curr_tag : curr_tags) {
        Glib::ustring tag_name = curr_tag->property_name();
        if (tag_name.empty() or CtConst::GTKSPELLCHECK_TAG_NAME == tag_name) {
            continue;
        }
        std::pair<std::string, std::string> tagPropNVal;
        if (str::startswith(tag_name, CtConst::TAG_WEIGHT_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_WEIGHT, tag_name.substr(7));
        else if (str::startswith(tag_name, CtConst::TAG_FOREGROUND_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_FOREGROUND, tag_name.substr(11));
        else if (str::startswith(tag_name, CtConst::TAG_BACKGROUND_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_BACKGROUND, tag_name.substr(11));
        else if (str::startswith(tag_name, CtConst::TAG_SCALE_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_SCALE, tag_name.substr(6));
        else if (str::startswith(tag_name, CtConst::TAG_JUSTIFICATION_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_JUSTIFICATION, tag_name.substr(14));
        else if (str::startswith(tag_name, CtConst::TAG_STYLE_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_STYLE, tag_name.substr(6));
        else if (str::startswith(tag_name, CtConst::TAG_UNDERLINE_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_UNDERLINE, tag_name.substr(10));
        else if (str::startswith(tag_name, CtConst::TAG_STRIKETHROUGH_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_STRIKETHROUGH, tag_name.substr(14));
        else if (str::startswith(tag_name, CtConst::TAG_INDENT_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_INDENT, tag_name.substr(7));
        //else if (str::startswith(tag_name, CtConst::TAG_LINK_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_LINK], tag_name.substr(5));
        else if (str::startswith(tag_name, CtConst::TAG_FAMILY_PREFIX)) tagPropNVal = std::make_pair(CtConst::TAG_FAMILY, tag_name.substr(7));
        if (not tagPropNVal.first.empty()) {
            tagProperties.push_back(tagPropNVal.first);
            tagValues.push_back(tagPropNVal.second);
        }
    }
    if (not tagProperties.empty()) {
        _pCtConfig->latestTagProp = str::join(tagProperties, ",");
        _pCtConfig->latestTagVal = str::join(tagValues, ",");
    }
}

void CtActions::save_tags_at_cursor_as_latest()
{
    _save_tags_at_cursor_as_latest(_curr_buffer(), -1);
}

// The Iterate Tagging Button was Pressed
void CtActions::apply_tags_latest()
{
    if (not _is_there_selected_node_or_error()) return;
    if (not _is_curr_node_not_syntax_highlighting_or_error()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (_pCtConfig->latestTagProp.empty()) {
        CtDialogs::warning_dialog(_("No Previous Text Format Was Performed During This Session."), *_pCtMainWin);
    }
    else {
        remove_text_formatting();
        std::vector<std::string> tagProperties = str::split(_pCtConfig->latestTagProp, ",");
        std::vector<std::string> tagValues = str::split(_pCtConfig->latestTagVal, ",");
        for (size_t i = 0; i < tagProperties.size(); ++i) {
            apply_tag(tagProperties.at(i), tagValues.at(i));
        }
    }
}

// Cleans the Selected Text from All Formatting Tags
void CtActions::_remove_text_formatting(const bool dismiss_link)
{
    if (not _is_there_selected_node_or_error()) return;
    if (not _is_curr_node_not_syntax_highlighting_or_error()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    Glib::RefPtr<Gtk::TextBuffer> pTextBuffer = _pCtMainWin->get_text_view().get_buffer();
    if (not pTextBuffer->get_has_selection() and not _pCtMainWin->apply_tag_try_automatic_bounds(pTextBuffer, pTextBuffer->get_insert()->get_iter())) {
        CtDialogs::warning_dialog(_("No Text is Selected."), *_pCtMainWin);
        return;
    }
    CtTreeIter ctTreeIter = _pCtMainWin->curr_tree_iter();
    Gtk::TextIter iter_sel_start, iter_sel_end;
    pTextBuffer->get_selection_bounds(iter_sel_start, iter_sel_end);
    (void)CtTextIterUtil::extend_selection_if_collapsed_text(iter_sel_end, ctTreeIter, _pCtMainWin);

    const int sel_start_offset = iter_sel_start.get_offset();
    const int sel_end_offset = iter_sel_end.get_offset();

    for (int offset = sel_start_offset; offset < sel_end_offset; ++offset) {
        Gtk::TextIter it_sel_start = pTextBuffer->get_iter_at_offset(offset);
        std::vector<Glib::RefPtr<Gtk::TextTag>> curr_tags = it_sel_start.get_tags();
        for (auto& curr_tag : curr_tags) {
            const Glib::ustring tag_name = curr_tag->property_name();
            if ( (not dismiss_link and
                   (str::startswith(tag_name, CtConst::TAG_WEIGHT_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_FOREGROUND_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_BACKGROUND_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_STYLE_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_UNDERLINE_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_STRIKETHROUGH_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_INDENT_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_SCALE_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_INVISIBLE_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_JUSTIFICATION_PREFIX) or
                    str::startswith(tag_name, CtConst::TAG_FAMILY_PREFIX)))
                or
                 (dismiss_link and str::startswith(tag_name, CtConst::TAG_LINK_PREFIX)) )
            {
                Gtk::TextIter it_sel_end = pTextBuffer->get_iter_at_offset(offset+1);
                pTextBuffer->remove_tag(curr_tag, it_sel_start, it_sel_end);
            }
        }
        Glib::RefPtr<Gtk::TextChildAnchor> pChildAnchor = it_sel_start.get_child_anchor();
        if (pChildAnchor) {
            CtAnchoredWidget* pCtAnchoredWidget = ctTreeIter.get_anchored_widget(pChildAnchor);
            if (pCtAnchoredWidget) {
                auto pCtImageAnchor = dynamic_cast<CtImageAnchor*>(pCtAnchoredWidget);
                if (pCtImageAnchor and 0 != CtStrUtil::is_header_anchor_name(pCtImageAnchor->get_anchor_name())) {
                    Gtk::TextIter iter_bound = it_sel_start;
                    iter_bound.forward_char();
                    pTextBuffer->erase(it_sel_start, iter_bound);
                }
            }
        }
    }
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

// The Foreground Color Chooser Button was Pressed
void CtActions::apply_tag_foreground()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_FOREGROUND);
}

// The Background Color Chooser Button was Pressed
void CtActions::apply_tag_background()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_BACKGROUND);
}

// The Bold Button was Pressed
void CtActions::apply_tag_bold()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
}

// The Italic Button was Pressed
void CtActions::apply_tag_italic()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
}

// The Underline Button was Pressed
void CtActions::apply_tag_underline()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE);
}

// The Strikethrough Button was Pressed
void CtActions::apply_tag_strikethrough()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
}

//The Indent button was pressed
void CtActions::apply_tag_indent()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    if (not range.iter_start) return;

    //Each time we increase indent, we'll add this much margin to the text
    int newMargin = _find_previous_indent_margin() + 1;
    apply_tag(CtConst::TAG_INDENT, std::to_string(newMargin), range.iter_start, range.iter_end);
}

//The 'unindent' button was pressed
void CtActions::reduce_tag_indent()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    if (not range.iter_start) return;

    int newMargin = _find_previous_indent_margin() -1;
    if (newMargin < 1) {
        // just remove prev indent tag
        _curr_buffer()->remove_tag_by_name("indent_1", range.iter_start, range.iter_end);
    }
    else {
        apply_tag(CtConst::TAG_INDENT, std::to_string(newMargin), range.iter_start, range.iter_end);
    }
}

//See if there's already an indent tag on the current text, & if so, return its numerical margin.
//If not, return the default "zero margin" (i.e. the margin shown in the UI when there's no indentation)
int CtActions::_find_previous_indent_margin()
{
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    std::vector<Glib::RefPtr<Gtk::TextTag>> curr_tags = range.iter_start.get_tags();
    for (auto& curr_tag : curr_tags) {
        Glib::ustring curr_tag_name = curr_tag->property_name();
        if(str::startswith(curr_tag_name, CtConst::TAG_INDENT_PREFIX)) {
            return std::stoi(curr_tag_name.substr(7, std::string::npos));
        }
    }
    return 0;
}

void CtActions::_apply_tag_hN(const char* tagPropScaleVal)
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    if (not range.iter_start) return;
    apply_tag(CtConst::TAG_SCALE, tagPropScaleVal, range.iter_start, range.iter_end);
}

// The Small Button was Pressed
void CtActions::apply_tag_small()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SMALL);
}

// The Superscript Button was Pressed
void CtActions::apply_tag_superscript()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP);
}

// The Subscript Button was Pressed
void CtActions::apply_tag_subscript()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB);
}

// The Monospace Button was Pressed
void CtActions::apply_tag_monospace()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    apply_tag(CtConst::TAG_FAMILY, CtConst::TAG_PROP_VAL_MONOSPACE);
}

// Handler of the Bulleted List
void CtActions::list_bulleted_handler()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    CtList{_pCtConfig, proof.text_view->get_buffer()}.list_handler(CtListType::Bullet);
}

// Handler of the Numbered List
void CtActions::list_numbered_handler()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    CtList{_pCtConfig, proof.text_view->get_buffer()}.list_handler(CtListType::Number);
}

// Handler of the ToDo List
void CtActions::list_todo_handler()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    CtList{_pCtConfig, proof.text_view->get_buffer()}.list_handler(CtListType::Todo);
}

// The Justify Left Button was Pressed
void CtActions::apply_tag_justify_left()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    if (not range.iter_start) return;
    apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT, range.iter_start, range.iter_end);
}

// The Justify Center Button was Pressed
void CtActions::apply_tag_justify_center()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    if (not range.iter_start) return;
    apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_CENTER, range.iter_start, range.iter_end);
}

// The Justify Right Button was Pressed
void CtActions::apply_tag_justify_right()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    if (not range.iter_start) return;
    apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_RIGHT, range.iter_start, range.iter_end);
}

// The Justify Fill Button was Pressed
void CtActions::apply_tag_justify_fill()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList{_pCtConfig, _curr_buffer()}.get_paragraph_iters();
    if (not range.iter_start) return;
    apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_FILL, range.iter_start, range.iter_end);
}

void CtActions::apply_tag(const Glib::ustring& tag_property,
                          Glib::ustring property_value/*= ""*/,
                          std::optional<Gtk::TextIter> iter_sel_start/*= std::nullopt*/,
                          std::optional<Gtk::TextIter> iter_sel_end/*= std::nullopt*/,
                          Glib::RefPtr<Gtk::TextBuffer> text_buffer/*= Glib::RefPtr<Gtk::TextBuffer>{}*/)
{
    if (_pCtMainWin->user_active() and !_is_curr_node_not_syntax_highlighting_or_error()) return;
    if (not text_buffer) text_buffer = _curr_buffer();

    int restore_cursor_offset = -1;
    if (not iter_sel_start.has_value() or not iter_sel_end.has_value()) {
        if (tag_property != CtConst::TAG_JUSTIFICATION) {
            if (not _is_there_selected_node_or_error()) return;
            if (not text_buffer->get_has_selection()) {
                if (tag_property != CtConst::TAG_LINK) {
                    restore_cursor_offset = text_buffer->get_insert()->get_iter().get_offset();
                    if (not _pCtMainWin->apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter())) {
                        CtDialogs::warning_dialog(_("No Text is Selected."), *_pCtMainWin);
                        return;
                    }
                }
                else {
                    Glib::ustring tag_property_value = _link_check_around_cursor();
                    if (tag_property_value == "") {
                        if (not _pCtMainWin->apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter())) {
                            Glib::ustring link_name = CtDialogs::img_n_entry_dialog(*_pCtMainWin, _("Link Name"), "", "ct_link_handle");
                            if (link_name.empty()) return;
                            int start_offset = text_buffer->get_insert()->get_iter().get_offset();
                            text_buffer->insert_at_cursor(link_name);
                            int end_offset = text_buffer->get_insert()->get_iter().get_offset();
                            text_buffer->select_range(text_buffer->get_iter_at_offset(start_offset),
                                                      text_buffer->get_iter_at_offset(end_offset));
                        }
                    }
                    else {
                        if (not _links_entries_pre_dialog(tag_property_value, _link_entry))
                            return;
                    }
                }
            }
            Gtk::TextIter it_sel_start, it_sel_end;
            text_buffer->get_selection_bounds(it_sel_start, it_sel_end);
            iter_sel_start = it_sel_start;
            iter_sel_end = it_sel_end;
        }
        else {
            CtDialogs::warning_dialog(_("The Cursor is Not into a Paragraph."), *_pCtMainWin);
            return;
        }
    }
    if (not iter_sel_start.has_value() or not iter_sel_end.has_value()) {
        spdlog::error("unexp no iter_sel");
        return;
    }
    if (property_value.empty()) {
        if (tag_property == CtConst::TAG_LINK) {
            if (CtTextIterUtil::startswith_any(*iter_sel_start, CtConst::WEB_LINK_STARTERS)) {
                _link_entry.type = CtConst::LINK_TYPE_WEBS;
                _link_entry.webs = text_buffer->get_text(*iter_sel_start, *iter_sel_end);
            }
            int insert_offset = iter_sel_start->get_offset();
            int bound_offset = iter_sel_end->get_offset();
            Gtk::TreeIter sel_tree_iter;
            if (_link_entry.node_id != -1)
                sel_tree_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(_link_entry.node_id);
            if (not CtDialogs::link_handle_dialog(*_pCtMainWin, _("Insert/Edit Link"), sel_tree_iter, _link_entry))
                return;
            iter_sel_start = text_buffer->get_iter_at_offset(insert_offset);
            iter_sel_end = text_buffer->get_iter_at_offset(bound_offset);
            property_value = _links_entries_post_dialog(_link_entry);
        }
        else {
            Glib::ustring& ret_colour = 'f' == tag_property[0] ? _pCtConfig->currColour_fg : _pCtConfig->currColour_bg;
            const Glib::ustring title = 'f' == tag_property[0] ? _("Pick a Foreground Color") : _("Pick a Background Color");
            const CtDialogs::CtPickDlgState res = CtDialogs::colour_pick_dialog(_pCtMainWin, title, ret_colour, true/*allow_remove_colour*/);
            if (res == CtDialogs::CtPickDlgState::CANCEL) {
                return;
            }
            if (res == CtDialogs::CtPickDlgState::REMOVE_COLOR) {
                property_value = "-"; // don't use empty because `apply prev tag` command brings a color dialog again
            }
            else {
                property_value = ret_colour;
            }
        }
    }
    const int sel_start_offset = iter_sel_start->get_offset();
    const int sel_end_offset = iter_sel_end->get_offset();
    // if there's already a tag about this property, we remove it before apply the new one
    for (int offset = sel_start_offset; offset < sel_end_offset; ++offset) {
        Gtk::TextIter it_sel_start = text_buffer->get_iter_at_offset(offset);
        std::vector<Glib::RefPtr<Gtk::TextTag>> curr_tags = it_sel_start.get_tags();
        for (auto& curr_tag : curr_tags) {
            Glib::ustring curr_tag_name = curr_tag->property_name();
            //#print tag_name
            if (curr_tag_name.empty()) continue;
            Gtk::TextIter it_sel_end = text_buffer->get_iter_at_offset(offset+1);
            if ((tag_property == CtConst::TAG_WEIGHT and str::startswith(curr_tag_name, CtConst::TAG_WEIGHT_PREFIX))
               or (tag_property == CtConst::TAG_STYLE and str::startswith(curr_tag_name, CtConst::TAG_STYLE_PREFIX))
               or (tag_property == CtConst::TAG_UNDERLINE and str::startswith(curr_tag_name, CtConst::TAG_UNDERLINE_PREFIX))
               or (tag_property == CtConst::TAG_STRIKETHROUGH and str::startswith(curr_tag_name, CtConst::TAG_STRIKETHROUGH_PREFIX))
               or (tag_property == CtConst::TAG_FAMILY and str::startswith(curr_tag_name, CtConst::TAG_FAMILY_PREFIX)))
            {
                text_buffer->remove_tag(curr_tag, it_sel_start, it_sel_end);
                property_value.clear(); // just tag removal
            }
            else if (tag_property == CtConst::TAG_INDENT and str::startswith(curr_tag_name, CtConst::TAG_INDENT_PREFIX)){
                //Remove old tag but don't reset the value (since we're increasing previous indent to a new value, not toggling it off)
                text_buffer->remove_tag(curr_tag, it_sel_start, it_sel_end);
            }
            else if (tag_property == CtConst::TAG_SCALE and str::startswith(curr_tag_name, CtConst::TAG_SCALE_PREFIX)) {
                text_buffer->remove_tag(curr_tag, it_sel_start, it_sel_end);
                if (property_value == curr_tag_name.substr(6)) {
                    property_value.clear(); // just tag removal
                }
            }
            else if (tag_property == CtConst::TAG_JUSTIFICATION and str::startswith(curr_tag_name, CtConst::TAG_JUSTIFICATION_PREFIX)) {
                text_buffer->remove_tag(curr_tag, it_sel_start, it_sel_end);
            }
            else if ((tag_property == CtConst::TAG_FOREGROUND and str::startswith(curr_tag_name, CtConst::TAG_FOREGROUND_PREFIX))
                  or (tag_property == CtConst::TAG_BACKGROUND and str::startswith(curr_tag_name, CtConst::TAG_BACKGROUND_PREFIX))
                  or (tag_property == CtConst::TAG_LINK and str::startswith(curr_tag_name, CtConst::TAG_LINK_PREFIX)))
            {
                text_buffer->remove_tag(curr_tag, it_sel_start, it_sel_end);
            }
        }
    }
    // avoid adding invalid color
    if (tag_property == CtConst::TAG_FOREGROUND or tag_property == CtConst::TAG_BACKGROUND) {
        if (property_value == "-") {
            property_value.clear();
        }
    }

    if (not property_value.empty()) {
        text_buffer->apply_tag_by_name(_pCtMainWin->get_text_tag_name_exist_or_create(tag_property, property_value),
                                       text_buffer->get_iter_at_offset(sel_start_offset),
                                       text_buffer->get_iter_at_offset(sel_end_offset));
    }

    if (restore_cursor_offset != -1) { // remove auto selection and restore cursor placement
        text_buffer->place_cursor(text_buffer->get_iter_at_offset(restore_cursor_offset));
    }
    if (_pCtMainWin->user_active()) {
        _save_tags_at_cursor_as_latest(text_buffer, sel_start_offset);
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
    }
}

CtActions::text_view_n_buffer_codebox_proof CtActions::_get_text_view_n_buffer_codebox_proof()
{
    if (auto pCodebox = _codebox_in_use()) {
        return text_view_n_buffer_codebox_proof{
            &pCodebox->get_text_view(),
            pCodebox->get_syntax_highlighting(),
            pCodebox,
            nullptr};
    }
    if (auto pTable = dynamic_cast<CtTableHeavy*>(_table_in_use())) {
        return text_view_n_buffer_codebox_proof{
            &pTable->curr_cell_text_view(),
            CtConst::PLAIN_TEXT_ID,
            nullptr,
            pTable};
    }
    return text_view_n_buffer_codebox_proof{
        &_pCtMainWin->get_text_view(),
        _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting(),
        nullptr,
        nullptr};
}

CtCodebox* CtActions::_codebox_in_use()
{
    if (not curr_codebox_anchor) return nullptr;
    if (not _curr_buffer()) return nullptr;
    Gtk::TextIter iter_sel_start = _curr_buffer()->get_insert()->get_iter();
    auto widgets = _pCtMainWin->curr_tree_iter().get_anchored_widgets(iter_sel_start.get_offset(), iter_sel_start.get_offset());
    if (widgets.empty()) return nullptr;
    if (auto pCtCodebox = dynamic_cast<CtCodebox*>(widgets.front())) {
        return pCtCodebox;
    }
    return nullptr;
}

CtTableCommon* CtActions::_table_in_use()
{
    if (not curr_table_anchor) return nullptr;
    if (not _curr_buffer()) return nullptr;
    Gtk::TextIter iter_sel_start = _curr_buffer()->get_insert()->get_iter();
    auto widgets = _pCtMainWin->curr_tree_iter().get_anchored_widgets(iter_sel_start.get_offset(), iter_sel_start.get_offset());
    if (widgets.empty()) return nullptr;
    if (auto pCtTable = dynamic_cast<CtTableCommon*>(widgets.front())) {
        return pCtTable;
    }
    return nullptr;
}

// Prepare Global Links Variables for Dialog
bool CtActions::_links_entries_pre_dialog(const Glib::ustring& curr_link, CtLinkEntry& link_entry)
{
    const CtLinkEntry new_entry = CtMiscUtil::get_link_entry(curr_link);
    if (new_entry.type.empty()) {
        CtDialogs::error_dialog(str::format("Tag Name Not Recognized! (%s)", str::xml_escape(curr_link)), *_pCtMainWin);
        link_entry.type = CtConst::LINK_TYPE_WEBS;
        return false;
    }
    link_entry = new_entry;
    return true;
}

// Read Global Links Variables from Dialog
Glib::ustring CtActions::_links_entries_post_dialog(CtLinkEntry& link_entry)
{
    Glib::ustring property_value;
    if (link_entry.type == CtConst::LINK_TYPE_WEBS) {
        std::string link_url = link_entry.webs;
        if (not link_url.empty()) {
            if (not str::startswith_url(link_url.c_str())) {
                link_url = "http://" + link_url;
            }
            property_value = CtConst::LINK_TYPE_WEBS + CtConst::CHAR_SPACE + link_url;
        }
    }
    else if (link_entry.type == CtConst::LINK_TYPE_FILE or link_entry.type == CtConst::LINK_TYPE_FOLD) {
        Glib::ustring link_uri = link_entry.type == CtConst::LINK_TYPE_FILE ? link_entry.file : link_entry.fold;
        if (not link_uri.empty()) {
            link_uri = Glib::Base64::encode(link_uri);
            property_value = link_entry.type + CtConst::CHAR_SPACE + link_uri;
        }
    }
    else if (link_entry.type == CtConst::LINK_TYPE_NODE) {
        gint64 node_id = link_entry.node_id;
        if (node_id != -1) {
            auto link_anchor = link_entry.anch;
            property_value = CtConst::LINK_TYPE_NODE + CtConst::CHAR_SPACE + std::to_string(node_id);
            if (not link_anchor.empty()) property_value += CtConst::CHAR_SPACE + link_anchor;
        }
    }
    return property_value;
}

// Check if the cursor is on a link, in this case select the link and return the tag_property_value
Glib::ustring CtActions::_link_check_around_cursor()
{
    auto link_check_around_cursor_iter = [](Gtk::TextIter text_iter)->Glib::ustring{
        auto tags = text_iter.get_tags();
        for (auto& tag : tags) {
            Glib::ustring tag_name = tag->property_name();
            if (str::startswith(tag_name, CtConst::TAG_LINK)) {
                return tag_name;
            }
        }
        return "";
    };
    auto text_iter = _curr_buffer()->get_insert()->get_iter();
    Glib::ustring tag_name = link_check_around_cursor_iter(text_iter);
    if (tag_name.empty()) {
        if (text_iter.get_char() == ' ' and text_iter.backward_char()) {
            tag_name = link_check_around_cursor_iter(text_iter);
            if (tag_name.empty()) return "";
        }
        else {
            return "";
        }
    }
    auto iter_end = text_iter;
    while (iter_end.forward_char()) {
        Glib::ustring ret_tag_name = link_check_around_cursor_iter(iter_end);
        if (ret_tag_name != tag_name) {
            break;
        }
    }
    while (text_iter.backward_char()) {
        Glib::ustring ret_tag_name = link_check_around_cursor_iter(text_iter);
        if (ret_tag_name != tag_name) {
            text_iter.forward_char();
            break;
        }
    }
    if (text_iter == iter_end) return "";
    _curr_buffer()->move_mark(_curr_buffer()->get_insert(), iter_end);
    _curr_buffer()->move_mark(_curr_buffer()->get_selection_bound(), text_iter);
    return tag_name.substr(5);
}
