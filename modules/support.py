# -*- coding: UTF-8 -*-
#
#       support.py
#
#       Copyright 2009-2013 Giuseppe Penone <giuspen@gmail.com>
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

import gtk, os, locale, webbrowser
import cons, pgsc_spellcheck


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
        print filepath, "empty lines removed"
        fd = open(filepath, 'w')
        fd.writelines(file_lines)
        fd.close()

def clean_from_chars_not_for_filename(filename_in):
    """Clean a string from chars not good for filename"""
    filename_out = filename_in.replace(cons.CHAR_SLASH, cons.CHAR_MINUS).replace(cons.CHAR_BSLASH, cons.CHAR_MINUS)
    filename_out = filename_out.replace(cons.CHAR_STAR, "").replace(cons.CHAR_QUESTION, "").replace(cons.CHAR_COLON, "")
    filename_out = filename_out.replace(cons.CHAR_LESSER, "").replace(cons.CHAR_GREATER, "")
    filename_out = filename_out.replace(cons.CHAR_PIPE, "").replace(cons.CHAR_DQUOTE, "")
    filename_out = filename_out.replace(cons.CHAR_NEWLINE, "").replace(cons.CHAR_CR, "").strip()
    return filename_out.replace(cons.CHAR_SPACE, cons.CHAR_USCORE)

def get_node_hierarchical_name(dad, tree_iter, separator="--"):
    """Get the Node Hierarchical Name"""
    hierarchical_name = dad.treestore[tree_iter][1].strip()
    father_iter = dad.treestore.iter_parent(tree_iter)
    while father_iter:
        hierarchical_name = dad.treestore[father_iter][1].strip() + separator + hierarchical_name
        father_iter = dad.treestore.iter_parent(father_iter)
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

def get_next_chars_from_iter_are(iter_start, num, chars):
    """Returns True if the Given Chars are the next 'num' after iter"""
    text_iter = iter_start.copy()
    for i in range(num):
        if text_iter.get_char() != chars[i]: return False
        if i != num-1 and not text_iter.forward_char(): return False
    return True

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

def windows_cmd_prepare_path(filepath):
    """Prepares a Path to be digested by windows command line"""
    return cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE

def dialog_file_save_as(filename=None, filter_pattern=None, filter_name=None, curr_folder=None, parent=None):
    """The Save file as dialog, Returns the retrieved filepath or None"""
    chooser = gtk.FileChooserDialog(title=_("Save File as"),
                                    action=gtk.FILE_CHOOSER_ACTION_SAVE,
                                    buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_SAVE, gtk.RESPONSE_ACCEPT) )
    chooser.set_do_overwrite_confirmation(True)
    if parent != None:
        chooser.set_transient_for(parent)
        chooser.set_property("modal", True)
        chooser.set_property("destroy-with-parent", True)
        chooser.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    else: chooser.set_position(gtk.WIN_POS_CENTER)
    if curr_folder == None or os.path.isdir(curr_folder) == False:
        chooser.set_current_folder(os.path.expanduser('~'))
    else:
        chooser.set_current_folder(curr_folder)
    if filename != None:
        chooser.set_current_name(filename)
    if filter_pattern != None:
        filter = gtk.FileFilter()
        filter.set_name(filter_name)
        filter.add_pattern(filter_pattern)
        chooser.add_filter(filter)
    if chooser.run() == gtk.RESPONSE_ACCEPT:
        filepath = chooser.get_filename()
        chooser.destroy()
        return unicode(filepath, cons.STR_UTF8, cons.STR_IGNORE) if filepath != None else None
    else:
        chooser.destroy()
        return None

def dialog_file_select(filter_pattern=[], filter_mime=[], filter_name=None, curr_folder=None, parent=None):
    """The Select file dialog, Returns the retrieved filepath or None"""
    chooser = gtk.FileChooserDialog(title = _("Select File"),
                                    action=gtk.FILE_CHOOSER_ACTION_OPEN,
                                    buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_ACCEPT) )
    if parent != None:
        chooser.set_transient_for(parent)
        chooser.set_property("modal", True)
        chooser.set_property("destroy-with-parent", True)
        chooser.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    else: chooser.set_position(gtk.WIN_POS_CENTER)
    if curr_folder == None or os.path.isdir(curr_folder) == False:
        chooser.set_current_folder(os.path.expanduser('~'))
    else:
        chooser.set_current_folder(curr_folder)
    if filter_pattern or filter_mime:
        filefilter = gtk.FileFilter()
        filefilter.set_name(filter_name)
        for element in filter_pattern:
            filefilter.add_pattern(element)
        for element in filter_mime:
            filefilter.add_mime_type(element)
        chooser.add_filter(filefilter)
    if chooser.run() == gtk.RESPONSE_ACCEPT:
        filepath = chooser.get_filename()
        chooser.destroy()
        return unicode(filepath, cons.STR_UTF8, cons.STR_IGNORE) if filepath != None else None
    else:
        chooser.destroy()
        return None

def dialog_folder_select(curr_folder=None, parent=None):
    """The Select folder dialog, returns the retrieved folderpath or None"""
    chooser = gtk.FileChooserDialog(title = _("Select Folder"),
                                    action=gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                    buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_ACCEPT) )
    if parent != None:
        chooser.set_transient_for(parent)
        chooser.set_property("modal", True)
        chooser.set_property("destroy-with-parent", True)
        chooser.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    else: chooser.set_position(gtk.WIN_POS_CENTER)
    if curr_folder == None or os.path.isdir(curr_folder) == False:
        chooser.set_current_folder(os.path.expanduser('~'))
    else:
        chooser.set_current_folder(curr_folder)
    if chooser.run() == gtk.RESPONSE_ACCEPT:
        folderpath = chooser.get_filename()
        chooser.destroy()
        return unicode(folderpath, cons.STR_UTF8, cons.STR_IGNORE) if folderpath != None else None
    else:
        chooser.destroy()
        return None

def dialog_question(message, parent=None):
    """The Question dialog, returns True if the user presses OK"""
    dialog = gtk.MessageDialog(parent=parent,
                               flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                               type=gtk.MESSAGE_QUESTION,
                               buttons=gtk.BUTTONS_OK_CANCEL,
                               message_format=message)
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_title(_("Question"))
    if dialog.run() == gtk.RESPONSE_ACCEPT:
        dialog.destroy()
        return True
    else:
        dialog.destroy()
        return False

def dialog_info(message, parent):
    """The Info dialog"""
    dialog = gtk.MessageDialog(parent=parent,
                               flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                               type=gtk.MESSAGE_INFO,
                               buttons=gtk.BUTTONS_OK,
                               message_format=message)
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_title(_("Info"))
    dialog.run()
    dialog.destroy()

def dialog_info_after_restart(parent=None):
    """Change Only After Restart"""
    dialog_info(_("This Change will have Effect Only After Restarting CherryTree"), parent)

def dialog_warning(message, parent):
    """The Warning dialog"""
    dialog = gtk.MessageDialog(parent=parent,
                               flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                               type=gtk.MESSAGE_WARNING,
                               buttons=gtk.BUTTONS_OK,
                               message_format=message)
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_title(_("Warning"))
    dialog.run()
    dialog.destroy()

def dialog_error(message, parent):
    """The Error dialog"""
    dialog = gtk.MessageDialog(parent=parent,
                               flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                               type=gtk.MESSAGE_ERROR,
                               buttons=gtk.BUTTONS_OK,
                               message_format=message)
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_title(_("Error"))
    dialog.run()
    dialog.destroy()

def dialog_about(dad):
    """Application About Dialog"""
    dialog = gtk.AboutDialog()
    dialog.set_program_name("CherryTree")
    dialog.set_version(cons.VERSION)
    dialog.set_copyright(_("""Copyright © 2009-2013
Giuseppe Penone <giuspen@gmail.com>"""))
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
    dialog.set_translator_credits(_("""Chinese Simplified (zh_CN) Channing Wong <mamimoluo@gmail.com>
Czech (cs) Pavel Fric <fripohled@blogspot.com>
Dutch (nl) Patrick Vijgeboom <pj.vijgeboom@gmail.com>
French (fr) Benoît D'Angelo <benoit.dangelo@gmx.fr> (former Ludovic Troisi)
German (de) Frank Brungräber <calexu@arcor.de> (former Sven Neubauer)
Italian (it) Giuseppe Penone <giuspen@gmail.com>
Polish (pl) Marcin Swierczynski <orneo1212@gmail.com>
Portuguese Brazil (pt_BR) Vinicius Schmidt <viniciussm@rocketmail.com>
Russian (ru) Andriy Kovtun <kovtunos@yandex.ru>
Spanish (es) Daniel MC <i.e.betel@gmail.com>
Ukrainian (uk) Andriy Kovtun <kovtunos@yandex.ru>"""))
    dialog.set_logo(gtk.gdk.pixbuf_new_from_file(os.path.join(cons.GLADE_PATH, "cherrytree.png")))
    dialog.set_title(_("About CherryTree"))
    dialog.set_transient_for(dad.window)
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_modal(True)
    dialog.run()
    dialog.hide()

def dialog_anchors_list(father_win, title, anchors_list):
    """List Anchors in a Node"""
    class AnchorParms:
        def __init__(self):
            self.sel_iter = None
    anchor_parms = AnchorParms()
    dialog = gtk.Dialog(title=title,
        parent=father_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                 gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_default_size(300, 200)
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    anchors_liststore = gtk.ListStore(str)
    anchors_treeview = gtk.TreeView(anchors_liststore)
    anchors_renderer_text = gtk.CellRendererText()
    anchors_column = gtk.TreeViewColumn(_("Anchor Name"), anchors_renderer_text, text=0)
    anchors_treeview.append_column(anchors_column)
    anchors_treeviewselection = anchors_treeview.get_selection()
    for anchor_element in anchors_list:
        anchors_liststore.append(anchor_element)
    scrolledwindow.add(anchors_treeview)
    anchor_parms.sel_iter = anchors_liststore.get_iter_first()
    if anchor_parms.sel_iter:
        anchors_treeview.set_cursor(anchors_liststore.get_path(anchor_parms.sel_iter))
    content_area = dialog.get_content_area()
    content_area.pack_start(scrolledwindow)
    def on_mouse_button_clicked_anchors_list(widget, event):
        if event.button != 1: return
        if event.type == gtk.gdk._2BUTTON_PRESS:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
    def on_treeview_event_after(treeview, event):
        if event.type not in [gtk.gdk.BUTTON_PRESS, gtk.gdk.KEY_PRESS]: return
        model, anchor_parms.sel_iter = anchors_treeviewselection.get_selected()
    def on_key_press_anchorslistdialog(widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == cons.STR_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        return False
    anchors_treeview.connect('event-after', on_treeview_event_after)
    dialog.connect('key_press_event', on_key_press_anchorslistdialog)
    anchors_treeview.connect('button-press-event', on_mouse_button_clicked_anchors_list)
    content_area.show_all()
    anchors_treeview.grab_focus()
    response = dialog.run()
    dialog.hide()
    if response != gtk.RESPONSE_ACCEPT or not anchor_parms.sel_iter: return ""
    return unicode(anchors_liststore[anchor_parms.sel_iter][0], cons.STR_UTF8, cons.STR_IGNORE)

def dialog_preferences(dad):
    """Preferences Dialog"""
    dialog = gtk.Dialog(title=_("Preferences"),
        parent=dad.window,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CLOSE, gtk.RESPONSE_ACCEPT))
    
    ### ALL NODES
    vbox_all_nodes = gtk.VBox()
    vbox_all_nodes.set_spacing(3)
    
    hbox_tab_width = gtk.HBox()
    label_tab_width = gtk.Label(_("Tab Width"))
    adj_tab_width = gtk.Adjustment(value=dad.tabs_width, lower=1, upper=10000, step_incr=1)
    spinbutton_tab_width = gtk.SpinButton(adj_tab_width)
    hbox_tab_width.pack_start(label_tab_width, expand=False)
    hbox_tab_width.pack_start(spinbutton_tab_width, expand=False)
    checkbutton_spaces_tabs = gtk.CheckButton(label=_("Insert Spaces Instead of Tabs"))
    checkbutton_spaces_tabs.set_active(dad.spaces_instead_tabs)
    checkbutton_line_wrap = gtk.CheckButton(label=_("Use Line Wrapping"))
    checkbutton_line_wrap.set_active(dad.line_wrapping)
    checkbutton_auto_indent = gtk.CheckButton(label=_("Enable Automatic Indentation"))
    checkbutton_auto_indent.set_active(dad.auto_indent)
    checkbutton_line_nums = gtk.CheckButton(label=_("Show Line Numbers"))
    checkbutton_line_nums.set_active(dad.show_line_numbers)
    
    vbox_text_editor = gtk.VBox()
    vbox_text_editor.pack_start(hbox_tab_width, expand=False)
    vbox_text_editor.pack_start(checkbutton_spaces_tabs, expand=False)
    vbox_text_editor.pack_start(checkbutton_line_wrap, expand=False)
    vbox_text_editor.pack_start(checkbutton_auto_indent, expand=False)
    vbox_text_editor.pack_start(checkbutton_line_nums, expand=False)
    frame_text_editor = gtk.Frame(label="<b>"+_("Text Editor")+"</b>")
    frame_text_editor.get_label_widget().set_use_markup(True)
    frame_text_editor.set_shadow_type(gtk.SHADOW_NONE)
    frame_text_editor.add(vbox_text_editor)
    
    hbox_timestamp = gtk.HBox()
    label_timestamp = gtk.Label(_("Timestamp Format"))
    entry_timestamp_format = gtk.Entry()
    entry_timestamp_format.set_text(dad.timestamp_format)
    button_strftime_help = gtk.Button()
    button_strftime_help.set_image(gtk.image_new_from_stock("gtk-help", gtk.ICON_SIZE_BUTTON))
    hbox_timestamp.pack_start(label_timestamp, expand=False)
    hbox_timestamp.pack_start(entry_timestamp_format)
    hbox_timestamp.pack_start(button_strftime_help, expand=False)
    hbox_horizontal_rule = gtk.HBox()
    label_horizontal_rule = gtk.Label(_("Horizontal Rule"))
    entry_horizontal_rule = gtk.Entry()
    entry_horizontal_rule.set_text(dad.h_rule)
    hbox_horizontal_rule.pack_start(label_horizontal_rule, expand=False)
    hbox_horizontal_rule.pack_start(entry_horizontal_rule)
    hbox_special_chars = gtk.HBox()
    label_special_chars = gtk.Label(_("Special Characters"))
    frame_special_chars = gtk.Frame()
    frame_special_chars.set_size_request(-1, 80)
    frame_special_chars.set_shadow_type(gtk.SHADOW_IN)
    scrolledwindow_special_chars = gtk.ScrolledWindow()
    scrolledwindow_special_chars.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    frame_special_chars.add(scrolledwindow_special_chars)
    textbuffer_special_chars = gtk.TextBuffer()
    textbuffer_special_chars.set_text(dad.special_chars)
    textview_special_chars = gtk.TextView(buffer=textbuffer_special_chars)
    textview_special_chars.set_wrap_mode(gtk.WRAP_CHAR)
    scrolledwindow_special_chars.add(textview_special_chars)
    hbox_special_chars.pack_start(label_special_chars, expand=False)
    hbox_special_chars.pack_start(frame_special_chars)
    
    vbox_misc_all = gtk.VBox()
    vbox_misc_all.pack_start(hbox_timestamp)
    vbox_misc_all.pack_start(hbox_horizontal_rule)
    vbox_misc_all.pack_start(hbox_special_chars)
    frame_misc_all = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_all.get_label_widget().set_use_markup(True)
    frame_misc_all.set_shadow_type(gtk.SHADOW_NONE)
    frame_misc_all.add(vbox_misc_all)
    
    vbox_all_nodes.pack_start(frame_text_editor)
    vbox_all_nodes.pack_start(frame_misc_all)
    def on_spinbutton_tab_width_value_changed(spinbutton):
        dad.tabs_width = int(spinbutton.get_value())
        dad.sourceview.set_tab_width(dad.tabs_width)
    spinbutton_tab_width.connect('value-changed', on_spinbutton_tab_width_value_changed)
    def on_checkbutton_spaces_tabs_toggled(checkbutton):
        dad.spaces_instead_tabs = checkbutton.get_active()
        dad.sourceview.set_insert_spaces_instead_of_tabs(dad.spaces_instead_tabs)
    checkbutton_spaces_tabs.connect('toggled', on_checkbutton_spaces_tabs_toggled)
    def on_checkbutton_line_wrap_toggled(checkbutton):
        dad.line_wrapping = checkbutton.get_active()
        dad.sourceview.set_wrap_mode(gtk.WRAP_WORD if dad.line_wrapping else gtk.WRAP_NONE)
    checkbutton_line_wrap.connect('toggled', on_checkbutton_line_wrap_toggled)
    def on_checkbutton_auto_indent_toggled(checkbutton):
        dad.auto_indent = checkbutton.get_active()
    checkbutton_auto_indent.connect('toggled', on_checkbutton_auto_indent_toggled)
    def on_checkbutton_line_nums_toggled(checkbutton):
        dad.show_line_numbers = checkbutton.get_active()
        dad.sourceview.set_show_line_numbers(dad.show_line_numbers)
    checkbutton_line_nums.connect('toggled', on_checkbutton_line_nums_toggled)
    def on_entry_timestamp_format_changed(entry):
        dad.timestamp_format = entry.get_text()
    entry_timestamp_format.connect('changed', on_entry_timestamp_format_changed)
    def on_button_strftime_help_clicked(menuitem, data=None):
        lang_code = locale.getdefaultlocale()[0]
        if lang_code:
            page_lang = lang_code[0:2] if lang_code[0:2] in ["de", "es", "fr"] else ""
        else: page_lang = ""
        webbrowser.open("http://man.cx/strftime(3)/" + page_lang)
    button_strftime_help.connect('clicked', on_button_strftime_help_clicked)
    def on_entry_horizontal_rule_changed(entry):
        dad.h_rule = entry.get_text()
    entry_horizontal_rule.connect('changed', on_entry_horizontal_rule_changed)
    
    ### TEXT NODES
    vbox_text_nodes = gtk.VBox()
    vbox_text_nodes.set_spacing(3)
    
    vbox_spell_check = gtk.VBox()
    checkbutton_spell_check = gtk.CheckButton(label=_("Enable Spell Check"))
    checkbutton_spell_check.set_active(dad.enable_spell_check)
    hbox_spell_check_lang = gtk.HBox()
    label_spell_check_lang = gtk.Label(_("Spell Check Language"))
    combobox_spell_check_lang = gtk.ComboBox()
    cell = gtk.CellRendererText()
    combobox_spell_check_lang.pack_start(cell, True)
    combobox_spell_check_lang.add_attribute(cell, 'text', 0)
    if dad.spell_check_init:
        combobox_spell_check_lang.set_model(dad.spell_check_lang_liststore)
        combobox_spell_check_lang.set_active_iter(dad.get_combobox_iter_from_value(dad.spell_check_lang_liststore, 0, dad.spell_check_lang))
    hbox_spell_check_lang.pack_start(label_spell_check_lang, expand=False)
    hbox_spell_check_lang.pack_start(combobox_spell_check_lang)
    vbox_spell_check.pack_start(checkbutton_spell_check, expand=False)
    vbox_spell_check.pack_start(hbox_spell_check_lang, expand=False)
    frame_spell_check = gtk.Frame(label="<b>"+_("Spell Check")+"</b>")
    frame_spell_check.get_label_widget().set_use_markup(True)
    frame_spell_check.set_shadow_type(gtk.SHADOW_NONE)
    frame_spell_check.add(vbox_spell_check)
    
    vbox_rt_theme = gtk.VBox()
    
    radiobutton_rt_col_light = gtk.RadioButton(label=_("Light Background, Dark Text"))
    radiobutton_rt_col_dark = gtk.RadioButton(label=_("Dark Background, Light Text"))
    radiobutton_rt_col_dark.set_group(radiobutton_rt_col_light)
    radiobutton_rt_col_custom = gtk.RadioButton(label=_("Custom Background"))
    radiobutton_rt_col_custom.set_group(radiobutton_rt_col_light)
    hbox_rt_col_custom = gtk.HBox()
    colorbutton_text_bg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.rt_def_bg))
    label_rt_col_custom = gtk.Label(_("and Text"))
    colorbutton_text_fg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.rt_def_fg))
    hbox_rt_col_custom.pack_start(radiobutton_rt_col_custom, expand=False)
    hbox_rt_col_custom.pack_start(colorbutton_text_bg, expand=False)
    hbox_rt_col_custom.pack_start(label_rt_col_custom, expand=False)
    hbox_rt_col_custom.pack_start(colorbutton_text_fg, expand=False)
    
    vbox_rt_theme.pack_start(radiobutton_rt_col_light, expand=False)
    vbox_rt_theme.pack_start(radiobutton_rt_col_dark, expand=False)
    vbox_rt_theme.pack_start(hbox_rt_col_custom, expand=False)
    frame_rt_theme = gtk.Frame(label="<b>"+_("Theme")+"</b>")
    frame_rt_theme.get_label_widget().set_use_markup(True)
    frame_rt_theme.set_shadow_type(gtk.SHADOW_NONE)
    frame_rt_theme.add(vbox_rt_theme)
    
    if dad.rt_def_fg == cons.RICH_TEXT_DARK_FG and dad.rt_def_bg == cons.RICH_TEXT_DARK_BG:
        radiobutton_rt_col_dark.set_active(True)
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
    elif dad.rt_def_fg == cons.RICH_TEXT_LIGHT_FG and dad.rt_def_bg == cons.RICH_TEXT_LIGHT_BG:
        radiobutton_rt_col_light.set_active(True)
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
    else: radiobutton_rt_col_custom.set_active(True)
    
    hbox_misc_text = gtk.HBox()
    label_limit_undoable_steps = gtk.Label(_("Limit of Undoable Steps Per Node"))
    adj_limit_undoable_steps = gtk.Adjustment(value=dad.limit_undoable_steps, lower=1, upper=10000, step_incr=1)
    spinbutton_limit_undoable_steps = gtk.SpinButton(adj_limit_undoable_steps)
    hbox_misc_text.pack_start(label_limit_undoable_steps, expand=False)
    hbox_misc_text.pack_start(spinbutton_limit_undoable_steps)
    
    vbox_misc_text = gtk.VBox()
    vbox_misc_text.pack_start(hbox_misc_text, expand=False)
    frame_misc_text = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_text.get_label_widget().set_use_markup(True)
    frame_misc_text.set_shadow_type(gtk.SHADOW_NONE)
    frame_misc_text.add(vbox_misc_text)
    
    vbox_text_nodes.pack_start(frame_spell_check)
    vbox_text_nodes.pack_start(frame_rt_theme)
    vbox_text_nodes.pack_start(frame_misc_text)
    def on_checkbutton_spell_check_toggled(checkbutton):
        dad.enable_spell_check = checkbutton.get_active()
        if dad.enable_spell_check: dad.spell_check_set_on()
        else: dad.spell_check_set_off()
        combobox_spell_check_lang.set_sensitive(dad.enable_spell_check)
    checkbutton_spell_check.connect('toggled', on_checkbutton_spell_check_toggled)
    def on_combobox_spell_check_lang_changed(combobox):
        new_iter = combobox.get_active_iter()
        new_lang_code = dad.spell_check_lang_liststore[new_iter][0]
        if new_lang_code != dad.spell_check_lang: dad.spell_check_set_new_lang(new_lang_code)
    combobox_spell_check_lang.connect('changed', on_combobox_spell_check_lang_changed)
    def on_colorbutton_text_fg_color_set(colorbutton):
        dad.rt_def_fg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.rt_def_fg))
    colorbutton_text_fg.connect('color-set', on_colorbutton_text_fg_color_set)
    def on_colorbutton_text_bg_color_set(colorbutton):
        dad.rt_def_bg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.rt_def_bg))
    colorbutton_text_bg.connect('color-set', on_colorbutton_text_bg_color_set)
    def on_radiobutton_rt_col_light_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_text_fg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_LIGHT_FG))
        colorbutton_text_bg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_LIGHT_BG))
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
        on_colorbutton_text_fg_color_set(colorbutton_text_fg)
        on_colorbutton_text_bg_color_set(colorbutton_text_bg)
    radiobutton_rt_col_light.connect('toggled', on_radiobutton_rt_col_light_toggled)
    def on_radiobutton_rt_col_dark_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_text_fg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_DARK_FG))
        colorbutton_text_bg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_DARK_BG))
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
        on_colorbutton_text_fg_color_set(colorbutton_text_fg)
        on_colorbutton_text_bg_color_set(colorbutton_text_bg)
    radiobutton_rt_col_dark.connect('toggled', on_radiobutton_rt_col_dark_toggled)
    def on_radiobutton_rt_col_custom_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_text_fg.set_sensitive(True)
        colorbutton_text_bg.set_sensitive(True)
    radiobutton_rt_col_custom.connect('toggled', on_radiobutton_rt_col_custom_toggled)
    def on_spinbutton_limit_undoable_steps_value_changed(spinbutton):
        dad.limit_undoable_steps = int(spinbutton.get_value())
    spinbutton_limit_undoable_steps.connect('value-changed', on_spinbutton_limit_undoable_steps_value_changed)
    
    if not pgsc_spellcheck.HAS_PYENCHANT:
        checkbutton_spell_check.set_sensitive(False)
        combobox_spell_check_lang.set_sensitive(False)
    
    ### CODE NODES
    vbox_code_nodes = gtk.VBox()
    vbox_code_nodes.set_spacing(3)
    
    vbox_syntax = gtk.VBox()
    hbox_style_scheme = gtk.HBox()
    label_style_scheme = gtk.Label(_("Style Scheme"))
    combobox_style_scheme = gtk.ComboBox(model=dad.style_scheme_liststore)
    cell = gtk.CellRendererText()
    combobox_style_scheme.pack_start(cell, True)
    combobox_style_scheme.add_attribute(cell, 'text', 0)
    combobox_style_scheme.set_active_iter(dad.get_combobox_iter_from_value(dad.style_scheme_liststore, 0, dad.style_scheme))
    hbox_style_scheme.pack_start(label_style_scheme, expand=False)
    hbox_style_scheme.pack_start(combobox_style_scheme)
    checkbutton_show_white_spaces = gtk.CheckButton(_("Show White Spaces"))
    checkbutton_show_white_spaces.set_active(dad.show_white_spaces)
    checkbutton_highlight_current_line = gtk.CheckButton(_("Highlight Current Line"))
    checkbutton_highlight_current_line.set_active(dad.highl_curr_line)
    
    vbox_syntax.pack_start(hbox_style_scheme, expand=False)
    vbox_syntax.pack_start(checkbutton_show_white_spaces, expand=False)
    vbox_syntax.pack_start(checkbutton_highlight_current_line, expand=False)
    
    frame_syntax = gtk.Frame(label="<b>"+_("Automatic Syntax Highlighting")+"</b>")
    frame_syntax.get_label_widget().set_use_markup(True)
    frame_syntax.set_shadow_type(gtk.SHADOW_NONE)
    frame_syntax.add(vbox_syntax)
    
    vbox_code_nodes.pack_start(frame_syntax)
    def on_combobox_style_scheme_changed(combobox):
        new_iter = combobox_style_scheme.get_active_iter()
        new_style = dad.style_scheme_liststore[new_iter][0]
        if new_style != dad.style_scheme:
            dad.style_scheme = new_style
            support.dialog_info_after_restart(dad.window)
    combobox_style_scheme.connect('changed', on_combobox_style_scheme_changed)
    def on_checkbutton_show_white_spaces_toggled(checkbutton):
        dad.show_white_spaces = checkbutton.get_active()
        support.dialog_info_after_restart(dad.window)
    checkbutton_show_white_spaces.connect('toggled', on_checkbutton_show_white_spaces_toggled)
    def on_checkbutton_highlight_current_line_toggled(checkbutton):
        dad.highl_curr_line = checkbutton.get_active()
        support.dialog_info_after_restart(dad.window)
    checkbutton_highlight_current_line.connect('toggled', on_checkbutton_highlight_current_line_toggled)
    
    ### TREE
    vbox_tree = gtk.VBox()
    vbox_tree.set_spacing(3)
    
    vbox_tt_theme = gtk.VBox()
    
    radiobutton_tt_col_light = gtk.RadioButton(label=_("Light Background, Dark Text"))
    radiobutton_tt_col_dark = gtk.RadioButton(label=_("Dark Background, Light Text"))
    radiobutton_tt_col_dark.set_group(radiobutton_tt_col_light)
    radiobutton_tt_col_custom = gtk.RadioButton(label=_("Custom Background"))
    radiobutton_tt_col_custom.set_group(radiobutton_tt_col_light)
    hbox_tt_col_custom = gtk.HBox()
    colorbutton_tree_bg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.tt_def_bg))
    label_tt_col_custom = gtk.Label(_("and Text"))
    colorbutton_tree_fg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.tt_def_fg))
    hbox_tt_col_custom.pack_start(radiobutton_tt_col_custom, expand=False)
    hbox_tt_col_custom.pack_start(colorbutton_tree_bg, expand=False)
    hbox_tt_col_custom.pack_start(label_tt_col_custom, expand=False)
    hbox_tt_col_custom.pack_start(colorbutton_tree_fg, expand=False)
    
    vbox_tt_theme.pack_start(radiobutton_tt_col_light, expand=False)
    vbox_tt_theme.pack_start(radiobutton_tt_col_dark, expand=False)
    vbox_tt_theme.pack_start(hbox_tt_col_custom, expand=False)
    frame_tt_theme = gtk.Frame(label="<b>"+_("Theme")+"</b>")
    frame_tt_theme.get_label_widget().set_use_markup(True)
    frame_tt_theme.set_shadow_type(gtk.SHADOW_NONE)
    frame_tt_theme.add(vbox_tt_theme)
    
    if dad.tt_def_fg == cons.TREE_TEXT_DARK_FG and dad.tt_def_bg == cons.TREE_TEXT_DARK_BG:
        radiobutton_tt_col_dark.set_active(True)
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
    elif dad.tt_def_fg == cons.TREE_TEXT_LIGHT_FG and dad.tt_def_bg == cons.TREE_TEXT_LIGHT_BG:
        radiobutton_tt_col_light.set_active(True)
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
    else: radiobutton_tt_col_custom.set_active(True)
    
    vbox_nodes_icons = gtk.VBox()
    
    radiobutton_node_icon_cherry = gtk.RadioButton(label=_("Use Cherries as Nodes Icons"))
    radiobutton_node_icon_bullet = gtk.RadioButton(label=_("Use Bullets as Nodes Icons"))
    radiobutton_node_icon_bullet.set_group(radiobutton_node_icon_cherry)
    radiobutton_node_icon_none = gtk.RadioButton(label=_("Do Not Display Nodes Icons"))
    radiobutton_node_icon_none.set_group(radiobutton_node_icon_cherry)
    
    vbox_nodes_icons.pack_start(radiobutton_node_icon_cherry, expand=False)
    vbox_nodes_icons.pack_start(radiobutton_node_icon_bullet, expand=False)
    vbox_nodes_icons.pack_start(radiobutton_node_icon_none, expand=False)
    frame_nodes_icons = gtk.Frame(label="<b>"+_("Nodes Icons")+"</b>")
    frame_nodes_icons.get_label_widget().set_use_markup(True)
    frame_nodes_icons.set_shadow_type(gtk.SHADOW_NONE)
    frame_nodes_icons.add(vbox_nodes_icons)
    
    radiobutton_node_icon_cherry.set_active(dad.nodes_icons == "c")
    radiobutton_node_icon_bullet.set_active(dad.nodes_icons == "b")
    radiobutton_node_icon_none.set_active(dad.nodes_icons == "n")
    
    vbox_nodes_startup = gtk.VBox()
    
    radiobutton_nodes_startup_restore = gtk.RadioButton(label=_("Restore Expanded/Collapsed Status"))
    radiobutton_nodes_startup_expand = gtk.RadioButton(label=_("Expand all Nodes"))
    radiobutton_nodes_startup_expand.set_group(radiobutton_nodes_startup_restore)
    radiobutton_nodes_startup_collapse = gtk.RadioButton(label=_("Collapse all Nodes"))
    radiobutton_nodes_startup_collapse.set_group(radiobutton_nodes_startup_restore)
    
    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_restore, expand=False)
    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_expand, expand=False)
    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_collapse, expand=False)
    frame_nodes_startup = gtk.Frame(label="<b>"+_("Nodes Status at Startup")+"</b>")
    frame_nodes_startup.get_label_widget().set_use_markup(True)
    frame_nodes_startup.set_shadow_type(gtk.SHADOW_NONE)
    frame_nodes_startup.add(vbox_nodes_startup)
    
    radiobutton_nodes_startup_restore.set_active(dad.rest_exp_coll == 0)
    radiobutton_nodes_startup_expand.set_active(dad.rest_exp_coll == 1)
    radiobutton_nodes_startup_collapse.set_active(dad.rest_exp_coll == 2)
    
    vbox_misc_tree = gtk.VBox()
    hbox_tree_nodes_names_width = gtk.HBox()
    label_tree_nodes_names_width = gtk.Label(_("Tree Nodes Names Wrapping Width"))
    adj_tree_nodes_names_width = gtk.Adjustment(value=dad.cherry_wrap_width, lower=10, upper=10000, step_incr=1)
    spinbutton_tree_nodes_names_width = gtk.SpinButton(adj_tree_nodes_names_width)
    hbox_tree_nodes_names_width.pack_start(label_tree_nodes_names_width, expand=False)
    hbox_tree_nodes_names_width.pack_start(spinbutton_tree_nodes_names_width)
    checkbutton_tree_right_side = gtk.CheckButton(_("Display Tree on the Right Side"))
    checkbutton_tree_right_side.set_active(dad.tree_right_side)
    
    vbox_misc_tree.pack_start(hbox_tree_nodes_names_width, expand=False)
    vbox_misc_tree.pack_start(checkbutton_tree_right_side, expand=False)
    frame_misc_tree = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_tree.get_label_widget().set_use_markup(True)
    frame_misc_tree.set_shadow_type(gtk.SHADOW_NONE)
    frame_misc_tree.add(vbox_misc_tree)
    
    vbox_tree.pack_start(frame_tt_theme)
    vbox_tree.pack_start(frame_nodes_icons)
    vbox_tree.pack_start(frame_nodes_startup)
    def on_colorbutton_tree_fg_color_set(colorbutton):
        dad.tt_def_fg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        dad.treeview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.tt_def_fg))
        if dad.curr_tree_iter: dad.update_node_name_header()
    colorbutton_tree_fg.connect('color-set', on_colorbutton_tree_fg_color_set)
    def on_colorbutton_tree_bg_color_set(colorbutton):
        dad.tt_def_bg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        dad.treeview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.tt_def_bg))
        if dad.curr_tree_iter: dad.update_node_name_header()
    colorbutton_tree_bg.connect('color-set', on_colorbutton_tree_bg_color_set)
    def on_radiobutton_tt_col_light_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_tree_fg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_LIGHT_FG))
        colorbutton_tree_bg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_LIGHT_BG))
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
        on_colorbutton_tree_fg_color_set(colorbutton_tree_fg)
        on_colorbutton_tree_bg_color_set(colorbutton_tree_bg)
    radiobutton_tt_col_light.connect('toggled', on_radiobutton_tt_col_light_toggled)
    def on_radiobutton_tt_col_dark_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_tree_fg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_DARK_FG))
        colorbutton_tree_bg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_DARK_BG))
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
        on_colorbutton_tree_fg_color_set(colorbutton_tree_fg)
        on_colorbutton_tree_bg_color_set(colorbutton_tree_bg)
    radiobutton_tt_col_dark.connect('toggled', on_radiobutton_tt_col_dark_toggled)
    def on_radiobutton_tt_col_custom_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_tree_fg.set_sensitive(True)
        colorbutton_tree_bg.set_sensitive(True)
    radiobutton_tt_col_custom.connect('toggled', on_radiobutton_tt_col_custom_toggled)
    def on_radiobutton_node_icon_cherry_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "c"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_cherry.connect('toggled', on_radiobutton_node_icon_cherry_toggled)
    def on_radiobutton_node_icon_bullet_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "b"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_bullet.connect('toggled', on_radiobutton_node_icon_bullet_toggled)
    def on_radiobutton_node_icon_none_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "n"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_none.connect('toggled', on_radiobutton_node_icon_none_toggled)
    def on_radiobutton_nodes_startup_restore_toggled(checkbutton):
        if checkbutton.get_active(): dad.rest_exp_coll = 0
    radiobutton_nodes_startup_restore.connect('toggled', on_radiobutton_nodes_startup_restore_toggled)
    def on_radiobutton_nodes_startup_expand_toggled(checkbutton):
        if checkbutton.get_active(): dad.rest_exp_coll = 1
    radiobutton_nodes_startup_expand.connect('toggled', on_radiobutton_nodes_startup_expand_toggled)
    def on_radiobutton_nodes_startup_collapse_toggled(checkbutton):
        if checkbutton.get_active(): dad.rest_exp_coll = 2
    radiobutton_nodes_startup_collapse.connect('toggled', on_radiobutton_nodes_startup_collapse_toggled)
    def on_spinbutton_tree_nodes_names_width_value_changed(spinbutton):
        dad.cherry_wrap_width = int(spinbutton.get_value())
        dad.renderer_text.set_property('wrap-width', dad.cherry_wrap_width)
        dad.treeview_refresh()
    spinbutton_tree_nodes_names_width.connect('value-changed', on_spinbutton_tree_nodes_names_width_value_changed)
    def on_checkbutton_tree_right_side_toggled(checkbutton):
        dad.tree_right_side = checkbutton.get_active()
        tree_width = dad.scrolledwindow_tree.get_allocation().width
        text_width = dad.vbox_text.get_allocation().width
        dad.hpaned.remove(dad.scrolledwindow_tree)
        dad.hpaned.remove(dad.vbox_text)
        if dad.tree_right_side:
            dad.hpaned.add1(dad.vbox_text)
            dad.hpaned.add2(dad.scrolledwindow_tree)
            dad.hpaned.set_property('position', text_width)
        else:
            dad.hpaned.add1(dad.scrolledwindow_tree)
            dad.hpaned.add2(dad.vbox_text)
            dad.hpaned.set_property('position', tree_width)
    checkbutton_tree_right_side.connect('toggled', on_checkbutton_tree_right_side_toggled)
    
    ### FONTS
    vbox_fonts = gtk.VBox()
    vbox_fonts.set_spacing(3)
    
    image_text = gtk.Image()
    image_text.set_from_stock(gtk.STOCK_SELECT_FONT, gtk.ICON_SIZE_MENU)
    image_code = gtk.Image()
    image_code.set_from_stock(gtk.STOCK_SELECT_FONT, gtk.ICON_SIZE_MENU)
    image_tree = gtk.Image()
    image_tree.set_from_stock('cherries', gtk.ICON_SIZE_MENU)
    label_text = gtk.Label(_("Text Font"))
    label_code = gtk.Label(_("Code Font"))
    label_tree = gtk.Label(_("Tree Font"))
    fontbutton_text = gtk.FontButton(fontname=dad.text_font)
    fontbutton_code = gtk.FontButton(fontname=dad.code_font)
    fontbutton_tree = gtk.FontButton(fontname=dad.tree_font)
    table_fonts = gtk.Table(3, 3)
    table_fonts.set_row_spacings(2)
    table_fonts.attach(image_text, 0, 1, 0, 1, 0, 0)
    table_fonts.attach(image_code, 0, 1, 1, 2, 0, 0)
    table_fonts.attach(image_tree, 0, 1, 2, 3, 0, 0)
    table_fonts.attach(label_text, 1, 2, 0, 1, 0, 0)
    table_fonts.attach(label_code, 1, 2, 1, 2, 0, 0)
    table_fonts.attach(label_tree, 1, 2, 2, 3, 0, 0)
    table_fonts.attach(fontbutton_text, 2, 3, 0, 1)
    table_fonts.attach(fontbutton_code, 2, 3, 1, 2)
    table_fonts.attach(fontbutton_tree, 2, 3, 2, 3)
    
    frame_fonts = gtk.Frame(label="<b>"+_("Fonts")+"</b>")
    frame_fonts.get_label_widget().set_use_markup(True)
    frame_fonts.set_shadow_type(gtk.SHADOW_NONE)
    frame_fonts.add(table_fonts)
    
    vbox_fonts.pack_start(frame_fonts)
    def on_fontbutton_text_font_set(picker):
        dad.text_font = picker.get_font_name()
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_font(pango.FontDescription(dad.text_font))
    fontbutton_text.connect('font-set', on_fontbutton_text_font_set)
    def on_fontbutton_code_font_set(picker):
        dad.code_font = picker.get_font_name()
        if dad.curr_tree_iter and dad.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_font(pango.FontDescription(dad.code_font))
    fontbutton_code.connect('font-set', on_fontbutton_code_font_set)
    def on_fontbutton_tree_font_set(picker):
        dad.tree_font = picker.get_font_name()
        dad.set_treeview_font()
    fontbutton_tree.connect('font-set', on_fontbutton_tree_font_set)
    
    ### LINKS
    vbox_links = gtk.VBox()
    vbox_links.set_spacing(3)
    
    vbox_links_actions = gtk.VBox()
    checkbutton_custom_weblink_cmd = gtk.CheckButton(_("Enable Custom Web Link Clicked Action"))
    entry_custom_weblink_cmd = gtk.Entry()
    checkbutton_custom_filelink_cmd = gtk.CheckButton(_("Enable Custom File Link Clicked Action"))
    entry_custom_filelink_cmd = gtk.Entry()
    checkbutton_custom_folderlink_cmd = gtk.CheckButton(_("Enable Custom Folder Link Clicked Action"))
    entry_custom_folderlink_cmd = gtk.Entry()
    vbox_links_actions.pack_start(checkbutton_custom_weblink_cmd, expand=False)
    vbox_links_actions.pack_start(entry_custom_weblink_cmd, expand=False)
    vbox_links_actions.pack_start(checkbutton_custom_filelink_cmd, expand=False)
    vbox_links_actions.pack_start(entry_custom_filelink_cmd, expand=False)
    vbox_links_actions.pack_start(checkbutton_custom_folderlink_cmd, expand=False)
    vbox_links_actions.pack_start(entry_custom_folderlink_cmd, expand=False)
    
    frame_links_actions = gtk.Frame(label="<b>"+_("Custom Actions")+"</b>")
    frame_links_actions.get_label_widget().set_use_markup(True)
    frame_links_actions.set_shadow_type(gtk.SHADOW_NONE)
    frame_links_actions.add(vbox_links_actions)
    
    checkbutton_custom_weblink_cmd.set_active(dad.weblink_custom_action[0])
    entry_custom_weblink_cmd.set_sensitive(dad.weblink_custom_action[0])
    entry_custom_weblink_cmd.set_text(dad.weblink_custom_action[1])
    checkbutton_custom_filelink_cmd.set_active(dad.filelink_custom_action[0])
    entry_custom_filelink_cmd.set_sensitive(dad.filelink_custom_action[0])
    entry_custom_filelink_cmd.set_text(dad.filelink_custom_action[1])
    checkbutton_custom_folderlink_cmd.set_active(dad.folderlink_custom_action[0])
    entry_custom_folderlink_cmd.set_sensitive(dad.folderlink_custom_action[0])
    entry_custom_folderlink_cmd.set_text(dad.folderlink_custom_action[1])
    
    vbox_links_colors = gtk.VBox()
    frame_links_colors = gtk.Frame(label="<b>"+_("Colors")+"</b>")
    frame_links_colors.get_label_widget().set_use_markup(True)
    frame_links_colors.set_shadow_type(gtk.SHADOW_NONE)
    frame_links_colors.add(vbox_links_colors)
    
    vbox_links_misc = gtk.VBox()
    hbox_anchor_size = gtk.HBox()
    label_anchor_size = gtk.Label(_("Anchor Size"))
    adj_anchor_size = gtk.Adjustment(value=dad.anchor_size, lower=1, upper=1000, step_incr=1)
    spinbutton_anchor_size = gtk.SpinButton(adj_anchor_size)
    hbox_anchor_size.pack_start(label_anchor_size, expand=False)
    hbox_anchor_size.pack_start(spinbutton_anchor_size, expand=False)
    vbox_links_misc.pack_start(hbox_anchor_size, expand=False)
    
    frame_links_misc = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_links_misc.get_label_widget().set_use_markup(True)
    frame_links_misc.set_shadow_type(gtk.SHADOW_NONE)
    frame_links_misc.add(vbox_links_misc)
    
    vbox_links.pack_start(frame_links_actions, expand=False)
    vbox_links.pack_start(frame_links_colors, expand=False)
    vbox_links.pack_start(frame_links_misc, expand=False)
    def on_checkbutton_custom_weblink_cmd_toggled(checkbutton):
        dad.weblink_custom_action[0] = checkbutton.get_active()
        entry_custom_weblink_cmd.set_sensitive(dad.weblink_custom_action[0])
    checkbutton_custom_weblink_cmd.connect('toggled', on_checkbutton_custom_weblink_cmd_toggled)
    def on_entry_custom_weblink_cmd_changed(entry):
        dad.weblink_custom_action[1] = entry.get_text()
    entry_custom_weblink_cmd.connect('changed', on_entry_custom_weblink_cmd_changed)
    def on_checkbutton_custom_filelink_cmd_toggled(checkbutton):
        dad.filelink_custom_action[0] = checkbutton.get_active()
        entry_custom_filelink_cmd.set_sensitive(dad.filelink_custom_action[0])
    checkbutton_custom_filelink_cmd.connect('toggled', on_checkbutton_custom_filelink_cmd_toggled)
    def on_entry_custom_filelink_cmd_changed(entry):
        dad.filelink_custom_action[1] = entry.get_text()
    entry_custom_filelink_cmd.connect('changed', on_entry_custom_filelink_cmd_changed)
    def on_checkbutton_custom_folderlink_cmd_toggled(checkbutton):
        dad.folderlink_custom_action[0] = checkbutton.get_active()
        entry_custom_folderlink_cmd.set_sensitive(dad.folderlink_custom_action[0])
    checkbutton_custom_folderlink_cmd.connect('toggled', on_checkbutton_custom_folderlink_cmd_toggled)
    def on_entry_custom_folderlink_cmd_changed(entry):
        dad.folderlink_custom_action[1] = entry.get_text()
    entry_custom_folderlink_cmd.connect('changed', on_entry_custom_folderlink_cmd_changed)
    def on_spinbutton_anchor_size_value_changed(spinbutton):
        dad.anchor_size = int(spinbutton_anchor_size.get_value())
    spinbutton_anchor_size.connect('value-changed', on_spinbutton_anchor_size_value_changed)
    
    ### MISCELLANEOUS
    vbox_misc = gtk.VBox()
    vbox_misc.set_spacing(3)
    
    vbox_system_tray = gtk.VBox()
    checkbutton_systray = gtk.CheckButton(_("Enable System Tray Docking"))
    checkbutton_start_on_systray = gtk.CheckButton(_("Start Minimized in the System Tray"))
    checkbutton_use_appind = gtk.CheckButton(_("Use AppIndicator for Docking"))
    vbox_system_tray.pack_start(checkbutton_systray, expand=False)
    vbox_system_tray.pack_start(checkbutton_start_on_systray, expand=False)
    vbox_system_tray.pack_start(checkbutton_use_appind, expand=False)
    
    frame_system_tray = gtk.Frame(label="<b>"+_("System Tray")+"</b>")
    frame_system_tray.get_label_widget().set_use_markup(True)
    frame_system_tray.set_shadow_type(gtk.SHADOW_NONE)
    frame_system_tray.add(vbox_system_tray)
    
    checkbutton_systray.set_active(dad.systray)
    checkbutton_start_on_systray.set_active(dad.start_on_systray)
    checkbutton_start_on_systray.set_sensitive(dad.systray)
    checkbutton_use_appind.set_active(dad.use_appind)
    if not cons.HAS_APPINDICATOR or not cons.HAS_SYSTRAY: checkbutton_use_appind.set_sensitive(False)
    
    vbox_saving = gtk.VBox()
    hbox_autosave = gtk.HBox()
    hbox_autosave.set_spacing(2)
    checkbutton_autosave = gtk.CheckButton(_("Autosave Every"))
    adjustment_autosave = gtk.Adjustment(value=img_parms.height, lower=1, upper=1000, step_incr=1)
    spinbutton_autosave = gtk.SpinButton(adjustment_autosave)
    label_autosave = gtk.Label(_("Minutes"))
    hbox_autosave.pack_start(checkbutton_autosave, expand=False)
    hbox_autosave.pack_start(spinbutton_autosave, expand=False)
    hbox_autosave.pack_start(label_autosave)
    checkbutton_autosave_on_quit = gtk.CheckButton(_("Autosave on Quit"))
    checkbutton_backup_before_saving = gtk.CheckButton(_("Create a Backup Copy Before Saving"))
    vbox_saving.pack_start(hbox_autosave, expand=False)
    vbox_saving.pack_start(checkbutton_autosave_on_quit, expand=False)
    vbox_saving.pack_start(checkbutton_backup_before_saving, expand=False)
    
    checkbutton_autosave.set_active(dad.autosave[0])
    spinbutton_autosave.set_value(dad.autosave[1])
    spinbutton_autosave.set_sensitive(dad.autosave[0])
    checkbutton_autosave_on_quit.set_active(dad.autosave_on_quit)
    checkbutton_backup_before_saving.set_active(dad.backup_copy)
    
    frame_saving = gtk.Frame(label="<b>"+_("Saving")+"</b>")
    frame_saving.get_label_widget().set_use_markup(True)
    frame_saving.set_shadow_type(gtk.SHADOW_NONE)
    frame_saving.add(vbox_saving)
    
    vbox_misc_misc = gtk.VBox()
    checkbutton_newer_version = gtk.CheckButton(_("Automatically Check for Newer Version"))
    checkbutton_reload_doc_last = gtk.CheckButton(_("Reload Document From Last Session"))
    checkbutton_mod_time_sentinel = gtk.CheckButton(_("Reload After External Update to CT* File"))
    vbox_misc_misc.pack_start(checkbutton_newer_version, expand=False)
    vbox_misc_misc.pack_start(checkbutton_reload_doc_last, expand=False)
    vbox_misc_misc.pack_start(checkbutton_mod_time_sentinel, expand=False)
    
    checkbutton_newer_version.set_active(dad.check_version)
    checkbutton_reload_doc_last.set_active(dad.reload_doc_last)
    checkbutton_mod_time_sentinel.set_active(dad.enable_mod_time_sentinel)
    
    frame_misc_misc = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_misc.get_label_widget().set_use_markup(True)
    frame_misc_misc.set_shadow_type(gtk.SHADOW_NONE)
    frame_misc_misc.add(vbox_misc_misc)
    
    vbox_language = gtk.VBox()
    combobox_country_language = gtk.ComboBox(model=dad.country_lang_liststore)
    cell = gtk.CellRendererText()
    combobox_country_language.pack_start(cell, True)
    combobox_country_language.add_attribute(cell, 'text', 0)
    combobox_country_language.set_active_iter(dad.get_combobox_iter_from_value(dad.country_lang_liststore, 0, dad.country_lang))
    
    frame_language = gtk.Frame(label="<b>"+_("Language")+"</b>")
    frame_language.get_label_widget().set_use_markup(True)
    frame_language.set_shadow_type(gtk.SHADOW_NONE)
    frame_language.add(vbox_language)
    
    vbox_misc.pack_start(frame_system_tray)
    vbox_misc.pack_start(frame_saving)
    vbox_misc.pack_start(frame_misc_misc)
    vbox_misc.pack_start(frame_language)
    def on_checkbutton_systray_toggled(checkbutton):
        dad.systray = checkbutton.get_active()
        if dad.systray:
            dad.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property(cons.STR_VISIBLE, True)
            checkbutton_start_on_systray.set_sensitive(True)
        else:
            dad.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property(cons.STR_VISIBLE, False)
            checkbutton_start_on_systray.set_sensitive(False)
        if dad.systray:
            if not dad.use_appind:
                if "status_icon" in dir(dad.boss): dad.boss.status_icon.set_property(cons.STR_VISIBLE, True)
                else: dad.status_icon_enable()
            else:
                if "ind" in dir(dad.boss): dad.boss.ind.set_status(appindicator.STATUS_ACTIVE)
                else: dad.status_icon_enable()
        else:
            if not dad.use_appind: dad.boss.status_icon.set_property(cons.STR_VISIBLE, False)
            else: dad.boss.ind.set_status(appindicator.STATUS_PASSIVE)
        dad.boss.systray_active = dad.systray
        if len(dad.boss.running_windows) > 1:
            for runn_win in dad.boss.running_windows:
                if runn_win.window == dad.window: continue
                runn_win.systray = dad.boss.systray_active
    checkbutton_systray.connect('toggled', on_checkbutton_systray_toggled)
    def on_checkbutton_start_on_systray_toggled(checkbutton):
        dad.start_on_systray = checkbutton.get_active()
    checkbutton_start_on_systray.connect('toggled', on_checkbutton_start_on_systray_toggled)
    def on_checkbutton_use_appind_toggled(checkbutton):
        if checkbutton_systray.get_active():
            former_active = True
            checkbutton_systray.set_active(False)
        else: former_active = False
        if checkbutton.get_active(): dad.use_appind = True
        else: dad.use_appind = False
        if former_active: checkbutton_systray.set_active(True)
        if len(dad.boss.running_windows) > 1:
            for runn_win in dad.boss.running_windows:
                if runn_win.window == dad.window: continue
                runn_win.use_appind = dad.use_appind
    checkbutton_use_appind.connect('toggled', on_checkbutton_use_appind_toggled)
    def on_checkbutton_autosave_toggled(checkbutton):
        dad.autosave[0] = checkbutton.get_active()
        if not dad.autosave[0] and dad.autosave_timer_id != None: dad.autosave_timer_stop()
        spinbutton_autosave.set_sensitive(dad.autosave[0])
    checkbutton_autosave.connect('toggled', on_checkbutton_autosave_toggled)
    def on_checkbutton_backup_before_saving_toggled(checkbutton):
        dad.backup_copy = checkbutton.get_active()
    checkbutton_backup_before_saving.connect('toggled', on_checkbutton_backup_before_saving_toggled)
    def on_checkbutton_autosave_on_quit_toggled(checkbutton):
        dad.autosave_on_quit = checkbutton.get_active()
    checkbutton_autosave_on_quit.connect('toggled', on_checkbutton_autosave_on_quit_toggled)
    def on_checkbutton_reload_doc_last_toggled(checkbutton):
        dad.reload_doc_last = checkbutton.get_active()
    checkbutton_reload_doc_last.connect('toggled', on_checkbutton_reload_doc_last_toggled)
    def on_checkbutton_mod_time_sentinel_toggled(checkbutton):
        dad.enable_mod_time_sentinel = checkbutton.get_active()
        if dad.enable_mod_time_sentinel:
            if dad.mod_time_sentinel_id == None:
                dad.modification_time_sentinel_start()
        else:
            if dad.mod_time_sentinel_id != None:
                dad.modification_time_sentinel_stop()
    checkbutton_mod_time_sentinel.connect('toggled', on_checkbutton_mod_time_sentinel_toggled)
    def on_checkbutton_newer_version_toggled(checkbutton):
        dad.check_version = checkbutton.get_active()
    checkbutton_newer_version.connect('toggled', on_checkbutton_newer_version_toggled)
    def on_combobox_country_language_changed(combobox):
        new_iter = combobox_country_language.get_active_iter()
        new_lang = dad.country_lang_liststore[new_iter][0]
        if new_lang != dad.country_lang:
            dad.country_lang = new_lang
            support.dialog_info(_("The New Language will be Available Only After Restarting CherryTree"), dad.window)
            lang_file_descriptor = file(cons.LANG_PATH, 'w')
            lang_file_descriptor.write(new_lang)
            lang_file_descriptor.close()
    combobox_country_language.connect('changed', on_combobox_country_language_changed)
    
    notebook = gtk.Notebook()
    notebook.set_tab_pos(gtk.POS_LEFT)
    notebook.append_page(vbox_all_nodes, gtk.Label(_("All Nodes")))
    notebook.append_page(vbox_text_nodes, gtk.Label(_("Text Nodes")))
    notebook.append_page(vbox_code_nodes, gtk.Label(_("Code Nodes")))
    notebook.append_page(vbox_tree, gtk.Label(_("Tree")))
    notebook.append_page(vbox_fonts, gtk.Label(_("Fonts")))
    notebook.append_page(vbox_links, gtk.Label(_("Links")))
    notebook.append_page(vbox_misc, gtk.Label(_("Miscellaneous")))
    content_area = dialog.get_content_area()
    content_area.pack_start(notebook)
    content_area.show_all()
    notebook.set_current_page(dad.prefpage)
    dialog.run()
    # latest tab page
    dad.prefpage = notebook.get_current_page()
    # timer activate/modify handling
    new_autosave_value = int(spinbutton_autosave.get_value())
    if dad.autosave[1] != new_autosave_value:
        dad.autosave[1] = new_autosave_value
        if dad.autosave_timer_id != None: dad.autosave_timer_stop()
    if dad.autosave[0] and dad.autosave_timer_id == None: dad.autosave_timer_start()
    dialog.hide()
    
    # special characters
    new_special_chars = unicode(textbuffer_special_chars.get_text(*textbuffer_special_chars.get_bounds()).replace(cons.CHAR_NEWLINE, ""), cons.STR_UTF8, cons.STR_IGNORE)
    if dad.special_chars != new_special_chars:
        dad.special_chars = new_special_chars
        support.set_menu_items_special_chars(dad)

def dialog_img_n_entry(father_win, title, entry_content, img_stock):
    """Insert/Edit Anchor Name"""
    dialog = gtk.Dialog(title=title,
        parent=father_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                 gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_default_size(300, -1)
    image = gtk.Image()
    image.set_from_stock(img_stock, gtk.ICON_SIZE_BUTTON)
    entry = gtk.Entry()
    entry.set_text(entry_content)
    hbox = gtk.HBox()
    hbox.pack_start(image, expand=False)
    hbox.pack_start(entry)
    hbox.set_spacing(5)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox)
    def on_key_press_anchoreditdialog(widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == cons.STR_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        return False
    dialog.connect('key_press_event', on_key_press_anchoreditdialog)
    content_area.show_all()
    entry.grab_focus()
    response = dialog.run()
    dialog.hide()
    return unicode(entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE).strip() if response == gtk.RESPONSE_ACCEPT else ""

def dialog_image_handle(father_win, title, original_pixbuf):
    """Insert/Edit Image"""
    class ImgParms:
        def __init__(self, original_pixbuf):
            self.original_pixbuf = original_pixbuf
            self.width = original_pixbuf.get_width()
            self.height = original_pixbuf.get_height()
            self.image_w_h_ration = float(self.width)/self.height
    img_parms = ImgParms(original_pixbuf)
    dialog = gtk.Dialog(title=title,
        parent=father_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                 gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_default_size(600, 600)
    button_rotate_90_ccw = gtk.Button()
    button_rotate_90_ccw.set_image(gtk.image_new_from_stock("object-rotate-left", gtk.ICON_SIZE_DND))
    button_rotate_90_cw = gtk.Button()
    button_rotate_90_cw.set_image(gtk.image_new_from_stock("object-rotate-right", gtk.ICON_SIZE_DND))
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    viewport = gtk.Viewport()
    image = gtk.image_new_from_pixbuf(original_pixbuf)
    scrolledwindow.add(viewport)
    viewport.add(image)
    hbox_1 = gtk.HBox()
    hbox_1.pack_start(button_rotate_90_ccw, expand=False)
    hbox_1.pack_start(scrolledwindow)
    hbox_1.pack_start(button_rotate_90_cw, expand=False)
    hbox_1.set_spacing(2)
    label_width = gtk.Label(_("Width"))
    adj_width = gtk.Adjustment(value=img_parms.width, lower=1, upper=10000, step_incr=1)
    spinbutton_width = gtk.SpinButton(adj_width)
    label_height = gtk.Label(_("Height"))
    adj_height = gtk.Adjustment(value=img_parms.height, lower=1, upper=10000, step_incr=1)
    spinbutton_height = gtk.SpinButton(adj_height)
    hbox_2 = gtk.HBox()
    hbox_2.pack_start(label_width)
    hbox_2.pack_start(spinbutton_width)
    hbox_2.pack_start(label_height)
    hbox_2.pack_start(spinbutton_height)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox_1)
    content_area.pack_start(hbox_2, expand=False)
    content_area.set_spacing(6)
    def image_load_into_dialog():
        spinbutton_width.set_value(img_parms.width)
        spinbutton_height.set_value(img_parms.height)
        if img_parms.width <= 900 and img_parms.height <= 600:
            # original size into the dialog
            pixbuf = img_parms.original_pixbuf.scale_simple(int(img_parms.width), int(img_parms.height), gtk.gdk.INTERP_BILINEAR)
        else:
            # reduced size visible into the dialog
            if img_parms.width > 900:
                img_parms.width = 900
                img_parms.height = img_parms.width / img_parms.image_w_h_ration
            else:
                img_parms.height = 600
                img_parms.width = img_parms.height * img_parms.image_w_h_ration
            pixbuf = img_parms.original_pixbuf.scale_simple(int(img_parms.width), int(img_parms.height), gtk.gdk.INTERP_BILINEAR)
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
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == cons.STR_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        return False
    button_rotate_90_ccw.connect('clicked', on_button_rotate_90_ccw_clicked)
    button_rotate_90_cw.connect('clicked', on_button_rotate_90_cw_clicked)
    spinbutton_width.connect('value-changed', on_spinbutton_image_width_value_changed)
    spinbutton_height.connect('value-changed', on_spinbutton_image_height_value_changed)
    dialog.connect('key_press_event', on_key_press_imagehandledialog)
    image_load_into_dialog()
    content_area.show_all()
    try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).grab_focus()
    except: pass
    response = dialog.run()
    dialog.hide()
    if response != gtk.RESPONSE_ACCEPT: return None
    return img_parms.original_pixbuf.scale_simple(int(img_parms.width), int(img_parms.height), gtk.gdk.INTERP_BILINEAR)

def dialog_node_delete(father_win, warning_label):
    """Confirmation before Node Remove"""
    dialog = gtk.Dialog(title=_("Warning"),
        parent=father_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                 gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_default_size(350, 150)
    image = gtk.Image()
    image.set_from_stock(gtk.STOCK_DIALOG_WARNING, gtk.ICON_SIZE_DIALOG)
    label = gtk.Label(warning_label)
    label.set_use_markup(True)
    hbox = gtk.HBox()
    hbox.pack_start(image)
    hbox.pack_start(label)
    hbox.set_spacing(5)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox)
    def on_key_press_nodedeletedialog(widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == cons.STR_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        return False
    dialog.connect('key_press_event', on_key_press_nodedeletedialog)
    content_area.show_all()
    try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).grab_focus()
    except: pass
    response = dialog.run()
    dialog.hide()
    return response

def dialog_exit_save(father_win):
    """Save before Exit Dialog"""
    dialog = gtk.Dialog(title=_("Warning"),
        parent=father_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(_("Cancel"), 6,
                 _("No"), 4,
                 _("Yes"), 2) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_default_size(350, 150)
    try:
        button = dialog.get_widget_for_response(6)
        button.set_image(gtk.image_new_from_stock(gtk.STOCK_CANCEL, gtk.ICON_SIZE_BUTTON))
        button = dialog.get_widget_for_response(4)
        button.set_image(gtk.image_new_from_stock(gtk.STOCK_CLEAR, gtk.ICON_SIZE_BUTTON))
        button = dialog.get_widget_for_response(2)
        button.set_image(gtk.image_new_from_stock(gtk.STOCK_SAVE, gtk.ICON_SIZE_BUTTON))
        button.grab_focus()
    except: pass
    image = gtk.Image()
    image.set_from_stock(gtk.STOCK_DIALOG_WARNING, gtk.ICON_SIZE_DIALOG)
    label = gtk.Label(_("""<b>The current document was updated</b>,
do you want to save the changes?"""))
    label.set_use_markup(True)
    hbox = gtk.HBox()
    hbox.pack_start(image)
    hbox.pack_start(label)
    hbox.set_spacing(5)
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox)
    def on_key_press_exitdialog(widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == cons.STR_RETURN:
            try: dialog.get_widget_for_response(2).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        elif keyname == "Escape":
            try: dialog.get_widget_for_response(6).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
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
            self.sel_iter = sel_tree_iter
    links_parms = LinksParms()
    dialog = gtk.Dialog(title=title,
        parent=dad.window,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
        gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_default_size(500, 400)
    
    hbox_webs = gtk.HBox()
    image_webs = gtk.Image()
    image_webs.set_from_stock("link_website", gtk.ICON_SIZE_BUTTON)
    radiobutton_webs = gtk.RadioButton(label=_("To WebSite"))
    entry_webs = gtk.Entry()
    entry_webs.set_text(dad.links_entries['webs'])
    hbox_webs.pack_start(image_webs, expand=False)
    hbox_webs.pack_start(radiobutton_webs, expand=False)
    hbox_webs.pack_start(entry_webs)
    hbox_webs.set_spacing(5)
    
    hbox_file = gtk.HBox()
    image_file = gtk.Image()
    image_file.set_from_stock(gtk.STOCK_FILE, gtk.ICON_SIZE_BUTTON)
    radiobutton_file = gtk.RadioButton(label=_("To File"))
    radiobutton_file.set_group(radiobutton_webs)
    entry_file = gtk.Entry()
    entry_file.set_text(dad.links_entries['file'])
    button_browse_file = gtk.Button()
    button_browse_file.set_image(gtk.image_new_from_stock("find", gtk.ICON_SIZE_BUTTON))
    hbox_file.pack_start(image_file, expand=False)
    hbox_file.pack_start(radiobutton_file, expand=False)
    hbox_file.pack_start(entry_file)
    hbox_file.pack_start(button_browse_file, expand=False)
    hbox_file.set_spacing(5)
    
    hbox_folder = gtk.HBox()
    image_folder = gtk.Image()
    image_folder.set_from_stock(gtk.STOCK_DIRECTORY, gtk.ICON_SIZE_BUTTON)
    radiobutton_folder = gtk.RadioButton(label=_("To Folder"))
    radiobutton_folder.set_group(radiobutton_webs)
    entry_folder = gtk.Entry()
    entry_folder.set_text(dad.links_entries['fold'])
    button_browse_folder = gtk.Button()
    button_browse_folder.set_image(gtk.image_new_from_stock("find", gtk.ICON_SIZE_BUTTON))
    hbox_folder.pack_start(image_folder, expand=False)
    hbox_folder.pack_start(radiobutton_folder, expand=False)
    hbox_folder.pack_start(entry_folder)
    hbox_folder.pack_start(button_browse_folder, expand=False)
    hbox_folder.set_spacing(5)
    
    hbox_node = gtk.HBox()
    image_node = gtk.Image()
    image_node.set_from_stock("cherrytree", gtk.ICON_SIZE_BUTTON)
    radiobutton_node = gtk.RadioButton(label=_("To Node"))
    radiobutton_node.set_group(radiobutton_webs)
    hbox_node.pack_start(image_node, expand=False)
    hbox_node.pack_start(radiobutton_node)
    hbox_node.set_spacing(5)
    
    hbox_detail = gtk.HBox()
    
    treeview_2 = gtk.TreeView(dad.treestore)
    treeview_2.set_headers_visible(False)
    renderer_pixbuf_2 = gtk.CellRendererPixbuf()
    renderer_text_2 = gtk.CellRendererText()
    column_2 = gtk.TreeViewColumn()
    column_2.pack_start(renderer_pixbuf_2, False)
    column_2.pack_start(renderer_text_2, True)
    column_2.set_attributes(renderer_pixbuf_2, stock_id=0)
    column_2.set_attributes(renderer_text_2, text=1)
    treeview_2.append_column(column_2)
    treeviewselection_2 = treeview_2.get_selection()
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    scrolledwindow.add(treeview_2)
    if links_parms.sel_iter:
        sel_path = dad.treestore.get_path(links_parms.sel_iter)
        treeview_2.expand_to_path(sel_path)
        treeview_2.set_cursor(sel_path)
        treeview_2.scroll_to_cell(sel_path)
    
    vbox_anchor = gtk.VBox()
    label_over = gtk.Label()
    label_below = gtk.Label()
    
    hbox_anchor = gtk.HBox()
    entry_anchor = gtk.Entry()
    entry_anchor.set_text(dad.links_entries['anch'])
    button_browse_anchor = gtk.Button()
    button_browse_anchor.set_image(gtk.image_new_from_stock("anchor", gtk.ICON_SIZE_BUTTON))
    hbox_anchor.pack_start(entry_anchor)
    hbox_anchor.pack_start(button_browse_anchor, expand=False)
    
    frame_anchor = gtk.Frame(label="<b>"+_("Anchor Name (optional)")+"</b>")
    frame_anchor.get_label_widget().set_use_markup(True)
    frame_anchor.set_shadow_type(gtk.SHADOW_NONE)
    frame_anchor.add(hbox_anchor)
    
    vbox_anchor.pack_start(label_over)
    vbox_anchor.pack_start(frame_anchor, expand=False)
    vbox_anchor.pack_start(label_below)
    
    hbox_detail.pack_start(scrolledwindow)
    hbox_detail.pack_start(vbox_anchor, expand=False)
    
    content_area = dialog.get_content_area()
    content_area.pack_start(hbox_webs, expand=False)
    content_area.pack_start(hbox_file, expand=False)
    content_area.pack_start(hbox_folder, expand=False)
    content_area.pack_start(hbox_node, expand=False)
    content_area.pack_start(hbox_detail)
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
        filepath = dialog_file_select(curr_folder=dad.pick_dir, parent=dialog)
        if not filepath: return
        dad.pick_dir = os.path.dirname(filepath)
        entry_file.set_text(filepath)
    def on_button_browse_for_folder_to_link_to_clicked(self, *args):
        filepath = dialog_folder_select(curr_folder=dad.pick_dir, parent=dialog)
        if not filepath: return
        dad.pick_dir = filepath
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
        iter_insert = dad.curr_buffer.get_iter_at_mark(dad.curr_buffer.get_insert())
        iter_bound = dad.curr_buffer.get_iter_at_mark(dad.curr_buffer.get_selection_bound())
        insert_offset = iter_insert.get_offset()
        bound_offset = iter_bound.get_offset()
        dad.objects_buffer_refresh()
        dad.curr_buffer.move_mark(dad.curr_buffer.get_insert(), dad.curr_buffer.get_iter_at_offset(insert_offset))
        dad.curr_buffer.move_mark(dad.curr_buffer.get_selection_bound(), dad.curr_buffer.get_iter_at_offset(bound_offset))
        dad.sourceview.scroll_to_mark(dad.curr_buffer.get_insert(), 0.3)
        if not anchors_list:
            dialog_info(_("There are No Anchors in the Selected Node"), dialog)
            return
        ret_anchor_name = dialog_anchors_list(dialog, _("Choose Existing Anchor"), anchors_list)
        if ret_anchor_name: entry_anchor.set_text(ret_anchor_name)
    def on_treeview_event_after(treeview, event):
        if event.type not in [gtk.gdk.BUTTON_PRESS, gtk.gdk.KEY_PRESS]: return
        model, links_parms.sel_iter = treeviewselection_2.get_selected()
    def on_key_press_links_handle_dialog(widget, event):
        if gtk.gdk.keyval_name(event.keyval) == cons.STR_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
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
    
    link_type_changed_on_dialog()
    content_area.show_all()
    response = dialog.run()
    dialog.hide()
    if response != gtk.RESPONSE_ACCEPT: return False
    dad.links_entries['webs'] = unicode(entry_webs.get_text(), cons.STR_UTF8, cons.STR_IGNORE).strip()
    dad.links_entries['file'] = unicode(entry_file.get_text(), cons.STR_UTF8, cons.STR_IGNORE).strip()
    dad.links_entries['fold'] = unicode(entry_folder.get_text(), cons.STR_UTF8, cons.STR_IGNORE).strip()
    dad.links_entries['anch'] = unicode(entry_anchor.get_text(), cons.STR_UTF8, cons.STR_IGNORE).strip()
    dad.links_entries['node'] = links_parms.sel_iter
    return True

def dialog_choose_node(father_win, title, treestore, sel_tree_iter):
    """Dialog to Select a Node"""
    class NodeParms:
        def __init__(self):
            self.sel_iter = sel_tree_iter
    node_parms = NodeParms()
    dialog = gtk.Dialog(title=title,
        parent=father_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
        gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    dialog.set_default_size(400, 300)
    treeview_2 = gtk.TreeView(treestore)
    treeview_2.set_headers_visible(False)
    renderer_pixbuf_2 = gtk.CellRendererPixbuf()
    renderer_text_2 = gtk.CellRendererText()
    column_2 = gtk.TreeViewColumn()
    column_2.pack_start(renderer_pixbuf_2, False)
    column_2.pack_start(renderer_text_2, True)
    column_2.set_attributes(renderer_pixbuf_2, stock_id=0)
    column_2.set_attributes(renderer_text_2, text=1)
    treeview_2.append_column(column_2)
    treeviewselection_2 = treeview_2.get_selection()
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    scrolledwindow.add(treeview_2)
    if node_parms.sel_iter:
        sel_path = treestore.get_path(node_parms.sel_iter)
        treeview_2.expand_to_path(sel_path)
        treeview_2.set_cursor(sel_path)
        treeview_2.scroll_to_cell(sel_path)
    content_area = dialog.get_content_area()
    content_area.pack_start(scrolledwindow)
    def on_key_press_choose_node_dialog(widget, event):
        if gtk.gdk.keyval_name(event.keyval) == cons.STR_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        return False
    def on_mouse_button_clicked_treeview_2(widget, event):
        if event.button != 1: return
        if event.type == gtk.gdk._2BUTTON_PRESS:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
    def on_treeview_event_after(treeview, event):
        if event.type not in [gtk.gdk.BUTTON_PRESS, gtk.gdk.KEY_PRESS]: return
        model, node_parms.sel_iter = treeviewselection_2.get_selected()
    dialog.connect("key_press_event", on_key_press_choose_node_dialog)
    treeview_2.connect('button-press-event', on_mouse_button_clicked_treeview_2)
    treeview_2.connect('event-after', on_treeview_event_after)
    content_area.show_all()
    response = dialog.run()
    dialog.hide()
    return None if response != gtk.RESPONSE_ACCEPT else node_parms.sel_iter

def dialog_selnode_selnodeandsub_alltree(father_win, also_selection, also_node_name=False):
    """Dialog to select between the Selected Node/Selected Node + Subnodes/All Tree"""
    dialog = gtk.Dialog(title=_("Involved Nodes"),
        parent=father_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
        gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    if also_selection: radiobutton_selection = gtk.RadioButton(label=_("Selected Text Only"))
    radiobutton_selnode = gtk.RadioButton(label=_("Selected Node Only"))
    radiobutton_selnodeandsub = gtk.RadioButton(label=_("Selected Node and Subnodes"))
    radiobutton_alltree = gtk.RadioButton(label=_("All the Tree"))
    radiobutton_selnodeandsub.set_group(radiobutton_selnode)
    radiobutton_alltree.set_group(radiobutton_selnode)
    if also_node_name:
        separator_item = gtk.HSeparator()
        checkbutton_node_name = gtk.CheckButton(label=_("Include Node Name"))
        checkbutton_node_name.set_active(True)
    if also_selection: radiobutton_selection.set_group(radiobutton_selnode)
    content_area = dialog.get_content_area()
    if also_selection: content_area.pack_start(radiobutton_selection)
    content_area.pack_start(radiobutton_selnode)
    content_area.pack_start(radiobutton_selnodeandsub)
    content_area.pack_start(radiobutton_alltree)
    if also_node_name:
        content_area.pack_start(separator_item)
        content_area.pack_start(checkbutton_node_name)
    def on_key_press_enter_dialog(widget, event):
        if gtk.gdk.keyval_name(event.keyval) == cons.STR_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        return False
    dialog.connect("key_press_event", on_key_press_enter_dialog)
    content_area.show_all()
    response = dialog.run()
    if radiobutton_selnode.get_active(): ret_val = 1
    elif radiobutton_selnodeandsub.get_active(): ret_val = 2
    elif radiobutton_alltree.get_active(): ret_val = 3
    else: ret_val = 4
    if also_node_name and checkbutton_node_name.get_active(): ret_node_name = True
    else: ret_node_name = False
    dialog.destroy()
    if response != gtk.RESPONSE_ACCEPT: ret_val = 0
    return [ret_val, ret_node_name]

def set_bookmarks_menu_items(inst):
    """Set Bookmarks Menu Items"""
    bookmarks_menu = inst.ui.get_widget("/MenuBar/BookmarksMenu").get_submenu()
    for menu_item in inst.bookmarks_menu_items:
        bookmarks_menu.remove(menu_item)
    menu_item = gtk.SeparatorMenuItem()
    menu_item.show()
    bookmarks_menu.append(menu_item)
    inst.bookmarks_menu_items = [menu_item]
    for node_id_str in inst.bookmarks:
        if not long(node_id_str) in inst.nodes_names_dict: continue
        menu_item = gtk.ImageMenuItem(inst.nodes_names_dict[long(node_id_str)])
        menu_item.set_image(gtk.image_new_from_stock("Red Cherry", gtk.ICON_SIZE_MENU))
        menu_item.connect("activate", select_bookmark_node, node_id_str, inst)
        menu_item.show()
        bookmarks_menu.append(menu_item)
        inst.bookmarks_menu_items.append(menu_item)

def set_menu_items_special_chars(inst):
    """Set Special Chars menu items"""
    if not "special_menu_1" in dir(inst):
        inst.special_menu_1 = gtk.Menu()
        first_run = True
    else:
        children_1 = inst.special_menu_1.get_children()
        for children in children_1:
            children.hide()
            del children
        first_run = False
    for special_char in inst.special_chars:
        menu_item = gtk.MenuItem(special_char)
        menu_item.connect("activate", insert_special_char, special_char, inst)
        menu_item.show()
        inst.special_menu_1.append(menu_item)
    if first_run:
        # main menu
        special_menuitem = gtk.ImageMenuItem(_("Insert _Special Character"))
        special_menuitem.set_image(gtk.image_new_from_stock("insert", gtk.ICON_SIZE_MENU))
        special_menuitem.set_tooltip_text(_("Insert a Special Character"))
        special_menuitem.set_submenu(inst.special_menu_1)
        inst.ui.get_widget("/MenuBar/EditMenu").get_submenu().insert(special_menuitem, 14)

def set_menu_items_recent_documents(inst):
    """Set Recent Documents menu items on Menu and Toolbar"""
    if not "recent_menu_1" in dir(inst):
        inst.recent_menu_1 = gtk.Menu()
        inst.recent_menu_2 = gtk.Menu()
        first_run = True
    else:
        children_1 = inst.recent_menu_1.get_children()
        children_2 = inst.recent_menu_2.get_children()
        for children in children_1:
            children.hide()
            del children
        for children in children_2:
            children.hide()
            del children
        first_run = False
    for target in [1, 2]:
        for i, filepath in enumerate(inst.recent_docs):
            if i >= cons.MAX_RECENT_DOCS: break
            menu_item = gtk.ImageMenuItem(filepath)
            menu_item.set_image(gtk.image_new_from_stock("gtk-open", gtk.ICON_SIZE_MENU))
            menu_item.connect("activate", open_recent_document, filepath, inst)
            menu_item.show()
            if target == 1: inst.recent_menu_1.append(menu_item)
            else: inst.recent_menu_2.append(menu_item)
    if first_run:
        # main menu
        recent_menuitem = gtk.ImageMenuItem(_("_Recent Documents"))
        recent_menuitem.set_image(gtk.image_new_from_stock("gtk-open", gtk.ICON_SIZE_MENU))
        recent_menuitem.set_tooltip_text(_("Open a Recent CherryTree Document"))
        recent_menuitem.set_submenu(inst.recent_menu_1)
        inst.ui.get_widget("/MenuBar/FileMenu").get_submenu().insert(recent_menuitem, 9)
        # toolbar
        menu_toolbutton = gtk.MenuToolButton("gtk-open")
        menu_toolbutton.set_tooltip_text(_("Open a CherryTree Document"))
        menu_toolbutton.set_arrow_tooltip_text(_("Open a Recent CherryTree Document"))
        menu_toolbutton.set_menu(inst.recent_menu_2)
        menu_toolbutton.connect("clicked", inst.file_open)
        inst.ui.get_widget("/ToolBar").insert(menu_toolbutton, 8)

def add_recent_document(inst, filepath):
    """Add a Recent Document if Needed"""
    if filepath in inst.recent_docs and inst.recent_docs[0] == filepath:
        return
    if filepath in inst.recent_docs: inst.recent_docs.remove(filepath)
    inst.recent_docs.insert(0, filepath)
    set_menu_items_recent_documents(inst)

def insert_special_char(menu_item, special_char, dad):
    """A Special character insert was Requested"""
    if not dad.is_there_selected_node_or_error(): return
    dad.curr_buffer.insert_at_cursor(special_char)

def open_recent_document(menu_item, filepath, dad):
    """A Recent Document was Requested"""
    if os.path.isfile(filepath):
        dad.filepath_boss_open(filepath, "")
    else:
        dialog_error(_("The Document %s was Not Found") % filepath, dad.window)
        menu_item.hide()
        try: dad.recent_docs.remove(filepath)
        except: pass

def select_bookmark_node(menu_item, node_id_str, dad):
    """Select a Node in the Bookmarks List"""
    node_iter = dad.get_tree_iter_from_node_id(long(node_id_str))
    if node_iter: dad.treeview_safe_set_cursor(node_iter)

def bookmarks_handle(dad):
    """Handle the Bookmarks List"""
    dialog = gtk.Dialog(title=_("Handle the Bookmarks List"),
        parent=dad.window,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
        gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
    dialog.set_default_size(500, 400)
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    liststore = gtk.ListStore(str, str, str)
    for node_id_str in dad.bookmarks:
        # icon, node name, node id string
        liststore.append(["Red Cherry", dad.nodes_names_dict[long(node_id_str)], node_id_str])
    treeview = gtk.TreeView(liststore)
    treeview.set_headers_visible(False)
    treeview.set_reorderable(True)
    treeview.set_tooltip_text(_("Sort with Drag and Drop, Delete with the Delete Key"))
    treeviewselection = treeview.get_selection()
    def on_key_press_liststore(widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == "Delete":
            model, tree_iter = treeviewselection.get_selected()
            if tree_iter: model.remove(tree_iter)
    def on_mouse_button_clicked_liststore(widget, event):
        """Catches mouse buttons clicks"""
        if event.button != 1: return
        if event.type != gtk.gdk._2BUTTON_PRESS: return
        path_n_tvc = treeview.get_path_at_pos(int(event.x), int(event.y))
        if not path_n_tvc: return
        tree_path = path_n_tvc[0]
        dad_tree_path = dad.get_tree_iter_from_node_id(long(liststore[tree_path][2]))
        dad.treeview_safe_set_cursor(dad_tree_path)
    treeview.connect('key_press_event', on_key_press_liststore)
    treeview.connect('button-press-event', on_mouse_button_clicked_liststore)
    renderer_pixbuf = gtk.CellRendererPixbuf()
    renderer_text = gtk.CellRendererText()
    column = gtk.TreeViewColumn()
    column.pack_start(renderer_pixbuf, False)
    column.pack_start(renderer_text, True)
    column.set_attributes(renderer_pixbuf, stock_id=0)
    column.set_attributes(renderer_text, text=1)
    treeview.append_column(column)
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    scrolledwindow.add(treeview)
    content_area = dialog.get_content_area()
    content_area.pack_start(scrolledwindow)
    content_area.show_all()
    response = dialog.run()
    temp_bookmarks = []
    tree_iter = liststore.get_iter_first()
    while tree_iter != None:
        temp_bookmarks.append(liststore[tree_iter][2])
        tree_iter = liststore.iter_next(tree_iter)
    dialog.destroy()
    if response != gtk.RESPONSE_ACCEPT: return False
    dad.bookmarks = temp_bookmarks
    set_bookmarks_menu_items(dad)
    dad.ctdb_handler.pending_edit_db_bookmarks()
    return True

def set_object_highlight(inst, obj_highl):
    """Set the Highlight to obj_highl only"""
    if inst.highlighted_obj:
        inst.highlighted_obj.drag_unhighlight()
        inst.highlighted_obj = None
    if obj_highl:
        obj_highl.drag_highlight()
        inst.highlighted_obj = obj_highl
