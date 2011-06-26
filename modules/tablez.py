# -*- coding: UTF-8 -*-
#
#       tablez.py
#
#       Copyright 2009-2011 Giuseppe Penone <giuspen@gmail.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
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

from gi.repository import Gtk, Pango
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

    def on_key_press_tablehandledialog(self, widget, event):
        """Catches TableHandle Dialog key presses"""
        keyname = Gdk.keyval_name(event.keyval)
        if keyname == "Return": self.dad.glade.tablehandledialog_button_ok.clicked()

    def table_export(self, action):
        """Table Export as CSV File"""
        filename = support.dialog_file_save_as(curr_folder=self.dad.pick_dir,
                                               filter_pattern="*.csv",
                                               filter_name=_("CSV File"),
                                               parent=self.dad.window)
        if filename == None: return
        if not os.path.isfile(filename)\
        or support.dialog_question(_("The File %s\nAlready Exists, do you want to Overwrite?") % filename, self.dad.window):
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
        self.dad.glade.tablehandledialog.set_title(_("Insert Table"))
        self.dad.glade.tablehandle_frame_table.show()
        self.dad.glade.tablehandle_frame_col.show()
        self.dad.glade.checkbutton_table_ins_from_file.set_active(False)
        self.dad.glade.checkbutton_table_ins_from_file.show()
        self.dad.glade.tablehandle_vbox_col.hide()
        response = self.dad.glade.tablehandledialog.run()
        self.dad.glade.tablehandledialog.hide()
        if response != 1: return
        if not self.dad.glade.checkbutton_table_ins_from_file.get_active():
            self.table_rows = int(self.dad.glade.spinbutton_table_rows.get_value())
            self.table_columns = int(self.dad.glade.spinbutton_table_columns.get_value())
            self.table_col_min = int(self.dad.glade.spinbutton_table_col_min.get_value())
            self.table_col_max = int(self.dad.glade.spinbutton_table_col_max.get_value())
            self.table_insert(iter_insert)
        else:
            filepath = support.dialog_file_select(filter_pattern="*.csv",
                                                  filter_name=_("CSV File"),
                                                  curr_folder=self.dad.pick_dir,
                                                  parent=self.dad.window)
            if filepath != None:
                self.dad.pick_dir = os.path.dirname(filepath)
                file_descriptor = open(filepath, "r")
                reader = UnicodeReader(file_descriptor)
                table_matrix = []
                row = reader.next()
                while row:
                    table_matrix.append(row)
                    row = reader.next()
                file_descriptor.close()
                table_matrix.append(table_matrix.pop(0))
                self.table_insert(iter_insert, {'col_min': 40, 'col_max': 1000, 'matrix': table_matrix})

    def table_insert(self, iter_insert, table=None, table_justification=None):
        """Insert a Table at the Given Iter"""
        if table != None:
            self.table_columns = len(table['matrix'][0])
            self.table_rows = len(table['matrix']) - 1
            headers = table['matrix'][-1]
            table_col_min = table['col_min']
            table_col_max = table['col_max']
        else:
            headers = [_("click me")]*self.table_columns
            table_col_min = self.table_col_min
            table_col_max = self.table_col_max
        anchor = self.dad.curr_buffer.create_child_anchor(iter_insert)
        anchor.liststore = Gtk.ListStore(*(str,)*self.table_columns)
        anchor.treeview = Gtk.TreeView(anchor.liststore)
        for element in range(self.table_columns):
            label = Gtk.Label(label='<b>' + headers[element] + '</b>')
            label.set_use_markup(True)
            label.set_tooltip_text(_("Click to Edit the Column Settings"))
            label.show()
            renderer_text = Gtk.CellRendererText()
            renderer_text.set_property('editable', True)
            renderer_text.set_property('wrap-width', table_col_max)
            renderer_text.set_property('wrap-mode', Pango.WrapMode.WORD_CHAR)
            renderer_text.set_property('font-desc', Pango.FontDescription(self.dad.text_font))
            renderer_text.connect('edited', self.on_table_cell_edited, anchor.liststore, element)
            renderer_text.connect('editing-started', self.on_table_cell_editing_started, anchor.liststore, element)
            column = Gtk.TreeViewColumn("", renderer_text, text=element)
            column.set_min_width(table_col_min)
            column.set_clickable(True)
            column.set_widget(label)
            column.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
            column.connect('clicked', self.table_column_clicked, anchor, element)
            anchor.treeview.append_column(column)
        anchor.headers = headers
        anchor.table_col_min = table_col_min
        anchor.table_col_max = table_col_max
        anchor.treeview.set_grid_lines(Gtk.TREE_VIEW_GRID_LINES_BOTH)
        anchor.treeview.connect('button-press-event', self.on_mouse_button_clicked_table, anchor)
        anchor.frame = Gtk.Frame()
        anchor.frame.add(anchor.treeview)
        anchor.frame.set_shadow_type(Gtk.ShadowType.NONE)
        anchor.eventbox = Gtk.EventBox()
        anchor.eventbox.add(anchor.frame)
        self.dad.sourceview.add_child_at_anchor(anchor.eventbox, anchor)
        anchor.eventbox.show_all()
        for row in range(self.table_rows):
            row_iter = anchor.liststore.append([""]*self.table_columns)
            if table != None:
                for column in range(self.table_columns):
                    anchor.liststore[row_iter][column] = table['matrix'][row][column]
        if table_justification:
            self.dad.state_machine.apply_image_justification(self.dad.curr_buffer.get_iter_at_child_anchor(anchor), table_justification)

    def table_edit_properties(self, *args):
        """Edit Table Properties"""
        self.table_col_min = self.curr_table_anchor.table_col_min
        self.table_col_max = self.curr_table_anchor.table_col_max
        self.dad.glade.spinbutton_table_col_min.set_value(self.curr_table_anchor.table_col_min)
        self.dad.glade.spinbutton_table_col_max.set_value(self.curr_table_anchor.table_col_max)
        self.dad.glade.tablehandledialog.set_title(_("Edit Table Properties"))
        self.dad.glade.tablehandle_frame_table.hide()
        self.dad.glade.tablehandle_frame_col.show()
        self.dad.glade.tablehandle_vbox_col.hide()
        self.dad.glade.checkbutton_table_ins_from_file.hide()
        self.dad.glade.tablehandle_frame_col.set_sensitive(True)
        response = self.dad.glade.tablehandledialog.run()
        self.dad.glade.tablehandledialog.hide()
        if response != 1: return
        self.table_col_min = int(self.dad.glade.spinbutton_table_col_min.get_value())
        table = self.dad.state_machine.table_to_dict(self.curr_table_anchor)
        self.table_col_max = int(self.dad.glade.spinbutton_table_col_max.get_value())
        table['col_min'] = self.table_col_min
        table['col_max'] = self.table_col_max
        iter_insert = self.dad.curr_buffer.get_iter_at_child_anchor(self.curr_table_anchor)
        table_justification = self.dad.state_machine.get_iter_alignment(iter_insert)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        self.dad.curr_buffer.delete(iter_insert, iter_bound)
        self.table_insert(iter_insert, table, table_justification)

    def on_key_press_table_cell(self, widget, event, path, model, col_num):
        """Catches Table Cell key presses"""
        keyname = Gdk.keyval_name(event.keyval)
        if event.get_state() & Gdk.EventMask.SHIFT_MASK:
            pass
        elif event.get_state() & Gdk.ModifierType.MOD1_MASK:
            pass
        elif event.get_state() & Gdk.EventMask.CONTROL_MASK:
            pass
        else:
            if keyname in ["Return", "Up", "Down"]:
                if model[path][col_num] != widget.get_text():
                    model[path][col_num] = widget.get_text()
                    self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])
                    self.dad.update_window_save_needed()
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
                        next_col_num = self.table_columns-1
                else:
                    if col_num < self.table_columns-1:
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
        if isinstance(editable, Gtk.Entry):
            editable.connect('key_press_event', self.on_key_press_table_cell, path, model, col_num)

    def on_table_cell_edited(self, cell, path, new_text, model, col_num):
        """A Table Cell has been Edited"""
        if model[path][col_num] != new_text:
            model[path][col_num] = new_text
            self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])
            self.dad.update_window_save_needed()

    def table_column_clicked(self, column, anchor, col_num):
        """The Column Header was Clicked"""
        self.dad.glade.tablehandledialog.set_title(_("Table Column Action"))
        self.dad.glade.tablehandle_frame_table.hide()
        self.dad.glade.tablehandle_frame_col.hide()
        self.dad.glade.checkbutton_table_ins_from_file.hide()
        self.dad.glade.tablehandle_vbox_col.show()
        col_label = column.get_widget()
        self.dad.glade.table_column_rename_entry.set_text(col_label.get_text())
        self.dad.glade.table_column_rename_entry.grab_focus()
        self.dad.glade.table_column_new_entry.set_sensitive(self.dad.table_column_mode == 'add')
        self.dad.glade.table_column_rename_entry.set_sensitive(self.dad.table_column_mode == 'rename')
        response = self.dad.glade.tablehandledialog.run()
        self.dad.glade.tablehandledialog.hide()
        if response != 1: return
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
            new_label = self.dad.glade.table_column_rename_entry.get_text()
            col_label.set_text("<b>" + new_label + "</b>")
            col_label.set_use_markup(True)
            headers[col_num] = new_label
        elif self.dad.table_column_mode == 'add':
            headers.insert(col_num + 1, self.dad.glade.table_column_new_entry.get_text())
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
            self.curr_table_anchor.treeview.set_cursor(model.get_path(iter), None, False)
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
        self.dad.update_window_save_needed()
        self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])

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
        return self.reader.next().encode("utf-8")


class UnicodeReader:
    """
    A CSV reader which will iterate over lines in the CSV file "f",
    which is encoded in the given encoding.
    """
    def __init__(self, f, dialect=csv.excel, encoding="raw_unicode_escape", **kwds):
        f = UTF8Recoder(f, encoding)
        self.reader = csv.reader(f, dialect=dialect, **kwds)

    def next(self):
        try:
            row = self.reader.next()
            return [unicode(s, "utf-8") for s in row]
        except: return None

    def __iter__(self):
        return self


class UnicodeWriter:
    """
    A CSV writer which will write rows to CSV file "f",
    which is encoded in the given encoding.
    """

    def __init__(self, f, dialect=csv.excel, encoding="raw_unicode_escape", **kwds):
        # Redirect output to a queue
        self.queue = cStringIO.StringIO()
        self.writer = csv.writer(self.queue, dialect=dialect, **kwds)
        self.stream = f
        self.encoder = codecs.getincrementalencoder(encoding)()

    def writerow(self, row):
        self.writer.writerow([s.encode("utf-8") for s in row])
        # Fetch UTF-8 output from the queue ...
        data = self.queue.getvalue()
        data = data.decode("utf-8")
        # ... and reencode it into the target encoding
        data = self.encoder.encode(data)
        # write to the target stream
        self.stream.write(data)
        # empty queue
        self.queue.truncate(0)

    def writerows(self, rows):
        for row in rows:
            self.writerow(row)
