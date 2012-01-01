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
import os, sqlite3, xml.dom.minidom
import cons, support


class CTDBHandler:
    """Handler of the CherryTree DataBase"""

    def __init__(self, dad):
        """CherryTree DataBase boot"""
        self.dad = dad
    
    def new_db(self, dbpath):
        """Create a new DataBase"""
        db = sqlite3.connect(dbpath)
        db.execute(cons.TABLE_NODE_CREATE)
        db.execute(cons.TABLE_CODEBOX_CREATE)
        db.execute(cons.TABLE_TABLE_CREATE)
        db.execute(cons.TABLE_IMAGE_CREATE)
        db.execute(cons.TABLE_CHILDREN_CREATE)
        db.execute(cons.TABLE_BOOKMARK_CREATE)
        db.commit()
        self.write_db_full(db)
        db.close()
    
    def write_db_bookmarks(self, db):
        """Write all the bookmarks in DB"""
        if not self.dad.bookmarks: return
        for bookmark_str in self.dad.bookmarks:
            bookmark_tuple = tuple(int(bookmark_str))
            db.execute('INSERT INTO bookmark VALUES(?)', bookmark_tuple)
        db.commit()
    
    def write_db_node(self, db, tree_iter, level):
        """Write a node in DB"""
        node_id = self.dad.treestore[tree_iter][3]
        name = self.dad.treestore[tree_iter][1]
        syntax = self.dad.treestore[tree_iter][4]
        tags = self.dad.treestore[tree_iter][6]
        is_ro = 1 if self.dad.treestore[tree_iter][7] else 0
        is_richtxt = 1 if syntax == cons.CUSTOM_COLORS_ID else 0
        has_codebox = 0
        has_table = 0
        has_image = 0
        has_children = 0
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
            
            # retrieve xml text
            txt = self.dom.toxml()
        else: txt = start_iter.get_text(end_iter)
        node_tuple = (node_id, name, txt, syntax, tags,
                      is_ro, is_richtxt, has_codebox, has_table, has_image,
                      has_children, level)
        db.execute('INSERT INTO node VALUES(?,?,?,?,?,?,?,?,?,?,?,?)', bookmark_tuple)
        db.commit()
    
    def write_db_full(self, db):
        """Write the whole DB"""
        tree_iter = self.dad.treestore.get_iter_first()
        while tree_iter != None:
            self.write_db_node(db, tree_iter, 0)
            tree_iter = self.dad.treestore.iter_next(tree_iter)
        self.write_db_bookmarks(db)
