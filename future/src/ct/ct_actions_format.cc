/*
 * ct_actions_format.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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
    if (!_is_curr_node_not_read_only_or_error()) return;
    if (CtApp::P_ctCfg->latestTagProp.empty())
        ct_dialogs::warning_dialog(_("No Previous Text Format Was Performed During This Session"), *_pCtMainWin);
    else
        _apply_tag(CtApp::P_ctCfg->latestTagProp, CtApp::P_ctCfg->latestTagVal);
}

// Cleans the Selected Text from All Formatting Tags
void CtActions::remove_text_formatting()
{
    if (!_is_there_selected_node_or_error()) return;
    if (!_is_curr_node_not_syntax_highlighting_or_error()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    auto curr_buffer = _pCtMainWin->get_text_view().get_buffer();
    if (!curr_buffer->get_has_selection() && !CtTextIterUtil::apply_tag_try_automatic_bounds(curr_buffer, curr_buffer->get_insert()->get_iter())) {
        ct_dialogs::warning_dialog(_("No Text is Selected"), *_pCtMainWin);
        return;
    }
    Gtk::TextIter iter_sel_start, iter_sel_end;
    curr_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
    curr_buffer->remove_all_tags(iter_sel_start, iter_sel_end);

    // todo: if self.enable_spell_check: self.spell_check_set_on()
    _pCtMainWin->update_window_save_needed("nbuf", true);
}

// The Foreground Color Chooser Button was Pressed
void CtActions::apply_tag_foreground()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_FOREGROUND);
}

// The Background Color Chooser Button was Pressed
void CtActions::apply_tag_background()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_BACKGROUND);
}

// The Bold Button was Pressed
void CtActions::apply_tag_bold()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
}

// The Italic Button was Pressed
void CtActions::apply_tag_italic()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
}

// The Underline Button was Pressed
void CtActions::apply_tag_underline()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE);
}

// The Strikethrough Button was Pressed
void CtActions::apply_tag_strikethrough()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
}

// The H1 Button was Pressed
void CtActions::apply_tag_h1()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(curr_buffer()).get_paragraph_iters();
    if (!range.iter_start) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H1, range.iter_start, range.iter_end);
}

// The H2 Button was Pressed
void CtActions::apply_tag_h2()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(curr_buffer()).get_paragraph_iters();
    if (!range.iter_start) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H2, range.iter_start, range.iter_end);
}

// The H3 Button was Pressed
void CtActions::apply_tag_h3()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(curr_buffer()).get_paragraph_iters();
    if (!range.iter_start) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3, range.iter_start, range.iter_end);
}

// The Small Button was Pressed
void CtActions::apply_tag_small()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SMALL);
}

// The Superscript Button was Pressed
void CtActions::apply_tag_superscript()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP);
}

// The Subscript Button was Pressed
void CtActions::apply_tag_subscript()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB);
}

// The Monospace Button was Pressed
void CtActions::apply_tag_monospace()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_FAMILY, CtConst::TAG_PROP_VAL_MONOSPACE);
}

// Handler of the Bulleted List
void CtActions::list_bulleted_handler()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (proof.from_codebox || _is_curr_node_not_syntax_highlighting_or_error(true))
        CtList(proof.text_buffer).list_handler(CtListType::Bullet);
}

// Handler of the Numbered List
void CtActions::list_numbered_handler()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (proof.from_codebox || _is_curr_node_not_syntax_highlighting_or_error(true))
        CtList(proof.text_buffer).list_handler(CtListType::Number);
}

// Handler of the ToDo List
void CtActions::list_todo_handler()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (proof.from_codebox || _is_curr_node_not_syntax_highlighting_or_error(true))
        CtList(proof.text_buffer).list_handler(CtListType::Todo);
}

// The Justify Left Button was Pressed
void CtActions::apply_tag_justify_left()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(curr_buffer()).get_paragraph_iters();
    if (!range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT, range.iter_start, range.iter_end);
}

// The Justify Center Button was Pressed
void CtActions::apply_tag_justify_center()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(curr_buffer()).get_paragraph_iters();
    if (!range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_CENTER, range.iter_start, range.iter_end);
}

// The Justify Right Button was Pressed
void CtActions::apply_tag_justify_right()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(curr_buffer()).get_paragraph_iters();
    if (!range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_RIGHT, range.iter_start, range.iter_end);
}

// The Justify Fill Button was Pressed
void CtActions::apply_tag_justify_fill()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    CtTextRange range = CtList(curr_buffer()).get_paragraph_iters();
    if (!range.iter_start) return;
    _apply_tag(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_FILL, range.iter_start, range.iter_end);
}

// Apply a tag
void CtActions::_apply_tag(const Glib::ustring& tag_property, Glib::ustring property_value /*= ""*/,
                std::optional<Gtk::TextIter> iter_sel_start /*= std::nullopt*/,
                std::optional<Gtk::TextIter> iter_sel_end /*= std::nullopt*/,
                Glib::RefPtr<Gtk::TextBuffer> text_buffer /*= Glib::RefPtr<Gtk::TextBuffer>()*/)
{
    if (_pCtMainWin->user_active() && !_is_curr_node_not_syntax_highlighting_or_error()) return;
    if (!text_buffer) text_buffer = curr_buffer();


    if (!iter_sel_start && !iter_sel_end) {
        if (tag_property != CtConst::TAG_JUSTIFICATION) {
            if (!_is_there_selected_node_or_error()) return;
            if (tag_property == CtConst::TAG_LINK)
                _link_entry = ct_dialogs::CtLinkEntry(); // reset
            if (!text_buffer->get_has_selection()) {
                if (tag_property != CtConst::TAG_LINK) {
                    if (!CtTextIterUtil::apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter())) {
                        ct_dialogs::warning_dialog(_("No Text is Selected"), *_pCtMainWin);
                        return;
                    }
                } else {
                    Glib::ustring tag_property_value = _link_check_around_cursor();
                    if (tag_property_value == "") {
                        if (!CtTextIterUtil::apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter())) {
                            Glib::ustring link_name = ct_dialogs::img_n_entry_dialog(*_pCtMainWin, _("Link Name"), "", "link_handle");
                            if (link_name.empty()) return;
                            int start_offset = text_buffer->get_insert()->get_iter().get_offset();
                            text_buffer->insert_at_cursor(link_name);
                            int end_offset = text_buffer->get_insert()->get_iter().get_offset();
                            text_buffer->select_range(text_buffer->get_iter_at_offset(start_offset), text_buffer->get_iter_at_offset(end_offset));
                        }
                        _link_entry.type = CtConst::LINK_TYPE_WEBS; // default value
                    } else {
                        if (!_links_entries_pre_dialog(tag_property_value, _link_entry))
                            return;
                    }
                }
            }
            text_buffer->get_selection_bounds(*iter_sel_start, *iter_sel_end);
         } else {
            ct_dialogs::warning_dialog(_("The Cursor is Not into a Paragraph"), *_pCtMainWin);
            return;
        }
    }
    if (property_value.empty()) {
        if (tag_property == CtConst::TAG_LINK) {
            if (CtTextIterUtil::get_next_chars_from_iter_are(*iter_sel_start, CtConst::WEB_LINK_STARTERS)) {
                _link_entry.type = CtConst::LINK_TYPE_WEBS;
                _link_entry.webs = text_buffer->get_text(*iter_sel_start, *iter_sel_end);
            }
            int insert_offset = iter_sel_start->get_offset();
            int bound_offset = iter_sel_end->get_offset();
            Gtk::TreeIter sel_tree_iter;
            if (_link_entry.node_id != -1)
                sel_tree_iter = _pCtTreestore->get_node_from_node_id(_link_entry.node_id);
            if (!ct_dialogs::link_handle_dialog(*_pCtMainWin, _("Insert/Edit Link"), sel_tree_iter, _link_entry))
                return;
            iter_sel_start = text_buffer->get_iter_at_offset(insert_offset);
            iter_sel_end = text_buffer->get_iter_at_offset(bound_offset);
            property_value = _links_entries_post_dialog(_link_entry);
        } else {
            // todo: assert tag_property[0] in ['f', 'b'], "!! bad tag_property '%s'" % tag_property
            gchar color_for = tag_property[0] == 'f' ? 'f' : 'b';
            Gdk::RGBA ret_color = Gdk::RGBA(CtApp::P_ctCfg->currColors.at(color_for));
            if (!ct_dialogs::color_pick_dialog(*_pCtMainWin, ret_color))
                return;
            CtApp::P_ctCfg->currColors[color_for] = CtRgbUtil::rgb_to_string(ret_color);
            property_value = CtRgbUtil::rgb_to_string(ret_color);
        }
    }
    if (_pCtMainWin->user_active() && tag_property != CtConst::TAG_LINK) {
        CtApp::P_ctCfg->latestTagProp = tag_property;
        CtApp::P_ctCfg->latestTagVal = property_value;
    }
    int sel_start_offset = iter_sel_start->get_offset();
    int sel_end_offset = iter_sel_end->get_offset();
    // if there's already a tag about this property, we remove it before apply the new one
    for (int offset = sel_start_offset; offset < sel_end_offset; ++offset) {
        auto iter_sel_start = text_buffer->get_iter_at_offset(offset);
        std::vector<Glib::RefPtr<Gtk::TextTag>> curr_tags = iter_sel_start.get_tags();
        for (auto& curr_tag : curr_tags) {
            Glib::ustring tag_name = curr_tag->property_name();
            //#print tag_name
            if (tag_name.empty()) continue;
            auto iter_sel_end = text_buffer->get_iter_at_offset(offset+1);
            if ((tag_property == CtConst::TAG_WEIGHT && str::startswith(tag_name, "weight_"))
               || (tag_property == CtConst::TAG_STYLE && str::startswith(tag_name, "style_"))
               || (tag_property == CtConst::TAG_UNDERLINE && str::startswith(tag_name, "underline_"))
               || (tag_property == CtConst::TAG_STRIKETHROUGH && str::startswith(tag_name, "strikethrough_"))
               || (tag_property == CtConst::TAG_FAMILY && str::startswith(tag_name, "family_")))
            {
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
                property_value = ""; // just tag removal
            }
            else if (tag_property == CtConst::TAG_SCALE && str::startswith(tag_name, "scale_"))
            {
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
                // #print property_value, tag_name[6:]
                if (property_value == tag_name.substr(6))
                    property_value = ""; // just tag removal
            }
            else if (tag_property == CtConst::TAG_JUSTIFICATION && str::startswith(tag_name, "justification_"))
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
            else if ((tag_property == CtConst::TAG_FOREGROUND && str::startswith(tag_name, "foreground_"))
               || (tag_property == CtConst::TAG_BACKGROUND && str::startswith(tag_name, "background_"))
               || (tag_property == CtConst::TAG_LINK && str::startswith(tag_name, "link_")))
                text_buffer->remove_tag(curr_tag, iter_sel_start, iter_sel_end);
        }
    }
    if (!property_value.empty())
        text_buffer->apply_tag_by_name(apply_tag_exist_or_create(tag_property, property_value),
                                      text_buffer->get_iter_at_offset(sel_start_offset),
                                      text_buffer->get_iter_at_offset(sel_end_offset));
    if (_pCtMainWin->user_active())
        _pCtMainWin->update_window_save_needed("nbuf", true);
}

// Check into the Tags Table whether the Tag Exists, if Not Creates it
Glib::ustring CtActions::apply_tag_exist_or_create(const Glib::ustring& tag_property, Glib::ustring property_value)
{
    if (property_value == "large")      property_value = CtConst::TAG_PROP_VAL_H1;
    else if (property_value == "largo") property_value = CtConst::TAG_PROP_VAL_H2;
    Glib::ustring tag_name = tag_property + "_" + property_value;
    auto tag = CtApp::R_textTagTable->lookup(tag_name);
    if (!tag) {
        tag = Gtk::TextTag::create(tag_name);
        if (property_value == CtConst::TAG_PROP_VAL_HEAVY)       tag->set_property(tag_property, Pango::WEIGHT_HEAVY);
        else if (property_value == CtConst::TAG_PROP_VAL_SMALL)  tag->set_property(tag_property, Pango::SCALE_SMALL);
        else if (property_value == CtConst::TAG_PROP_VAL_H1)     tag->set_property(tag_property, Pango::SCALE_XX_LARGE);
        else if (property_value == CtConst::TAG_PROP_VAL_H2)     tag->set_property(tag_property, Pango::SCALE_X_LARGE);
        else if (property_value == CtConst::TAG_PROP_VAL_H3)     tag->set_property(tag_property, Pango::SCALE_LARGE);
        else if (property_value == CtConst::TAG_PROP_VAL_ITALIC) tag->set_property(tag_property, Pango::STYLE_ITALIC);
        else if (property_value == CtConst::TAG_PROP_VAL_SINGLE) tag->set_property(tag_property, Pango::UNDERLINE_SINGLE);
        else if (property_value == CtConst::TAG_PROP_VAL_TRUE)   tag->set_property(tag_property, true);
        else if (property_value == CtConst::TAG_PROP_VAL_LEFT)   tag->set_property(tag_property, Gtk::JUSTIFY_LEFT);
        else if (property_value == CtConst::TAG_PROP_VAL_RIGHT)  tag->set_property(tag_property, Gtk::JUSTIFY_RIGHT);
        else if (property_value == CtConst::TAG_PROP_VAL_CENTER) tag->set_property(tag_property, Gtk::JUSTIFY_CENTER);
        else if (property_value == CtConst::TAG_PROP_VAL_FILL)   tag->set_property(tag_property, Gtk::JUSTIFY_FILL);
        else if (property_value == CtConst::TAG_PROP_VAL_MONOSPACE) {
            tag->set_property(tag_property, property_value);
            if (CtApp::P_ctCfg->monospaceBg != "")
                tag->set_property(CtConst::TAG_BACKGROUND, CtApp::P_ctCfg->monospaceBg);
        } else if (property_value == CtConst::TAG_PROP_VAL_SUB) {
            tag->set_property(CtConst::TAG_SCALE, Pango::SCALE_X_SMALL);
            auto rise = Pango::FontDescription(CtApp::P_ctCfg->rtFont).get_size() / -4;
            tag->set_property("rise", rise);
        } else if (property_value == CtConst::TAG_PROP_VAL_SUP) {
            tag->set_property(CtConst::TAG_SCALE, Pango::SCALE_X_SMALL);
            auto rise = Pango::FontDescription(CtApp::P_ctCfg->rtFont).get_size() / 2;
            tag->set_property("rise", rise);
        } else if (str::startswith(property_value, CtConst::LINK_TYPE_WEBS)) {
            if (CtApp::P_ctCfg->linksUnderline)
                tag->set_property(CtConst::TAG_UNDERLINE, Pango::UNDERLINE_SINGLE);
            tag->set_property(CtConst::TAG_FOREGROUND, CtApp::P_ctCfg->colLinkWebs);
        } else if (str::startswith(property_value, CtConst::LINK_TYPE_NODE)) {
            if (CtApp::P_ctCfg->linksUnderline)
                tag->set_property(CtConst::TAG_UNDERLINE, Pango::UNDERLINE_SINGLE);
            tag->set_property(CtConst::TAG_FOREGROUND, CtApp::P_ctCfg->colLinkNode);
        } else if (str::startswith(property_value, CtConst::LINK_TYPE_FILE)) {
            if (CtApp::P_ctCfg->linksUnderline)
                tag->set_property(CtConst::TAG_UNDERLINE, Pango::UNDERLINE_SINGLE);
            tag->set_property(CtConst::TAG_FOREGROUND, CtApp::P_ctCfg->colLinkFile);
        } else if (str::startswith(property_value, CtConst::LINK_TYPE_FOLD)) {
            if (CtApp::P_ctCfg->linksUnderline)
                tag->set_property(CtConst::TAG_UNDERLINE, Pango::UNDERLINE_SINGLE);
            tag->set_property(CtConst::TAG_FOREGROUND, CtApp::P_ctCfg->colLinkFold);
        } else
            tag->set_property(tag_property, property_value);
        CtApp::R_textTagTable->add(tag);
    }
    return tag_name;
}

CtActions::text_view_n_buffer_codebox_proof CtActions::_get_text_view_n_buffer_codebox_proof()
{
    CtCodebox* codebox = _codebox_in_use();
    if (codebox)
        return text_view_n_buffer_codebox_proof{&codebox->getTextView(),
                    codebox->getTextView().get_buffer(),
                    codebox->getSyntaxHighlighting(),
                    true};
    else
        return text_view_n_buffer_codebox_proof{&_pCtMainWin->get_text_view(),
                    _pCtMainWin->get_text_view().get_buffer(),
                    _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting(),
                    false};
}

// Returns a CodeBox SourceView if Currently in Use or None
CtCodebox* CtActions::_codebox_in_use()
{
    if (!curr_codebox_anchor) return nullptr;
    if (!curr_buffer()) return nullptr;
    Gtk::TextIter iter_sel_start = curr_buffer()->get_insert()->get_iter();
    auto widgets = _pCtMainWin->curr_tree_iter().get_embedded_pixbufs_tables_codeboxes({iter_sel_start.get_offset(), iter_sel_start.get_offset()});
    if (widgets.empty()) return nullptr;
    if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widgets.front()))
        return codebox;
    return nullptr;
}

// Prepare Global Links Variables for Dialog
bool CtActions::_links_entries_pre_dialog(const Glib::ustring& curr_link, ct_dialogs::CtLinkEntry& link_entry)
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
        ct_dialogs::error_dialog(str::format("Tag Name Not Recognized! (%s)", std::string(link_entry.type)), *_pCtMainWin);
        link_entry.type = CtConst::LINK_TYPE_WEBS;
        return false;
    }
    return true;
}

// Read Global Links Variables from Dialog
Glib::ustring CtActions::_links_entries_post_dialog(ct_dialogs::CtLinkEntry& link_entry)
{
     Glib::ustring property_value = "";
     if (link_entry.type == CtConst::LINK_TYPE_WEBS) {
         std::string link_url = link_entry.webs;
         if (!link_url.empty()) {
             if (link_url.size() < 8
                || (!str::startswith(link_url, "http://") && !str::startswith(link_url, "https://")))
                link_url = "http://" + link_url;
             property_value = std::string(CtConst::LINK_TYPE_WEBS) + CtConst::CHAR_SPACE + link_url;
         }
    } else if (link_entry.type == CtConst::LINK_TYPE_FILE || link_entry.type == CtConst::LINK_TYPE_FOLD) {
        Glib::ustring link_uri = link_entry.type == CtConst::LINK_TYPE_FILE ? link_entry.file : link_entry.fold;
        if (!link_uri.empty()) {
            link_uri = Glib::Base64::encode(link_uri);
            property_value = link_entry.type + CtConst::CHAR_SPACE + link_uri;
        }
    } else if (link_entry.type == CtConst::LINK_TYPE_NODE) {
        gint64 node_id = link_entry.node_id;
        if (node_id != -1) {
            auto link_anchor = link_entry.anch;
            property_value = std::string(CtConst::LINK_TYPE_NODE) + CtConst::CHAR_SPACE + std::to_string(node_id);
            if (!link_anchor.empty()) property_value += CtConst::CHAR_SPACE + link_anchor;
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
    auto text_iter = curr_buffer()->get_insert()->get_iter();
    Glib::ustring tag_name = link_check_around_cursor_iter(text_iter);
    if (tag_name.empty()) {
        if (text_iter.get_char() == CtConst::CHAR_SPACE[0] && text_iter.backward_char()) {
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
    curr_buffer()->move_mark(curr_buffer()->get_insert(), iter_end);
    curr_buffer()->move_mark(curr_buffer()->get_selection_bound(), text_iter);
    return tag_name.substr(5);
}
