# -*- coding: UTF-8 -*-
#
#       tablez.py
#
#       Copyright 2009-2014 Giuseppe Penone <giuspen@gmail.com>
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

import gtk, pango
import os, csv, codecs, cStringIO, copy
import cons, support


class TablesHandler:
    """Handler of the Tables"""

    def __init__(self, dad):
        """Lists Handler boot"""
        self.dad = dad

    def table_cut(self, *args):
        """Cut Table"""
        self.dad.object_set_selection(self.curr_table_anchor)
        self.dad.sourceview.emit("cut-clipboard")

    def table_copy(self, *args):
        """Copy Table"""
        self.dad.object_set_selection(self.curr_table_anchor)
        self.dad.sourceview.emit("copy-clipboard")

    def table_delete(self, *args):
        """Delete Table"""
        self.dad.object_set_selection(self.curr_table_anchor)
        self.dad.curr_buffer.delete_selection(True, self.dad.sourceview.get_editable())
        self.dad.sourceview.grab_focus()

    def dialog_tablecolhandle(self, title, rename_text):
        """Opens the Table Column Handle Dialog"""
        dialog = gtk.Dialog(title=title,
                            parent=self.dad.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
        dialog.set_default_size(300, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        
        hbox_column_rename = gtk.HBox()
        image_column_rename = gtk.Image()
        image_column_rename.set_from_stock(gtk.STOCK_EDIT, gtk.ICON_SIZE_BUTTON)
        table_column_rename_radiobutton = gtk.RadioButton(label=_("Rename Column"))
        table_column_rename_entry = gtk.Entry()
        table_column_rename_entry.set_text(rename_text)
        table_column_rename_entry.set_sensitive(self.dad.table_column_mode == 'rename')
        hbox_column_rename.pack_start(image_column_rename, expand=False)
        hbox_column_rename.pack_start(table_column_rename_radiobutton)
        hbox_column_rename.pack_start(table_column_rename_entry)
        
        hbox_column_delete = gtk.HBox()
        image_column_delete = gtk.Image()
        image_column_delete.set_from_stock(gtk.STOCK_CLEAR, gtk.ICON_SIZE_BUTTON)
        table_column_delete_radiobutton = gtk.RadioButton(label=_("Delete Column"))
        table_column_delete_radiobutton.set_group(table_column_rename_radiobutton)
        hbox_column_delete.pack_start(image_column_delete, expand=False)
        hbox_column_delete.pack_start(table_column_delete_radiobutton)
        
        hbox_column_add = gtk.HBox()
        image_column_add = gtk.Image()
        image_column_add.set_from_stock(gtk.STOCK_ADD, gtk.ICON_SIZE_BUTTON)
        table_column_add_radiobutton = gtk.RadioButton(label=_("Add Column"))
        table_column_add_radiobutton.set_group(table_column_rename_radiobutton)
        table_column_new_entry = gtk.Entry()
        hbox_column_add.pack_start(image_column_add, expand=False)
        hbox_column_add.pack_start(table_column_add_radiobutton)
        hbox_column_add.pack_start(table_column_new_entry)
        table_column_new_entry.set_sensitive(self.dad.table_column_mode == 'add')
        
        hbox_column_left = gtk.HBox()
        image_column_left = gtk.Image()
        image_column_left.set_from_stock(gtk.STOCK_GO_BACK, gtk.ICON_SIZE_BUTTON)
        table_column_left_radiobutton = gtk.RadioButton(label=_("Move Column Left"))
        table_column_left_radiobutton.set_group(table_column_rename_radiobutton)
        hbox_column_left.pack_start(image_column_left, expand=False)
        hbox_column_left.pack_start(table_column_left_radiobutton)
        
        hbox_column_right = gtk.HBox()
        image_column_right = gtk.Image()
        image_column_right.set_from_stock(gtk.STOCK_GO_FORWARD, gtk.ICON_SIZE_BUTTON)
        table_column_right_radiobutton = gtk.RadioButton(label=_("Move Column Right"))
        table_column_right_radiobutton.set_group(table_column_rename_radiobutton)
        hbox_column_right.pack_start(image_column_right, expand=False)
        hbox_column_right.pack_start(table_column_right_radiobutton)
        
        table_column_rename_radiobutton.set_active(self.dad.table_column_mode == "rename")
        table_column_delete_radiobutton.set_active(self.dad.table_column_mode == "delete")
        table_column_add_radiobutton.set_active(self.dad.table_column_mode == "add")
        table_column_left_radiobutton.set_active(self.dad.table_column_mode == cons.TAG_PROP_LEFT)
        table_column_right_radiobutton.set_active(self.dad.table_column_mode == cons.TAG_PROP_RIGHT)
        
        tablehandle_vbox_col = gtk.VBox()
        tablehandle_vbox_col.pack_start(hbox_column_rename)
        tablehandle_vbox_col.pack_start(hbox_column_delete)
        tablehandle_vbox_col.pack_start(hbox_column_add)
        tablehandle_vbox_col.pack_start(hbox_column_left)
        tablehandle_vbox_col.pack_start(hbox_column_right)
        
        content_area = dialog.get_content_area()
        content_area.set_spacing(5)
        content_area.pack_start(tablehandle_vbox_col)
        content_area.show_all()
        def on_key_press_tablecolhandle(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if keyname == cons.STR_RETURN:
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
            return False
        def on_table_column_rename_radiobutton_toggled(radiobutton):
            if radiobutton.get_active():
                table_column_rename_entry.set_sensitive(True)
                self.dad.table_column_mode = "rename"
            else: table_column_rename_entry.set_sensitive(False)
        def on_table_column_delete_radiobutton_toggled(radiobutton):
            if radiobutton.get_active(): self.dad.table_column_mode = "delete"
        def on_table_column_add_radiobutton_toggled(radiobutton):
            if radiobutton.get_active():
                table_column_new_entry.set_sensitive(True)
                self.dad.table_column_mode = "add"
            else: table_column_new_entry.set_sensitive(False)
        def on_table_column_left_radiobutton_toggled(radiobutton):
            if radiobutton.get_active(): self.dad.table_column_mode = cons.TAG_PROP_LEFT
        def on_table_column_right_radiobutton_toggled(radiobutton):
            if radiobutton.get_active(): self.dad.table_column_mode = cons.TAG_PROP_RIGHT
        dialog.connect('key_press_event', on_key_press_tablecolhandle)
        table_column_rename_radiobutton.connect('toggled', on_table_column_rename_radiobutton_toggled)
        table_column_delete_radiobutton.connect('toggled', on_table_column_delete_radiobutton_toggled)
        table_column_add_radiobutton.connect('toggled', on_table_column_add_radiobutton_toggled)
        table_column_left_radiobutton.connect('toggled', on_table_column_left_radiobutton_toggled)
        table_column_right_radiobutton.connect('toggled', on_table_column_right_radiobutton_toggled)
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            ret_rename = unicode(table_column_rename_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            ret_add = unicode(table_column_new_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            return [True, ret_rename, ret_add]
        return [False, None, None]

    def dialog_tablehandle(self, title, is_insert):
        """Opens the Table Handle Dialog"""
        dialog = gtk.Dialog(title=title,
                            parent=self.dad.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
        dialog.set_default_size(300, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        
        label_rows = gtk.Label(_("Rows"))
        adj_rows = gtk.Adjustment(value=self.dad.table_rows, lower=1, upper=10000, step_incr=1)
        spinbutton_rows = gtk.SpinButton(adj_rows)
        spinbutton_rows.set_value(self.dad.table_rows)
        label_columns = gtk.Label(_("Columns"))
        adj_columns = gtk.Adjustment(value=self.dad.table_columns, lower=1, upper=10000, step_incr=1)
        spinbutton_columns = gtk.SpinButton(adj_columns)
        spinbutton_columns.set_value(self.dad.table_columns)
        
        hbox_rows_cols = gtk.HBox()
        hbox_rows_cols.pack_start(label_rows, expand=False)
        hbox_rows_cols.pack_start(spinbutton_rows, expand=False)
        hbox_rows_cols.pack_start(label_columns, expand=False)
        hbox_rows_cols.pack_start(spinbutton_columns, expand=False)
        hbox_rows_cols.set_spacing(5)
        size_align = gtk.Alignment()
        size_align.set_padding(6, 6, 6, 6)
        size_align.add(hbox_rows_cols)
        
        size_frame = gtk.Frame(label="<b>"+_("Table Size")+"</b>")
        size_frame.get_label_widget().set_use_markup(True)
        size_frame.set_shadow_type(gtk.SHADOW_NONE)
        size_frame.add(size_align)
        
        label_col_min = gtk.Label(_("Min Width"))
        adj_col_min = gtk.Adjustment(value=self.dad.table_col_min, lower=1, upper=10000, step_incr=1)
        spinbutton_col_min = gtk.SpinButton(adj_col_min)
        spinbutton_col_min.set_value(self.dad.table_col_min)
        label_col_max = gtk.Label(_("Max Width"))
        adj_col_max = gtk.Adjustment(value=self.dad.table_col_max, lower=1, upper=10000, step_incr=1)
        spinbutton_col_max = gtk.SpinButton(adj_col_max)
        spinbutton_col_max.set_value(self.dad.table_col_max)
        
        hbox_col_min_max = gtk.HBox()
        hbox_col_min_max.pack_start(label_col_min, expand=False)
        hbox_col_min_max.pack_start(spinbutton_col_min, expand=False)
        hbox_col_min_max.pack_start(label_col_max, expand=False)
        hbox_col_min_max.pack_start(spinbutton_col_max, expand=False)
        hbox_col_min_max.set_spacing(5)
        col_min_max_align = gtk.Alignment()
        col_min_max_align.set_padding(6, 6, 6, 6)
        col_min_max_align.add(hbox_col_min_max)
        
        col_min_max_frame = gtk.Frame(label="<b>"+_("Column Properties")+"</b>")
        col_min_max_frame.get_label_widget().set_use_markup(True)
        col_min_max_frame.set_shadow_type(gtk.SHADOW_NONE)
        col_min_max_frame.add(col_min_max_align)
        
        checkbutton_table_ins_from_file = gtk.CheckButton(label=_("Import from CSV File"))
        
        content_area = dialog.get_content_area()
        content_area.set_spacing(5)
        if is_insert: content_area.pack_start(size_frame)
        content_area.pack_start(col_min_max_frame)
        if is_insert: content_area.pack_start(checkbutton_table_ins_from_file)
        content_area.show_all()
        def on_key_press_tablehandle(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if keyname == cons.STR_RETURN:
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
            return False
        def on_checkbutton_table_ins_from_file_toggled(checkbutton):
            size_frame.set_sensitive(not checkbutton.get_active())
            col_min_max_frame.set_sensitive(not checkbutton.get_active())
        dialog.connect('key_press_event', on_key_press_tablehandle)
        checkbutton_table_ins_from_file.connect('toggled', on_checkbutton_table_ins_from_file_toggled)
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            self.dad.table_rows = int(spinbutton_rows.get_value())
            self.dad.table_columns = int(spinbutton_columns.get_value())
            self.dad.table_col_min = int(spinbutton_col_min.get_value())
            self.dad.table_col_max = int(spinbutton_col_max.get_value())
            ret_csv = checkbutton_table_ins_from_file.get_active()
            return [True, ret_csv]
        return [False, None]

    def table_export(self, action):
        """Table Export as CSV File"""
        filename = support.dialog_file_save_as(curr_folder=self.dad.pick_dir,
                                               filter_pattern="*.csv",
                                               filter_name=_("CSV File"),
                                               parent=self.dad.window)
        if filename == None: return
        if len(filename) < 4 or filename[-4:] != ".csv": filename += ".csv"
        self.dad.pick_dir = os.path.dirname(filename)
        table_dict = self.dad.state_machine.table_to_dict(self.curr_table_anchor)
        table_matrix = table_dict['matrix']
        table_matrix.insert(0, table_matrix.pop())
        file_descriptor = open(filename, "w")
        writer = UnicodeWriter(file_descriptor)
        writer.writerows(table_matrix)
        file_descriptor.close()

    def table_handle(self):
        """Insert Table"""
        iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
        ret_ok, ret_csv = self.dialog_tablehandle(_("Insert Table"), True)
        if not ret_ok: return
        if not ret_csv:
            self.table_insert(iter_insert)
        else:
            filepath = support.dialog_file_select(filter_pattern=["*.csv"],
                                                  filter_name=_("CSV File"),
                                                  curr_folder=self.dad.pick_dir,
                                                  parent=self.dad.window)
            if filepath != None:
                self.dad.pick_dir = os.path.dirname(filepath)
                support.text_file_rm_emptylines(filepath)
                file_descriptor = open(filepath, 'r')
                reader = UnicodeReader(file_descriptor)
                table_matrix = []
                row = reader.next()
                while row:
                    table_matrix.append(row)
                    row = reader.next()
                file_descriptor.close()
                table_matrix.append(table_matrix.pop(0))
                self.table_insert(iter_insert, {'col_min': cons.TABLE_DEFAULT_COL_MIN,
                                                'col_max': cons.TABLE_DEFAULT_COL_MAX,
                                                'matrix': table_matrix})

    def table_insert(self, iter_insert, table=None, table_justification=None, text_buffer=None):
        """Insert a Table at the Given Iter"""
        if not text_buffer: text_buffer = self.dad.curr_buffer
        if table != None:
            self.dad.table_columns = len(table['matrix'][0])
            self.dad.table_rows = len(table['matrix']) - 1
            headers = table['matrix'][-1]
            table_col_min = table['col_min']
            table_col_max = table['col_max']
        else:
            headers = [_("click me")]*self.dad.table_columns
            table_col_min = self.dad.table_col_min
            table_col_max = self.dad.table_col_max
        anchor = text_buffer.create_child_anchor(iter_insert)
        anchor.liststore = gtk.ListStore(*(str,)*self.dad.table_columns)
        anchor.treeview = gtk.TreeView(anchor.liststore)
        for element in range(self.dad.table_columns):
            label = gtk.Label('<b>' + headers[element] + '</b>')
            label.set_use_markup(True)
            label.set_tooltip_text(_("Click to Edit the Column Settings"))
            label.show()
            renderer_text = gtk.CellRendererText()
            renderer_text.set_property('editable', True)
            renderer_text.set_property('wrap-width', table_col_max)
            renderer_text.set_property('wrap-mode', pango.WRAP_WORD_CHAR)
            renderer_text.set_property('font-desc', pango.FontDescription(self.dad.text_font))
            renderer_text.connect('edited', self.on_table_cell_edited, anchor.liststore, element)
            renderer_text.connect('editing-started', self.on_table_cell_editing_started, anchor.liststore, element)
            column = gtk.TreeViewColumn("", renderer_text, text=element)
            column.set_min_width(table_col_min)
            column.set_clickable(True)
            column.set_widget(label)
            column.set_sizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
            column.connect('clicked', self.table_column_clicked, anchor, element)
            anchor.treeview.append_column(column)
        anchor.headers = headers
        anchor.table_col_min = table_col_min
        anchor.table_col_max = table_col_max
        anchor.treeview.set_grid_lines(gtk.TREE_VIEW_GRID_LINES_BOTH)
        anchor.treeview.connect('button-press-event', self.on_mouse_button_clicked_table, anchor)
        anchor.frame = gtk.Frame()
        anchor.frame.add(anchor.treeview)
        anchor.frame.set_shadow_type(gtk.SHADOW_NONE)
        anchor.eventbox = gtk.EventBox()
        anchor.eventbox.add(anchor.frame)
        self.dad.sourceview.add_child_at_anchor(anchor.eventbox, anchor)
        anchor.eventbox.show_all()
        for row in range(self.dad.table_rows):
            row_iter = anchor.liststore.append([""]*self.dad.table_columns)
            if table != None:
                for column in range(self.dad.table_columns):
                    try: anchor.liststore[row_iter][column] = table['matrix'][row][column]
                    except: pass # there are cases when some rows have less columns
        if table_justification:
            text_iter = text_buffer.get_iter_at_child_anchor(anchor)
            self.dad.state_machine.apply_object_justification(text_iter, table_justification, text_buffer)
        elif self.dad.user_active:
            # if I apply a justification, the state is already updated
            self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])

    def table_edit_properties(self, *args):
        """Edit Table Properties"""
        self.dad.table_col_min = self.curr_table_anchor.table_col_min
        self.dad.table_col_max = self.curr_table_anchor.table_col_max
        ret_ok, ret_csv = self.dialog_tablehandle(_("Edit Table Properties"), False)
        if not ret_ok: return
        table = self.dad.state_machine.table_to_dict(self.curr_table_anchor)
        table['col_min'] = self.dad.table_col_min
        table['col_max'] = self.dad.table_col_max
        iter_insert = self.dad.curr_buffer.get_iter_at_child_anchor(self.curr_table_anchor)
        table_justification = self.dad.state_machine.get_iter_alignment(iter_insert)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        self.dad.curr_buffer.delete(iter_insert, iter_bound)
        self.table_insert(iter_insert, table, table_justification)

    def on_key_press_table_cell(self, widget, event, path, model, col_num):
        """Catches Table Cell key presses"""
        keyname = gtk.gdk.keyval_name(event.keyval)
        if event.state & gtk.gdk.SHIFT_MASK:
            pass
        elif event.state & gtk.gdk.MOD1_MASK:
            pass
        elif event.state & gtk.gdk.CONTROL_MASK:
            if keyname == "period":
                cursor_pos = widget.get_position()
                widget.insert_text(cons.CHAR_NEWLINE, cursor_pos)
                widget.set_position(cursor_pos+1)
        else:
            if keyname in [cons.STR_RETURN, "Up", "Down"]:
                if model[path][col_num] != widget.get_text():
                    model[path][col_num] = widget.get_text()
                    self.dad.update_window_save_needed("nbuf", True)
                if keyname == "Up":
                    if col_num > 0:
                        next_col_num = col_num - 1
                        next_path = path
                    else:
                        next_iter = None
                        next_path =  model.get_path(model.get_iter(path))
                        while not next_iter and next_path[0] > 0:
                            node_path_list = list(next_path)
                            node_path_list[0] -= 1
                            next_path = tuple(node_path_list)
                            next_iter = model.get_iter(next_path)
                        #next_iter = model.iter_next(model.get_iter(path))
                        if not next_iter: return
                        next_path = model.get_path(next_iter)
                        next_col_num = self.dad.table_columns-1
                else:
                    if col_num < self.dad.table_columns-1:
                        next_col_num = col_num + 1
                        next_path = path
                    else:
                        next_iter = model.iter_next(model.get_iter(path))
                        if not next_iter: return
                        next_path = model.get_path(next_iter)
                        next_col_num = 0
                #print "(path, col_num) = (%s, %s)" % (path, col_num)
                #print "(next_path, next_col_num) = (%s, %s)" % (next_path, next_col_num)
                next_column = self.curr_table_anchor.treeview.get_columns()[next_col_num]
                self.curr_table_anchor.treeview.set_cursor_on_cell(next_path,
                                                                   focus_column=next_column,
                                                                   focus_cell=next_column.get_cell_renderers()[0],
                                                                   start_editing=True)

    def on_table_cell_editing_started(self, cell, editable, path, model, col_num):
        """A Table Cell is going to be Edited"""
        if isinstance(editable, gtk.Entry):
            editable.connect('key_press_event', self.on_key_press_table_cell, path, model, col_num)

    def on_table_cell_edited(self, cell, path, new_text, model, col_num):
        """A Table Cell has been Edited"""
        if model[path][col_num] != new_text:
            model[path][col_num] = new_text
            self.dad.update_window_save_needed("nbuf", True)

    def table_column_clicked(self, column, anchor, col_num):
        """The Column Header was Clicked"""
        col_label = column.get_widget()
        ret_ok, ret_rename, ret_add = self.dialog_tablecolhandle(_("Table Column Action"), col_label.get_text())
        if not ret_ok: return
        table = self.dad.state_machine.table_to_dict(anchor)
        headers = table['matrix'].pop()
        if (self.dad.table_column_mode == 'right' and col_num == len(headers)-1)\
        or (self.dad.table_column_mode == 'left' and col_num == 0):
            return
        iter_insert = self.dad.curr_buffer.get_iter_at_child_anchor(anchor)
        table_justification = self.dad.state_machine.get_iter_alignment(iter_insert)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        self.dad.curr_buffer.delete(iter_insert, iter_bound)
        if self.dad.table_column_mode == 'rename':
            new_label = ret_rename
            col_label.set_text("<b>" + new_label + "</b>")
            col_label.set_use_markup(True)
            headers[col_num] = new_label
        elif self.dad.table_column_mode == 'add':
            headers.insert(col_num + 1, ret_add)
            for row in table['matrix']: row.insert(col_num + 1, "")
        elif self.dad.table_column_mode == 'delete':
            headers.pop(col_num)
            for row in table['matrix']: row.pop(col_num)
        elif self.dad.table_column_mode == 'right':
            temp = headers.pop(col_num)
            headers.insert(col_num + 1, temp)
            for row in table['matrix']:
                temp = row.pop(col_num)
                row.insert(col_num + 1, temp)
        elif self.dad.table_column_mode == 'left':
            temp = headers.pop(col_num)
            headers.insert(col_num - 1, temp)
            for row in table['matrix']:
                temp = row.pop(col_num)
                row.insert(col_num - 1, temp)
        table['matrix'].append(headers)
        self.table_insert(iter_insert, table, table_justification)

    def table_row_action(self, action):
        """All Rows Actions"""
        treeviewselection = self.curr_table_anchor.treeview.get_selection()
        model, iter = treeviewselection.get_selected()
        if action == "delete": model.remove(iter)
        elif action == "add": model.insert_after(iter, [""]*len(self.curr_table_anchor.headers))
        elif action == "move_up":
            prev_iter = self.dad.get_tree_iter_prev_sibling(model, iter)
            if prev_iter == None: return
            model.swap(iter, prev_iter)
            self.curr_table_anchor.treeview.set_cursor(model.get_path(iter))
        elif action == "move_down":
            subseq_iter = model.iter_next(iter)
            if subseq_iter == None: return
            model.swap(iter, subseq_iter)
        elif action == "sort_desc":
            father_iter = model.iter_parent(iter)
            movements = False
            while self.dad.node_siblings_sort_iteration(model, father_iter, True, 0):
                movements = True
            if not movements: return
        elif action == "sort_asc":
            father_iter = model.iter_parent(iter)
            movements = False
            while self.dad.node_siblings_sort_iteration(model, father_iter, False, 0):
                movements = True
            if not movements: return
        elif action in ["cut", "copy"]:
            columns_num = len(self.curr_table_anchor.headers)
            table = {'matrix':[],
                     'col_min': self.curr_table_anchor.table_col_min,
                     'col_max': self.curr_table_anchor.table_col_max}
            row = []
            for column in range(columns_num): row.append(self.curr_table_anchor.liststore[iter][column])
            table['matrix'].append(row)
            table['matrix'].append(copy.deepcopy(self.curr_table_anchor.headers))
            self.dad.clipboard_handler.table_row_to_clipboard(table)
            if action == "cut": model.remove(iter)
            else: return
        elif action == "paste":
            if not self.dad.clipboard_handler.table_row_paste([model, iter]): return
        else: return
        self.dad.update_window_save_needed("nbuf", True)

    def table_row_add(self, *args):
        """Add a Table Row"""
        self.table_row_action("add")

    def table_row_cut(self, *args):
        """Cut a Table Row"""
        self.table_row_action("cut")

    def table_row_copy(self, *args):
        """Copy a Table Row"""
        self.table_row_action("copy")

    def table_row_paste(self, *args):
        """Paste a Table Row"""
        self.table_row_action("paste")

    def table_row_delete(self, *args):
        """Delete a Table Row"""
        self.table_row_action("delete")

    def table_row_up(self, *args):
        """Move the Selected Row Up"""
        self.table_row_action("move_up")

    def table_row_down(self, *args):
        """Move the Selected Row Down"""
        self.table_row_action("move_down")

    def table_rows_sort_descending(self, *args):
        """Sort all the Rows Descending"""
        self.table_row_action("sort_desc")

    def table_rows_sort_ascending(self, *args):
        """Sort all the Rows Ascending"""
        self.table_row_action("sort_asc")

    def on_mouse_button_clicked_table(self, widget, event, anchor):
        """Catches mouse buttons clicks"""
        self.curr_table_anchor = anchor
        self.dad.object_set_selection(self.curr_table_anchor)
        if event.button == 3:
            self.dad.ui.get_widget("/TableMenu").popup(None, None, None, event.button, event.time)


class UTF8Recoder:
    """
    Iterator that reads an encoded stream and reencodes the input to UTF-8
    """
    def __init__(self, f, encoding):
        self.reader = codecs.getreader(encoding)(f)

    def __iter__(self):
        return self

    def next(self):
        return self.reader.next().encode(cons.STR_UTF8)


class UnicodeReader:
    """
    A CSV reader which will iterate over lines in the CSV file "f",
    which is encoded in the given encoding.
    """
    def __init__(self, f, dialect=csv.excel, encoding=cons.STR_UTF8, **kwds):
        f = UTF8Recoder(f, encoding)
        self.reader = csv.reader(f, dialect=dialect, **kwds)

    def next(self):
        try:
            row = self.reader.next()
            return [unicode(s, cons.STR_UTF8, cons.STR_IGNORE) for s in row]
        except: return None

    def __iter__(self):
        return self


class UnicodeWriter:
    """
    A CSV writer which will write rows to CSV file "f",
    which is encoded in the given encoding.
    """

    def __init__(self, f, dialect=csv.excel, encoding=cons.STR_UTF8, **kwds):
        # Redirect output to a queue
        self.queue = cStringIO.StringIO()
        self.writer = csv.writer(self.queue, dialect=dialect, **kwds)
        self.stream = f
        self.encoder = codecs.getincrementalencoder(encoding)()

    def writerow(self, row):
        self.writer.writerow([s.encode(cons.STR_UTF8) for s in row])
        # Fetch UTF-8 output from the queue ...
        data = self.queue.getvalue()
        data = data.decode(cons.STR_UTF8)
        # ... and reencode it into the target encoding
        data = self.encoder.encode(data)
        # write to the target stream
        self.stream.write(data)
        # empty queue
        self.queue.truncate(0)

    def writerows(self, rows):
        for row in rows:
            self.writerow(row)
