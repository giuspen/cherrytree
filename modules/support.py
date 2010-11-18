# -*- coding: UTF-8 -*-
#
#       support.py
#       
#       Copyright 2009-2010 Giuseppe Penone <giuspen@gmail.com>
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

import gtk, os


def dialog_file_save_as(filename=None, filter_pattern=None, filter_name=None, curr_folder=None, parent=None):
   """The Save file as dialog, Returns the retrieved filepath or None"""
   chooser = gtk.FileChooserDialog(title=_("Save File as"),
                                   action=gtk.FILE_CHOOSER_ACTION_SAVE,
                                   buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_SAVE, gtk.RESPONSE_OK) )
   if parent != None: chooser.set_transient_for(parent)
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
   if chooser.run() == gtk.RESPONSE_OK:
      filepath = chooser.get_filename()
      chooser.destroy()
      return filepath
   else:
      chooser.destroy()
      return None

def dialog_file_select(filter_pattern=None, filter_name=None, curr_folder=None, parent=None):
   """The Select file dialog, Returns the retrieved filepath or None"""
   chooser = gtk.FileChooserDialog(title = _("Select File"),
                                   action=gtk.FILE_CHOOSER_ACTION_OPEN,
                                   buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK) )
   if parent != None: chooser.set_transient_for(parent)
   if curr_folder == None or os.path.isdir(curr_folder) == False:
      chooser.set_current_folder(os.path.expanduser('~'))
   else:
      chooser.set_current_folder(curr_folder)
   if filter_pattern != None:
      filter = gtk.FileFilter()
      filter.set_name(filter_name)
      filter.add_pattern(filter_pattern)
      chooser.add_filter(filter)
   if chooser.run() == gtk.RESPONSE_OK:
      filepath = chooser.get_filename()
      chooser.destroy()
      return filepath
   else:
      chooser.destroy()
      return None
   
def dialog_folder_select(curr_folder=None, parent=None):
   """The Select folder dialog, returns the retrieved folderpath or None"""
   chooser = gtk.FileChooserDialog(title = _("Select Folder"),
                                   action=gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                   buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK) )
   if parent != None: chooser.set_transient_for(parent)
   if curr_folder == None or os.path.isdir(curr_folder) == False:
      chooser.set_current_folder(os.path.expanduser('~'))
   else:
      chooser.set_current_folder(curr_folder)
   if chooser.run() == gtk.RESPONSE_OK:
      folderpath = chooser.get_filename()
      chooser.destroy()
      return folderpath
   else:
      chooser.destroy()
      return None
   
def dialog_question(message, parent=None):
   """The Question dialog, returns True if the user presses OK"""
   dialog = gtk.MessageDialog(flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                              type=gtk.MESSAGE_QUESTION,
                              buttons=gtk.BUTTONS_OK_CANCEL,
                              message_format=message)
   if parent != None: dialog.set_transient_for(parent)
   dialog.set_title(_("Question"))
   if dialog.run() == gtk.RESPONSE_OK:
      dialog.destroy()
      return True
   else:
      dialog.destroy()
      return False

def dialog_info(message, parent=None):
   """The Info dialog"""
   dialog = gtk.MessageDialog(flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                              type=gtk.MESSAGE_INFO,
                              buttons=gtk.BUTTONS_OK,
                              message_format=message)
   if parent != None: dialog.set_transient_for(parent)
   dialog.set_title(_("Info"))
   dialog.run()
   dialog.destroy()

def dialog_warning(message, parent=None):
   """The Warning dialog"""
   dialog = gtk.MessageDialog(flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                              type=gtk.MESSAGE_WARNING,
                              buttons=gtk.BUTTONS_OK,
                              message_format=message)
   if parent != None: dialog.set_transient_for(parent)
   dialog.set_title(_("Warning"))
   dialog.run()
   dialog.destroy()
   
def dialog_error(message, parent=None):
   """The Error dialog"""
   dialog = gtk.MessageDialog(flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                              type=gtk.MESSAGE_ERROR,
                              buttons=gtk.BUTTONS_OK,
                              message_format=message)
   if parent != None: dialog.set_transient_for(parent)
   dialog.set_title(_("Error"))
   dialog.run()
   dialog.destroy()
