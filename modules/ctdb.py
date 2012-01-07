# -*- coding: UTF-8 -*-
#
#       ctdb.py
#
#       Copyright 2012 Giuseppe Penone <giuspen@gmail.com>
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

import gtk
import sqlite3, xml.dom.minidom
import cons, machines


class CTDBHandler:
    """Handler of the CherryTree DataBase"""

    def __init__(self, dad):
        """CherryTree DataBase boot"""
        self.dad = dad
    
    def get_image_db_tuple(self, image_element, node_id):
        """From image element to db tuple"""
        offset = image_element[0]
        pixbuf = image_element[1]
        justification = image_element[2]
        if not "anchor" in dir(pixbuf):
            anchor = ""
            png_blob = machines.get_blob_buffer_from_pixbuf(pixbuf)
        else:
            anchor = pixbuf.anchor
            png_blob = None
        return (node_id, offset, justification, anchor, png_blob)
    
    def get_table_db_tuple(self, table_element, node_id):
        """From table element to db tuple"""
        offset = table_element[0]
        table_dict = table_element[1]
        justification = table_element[2]
        col_min = table_dict['col_min']
        col_max = table_dict['col_max']
        table_dom = xml.dom.minidom.Document()
        dom_iter = table_dom.createElement("table")
        table_dom.appendChild(dom_iter)
        for row in table_dict['matrix']:
            dom_row = table_dom.createElement("row")
            dom_iter.appendChild(dom_row)
            for cell in row:
                dom_cell = table_dom.createElement("cell")
                dom_row.appendChild(dom_cell)
                text_iter = table_dom.createTextNode(cell)
                dom_cell.appendChild(text_iter)
        txt = table_dom.toxml()
        return (node_id, offset, justification, txt, col_min, col_max)
    
    def get_codebox_db_tuple(self, codebox_element, node_id):
        """From codebox element to db tuple"""
        offset = codebox_element[0]
        codebox_dict = codebox_element[1]
        justification = codebox_element[2]
        txt = codebox_dict['fill_text']
        syntax = codebox_dict['syntax_highlighting']
        width = codebox_dict['frame_width']
        height = codebox_dict['frame_height']
        is_width_pix = codebox_dict['width_in_pixels']
        do_highl_bra = codebox_dict['highlight_brackets']
        do_show_linenum = codebox_dict['show_line_numbers']
        return (node_id, offset, justification, txt, syntax,
                width, height, is_width_pix, do_highl_bra, do_show_linenum)
    
    def get_connected_db_from_dbpath(self, dbpath):
        """Returns DB connection descriptor given the dbpath"""
        return sqlite3.connect(dbpath)
    
    def new_db(self, dbpath):
        """Create a new DataBase"""
        db = self.get_connected_db_from_dbpath(dbpath)
        db.execute(cons.TABLE_NODE_CREATE)
        db.execute(cons.TABLE_CODEBOX_CREATE)
        db.execute(cons.TABLE_TABLE_CREATE)
        db.execute(cons.TABLE_IMAGE_CREATE)
        db.execute(cons.TABLE_CHILDREN_CREATE)
        db.execute(cons.TABLE_BOOKMARK_CREATE)
        self.write_db_full(db)
        db.commit()
        db.close()
    
    def write_db_bookmarks(self, db):
        """Write all the bookmarks in DB"""
        if not self.dad.bookmarks: return
        sequence = 0
        for bookmark_str in self.dad.bookmarks:
            sequence += 1
            bookmark_tuple = (int(bookmark_str), sequence)
            db.execute('REMOVE * FROM bookmark')
            db.execute('INSERT INTO bookmark VALUES(?,?)', bookmark_tuple)
    
    def write_db_node(self, db, tree_iter, level, sequence, node_father_id, write_dict):
        """Write a node in DB"""
        node_id = self.dad.treestore[tree_iter][3]
        child_tree_iter = self.dad.treestore.iter_children(tree_iter) # check for children
        has_children = 1 if child_tree_iter != None else 0
        name = self.dad.treestore[tree_iter][1]
        syntax = self.dad.treestore[tree_iter][4]
        tags = self.dad.treestore[tree_iter][6]
        is_ro = 1 if self.dad.treestore[tree_iter][7] else 0
        is_richtxt = 1 if syntax == cons.CUSTOM_COLORS_ID else 0
        if write_dict['buff']:
            curr_buffer = self.dad.treestore[tree_iter][2]
            start_iter = curr_buffer.get_start_iter()
            end_iter = curr_buffer.get_end_iter()
            if is_richtxt:
                # prepare xml dom node
                if "dom" in dir(self): del self.dom
                self.dom = xml.dom.minidom.Document()
                dom_iter = self.dom.createElement("node")
                self.dom.appendChild(dom_iter)
                # init attributes
                self.curr_attributes = {}
                for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
                # go!
                curr_iter = start_iter.copy()
                self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
                tag_found = curr_iter.forward_to_tag_toggle(None)
                while tag_found:
                    self.dad.xml_handler.rich_txt_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes, dom=self.dom)
                    if curr_iter.compare(end_iter) == 0: break
                    else:
                        self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
                        offset_old = curr_iter.get_offset()
                        start_iter.set_offset(offset_old)
                        tag_found = curr_iter.forward_to_tag_toggle(None)
                        if curr_iter.get_offset() == offset_old: break
                else:  self.dad.xml_handler.rich_txt_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes, dom=self.dom)
                # time for the objects
                if write_dict['upd']:
                    db.execute('REMOVE FROM codebox WHERE node_id=?', node_id)
                    db.execute('REMOVE FROM grid WHERE node_id=?', node_id)
                    db.execute('REMOVE FROM image WHERE node_id=?', node_id)
                pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer)
                # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
                codeboxes_tuples = []
                tables_tuples = []
                images_tuples = []
                for element in pixbuf_table_codebox_vector:
                    if element[0] == "pixbuf": images_tuples.append(self.get_image_db_tuple(element[1], node_id))
                    elif element[0] == "table": tables_tuples.append(self.get_table_db_tuple(element[1], node_id))
                    elif element[0] == "codebox": codeboxes_tuples.append(self.get_codebox_db_tuple(element[1], node_id))
                if codeboxes_tuples:
                    has_codebox = 1
                    db.executemany('INSERT INTO codebox VALUES(?,?,?,?,?,?,?,?,?,?)', codeboxes_tuples)
                if tables_tuples:
                    has_table = 1
                    db.executemany('INSERT INTO grid VALUES(?,?,?,?,?,?)', tables_tuples)
                if images_tuples:
                    has_image = 1
                    db.executemany('INSERT INTO image VALUES(?,?,?,?,?)', images_tuples)
                # retrieve xml text
                txt = self.dom.toxml()
            else:
                # not richtext
                has_codebox = 0
                has_table = 0
                has_image = 0
                txt = start_iter.get_text(end_iter)
        if write_dict['prop'] and write_dict['buff']:
            if write_dict['upd']:
                db.execute('REMOVE FROM node WHERE node_id=?', node_id)
            node_tuple = (node_id, name, txt, syntax, tags,
                          is_ro, is_richtxt, has_codebox, has_table, has_image,
                          has_children, level)
            db.execute('INSERT INTO node VALUES(?,?,?,?,?,?,?,?,?,?,?,?)', node_tuple)
        elif write_dict['buff']:
            db.execute('UPDATE node SET txt=?, syntax=?, is_richtxt=?, has_codebox=?, has_table=?, has_image=? WHERE node_id=?', (txt, syntax, is_richtxt, has_codebox, has_table, has_image, node_id))
        elif write_dict['prop']:
            db.execute('UPDATE node SET name=?, tags=?, is_ro=? WHERE node_id=?', (name, tags, is_ro, node_id))
        if write_dict['hier']:
            if write_dict['upd']:
                db.execute('REMOVE FROM children WHERE node_id=?', node_id)
            db.execute('INSERT INTO children VALUES(?,?,?)', (node_id, node_father_id, sequence))
        if not write_dict['child']: return
        # let's take care about the children
        if has_children:
            child_sequence = 0
            while child_tree_iter != None:
                child_sequence += 1
                self.write_db_node(db, child_tree_iter, level+1, child_sequence, node_id, write_dict)
                child_tree_iter = self.dad.treestore.iter_next(child_tree_iter)
    
    def write_db_full(self, db):
        """Write the whole DB"""
        tree_iter = self.dad.treestore.get_iter_first()
        sequence = 0
        write_dict = {'upd': False, 'prop': True, 'buff': True, 'hier': True, 'child': True}
        while tree_iter != None:
            sequence += 1
            self.write_db_node(db, tree_iter, 1, sequence, 0, write_dict)
            tree_iter = self.dad.treestore.iter_next(tree_iter)
        self.write_db_bookmarks(db)
    
    def add_node_codebox(self, codebox_row, text_buffer):
        """Add Codebox to Text Buffer"""
        iter_insert = text_buffer.get_iter_at_offset(codebox_row['offset'])
        codebox_dict = {
           'frame_width': codebox_row['width'],
           'frame_height': codebox_row['height'],
           'width_in_pixels': bool(codebox_row['is_width_pix']),
           'syntax_highlighting': codebox_row['syntax'],
           'highlight_brackets': bool(codebox_row['do_highl_bra']),
           'show_line_numbers': bool(codebox_row['do_show_linenum']),
           'fill_text': codebox_row['txt']
        }
        self.dad.curr_buffer = text_buffer # the codebox_insert method will need this
        self.dad.codeboxes_handler.codebox_insert(iter_insert, codebox_dict, codebox_row['justification'])
    
    def add_node_table(self, table_row, text_buffer):
        """Add Table to Text Buffer"""
        iter_insert = text_buffer.get_iter_at_offset(table_row['offset'])
        table_dict = {
            'matrix': [],
            'col_min': table_row['col_min'],
            'col_max': table_row["col_max"]
        }
        try: dom = xml.dom.minidom.parseString(table_row['txt'])
        except:
            print "** failed to parse **"
            print table_row['txt']
            return
        dom_node = dom.firstChild
        if not dom_node or dom_node.nodeName != "node":
            print "** table name != 'table' **"
            print table_row['txt']
            return
        child_dom_iter = dom_node.firstChild
        while child_dom_iter != None:
            if child_dom_iter.nodeName == "row":
                table_dict['matrix'].append([])
                nephew_dom_iter = child_dom_iter.firstChild
                while nephew_dom_iter != None:
                    if nephew_dom_iter.nodeName == "cell":
                        if nephew_dom_iter.firstChild != None:
                            table_dict['matrix'][-1].append(nephew_dom_iter.firstChild.data)
                        else: table_dict['matrix'][-1].append("")
                    nephew_dom_iter = nephew_dom_iter.nextSibling
            child_dom_iter = child_dom_iter.nextSibling
        self.dad.curr_buffer = text_buffer # the table_insert method will need this
        self.dad.tables_handler.table_insert(iter_insert, table_dict, table_row['justification'])
    
    def add_node_image(self, image_row, text_buffer):
        """Add Image to Text Buffer"""
        iter_insert = text_buffer.get_iter_at_offset(image_row['offset'])
        if image_row['anchor']:
            pixbuf = gtk.gdk.pixbuf_new_from_file(cons.ANCHOR_CHAR)
            pixbuf.anchor = image_row['anchor']
        else: pixbuf = machines.get_pixbuf_from_png_blob_buffer(image_row['png'])
        self.dad.curr_buffer = curr_buffer # the apply_tag method will need this
        if pixbuf: self.dad.image_insert(iter_insert, pixbuf, image_row['justification'])
    
    def read_db_node_content(self, tree_iter, db):
        """Read a node content from DB"""
        syntax_highlighting = self.dad.treestore[tree_iter][4]
        node_id = self.dad.treestore[tree_iter][3]
        curr_buffer = self.dad.buffer_create(syntax_highlighting)
        self.dad.treestore[tree_iter][3] = curr_buffer
        node_row = db.execute('SELECT txt, has_codebox, has_table, has_image FROM node WHERE node_id=?', node_id).fetchone()
        if syntax_highlighting != cons.CUSTOM_COLORS_ID:
            curr_buffer.begin_not_undoable_action()
            curr_buffer.set_text(node_row['txt'])
            curr_buffer.end_not_undoable_action()
        else:
            # first we go for the rich text
            try: dom = xml.dom.minidom.parseString(node_row['txt'])
            except:
                print "** failed to parse **"
                print node_row['txt']
                return
            dom_node = dom.firstChild
            if not dom_node or dom_node.nodeName != "node":
                print "** node name != 'node' **"
                print node_row['txt']
                return
            child_dom_iter = dom_node.firstChild
            while child_dom_iter != None:
                if child_dom_iter.nodeName == "rich_text":
                    self.dad.xml_handler.rich_text_deserialize(curr_buffer, child_dom_iter)
                child_dom_iter = child_dom_iter.nextSibling
            # then we go for the objects
            objects_index_list = []
            if node_row['has_codebox']:
                codeboxes_rows = db.execute('SELECT * FROM codebox WHERE node_id=? ORDER BY offset ASC', node_id).fetchall()
                for i, c_row in enumerate(codeboxes_rows):
                    objects_index_list.append(['c', i, c_row['offset']])
            if node_row['has_table']:
                tables_rows = db.execute('SELECT * FROM grid WHERE node_id=? ORDER BY offset ASC', node_id).fetchall()
                for i, t_row in enumerate(tables_rows):
                    new_obj_idx_elem = ['t', i, t_row['offset']]
                    for j, obj_idx in enumerate(objects_index_list):
                        if new_obj_idx_elem[2] < obj_idx[2]:
                            objects_index_list.insert(j, new_obj_idx_elem)
                            break
                    else: objects_index_list.append(new_obj_idx_elem)
            if node_row['has_image']:
                images_rows = db.execute('SELECT * FROM image WHERE node_id=? ORDER BY offset ASC', node_id).fetchall()
                for i, i_row in enumerate(images_rows):
                    new_obj_idx_elem = ['i', i, i_row['offset']]
                    for j, obj_idx in enumerate(objects_index_list):
                        if new_obj_idx_elem[2] < obj_idx[2]:
                            objects_index_list.insert(j, new_obj_idx_elem)
                            break
                    else: objects_index_list.append(new_obj_idx_elem)
            for obj_idx in objects_index_list:
                if obj_idx[0] == 'c': self.add_node_codebox(codeboxes_rows[obj_idx[1]], curr_buffer)
                elif obj_idx[0] == 't': self.add_node_table(tables_rows[obj_idx[1]], curr_buffer)
                else: self.add_node_image(images_rows[obj_idx[1]], curr_buffer)
        curr_buffer.set_modified(False)
    
    def read_db_node_n_children(self, node_row, tree_father, discard_ids):
        """Read a node and his children from DB"""
        if not discard_ids:
            unique_id = node_row['node_id']
            self.dad.node_id_add(unique_id)
        else: unique_id = self.dad.node_id_get()
        node_tags = node_row['tags']
        readonly = node_row['is_ro']
        syntax_highlighting = node_row['syntax']
        if syntax_highlighting != cons.CUSTOM_COLORS_ID and syntax_highlighting not in self.dad.available_languages:
            syntax_highlighting = syntax_highlighting.lower().replace("C++", "cpp")
            if syntax_highlighting not in self.dad.available_languages:
                syntax_highlighting = cons.CUSTOM_COLORS_ID
        if tree_father == None: node_level = 0
        else: node_level = self.dad.treestore[tree_father][5] + 1
        cherry = self.dad.get_node_icon(node_level, syntax_highlighting)
        #print unique_id
        # insert the node containing the buffer into the tree
        tree_iter = self.dad.treestore.append(tree_father, [cherry,
                                                            node_row['name'],
                                                            None, # no buffer for now
                                                            unique_id,
                                                            syntax_highlighting,
                                                            node_level,
                                                            node_tags,
                                                            readonly])
        self.dad.nodes_names_dict[self.dad.treestore[tree_iter][3]] = self.dad.treestore[tree_iter][1]
        # loop for child nodes
        children_rows = db.execute('SELECT * FROM children WHERE father_id=? ORDER BY sequence ASC', unique_id).fetchall()
        for child_row in children_rows:
            child_node_row = db.execute('SELECT node_id, name, syntax, tags, has_children, level FROM node WHERE node_id=?', child_row['node_id']).fetchone()
            if child_node_row: read_db_node_n_children(self, child_node_row, tree_iter, discard_ids)
    
    def read_db_full(self, db, discard_ids, tree_father=None, reset_nodes_names=True):
        """Read the whole DB"""
        self.dad.bookmarks = []
        if reset_nodes_names:
            self.dad.nodes_names_dict = {}
            bookmarks_menu = self.dad.ui.get_widget("/MenuBar/BookmarksMenu").get_submenu()
            for menu_item in self.dad.bookmarks_menu_items:
                bookmarks_menu.remove(menu_item)
            self.dad.bookmarks_menu_items = []
        db.row_factory = sqlite3.Row
        # tree nodes
        children_rows = db.execute('SELECT * FROM children WHERE father_id=0 ORDER BY sequence ASC').fetchall()
        for child_row in children_rows:
            child_node_row = db.execute('SELECT node_id, name, syntax, tags, has_children, level FROM node WHERE node_id=?', child_row['node_id']).fetchone()
            if child_node_row: read_db_node_n_children(self, child_node_row, None, discard_ids)
        # bookmarks
        bookmarks_rows = db.execute('SELECT * FROM bookmark ORDER BY sequence ASC').fetchall()
        for bookmark_row in bookmarks_rows: self.dad.bookmarks.append(str(bookmark_row['node_id']))
