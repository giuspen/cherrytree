# -*- coding: UTF-8 -*-
#
#       clipboard.py
#       
#       Copyright 2010 Giuseppe Penone <giuspen@gmail.com>
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
import cons


TARGET_CTD_PLAIN_TEXT = 'UTF8_STRING'
TARGET_CTD_RICH_TEXT = 'CTD_RICH'
TARGET_CTD_IMAGE = 'image/png'
TARGET_CTD_TABLE = 'CTD_TABLE'
TARGET_CTD_CODEBOX = 'CTD_CODEBOX'
TARGETS_PLAIN_TEXT = ("UTF8_STRING", "COMPOUND_TEXT", "STRING", "TEXT")
TARGETS_IMAGES = ('image/png', 'image/jpeg', 'image/bmp', 'image/tiff', 'image/x-MS-bmp', 'image/x-bmp')


class ClipboardHandler:
   """Handler of Clipboard"""
   
   def __init__(self, dad):
      """Clipboard Handler boot"""
      self.dad = dad
      self.clipboard = gtk.clipboard_get()
      
   def copy(self, sourceview):
      """Copy to Clipboard"""
      sourceview.stop_emission("copy-clipboard")
      if not self.dad.curr_buffer.get_has_selection(): return
      self.selection_to_clipboard(self.dad.curr_buffer, sourceview)
      
   def cut(self, sourceview):
      """Cut to Clipboard"""
      sourceview.stop_emission("cut-clipboard")
      if not self.dad.curr_buffer.get_has_selection(): return
      self.selection_to_clipboard(self.dad.curr_buffer, sourceview)
      self.dad.curr_buffer.delete_selection(True, sourceview.get_editable())
      self.dad.sourceview.grab_focus()
      
   def selection_to_clipboard(self, text_buffer, sourceview):
      """Write the Selected Content to the Clipboard"""
      iter_sel_start, iter_sel_end = text_buffer.get_selection_bounds()
      num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset()
      if num_chars == 1:
         anchor = iter_sel_start.get_child_anchor()
         if anchor:
            anchor_dir = dir(anchor)
            if "pixbuf" in anchor_dir:
               self.clipboard.set_with_data([(TARGET_CTD_IMAGE, 0, 0)],
                                            self.get_func,
                                            self.clear_func,
                                            ("p", anchor.pixbuf))
               return
            elif "liststore" in anchor_dir:
               print "got table"
               return
            elif "sourcebuffer" in anchor_dir:
               print "got codebox"
               return
      plain_text = text_buffer.get_text(iter_sel_start, iter_sel_end)
      self.clipboard.set_with_data([(TARGET_CTD_PLAIN_TEXT, 0, 0)],
                                    self.get_func,
                                    self.clear_func,
                                    ("t", plain_text))
      
   def get_func(self, clipboard, selectiondata, info, data):
      """Connected with clipboard.set_with_data"""
      if data[0] == "t": selectiondata.set(TARGET_CTD_PLAIN_TEXT, 8, data[1])
      elif data[0] == "p": selectiondata.set_pixbuf(data[1])
      
   def clear_func(self, clipboard, data):
      """Connected with clipboard.set_with_data"""
      # this is to eventually free memory allocated when filling the clipboard
      pass
      
   def paste(self, sourceview):
      """Paste from Clipboard"""
      sourceview.stop_emission("paste-clipboard")
      targets = self.clipboard.wait_for_targets()
      if not targets: return
      for target in TARGETS_IMAGES:
         if target in targets:
            self.clipboard.request_contents(target, self.to_image)
            return
      for target in TARGETS_PLAIN_TEXT:
         if target in targets:
            self.clipboard.request_contents(target, self.to_plain_text)
            break
   
   def to_plain_text(self, clipboard, selectiondata, data):
      """From Clipboard to Plain Text"""
      self.dad.curr_buffer.insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()), selectiondata.get_text())
   
   def to_image(self, clipboard, selectiondata, data):
      """From Clipboard to Image"""
      if self.dad.syntax_highlighting != cons.CUSTOM_COLORS_ID: return
      pixbuf = selectiondata.get_pixbuf()
      self.dad.image_edit_dialog(pixbuf, self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()))
