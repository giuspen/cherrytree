# -*- coding: utf-8 -*-
#
#       support.py
#
#       Copyright 2009-2017 Giuseppe Penone <giuspen@gmail.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.

from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import Pango
import locale
import os
import webbrowser
import re
import time
from . import cons
from . import config
from . import exports


def get_timestamp_str(timestamp_format, time_float):
    """Get timestamp printable from float"""
    struct_time = time.localtime(time_float)
    try:
        timestamp_str = time.strftime(timestamp_format, struct_time)
    except:
        timestamp_str = time.strftime(config.TIMESTAMP_FORMAT_DEFAULT, struct_time)
    return timestamp_str

def get_word_count(dad):
    if dad.curr_buffer:
        all_text = dad.curr_buffer.get_text(*dad.curr_buffer.get_bounds(), False)
        word_count = len([w for w in all_text.split() if re.search(r"\w", w, re.UNICODE)])
    else:
        word_count = 0
    return word_count

def auto_decode_str(in_str, from_clipboard=False):
    """Try to Detect Encoding and Decode"""
    try:
        import chardet
        enc_dict = chardet.detect(in_str)
        encoding = enc_dict["encoding"]
        encodings = [encoding]
    except:
        encodings = []
    if in_str.startswith("\xEF\xBB\xBF"): # UTF-8 "BOM"
        encodings += ["utf-8-sig"]
    elif in_str.startswith(("\xFF\xFE", "\xFE\xFF")): # UTF-16 BOMs
        encodings += [cons.STR_UTF16]
    elif from_clipboard:
        if "html" in in_str or "HTML" in in_str:
            encodings = [cons.STR_UTF8] + encodings
        else:
            encodings = [cons.STR_UTF16, cons.STR_UTF8] + encodings
    else:
        encodings += [cons.STR_UTF16, "utf-16le", cons.STR_UTF8, cons.STR_ISO_8859, locale.getdefaultlocale()[1]]
    for enc in encodings:
        try:
            out_str = in_str.decode(enc)
            print(enc)
            break
        except: pass
    else:
        out_str = in_str
    return out_str

def apply_tag_try_node_name(dad, iter_start, iter_end):
    """Apply Link to Node Tag if the text is a node name"""
    node_name = dad.curr_buffer.get_text(iter_start, iter_end, False)
    node_dest = dad.get_tree_iter_from_node_name(node_name)
    if node_dest:
        dad.curr_buffer.select_range(iter_start, iter_end)
        property_value = cons.LINK_TYPE_NODE + cons.CHAR_SPACE + str(dad.treestore[node_dest][3])
        dad.apply_tag(cons.TAG_LINK, property_value=property_value)
        return True
    return False

def apply_tag_try_link(dad, iter_end, offset_cursor=None):
    """Try and apply link to previous word (after space or newline)"""
    tag_applied = False
    iter_start = iter_end.copy()
    if iter_start.backward_char() and iter_start.get_char() == cons.CHAR_SQ_BR_CLOSE\
    and iter_start.backward_char() and iter_start.get_char() == cons.CHAR_SQ_BR_CLOSE:
        curr_state = 0
        end_offset = iter_start.get_offset()
        while iter_start.backward_char():
            curr_char = iter_start.get_char()
            if curr_char == cons.CHAR_NEWLINE:
                break
            if curr_char == cons.CHAR_SQ_BR_OPEN:
                if curr_state == 0:
                    curr_state = 1
                else:
                    curr_state = 2
                    break
        if curr_state == 2:
            start_offset = iter_start.get_offset()+2
            end_offset = iter_end.get_offset()-2
            if apply_tag_try_node_name(dad, dad.curr_buffer.get_iter_at_offset(start_offset),
                                       dad.curr_buffer.get_iter_at_offset(end_offset)):
                tag_applied = True
                dad.curr_buffer.delete(dad.curr_buffer.get_iter_at_offset(end_offset),
                                       dad.curr_buffer.get_iter_at_offset(end_offset+2))
                dad.curr_buffer.delete(dad.curr_buffer.get_iter_at_offset(start_offset-2),
                                       dad.curr_buffer.get_iter_at_offset(start_offset))
                if offset_cursor != None:
                    offset_cursor -= 4
    else:
        iter_start = iter_end.copy()
        while iter_start.backward_char():
            curr_char = iter_start.get_char()
            if curr_char in cons.WEB_LINK_SEPARATORS:
                iter_start.forward_char()
                break
        num_chars = iter_end.get_offset() - iter_start.get_offset()
        if num_chars > 4 and get_next_chars_from_iter_are(iter_start, cons.WEB_LINK_STARTERS):
            dad.curr_buffer.select_range(iter_start, iter_end)
            link_url = dad.curr_buffer.get_text(iter_start, iter_end, False)
            if link_url[0:3] not in ["htt", "ftp"]: link_url = "http://" + link_url
            property_value = cons.LINK_TYPE_WEBS + cons.CHAR_SPACE + link_url
            dad.apply_tag(cons.TAG_LINK, property_value=property_value)
            tag_applied = True
        elif num_chars > 2 and get_is_camel_case(iter_start, num_chars):
            if apply_tag_try_node_name(dad, iter_start, iter_end):
                tag_applied = True
    if tag_applied and offset_cursor != None:
        dad.curr_buffer.place_cursor(dad.curr_buffer.get_iter_at_offset(offset_cursor))
    return tag_applied

def apply_tag_try_automatic_bounds(dad, text_buffer=None, iter_start=None):
    """Try to Select a Word Forward/Backward the Cursor"""
    if not text_buffer: text_buffer = dad.curr_buffer
    if not iter_start: iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
    iter_end = iter_start.copy()
    curr_char = iter_end.get_char()
    # 1) select alphanumeric + special
    match = re.match(r'\w', curr_char, re.UNICODE)
    if not match and not curr_char in dad.selword_chars:
        iter_start.backward_char()
        iter_end.backward_char()
        curr_char = iter_end.get_char()
        match = re.match(r'\w', curr_char, re.UNICODE)
        if not match and not curr_char in dad.selword_chars:
            return False
    while match or curr_char in dad.selword_chars:
        if not iter_end.forward_char(): break # end of buffer
        curr_char = iter_end.get_char()
        match = re.match(r'\w', curr_char, re.UNICODE)
    iter_start.backward_char()
    curr_char = iter_start.get_char()
    match = re.match(r'\w', curr_char, re.UNICODE)
    while match or curr_char in dad.selword_chars:
        if not iter_start.backward_char(): break # start of buffer
        curr_char = iter_start.get_char()
        match = re.match(r'\w', curr_char, re.UNICODE)
    if not match and not curr_char in dad.selword_chars: iter_start.forward_char()
    # 2) remove non alphanumeric from borders
    iter_end.backward_char()
    curr_char = iter_end.get_char()
    while curr_char in dad.selword_chars:
        if not iter_end.backward_char(): break # start of buffer
        curr_char = iter_end.get_char()
    iter_end.forward_char()
    curr_char = iter_start.get_char()
    while curr_char in dad.selword_chars:
        if not iter_start.forward_char(): break # end of buffer
        curr_char = iter_start.get_char()
    if iter_end.compare(iter_start) > 0:
        text_buffer.move_mark(text_buffer.get_insert(), iter_start)
        text_buffer.move_mark(text_buffer.get_selection_bound(), iter_end)
        return True
    return False

def on_sourceview_event_after_double_click_button1(dad, text_view, event):
    """Called after every Double Click with button 1"""
    text_buffer = text_view.get_buffer()
    x, y = text_view.window_to_buffer_coords(Gtk.TextWindowType.TEXT, int(event.x), int(event.y))
    iter_start = text_view.get_iter_at_location(x, y)
    apply_tag_try_automatic_bounds(dad, text_buffer=text_buffer, iter_start=iter_start)

def on_sourceview_event_after_button_press(dad, text_view, event):
    """Called after every Gdk.EventType.BUTTON_PRESS on the SourceView"""
    text_buffer = text_view.get_buffer()
    if event.button in [1, 2]:
        x, y = text_view.window_to_buffer_coords(Gtk.TextWindowType.TEXT, int(event.x), int(event.y))
        text_iter = text_view.get_iter_at_location(x, y)
        tags = text_iter.get_tags()
        # check whether we are hovering a link
        if not tags: tags = []
        for tag in tags:
            tag_name = tag.get_property("name")
            if tag_name and tag_name[0:4] == cons.TAG_LINK:
                dad.link_clicked(tag_name[5:], event.button == 2)
                return False
        if dad.lists_handler.is_list_todo_beginning(text_iter):
            if dad.is_curr_node_not_read_only_or_error():
                dad.lists_handler.todo_list_rotate_status(text_iter, text_buffer)
    elif event.button == 3 and not text_buffer.get_has_selection():
        x, y = text_view.window_to_buffer_coords(Gtk.TextWindowType.TEXT, int(event.x), int(event.y))
        text_iter = text_view.get_iter_at_location(x, y)
        text_buffer.place_cursor(text_iter)
    return False

def on_sourceview_list_change_level(dad, iter_insert, list_info, text_buffer, level_increase):
    """Called at list indent/unindent time"""
    if not dad.is_curr_node_not_read_only_or_error(): return
    dad.user_active = False
    end_offset = dad.lists_handler.get_multiline_list_element_end_offset(iter_insert, list_info)
    curr_offset = list_info["startoffs"]
    curr_level = list_info["level"]
    next_level = curr_level+1 if level_increase else curr_level-1
    iter_start = text_buffer.get_iter_at_offset(curr_offset)
    prev_list_info = dad.lists_handler.get_prev_list_info_on_level(iter_start, next_level)
    #print prev_list_info
    if list_info["num"] != 0:
        bull_offset = curr_offset + 3*list_info["level"]
        if list_info["num"] < 0:
            if prev_list_info != None and prev_list_info["num"] < 0:
                bull_idx = prev_list_info["num"]*(-1) - 1
            else:
                idx_old = list_info["num"]*(-1) - 1
                idx_offset = idx_old - curr_level % len(dad.chars_listbul)
                bull_idx = (next_level + idx_offset) % len(dad.chars_listbul)
            dad.replace_text_at_offset(dad.chars_listbul[bull_idx],
                bull_offset, bull_offset+1, text_buffer)
        else:
            idx = list_info["aux"]
            if prev_list_info != None and prev_list_info["num"] > 0:
                this_num = prev_list_info["num"] + 1
                index = prev_list_info["aux"]
            else:
                this_num = 1
                idx_old = list_info["aux"]
                idx_offset = idx_old - curr_level % cons.NUM_CHARS_LISTNUM
                index = (next_level + idx_offset) % cons.NUM_CHARS_LISTNUM
            text_to = str(this_num) + cons.CHARS_LISTNUM[index] + cons.CHAR_SPACE
            dad.replace_text_at_offset(text_to, bull_offset,
                bull_offset+dad.lists_handler.get_leading_chars_num(list_info["num"]), text_buffer)
    iter_start = text_buffer.get_iter_at_offset(curr_offset)
    #print "%s -> %s" % (curr_offset, end_offset)
    while curr_offset < end_offset:
        if level_increase:
            text_buffer.insert(iter_start, 3*cons.CHAR_SPACE)
            end_offset += 3
            iter_start = text_buffer.get_iter_at_offset(curr_offset+3)
        else:
            text_buffer.delete(iter_start, text_buffer.get_iter_at_offset(curr_offset+3))
            end_offset -= 3
            iter_start = text_buffer.get_iter_at_offset(curr_offset+1)
        if not dad.lists_handler.char_iter_forward_to_newline(iter_start) or not iter_start.forward_char():
            break
        curr_offset = iter_start.get_offset()
    dad.user_active = True
    dad.update_window_save_needed("nbuf", True)

def on_sourceview_event_after_scroll(dad, text_view, event):
    """Called after every Gdk.EventType.SCROLL on the SourceView"""
    if dad.ctrl_down:
        if event.direction == Gdk.ScrollDirection.UP:
            dad.zoom_text_p()
        elif event.direction == Gdk.ScrollDirection.DOWN:
            dad.zoom_text_m()
    return False

def on_sourceview_event_after_key_release(dad, text_view, event):
    """Called after every Gdk.EventType.KEY_RELEASE on the SourceView"""
    keyname = Gdk.keyval_name(event.keyval)
    if dad.ctrl_down:
        if keyname in cons.STR_KEYS_CONTROL:
            dad.ctrl_down = False
    elif keyname in [cons.STR_KEY_RETURN, cons.STR_KEY_SPACE]:
        if dad.word_count:
            dad.update_selected_node_statusbar_info()
    return False

def on_sourceview_event_after_key_press(dad, text_view, event, syntax_highl):
    """Called after every Gdk.EventType.KEY_PRESS on the SourceView"""
    text_buffer = text_view.get_buffer()
    keyname = Gdk.keyval_name(event.keyval)
    if not dad.ctrl_down:
        if keyname in cons.STR_KEYS_CONTROL:
            dad.ctrl_down = True
    if dad.auto_smart_quotes and keyname in [cons.STR_KEY_DQUOTE, cons.STR_KEY_SQUOTE]:
        if syntax_highl in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            iter_insert = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            if iter_insert:
                offset_1 = iter_insert.get_offset()-1
                if offset_1 > 0:
                    if keyname == cons.STR_KEY_DQUOTE:
                        start_char = cons.CHAR_DQUOTE
                        char_0 = cons.CHAR_SMART_DQUOTE_0
                        char_1 = cons.CHAR_SMART_DQUOTE_1
                    else:
                        start_char = cons.CHAR_SQUOTE
                        char_0 = cons.CHAR_SMART_SQUOTE_0
                        char_1 = cons.CHAR_SMART_SQUOTE_1
                    iter_start = text_buffer.get_iter_at_offset(offset_1-1)
                    offset_0 = -1
                    while iter_start:
                        curr_char = iter_start.get_char()
                        if curr_char == start_char:
                            candidate_offset = iter_start.get_offset()
                            if not iter_start.backward_char() or iter_start.get_char() in [cons.CHAR_NEWLINE, cons.CHAR_SPACE, cons.CHAR_TAB]:
                                offset_0 = candidate_offset
                            break
                        if curr_char == cons.CHAR_NEWLINE: break
                        if not iter_start.backward_char(): break
                    if offset_0 >= 0:
                        dad.replace_text_at_offset(char_0, offset_0, offset_0+1, text_buffer)
                        dad.replace_text_at_offset(char_1, offset_1, offset_1+1, text_buffer)
    elif (event.get_state()[1] & Gdk.ModifierType.SHIFT_MASK):
        if keyname == cons.STR_KEY_RETURN:
            iter_insert = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            if not iter_insert: return False
            iter_start = iter_insert.copy()
            iter_start.backward_char()
            list_info = dad.lists_handler.get_paragraph_list_info(iter_start)
            if list_info:
                text_buffer.insert(text_buffer.get_iter_at_mark(text_buffer.get_insert()), 3*(1+list_info["level"])*cons.CHAR_SPACE)
    elif keyname in [cons.STR_KEY_RETURN, cons.STR_KEY_SPACE]:
        iter_insert = text_buffer.get_iter_at_mark(text_buffer.get_insert())
        if not iter_insert: return False
        if syntax_highl == cons.RICH_TEXT_ID:
            iter_end_link = iter_insert.copy()
            iter_end_link.backward_char()
            if apply_tag_try_link(dad, iter_end_link, iter_insert.get_offset()):
                iter_insert = text_buffer.get_iter_at_mark(text_buffer.get_insert())
        iter_start = iter_insert.copy()
        if keyname == cons.STR_KEY_RETURN:
            cursor_key_press = iter_insert.get_offset()
            #print "cursor_key_press", cursor_key_press
            if cursor_key_press == dad.cursor_key_press:
                # problem of event-after called twice, once before really executing
                return False
            if not iter_start.backward_char(): return False
            if iter_start.get_char() != cons.CHAR_NEWLINE: return False
            if iter_start.backward_char() and iter_start.get_char() == cons.CHAR_NEWLINE:
                return False # former was an empty row
            list_info = dad.lists_handler.get_paragraph_list_info(iter_start)
            if not list_info:
                if dad.auto_indent:
                    iter_start = iter_insert.copy()
                    former_line_indent = get_former_line_indentation(iter_start)
                    if former_line_indent: text_buffer.insert_at_cursor(former_line_indent)
                return False # former was not a list
            # possible enter on empty list element
            insert_offset = iter_insert.get_offset()
            chars_to_startoffs = 1 + dad.lists_handler.get_leading_chars_num(list_info["num"]) + 3*list_info["level"]
            if (insert_offset - list_info["startoffs"]) == chars_to_startoffs:
                # enter on empty list element
                if list_info["level"] > 0:
                    on_sourceview_list_change_level(dad, iter_insert, list_info, text_buffer, False)
                    iter_insert = text_buffer.get_iter_at_mark(text_buffer.get_insert())
                    iter_list_quit = text_buffer.get_iter_at_offset(iter_insert.get_offset()-1)
                else:
                    iter_list_quit = text_buffer.get_iter_at_offset(list_info["startoffs"])
                text_buffer.delete(iter_list_quit, iter_insert)
                return False
            # list new element
            curr_level = list_info["level"]
            pre_spaces = 3*curr_level*cons.CHAR_SPACE if curr_level else ""
            if list_info["num"] < 0:
                index = list_info["num"]*(-1) - 1
                text_buffer.insert(iter_insert, pre_spaces+dad.chars_listbul[index]+cons.CHAR_SPACE)
            elif list_info["num"] == 0:
                text_buffer.insert(iter_insert, pre_spaces+cons.CHAR_LISTTODO+cons.CHAR_SPACE)
            else:
                new_num = list_info["num"] + 1
                index = list_info["aux"]
                text_buffer.insert(iter_insert, pre_spaces + str(new_num) + cons.CHARS_LISTNUM[index] + cons.CHAR_SPACE)
                new_num += 1
                iter_start = text_buffer.get_iter_at_offset(insert_offset)
                dad.lists_handler.char_iter_forward_to_newline(iter_start)
                list_info = dad.lists_handler.get_next_list_info_on_level(iter_start, curr_level)
                #print list_info
                while list_info and list_info["num"] > 0:
                    iter_start = text_buffer.get_iter_at_offset(list_info["startoffs"])
                    end_offset = dad.lists_handler.get_multiline_list_element_end_offset(iter_start, list_info)
                    iter_end = text_buffer.get_iter_at_offset(end_offset)
                    iter_start, iter_end, chars_rm = dad.lists_handler.list_check_n_remove_old_list_type_leading(iter_start, iter_end, text_buffer)
                    end_offset -= chars_rm
                    text_buffer.insert(iter_start, str(new_num) + cons.CHARS_LISTNUM[index] + cons.CHAR_SPACE)
                    end_offset += dad.lists_handler.get_leading_chars_num(new_num)
                    iter_start = text_buffer.get_iter_at_offset(end_offset)
                    new_num += 1
                    list_info = dad.lists_handler.get_next_list_info_on_level(iter_start, curr_level)
        elif keyname == cons.STR_KEY_SPACE:
            if iter_start.backward_chars(2):
                if iter_start.get_char() == cons.CHAR_GREATER and iter_start.backward_char():
                    if iter_start.get_line_offset() == 0:
                        # at line start
                        if iter_start.get_char() == cons.CHAR_LESSER:
                            # "<> " becoming "◇ "
                            dad.special_char_replace(config.CHARS_LISTBUL_DEFAULT[1], iter_start, iter_insert, text_buffer)
                        elif iter_start.get_char() == cons.CHAR_MINUS:
                            # "-> " becoming "→ "
                            dad.special_char_replace(config.CHARS_LISTBUL_DEFAULT[4], iter_start, iter_insert, text_buffer)
                        elif iter_start.get_char() == cons.CHAR_EQUAL:
                            # "=> " becoming "⇒ "
                            dad.special_char_replace(config.CHARS_LISTBUL_DEFAULT[5], iter_start, iter_insert, text_buffer)
                    elif iter_start.get_char() == cons.CHAR_MINUS and iter_start.backward_char():
                        if iter_start.get_char() == cons.CHAR_LESSER:
                            # "<-> " becoming "↔ "
                            dad.special_char_replace(cons.SPECIAL_CHAR_ARROW_DOUBLE, iter_start, iter_insert, text_buffer)
                        elif iter_start.get_char() == cons.CHAR_MINUS:
                            # "--> " becoming "→ "
                            dad.special_char_replace(cons.SPECIAL_CHAR_ARROW_RIGHT, iter_start, iter_insert, text_buffer)
                    elif iter_start.get_char() == cons.CHAR_EQUAL and iter_start.backward_char():
                        if iter_start.get_char() == cons.CHAR_LESSER:
                            # "<=> " becoming "⇔ "
                            dad.special_char_replace(cons.SPECIAL_CHAR_ARROW_DOUBLE2, iter_start, iter_insert, text_buffer)
                        elif iter_start.get_char() == cons.CHAR_EQUAL:
                            # "==> " becoming "⇒ "
                            dad.special_char_replace(cons.SPECIAL_CHAR_ARROW_RIGHT2, iter_start, iter_insert, text_buffer)
                elif iter_start.get_char() == cons.CHAR_MINUS and iter_start.backward_char()\
                and iter_start.get_char() == cons.CHAR_MINUS and iter_start.backward_char()\
                and iter_start.get_char() == cons.CHAR_LESSER:
                    # "<-- " becoming "← "
                    dad.special_char_replace(cons.SPECIAL_CHAR_ARROW_LEFT, iter_start, iter_insert, text_buffer)
                elif iter_start.get_char() == cons.CHAR_EQUAL and iter_start.backward_char()\
                and iter_start.get_char() == cons.CHAR_EQUAL and iter_start.backward_char()\
                and iter_start.get_char() == cons.CHAR_LESSER:
                    # "<== " becoming "⇐ "
                    dad.special_char_replace(cons.SPECIAL_CHAR_ARROW_LEFT2, iter_start, iter_insert, text_buffer)
                elif iter_start.get_char() == cons.CHAR_PARENTH_CLOSE and iter_start.backward_char():
                    if iter_start.get_char().lower() == "c" and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_PARENTH_OPEN:
                        # "(c) " becoming "© "
                        dad.special_char_replace(cons.SPECIAL_CHAR_COPYRIGHT, iter_start, iter_insert, text_buffer)
                    elif iter_start.get_char().lower() == "r" and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_PARENTH_OPEN:
                        # "(r) " becoming "® "
                        dad.special_char_replace(cons.SPECIAL_CHAR_REGISTERED_TRADEMARK, iter_start, iter_insert, text_buffer)
                    elif iter_start.get_char().lower() == "m" and iter_start.backward_char()\
                    and iter_start.get_char() == "t" and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_PARENTH_OPEN:
                        # "(tm) " becoming "™ "
                        dad.special_char_replace(cons.SPECIAL_CHAR_UNREGISTERED_TRADEMARK, iter_start, iter_insert, text_buffer)
                elif iter_start.get_char() == cons.CHAR_STAR and iter_start.get_line_offset() == 0:
                    # "* " becoming "• " at line start
                    dad.special_char_replace(config.CHARS_LISTBUL_DEFAULT[0], iter_start, iter_insert, text_buffer)
                elif iter_start.get_char() == cons.CHAR_SQ_BR_CLOSE and iter_start.backward_char():
                    if iter_start.get_line_offset() == 0 and iter_start.get_char() == cons.CHAR_SQ_BR_OPEN:
                        # "[] " becoming "☐ " at line start
                        dad.special_char_replace(cons.CHAR_LISTTODO, iter_start, iter_insert, text_buffer)
                elif iter_start.get_char() == cons.CHAR_COLON and iter_start.backward_char():
                    if iter_start.get_line_offset() == 0 and iter_start.get_char() == cons.CHAR_COLON:
                        # ":: " becoming "▪ " at line start
                        dad.special_char_replace(config.CHARS_LISTBUL_DEFAULT[2], iter_start, iter_insert, text_buffer)
    return False

def sourceview_cursor_and_tooltips_handler(dad, text_view, x, y):
    """Looks at all tags covering the position (x, y) in the text view,
       and if one of them is a link, change the cursor to the HAND2 cursor"""
    hovering_link_iter_offset = -1
    text_iter = text_view.get_iter_at_location(x, y)
    if dad.lists_handler.is_list_todo_beginning(text_iter):
        text_view.get_window(Gtk.TextWindowType.TEXT).set_cursor(Gdk.Cursor.new(Gdk.CursorType.X_CURSOR))
        text_view.set_tooltip_text(None)
        return
    tags = text_iter.get_tags()
    if not tags: tags = []
    for tag in tags:
        tag_name = tag.get_property("name")
        if tag_name and tag_name[0:4] == cons.TAG_LINK:
            hovering_link_iter_offset = text_iter.get_offset()
            tooltip = dad.sourceview_hovering_link_get_tooltip(tag_name[5:])
            break
    else:
        iter_anchor = text_iter.copy()
        for i in [0, 1]:
            if i == 1: iter_anchor.backward_char()
            anchor = iter_anchor.get_child_anchor()
            if anchor and "pixbuf" in dir(anchor):
                pixbuf_attrs = dir(anchor.pixbuf)
                if "link" in pixbuf_attrs and anchor.pixbuf.link:
                    hovering_link_iter_offset = text_iter.get_offset()
                    tooltip = dad.sourceview_hovering_link_get_tooltip(anchor.pixbuf.link)
                    break
    if dad.hovering_link_iter_offset != hovering_link_iter_offset:
        dad.hovering_link_iter_offset = hovering_link_iter_offset
        #print "link", dad.hovering_link_iter_offset
    if dad.hovering_link_iter_offset >= 0:
        text_view.get_window(Gtk.TextWindowType.TEXT).set_cursor(Gdk.Cursor.new(Gdk.CursorType.HAND2))
        if len(tooltip) > cons.MAX_TOOLTIP_LINK_CHARS:
            tooltip = tooltip[:cons.MAX_TOOLTIP_LINK_CHARS] + "..."
        text_view.set_tooltip_text(tooltip)
    else:
        text_view.get_window(Gtk.TextWindowType.TEXT).set_cursor(Gdk.Cursor.new(Gdk.CursorType.XTERM))
        text_view.set_tooltip_text(None)

def rich_text_node_modify_codeboxes_font(start_iter, code_font):
    """Modify Font to CodeBoxes"""
    curr_iter = start_iter.copy()
    while 1:
        anchor = curr_iter.get_child_anchor()
        if anchor and "sourcebuffer" in dir(anchor):
            anchor.sourceview.modify_font(Pango.FontDescription(code_font))
        if not curr_iter.forward_char(): break

def rich_text_node_modify_codeboxes_color(start_iter, dad):
    """Modify Color to CodeBoxes"""
    curr_iter = start_iter.copy()
    while 1:
        anchor = curr_iter.get_child_anchor()
        if anchor and "sourcebuffer" in dir(anchor):
            dad.widget_set_colors(anchor.sourceview, dad.rt_def_fg, dad.rt_def_bg, True)
        if not curr_iter.forward_char(): break

def text_file_rm_emptylines(filepath):
    """Remove empty lines in a text file"""
    overwrite_needed = False
    fd = open(filepath, 'r')
    file_lines = []
    for file_line in fd:
        if file_line not in ["\r\n", "\n\r", cons.CHAR_NEWLINE]:
            file_lines.append(file_line)
        else: overwrite_needed = True
    fd.close()
    if overwrite_needed:
        print(filepath, "empty lines removed")
        fd = open(filepath, 'w')
        fd.writelines(file_lines)
        fd.close()

def get_proper_platform_filepath(filepath_in, is_file):
    """From Slash to Backslash when needed"""
    filepath_out = filepath_in
    if cons.IS_WIN_OS:
        if cons.CHAR_SLASH in filepath_in:
            filepath_out = filepath_in.replace(cons.CHAR_SLASH, cons.CHAR_BSLASH)
    else:
        if cons.CHAR_BSLASH in filepath_in:
            filepath_out = filepath_in.replace(cons.CHAR_BSLASH, cons.CHAR_SLASH)
    return filepath_out

def clean_from_chars_not_for_filename(filename_in):
    """Clean a string from chars not good for filename"""
    filename_out = filename_in.replace(cons.CHAR_SLASH, cons.CHAR_MINUS).replace(cons.CHAR_BSLASH, cons.CHAR_MINUS)
    filename_out = filename_out.replace(cons.CHAR_STAR, "").replace(cons.CHAR_QUESTION, "").replace(cons.CHAR_COLON, "")
    filename_out = filename_out.replace(cons.CHAR_LESSER, "").replace(cons.CHAR_GREATER, "")
    filename_out = filename_out.replace(cons.CHAR_PIPE, "").replace(cons.CHAR_DQUOTE, "")
    filename_out = filename_out.replace(cons.CHAR_NEWLINE, "").replace(cons.CHAR_CR, "").strip()
    return filename_out.replace(cons.CHAR_SPACE, cons.CHAR_USCORE)

def get_node_hierarchical_name(dad, tree_iter, separator="--", for_filename=True, root_to_leaf=True):
    """Get the Node Hierarchical Name"""
    hierarchical_name = dad.treestore[tree_iter][1].strip()
    father_iter = dad.treestore.iter_parent(tree_iter)
    while father_iter:
        if root_to_leaf:
            hierarchical_name = dad.treestore[father_iter][1].strip() + separator + hierarchical_name
        else:
            hierarchical_name = hierarchical_name + separator + dad.treestore[father_iter][1].strip()
        father_iter = dad.treestore.iter_parent(father_iter)
    if for_filename:
        hierarchical_name = clean_from_chars_not_for_filename(hierarchical_name)
        if len(hierarchical_name) > cons.MAX_FILE_NAME_LEN:
            hierarchical_name = hierarchical_name[-cons.MAX_FILE_NAME_LEN:]
    return hierarchical_name

def strip_trailing_spaces(text_buffer):
    """Remove trailing spaces/tabs"""
    cleaned_lines = 0
    removed_something = True
    while removed_something:
        removed_something = False
        curr_iter = text_buffer.get_start_iter()
        curr_state = 0
        start_offset = 0
        while curr_iter:
            curr_char = curr_iter.get_char()
            if curr_state == 0:
                if curr_char in [cons.CHAR_SPACE, cons.CHAR_TAB]:
                    start_offset = curr_iter.get_offset()
                    curr_state = 1
            elif curr_state == 1:
                if curr_char == cons.CHAR_NEWLINE:
                    text_buffer.delete(text_buffer.get_iter_at_offset(start_offset), curr_iter)
                    removed_something = True
                    cleaned_lines += 1
                    break
                elif not curr_char in [cons.CHAR_SPACE, cons.CHAR_TAB]:
                    curr_state = 0
            if not curr_iter.forward_char():
                if curr_state == 1:
                    text_buffer.delete(text_buffer.get_iter_at_offset(start_offset), curr_iter)
                break
    return cleaned_lines

def get_is_camel_case(iter_start, num_chars):
    """Returns True if the characters compose a camel case word"""
    text_iter = iter_start.copy()
    curr_state = 0
    for i in range(num_chars):
        curr_char = text_iter.get_char()
        alphanumeric = re.match(r'\w', curr_char, re.UNICODE)
        if not alphanumeric:
            curr_state = -1
            break
        if curr_state == 0:
            if curr_char.islower():
                curr_state = 1
        elif curr_state == 1:
            if curr_char.isupper():
                curr_state = 2
        elif curr_state == 2:
            if curr_char.islower():
                curr_state = 3
        else:
            pass
        text_iter.forward_char()
    return curr_state == 3

def get_next_chars_from_iter_are(iter_start, chars_list):
    """Returns True if one set of the Given Chars are the first after iter"""
    for chars in chars_list:
        text_iter = iter_start.copy()
        num = len(chars)
        for i in range(num):
            if text_iter.get_char() != chars[i]:
                break
            if i != num-1 and not text_iter.forward_char():
                break
        else:
            return True
    return False

def get_first_chars_of_string_are(in_string, chars_list):
    """Returns True if one set of the Given Chars are the first of in_string"""
    for chars in chars_list:
        if in_string.startswith(chars):
            return True
    return False

def get_first_chars_of_string_at_offset_are(in_string, offset, chars_list):
    """Returns True if one set of the Given Chars are the first of in_string"""
    for chars in chars_list:
        num = len(chars)
        for i in range(num):
            if in_string[offset+i] != chars[i]:
                break
        else: return True
    return False

def get_former_line_indentation(iter_start):
    """Returns the indentation of the former paragraph or empty string"""
    if not iter_start.backward_chars(2) or iter_start.get_char() == cons.CHAR_NEWLINE: return ""
    buffer_start = False
    while iter_start:
        if iter_start.get_char() == cons.CHAR_NEWLINE: break # we got the previous paragraph start
        elif not iter_start.backward_char():
            buffer_start = True
            break # we reached the buffer start
    if not buffer_start: iter_start.forward_char()
    if iter_start.get_char() == cons.CHAR_SPACE:
        num_spaces = 1
        while iter_start.forward_char() and iter_start.get_char() == cons.CHAR_SPACE:
            num_spaces += 1
        return num_spaces*cons.CHAR_SPACE
    if iter_start.get_char() == cons.CHAR_TAB:
        num_tabs = 1
        while iter_start.forward_char() and iter_start.get_char() == cons.CHAR_TAB:
            num_tabs += 1
        return num_tabs*cons.CHAR_TAB
    return ""

def get_pango_weight(is_bold):
    """Get pango weight (integer 200:900) heavy=900, normal=400"""
    return Pango.Weight.HEAVY if is_bold else Pango.Weight.NORMAL

def get_pango_is_bold(weight):
    """Get True if pango weight is bold (heavy)"""
    return weight == Pango.Weight.HEAVY

def windows_cmd_prepare_path(filepath):
    """Prepares a Path to be digested by windows command line"""
    return cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE

def dialog_file_save_as(filename=None, filter_pattern=None, filter_name=None, curr_folder=None, parent=None):
    """The Save file as dialog, Returns the retrieved filepath or None"""
    chooser = Gtk.FileChooserDialog(title=_("Save File as"),
                                    action=Gtk.FileChooserAction.SAVE,
                                    buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_SAVE, Gtk.ResponseType.ACCEPT) )
    chooser.set_do_overwrite_confirmation(True)
    if parent != None:
        chooser.set_transient_for(parent)
        chooser.set_property("modal", True)
        chooser.set_property("destroy-with-parent", True)
        chooser.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    else: chooser.set_position(Gtk.WindowPosition.CENTER)
    if curr_folder == None or os.path.isdir(curr_folder) == False:
        chooser.set_current_folder(os.path.expanduser('~'))
    else:
        chooser.set_current_folder(curr_folder)
    if filename != None:
        chooser.set_current_name(filename)
    if filter_pattern != None:
        filter = Gtk.FileFilter()
        filter.set_name(filter_name)
        filter.add_pattern(filter_pattern)
        chooser.add_filter(filter)
    if chooser.run() == Gtk.ResponseType.ACCEPT:
        filepath = chooser.get_filename()
        chooser.destroy()
        return filepath
    else:
        chooser.destroy()
        return None

def dialog_file_select(filter_pattern=[], filter_mime=[], filter_name=None, curr_folder=None, parent=None):
    """The Select file dialog, Returns the retrieved filepath or None"""
    chooser = Gtk.FileChooserDialog(title = _("Select File"),
                                    action=Gtk.FileChooserAction.OPEN,
                                    buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.ACCEPT) )
    if parent != None:
        chooser.set_transient_for(parent)
        chooser.set_property("modal", True)
        chooser.set_property("destroy-with-parent", True)
        chooser.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    else: chooser.set_position(Gtk.WindowPosition.CENTER)
    if curr_folder == None or os.path.isdir(curr_folder) == False:
        chooser.set_current_folder(os.path.expanduser('~'))
    else:
        chooser.set_current_folder(curr_folder)
    if filter_pattern or filter_mime:
        filefilter = Gtk.FileFilter()
        filefilter.set_name(filter_name)
        for element in filter_pattern:
            filefilter.add_pattern(element)
        for element in filter_mime:
            filefilter.add_mime_type(element)
        chooser.add_filter(filefilter)
    if chooser.run() == Gtk.ResponseType.ACCEPT:
        filepath = chooser.get_filename()
        chooser.destroy()
        return filepath
    else:
        chooser.destroy()
        return None

def dialog_folder_select(curr_folder=None, parent=None):
    """The Select folder dialog, returns the retrieved folderpath or None"""
    chooser = Gtk.FileChooserDialog(title = _("Select Folder"),
                                    action=Gtk.FileChooserAction.SELECT_FOLDER,
                                    buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.ACCEPT) )
    if parent != None:
        chooser.set_transient_for(parent)
        chooser.set_property("modal", True)
        chooser.set_property("destroy-with-parent", True)
        chooser.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    else: chooser.set_position(Gtk.WindowPosition.CENTER)
    if curr_folder == None or os.path.isdir(curr_folder) == False:
        chooser.set_current_folder(os.path.expanduser('~'))
    else:
        chooser.set_current_folder(curr_folder)
    if chooser.run() == Gtk.ResponseType.ACCEPT:
        folderpath = chooser.get_filename()
        chooser.destroy()
        return folderpath
    else:
        chooser.destroy()
        return None

def dialog_question(message, parent=None):
    """The Question dialog, returns True if the user presses OK"""
    dialog = Gtk.MessageDialog(parent=parent,
                               flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
                               type=Gtk.MessageType.QUESTION,
                               message_format=message)
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_title(_("Question"))
    dialog.add_button(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT)
    dialog.add_button(Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT)
    response = dialog.run()
    dialog.hide()
    return True if response == Gtk.ResponseType.ACCEPT else False

def dialog_info(message, parent):
    """The Info dialog"""
    dialog = Gtk.MessageDialog(parent=parent,
                               flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
                               type=Gtk.MessageType.INFO,
                               buttons=Gtk.ButtonsType.OK,
                               message_format=message)
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_title(_("Info"))
    dialog.run()
    dialog.destroy()

def dialog_info_after_restart(parent=None):
    """Change Only After Restart"""
    dialog_info(_("This Change will have Effect Only After Restarting CherryTree"), parent)

def dialog_warning(message, parent):
    """The Warning dialog"""
    dialog = Gtk.MessageDialog(parent=parent,
                               flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
                               type=Gtk.MessageType.WARNING,
                               buttons=Gtk.ButtonsType.OK,
                               message_format=message)
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_title(_("Warning"))
    dialog.run()
    dialog.destroy()

def dialog_error(message, parent):
    """The Error dialog"""
    dialog = Gtk.MessageDialog(parent=parent,
                               flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
                               type=Gtk.MessageType.ERROR,
                               buttons=Gtk.ButtonsType.OK,
                               message_format=message)
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_title(_("Error"))
    dialog.run()
    dialog.destroy()

def dialog_about(dad):
    """Application About Dialog"""
    dialog = Gtk.AboutDialog()
    dialog.set_program_name("CherryTree")
    dialog.set_version(cons.VERSION)
    dialog.set_copyright("""Copyright © 2009-2017
Giuseppe Penone <giuspen@gmail.com>""")
    dialog.set_comments(_("A Hierarchical Note Taking Application, featuring Rich Text and Syntax Highlighting"))
    dialog.set_license(_("""
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
  
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
  
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301, USA.
"""))
    dialog.set_website("http://www.giuspen.com/cherrytree/")
    dialog.set_authors(["Giuseppe Penone <giuspen@gmail.com>"])
    dialog.set_documenters(["Robert Boudreau <RobtTheB@gmail.com>"])
    dialog.set_artists(["OCAL <http://www.openclipart.org/>", "Zeltak <zeltak@gmail.com>", "Angelo Penone <angelo.penone@gmail.com>"])
    dialog.set_translator_credits(_("Armenian")+" (hy) Seda Stamboltsyan <sedastam@yandex.com>"+cons.CHAR_NEWLINE+
_("Chinese Simplified")+" (zh_CN) Channing Wong <channing.wong@qq.com>"+cons.CHAR_NEWLINE+
_("Czech")+" (cs) Pavel Fric <fripohled@blogspot.com>"+cons.CHAR_NEWLINE+
_("Dutch")+" (nl) Luuk Geurts"+cons.CHAR_NEWLINE+
_("French")+" (fr) Klaus Becker <colonius@free.fr>"+cons.CHAR_NEWLINE+
_("German")+" (de) Frank Brungräber <calexu@arcor.de>"+cons.CHAR_NEWLINE+
_("Greek")+" (el) Delphina <delphina.2009@yahoo.gr>"+cons.CHAR_NEWLINE+
_("Italian")+" (it) Vincenzo Reale <smart2128@baslug.org>"+cons.CHAR_NEWLINE+
_("Japanese")+" (ja) Piyo <py2@live.jp>"+cons.CHAR_NEWLINE+
_("Lithuanian")+" (lt) Zygis <zygimantus@gmail.com>"+cons.CHAR_NEWLINE+
_("Polish")+" (pl) Marcin Swierczynski <orneo1212@gmail.com>"+cons.CHAR_NEWLINE+
_("Portuguese Brazil")+" (pt_BR) Vinicius Schmidt <viniciussm@rocketmail.com>"+cons.CHAR_NEWLINE+
_("Russian")+" (ru) Andriy Kovtun <kovtunos@yandex.ru>"+cons.CHAR_NEWLINE+
_("Slovenian")+" (sl) Erik Lovrič <erik.lovric@gmail.com>"+cons.CHAR_NEWLINE+
_("Spanish")+" (es) Daniel MC <i.e.betel@gmail.com>"+cons.CHAR_NEWLINE+
_("Turkish")+" (tr) Ferhat Aydin <ferhataydin44@gmail.com>"+cons.CHAR_NEWLINE+
_("Ukrainian")+" (uk) Andriy Kovtun <kovtunos@yandex.ru>")
    dialog.set_logo(GdkPixbuf.Pixbuf.new_from_file(os.path.join(cons.GLADE_PATH, "cherrytree.png")))
    dialog.set_title(_("About CherryTree"))
    dialog.set_transient_for(dad.window)
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_modal(True)
    dialog.run()
    dialog.hide()

def dialog_choose_element_in_list(father_win, title, elements_list, column_title, icon_n_label_list=None):
    """Choose Between Elements in List"""
    class ListParms:
        def __init__(self):
            self.sel_iter = None
    list_parms = ListParms()
    dialog = Gtk.Dialog(title=title,
        parent=father_win,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
                 Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(400, 300)
    scrolledwindow = Gtk.ScrolledWindow()
    scrolledwindow.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
    if not icon_n_label_list: elements_liststore = Gtk.ListStore(str)
    else: elements_liststore = Gtk.ListStore(str,str,str)
    elements_treeview = Gtk.TreeView(elements_liststore)
    elements_renderer_text = Gtk.CellRendererText()
    if not icon_n_label_list:
        elements_column = Gtk.TreeViewColumn(column_title, elements_renderer_text, text=0)
    else:
        elements_treeview.set_headers_visible(False)
        renderer_pixbuf = Gtk.CellRendererPixbuf()
        renderer_text = Gtk.CellRendererText()
        elements_column = Gtk.TreeViewColumn()
        elements_column.pack_start(renderer_pixbuf, False)
        elements_column.pack_start(renderer_text, True)
        elements_column.set_attributes(renderer_pixbuf, stock_id=1)
        elements_column.set_attributes(renderer_text, text=2)
    elements_treeview.append_column(elements_column)
    elements_treeviewselection = elements_treeview.get_selection()
    if not icon_n_label_list:
        for element_name in elements_list:
            elements_liststore.append(element_name)
    else:
        for element in icon_n_label_list:
            elements_liststore.append([element[0], element[1], element[2]])
    scrolledwindow.add(elements_treeview)
    list_parms.sel_iter = elements_liststore.get_iter_first()
    if list_parms.sel_iter:
        elements_treeview.set_cursor(elements_liststore.get_path(list_parms.sel_iter))
    content_area = dialog.get_content_area()
    content_area.pack_start(scrolledwindow, True, True, 0)
    def on_mouse_button_clicked_elements_list(widget, event):
        if event.button != 1: return
        if event.type == Gdk.EventType._2BUTTON_PRESS:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
    def on_treeview_event_after(treeview, event):
        if event.type not in [Gdk.EventType.BUTTON_PRESS, Gdk.EventType.KEY_PRESS]: return
        model, list_parms.sel_iter = elements_treeviewselection.get_selected()
    def on_key_press_elementslistdialog(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        return False
    elements_treeview.connect('event-after', on_treeview_event_after)
    dialog.connect('key_press_event', on_key_press_elementslistdialog)
    elements_treeview.connect('button-press-event', on_mouse_button_clicked_elements_list)
    content_area.show_all()
    elements_treeview.grab_focus()
    response = dialog.run()
    dialog.hide()
    if response != Gtk.ResponseType.ACCEPT or not list_parms.sel_iter: return ""
    return elements_liststore[list_parms.sel_iter][0]

def dialog_img_n_entry(father_win, title, entry_content, img_stock):
    """Insert/Edit Anchor Name"""
    dialog = Gtk.Dialog(title=title,
        parent=father_win,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
                 Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(300, -1)
    image = Gtk.Image()
    image.set_from_stock(img_stock, Gtk.IconSize.BUTTON)
    entry = Gtk.Entry()
    entry.set_text(entry_content)
    hbox = Gtk.HBox()
    hbox.pack_start(image, False, True, 0)
    hbox.pack_start(entry, True, True, 0)
    hbox.set_spacing(5)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox, True, True, 0)
    def on_key_press_anchoreditdialog(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        return False
    dialog.connect('key_press_event', on_key_press_anchoreditdialog)
    content_area.show_all()
    entry.grab_focus()
    response = dialog.run()
    dialog.hide()
    return entry.get_text().strip() if response == Gtk.ResponseType.ACCEPT else ""

def dialog_image_handle(father_win, title, original_pixbuf):
    """Insert/Edit Image"""
    class ImgParms:
        def __init__(self, original_pixbuf):
            self.original_pixbuf = original_pixbuf
            self.width = original_pixbuf.get_width()
            self.height = original_pixbuf.get_height()
            self.image_w_h_ration = float(self.width)/self.height
    img_parms = ImgParms(original_pixbuf)
    dialog = Gtk.Dialog(title=title,
        parent=father_win,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
                 Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(600, 500)
    button_rotate_90_ccw = Gtk.Button()
    button_rotate_90_ccw.set_image(Gtk.Image.new_from_stock("object-rotate-left", Gtk.IconSize.DND))
    button_rotate_90_cw = Gtk.Button()
    button_rotate_90_cw.set_image(Gtk.Image.new_from_stock("object-rotate-right", Gtk.IconSize.DND))
    scrolledwindow = Gtk.ScrolledWindow()
    scrolledwindow.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
    viewport = Gtk.Viewport()
    image = Gtk.image_new_from_pixbuf(original_pixbuf)
    scrolledwindow.add(viewport)
    viewport.add(image)
    hbox_1 = Gtk.HBox()
    hbox_1.pack_start(button_rotate_90_ccw, False, True, 0)
    hbox_1.pack_start(scrolledwindow, True, True, 0)
    hbox_1.pack_start(button_rotate_90_cw, False, True, 0)
    hbox_1.set_spacing(2)
    label_width = Gtk.Label(label=_("Width"))
    adj_width = Gtk.Adjustment(value=img_parms.width, lower=1, upper=10000, step_incr=1)
    spinbutton_width = Gtk.SpinButton()
    spinbutton_width.set_adjustment(adj_width)
    label_height = Gtk.Label(label=_("Height"))
    adj_height = Gtk.Adjustment(value=img_parms.height, lower=1, upper=10000, step_incr=1)
    spinbutton_height = Gtk.SpinButton()
    spinbutton_height.set_adjustment(adj_height)
    hbox_2 = Gtk.HBox()
    hbox_2.pack_start(label_width, True, True, 0)
    hbox_2.pack_start(spinbutton_width, True, True, 0)
    hbox_2.pack_start(label_height, True, True, 0)
    hbox_2.pack_start(spinbutton_height, True, True, 0)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox_1, True, True, 0)
    content_area.pack_start(hbox_2, False, True, 0)
    content_area.set_spacing(6)
    def image_load_into_dialog():
        spinbutton_width.set_value(img_parms.width)
        spinbutton_height.set_value(img_parms.height)
        if img_parms.width <= 900 and img_parms.height <= 600:
            # original size into the dialog
            pixbuf = img_parms.original_pixbuf.scale_simple(int(img_parms.width), int(img_parms.height), GdkPixbuf.InterpType.BILINEAR)
        else:
            # reduced size visible into the dialog
            if img_parms.width > 900:
                img_parms_width = 900
                img_parms_height = img_parms_width / img_parms.image_w_h_ration
            else:
                img_parms_height = 600
                img_parms_width = img_parms_height * img_parms.image_w_h_ration
            pixbuf = img_parms.original_pixbuf.scale_simple(int(img_parms_width), int(img_parms_height), GdkPixbuf.InterpType.BILINEAR)
        image.set_from_pixbuf(pixbuf)
    def on_button_rotate_90_cw_clicked(*args):
        img_parms.original_pixbuf = img_parms.original_pixbuf.rotate_simple(270)
        img_parms.image_w_h_ration = 1/img_parms.image_w_h_ration
        new_width = img_parms.height # new width is the former height and vice versa
        img_parms.height = img_parms.width
        img_parms.width = new_width
        image_load_into_dialog()
    def on_button_rotate_90_ccw_clicked(*args):
        img_parms.original_pixbuf = img_parms.original_pixbuf.rotate_simple(90)
        img_parms.image_w_h_ration = 1/img_parms.image_w_h_ration
        new_width = img_parms.height # new width is the former height and vice versa
        img_parms.height = img_parms.width
        img_parms.width = new_width
        image_load_into_dialog()
    def on_spinbutton_image_width_value_changed(spinbutton):
        img_parms.width = spinbutton_width.get_value()
        img_parms.height = img_parms.width/img_parms.image_w_h_ration
        image_load_into_dialog()
    def on_spinbutton_image_height_value_changed(spinbutton):
        img_parms.height = spinbutton_height.get_value()
        img_parms.width = img_parms.height*img_parms.image_w_h_ration
        image_load_into_dialog()
    def on_key_press_imagehandledialog(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_RETURN:
            spinbutton_width.update()
            spinbutton_height.update()
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        return False
    button_rotate_90_ccw.connect('clicked', on_button_rotate_90_ccw_clicked)
    button_rotate_90_cw.connect('clicked', on_button_rotate_90_cw_clicked)
    spinbutton_width.connect('value-changed', on_spinbutton_image_width_value_changed)
    spinbutton_height.connect('value-changed', on_spinbutton_image_height_value_changed)
    dialog.connect('key_press_event', on_key_press_imagehandledialog)
    image_load_into_dialog()
    content_area.show_all()
    try: dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).grab_focus()
    except: pass
    response = dialog.run()
    dialog.hide()
    if response != Gtk.ResponseType.ACCEPT: return None
    return img_parms.original_pixbuf.scale_simple(int(img_parms.width), int(img_parms.height), GdkPixbuf.InterpType.BILINEAR)

def dialog_question_warning(father_win, warning_label):
    """Question with Warning"""
    dialog = Gtk.Dialog(title=_("Warning"),
        parent=father_win,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
                 Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(350, 150)
    image = Gtk.Image()
    image.set_from_stock(Gtk.STOCK_DIALOG_WARNING, Gtk.IconSize.DIALOG)
    label = Gtk.Label(label=warning_label)
    label.set_use_markup(True)
    hbox = Gtk.HBox()
    hbox.pack_start(image, True, True, 0)
    hbox.pack_start(label, True, True, 0)
    hbox.set_spacing(5)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox, True, True, 0)
    def on_key_press_nodedeletedialog(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        return False
    dialog.connect('key_press_event', on_key_press_nodedeletedialog)
    content_area.show_all()
    try: dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).grab_focus()
    except: pass
    response = dialog.run()
    dialog.hide()
    return response

def dialog_exit_del_temp_files(dad):
    """Temporary Files will be deleted, close applications using them"""
    dialog = Gtk.Dialog(title=_("Warning"),
        parent=dad.window,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(_("Cancel"), 2,
                 _("Yes"), 1) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(350, 150)
    try:
        button = dialog.get_widget_for_response(2)
        button.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_CANCEL, Gtk.IconSize.BUTTON))
        button = dialog.get_widget_for_response(1)
        button.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_DIALOG_WARNING, Gtk.IconSize.BUTTON))
        button.grab_focus()
    except: pass
    image = Gtk.Image()
    image.set_from_stock(Gtk.STOCK_DIALOG_WARNING, Gtk.IconSize.DIALOG)
    label = Gtk.Label(label="<b>"+_("Temporary Files were Created and Opened with External Applications.")+"</b>"+2*cons.CHAR_NEWLINE+"<b>"+_("Quit the External Applications Before Quit CherryTree.")+"</b>"+2*cons.CHAR_NEWLINE+"<b>"+_("Did you Quit the External Applications?")+"</b>")
    label.set_use_markup(True)
    hbox = Gtk.HBox()
    hbox.pack_start(image, True, True, 0)
    hbox.pack_start(label, True, True, 0)
    hbox.set_spacing(5)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox, True, True, 0)
    def on_key_press_exitdialog(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(1).clicked()
            return True
        elif keyname == "Escape":
            dialog.get_widget_for_response(2).clicked()
            return True
        return False
    dialog.connect('key_press_event', on_key_press_exitdialog)
    content_area.show_all()
    response = dialog.run()
    dialog.hide()
    return False if response != 1 else True

def dialog_exit_save(father_win):
    """Save before Exit Dialog"""
    dialog = Gtk.Dialog(title=_("Warning"),
        parent=father_win,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(_("Cancel"), 6,
                 _("No"), 4,
                 _("Yes"), 2) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(350, 150)
    try:
        button = dialog.get_widget_for_response(6)
        button.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_CANCEL, Gtk.IconSize.BUTTON))
        button = dialog.get_widget_for_response(4)
        button.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_CLEAR, Gtk.IconSize.BUTTON))
        button = dialog.get_widget_for_response(2)
        button.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_SAVE, Gtk.IconSize.BUTTON))
        button.grab_focus()
    except: pass
    image = Gtk.Image()
    image.set_from_stock(Gtk.STOCK_DIALOG_WARNING, Gtk.IconSize.DIALOG)
    label = Gtk.Label(label="<b>"+_("The Current Document was Updated.")+"</b>"+2*cons.CHAR_NEWLINE+"<b>"+_("Do you want to Save the Changes?")+"</b>")
    label.set_use_markup(True)
    hbox = Gtk.HBox()
    hbox.pack_start(image, True, True, 0)
    hbox.pack_start(label, True, True, 0)
    hbox.set_spacing(5)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox, True, True, 0)
    def on_key_press_exitdialog(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(2).clicked()
            return True
        elif keyname == "Escape":
            dialog.get_widget_for_response(6).clicked()
            return True
        return False
    dialog.connect('key_press_event', on_key_press_exitdialog)
    content_area.show_all()
    response = dialog.run()
    dialog.hide()
    return response

def dialog_link_handle(dad, title, sel_tree_iter):
    """Dialog to Insert/Edit Links"""
    class LinksParms:
        def __init__(self):
            self.sel_iter = sel_tree_iter if sel_tree_iter else dad.links_entries['node']
            self.first_in = True
    links_parms = LinksParms()
    dialog = Gtk.Dialog(title=title,
        parent=dad.window,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
        Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(600, 500)

    hbox_webs = Gtk.HBox()
    image_webs = Gtk.Image()
    image_webs.set_from_stock("link_website", Gtk.IconSize.BUTTON)
    radiobutton_webs = Gtk.RadioButton(label=_("To WebSite"))
    entry_webs = Gtk.Entry()
    entry_webs.set_text(dad.links_entries['webs'])
    hbox_webs.pack_start(image_webs, False, True, 0)
    hbox_webs.pack_start(radiobutton_webs, False, True, 0)
    hbox_webs.pack_start(entry_webs, True, True, 0)
    hbox_webs.set_spacing(5)

    hbox_file = Gtk.HBox()
    image_file = Gtk.Image()
    image_file.set_from_stock(Gtk.STOCK_FILE, Gtk.IconSize.BUTTON)
    radiobutton_file = Gtk.RadioButton.new_with_label_from_widget(radiobutton_webs, _("To File"))
    entry_file = Gtk.Entry()
    entry_file.set_text(dad.links_entries['file'])
    button_browse_file = Gtk.Button()
    button_browse_file.set_image(Gtk.Image.new_from_stock("find", Gtk.IconSize.BUTTON))
    hbox_file.pack_start(image_file, False, True, 0)
    hbox_file.pack_start(radiobutton_file, False, True, 0)
    hbox_file.pack_start(entry_file, True, True, 0)
    hbox_file.pack_start(button_browse_file, False, True, 0)
    hbox_file.set_spacing(5)

    hbox_folder = Gtk.HBox()
    image_folder = Gtk.Image()
    image_folder.set_from_stock(Gtk.STOCK_DIRECTORY, Gtk.IconSize.BUTTON)
    radiobutton_folder = Gtk.RadioButton.new_with_label_from_widget(radiobutton_webs, _("To Folder"))
    entry_folder = Gtk.Entry()
    entry_folder.set_text(dad.links_entries['fold'])
    button_browse_folder = Gtk.Button()
    button_browse_folder.set_image(Gtk.Image.new_from_stock("find", Gtk.IconSize.BUTTON))
    hbox_folder.pack_start(image_folder, False, True, 0)
    hbox_folder.pack_start(radiobutton_folder, False, True, 0)
    hbox_folder.pack_start(entry_folder, True, True, 0)
    hbox_folder.pack_start(button_browse_folder, False, True, 0)
    hbox_folder.set_spacing(5)

    hbox_node = Gtk.HBox()
    image_node = Gtk.Image()
    image_node.set_from_stock("cherrytree", Gtk.IconSize.BUTTON)
    radiobutton_node = Gtk.RadioButton.new_with_label_from_widget(radiobutton_webs, _("To Node"))
    hbox_node.pack_start(image_node, False, True, 0)
    hbox_node.pack_start(radiobutton_node, True, True, 0)
    hbox_node.set_spacing(5)

    hbox_detail = Gtk.HBox()

    treeview_2 = Gtk.TreeView(dad.treestore)
    treeview_2.set_headers_visible(False)
    treeview_2.set_search_column(1)
    renderer_pixbuf_2 = Gtk.CellRendererPixbuf()
    renderer_text_2 = Gtk.CellRendererText()
    column_2 = Gtk.TreeViewColumn()
    column_2.pack_start(renderer_pixbuf_2, False)
    column_2.pack_start(renderer_text_2, True)
    column_2.set_attributes(renderer_pixbuf_2, stock_id=0)
    column_2.set_attributes(renderer_text_2, text=1)
    treeview_2.append_column(column_2)
    treeviewselection_2 = treeview_2.get_selection()
    scrolledwindow = Gtk.ScrolledWindow()
    scrolledwindow.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
    scrolledwindow.add(treeview_2)

    vbox_anchor = Gtk.VBox()
    label_over = Gtk.Label()
    label_below = Gtk.Label()

    hbox_anchor = Gtk.HBox()
    entry_anchor = Gtk.Entry()
    entry_anchor.set_text(dad.links_entries['anch'])
    button_browse_anchor = Gtk.Button()
    button_browse_anchor.set_image(Gtk.Image.new_from_stock("anchor", Gtk.IconSize.BUTTON))
    hbox_anchor.pack_start(entry_anchor, True, True, 0)
    hbox_anchor.pack_start(button_browse_anchor, False, True, 0)

    frame_anchor = Gtk.Frame(label="<b>"+_("Anchor Name (optional)")+"</b>")
    frame_anchor.get_label_widget().set_use_markup(True)
    frame_anchor.set_shadow_type(Gtk.ShadowType.NONE)
    frame_anchor.add(hbox_anchor)

    vbox_anchor.pack_start(label_over, True, True, 0)
    vbox_anchor.pack_start(frame_anchor, False, True, 0)
    vbox_anchor.pack_start(label_below, True, True, 0)

    hbox_detail.pack_start(scrolledwindow, True, True, 0)
    hbox_detail.pack_start(vbox_anchor, False, True, 0)

    content_area = dialog.get_content_area()
    content_area.pack_start(hbox_webs, False, True, 0)
    content_area.pack_start(hbox_file, False, True, 0)
    content_area.pack_start(hbox_folder, False, True, 0)
    content_area.pack_start(hbox_node, False, True, 0)
    content_area.pack_start(hbox_detail, True, True, 0)
    content_area.set_spacing(5)

    radiobutton_webs.set_active(dad.link_type == cons.LINK_TYPE_WEBS)
    radiobutton_node.set_active(dad.link_type == cons.LINK_TYPE_NODE)
    radiobutton_file.set_active(dad.link_type == cons.LINK_TYPE_FILE)
    radiobutton_folder.set_active(dad.link_type == cons.LINK_TYPE_FOLD)

    def link_type_changed_on_dialog():
        entry_webs.set_sensitive(dad.link_type == cons.LINK_TYPE_WEBS)
        hbox_detail.set_sensitive(dad.link_type == cons.LINK_TYPE_NODE)
        entry_file.set_sensitive(dad.link_type == cons.LINK_TYPE_FILE)
        entry_folder.set_sensitive(dad.link_type == cons.LINK_TYPE_FOLD)
        if dad.link_type == cons.LINK_TYPE_WEBS: entry_webs.grab_focus()
        elif dad.link_type == cons.LINK_TYPE_NODE:
            treeview_2.grab_focus()
            if links_parms.first_in:
                links_parms.first_in = False
                config.get_tree_expanded_collapsed_string(dad)
                config.set_tree_expanded_collapsed_string(dad, treeview=treeview_2)
            if not links_parms.sel_iter: links_parms.sel_iter = dad.treestore.get_iter_first()
            sel_path = dad.treestore.get_path(links_parms.sel_iter)
            treeview_2.expand_to_path(sel_path)
            treeview_2.set_cursor(sel_path)
            treeview_2.scroll_to_cell(sel_path)
        elif dad.link_type == cons.LINK_TYPE_FILE: entry_file.grab_focus()
        else: entry_folder.grab_focus()
    def on_radiobutton_link_website_toggled(radiobutton):
        if radiobutton.get_active(): dad.link_type = cons.LINK_TYPE_WEBS
        link_type_changed_on_dialog()
    def on_radiobutton_link_node_anchor_toggled(radiobutton):
        if radiobutton.get_active(): dad.link_type = cons.LINK_TYPE_NODE
        link_type_changed_on_dialog()
    def on_radiobutton_link_file_toggled(radiobutton):
        if radiobutton.get_active(): dad.link_type = cons.LINK_TYPE_FILE
        link_type_changed_on_dialog()
    def on_radiobutton_link_folder_toggled(radiobutton):
        if radiobutton.get_active(): dad.link_type = cons.LINK_TYPE_FOLD
        link_type_changed_on_dialog()
    def on_button_browse_for_file_to_link_to_clicked(self, *args):
        filepath = dialog_file_select(curr_folder=dad.pick_dir_file, parent=dialog)
        if not filepath: return
        dad.pick_dir_file = os.path.dirname(filepath)
        if dad.links_relative:
            try: filepath = os.path.relpath(filepath, dad.file_dir)
            except: print("cannot set relative path for different drives")
        entry_file.set_text(filepath)
    def on_button_browse_for_folder_to_link_to_clicked(self, *args):
        filepath = dialog_folder_select(curr_folder=dad.pick_dir_file, parent=dialog)
        if not filepath: return
        dad.pick_dir_file = filepath
        if dad.links_relative:
            try: filepath = os.path.relpath(filepath, dad.file_dir)
            except: print("cannot set relative path for different drives")
        entry_folder.set_text(filepath)
    def on_browse_anchors_button_clicked(*args):
        if not links_parms.sel_iter:
            dialog_warning(_("No Node is Selected"), dialog)
            return
        anchors_list = []
        text_buffer = dad.get_textbuffer_from_tree_iter(links_parms.sel_iter)
        curr_iter = text_buffer.get_start_iter()
        while 1:
            anchor = curr_iter.get_child_anchor()
            if anchor != None:
                if "pixbuf" in dir(anchor) and "anchor" in dir(anchor.pixbuf):
                    anchors_list.append([anchor.pixbuf.anchor])
            if not curr_iter.forward_char(): break
        dad.objects_buffer_refresh()
        if not anchors_list:
            dialog_info(_("There are No Anchors in the Selected Node"), dialog)
            return
        ret_anchor_name = dialog_choose_element_in_list(dialog, _("Choose Existing Anchor"), anchors_list, _("Anchor Name"))
        if ret_anchor_name: entry_anchor.set_text(ret_anchor_name)
    def on_treeview_event_after(treeview, event):
        if event.type not in [Gdk.EventType.BUTTON_PRESS, Gdk.EventType._2BUTTON_PRESS, Gdk.EventType.KEY_PRESS]: return
        model, links_parms.sel_iter = treeviewselection_2.get_selected()
        if event.type == Gdk.EventType.BUTTON_PRESS:
            if event.button == 2:
                path_at_click = treeview.get_path_at_pos(int(event.x), int(event.y))
                if path_at_click:
                    if treeview.row_expanded(path_at_click[0]):
                        treeview.collapse_row(path_at_click[0])
                    else: treeview.expand_row(path_at_click[0], False)
        elif event.type == Gdk.EventType._2BUTTON_PRESS and event.button == 1:
            if links_parms.sel_iter:
                treestore = treeview.get_model()
                if treeview.row_expanded(treestore.get_path(links_parms.sel_iter)):
                    treeview.collapse_row(treestore.get_path(links_parms.sel_iter))
                else:
                    treeview.expand_row(treestore.get_path(links_parms.sel_iter), open_all=False)
        elif event.type == Gdk.EventType.KEY_PRESS:
            if links_parms.sel_iter:
                treestore = treeview.get_model()
                keyname = Gdk.keyval_name(event.keyval)
                if keyname == cons.STR_KEY_LEFT:
                    treeview.collapse_row(treestore.get_path(links_parms.sel_iter))
                elif keyname == cons.STR_KEY_RIGHT:
                    treeview.expand_row(treestore.get_path(links_parms.sel_iter), open_all=False)
    def on_key_press_links_handle_dialog(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        elif keyname == cons.STR_KEY_TAB:
            if dad.link_type == cons.LINK_TYPE_WEBS: radiobutton_file.set_active(True)
            elif dad.link_type == cons.LINK_TYPE_FILE: radiobutton_folder.set_active(True)
            elif dad.link_type == cons.LINK_TYPE_FOLD: radiobutton_node.set_active(True)
            else: radiobutton_webs.set_active(True)
            return True
        return False
    radiobutton_webs.connect("toggled", on_radiobutton_link_website_toggled)
    radiobutton_node.connect("toggled", on_radiobutton_link_node_anchor_toggled)
    radiobutton_file.connect("toggled", on_radiobutton_link_file_toggled)
    radiobutton_folder.connect("toggled", on_radiobutton_link_folder_toggled)
    button_browse_file.connect('clicked', on_button_browse_for_file_to_link_to_clicked)
    button_browse_folder.connect('clicked', on_button_browse_for_folder_to_link_to_clicked)
    button_browse_anchor.connect('clicked', on_browse_anchors_button_clicked)
    treeview_2.connect('event-after', on_treeview_event_after)
    dialog.connect("key_press_event", on_key_press_links_handle_dialog)

    content_area.show_all()
    link_type_changed_on_dialog()
    response = dialog.run()
    dialog.hide()
    if response != Gtk.ResponseType.ACCEPT: return False
    dad.links_entries['webs'] = entry_webs.get_text().strip()
    dad.links_entries['file'] = entry_file.get_text().strip()
    dad.links_entries['fold'] = entry_folder.get_text().strip()
    dad.links_entries['anch'] = entry_anchor.get_text().strip()
    dad.links_entries['node'] = links_parms.sel_iter
    return True

def dialog_choose_data_storage(dad):
    """Choose the CherryTree data storage type (xml or db) and protection"""
    dialog = Gtk.Dialog(title=_("Choose Storage Type"),
                        parent=dad.window,
                        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
                        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
                        Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(350, -1)
    radiobutton_sqlite_not_protected = Gtk.RadioButton(label="SQLite, " + _("Not Protected") + " (.ctb)")
    radiobutton_sqlite_pass_protected = Gtk.RadioButton.new_with_label_from_widget(radiobutton_sqlite_not_protected, "SQLite, " + _("Password Protected") + " (.ctx)")
    radiobutton_xml_not_protected = Gtk.RadioButton.new_with_label_from_widget(radiobutton_sqlite_not_protected, "XML, " + _("Not Protected") + " (.ctd)")
    radiobutton_xml_pass_protected = Gtk.RadioButton.new_with_label_from_widget(radiobutton_sqlite_not_protected, "XML, " + _("Password Protected") + " (.ctz)")
    type_vbox = Gtk.VBox()
    type_vbox.pack_start(radiobutton_sqlite_not_protected, True, True, 0)
    type_vbox.pack_start(radiobutton_sqlite_pass_protected, True, True, 0)
    type_vbox.pack_start(radiobutton_xml_not_protected, True, True, 0)
    type_vbox.pack_start(radiobutton_xml_pass_protected, True, True, 0)
    type_frame = Gtk.Frame(label="<b>"+_("Storage Type")+"</b>")
    type_frame.get_label_widget().set_use_markup(True)
    type_frame.set_shadow_type(Gtk.ShadowType.NONE)
    type_frame.add(type_vbox)
    entry_passw_1 = Gtk.Entry()
    entry_passw_1.set_visibility(False)
    entry_passw_2 = Gtk.Entry()
    entry_passw_2.set_visibility(False)
    label_passwd = Gtk.Label(_("CT saves the document in an encrypted 7zip archive. When viewing or editing the document, CT extracts the encrypted archive to a temporary folder, and works on the unencrypted copy. When closing, the unencrypted copy is deleted from the temporary directory. Note that in the case of application or system crash, the unencrypted document will remain in the temporary folder."))
    label_passwd.set_width_chars(70)
    label_passwd.set_line_wrap(True)
    vbox_passw = Gtk.VBox()
    vbox_passw.pack_start(entry_passw_1, True, True, 0)
    vbox_passw.pack_start(entry_passw_2, True, True, 0)
    vbox_passw.pack_start(label_passwd, True, True, 0)
    passw_frame = Gtk.Frame(label="<b>"+_("Enter the New Password Twice")+"</b>")
    passw_frame.get_label_widget().set_use_markup(True)
    passw_frame.set_shadow_type(Gtk.ShadowType.NONE)
    passw_frame.add(vbox_passw)
    if len(dad.file_name) > 4:
        if dad.file_name[-1] == "b": radiobutton_sqlite_not_protected.set_active(True)
        elif dad.file_name[-1] == "x": radiobutton_sqlite_pass_protected.set_active(True)
        elif dad.file_name[-1] == "d": radiobutton_xml_not_protected.set_active(True)
        else: radiobutton_xml_pass_protected.set_active(True)
    if dad.password: passw_frame.set_sensitive(True)
    else: passw_frame.set_sensitive(False)
    content_area = dialog.get_content_area()
    content_area.set_spacing(5)
    content_area.pack_start(type_frame, True, True, 0)
    content_area.pack_start(passw_frame, True, True, 0)
    content_area.show_all()
    def on_radiobutton_savetype_toggled(widget):
        if radiobutton_sqlite_pass_protected.get_active()\
        or radiobutton_xml_pass_protected.get_active():
            passw_frame.set_sensitive(True)
            entry_passw_1.grab_focus()
        else: passw_frame.set_sensitive(False)
    def on_key_press_edit_data_storage_type_dialog(widget, event):
        if Gdk.keyval_name(event.keyval) == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        return False
    radiobutton_sqlite_not_protected.connect("toggled", on_radiobutton_savetype_toggled)
    radiobutton_sqlite_pass_protected.connect("toggled", on_radiobutton_savetype_toggled)
    radiobutton_xml_not_protected.connect("toggled", on_radiobutton_savetype_toggled)
    dialog.connect("key_press_event", on_key_press_edit_data_storage_type_dialog)
    response = dialog.run()
    storage_type_is_xml = (radiobutton_xml_not_protected.get_active()\
                           or radiobutton_xml_pass_protected.get_active())
    new_protection = {'on': (radiobutton_sqlite_pass_protected.get_active()\
                             or radiobutton_xml_pass_protected.get_active()),
                      'p1': entry_passw_1.get_text(),
                      'p2': entry_passw_2.get_text()}
    dialog.destroy()
    if response != Gtk.ResponseType.ACCEPT: return False
    if new_protection['on']:
        if new_protection['p1'] == "":
            dialog_error(_("The Password Fields Must be Filled"), dad.window)
            return False
        if new_protection['p1'] != new_protection['p2']:
            dialog_error(_("The Two Inserted Passwords Do Not Match"), dad.window)
            return False
        for bad_char in cons.CHARS_NOT_FOR_PASSWD:
            if bad_char in new_protection['p1']:
                dialog_error(_("The Characters  %s  are Not Allowed") % cons.CHAR_SPACE.join(cons.CHARS_NOT_FOR_PASSWD), dad.window)
                return False
        if not new_protection['p1'] or not dad.is_7za_available(): return False
        dad.password = new_protection['p1']
    else: dad.password = None
    if storage_type_is_xml:
        if dad.password: dad.filetype = "z"
        else: dad.filetype = "d"
    else:
        if dad.password: dad.filetype = "x"
        else: dad.filetype = "b"
    #print "dad.filetype = '%s'" % dad.filetype
    return True

def dialog_choose_node(dad, title, treestore, sel_tree_iter):
    """Dialog to Select a Node"""
    class NodeParms:
        def __init__(self):
            self.sel_iter = sel_tree_iter
    node_parms = NodeParms()
    dialog = Gtk.Dialog(title=title,
        parent=dad.window,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
        Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    dialog.set_default_size(600, 500)
    treeview_2 = Gtk.TreeView(treestore)
    treeview_2.set_headers_visible(False)
    treeview_2.set_search_column(1)
    renderer_pixbuf_2 = Gtk.CellRendererPixbuf()
    renderer_text_2 = Gtk.CellRendererText()
    column_2 = Gtk.TreeViewColumn()
    column_2.pack_start(renderer_pixbuf_2, False)
    column_2.pack_start(renderer_text_2, True)
    column_2.set_attributes(renderer_pixbuf_2, stock_id=0)
    column_2.set_attributes(renderer_text_2, text=1)
    treeview_2.append_column(column_2)
    treeviewselection_2 = treeview_2.get_selection()
    scrolledwindow = Gtk.ScrolledWindow()
    scrolledwindow.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
    scrolledwindow.add(treeview_2)
    content_area = dialog.get_content_area()
    content_area.pack_start(scrolledwindow, True, True, 0)
    def on_key_press_choose_node_dialog(widget, event):
        if Gdk.keyval_name(event.keyval) == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        return False
    def on_treeview_event_after(treeview, event):
        if event.type not in [Gdk.EventType.BUTTON_PRESS, Gdk.EventType._2BUTTON_PRESS, Gdk.EventType.KEY_PRESS]: return
        model, node_parms.sel_iter = treeviewselection_2.get_selected()
        if event.type == Gdk.EventType.BUTTON_PRESS:
            if event.button == 2:
                path_at_click = treeview.get_path_at_pos(int(event.x), int(event.y))
                if path_at_click:
                    if treeview.row_expanded(path_at_click[0]):
                        treeview.collapse_row(path_at_click[0])
                    else: treeview.expand_row(path_at_click[0], False)
        elif event.type == Gdk.EventType._2BUTTON_PRESS and event.button == 1:
            if node_parms.sel_iter:
                treestore = treeview.get_model()
                if treeview.row_expanded(treestore.get_path(node_parms.sel_iter)):
                    treeview.collapse_row(treestore.get_path(node_parms.sel_iter))
                else:
                    treeview.expand_row(treestore.get_path(node_parms.sel_iter), open_all=False)
        elif event.type == Gdk.EventType.KEY_PRESS:
            if node_parms.sel_iter:
                treestore = treeview.get_model()
                keyname = Gdk.keyval_name(event.keyval)
                if keyname == cons.STR_KEY_LEFT:
                    treeview.collapse_row(treestore.get_path(node_parms.sel_iter))
                elif keyname == cons.STR_KEY_RIGHT:
                    treeview.expand_row(treestore.get_path(node_parms.sel_iter), open_all=False)
    dialog.connect("key_press_event", on_key_press_choose_node_dialog)
    treeview_2.connect('event-after', on_treeview_event_after)
    content_area.show_all()
    config.get_tree_expanded_collapsed_string(dad)
    config.set_tree_expanded_collapsed_string(dad, treeview=treeview_2)
    if node_parms.sel_iter:
        sel_path = treestore.get_path(node_parms.sel_iter)
        treeview_2.expand_to_path(sel_path)
        treeview_2.set_cursor(sel_path)
        treeview_2.scroll_to_cell(sel_path)
    response = dialog.run()
    dialog.hide()
    return None if response != Gtk.ResponseType.ACCEPT else node_parms.sel_iter

def dialog_selnode_selnodeandsub_alltree(dad, also_selection, also_include_node_name=False, also_new_node_page=False, also_index_in_page=False):
    """Dialog to select between the Selected Node/Selected Node + Subnodes/All Tree"""
    dialog = Gtk.Dialog(title=_("Involved Nodes"),
        parent=dad.window,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
        Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    radiobutton_selnode = Gtk.RadioButton.new_with_label_from_widget(label=_("Selected Node Only"))
    if also_selection:
        radiobutton_selection = Gtk.RadioButton.new_with_label_from_widget(radiobutton_selnode, _("Selected Text Only"))
    radiobutton_selnodeandsub = Gtk.RadioButton.new_with_label_from_widget(radiobutton_selnode, _("Selected Node and Subnodes"))
    radiobutton_alltree = Gtk.RadioButton.new_with_label_from_widget(radiobutton_selnode, _("All the Tree"))
    content_area = dialog.get_content_area()
    if also_selection:
        content_area.pack_start(radiobutton_selection, True, True, 0)
    content_area.pack_start(radiobutton_selnode, True, True, 0)
    content_area.pack_start(radiobutton_selnodeandsub, True, True, 0)
    content_area.pack_start(radiobutton_alltree, True, True, 0)
    if also_include_node_name:
        separator_item = Gtk.HSeparator()
        checkbutton_node_name = Gtk.CheckButton(label=_("Include Node Name"))
        checkbutton_node_name.set_active(dad.last_include_node_name)
        content_area.pack_start(separator_item, True, True, 0)
        content_area.pack_start(checkbutton_node_name, True, True, 0)
    if also_index_in_page:
        separator_item = Gtk.HSeparator()
        checkbutton_index_in_page = Gtk.CheckButton(label=_("Links Tree in Every Page"))
        checkbutton_index_in_page.set_active(dad.last_index_in_page)
        content_area.pack_start(separator_item, True, True, 0)
        content_area.pack_start(checkbutton_index_in_page, True, True, 0)
    if also_new_node_page:
        checkbutton_new_node_page = Gtk.CheckButton(label=_("New Node in New Page"))
        checkbutton_new_node_page.set_active(dad.last_new_node_page)
        content_area.pack_start(checkbutton_new_node_page, True, True, 0)
    def on_key_press_enter_dialog(widget, event):
        if Gdk.keyval_name(event.keyval) == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.ACCEPT).clicked()
            return True
        return False
    dialog.connect("key_press_event", on_key_press_enter_dialog)
    content_area.show_all()
    response = dialog.run()
    if radiobutton_selnode.get_active(): ret_val = 1
    elif radiobutton_selnodeandsub.get_active(): ret_val = 2
    elif radiobutton_alltree.get_active(): ret_val = 3
    else: ret_val = 4
    if also_include_node_name:
        dad.last_include_node_name = checkbutton_node_name.get_active()
    if also_index_in_page:
        dad.last_index_in_page = checkbutton_index_in_page.get_active()
    if also_new_node_page:
        dad.last_new_node_page = checkbutton_new_node_page.get_active()
    dialog.destroy()
    if response != Gtk.ResponseType.ACCEPT: ret_val = 0
    return ret_val

def dialog_color_pick(dad, curr_color=None):
    """Dialog to select a color, featuring a palette"""
    dialog = Gtk.ColorSelectionDialog(_("Pick a Color"))
    dialog.set_transient_for(dad.window)
    dialog.set_property("modal", True)
    dialog.set_property("destroy-with-parent", True)
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    gtk_settings = Gtk.Settings.get_default()
    gtk_settings.set_property("gtk-color-palette", ":".join(dad.palette_list))
    colorselection = dialog.get_color_selection()
    colorselection.set_has_palette(True)
    if curr_color:
        colorselection.set_current_color(curr_color)
    def on_key_press_color_pick_dialog(widget, event):
        if Gdk.keyval_name(event.keyval) == cons.STR_KEY_RETURN:
            dialog.get_widget_for_response(Gtk.ResponseType.OK).clicked()
            return True
        return False
    dialog.connect("key_press_event", on_key_press_color_pick_dialog)
    def on_mouse_button_clicked_color_pick_dialog(widget, event):
        if event.button == 1 and event.type == Gdk.EventType._2BUTTON_PRESS:
            dialog.get_widget_for_response(Gtk.ResponseType.OK).clicked()
    dialog.connect('button-press-event', on_mouse_button_clicked_color_pick_dialog)
    response = dialog.run()
    dialog.hide()
    if response != Gtk.ResponseType.OK: return None
    ret_color = colorselection.get_current_color()
    color_str_hex8 = "#" + exports.rgb_any_to_24(ret_color.to_string()[1:])
    if color_str_hex8 in dad.palette_list:
        dad.palette_list.remove(color_str_hex8)
    else:
        dad.palette_list.pop()
    dad.palette_list.insert(0, color_str_hex8)
    return ret_color

def set_bookmarks_menu_items(dad):
    """Set Bookmarks Menu Items"""
    bookmarks_menu = dad.ui.get_widget("/MenuBar/BookmarksMenu").get_submenu()
    for menu_item in dad.bookmarks_menu_items:
        bookmarks_menu.remove(menu_item)
    menu_item = Gtk.SeparatorMenuItem()
    menu_item.show()
    bookmarks_menu.append(menu_item)
    dad.bookmarks_menu_items = [menu_item]
    bookmarks_to_rm = []
    for node_id_str in dad.bookmarks:
        if not int(node_id_str) in dad.nodes_names_dict:
            bookmarks_to_rm.append(node_id_str)
            continue
        menu_item = Gtk.ImageMenuItem(dad.nodes_names_dict[int(node_id_str)])
        menu_item.set_image(Gtk.Image.new_from_stock("pin", Gtk.IconSize.MENU))
        menu_item.connect("activate", select_bookmark_node, node_id_str, dad)
        menu_item.show()
        bookmarks_menu.append(menu_item)
        dad.bookmarks_menu_items.append(menu_item)
    for element in bookmarks_to_rm: dad.bookmarks.remove(element)

def set_menu_items_special_chars(dad):
    """Set Special Chars menu items"""
    if not "special_menu_1" in dir(dad):
        dad.special_menu_1 = Gtk.Menu()
        first_run = True
    else:
        children_1 = dad.special_menu_1.get_children()
        for children in children_1:
            children.destroy()
        first_run = False
    for special_char in dad.special_chars:
        menu_item = Gtk.MenuItem(special_char)
        menu_item.connect("activate", insert_special_char, special_char, dad)
        menu_item.show()
        dad.special_menu_1.append(menu_item)
    if first_run:
        # main menu
        special_menuitem = Gtk.ImageMenuItem(_("Insert _Special Character"))
        special_menuitem.set_image(Gtk.Image.new_from_stock("insert", Gtk.IconSize.MENU))
        special_menuitem.set_tooltip_text(_("Insert a Special Character"))
        special_menuitem.set_submenu(dad.special_menu_1)
        dad.ui.get_widget("/MenuBar/EditMenu").get_submenu().insert(special_menuitem, 14)

def set_menu_items_recent_documents(dad):
    """Set Recent Documents menu items on Menu and Toolbar"""
    if not "recent_menu_1" in dir(dad):
        dad.recent_menu_1 = Gtk.Menu()
        dad.recent_menu_2 = Gtk.Menu()
        first_run = True
    else:
        children_1 = dad.recent_menu_1.get_children()
        children_2 = dad.recent_menu_2.get_children()
        for children in children_1:
            children.destroy()
        for children in children_2:
            children.destroy()
        first_run = False
    for target in [1, 2]:
        for i, filepath in enumerate(dad.recent_docs):
            if i >= cons.MAX_RECENT_DOCS: break
            menu_item = Gtk.ImageMenuItem(filepath)
            menu_item.set_image(Gtk.Image.new_from_stock("gtk-open", Gtk.IconSize.MENU))
            menu_item.connect("activate", open_recent_document, filepath, dad)
            menu_item.show()
            if target == 1: dad.recent_menu_1.append(menu_item)
            else: dad.recent_menu_2.append(menu_item)
    if first_run:
        # main menu
        recent_menuitem = Gtk.ImageMenuItem(_("_Recent Documents"))
        recent_menuitem.set_image(Gtk.Image.new_from_stock("gtk-open", Gtk.IconSize.MENU))
        recent_menuitem.set_tooltip_text(_("Open a Recent CherryTree Document"))
        recent_menuitem.set_submenu(dad.recent_menu_1)
        dad.ui.get_widget("/MenuBar/FileMenu").get_submenu().insert(recent_menuitem, 3)
        # toolbar
        if dad.toolbar_open_n_recent >= 0:
            menu_toolbutton = Gtk.MenuToolButton("gtk-open")
            menu_toolbutton.set_tooltip_text(_("Open a CherryTree Document"))
            menu_toolbutton.set_arrow_tooltip_text(_("Open a Recent CherryTree Document"))
            menu_toolbutton.set_menu(dad.recent_menu_2)
            menu_toolbutton.connect("clicked", dad.file_open)
            dad.ui.get_widget("/ToolBar").insert(menu_toolbutton, dad.toolbar_open_n_recent)

def add_recent_document(dad, filepath):
    """Add a Recent Document if Needed"""
    if filepath in dad.recent_docs and dad.recent_docs[0] == filepath:
        return
    if filepath in dad.recent_docs: dad.recent_docs.remove(filepath)
    dad.recent_docs.insert(0, filepath)
    set_menu_items_recent_documents(dad)

def insert_special_char(menu_item, special_char, dad):
    """A Special character insert was Requested"""
    text_view, text_buffer, from_codebox = dad.get_text_view_n_buffer_codebox_proof()
    if not text_buffer: return
    if not dad.is_curr_node_not_read_only_or_error(): return
    text_buffer.insert_at_cursor(special_char)

def open_recent_document(menu_item, filepath, dad):
    """A Recent Document was Requested"""
    if os.path.isfile(filepath):
        #dad.filepath_boss_open(filepath, "")
        dad.filepath_open(filepath)
    else:
        dialog_error(_("The Document %s was Not Found") % filepath, dad.window)
        try:
            idx = dad.recent_docs.index(filepath)
            dad.recent_docs.append(dad.recent_docs.pop(idx))
            set_menu_items_recent_documents(dad)
        except:
            pass

def select_bookmark_node(menu_item, node_id_str, dad):
    """Select a Node in the Bookmarks List"""
    node_iter = dad.get_tree_iter_from_node_id(int(node_id_str))
    if node_iter:
        dad.treeview_safe_set_cursor(node_iter)
        if dad.tree_click_expand:
            dad.treeview.expand_row(dad.treestore.get_path(node_iter), open_all=False)

def bookmarks_handle(dad):
    """Handle the Bookmarks List"""
    dialog = Gtk.Dialog(title=_("Handle the Bookmarks List"),
        parent=dad.window,
        flags=Gtk.DialogFlags.MODAL|Gtk.DialogFlags.DESTROY_WITH_PARENT,
        buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
        Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT) )
    dialog.set_default_size(500, 400)
    dialog.set_position(Gtk.WindowPosition.CENTER_ON_PARENT)
    liststore = Gtk.ListStore(str, str, str)
    for node_id_str in dad.bookmarks:
        # icon, node name, node id string
        liststore.append(["pin", dad.nodes_names_dict[int(node_id_str)], node_id_str])
    treeview = Gtk.TreeView(liststore)
    treeview.set_headers_visible(False)
    treeview.set_reorderable(True)
    treeviewselection = treeview.get_selection()
    def on_key_press_liststore(widget, event):
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_DELETE:
            model, tree_iter = treeviewselection.get_selected()
            if tree_iter: model.remove(tree_iter)
    def on_mouse_button_clicked_liststore(widget, event):
        """Catches mouse buttons clicks"""
        if event.button != 1: return
        if event.type != Gdk.EventType._2BUTTON_PRESS: return
        path_n_tvc = treeview.get_path_at_pos(int(event.x), int(event.y))
        if not path_n_tvc: return
        tree_path = path_n_tvc[0]
        dad_tree_path = dad.get_tree_iter_from_node_id(int(liststore[tree_path][2]))
        dad.treeview_safe_set_cursor(dad_tree_path)
    treeview.connect('key_press_event', on_key_press_liststore)
    treeview.connect('button-press-event', on_mouse_button_clicked_liststore)
    renderer_pixbuf = Gtk.CellRendererPixbuf()
    renderer_text = Gtk.CellRendererText()
    column = Gtk.TreeViewColumn()
    column.pack_start(renderer_pixbuf, False)
    column.pack_start(renderer_text, True)
    column.set_attributes(renderer_pixbuf, stock_id=0)
    column.set_attributes(renderer_text, text=1)
    treeview.append_column(column)
    scrolledwindow = Gtk.ScrolledWindow()
    scrolledwindow.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
    scrolledwindow.add(treeview)
    content_area = dialog.get_content_area()
    hbox = Gtk.HBox()
    vbox = Gtk.VBox()
    vbox.set_spacing(1)
    button_move_up = Gtk.Button()
    button_move_up.set_image(Gtk.Image.new_from_stock("gtk-go-up", Gtk.IconSize.DND))
    button_move_down = Gtk.Button()
    button_move_down.set_image(Gtk.Image.new_from_stock("gtk-go-down", Gtk.IconSize.DND))
    button_delete = Gtk.Button()
    button_delete.set_image(Gtk.Image.new_from_stock("gtk-clear", Gtk.IconSize.DND))
    button_sort_desc = Gtk.Button()
    button_sort_desc.set_image(Gtk.Image.new_from_stock("gtk-sort-descending", Gtk.IconSize.DND))
    button_sort_asc = Gtk.Button()
    button_sort_asc.set_image(Gtk.Image.new_from_stock("gtk-sort-ascending", Gtk.IconSize.DND))
    def on_button_move_up_clicked(*args):
        model, tree_iter = treeviewselection.get_selected()
        if tree_iter:
            prev_iter = dad.get_tree_iter_prev_sibling(model, tree_iter)
            if prev_iter:
                model.swap(tree_iter, prev_iter)
    button_move_up.connect('clicked', on_button_move_up_clicked)
    def on_button_move_down_clicked(*args):
        model, tree_iter = treeviewselection.get_selected()
        if tree_iter:
            next_iter = model.iter_next(tree_iter)
            if next_iter:
                model.swap(tree_iter, next_iter)
    button_move_down.connect('clicked', on_button_move_down_clicked)
    def on_button_delete_clicked(*args):
        model, tree_iter = treeviewselection.get_selected()
        if tree_iter:
            model.remove(tree_iter)
    button_delete.connect('clicked', on_button_delete_clicked)
    def on_button_sort_desc_clicked(*args):
        while dad.node_siblings_sort_iteration(liststore, None, False, 1):
            pass
    button_sort_desc.connect('clicked', on_button_sort_desc_clicked)
    def on_button_sort_asc_clicked(*args):
        while dad.node_siblings_sort_iteration(liststore, None, True, 1):
            pass
    button_sort_asc.connect('clicked', on_button_sort_asc_clicked)
    vbox.pack_start(button_move_up, False, True, 0)
    vbox.pack_start(button_move_down, False, True, 0)
    vbox.pack_start(button_delete, False, True, 0)
    vbox.pack_start(Gtk.Label(), True, True, 0)
    vbox.pack_start(button_sort_desc, False, True, 0)
    vbox.pack_start(button_sort_asc, False, True, 0)
    vbox.pack_start(Gtk.Label(), True, True, 0)
    hbox.pack_start(scrolledwindow, True, True, 0)
    hbox.pack_start(vbox, False, True, 0)
    content_area.pack_start(hbox, True, True, 0)
    content_area.show_all()
    response = dialog.run()
    temp_bookmarks = []
    tree_iter = liststore.get_iter_first()
    while tree_iter != None:
        temp_bookmarks.append(liststore[tree_iter][2])
        tree_iter = liststore.iter_next(tree_iter)
    dialog.destroy()
    if response != Gtk.ResponseType.ACCEPT: return False
    removed_bookmarks = []
    for old_bookmark in dad.bookmarks:
        if not old_bookmark in temp_bookmarks:
            removed_bookmarks.append(old_bookmark)
    dad.bookmarks = temp_bookmarks
    for removed_bookmark in removed_bookmarks:
        tree_iter = dad.get_tree_iter_from_node_id(int(removed_bookmark))
        if tree_iter:
            dad.update_node_aux_icon(tree_iter)
            if dad.curr_tree_iter and dad.treestore[tree_iter][3] == dad.treestore[dad.curr_tree_iter][3]:
                dad.menu_tree_update_for_bookmarked_node(False)
    set_bookmarks_menu_items(dad)
    dad.ctdb_handler.pending_edit_db_bookmarks()
    return True

def set_object_highlight(dad, obj_highl):
    """Set the Highlight to obj_highl only"""
    if dad.highlighted_obj:
        dad.highlighted_obj.drag_unhighlight()
        dad.highlighted_obj = None
    if obj_highl:
        obj_highl.drag_highlight()
        dad.highlighted_obj = obj_highl
