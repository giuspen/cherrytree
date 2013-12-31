# -*- coding: UTF-8 -*-
#
#       printing.py
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

import gtk, gobject, pango, cairo
import copy, cgi
import support, cons

BOX_OFFSET = 4


class PrintData:
    """Print Operation Data"""
    text = None
    layout = None
    layout_is_new_line = None
    layout_num_lines = None
    page_breaks = None
    all_lines_y = None


class PrintHandler:
    """Handler for the CherryTree Prints"""

    def __init__(self):
        """Instantiate Variables"""
        self.active_prints = []
        self.settings = None
        self.page_setup = None
        self.pdf_filepath = ""

    def get_print_operation(self):
        """Return a Print Operation"""
        print_operation = gtk.PrintOperation()
        print_operation.set_show_progress(True)
        if self.page_setup is None: self.page_setup = gtk.PageSetup()
        if self.settings is None: self.settings = gtk.PrintSettings()
        print_operation.set_default_page_setup(self.page_setup)
        print_operation.set_print_settings(self.settings)
        return print_operation

    def run_print_operation(self, print_operation, parent):
        """Run a Ready Print Operation"""
        if self.pdf_filepath: print_operation.set_export_filename(self.pdf_filepath)
        print_operation_action = gtk.PRINT_OPERATION_ACTION_EXPORT if self.pdf_filepath else gtk.PRINT_OPERATION_ACTION_PRINT_DIALOG
        try: res = print_operation.run(print_operation_action, parent)
        except gobject.GError, ex:
            support.dialog_error("Error printing file:\n%s (exception catched)" % str(ex), parent)
        else:
            if res == gtk.PRINT_OPERATION_RESULT_ERROR:
                support.dialog_error("Error printing file (bad res)", parent)
            elif res == gtk.PRINT_OPERATION_RESULT_APPLY:
                self.settings = print_operation.get_print_settings()
        if not print_operation.is_finished():
            print_operation.connect("status_changed", self.on_print_status_changed)

    def on_print_status_changed(self, operation):
        """Print Operation Status Changed"""
        if operation.is_finished(): active_prints.remove(operation)

    def print_text(self, window, pango_text, text_font, code_font, pixbuf_table_codebox_vector, text_window_width):
        """Start the Print Operations for Text"""
        self.pango_font = pango.FontDescription(text_font)
        self.codebox_font = pango.FontDescription(code_font)
        self.text_window_width = text_window_width
        self.table_text_row_height = self.pango_font.get_size()/pango.SCALE
        self.table_line_thickness = 6
        self.pixbuf_table_codebox_vector = pixbuf_table_codebox_vector
        # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
        print_data = PrintData()
        print_data.text = pango_text
        print_operation = self.get_print_operation()
        print_operation.connect("begin_print", self.on_begin_print_text, print_data)
        print_operation.connect("draw_page", self.on_draw_page_text, print_data)
        self.run_print_operation(print_operation, window)

    def on_begin_print_text(self, operation, context, print_data):
        """Here we Compute the Lines Positions, the Number of Pages Needed and the Page Breaks"""
        self.page_width = context.get_width()
        self.page_height = context.get_height() * 1.02 # tolerance at bottom of the page
        while 1:
            exit_ok = True
            print_data.layout = []
            print_data.forced_page_break = []
            print_data.layout_is_new_line = []
            print_data.layout_num_lines = []
            print_data.all_lines_y = []
            #print "print_data.text", print_data.text
            for i, text_slot in enumerate(print_data.text):
                print_data.layout.append(context.create_pango_layout())
                print_data.layout[-1].set_font_description(self.pango_font)
                print_data.layout[-1].set_width(int(self.page_width*pango.SCALE))
                is_forced_page_break = text_slot.startswith(2*cons.CHAR_NEWPAGE)
                print_data.forced_page_break.append(is_forced_page_break)
                print_data.layout[-1].set_markup(text_slot if not is_forced_page_break else text_slot[2:])
                if text_slot == cons.CHAR_NEWLINE:
                    print_data.layout_is_new_line.append(True) # in other case we detect the newline from a following line
                else: print_data.layout_is_new_line.append(False) # but here we have a single layout line
                print_data.layout_num_lines.append(print_data.layout[-1].get_line_count())
            self.y_idx = 0
            print_data.page_breaks = []
            curr_y = float(0)
            inline_pending_height = float(0)
            inline_starter = [0, 0]
            for i, layout in enumerate(print_data.layout):
                #print "layout", i
                # text
                if print_data.forced_page_break[i] and curr_y > 0:
                    #print "forced_page_break"
                    print_data.page_breaks.append(inline_starter)
                    curr_y = 0;
                layout_line_idx = 0
                while layout_line_idx < print_data.layout_num_lines[i]:
                    #print "layout_line_idx", layout_line_idx
                    layout_line = print_data.layout[i].get_line(layout_line_idx)
                    line_width, line_height = self.layout_line_get_width_height(layout_line)
                    # process the line
                    if line_height > inline_pending_height: inline_pending_height = line_height
                    if layout_line_idx < print_data.layout_num_lines[i]-1 or print_data.layout_is_new_line[i]:
                        if curr_y + inline_pending_height > self.page_height:
                            print_data.page_breaks.append(inline_starter)
                            #print "added page break", inline_starter
                            curr_y = 0
                            if inline_pending_height > self.page_height:
                                if self.pixbuf_table_codebox_vector[i-1][0] == "codebox"\
                                and codebox_height > self.page_height:
                                    self.codebox_long_split(i-1, context, print_data)
                                    exit_ok = False
                                    break # go to a new main loop
                        curr_y += inline_pending_height
                        print_data.all_lines_y.append(curr_y)
                        #print "added line y <%s> (%s, %s)" % (curr_y, i, layout_line_idx)
                        inline_pending_height = 0 # reset the pending elements line to append
                        inline_starter = [i, layout_line_idx+1]
                    layout_line_idx += 1
                if not exit_ok: break # go to a new main loop
                # pixbuf or table or codebox
                if i < len(print_data.layout) - 1: # the latest element is supposed to be text
                    if self.pixbuf_table_codebox_vector[i][0] == "pixbuf":
                        pixbuf = self.pixbuf_table_codebox_vector[i][1][1]
                        pixbuf_was_resized = False
                        pixbuf_width = pixbuf.get_width()
                        pixbuf_height = pixbuf.get_height()
                        if pixbuf_width > self.page_width:
                            image_w_h_ration = float(pixbuf_width)/pixbuf_height
                            image_width = self.page_width
                            image_height = image_width / image_w_h_ration
                            pixbuf = pixbuf.scale_simple(int(image_width),
                                                         int(image_height),
                                                         gtk.gdk.INTERP_BILINEAR)
                            pixbuf_was_resized = True
                        if pixbuf_height > self.page_height:
                            image_w_h_ration = float(pixbuf_width)/pixbuf_height
                            image_height = self.page_height
                            image_width = image_height * image_w_h_ration
                            pixbuf = pixbuf.scale_simple(int(image_width),
                                                         int(image_height),
                                                         gtk.gdk.INTERP_BILINEAR)
                            pixbuf_was_resized = True
                        if pixbuf_was_resized:
                            self.pixbuf_table_codebox_vector[i][1][1] = pixbuf
                        pixbuf_height = pixbuf.get_height() + cons.WHITE_SPACE_BETW_PIXB_AND_TEXT
                        if inline_pending_height < pixbuf_height: inline_pending_height = pixbuf_height
                    elif self.pixbuf_table_codebox_vector[i][0] == "table":
                        table = self.pixbuf_table_codebox_vector[i][1][1]
                        table['matrix'].insert(0, table['matrix'].pop()) # let's put the title to first row
                        table_layouts = self.get_table_layouts(context, table)
                        table_grid = self.get_table_grid(table_layouts, table['col_min'])
                        table_height = self.get_table_height_from_grid(table_grid)
                        if inline_pending_height < table_height+BOX_OFFSET: inline_pending_height = table_height+BOX_OFFSET
                    elif self.pixbuf_table_codebox_vector[i][0] == "codebox":
                        codebox_dict = self.pixbuf_table_codebox_vector[i][1][1]
                        codebox_layout = self.get_codebox_layout(context, codebox_dict)
                        codebox_height = self.get_height_from_layout(codebox_layout)
                        if inline_pending_height < codebox_height+BOX_OFFSET: inline_pending_height = codebox_height+BOX_OFFSET
            if exit_ok: break
        operation.set_n_pages(len(print_data.page_breaks) + 1)

    def on_draw_page_text(self, operation, context, page_nr, print_data):
        """This Function is Called For Each Page Set in on_begin_print_text"""
        if page_nr == 0: start_line_num = [0, 0] # layout num, line num
        else: start_line_num = print_data.page_breaks[page_nr - 1]
        #print "start_line_num", start_line_num
        if page_nr < len(print_data.page_breaks): end_line_num = print_data.page_breaks[page_nr]
        else: end_line_num = [len(print_data.layout)-1, print_data.layout_num_lines[-1]]
        #print "end_line_num", end_line_num
        cairo_context = context.get_cairo_context()
        cairo_context.set_source_rgb(0.5, 0.5, 0.5)
        cairo_context.set_font_size(12)
        page_num_str = "%s/%s" % (page_nr+1, operation.get_property("n-pages"))
        cairo_context.move_to(self.page_width/2, self.page_height+17)
        cairo_context.show_text(page_num_str)
        curr_x = float(0)
        i = start_line_num[0]
        layout_line_idx = start_line_num[1]
        while i <= end_line_num[0]:
            # text
            cairo_context.set_source_rgb(0, 0, 0)
            if i > start_line_num[0]: layout_line_idx = 0 # reset line idx
            while layout_line_idx < print_data.layout_num_lines[i]:
                layout_line = print_data.layout[i].get_line(layout_line_idx)
                line_width, line_height = self.layout_line_get_width_height(layout_line)
                # process the line
                if line_width > 0:
                    #print "text (%s, %s) to (%s, %s)" % (line_width, line_height, curr_x, print_data.all_lines_y[self.y_idx])
                    cairo_context.move_to(curr_x, print_data.all_lines_y[self.y_idx])
                    cairo_context.show_layout_line(layout_line)
                    curr_x += line_width
                if layout_line_idx < print_data.layout_num_lines[i]-1 or print_data.layout_is_new_line[i]:
                    curr_x = 0.0
                    self.y_idx += 1
                    #print "new index value <%s> (%s, %s)" % (self.y_idx, i, layout_line_idx)
                layout_line_idx += 1
                if i >= end_line_num[0] and layout_line_idx >= end_line_num[1]: return
            # pixbuf or table or codebox
            if i < len(print_data.layout) - 1: # the latest element is supposed to be text
                if self.pixbuf_table_codebox_vector[i][0] == "pixbuf":
                    pixbuf = self.pixbuf_table_codebox_vector[i][1][1]
                    pixbuf_width = pixbuf.get_width()
                    pixbuf_height = pixbuf.get_height()
                    #print "pixbuf (%s, %s) to (%s, %s)" % (pixbuf_width, pixbuf_height, curr_x, print_data.all_lines_y[self.y_idx]-pixbuf_height)
                    cairo_context.set_source_pixbuf(pixbuf, curr_x, print_data.all_lines_y[self.y_idx] - pixbuf_height)
                    cairo_context.paint()
                    curr_x += pixbuf_width
                elif self.pixbuf_table_codebox_vector[i][0] == "table":
                    table = self.pixbuf_table_codebox_vector[i][1][1]
                    table_layouts = self.get_table_layouts(context, table)
                    table_grid = self.get_table_grid(table_layouts, table['col_min'])
                    table_width = self.get_table_width_from_grid(table_grid)
                    table_height = self.get_table_height_from_grid(table_grid)
                    self.table_draw_grid(cairo_context,
                                         table_grid,
                                         curr_x,
                                         print_data.all_lines_y[self.y_idx] - table_height,
                                         table_width,
                                         table_height)
                    self.table_draw_text(cairo_context,
                                         table_grid,
                                         table_layouts,
                                         curr_x,
                                         print_data.all_lines_y[self.y_idx] - table_height)
                    curr_x += table_width
                elif self.pixbuf_table_codebox_vector[i][0] == "codebox":
                    codebox_dict = self.pixbuf_table_codebox_vector[i][1][1]
                    codebox_layout = self.get_codebox_layout(context, codebox_dict)
                    codebox_height = self.get_height_from_layout(codebox_layout)
                    codebox_width = self.get_width_from_layout(codebox_layout)
                    #print "codebox (%s, %s) to (%s, %s)" % (codebox_width, codebox_height, curr_x, print_data.all_lines_y[self.y_idx]-codebox_height)
                    self.codebox_draw_box(cairo_context,
                                          curr_x,
                                          print_data.all_lines_y[self.y_idx] - codebox_height,
                                          codebox_width,
                                          codebox_height)
                    self.codebox_draw_code(cairo_context,
                                           codebox_layout,
                                           curr_x,
                                           print_data.all_lines_y[self.y_idx] - codebox_height)
                    curr_x += codebox_width
            i += 1 # layout increment

    def get_codebox_layout(self, context, codebox_dict):
        """Return the CodeBox Layout"""
        layout = context.create_pango_layout()
        layout.set_font_description(self.codebox_font)
        if codebox_dict['width_in_pixels']: codebox_width = codebox_dict['frame_width']
        else: codebox_width = self.text_window_width*codebox_dict['frame_width']/100
        if codebox_width > self.page_width: codebox_width = self.page_width
        layout.set_width(int(codebox_width*pango.SCALE))
        layout.set_wrap(pango.WRAP_WORD_CHAR)
        layout.set_markup(codebox_dict['fill_text'])
        return layout

    def get_height_from_layout(self, layout):
        """Returns the Height given the Layout"""
        height = 0
        num_layout_lines = layout.get_line_count()
        for layout_line_idx in range(num_layout_lines):
            layout_line = layout.get_line(layout_line_idx)
            line_width, line_height = self.layout_line_get_width_height(layout_line)
            height += line_height
        return height + 2*cons.GRID_SLIP_OFFSET

    def get_width_from_layout(self, layout):
        """Returns the Height given the Layout"""
        width = 0
        num_layout_lines = layout.get_line_count()
        for layout_line_idx in range(num_layout_lines):
            layout_line = layout.get_line(layout_line_idx)
            line_width, line_height = self.layout_line_get_width_height(layout_line)
            if line_width > width: width = line_width
        return width + 2*cons.GRID_SLIP_OFFSET

    def get_table_layouts(self, context, table):
        """Return the Table Cells Layouts"""
        table_layouts = []
        for i, table_row in enumerate(table['matrix']):
            table_layouts.append([])
            for j, cell_text in enumerate(table_row):
                layout = context.create_pango_layout()
                layout.set_font_description(self.pango_font)
                cell_text = cgi.escape(cell_text)
                if i == 0: cell_text = "<b>" + cell_text + "</b>"
                layout.set_width(int(table['col_max']*pango.SCALE))
                layout.set_wrap(pango.WRAP_WORD_CHAR)
                layout.set_markup(cell_text)
                table_layouts[i].append(layout)
        return table_layouts

    def layout_line_get_width_height(self, layout_line):
        """Returns Width and Height of a layout line"""
        ink_rect, logical_rect = layout_line.get_extents()
        lx, ly, lwidth, lheight = logical_rect
        return [lwidth / 1024.0, lheight / 1024.0]

    def get_table_grid(self, table_layouts, col_min):
        """Returns the Dimensions of Rows and Columns"""
        rows_h = [0] * len(table_layouts)
        cols_w = [col_min] * len(table_layouts[0])
        for i, layout_row in enumerate(table_layouts):
            for j, layout_cell in enumerate(layout_row):
                cell_height = 0
                num_layout_lines = layout_cell.get_line_count()
                for layout_line_idx in range(num_layout_lines):
                    layout_line = layout_cell.get_line(layout_line_idx)
                    line_width, line_height = self.layout_line_get_width_height(layout_line)
                    cell_height += line_height
                    if cols_w[j] < line_width: cols_w[j] = line_width
                if rows_h[i] < cell_height: rows_h[i] = cell_height
        return [rows_h, cols_w]

    def get_table_width_from_grid(self, table_grid):
        """Returns the Table Width given the table_grid vector"""
        table_width = 0
        for col_w in table_grid[1]:
            table_width += (col_w + self.table_line_thickness)
        return table_width

    def get_table_height_from_grid(self, table_grid):
        """Returns the Table Height given the table_grid vector"""
        table_height = 0
        for row_h in table_grid[0]:
            table_height += (row_h + self.table_line_thickness)
        return table_height

    def codebox_draw_box(self, cairo_context, x0, y0, codebox_width, codebox_height):
        """Draw the CodeBox Box"""
        cairo_context.set_source_rgba(0, 0, 0, 0.3)
        cairo_context.rectangle(x0, y0, codebox_width, codebox_height)
        cairo_context.stroke()

    def table_draw_grid(self, cairo_context, table_grid, x0, y0, table_width, table_height):
        """Draw the Table Grid"""
        x = x0
        y = y0
        cairo_context.set_source_rgba(0, 0, 0, 0.3)
        # draw lines
        cairo_context.move_to(x, y)
        cairo_context.line_to(x + table_width, y)
        for row_h in table_grid[0]:
            y += (row_h + self.table_line_thickness)
            cairo_context.move_to(x, y)
            cairo_context.line_to(x + table_width, y)
        # draw columns
        y = y0
        cairo_context.move_to(x, y)
        cairo_context.line_to(x, y + table_height)
        for col_w in table_grid[1]:
            x += (col_w + self.table_line_thickness)
            cairo_context.move_to(x, y)
            cairo_context.line_to(x, y + table_height)
        cairo_context.stroke()

    def table_draw_text(self, cairo_context, table_grid, table_layouts, x0, y0):
        """Draw the text inside of the Table Cells"""
        cairo_context.set_source_rgb(0, 0, 0)
        y = y0
        for i, row_h in enumerate(table_grid[0]):
            x = x0 + cons.GRID_SLIP_OFFSET
            for j, col_w in enumerate(table_grid[1]):
                layout_cell = table_layouts[i][j]
                local_y = y
                num_layout_lines = layout_cell.get_line_count()
                for layout_line_idx in range(num_layout_lines):
                    layout_line = layout_cell.get_line(layout_line_idx)
                    line_width, line_height = self.layout_line_get_width_height(layout_line)
                    cairo_context.move_to(x, local_y + line_height)
                    local_y += line_height
                    cairo_context.show_layout_line(layout_line)
                x += col_w + self.table_line_thickness
            y += row_h + self.table_line_thickness

    def codebox_draw_code(self, cairo_context, codebox_layout, x0, y0):
        """Draw the code inside of the Box"""
        cairo_context.set_source_rgb(0, 0, 0)
        y = y0
        num_layout_lines = codebox_layout.get_line_count()
        for layout_line_idx in range(num_layout_lines):
            layout_line = codebox_layout.get_line(layout_line_idx)
            line_width, line_height = self.layout_line_get_width_height(layout_line)
            cairo_context.move_to(x0 + cons.GRID_SLIP_OFFSET, y + line_height)
            y += line_height
            cairo_context.show_layout_line(layout_line)

    def codebox_long_split(self, idx, context, print_data):
        """Split Long CodeBoxes"""
        codebox_dict = self.pixbuf_table_codebox_vector[idx][1][1]
        original_splitted_pango = codebox_dict['fill_text'].split(cons.CHAR_NEWLINE)
        splitted_pango = copy.deepcopy(original_splitted_pango)
        codebox_dict_jolly = copy.deepcopy(codebox_dict)
        partial_pango_vec = []
        while len(splitted_pango) > 1:
            splitted_pango = splitted_pango[:-1]
            partial_pango = cons.CHAR_NEWLINE.join(splitted_pango)
            codebox_dict_jolly['fill_text'] = partial_pango
            codebox_layout = self.get_codebox_layout(context, codebox_dict_jolly)
            codebox_height = self.get_height_from_layout(codebox_layout)
            if codebox_height < self.page_height:
                # this slot is done
                partial_pango_vec.append(partial_pango)
                # let's get ready for the next slot
                splitted_pango = original_splitted_pango[len(splitted_pango):]
                original_splitted_pango = splitted_pango
                partial_pango = cons.CHAR_NEWLINE.join(splitted_pango)
                codebox_dict_jolly['fill_text'] = partial_pango
                codebox_layout = self.get_codebox_layout(context, codebox_dict_jolly)
                codebox_height = self.get_height_from_layout(codebox_layout)
                if codebox_height < self.page_height:
                    # this is the last piece
                    partial_pango_vec.append(partial_pango)
                    break
        for i, element in enumerate(partial_pango_vec):
            if i == 0: codebox_dict['fill_text'] = element
            else:
                index = idx+i
                # add a newline
                print_data.text.insert(index, cons.CHAR_NEWLINE)
                # add a codebox
                new_codebox_dict = copy.deepcopy(codebox_dict)
                new_codebox_dict['fill_text'] = element
                pixbuf_table_codebox_element = ["codebox", [None, new_codebox_dict, None]]
                self.pixbuf_table_codebox_vector.insert(index, pixbuf_table_codebox_element)
