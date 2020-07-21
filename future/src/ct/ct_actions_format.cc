/*
 * ct_actions_format.cc
 *
 * Copyright 2009-2020
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


// The Iterate Tagging Button was Pressed
void CtActions::apply_tag_latest()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (_pCtMainWin->get_ct_config()->latestTagProp.empty())
        CtDialogs::warning_dialog(_("No Previous Text Format Was Performed During This Session"), *_pCtMainWin);
    else
        _apply_tag(_pCtMainWin->get_ct_config()->latestTagProp, _pCtMainWin->get_ct_config()->latestTagVal);
}

// Cleans the Selected Text from All Formatting Tags
void CtActions::remove_text_formatting()
{
    if (not _is_there_selected_node_or_error()) return;
    if (not _is_curr_node_not_syntax_highlighting_or_error()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    auto curr_buffer = _pCtMainWin->get_text_view().get_buffer();
    if (not curr_buffer->get_has_selection() and !_pCtMainWin->apply_tag_try_automatic_bounds(curr_buffer, curr_buffer->get_insert()->get_iter())) {
        CtDialogs::warning_dialog(_("No Text is Selected"), *_pCtMainWin);
        return;
    }
    Gtk::TextIter iter_sel_start, iter_sel_end;
    curr_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
    curr_buffer->remove_all_tags(iter_sel_start, iter_sel_end);

    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

// The Foreground Color Chooser Button was Pressed
void CtActions::apply_tag_foreground()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_FOREGROUND);
}

// The Background Color Chooser Button was Pressed
void CtActions::apply_tag_background()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_BACKGROUND);
}

// The Bold Button was Pressed
void CtActions::apply_tag_bold()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
}

// The Italic Button was Pressed
void CtActions::apply_tag_italic()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
}

// The Underline Button was Pressed
void CtActions::apply_tag_underline()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE);
}

// The Strikethrough Button was Pressed
void CtActions::apply_tag_strikethrough()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
}

//The Indent button was pressed
void CtActions::apply_tag_indent()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;

    //Each time we increase indent, we'll add this much margin to the text
    int newMargin = _find_previous_indent_margin() + 1;
    _apply_tag(CtConst::TAG_INDENT, std::to_string(newMargin), range.iter_start, range.iter_end);
}

//The 'unindent' button was pressed
void CtActions::reduce_tag_indent()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;

    int newMargin = _find_previous_indent_margin() -1;
    if (newMargin < 1)
    {
        // just remove prev indent tag
        _curr_buffer()->remove_tag_by_name("indent_1", range.iter_start, range.iter_end);
    }
    else
    {
        _apply_tag(CtConst::TAG_INDENT, std::to_string(newMargin), range.iter_start, range.iter_end);
    }
}

//See if there's already an indent tag on the current text, & if so, return its numerical margin.
//If not, return the default "zero margin" (i.e. the margin shown in the UI when there's no indentation)
int CtActions::_find_previous_indent_margin()
{
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    std::vector<Glib::RefPtr<Gtk::TextTag>> curr_tags = range.iter_start.get_tags();
    for (auto& curr_tag : curr_tags) {
            Glib::ustring curr_tag_name = curr_tag->property_name();
            if(str::startswith(curr_tag_name, "indent_"))
            {
                return std::stoi(curr_tag_name.substr(7, std::string::npos));
            }
    }
    return 0;
}

// The H1 Button was Pressed
void CtActions::apply_tag_h1()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H1, range.iter_start, range.iter_end);
}

// The H2 Button was Pressed
void CtActions::apply_tag_h2()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H2, range.iter_start, range.iter_end);
}

// The H3 Button was Pressed
void CtActions::apply_tag_h3()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3, range.iter_start, range.iter_end);
}

// The Small Button was Pressed
void CtActions::apply_tag_small()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SMALL);
}

// The Superscript Button was Pressed
void CtActions::apply_tag_superscript()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP);
}

// The Subscript Button was Pressed
void CtActions::apply_tag_subscript()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB);
}

// The Monospace Button was Pressed
void CtActions::apply_tag_monospace()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_FAMILY, CtConst::TAG_PROP_VAL_MONOSPACE);
}

// Handler of the Bulleted List
void CtActions::list_bulleted_handler()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (proof.from_codebox or _is_curr_node_not_syntax_highlighting_or_error(true))
        CtList(_pCtMainWin, proof.text_buffer).list_handler(CtListType::Bullet);
}

// Handler of the Numbered List
void CtActions::list_numbered_handler()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (proof.from_codebox or _is_curr_node_not_syntax_highlighting_or_error(true))
        CtList(_pCtMainWin, proof.text_buffer).list_handler(CtListType::Number);
}

// Handler of the ToDo List
void CtActions::list_todo_handler()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (proof.from_codebox or _is_curr_node_not_syntax_highlighting_or_error(true))
        CtList(_pCtMainWin, proof.text_buffer).list_handler(CtListType::Todo);
}

// The Justify Left Button was Pressed
void CtActions::apply_tag_justify_left()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT, range.iter_start, range.iter_end);
}

// The Justify Center Button was Pressed
void CtActions::apply_tag_justify_center()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_CENTER, range.iter_start, range.iter_end);
}

// The Justify Right Button was Pressed
void CtActions::apply_tag_justify_right()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_RIGHT, range.iter_start, range.iter_end);
}

// The Justify Fill Button was Pressed
void CtActions::apply_tag_justify_fill()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(_pCtMainWin, _curr_buffer()).get_paragraph_iters();
    if (not range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_FILL, range.iter_start, range.iter_end);
}

// Apply a tag
void CtActions::_apply_tag(const Glib::ustring& tag_property, Glib::ustring property_value /*= ""*/,
                std::optional<Gtk::TextIter> iter_sel_start /*= std::nullopt*/,
                std::optional<Gtk::TextIter> iter_sel_end /*= std::nullopt*/,
                Glib::RefPtr<Gtk::TextBuffer> text_buffer /*= Glib::RefPtr<Gtk::TextBuffer>()*/)
{
    if (_pCtMainWin->user_active() and !_is_curr_node_not_syntax_highlighting_or_error()) return;
    if (not text_buffer) text_buffer = _curr_buffer();


    if (not iter_sel_start and !iter_sel_end) {
        if (tag_property != CtConst::TAG_JUSTIFICATION) {
            if (not _is_there_selected_node_or_error()) return;
            if (tag_property == CtConst::TAG_LINK)
                _link_entry = CtDialogs::CtLinkEntry(); // reset
            if (not text_buffer->get_has_selection()) {
                if (tag_property != CtConst::TAG_LINK) {
                    if (not _pCtMainWin->apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter())) {
                        CtDialogs::warning_dialog(_("No Text is Selected"), *_pCtMainWin);
                        return;
                    }
                } else {
                    Glib::ustring tag_property_value = _link_check_around_cursor();
                    if (tag_property_value == "") {
                        if (not _pCtMainWin->apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter())) {
                            Glib::ustring link_name = CtDialogs::img_n_entry_dialog(*_pCtMainWin, _("Link Name"), "", "ct_link_handle");
                            if (link_name.empty()) return;
                            int start_offset = text_buffer->get_insert()->get_iter().get_offset();
                            text_buffer->insert_at_cursor(link_name);
                            int end_offset = text_buffer->get_insert()->get_iter().get_offset();
                            text_buffer->select_range(text_buffer->get_iter_at_offset(start_offset), text_buffer->get_iter_at_offset(end_offset));
                        }
                        _link_entry.type = CtConst::LINK_TYPE_WEBS; // default value
                    } else {
                        if (not _links_entries_pre_dialog(tag_property_value, _link_entry))
                            return;
                    }
                }
            }
            text_buffer->get_selection_bounds(*iter_sel_start, *iter_sel_end);
         } else {
            CtDialogs::warning_dialog(_("The Cursor is Not into a Paragraph"), *_pCtMainWin);
            return;
        }
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
        } else {
            // todo: assert tag_property[0] in ['f', 'b'], "!! bad tag_property '%s'" % tag_property
            gchar color_for = tag_property[0] == 'f' ? 'f' : 'b';
            Gdk::RGBA ret_color = Gdk::RGBA(_pCtMainWin->get_ct_config()->currColors.at(color_for));
            if (not CtDialogs::color_pick_dialog(_pCtMainWin, ret_color))
                return;
            _pCtMainWin->get_ct_config()->currColors[color_for] = CtRgbUtil::rgb_to_string(ret_color);
            property_value = CtRgbUtil::rgb_to_string(ret_color);
        }
    }
    if (_pCtMainWin->user_active() and tag_property != CtConst::TAG_LINK) {
        _pCtMainWin->get_ct_config()->latestTagProp = tag_property;
        _pCtMainWin->get_ct_config()->latestTagVal = property_value;
    }
    int sel_start_offset = iter_sel_start->get_offset();
    int sel_end_offset = iter_sel_end->get_offset();
    // if there's already a tag about this property, we remove it before apply the new one
    for (int offset = sel_start_offset; offset < sel_end_offset; ++offset) {
        auto iter_sel_start = text_buffer->get_iter_at_offset(offset);
        std::vector<Glib::RefPtr<Gtk::TextTag>> curr_tags = iter_sel_start.get_tags();
        for (auto& curr_tag : curr_tags) {
            Glib::ustring curr_tag_name = curr_tag->property_name();
            //#print tag_name
            if (curr_tag_name.empty()) continue;
            auto iter_sel_end = text_buffer->get_iter_at_offset(offset+1);
            if ((tag_property == CtConst::TAG_WEIGHT and str::startswith(curr_tag_name, "weight_"))
               or (tag_property == CtConst::TAG_STYLE and str::startswith(curr_tag_name, "style_"))
               or (tag_property == CtConst::TAG_UNDERLINE and str::startswith(curr_tag_name, "underline_"))
               or (tag_property == CtConst::TAG_STRIKETHROUGH and str::startswith(curr_tag_name, "strikethrough_"))
               or (tag_property == CtConst::TAG_FAMILY and str::startswith(curr_tag_name, "family_")))
            {
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
                property_value = ""; // just tag removal
            }
            else if (tag_property == CtConst::TAG_INDENT and str::startswith(curr_tag_name, "indent_")){
                //Remove old tag but don't reset the value (since we're increasing previous indent to a new value, not toggling it off)
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
            }
            else if (tag_property == CtConst::TAG_SCALE and str::startswith(curr_tag_name, "scale_"))
            {
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
                // #print property_value, tag_name[6:]
                if (property_value == curr_tag_name.substr(6))
                    property_value = ""; // just tag removal
            }
            else if (tag_property == CtConst::TAG_JUSTIFICATION and str::startswith(curr_tag_name, "justification_"))
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
            else if ((tag_property == CtConst::TAG_FOREGROUND and str::startswith(curr_tag_name, "foreground_"))
               or (tag_property == CtConst::TAG_BACKGROUND and str::startswith(curr_tag_name, "background_"))
               or (tag_property == CtConst::TAG_LINK and str::startswith(curr_tag_name, "link_")))
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
        }
    }
    if (not property_value.empty())
    {
        text_buffer->apply_tag_by_name(_pCtMainWin->get_text_tag_name_exist_or_create(tag_property, property_value),
                                       text_buffer->get_iter_at_offset(sel_start_offset),
                                       text_buffer->get_iter_at_offset(sel_end_offset));
    }
    if (_pCtMainWin->user_active())
    {
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
    }
}

CtActions::text_view_n_buffer_codebox_proof CtActions::_get_text_view_n_buffer_codebox_proof()
{
    CtCodebox* codebox = _codebox_in_use();
    if (codebox)
        return text_view_n_buffer_codebox_proof{&codebox->get_text_view(),
                    codebox->get_text_view().get_buffer(),
                    codebox->get_syntax_highlighting(),
                    codebox,
                    true};
    else
        return text_view_n_buffer_codebox_proof{&_pCtMainWin->get_text_view(),
                    _pCtMainWin->get_text_view().get_buffer(),
                    _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting(),
                    nullptr,
                    false};
}

// Returns a CodeBox SourceView if Currently in Use or None
CtCodebox* CtActions::_codebox_in_use()
{
    if (not curr_codebox_anchor) return nullptr;
    if (not _curr_buffer()) return nullptr;
    Gtk::TextIter iter_sel_start = _curr_buffer()->get_insert()->get_iter();
    auto widgets = _pCtMainWin->curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(iter_sel_start.get_offset(), iter_sel_start.get_offset());
    if (widgets.empty()) return nullptr;
    if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widgets.front()))
        return codebox;
    return nullptr;
}

// Prepare Global Links Variables for Dialog
bool CtActions::_links_entries_pre_dialog(const Glib::ustring& curr_link, CtDialogs::CtLinkEntry& link_entry)
{
    auto vec = str::split(curr_link, " ");
    link_entry.type = vec[0];
    if (link_entry.type == CtConst::LINK_TYPE_WEBS)        link_entry.webs = vec[1];
    else if (link_entry.type == CtConst::LINK_TYPE_FILE)   link_entry.file = Glib::Base64::decode(vec[1]);
    else if (link_entry.type == CtConst::LINK_TYPE_FOLD)   link_entry.fold = Glib::Base64::decode(vec[1]);
    else if (link_entry.type == CtConst::LINK_TYPE_NODE) {
        link_entry.node_id = std::stol(vec[1]);
        if (vec.size() >= 3) {
            if (vec.size() == 3) link_entry.anch = vec[2];
            else                 link_entry.anch = curr_link.substr(vec[0].size() + vec[1].size() + 2);
        }
    } else {
        CtDialogs::error_dialog(str::format("Tag Name Not Recognized! (%s)", std::string(link_entry.type)), *_pCtMainWin);
        link_entry.type = CtConst::LINK_TYPE_WEBS;
        return false;
    }
    return true;
}

// Read Global Links Variables from Dialog
Glib::ustring CtActions::_links_entries_post_dialog(CtDialogs::CtLinkEntry& link_entry)
{
     Glib::ustring property_value = "";
     if (link_entry.type == CtConst::LINK_TYPE_WEBS) {
         std::string link_url = link_entry.webs;
         if (not link_url.empty()) {
             if (link_url.size() < 8
                or (not str::startswith(link_url, "http://") and not str::startswith(link_url, "https://")))
                link_url = "http://" + link_url;
             property_value = std::string(CtConst::LINK_TYPE_WEBS) + CtConst::CHAR_SPACE + link_url;
         }
    } else if (link_entry.type == CtConst::LINK_TYPE_FILE or link_entry.type == CtConst::LINK_TYPE_FOLD) {
        Glib::ustring link_uri = link_entry.type == CtConst::LINK_TYPE_FILE ? link_entry.file : link_entry.fold;
        if (not link_uri.empty()) {
            link_uri = Glib::Base64::encode(link_uri);
            property_value = link_entry.type + CtConst::CHAR_SPACE + link_uri;
        }
    } else if (link_entry.type == CtConst::LINK_TYPE_NODE) {
        gint64 node_id = link_entry.node_id;
        if (node_id != -1) {
            auto link_anchor = link_entry.anch;
            property_value = std::string(CtConst::LINK_TYPE_NODE) + CtConst::CHAR_SPACE + std::to_string(node_id);
            if (not link_anchor.empty()) property_value += CtConst::CHAR_SPACE + link_anchor;
        }
    }
    return property_value;
}

// Check if the cursor is on a link, in this case select the link and return the tag_property_value
Glib::ustring CtActions::_link_check_around_cursor()
{
    auto link_check_around_cursor_iter = [](Gtk::TextIter text_iter) -> Glib::ustring {
        auto tags = text_iter.get_tags();
        for (auto& tag: tags) {
            Glib::ustring tag_name = tag->property_name();
            if (str::startswith(tag_name, CtConst::TAG_LINK))
                return tag_name;
        }
        return "";
    };
    auto text_iter = _curr_buffer()->get_insert()->get_iter();
    Glib::ustring tag_name = link_check_around_cursor_iter(text_iter);
    if (tag_name.empty()) {
        if (text_iter.get_char() == g_utf8_get_char(CtConst::CHAR_SPACE) and text_iter.backward_char()) {
            tag_name = link_check_around_cursor_iter(text_iter);
            if (tag_name.empty()) return "";
        } else
            return "";
    }
    auto iter_end = text_iter;
    while (iter_end.forward_char()) {
        Glib::ustring ret_tag_name = link_check_around_cursor_iter(iter_end);
        if (ret_tag_name != tag_name)
            break;
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
