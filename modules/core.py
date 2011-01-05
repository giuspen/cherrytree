# -*- coding: UTF-8 -*-
#
#       core.py
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

import gtk, pango, gtksourceview2, gobject
import sys, os, re, subprocess, webbrowser, base64, cgi, urllib2, shutil
import cons, support, config, machines, clipboard, imports, exports, printing, tablez, lists, findreplace, codeboxes


class GladeWidgetsWrapper:
   """Handles the retrieval of glade widgets"""
   
   def __init__(self, glade_file_path, gui_instance):
      try:
         self.glade_widgets = gtk.Builder()
         self.glade_widgets.set_translation_domain(cons.APP_NAME)
         self.glade_widgets.add_from_file(glade_file_path)
         self.glade_widgets.connect_signals(gui_instance)
      except: print "Failed to load the glade file"
      
   def __getitem__(self, key):
      """Gives us the ability to do: wrapper['widget_name'].action()"""
      return self.glade_widgets.get_object(key)
      
   def __getattr__(self, attr):
      """Gives us the ability to do: wrapper.widget_name.action()"""
      new_widget = self.glade_widgets.get_object(attr)
      if new_widget is None: raise AttributeError, 'Widget %r not found' % attr
      setattr(self, attr, new_widget)
      return new_widget


class CherryTree:
   """Application's GUI"""
   
   def __init__(self, lang_str, first_instance, open_with_file):
      """GUI Startup"""
      # instantiate external handlers
      self.clipboard_handler = clipboard.ClipboardHandler(self)
      self.lists_handler = lists.ListsHandler(self)
      self.tables_handler = tablez.TablesHandler(self)
      self.codeboxes_handler = codeboxes.CodeBoxesHandler(self)
      self.state_machine = machines.StateMachine(self)
      self.xml_handler = machines.XMLHandler(self)
      self.html_handler = exports.Export2Html(self)
      self.find_handler = findreplace.FindReplace(self)
      self.print_handler = printing.PrintHandler()
      # icon factory
      factory = gtk.IconFactory()
      for stock_name in cons.STOCKS_N_FILES:
         pixbuf = gtk.gdk.pixbuf_new_from_file(cons.GLADE_PATH + cons.STOCKS_N_FILES[stock_name])
         iconset = gtk.IconSet(pixbuf)
         factory.add(stock_name, iconset)
      factory.add_default()
      # glade
      self.glade = GladeWidgetsWrapper(cons.GLADE_PATH + 'cherrytree.glade', self) # glade widgets access
      self.window = self.glade.window
      vbox_main = gtk.VBox()
      self.window.add(vbox_main)
      if not first_instance:
         support.dialog_error(_("Another Instance of CherryTree is Already Running"), self.window)
         sys.exit(1)
      self.country_lang = lang_str
      config.config_file_load(self)
      # ui manager
      actions = gtk.ActionGroup("Actions")
      actions.add_actions(cons.get_entries(self))
      self.ui = gtk.UIManager()
      self.ui.insert_action_group(actions, 0)
      self.window.add_accel_group(self.ui.get_accel_group())
      self.ui.add_ui_from_string(cons.UI_INFO)
      # menubar add
      vbox_main.pack_start(self.ui.get_widget("/MenuBar"), False, False)
      # toolbar add
      vbox_main.pack_start(self.ui.get_widget("/ToolBar"), False, False)
      # hpaned add
      self.hpaned = gtk.HPaned()
      self.scrolledwindow_tree = gtk.ScrolledWindow()
      self.scrolledwindow_tree.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
      self.scrolledwindow_text = gtk.ScrolledWindow()
      self.scrolledwindow_text.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
      self.vbox_text = gtk.VBox()
      self.header_node_name_label = gtk.Label()
      self.vbox_text.pack_start(self.header_node_name_label, False, False)
      self.vbox_text.pack_start(self.scrolledwindow_text)
      if self.tree_right_side:
         self.hpaned.add1(self.vbox_text)
         self.hpaned.add2(self.scrolledwindow_tree)
      else:
         self.hpaned.add1(self.scrolledwindow_tree)
         self.hpaned.add2(self.vbox_text)
      vbox_main.pack_start(self.hpaned)
      # statusbar add
      self.statusbar = gtk.Statusbar()
      self.statusbar_context_id = self.statusbar.get_context_id('')
      vbox_main.pack_start(self.statusbar, False, False)
      # ROW: 0-icon_stock_id, 1-name, 2-buffer, 3-unique_id, 4-syntax_highlighting, 5-level, 6-tags
      self.treestore = gtk.TreeStore(str, str, gobject.TYPE_PYOBJECT, long, str, int, str)
      self.treeview = gtk.TreeView(self.treestore)
      self.treeview.set_headers_visible(False)
      self.renderer_pixbuf = gtk.CellRendererPixbuf()
      self.renderer_text = gtk.CellRendererText()
      self.renderer_text.set_property('wrap-mode', pango.WRAP_WORD_CHAR)
      self.renderer_text.connect('edited', self.tree_cell_edited)
      self.column = gtk.TreeViewColumn()
      self.column.pack_start(self.renderer_pixbuf, False)
      self.column.pack_start(self.renderer_text, True)
      self.column.set_attributes(self.renderer_pixbuf, stock_id=0)
      self.column.set_attributes(self.renderer_text, text=1)
      self.treeview.append_column(self.column)
      self.treeviewselection = self.treeview.get_selection()
      self.treeview.connect('cursor-changed', self.on_node_changed)
      self.treeview.connect('button-press-event', self.on_mouse_button_clicked_tree)
      self.treeview.connect('key_press_event', self.on_key_press_cherrytree)
      self.scrolledwindow_tree.add(self.treeview)
      self.window.connect('window-state-event', self.on_window_state_event)
      self.window.connect("size-allocate", self.on_window_n_tree_size_allocate_event)
      self.scrolledwindow_tree.connect("size-allocate", self.on_window_n_tree_size_allocate_event)
      self.glade.inputdialog.connect('key_press_event', self.on_key_press_input_dialog)
      self.glade.anchorhandledialog.connect('key_press_event', self.on_key_press_anchorhandledialog)
      self.glade.choosenodedialog.connect('key_press_event', self.on_key_press_choosenodedialog)
      self.glade.tablehandledialog.connect('key_press_event', self.tables_handler.on_key_press_tablehandledialog)
      self.glade.codeboxhandledialog.connect('key_press_event', self.codeboxes_handler.on_key_press_codeboxhandledialog)
      self.sourceview = gtksourceview2.View()
      self.sourceview.set_sensitive(False)
      self.sourceview.connect('populate-popup', self.on_sourceview_populate_popup)
      self.sourceview.connect("motion-notify-event", self.on_sourceview_motion_notify_event)
      self.sourceview.connect("event-after", self.on_sourceview_event_after)
      self.sourceview.connect("visibility-notify-event", self.on_sourceview_visibility_notify_event)
      self.sourceview.connect("copy-clipboard", self.clipboard_handler.copy)
      self.sourceview.connect("cut-clipboard", self.clipboard_handler.cut)
      self.sourceview.connect("paste-clipboard", self.clipboard_handler.paste)
      self.sourceview.set_left_margin(7)
      self.sourceview.set_right_margin(7)
      self.hovering_over_link = False
      self.tag_table = gtk.TextTagTable()
      self.scrolledwindow_text.add(self.sourceview)
      self.password = None
      self.curr_tree_iter = None
      self.curr_window_n_tree_width = None
      self.curr_buffer = None
      self.nodes_cursor_pos = {}
      self.latest_tag = ["", ""] # [latest tag property, latest tag value]
      self.file_update = False
      self.autosave_timer_id = None
      self.node_id_counter = long(0)
      self.glade.aboutdialog.set_version(cons.VERSION)
      self.window.show_all() # this before the config_file_apply that could hide something
      config.config_file_apply(self)
      self.combobox_country_lang_init()
      self.combobox_prog_lang_init()
      if self.systray:
         self.status_icon_enable()
         if self.start_on_systray: self.window.hide()
      else: self.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property('visible', False)
      self.file_startup_load(open_with_file)
      if self.check_version: self.check_for_newer_version()
      else: self.statusbar.push(self.statusbar_context_id, _("Version %s") % cons.VERSION)
      
   def check_for_newer_version(self, *args):
      """Check for a Newer Version"""
      self.statusbar.push(self.statusbar_context_id, _("Checking for Newer Version..."))
      while gtk.events_pending(): gtk.main_iteration()
      try:
         fd = urllib2.urlopen(cons.NEWER_VERSION_URL, timeout=3)
         latest_version = fd.read().replace("\n", "")
         if latest_version != cons.VERSION:
            support.dialog_info(_("A Newer Version Is Available!") + " (%s)" % latest_version, self.window)
      except: pass
      self.statusbar.push(self.statusbar_context_id, _("Version %s") % cons.VERSION)
      
   def get_node_icon(self, node_level, node_code):
      """Returns the Stock Id given the Node Level"""
      if self.nodes_icons == "c":
         if node_code == cons.CUSTOM_COLORS_ID:
            if node_level in cons.NODES_ICONS: return cons.NODES_ICONS[node_level]
            else: return cons.NODES_ICONS[6]
         else:
            if node_code in cons.CODE_ICONS: return cons.CODE_ICONS[node_code]
            else: return "Gray Cherry"
      elif self.nodes_icons == "b": return "Node Bullet"
      else: return "Node NoIcon"
      
   def text_selection_change_case(self, change_type):
      """Change the Case of the Selected Text/the Underlying Word"""
      if not self.curr_buffer.get_has_selection() and not self.apply_tag_try_automatic_bounds():
         support.dialog_warning(_("No Text is Selected"), self.window)
         return
      iter_start, iter_end = self.curr_buffer.get_selection_bounds()
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
         text_to_change_case = self.curr_buffer.get_slice(iter_start, iter_end)
         if change_type == "l": text_to_change_case = text_to_change_case.lower()
         elif change_type == "u": text_to_change_case = text_to_change_case.upper()
         elif change_type == "t": text_to_change_case = text_to_change_case.swapcase()
      else:
         rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(self.curr_buffer,
                                                                                     iter_start,
                                                                                     iter_end,
                                                                                     change_case=change_type)
      start_offset = iter_start.get_offset()
      end_offset = iter_end.get_offset()
      self.curr_buffer.delete(iter_start, iter_end)
      iter_insert = self.curr_buffer.get_iter_at_offset(start_offset)
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
         self.curr_buffer.insert(iter_insert, text_to_change_case)
      else:
         self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_insert)
         self.clipboard_handler.from_xml_string_to_buffer(rich_text)
      self.curr_buffer.select_range(self.curr_buffer.get_iter_at_offset(start_offset),
                                    self.curr_buffer.get_iter_at_offset(end_offset))
      
   def text_selection_toggle_case(self, *args):
      """Toggles the Case of the Selected Text/the Underlying Word"""
      self.text_selection_change_case("t")
      
   def text_selection_upper_case(self, *args):
      """Uppers the Case of the Selected Text/the Underlying Word"""
      self.text_selection_change_case("u")
   
   def text_selection_lower_case(self, *args):
      """Lowers the Case of the Selected Text/the Underlying Word"""
      self.text_selection_change_case("l")
      
   def text_row_selection_duplicate(self, *args):
      """Duplicates the Whole Row or a Selection"""
      if self.curr_buffer.get_has_selection():
         iter_start, iter_end = self.curr_buffer.get_selection_bounds() # there's a selection
         text_to_duplicate = self.curr_buffer.get_slice(iter_start, iter_end)
         if cons.CHAR_NEWLINE in text_to_duplicate: text_to_duplicate = cons.CHAR_NEWLINE + text_to_duplicate
         self.curr_buffer.insert(iter_end, text_to_duplicate)
      else:
         iter_start, iter_end = self.lists_handler.get_paragraph_iters()
         if iter_start == None:
            iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
            self.curr_buffer.insert(iter_start, cons.CHAR_NEWLINE)
         else:
            text_to_duplicate = self.curr_buffer.get_slice(iter_start, iter_end)
            self.curr_buffer.insert(iter_end, cons.CHAR_NEWLINE + text_to_duplicate)
      self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      
   def text_row_delete(self, *args):
      """Deletes the Whole Row"""
      iter_start, iter_end = self.lists_handler.get_paragraph_iters()
      if iter_start == None:
         iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
         iter_end = iter_start.copy()
      if not iter_end.forward_char() and not iter_start.backward_char(): return
      self.curr_buffer.delete(iter_start, iter_end)
      self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      
   def on_key_press_cherrytree(self, widget, event):
      """Catches ChooseNode Dialog key presses"""
      keyname = gtk.gdk.keyval_name(event.keyval)
      if keyname == "Return": self.node_edit()
      elif keyname == "Menu":
         self.ui.get_widget("/TreeMenu").popup(None, None, None, 0, event.time)
         widget.stop_emission("key_press_event")
   
   def on_key_press_choosenodedialog(self, widget, event):
      """Catches ChooseNode Dialog key presses"""
      keyname = gtk.gdk.keyval_name(event.keyval)
      if keyname == "Return": self.glade.choosenodedialog_button_ok.clicked()
   
   def on_key_press_anchorhandledialog(self, widget, event):
      """Catches AnchorHandle Dialog key presses"""
      keyname = gtk.gdk.keyval_name(event.keyval)
      if keyname == "Return": self.glade.anchorhandledialog_button_ok.clicked()
      
   def on_key_press_input_dialog(self, widget, event):
      """Catches Input Dialog key presses"""
      keyname = gtk.gdk.keyval_name(event.keyval)
      if keyname == "Return": self.glade.input_dialog_ok_button.clicked()
      
   def nodes_add_from_cherrytree_file(self, action):
      """Appends Nodes at the Bottom of the Current Ones, Importing from a CherryTree File"""
      filepath = support.dialog_file_select(filter_pattern="*.ctd",
                                            filter_name=_("CherryTree Document"),
                                            curr_folder=self.file_dir,
                                            parent=self.window)
      if filepath == None: return
      try:
         file_descriptor = open(filepath, 'r')
         cherrytree_string = file_descriptor.read()
         file_descriptor.close()
      except:
         support.dialog_error("Error opening the file %s" % filepath, self.window)
         return
      self.nodes_add_from_string(cherrytree_string)
      
   def nodes_add_from_notecase_file(self, action):
      """Add Nodes Parsing a NoteCase File"""
      filepath = support.dialog_file_select(filter_pattern="*.ncd",
                                            filter_name=_("NoteCase Document"),
                                            curr_folder=self.file_dir,
                                            parent=self.window)
      if filepath == None: return
      try:
         file_descriptor = open(filepath, 'r')
         notecase_string = file_descriptor.read()
         file_descriptor.close()
      except:
         support.dialog_error("Error opening the file %s" % filepath, self.window)
         return
      notecase = imports.NotecaseHandler()
      cherrytree_string = notecase.get_cherrytree_xml(notecase_string)
      self.nodes_add_from_string(cherrytree_string)
      
   def nodes_add_from_tuxcards_file(self, action):
      """Add Nodes Parsing a TuxCards File"""
      filepath = support.dialog_file_select(curr_folder=self.file_dir,
                                            parent=self.window)
      if filepath == None: return
      try:
         file_descriptor = open(filepath, 'r')
         tuxcards_string = file_descriptor.read()
         file_descriptor.close()
      except:
         support.dialog_error("Error opening the file %s" % filepath, self.window)
         return
      tuxcards = imports.TuxCardsHandler()
      cherrytree_string = tuxcards.get_cherrytree_xml(tuxcards_string)
      self.nodes_add_from_string(cherrytree_string)
      
   def nodes_add_from_keepnote_folder(self, action):
      """Add Nodes Parsing a KeepNote Folder"""
      folderpath = support.dialog_folder_select(curr_folder=self.file_dir, parent=self.window)
      if folderpath == None: return
      keepnote = imports.KeepnoteHandler(folderpath)
      cherrytree_string = keepnote.get_cherrytree_xml()
      self.nodes_add_from_string(cherrytree_string)
      
   def nodes_add_from_basket_folder(self, action):
      """Add Nodes Parsing a Basket Folder"""
      folderpath = support.dialog_folder_select(curr_folder=os.path.join(os.path.expanduser('~'), ".kde/share/apps/basket/baskets"),
                                                parent=self.window)
      if folderpath == None: return
      basket = imports.BasketHandler(folderpath)
      if basket.check_basket_structure():
         cherrytree_string = basket.get_cherrytree_xml()
         self.nodes_add_from_string(cherrytree_string)
      
   def nodes_add_from_treepad_file(self, action):
      """Add Nodes Parsing a Treepad File"""
      filepath = support.dialog_file_select(filter_pattern="*.hjt",
                                            filter_name=_("Treepad Document"),
                                            curr_folder=self.file_dir,
                                            parent=self.window)
      if filepath == None: return
      try:
         file_descriptor = open(filepath, 'r')
         treepad = imports.TreepadHandler()
         cherrytree_string = treepad.get_cherrytree_xml(file_descriptor)
         file_descriptor.close()
      except:
         support.dialog_error("Error opening the file %s" % filepath, self.window)
         raise
         return
      self.nodes_add_from_string(cherrytree_string)
      
   def nodes_add_from_string(self, cherrytree_string):
      """Adds Nodes to the Tree Parsing a CherryTree XML String"""
      cherrytree_string = re.sub(cons.BAD_CHARS, "", cherrytree_string)
      self.user_active = False
      file_loaded = False
      former_node = self.curr_tree_iter # we'll restore after the import
      tree_father = None # init the value of the imported nodes father
      if self.curr_tree_iter:
         self.nodes_cursor_pos[self.treestore[self.curr_tree_iter][3]] = self.curr_buffer.get_property('cursor-position')
         if self.curr_buffer.get_modified() == True:
            self.file_update = True
            self.curr_buffer.set_modified(False)
            self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
         append_location_dialog = gtk.Dialog(title=_("Who is the Father?"),
                                             parent=self.window,
                                             flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                                             buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                                                      gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
         radiobutton_root = gtk.RadioButton(label=_("The Tree Root"))
         radiobutton_curr_node = gtk.RadioButton(label=_("The Selected Node"))
         radiobutton_curr_node.set_group(radiobutton_root)
         content_area = append_location_dialog.get_content_area()
         content_area.pack_start(radiobutton_root)
         content_area.pack_start(radiobutton_curr_node)
         content_area.show_all()
         response = append_location_dialog.run()
         if radiobutton_curr_node.get_active(): tree_father = self.curr_tree_iter
         append_location_dialog.destroy()
         if response != gtk.RESPONSE_ACCEPT:
            self.user_active = True
            return
      try:
         # the imported nodes unique_ids must be discarded!
         if self.xml_handler.dom_to_treestore(cherrytree_string, discard_ids=True, tree_father=tree_father):
            if self.expand_tree: self.treeview.expand_all()
            file_loaded = True
      except: pass
      if file_loaded:
         self.update_window_save_needed()
         if not former_node: former_node = self.treestore.get_iter_first()
         if former_node:
            self.curr_tree_iter = former_node
            self.curr_buffer = self.treestore[self.curr_tree_iter][2]
            self.sourceview.set_buffer(None)
            self.sourceview.set_buffer(self.curr_buffer)
            self.syntax_highlighting = self.treestore[self.curr_tree_iter][4]
            self.curr_buffer.connect('modified-changed', self.on_modified_changed)
            if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
               self.curr_buffer.connect('insert-text', self.on_text_insertion)
               self.curr_buffer.connect('delete-range', self.on_text_removal)
               self.sourceview.modify_font(pango.FontDescription(self.text_font))
            else: self.sourceview.modify_font(pango.FontDescription(self.code_font))
            self.sourceview.set_sensitive(True)
            self.header_node_name_label.set_text("<big><b><i>"+cgi.escape(self.treestore[self.curr_tree_iter][1])+"</i></b></big>")
            self.header_node_name_label.set_use_markup(True)
            self.state_machine.node_selected_changed(self.treestore[self.curr_tree_iter][3])
            self.objects_buffer_refresh()
            # try to restore cursor position if in memory
            if self.treestore[former_node][3] in self.nodes_cursor_pos:
               self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(self.nodes_cursor_pos[self.treestore[former_node][3]]))
               self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
      else: support.dialog_error('Error Parsing the CherryTree XML String', self.window)
      self.user_active = True
      
   def nodes_expand_all(self, action):
      """Expand all Tree Nodes"""
      self.treeview.expand_all()
      
   def nodes_collapse_all(self, action):
      """Collapse all Tree Nodes"""
      self.treeview.collapse_all()
      
   def on_sourceview_populate_popup(self, textview, menu):
      """Extend the Default Right-Click Menu"""
      if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         for menuitem in menu.get_children():
            try:
               if menuitem.get_image().get_property("stock") == "gtk-paste":
                  menuitem.set_sensitive(True)
            except: pass
         self.menu_populate_popup(menu, cons.get_popup_menu_entries_text(self))
      else: self.menu_populate_popup(menu, cons.get_popup_menu_entries_code(self))
      
   def menu_populate_popup(self, menu, entries):
      """Populate the given menu with the given entries"""
      curr_submenu = None
      for attributes in entries:
         if attributes[0] == "separator":
            menu_item = gtk.SeparatorMenuItem()
            if curr_submenu: curr_submenu.append(menu_item)
            else: menu.append(menu_item)
         elif attributes[0] == "submenu-start":
            curr_submenu = gtk.Menu()
            menu_item = gtk.ImageMenuItem(attributes[1])
            menu_item.set_image(gtk.image_new_from_stock(attributes[2], gtk.ICON_SIZE_MENU))
            menu_item.set_submenu(curr_submenu)
            menu.append(menu_item)
         elif attributes[0] == "submenu-end":
            curr_submenu = None
            continue
         else:
            menu_item = gtk.ImageMenuItem(attributes[1])
            menu_item.connect('activate', attributes[3])
            menu_item.set_image(gtk.image_new_from_stock(attributes[0], gtk.ICON_SIZE_MENU))
            menu_item.set_tooltip_text(attributes[2])
            if curr_submenu: curr_submenu.append(menu_item)
            else: menu.append(menu_item)
         menu_item.show()
      
   def on_window_state_event(self, window, event):
      """Catch Window's Events"""
      if event.changed_mask & gtk.gdk.WINDOW_STATE_MAXIMIZED:
         if event.new_window_state & gtk.gdk.WINDOW_STATE_MAXIMIZED:
            # the window was maximized
            self.win_is_maximized = True
         else:
            # the window was unmaximized
            self.win_is_maximized = False
      
   def dialog_preferences(self, *args):
      """Opens the Preferences Dialog"""
      self.glade.prefdialog.run()
      self.glade.prefdialog.hide()
      # timer activate/modify handling
      new_autosave_value = int(self.glade.spinbutton_autosave.get_value())
      if self.autosave[1] != new_autosave_value:
         self.autosave[1] = new_autosave_value
         if self.autosave_timer_id != None: self.autosave_timer_stop()
      if self.autosave[0] and self.autosave_timer_id == None: self.autosave_timer_start()
      
   def autosave_timer_start(self):
      """Start Autosave Timer"""
      self.autosave_timer_id = gobject.timeout_add(self.autosave[1]*1000*60, self.autosave_timer_iter)
      
   def autosave_timer_stop(self):
      """Stop Autosave Timer"""
      gobject.source_remove(self.autosave_timer_id)
      self.autosave_timer_id = None
      
   def autosave_timer_iter(self):
      """Iteration of the Autosave"""
      if self.file_update or (self.curr_tree_iter != None and self.curr_buffer.get_modified() == True):
         self.file_save()
      return True # this way we keep the timer alive
      
   def status_icon_enable(self):
      """Creates the Stats Icon"""
      self.status_icon = gtk.StatusIcon()
      self.status_icon.set_from_stock("CherryTree")
      self.status_icon.connect('button-press-event', self.on_mouse_button_clicked_systray)
      self.status_icon.set_tooltip(_("CherryTree Hierarchical Note Taking"))
   
   def on_mouse_button_clicked_systray(self, widget, event):
      """Catches mouse buttons clicks upon the system tray icon"""
      if event.button == 1:
         if self.window.get_property('visible'): self.window.hide()
         else:
            self.window.show()
            self.window.deiconify()
      elif event.button == 3: self.ui.get_widget("/SysTrayMenu").popup(None, None, None, event.button, event.time)
   
   def node_id_get(self):
      """Returns the node_ids, all Different Each Other"""
      self.node_id_counter += 1
      return self.node_id_counter
      
   def node_id_add(self, node_id):
      """Updates the Maximum node_id Value Considering the Received One"""
      if node_id > self.node_id_counter: self.node_id_counter = node_id
   
   def on_fontbutton_text_font_set(self, picker):
      """A New Font For the Text was Chosen"""
      self.text_font = picker.get_font_name()
      if self.treestore[self.curr_tree_iter][4] == cons.CUSTOM_COLORS_ID:
         self.sourceview.modify_font(pango.FontDescription(self.text_font))
      
   def on_fontbutton_code_font_set(self, picker):
      """A New Font For the Text was Chosen"""
      self.code_font = picker.get_font_name()
      if self.treestore[self.curr_tree_iter][4] != cons.CUSTOM_COLORS_ID:
         self.sourceview.modify_font(pango.FontDescription(self.code_font))
      
   def on_fontbutton_tree_font_set(self, picker):
      """A New Font For the Tree was Chosen"""
      self.tree_font = picker.get_font_name()
      self.set_treeview_font()
   
   def set_treeview_font(self):
      """Update the TreeView Font"""
      self.renderer_text.set_property('font-desc', pango.FontDescription(self.tree_font))
      self.treeview_refresh()
   
   def treeview_refresh(self, change_icon=False):
      """Refresh the Treeview"""
      if self.curr_buffer:
         if not self.expand_tree: self.expanded_collapsed_string = config.get_tree_expanded_collapsed_string(self)
         self.treeview.set_model(None)
         if change_icon:
            tree_iter = self.treestore.get_iter_first()
            while tree_iter:
               self.change_icon_iter(tree_iter)
               tree_iter = self.treestore.iter_next(tree_iter)
         self.treeview.set_model(self.treestore)
         if self.expand_tree: self.treeview.expand_all()
         else: config.set_tree_expanded_collapsed_string(self)
         self.treeview.set_cursor(self.treestore.get_path(self.curr_tree_iter))
   
   def change_icon_iter(self, tree_iter):
      """Changing all icons type - iter"""
      self.treestore[tree_iter][0] = self.get_node_icon(self.treestore[tree_iter][5], self.treestore[tree_iter][4])
      child_tree_iter = self.treestore.iter_children(tree_iter)
      while child_tree_iter:
         self.change_icon_iter(child_tree_iter)
         child_tree_iter = self.treestore.iter_next(child_tree_iter)
   
   def file_startup_load(self, open_with_file):
      """Try to load a file if there are the conditions"""
      if open_with_file != "":
         self.file_name = os.path.basename(open_with_file)
         self.file_dir = os.path.dirname(open_with_file)
      if self.file_dir != "" and self.file_name != "" and os.path.isfile(os.path.join(self.file_dir, self.file_name)):
         self.file_load(os.path.join(self.file_dir, self.file_name))
         if self.expand_tree: self.treeview.expand_all()
         else: config.set_tree_expanded_collapsed_string(self) # restore expanded/collapsed nodes
         # we put the cursor on the first node as default
         first_node_iter = self.treestore.get_iter_first()
         if first_node_iter != None:
            self.treeview.set_cursor(self.treestore.get_path(first_node_iter))
            self.sourceview.grab_focus()
            # but then we try restore the latest situation
            if self.node_path != None:
               self.treeview.set_cursor(self.node_path)
               self.sourceview.grab_focus()
               self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(self.cursor_position))
               self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
      else: self.file_name = ""
      self.file_update = False
      
   def on_modified_changed(self, sourcebuffer):
      """When the modification flag is changed"""
      if self.user_active and sourcebuffer.get_modified() == True:
         self.update_window_save_needed()
      
   def file_save_as(self, *args):
      """Save the file providing a new name"""
      if self.tree_is_empty(): support.dialog_warning(_("The Tree is Empty!"), self.window)
      else:
         filepath = support.dialog_file_save_as(self.file_name,
                                                filter_pattern="*.ctd",
                                                filter_name=_("CherryTree Document"),
                                                curr_folder=self.file_dir,
                                                parent=self.window)
         if filepath != None:
            if not os.path.isfile(filepath)\
            or support.dialog_question(_("The File %s\nAlready Exists, do you want to Overwrite?") % filepath, self.window):
               if len(filepath) < 4 or filepath[-4:] != ".ctd": filepath += ".ctd"
               if self.file_write(filepath):
                  self.file_dir = os.path.dirname(filepath)
                  self.file_name = os.path.basename(filepath)
                  self.update_window_save_not_needed()
                  self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      
   def file_save(self, *args):
      """Save the file"""
      if self.file_dir != "" and self.file_name != "":
         if self.tree_is_empty(): support.dialog_warning(_("The Tree is Empty!"), self.window)
         elif self.file_write(os.path.join(self.file_dir, self.file_name)):
            self.update_window_save_not_needed()
            self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      else: self.file_save_as()
      
   def file_write(self, filepath):
      """File Write"""
      try: xml_string = self.xml_handler.treestore_to_dom()
      except:
         support.dialog_error("%s write failed - tree to xml" % filepath, self.window)
         return False
      # backup before save new version
      if os.path.isfile(filepath): shutil.move(filepath, filepath + cons.CHAR_TILDE)
      # if the filename is protected, we use unprotected type before compress and protect
      try:
         self.statusbar.push(self.statusbar_context_id, _("Writing to Disk..."))
         while gtk.events_pending(): gtk.main_iteration()
         if self.password:
            if not os.path.isdir(cons.TMP_FOLDER): os.mkdir(cons.TMP_FOLDER)
            filepath_tmp = os.path.join(cons.TMP_FOLDER, os.path.basename(filepath[:-1] + "d"))
            file_descriptor = open(filepath_tmp, 'w')
         else: file_descriptor = open(filepath, 'w')
         file_descriptor.write(xml_string)
         file_descriptor.close()
         if self.password:
            if sys.platform[0:3] == "win":
               filepath_4win = support.windows_cmd_prepare_path(filepath)
               filepath_tmp_4win = support.windows_cmd_prepare_path(filepath_tmp)
               if not filepath_4win or not filepath_tmp_4win:
                  support.dialog_error(_("The Contemporary Presence of Single and Double Quotes in the File Path Prevents 7za.exe to Work, Please Change the File Name"), self.window)
                  raise
               bash_str = '7za a -p%s -bd -y ' % self.password +\
                          filepath_4win + cons.CHAR_SPACE + filepath_tmp_4win
            else:
               bash_str = '7za a -p%s -bd -y %s %s' % (self.password,
                                                       re.escape(filepath),
                                                       re.escape(filepath_tmp))
            #print bash_str
            ret_code = subprocess.call(bash_str, shell=True)
            #print ret_code
            os.remove(filepath_tmp)
         self.statusbar.pop(self.statusbar_context_id)
         return True
      except:
         support.dialog_error("%s write failed - writing to disk" % filepath, self.window)
         return False
      
   def file_write_node_and_subnodes(self, filepath):
      """File Write with Selected Node and Subnodes"""
      try: xml_string = self.xml_handler.treestore_sel_node_and_subnodes_to_dom(self.curr_tree_iter)
      except:
         support.dialog_error("%s write failed - sel node and subnodes to xml" % filepath, self.window)
         return
      try:
         file_descriptor = open(filepath, 'w')
         file_descriptor.write(xml_string)
         file_descriptor.close()
      except: support.dialog_error("%s write failed - writing to disk" % filepath, self.window)
      
   def file_open(self, *args):
      """Opens a .CTD File"""
      filepath = support.dialog_file_select(filter_pattern="*.ct*",
                                            filter_name=_("CherryTree Document"),
                                            curr_folder=self.file_dir,
                                            parent=self.window)
      if filepath != None:
         if not self.reset(): return
         self.file_load(filepath)
         if self.expand_tree: self.treeview.expand_all()
         first_node_iter = self.treestore.get_iter_first()
         if first_node_iter != None:
            self.treeview.set_cursor(self.treestore.get_path(first_node_iter))
            self.sourceview.grab_focus()
      
   def dialog_edit_protection(self, *args):
      """Edit the Password for the Current Document"""
      if len(self.file_name) < 4:
         support.dialog_warning(_("No Document is Opened"), self.window)
         return
      edit_protection_dialog = gtk.Dialog(title=_("Document Protection"),
                                          parent=self.window,
                                          flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                                          buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                                          gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
      radiobutton_unprotected = gtk.RadioButton(label=_("Not Protected"))
      radiobutton_protected = gtk.RadioButton(label=_("Password Protected"))
      radiobutton_protected.set_group(radiobutton_unprotected)
      entry_passw_1 = gtk.Entry()
      entry_passw_1.set_visibility(False)
      entry_passw_2 = gtk.Entry()
      entry_passw_2.set_visibility(False)
      vbox_passw = gtk.VBox()
      vbox_passw.pack_start(entry_passw_1)
      vbox_passw.pack_start(entry_passw_2)
      passw_frame = gtk.Frame(label="<b>"+_("Enter the New Password Twice")+"</b>")
      passw_frame.get_label_widget().set_use_markup(True)
      passw_frame.add(vbox_passw)
      if self.password:
         radiobutton_unprotected.set_active(False)
         radiobutton_protected.set_active(True)
         passw_frame.set_sensitive(True)
      else:
         radiobutton_unprotected.set_active(True)
         radiobutton_protected.set_active(False)
         passw_frame.set_sensitive(False)
      content_area = edit_protection_dialog.get_content_area()
      content_area.pack_start(radiobutton_unprotected)
      content_area.pack_start(radiobutton_protected)
      content_area.pack_start(passw_frame)
      content_area.show_all()
      def on_key_press_edit_protection_dialog(widget, event):
         if gtk.gdk.keyval_name(event.keyval) == "Return":
            button_box = edit_protection_dialog.get_action_area()
            buttons = button_box.get_children()
            buttons[0].clicked() # first is the ok button
      def on_radiobutton_protected_toggled(widget):
         passw_frame.set_sensitive(widget.get_active())
      radiobutton_protected.connect("toggled", on_radiobutton_protected_toggled)
      edit_protection_dialog.connect("key_press_event", on_key_press_edit_protection_dialog)
      response = edit_protection_dialog.run()
      new_protection = {'on':radiobutton_protected.get_active(),
                        'p1':entry_passw_1.get_text(),
                        'p2':entry_passw_2.get_text()}
      edit_protection_dialog.destroy()
      if response != gtk.RESPONSE_ACCEPT: return
      former_filename = self.file_name
      if new_protection['on']:
         if new_protection['p1'] == "":
            support.dialog_error(_("The Password Fields Must be Filled"), self.window)
            return
         if new_protection['p1'] != new_protection['p2']:
            support.dialog_error(_("The Two Inserted Passwords Do Not Match"), self.window)
            return
         if not self.password and not self.is_7za_available(): return
         self.password = new_protection['p1']
         self.file_name = self.file_name[:-1] + "z"
      else:
         self.password = None
         self.file_name = self.file_name[:-1] + "d"
      self.window.set_title(self.window.get_title().replace(former_filename, self.file_name))
      self.file_save()
      
   def is_7za_available(self):
      """Check 7za binary executable to be available"""
      ret_code = subprocess.call("7za", shell=True)
      if ret_code:
         support.dialog_error(_("Binary Executable '7za' Not Found, Check The Package 'p7zip-full' to be Installed Properly"), self.window)
         return False
      return True
      
   def dialog_insert_password(self, file_name):
      """Prompts a Dialog Asking for the File Password"""
      enter_password_dialog = gtk.Dialog(title=_("Enter Password for %s") % file_name,
                                         parent=self.window,
                                         flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                                         buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                                         gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
      enter_password_dialog.set_default_size(300, -1)
      entry = gtk.Entry()
      entry.set_visibility(False)
      content_area = enter_password_dialog.get_content_area()
      content_area.pack_start(entry)
      content_area.show_all()
      def on_key_press_enter_password_dialog(widget, event):
         if gtk.gdk.keyval_name(event.keyval) == "Return":
            button_box = enter_password_dialog.get_action_area()
            buttons = button_box.get_children()
            buttons[0].clicked() # first is the ok button
      enter_password_dialog.connect("key_press_event", on_key_press_enter_password_dialog)
      response = enter_password_dialog.run()
      passw = entry.get_text()
      enter_password_dialog.destroy()
      while gtk.events_pending(): gtk.main_iteration()
      if response != gtk.RESPONSE_ACCEPT: return False
      self.password = passw
      return True
      
   def file_load(self, filepath):
      """Loads a .CTD into a GTK TreeStore"""
      if filepath[-1] == "z":
         if not self.dialog_insert_password(os.path.basename(filepath)): return
         if not self.is_7za_available(): return
         if not os.path.isdir(cons.TMP_FOLDER): os.mkdir(cons.TMP_FOLDER)
         filepath_tmp = os.path.join(cons.TMP_FOLDER, os.path.basename(filepath[:-1] + "d"))
         if sys.platform[0:3] == "win":
            dest_dir_4win = support.windows_cmd_prepare_path("-o" + cons.TMP_FOLDER)
            filepath_4win = support.windows_cmd_prepare_path(filepath)
            if not dest_dir_4win or not filepath_4win:
               support.dialog_error(_("The Contemporary Presence of Single and Double Quotes in the File Path Prevents 7za.exe to Work, Please Change the File Name"), self.window)
               return
            bash_str = '7za e -p%s -bd -y ' % self.password +\
                       dest_dir_4win + cons.CHAR_SPACE + filepath_4win
         else:
            bash_str = '7za e -p%s -bd -y -o%s %s' % (self.password,
                                                      re.escape(cons.TMP_FOLDER),
                                                      re.escape(filepath))
         #print bash_str
         ret_code = subprocess.call(bash_str, shell=True)
         if ret_code != 0:
            support.dialog_error(_('Wrong Password'), self.window)
            return
      elif filepath[-1] != "d":
         print "bad filepath[-1]", filepath[-1]
         support.dialog_error(_('"%s" is Not a CherryTree Document') % filepath, self.window)
         return
      else: self.password = None
      try:
         if self.password: file_descriptor = open(filepath_tmp, 'r')
         else: file_descriptor = open(filepath, 'r')
         cherrytree_string = file_descriptor.read()
         file_descriptor.close()
         if self.password: os.remove(filepath_tmp)
      except:
         support.dialog_error("Error opening the file %s" % filepath, self.window)
         return
      cherrytree_string = re.sub(cons.BAD_CHARS, "", cherrytree_string)
      self.user_active = False
      file_loaded = False
      try:
         if self.xml_handler.dom_to_treestore(cherrytree_string, discard_ids=False):
            self.file_dir = os.path.dirname(filepath)
            self.file_name = os.path.basename(filepath)
            self.update_window_save_not_needed()
            file_loaded = True
      except: pass
      if not file_loaded:
         support.dialog_error(_('"%s" is Not a CherryTree Document') % filepath, self.window)
         self.file_name = ""
      self.user_active = True
      
   def file_new(self, *args):
      """Starts a new unsaved instance"""
      if self.reset(): self.node_add()
      
   def reset(self):
      """Reset the Application"""
      if not self.tree_is_empty() and not self.check_unsaved(): return False
      if self.curr_tree_iter != None:
         self.curr_buffer.set_text("")
         self.curr_tree_iter = None
      self.treestore.clear()
      self.file_name = ""
      self.password = None
      self.node_id_counter = long(0)
      self.update_window_save_not_needed()
      self.state_machine.reset()
      self.sourceview.set_sensitive(False)
      return True
      
   def node_and_subnodes_export(self, action):
      """Export the Selected Node and its Subnodes"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      filepath = support.dialog_file_save_as(self.treestore[self.curr_tree_iter][1] + ".ctd",
                                             filter_pattern="*.ctd",
                                             filter_name=_("CherryTree Document"),
                                             curr_folder=self.file_dir,
                                             parent=self.window)
      if filepath == None: return
      if not os.path.isfile(filepath)\
      or support.dialog_question(_("The File %s\nAlready Exists, do you want to Overwrite?") % filepath, self.window):
         if len(filepath) < 4 or filepath[-4:] != ".ctd": filepath += ".ctd"
         self.file_write_node_and_subnodes(filepath)
      
   def node_export_to_plain_text(self, *args):
      """Export the Selected Node To Plain Text"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      filepath = support.dialog_file_save_as(self.treestore[self.curr_tree_iter][1] + ".txt",
                                             filter_pattern="*.txt",
                                             filter_name=_("Plain Text Document"),
                                             curr_folder=self.file_dir,
                                             parent=self.window)
      if filepath == None: return
      if not os.path.isfile(filepath)\
      or support.dialog_question(_("The File %s\nAlready Exists, do you want to Overwrite?") % filepath, self.window):
         if len(filepath) < 4 or filepath[-4:] != ".txt": filepath += ".txt"
         plain_text = self.curr_buffer.get_text(*self.curr_buffer.get_bounds())
         file_descriptor = open(filepath, 'w')
         file_descriptor.write(plain_text)
         file_descriptor.close()
      
   def node_export_to_html(self, *args):
      """Export the Selected Node To HTML"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      if self.html_handler.prepare_html_folder(self.treestore[self.curr_tree_iter][1]):
         self.html_handler.node_export_to_html(self.curr_tree_iter)
   
   def nodes_all_export_to_html(self, *args):
      """Export All Nodes To HTML"""
      if self.tree_is_empty():
         support.dialog_warning(_("The Tree is Empty!"), self.window)
         return
      if self.html_handler.prepare_html_folder(self.file_name):
         self.html_handler.nodes_all_export_to_html()
      
   def node_print_page_setup(self, action):
      """Print Page Setup Operations"""
      if self.print_handler.settings is None:
         self.print_handler.settings = gtk.PrintSettings()
      self.print_handler.page_setup = gtk.print_run_page_setup_dialog(self.glade.window,
                                                                      self.print_handler.page_setup,
                                                                      self.print_handler.settings)
      
   def node_print(self, action):
      """Start Print Operations"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      if self.treestore[self.curr_tree_iter][4] == cons.CUSTOM_COLORS_ID:
         pango_handler = exports.Export2Pango(self)
         pango_text, pixbuf_table_codebox_vector = pango_handler.pango_get_from_treestore_node(self.curr_tree_iter)
         self.print_handler.print_text(self.glade.window,
                                       pango_text,
                                       self.text_font,
                                       self.code_font,
                                       pixbuf_table_codebox_vector,
                                       self.get_text_window_width())
      else:
         print_compositor = gtksourceview2.print_compositor_new_from_view(self.sourceview)
         self.print_handler.print_code(self.glade.window, print_compositor, self.code_font)
   
   def node_siblings_sort_ascending(self, *args):
      """Sorts all the Siblings of the Selected Node Ascending"""
      father_iter = self.treestore.iter_parent(self.curr_tree_iter)
      movements = False
      while self.node_siblings_sort_iteration(self.treestore, father_iter, True, 1):
         movements = True
      if movements: self.update_window_save_needed()
      
   def node_siblings_sort_descending(self, *args):
      """Sorts all the Siblings of the Selected Node Descending"""
      father_iter = self.treestore.iter_parent(self.curr_tree_iter)
      movements = False
      while self.node_siblings_sort_iteration(self.treestore, father_iter, False, 1):
         movements = True
      if movements: self.update_window_save_needed()
      
   def node_siblings_sort_iteration(self, model, father_iter, descending, column):
      """Runs One Sorting Iteration, Returns True if Any Swap was Necessary"""
      if father_iter != None: curr_sibling = model.iter_children(father_iter)
      else: curr_sibling = model.get_iter_first()
      next_sibling = model.iter_next(curr_sibling)
      swap_executed = False
      while next_sibling != None:
         if (descending and model[next_sibling][column].lower() < model[curr_sibling][column].lower())\
         or (not descending and model[next_sibling][column].lower() > model[curr_sibling][column].lower()):
            model.swap(next_sibling, curr_sibling)
            swap_executed = True
         else: curr_sibling = next_sibling
         next_sibling = model.iter_next(curr_sibling)
      return swap_executed
   
   def node_inherit_syntax(self, *args):
      """Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      self.node_inherit_syntax_iter(self.curr_tree_iter)
      self.sourceview.set_buffer(self.treestore[self.curr_tree_iter][2])
      
   def node_inherit_syntax_iter(self, iter_father):
      """Iteration of the Node Inherit Syntax"""
      iter_child = self.treestore.iter_children(iter_father)
      while iter_child != None:
         if self.treestore[iter_child][4] != self.treestore[iter_father][4]:
            self.treestore[iter_child][4] = self.treestore[iter_father][4]
            self.treestore[iter_child][0] = self.get_node_icon(self.treestore[iter_child][5],
                                                               self.treestore[iter_child][4])
            self.switch_buffer_text_source(self.treestore[iter_child][2],
                                           iter_child,
                                           self.treestore[iter_child][4])
            if self.treestore[iter_child][4] != cons.CUSTOM_COLORS_ID:
               self.set_sourcebuffer_syntax_highlight(self.treestore[iter_child][2],
                                                      self.treestore[iter_child][4])
            self.update_window_save_needed()
         self.node_inherit_syntax_iter(iter_child)
         iter_child = self.treestore.iter_next(iter_child)
   
   def node_up(self, *args):
      """Node up one position"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      prev_iter = self.get_tree_iter_prev_sibling(self.treestore, self.curr_tree_iter)
      if prev_iter != None:
         self.treestore.swap(self.curr_tree_iter, prev_iter)
         self.treeview.set_cursor(self.treestore.get_path(self.curr_tree_iter))
         self.update_window_save_needed()
      
   def node_down(self, *args):
      """Node down one position"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      subseq_iter = self.treestore.iter_next(self.curr_tree_iter)
      if subseq_iter != None:
         self.treestore.swap(self.curr_tree_iter, subseq_iter)
         self.treeview.set_cursor(self.treestore.get_path(self.curr_tree_iter))
         self.update_window_save_needed()
   
   def node_left(self, *args):
      """Node left one position"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      father_iter = self.treestore.iter_parent(self.curr_tree_iter)
      if father_iter != None:
         self.treestore[self.curr_tree_iter][5] = self.treestore[father_iter][5]
         self.treestore[self.curr_tree_iter][0] = self.get_node_icon(self.treestore[self.curr_tree_iter][5],
                                                                     self.treestore[self.curr_tree_iter][4])
         self.node_move_after(self.curr_tree_iter,
                              self.treestore.iter_parent(father_iter),
                              father_iter)
   
   def node_move_after(self, iter_to_move, father_iter, brother_iter=None):
      """Move a node to a father and after a brother"""
      if brother_iter != None:
         new_node_iter = self.treestore.insert_after(father_iter, brother_iter, self.treestore[iter_to_move])
      else: new_node_iter = self.treestore.append(father_iter, self.treestore[iter_to_move])
      # we move also all the children
      self.node_move_children(iter_to_move, new_node_iter)
      # now we can remove the old iter (and all children)
      self.treestore.remove(iter_to_move)
      if father_iter != None: self.treeview.expand_row(self.treestore.get_path(father_iter), True)
      else: self.treeview.expand_row(self.treestore.get_path(new_node_iter), True)
      self.curr_tree_iter = new_node_iter
      self.treeview.set_cursor(self.treestore.get_path(new_node_iter))
      self.update_window_save_needed()
   
   def node_move_children(self, old_father, new_father):
      """Move the children from a father to another"""
      children_iter_to_move = self.treestore.iter_children(old_father)
      while children_iter_to_move != None:
         new_children_iter = self.treestore.append(new_father, self.treestore[children_iter_to_move])
         self.node_move_children(children_iter_to_move, new_children_iter)
         children_iter_to_move = self.treestore.iter_next(children_iter_to_move)
   
   def node_change_father(self, *args):
      """Node browse for a new father"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      curr_node_id = self.treestore[self.curr_tree_iter][3]
      old_father_iter = self.treestore.iter_parent(self.curr_tree_iter)
      if old_father_iter != None: old_father_node_id = self.treestore[old_father_iter][3]
      else: old_father_node_id = None
      self.node_choose_view_exist_or_create()
      self.glade.link_dialog_top_vbox.hide()
      self.glade.frame_link_anchor.hide()
      self.glade.choosenodedialog.set_title(_("Select the New Father"))
      self.glade.hbox_link_node_anchor.set_sensitive(True)
      response = self.glade.choosenodedialog.run()
      self.glade.choosenodedialog.hide()
      if response != 1: return # the user aborted the operation
      model, father_iter = self.treeviewselection_2.get_selected()
      new_father_node_id = self.treestore[father_iter][3]
      if curr_node_id == new_father_node_id:
         support.dialog_error(_("The new father can't be the very node to move!"), self.window)
         return
      if old_father_node_id != None and new_father_node_id == old_father_node_id:
         support.dialog_info(_("The new choosed father is again the old father!"), self.window)
         return
      move_towards_top_iter = self.treestore.iter_parent(father_iter)
      while move_towards_top_iter:
         if self.treestore[move_towards_top_iter][3] == curr_node_id:
            support.dialog_error(_("The new father can't be one of his sons!"), self.window)
            return
         move_towards_top_iter = self.treestore.iter_parent(move_towards_top_iter)
      self.treestore[self.curr_tree_iter][5] = self.treestore[father_iter][5] + 1
      self.treestore[self.curr_tree_iter][0] = self.get_node_icon(self.treestore[self.curr_tree_iter][5],
                                                                  self.treestore[self.curr_tree_iter][4])
      self.node_move_after(self.curr_tree_iter, father_iter)
   
   def node_choose_view_exist_or_create(self, node_sel_id=None):
      """If The View Was Never Used, this will Create It"""
      if "treeview_2" not in dir(self):
         self.treeview_2 = gtk.TreeView(self.treestore)
         self.treeview_2.set_headers_visible(False)
         self.renderer_pixbuf_2 = gtk.CellRendererPixbuf()
         self.renderer_text_2 = gtk.CellRendererText()
         self.column_2 = gtk.TreeViewColumn()
         self.column_2.pack_start(self.renderer_pixbuf_2, False)
         self.column_2.pack_start(self.renderer_text_2, True)
         self.column_2.set_attributes(self.renderer_pixbuf_2, stock_id=0)
         self.column_2.set_attributes(self.renderer_text_2, text=1)
         self.treeview_2.append_column(self.column_2)
         self.treeviewselection_2 = self.treeview_2.get_selection()
         self.treeview_2.connect('button-press-event', self.on_mouse_button_clicked_treeview_2)
         self.glade.scrolledwindow_choosenode.add(self.treeview_2)
         self.glade.scrolledwindow_choosenode.show_all()
      if node_sel_id == None:
         self.treeview_2.set_cursor(self.treestore.get_path(self.curr_tree_iter))
      else:
         tree_iter_sel = self.get_tree_iter_from_node_id(node_sel_id)
         if tree_iter_sel != None: self.treeview_2.set_cursor(self.treestore.get_path(tree_iter_sel))
         
   def on_mouse_button_clicked_treeview_2(self, widget, event):
      """Catches mouse buttons clicks"""
      if event.button != 1: return
      if event.type == gtk.gdk._2BUTTON_PRESS: self.glade.choosenodedialog_button_ok.clicked()
         
   def get_tree_iter_from_node_id(self, node_id):
      """Given a Node Id, Returns the TreeIter or None"""
      tree_iter = self.treestore.get_iter_first()
      while tree_iter != None:
         if self.treestore[tree_iter][3] == node_id: return tree_iter
         child_tree_iter = self.get_tree_iter_from_node_id_children(tree_iter, node_id)
         if child_tree_iter != None: return child_tree_iter
         tree_iter = self.treestore.iter_next(tree_iter)
      return None
   
   def get_tree_iter_from_node_id_children(self, father_iter, node_id):
      """Iterative function searching for Node Id between Children"""
      tree_iter = self.treestore.iter_children(father_iter)
      while tree_iter != None:
         if self.treestore[tree_iter][3] == node_id: return tree_iter
         child_tree_iter = self.get_tree_iter_from_node_id_children(tree_iter, node_id)
         if child_tree_iter != None: return child_tree_iter
         tree_iter = self.treestore.iter_next(tree_iter)
      return None
   
   def treeview_safe_set_cursor(self, tree_iter):
      """Set Cursor being sure the Node is Expanded"""
      nodes_to_expand = []
      father_iter = self.treestore.iter_parent(tree_iter)
      while father_iter:
         nodes_to_expand.insert(0, father_iter)
         father_iter = self.treestore.iter_parent(father_iter)
      for element_iter in nodes_to_expand:
         self.treeview.expand_row(self.treestore.get_path(element_iter), open_all=False)
      self.treeview.set_cursor(self.treestore.get_path(tree_iter))
   
   def on_table_column_rename_radiobutton_toggled(self, radiobutton):
      """Table Column Rename Toggled"""
      if radiobutton.get_active():
         self.glade.table_column_rename_entry.set_sensitive(True)
         self.table_column_mode = "rename"
      else: self.glade.table_column_rename_entry.set_sensitive(False)
      
   def on_table_column_delete_radiobutton_toggled(self, radiobutton):
      """Table Column Delete Toggled"""
      if radiobutton.get_active(): self.table_column_mode = "delete"
      
   def on_table_column_add_radiobutton_toggled(self, radiobutton):
      """Table Column Delete Toggled"""
      if radiobutton.get_active():
         self.glade.table_column_new_entry.set_sensitive(True)
         self.table_column_mode = "add"
      else: self.glade.table_column_new_entry.set_sensitive(False)
      
   def on_table_column_left_radiobutton_toggled(self, radiobutton):
      """Table Column Left Toggled"""
      if radiobutton.get_active(): self.table_column_mode = "left"
      
   def on_table_column_right_radiobutton_toggled(self, radiobutton):
      """Table Column Right Toggled"""
      if radiobutton.get_active(): self.table_column_mode = "right"
   
   def on_help_menu_item_activated(self, menuitem, data=None):
      """Show the application's Home Page"""
      webbrowser.open("http://www.giuspen.com/cherrytree/")
   
   def on_spinbutton_tab_width_value_changed(self, spinbutton):
      """Tabs Width (in chars) Change Handling"""
      self.tabs_width = int(spinbutton.get_value())
      self.sourceview.set_tab_width(self.tabs_width)
      
   def on_spinbutton_tree_nodes_names_width_value_changed(self, spinbutton):
      """Cherry Wrap Width Change Handling"""
      self.cherry_wrap_width = int(spinbutton.get_value())
      self.renderer_text.set_property('wrap-width', self.cherry_wrap_width)
      self.treeview_refresh()
      
   def on_checkbutton_custom_weblink_cmd_toggled(self, checkbutton):
      """Custom Weblink Clicked Action Toggled Handling"""
      if checkbutton.get_active(): self.weblink_custom_action[0] = True
      else: self.weblink_custom_action[0] = False
      self.glade.entry_custom_weblink_cmd.set_sensitive(self.weblink_custom_action[0])
      
   def on_entry_custom_weblink_cmd_changed(self, entry):
      """Custom Weblink Clicked Action Edited"""
      self.weblink_custom_action[1] = entry.get_text()
      
   def on_checkbutton_systray_toggled(self, checkbutton):
      """SysTray Toggled Handling"""
      self.systray = checkbutton.get_active()
      if self.systray:
         if "status_icon" in dir(self): self.status_icon.set_property('visible', True)
         else: self.status_icon_enable()
         self.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property('visible', True)
         self.glade.checkbutton_start_on_systray.set_sensitive(True)
      else:
         self.status_icon.set_property('visible', False)
         self.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property('visible', False)
         self.glade.checkbutton_start_on_systray.set_sensitive(False)
      
   def on_checkbutton_start_on_systray_toggled(self, checkbutton):
      """Start Minimized on SysTray Toggled Handling"""
      if checkbutton.get_active(): self.start_on_systray = True
      else: self.start_on_systray = False
      
   def on_checkbutton_autosave_toggled(self, checkbutton):
      """Autosave Toggled Handling"""
      self.autosave[0] = checkbutton.get_active()
      if not self.autosave[0] and self.autosave_timer_id != None: self.autosave_timer_stop()
      self.glade.spinbutton_autosave.set_sensitive(self.autosave[0])
      
   def on_checkbutton_line_wrap_toggled(self, checkbutton):
      """Lines Wrapping Toggled Handling"""
      self.line_wrapping = checkbutton.get_active()
      if self.line_wrapping: self.sourceview.set_wrap_mode(gtk.WRAP_WORD)
      else: self.sourceview.set_wrap_mode(gtk.WRAP_NONE)
      
   def on_checkbutton_spaces_tabs_toggled(self, checkbutton):
      """Insert Spaces Instead of Tabs Toggled Handling"""
      self.spaces_instead_tabs = checkbutton.get_active()
      self.sourceview.set_insert_spaces_instead_of_tabs(self.spaces_instead_tabs)
   
   def on_checkbutton_auto_indent_toggled(self, checkbutton):
      """Automatic Indentation Toggled Handling"""
      self.auto_indent = self.glade.checkbutton_auto_indent.get_active()
      self.sourceview.set_auto_indent(self.auto_indent)
   
   def on_checkbutton_line_nums_toggled(self, checkbutton):
      """Show Line Num Toggled Handling"""
      self.show_line_numbers = checkbutton.get_active()
      self.sourceview.set_show_line_numbers(self.show_line_numbers)
      
   def on_checkbutton_expand_tree_toggled(self, checkbutton):
      """Expand Tree When Loaded Toggled"""
      self.expand_tree = checkbutton.get_active()
      
   def on_checkbutton_newer_version_toggled(self, checkbutton):
      """Automatically Check for Newer Version Toggled"""
      self.check_version = checkbutton.get_active()
   
   def on_checkbutton_tree_right_side_toggled(self, checkbutton):
      """Display Tree on the Right Side Toggled"""
      if not self.user_active: return
      self.tree_right_side = checkbutton.get_active()
      tree_width = self.scrolledwindow_tree.get_allocation().width
      text_width = self.vbox_text.get_allocation().width
      self.hpaned.remove(self.scrolledwindow_tree)
      self.hpaned.remove(self.vbox_text)
      if self.tree_right_side:
         self.hpaned.add1(self.vbox_text)
         self.hpaned.add2(self.scrolledwindow_tree)
         self.hpaned.set_property('position', text_width)
      else:
         self.hpaned.add1(self.scrolledwindow_tree)
         self.hpaned.add2(self.vbox_text)
         self.hpaned.set_property('position', tree_width)
      
   def on_checkbutton_table_ins_from_file_toggled(self, checkbutton):
      """Import Table from CSV File Toggled"""
      self.glade.tablehandle_frame_table.set_sensitive(not checkbutton.get_active())
      self.glade.tablehandle_frame_col.set_sensitive(not checkbutton.get_active())
      
   def on_radiobutton_link_website_toggled(self, radiobutton):
      """Show/Hide Relative Frames"""
      if radiobutton.get_active(): self.link_type = "webs"
      self.glade.frame_link_website_url.set_sensitive(self.link_type == "webs")
      self.glade.hbox_link_node_anchor.set_sensitive(self.link_type == "node")
      self.glade.frame_link_filepath.set_sensitive(self.link_type == "file")
      
   def on_radiobutton_link_node_anchor_toggled(self, checkbutton):
      """Show/Hide Relative Frames"""
      if checkbutton.get_active(): self.link_type = "node"
      self.glade.frame_link_website_url.set_sensitive(self.link_type == "webs")
      self.glade.hbox_link_node_anchor.set_sensitive(self.link_type == "node")
      self.glade.frame_link_filepath.set_sensitive(self.link_type == "file")
      
   def on_radiobutton_link_file_toggled(self, radiobutton):
      """Show/Hide Relative Frames"""
      if radiobutton.get_active(): self.link_type = "file"
      self.glade.frame_link_website_url.set_sensitive(self.link_type == "webs")
      self.glade.hbox_link_node_anchor.set_sensitive(self.link_type == "node")
      self.glade.frame_link_filepath.set_sensitive(self.link_type == "file")
   
   def on_radiobutton_node_icon_cherry_toggled(self, radiobutton):
      """Change Variable Value Accordingly"""
      if radiobutton.get_active():
         self.nodes_icons = "c"
         if self.user_active: self.treeview_refresh(change_icon=True)
   
   def on_radiobutton_node_icon_bullet_toggled(self, radiobutton):
      """Change Variable Value Accordingly"""
      if radiobutton.get_active():
         self.nodes_icons = "b"
         if self.user_active: self.treeview_refresh(change_icon=True)
   
   def on_radiobutton_node_icon_none_toggled(self, radiobutton):
      """Change Variable Value Accordingly"""
      if radiobutton.get_active():
         self.nodes_icons = "n"
         if self.user_active: self.treeview_refresh(change_icon=True)
   
   def on_mouse_button_clicked_tree(self, widget, event):
      """Catches mouse buttons clicks"""
      if event.button == 3:
         self.ui.get_widget("/TreeMenu").popup(None, None, None, event.button, event.time)
      elif event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
         self.node_edit()
      
   def buffer_create(self, syntax_highlighting):
      """Returns a New Instantiated SourceBuffer"""
      if syntax_highlighting != cons.CUSTOM_COLORS_ID:
         buffer = gtksourceview2.Buffer()
         self.set_sourcebuffer_syntax_highlight(buffer, syntax_highlighting)
         buffer.set_highlight_matching_brackets(True)
         return buffer
      else: return gtk.TextBuffer(self.tag_table)
      
   def combobox_prog_lang_init(self):
      """Init The Programming Languages Syntax Highlighting"""
      self.prog_lang_liststore = gtk.ListStore(str, str)
      self.prog_lang_liststore.append([_("Disabled (Custom Colors)"), cons.CUSTOM_COLORS_ID])
      self.language_manager = gtksourceview2.LanguageManager()
      self.available_languages = self.language_manager.get_language_ids()
      if "def" in self.available_languages: self.available_languages.remove("def")
      for language_id in sorted(self.available_languages):
         self.prog_lang_liststore.append([self.language_manager.get_language(language_id).get_name(), language_id])
      for combobox in [self.glade.combobox_prog_lang, self.glade.combobox_prog_lang_codebox]:
         combobox.set_model(self.prog_lang_liststore)
         cell = gtk.CellRendererText()
         combobox.pack_start(cell, True)
         combobox.add_attribute(cell, 'text', 0)
         combobox.set_active_iter(self.get_combobox_prog_lang_iter(self.syntax_highlighting))
   
   def get_combobox_prog_lang_iter(self, prog_language):
      """Returns the Language iter Given the Language Name"""
      curr_iter = self.prog_lang_liststore.get_iter_first()
      while curr_iter != None:
         if self.prog_lang_liststore[curr_iter][1] == prog_language: break
         else: curr_iter = self.prog_lang_liststore.iter_next(curr_iter)
      else: return self.prog_lang_liststore.get_iter_first()
      return curr_iter
   
   def on_combobox_country_language_changed(self, combobox):
      """New Country Language Choosed"""
      new_iter = self.glade.combobox_country_language.get_active_iter()
      new_lang = self.country_lang_liststore[new_iter][0]
      if new_lang != self.country_lang:
         self.country_lang = new_lang
         support.dialog_info(_("The New Language will be Available Only After Restarting CherryTree"), self.window)
         lang_file_descriptor = file(cons.LANG_PATH, 'w')
         lang_file_descriptor.write(new_lang)
         lang_file_descriptor.close()
   
   def combobox_country_lang_init(self):
      """Init The Programming Languages Syntax Highlighting"""
      combobox = self.glade.combobox_country_language
      self.country_lang_liststore = gtk.ListStore(str)
      combobox.set_model(self.country_lang_liststore)
      cell = gtk.CellRendererText()
      combobox.pack_start(cell, True)
      combobox.add_attribute(cell, 'text', 0)
      for country_lang in cons.AVAILABLE_LANGS:
         self.country_lang_liststore.append([country_lang])
      combobox.set_active_iter(self.get_combobox_country_lang_iter(self.country_lang))
      self.glade.combobox_country_language.connect('changed', self.on_combobox_country_language_changed)
   
   def get_combobox_country_lang_iter(self, country_language):
      """Returns the Language iter Given the Language Name"""
      curr_iter = self.country_lang_liststore.get_iter_first()
      while curr_iter != None:
         if self.country_lang_liststore[curr_iter][0] == country_language: break
         else: curr_iter = self.country_lang_liststore.iter_next(curr_iter)
      else: return self.country_lang_liststore.get_iter_first()
      return curr_iter
      
   def set_sourcebuffer_syntax_highlight(self, sourcebuffer, syntax_highlighting):
      """Set the given syntax highlighting to the given sourcebuffer"""
      language_id = self.prog_lang_liststore[self.get_combobox_prog_lang_iter(syntax_highlighting)][1]
      sourcebuffer.set_language(self.language_manager.get_language(language_id))
      sourcebuffer.set_highlight_syntax(True)
      
   def node_add(self, *args):
      """Add a node having common father with the selected node's"""
      node_name = self.dialog_input(title=_("Insert the New Node Name..."), syntax_highlight=True)
      if node_name == None: return
      self.update_window_save_needed()
      self.syntax_highlighting = self.prog_lang_liststore[self.glade.combobox_prog_lang.get_active_iter()][1]
      if self.curr_tree_iter == None: node_level = 0
      else: node_level = self.treestore[self.curr_tree_iter][5]
      cherry = self.get_node_icon(node_level, self.syntax_highlighting)
      if self.curr_tree_iter != None:
         new_node_iter = self.treestore.insert_after(self.treestore.iter_parent(self.curr_tree_iter), 
                                                     self.curr_tree_iter, [cherry,
                                                                           node_name,
                                                                           self.buffer_create(self.syntax_highlighting),
                                                                           self.node_id_get(),
                                                                           self.syntax_highlighting,
                                                                           node_level,
                                                                           self.glade.tags_searching_entry.get_text()])
      else:
         new_node_iter = self.treestore.append(None, [cherry,
                                                      node_name,
                                                      self.buffer_create(self.syntax_highlighting),
                                                      self.node_id_get(),
                                                      self.syntax_highlighting,
                                                      node_level,
                                                      self.glade.tags_searching_entry.get_text()])
      new_node_path = self.treestore.get_path(new_node_iter)
      self.treeview.set_cursor(new_node_path)
      self.sourceview.grab_focus()
      
   def node_child_add(self, *args):
      """Add a node having as father the selected node"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      node_name = self.dialog_input(title=_("Insert the New Child Node Name..."), syntax_highlight=True)
      if node_name != None:
         self.update_window_save_needed()
         self.syntax_highlighting = self.prog_lang_liststore[self.glade.combobox_prog_lang.get_active_iter()][1]
         if self.curr_tree_iter == None: node_level = 0
         else: node_level = self.treestore[self.curr_tree_iter][5] + 1
         cherry = self.get_node_icon(node_level, self.syntax_highlighting)
         new_node_iter = self.treestore.append(self.curr_tree_iter, [cherry,
                                                                     node_name,
                                                                     self.buffer_create(self.syntax_highlighting),
                                                                     self.node_id_get(),
                                                                     self.syntax_highlighting,
                                                                     node_level,
                                                                     self.glade.tags_searching_entry.get_text()])
         new_node_path = self.treestore.get_path(new_node_iter)
         father_node_path = self.treestore.get_path(self.curr_tree_iter)
         self.treeview.expand_row(father_node_path, True) # second parameter tells whether to expand childrens too
         self.treeview.set_cursor(new_node_path)
         self.sourceview.grab_focus()
         
   def node_delete(self, *args):
      """Delete the Selected Node"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      warning_label = _("Are you sure to <b>Delete the node '%s'?</b>") % self.treestore[self.curr_tree_iter][1]
      if self.treestore.iter_children(self.curr_tree_iter) != None:
         warning_label += _("\n\nThe node <b>has Children, they will be Deleted too!</b>")
      self.glade.label_node_delete.set_text(warning_label)
      self.glade.label_node_delete.set_use_markup(True)
      response = self.glade.nodedeletedialog.run()
      self.glade.nodedeletedialog.hide()
      if response != 1: return # the user did not confirm
      # next selected node will be previous sibling or next sibling or father or None
      new_iter = self.get_tree_iter_prev_sibling(self.treestore, self.curr_tree_iter)
      if new_iter == None:
         new_iter = self.treestore.iter_next(self.curr_tree_iter)
         if new_iter == None: new_iter = self.treestore.iter_parent(self.curr_tree_iter)
      self.treestore.remove(self.curr_tree_iter)
      self.curr_tree_iter = None
      if new_iter != None:
         self.treeview.set_cursor(self.treestore.get_path(new_iter))
         self.sourceview.grab_focus()
      self.update_window_save_needed()
         
   def node_edit(self, *args):
      """Edit the Properties of the Selected Node"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      self.glade.combobox_prog_lang.set_active_iter(self.get_combobox_prog_lang_iter(self.treestore[self.curr_tree_iter][4]))
      self.glade.tags_searching_entry.set_text(self.treestore[self.curr_tree_iter][6])
      node_name = self.dialog_input(entry_hint=self.treestore[self.curr_tree_iter][1],
                                    title=_("Insert the New Name for the Node..."),
                                    syntax_highlight=True)
      if node_name == None: return
      self.syntax_highlighting = self.prog_lang_liststore[self.glade.combobox_prog_lang.get_active_iter()][1]
      if self.treestore[self.curr_tree_iter][4] == cons.CUSTOM_COLORS_ID and self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
         if not support.dialog_question(_("Entering the Automatic Syntax Highlighting you will Lose all Custom Colors for This Node, Do you want to Continue?"), self.window):
            self.syntax_highlighting = cons.CUSTOM_COLORS_ID # STEP BACK (we stay in CUSTOM COLORS)
            return
         else:
            # SWITCH TextBuffer -> SourceBuffer
            self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting)
            self.curr_buffer = self.treestore[self.curr_tree_iter][2]
      elif self.treestore[self.curr_tree_iter][4] != cons.CUSTOM_COLORS_ID and self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         # SWITCH SourceBuffer -> TextBuffer
         self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting)
         self.curr_buffer = self.treestore[self.curr_tree_iter][2]
      self.treestore[self.curr_tree_iter][1] = node_name
      self.treestore[self.curr_tree_iter][4] = self.syntax_highlighting
      self.treestore[self.curr_tree_iter][6] = self.glade.tags_searching_entry.get_text()
      self.treestore[self.curr_tree_iter][0] = self.get_node_icon(self.treestore[self.curr_tree_iter][5],
                                                                  self.syntax_highlighting)
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
         self.set_sourcebuffer_syntax_highlight(self.curr_buffer, self.syntax_highlighting)
      self.update_window_save_needed()
      self.sourceview.grab_focus()
      
   def switch_buffer_text_source(self, buffer, iter, new_syntax_highl):
      """Switch TextBuffer -> SourceBuffer or SourceBuffer -> TextBuffer"""
      self.user_active = False
      node_text = buffer.get_text(*buffer.get_bounds())
      self.treestore[iter][2] = self.buffer_create(new_syntax_highl)
      self.treestore[iter][2].set_text(node_text)
      self.sourceview.set_buffer(self.treestore[iter][2])
      self.treestore[iter][2].connect('modified-changed', self.on_modified_changed)
      if new_syntax_highl == cons.CUSTOM_COLORS_ID:
         self.treestore[iter][2].connect('insert-text', self.on_text_insertion)
         self.treestore[iter][2].connect('delete-range', self.on_text_removal)
         self.sourceview.modify_font(pango.FontDescription(self.text_font))
      else:
         self.sourceview.modify_font(pango.FontDescription(self.code_font))
      self.user_active = True
      
   def on_node_changed(self, *args):
      """Actions to be triggered from the changing of node"""
      model, new_iter = self.treeviewselection.get_selected()
      if new_iter == None: return # no node selected
      elif self.curr_tree_iter != None and model[new_iter][3] == model[self.curr_tree_iter][3]:
         return # if i click on an already selected node
      if self.curr_tree_iter != None:
         self.nodes_cursor_pos[model[self.curr_tree_iter][3]] = self.curr_buffer.get_property('cursor-position')
         if self.curr_buffer.get_modified() == True:
            self.file_update = True
            self.curr_buffer.set_modified(False)
            self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      self.curr_tree_iter = new_iter
      self.curr_buffer = self.treestore[self.curr_tree_iter][2]
      self.sourceview.set_buffer(self.curr_buffer)
      self.syntax_highlighting = self.treestore[self.curr_tree_iter][4]
      self.curr_buffer.connect('modified-changed', self.on_modified_changed)
      if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         self.curr_buffer.connect('insert-text', self.on_text_insertion)
         self.curr_buffer.connect('delete-range', self.on_text_removal)
         self.sourceview.modify_font(pango.FontDescription(self.text_font))
      else: self.sourceview.modify_font(pango.FontDescription(self.code_font))
      self.sourceview.set_sensitive(True)
      self.header_node_name_label.set_text("<big><b><i>"+cgi.escape(self.treestore[self.curr_tree_iter][1])+"</i></b></big>")
      self.header_node_name_label.set_use_markup(True)
      self.state_machine.node_selected_changed(self.treestore[self.curr_tree_iter][3])
      self.objects_buffer_refresh()
      # try to restore cursor position if in memory
      if model[new_iter][3] in self.nodes_cursor_pos:
         self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(self.nodes_cursor_pos[model[new_iter][3]]))
         self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
      
   def update_window_save_needed(self):
      """Window title preceeded by an asterix"""
      if self.file_name != "": self.window.set_title("*" + self.file_name + " - CherryTree")
      else: self.window.set_title("*CherryTree")
      self.file_update = True
      
   def update_window_save_not_needed(self):
      """Window title not preceeded by an asterix"""
      if self.file_name != "": self.window.set_title(self.file_name + " - CherryTree")
      else: self.window.set_title("CherryTree")
      self.file_update = False
      if self.curr_tree_iter != None:
         self.curr_buffer.set_modified(False)
         curr_iter = self.curr_buffer.get_start_iter()
         while 1:
            anchor = curr_iter.get_child_anchor()
            if anchor != None and "sourcebuffer" in dir(anchor): anchor.sourcebuffer.set_modified(False)
            if not curr_iter.forward_char(): break
      
   def replace_again(self, *args):
      """Continue the previous replace (a_node/in_selected_node/in_all_nodes)"""
      self.find_handler.replace_again()
      
   def find_again(self, *args):
      """Continue the previous search (a_node/in_selected_node/in_all_nodes)"""
      self.find_handler.find_again()
      
   def find_back(self, *args):
      """Continue the previous search (a_node/in_selected_node/in_all_nodes) but in Opposite Direction"""
      self.find_handler.find_back()
      
   def replace_in_selected_node(self, *args):
      """Replace a pattern in the selected Node"""
      self.find_handler.replace_in_selected_node()
      
   def find_in_selected_node(self, *args):
      """Search for a pattern in the selected Node"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected!"), self.window)
         return
      self.find_handler.find_in_selected_node()
      
   def replace_in_all_nodes(self, *args):
      """Replace the pattern in all the Tree Nodes"""
      self.find_handler.replace_in_all_nodes()
      
   def find_in_all_nodes(self, *args):
      """Search for a pattern in all the Tree Nodes"""
      if self.tree_is_empty():
         support.dialog_warning(_("The Tree is Empty!"), self.window)
         return
      self.find_handler.find_in_all_nodes()
      
   def replace_in_nodes_names(self, *args):
      """Replace the pattern between all the Node's Names"""
      self.find_handler.replace_in_nodes_names()
      
   def find_a_node(self, *args):
      """Search for a pattern between all the Node's Names"""
      if self.tree_is_empty():
         support.dialog_warning(_("The Tree is Empty!"), self.window)
         return
      self.find_handler.find_a_node()
      
   def get_tree_iter_last_sibling(self, node_iter):
      """Returns the last top level iter or None if the tree is empty"""
      if node_iter == None:
         node_iter = self.treestore.get_iter_first()
         if node_iter == None: return None
      next_iter = self.treestore.iter_next(node_iter)
      while next_iter != None:
         node_iter = next_iter
         next_iter = self.treestore.iter_next(next_iter)
      return node_iter
      
   def get_tree_iter_prev_sibling(self, model, node_iter):
      """Returns the previous sibling iter or None if the given iter is the first"""
      node_path = model.get_path(node_iter)
      sibling_index = len(node_path)-1
      prev_iter = None
      while prev_iter == None and node_path[sibling_index] > 0:
         node_path_list = list(node_path)
         node_path_list[sibling_index] -= 1
         prev_path = tuple(node_path_list)
         prev_iter = model.get_iter(prev_path)
      return prev_iter
      
   def requested_step_back(self, *args):
      """Step Back for the Current Node, if Possible"""
      if self.curr_tree_iter == None: return
      if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         # TEXT BUFFER STATE MACHINE
         step_back = self.state_machine.requested_previous_state(self.treestore[self.curr_tree_iter][3])
         # step_back is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
         if step_back != None:
            self.user_active = False
            self.xml_handler.dom_to_buffer(self.curr_buffer, step_back[0])
            pixbuf_table_vector = step_back[1]
            # pixbuf_table_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
            for element in pixbuf_table_vector:
               if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(self.curr_buffer, element[1])
               elif element[0] == "table": self.state_machine.load_embedded_table_element(self.curr_buffer, element[1])
               elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(self.curr_buffer, element[1])
            self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(step_back[2]))
            self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
            self.user_active = True
            self.update_window_save_needed()
      elif self.curr_buffer.can_undo():
         self.curr_buffer.undo()
         self.update_window_save_needed()
      
   def requested_step_ahead(self, *args):
      """Step Ahead for the Current Node, if Possible"""
      if self.curr_tree_iter == None: return
      if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         # TEXT BUFFER STATE MACHINE
         step_ahead = self.state_machine.requested_subsequent_state(self.treestore[self.curr_tree_iter][3])
         # step_ahead is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
         if step_ahead != None:
            self.user_active = False
            self.xml_handler.dom_to_buffer(self.curr_buffer, step_ahead[0])
            pixbuf_table_vector = step_ahead[1]
            # pixbuf_table_vector is [ [ "pixbuf"/"table", [offset, pixbuf, alignment] ],... ]
            for element in pixbuf_table_vector:
               if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(self.curr_buffer, element[1])
               elif element[0] == "table": self.state_machine.load_embedded_table_element(self.curr_buffer, element[1])
               elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(self.curr_buffer, element[1])
            self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(step_ahead[2]))
            self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
            self.user_active = True
            self.update_window_save_needed()
      elif self.curr_buffer.can_redo():
         self.curr_buffer.redo()
         self.update_window_save_needed()
   
   def objects_buffer_refresh(self):
      """Buffer Refresh (Needed for Objects)"""
      refresh = self.state_machine.requested_current_state(self.treestore[self.curr_tree_iter][3])
      # refresh is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
      pixbuf_table_vector = refresh[1]
      if len(pixbuf_table_vector) > 0:
         self.user_active = False
         self.curr_buffer.set_text("")
         self.xml_handler.dom_to_buffer(self.curr_buffer, refresh[0])
         for element in pixbuf_table_vector:
            if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(self.curr_buffer, element[1])
            elif element[0] == "table": self.state_machine.load_embedded_table_element(self.curr_buffer, element[1])
            elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(self.curr_buffer, element[1])
         self.curr_buffer.set_modified(False)
         self.user_active = True
   
   def on_text_insertion(self, sourcebuffer, text_iter, text_inserted, length):
      """Text insertion callback"""
      if self.user_active:
         self.state_machine.text_variation(self.treestore[self.curr_tree_iter][3], text_inserted)
   
   def on_text_removal(self, sourcebuffer, start_iter, end_iter):
      """Text removal callback"""
      if self.user_active:
         self.state_machine.text_variation(self.treestore[self.curr_tree_iter][3], 
                                           sourcebuffer.get_text(start_iter, end_iter))
   
   def horizontal_rule_insert(self, action):
      """Insert a Horizontal Line"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected"), self.window)
         return
      self.curr_buffer.insert_at_cursor(cons.HORIZONTAL_RULE)
   
   def dialog_input(self, entry_hint=None, title=None, search_opt=False, replace_opt=False, syntax_highlight=False):
      """Opens the Input Dialog"""
      if title != None: self.glade.inputdialog.set_title(title)
      if entry_hint != None: self.glade.input_entry.set_text(entry_hint)
      else: self.glade.input_entry.set_text("")
      self.glade.input_entry.grab_focus()
      self.glade.search_options_frame.set_property("visible", search_opt)
      self.glade.replace_options_frame.set_property("visible", replace_opt)
      self.glade.syntax_highlighting_frame.set_property("visible", syntax_highlight)
      self.glade.tags_searching_frame.set_property("visible", syntax_highlight)
      response = self.glade.inputdialog.run()
      self.glade.inputdialog.hide()
      if response == 1:
         input_text = self.glade.input_entry.get_text().decode("utf-8")
         if len(input_text) > 0: return input_text
         else: return None
      else: return None
      
   def toolbar_icons_size_increase(self, *args):
      """Increase the Size of the Toolbar Icons"""
      if self.toolbar_icon_size == 5:
         support.dialog_info(_("The Size of the Toolbar Icons is already at the Maximum Value"), self.window)
         return
      self.toolbar_icon_size += 1
      self.ui.get_widget("/ToolBar").set_property("icon-size", cons.ICONS_SIZE[self.toolbar_icon_size])
   
   def toolbar_icons_size_decrease(self, *args):
      """Decrease the Size of the Toolbar Icons"""
      if self.toolbar_icon_size == 1:
         support.dialog_info(_("The Size of the Toolbar Icons is already at the Minimum Value"), self.window)
         return
      self.toolbar_icon_size -= 1
      self.ui.get_widget("/ToolBar").set_property("icon-size", cons.ICONS_SIZE[self.toolbar_icon_size])
      
   def toggle_show_hide_toolbar(self, *args):
      """Toggle Show/Hide the Toolbar"""
      if self.ui.get_widget("/ToolBar").get_property("visible"): self.ui.get_widget("/ToolBar").hide()
      else: self.ui.get_widget("/ToolBar").show()
      
   def toggle_show_hide_tree(self, *args):
      """Toggle Show/Hide the Tree"""
      if self.scrolledwindow_tree.get_property("visible"):
         self.scrolledwindow_tree.hide()
      else: self.scrolledwindow_tree.show()
      
   def toggle_show_hide_node_name_header(self, *args):
      """Toggle Show/Hide the Node Title Header"""
      if self.header_node_name_label.get_property("visible"):
         self.header_node_name_label.hide()
      else: self.header_node_name_label.show()
      
   def quit_application(self, *args):
      """Just Hide or Quit the gtk main loop"""
      if self.systray: self.window.hide()
      else: self.quit_application_totally()
      
   def quit_application_totally(self, *args):
      """The process is Shut Down"""
      if not self.check_unsaved():
         self.really_quit = False # user pressed cancel
         return
      config.config_file_save(self)
      self.window.destroy()
      gtk.main_quit()
      if "status_icon" in dir(self): self.status_icon.set_visible(False)
      
   def on_window_delete_event(self, widget, event, data=None):
      """Before close the application (from the window top right X)..."""
      self.really_quit = True
      self.quit_application()
      if not self.really_quit: return True # stop the delete event (user pressed cancel)
      else: return self.systray # True == stop the delete event, False == propogate the delete event
      
   def check_unsaved(self):
      """Before close the current document, check for possible Unsaved"""
      if self.curr_tree_iter != None and (self.curr_buffer.get_modified() == True or self.file_update == True):
         if self.autosave[0]: response = 2
         else:
            response = self.glade.exitdialog.run()
            self.glade.exitdialog.hide()
         if response == 2: self.file_save() # button YES pressed or autosave ON
      else: response = 0 # no need to save
      if response == 6: return False # button CANCEL
      else: return True
      
   def dialog_about(self, *args):
      """Show the About Dialog and hide it when a button is pressed"""
      self.glade.aboutdialog.run()
      self.glade.aboutdialog.hide()
      
   def anchor_handle(self, action):
      """Insert an Anchor"""
      if not self.node_sel_and_rich_text(): return
      iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
      self.anchor_edit_dialog(gtk.gdk.pixbuf_new_from_file(cons.ANCHOR_CHAR), iter_insert)
      
   def anchor_edit(self, *args):
      """Edit an Anchor"""
      iter_insert = self.curr_buffer.get_iter_at_child_anchor(self.curr_anchor_anchor)
      iter_bound = iter_insert.copy()
      iter_bound.forward_char()
      self.anchor_edit_dialog(self.curr_anchor_anchor.pixbuf, iter_insert, iter_bound)
      
   def anchor_edit_dialog(self, pixbuf, iter_insert, iter_bound=None):
      """Anchor Edit Dialog"""
      if "anchor" in dir (pixbuf):
         self.glade.anchor_insert_edit_entry.set_text(pixbuf.anchor)
         self.glade.anchorhandledialog.set_title(_("Edit Anchor"))
      else:
         self.glade.anchor_insert_edit_entry.set_text("")
         self.glade.anchorhandledialog.set_title(_("Insert Anchor"))
      self.glade.anchor_enter_name_hbox.show()
      self.glade.scrolledwindow_anchors_list.hide()
      response = self.glade.anchorhandledialog.run()
      self.glade.anchorhandledialog.hide()
      if response != 1: return # the user aborted the operation
      anchor = self.glade.anchor_insert_edit_entry.get_text().strip()
      if anchor == "":
         support.dialog_error(_("The Anchor Name is Mandatory!"), self.window)
         return
      pixbuf.anchor = anchor
      if iter_bound != None: # only in case of modify
         image_justification = self.state_machine.get_iter_alignment(iter_insert)
         image_offset = iter_insert.get_offset()
         self.curr_buffer.delete(iter_insert, iter_bound)
         iter_insert = self.curr_buffer.get_iter_at_offset(image_offset)
      else: image_justification = None
      self.image_insert(iter_insert, pixbuf, image_justification)
      
   def toc_insert(self, *args):
      """Insert Table Of Contents"""
      if not self.node_sel_and_rich_text(): return
      if not self.xml_handler.toc_insert(self.curr_buffer, self.treestore[self.curr_tree_iter][3]):
         support.dialog_warning(_("Not Any H1 or H2 Formatting Found"), self.window)
   
   def table_handle(self, *args):
      """Insert Table"""
      if not self.node_sel_and_rich_text(): return
      self.tables_handler.table_handle()
   
   def codebox_handle(self, *args):
      """Insert Code Box"""
      if not self.node_sel_and_rich_text(): return
      self.codeboxes_handler.codebox_handle()
      
   def on_radiobutton_codebox_pixels_toggled(self, radiobutton):
      """Radiobutton CodeBox Pixels/Percent Toggled"""
      if not self.user_active: return
      if radiobutton.get_active():
         self.glade.spinbutton_codebox_width.set_value(700)
      else: self.glade.spinbutton_codebox_width.set_value(100)
      
   def node_sel_and_rich_text(self):
      """Returns True if there's not a node selected or is not rich text"""
      if self.curr_tree_iter == None:
         support.dialog_warning(_("No Node is Selected"), self.window)
         return False
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
         support.dialog_warning(_("Automatic Syntax Highlighting Must be Disabled in order to Use This Feature"), self.window)
         return False
      return True
      
   def tree_cell_edited(self, cell, path, new_text):
      """A Table Cell is going to be Edited"""
      if new_text != self.treestore[path][1]:
         self.treestore[path][1] = new_text
         self.header_node_name_label.set_text("<big><b><i>"+new_text+"</i></b></big>")
         self.header_node_name_label.set_use_markup(True)
         self.update_window_save_needed()
      
   def tree_info(self, action):
      """Tree Summary Information"""
      if self.tree_is_empty(): support.dialog_warning(_("The Tree is Empty!"), self.window)
      else:
         self.summary_nodes_text_num = 0
         self.summary_nodes_code_num = 0
         self.summary_images_num = 0
         self.summary_tables_num = 0
         # full tree parsing
         tree_iter = self.treestore.get_iter_first()
         while tree_iter != None:
            self.tree_info_iter(tree_iter)
            tree_iter = self.treestore.iter_next(tree_iter)
         self.glade.label_summary_text_nodes.set_text("%s" % self.summary_nodes_text_num)
         self.glade.label_summary_code_nodes.set_text("%s" % self.summary_nodes_code_num)
         self.glade.label_summary_images.set_text("%s" % self.summary_images_num)
         self.glade.label_summary_tables.set_text("%s" % self.summary_tables_num)
         self.glade.dialogtreeinfo.run()
         self.glade.dialogtreeinfo.hide()
      
   def tree_info_iter(self, tree_iter):
      """Tree Summary Information Iteration"""
      curr_buffer = self.treestore[tree_iter][2]
      pixbuf_table_vector = self.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer)
      # pixbuf_table_vector is [ [ "pixbuf"/"table", [offset, pixbuf, alignment] ],... ]
      if self.treestore[tree_iter][4] == cons.CUSTOM_COLORS_ID: self.summary_nodes_text_num += 1
      else: self.summary_nodes_code_num += 1
      curr_node_images = 0
      curr_node_tables = 0
      for element in pixbuf_table_vector:
         if element[0] == "pixbuf": curr_node_images += 1
         elif element[0] == "table": curr_node_tables += 1
      self.summary_images_num += curr_node_images
      self.summary_tables_num += curr_node_tables
      # iterate children
      tree_iter = self.treestore.iter_children(tree_iter)
      while tree_iter != None:
         self.tree_info_iter(tree_iter)
         tree_iter = self.treestore.iter_next(tree_iter)
      
   def image_handle(self, *args):
      """Insert/Edit Image"""
      if not self.node_sel_and_rich_text(): return
      iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
      filename = support.dialog_file_select(curr_folder=self.pick_dir, parent=self.window)
      if filename == None: return
      self.pick_dir = os.path.dirname(filename)
      pixbuf = gtk.gdk.pixbuf_new_from_file(filename)
      self.image_edit_dialog(pixbuf, self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert()))
      
   def image_edit_dialog(self, pixbuf, insert_iter, iter_bound=None):
      """Insert/Edit Image Dialog"""
      self.original_pixbuf = pixbuf
      self.temp_image_width = pixbuf.get_width()
      self.temp_image_height = pixbuf.get_height()
      self.image_w_h_ration = float(self.temp_image_width)/self.temp_image_height
      self.image_load_into_dialog()
      response = self.glade.imageeditdialog.run()
      self.glade.imageeditdialog.hide()
      if response != 1: return # cancel was pressed
      if iter_bound != None: # only in case of modify
         image_justification = self.state_machine.get_iter_alignment(insert_iter)
         image_offset = insert_iter.get_offset()
         self.curr_buffer.delete(insert_iter, iter_bound)
         insert_iter = self.curr_buffer.get_iter_at_offset(image_offset)
      else: image_justification = None
      self.image_insert(insert_iter,
                        self.original_pixbuf.scale_simple(int(self.temp_image_width),
                                                          int(self.temp_image_height),
                                                          gtk.gdk.INTERP_BILINEAR),
                        image_justification)
      
   def image_insert(self, iter_insert, pixbuf, image_justification=None):
      image_offset = iter_insert.get_offset()
      anchor = self.curr_buffer.create_child_anchor(iter_insert)
      anchor.pixbuf = pixbuf
      anchor.eventbox = gtk.EventBox()
      anchor.eventbox.set_visible_window(False)
      if "anchor" in dir(pixbuf):
         anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_anchor, anchor)
         anchor.eventbox.set_tooltip_text(pixbuf.anchor)
      else:
         anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_image, anchor)
      anchor.image = gtk.Image()
      anchor.eventbox.add(anchor.image)
      anchor.image.set_from_pixbuf(anchor.pixbuf)
      self.sourceview.add_child_at_anchor(anchor.eventbox, anchor)
      anchor.eventbox.show_all()
      if image_justification:
         self.state_machine.apply_image_justification(self.curr_buffer.get_iter_at_offset(image_offset),
                                                      image_justification)
      else:
         # if I apply a justification, the state is already updated
         self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      
   def image_edit(self, *args):
      """Edit the selected Image"""
      iter_insert = self.curr_buffer.get_iter_at_child_anchor(self.curr_image_anchor)
      iter_bound = iter_insert.copy()
      iter_bound.forward_char()
      self.image_edit_dialog(self.curr_image_anchor.pixbuf, iter_insert, iter_bound)
      
   def image_save(self, *args):
      """Save to Disk the selected Image"""
      filename = support.dialog_file_save_as(curr_folder=self.pick_dir,
                                             filter_pattern="*.png",
                                             filter_name=_("PNG Image"),
                                             parent=self.window)
      if filename == None: return
      if len(filename) < 4 or filename[-4:] != ".png": filename += ".png"
      self.pick_dir = os.path.dirname(filename)
      try: self.curr_image_anchor.pixbuf.save(filename, "png")
      except: support.dialog_error(_("Write to %s Failed") % filename, self.window)
      
   def object_set_selection(self, anchor):
      """Put Selection Upon the Image"""
      iter_image = self.curr_buffer.get_iter_at_child_anchor(anchor)
      iter_bound = iter_image.copy()
      iter_bound.forward_char()
      self.curr_buffer.select_range(iter_image, iter_bound)
      
   def image_cut(self, *args):
      """Cut Image"""
      self.object_set_selection(self.curr_image_anchor)
      self.sourceview.emit("cut-clipboard")
   
   def image_copy(self, *args):
      """Copy Image"""
      self.object_set_selection(self.curr_image_anchor)
      self.sourceview.emit("copy-clipboard")
   
   def image_delete(self, *args):
      """Delete Image"""
      self.object_set_selection(self.curr_image_anchor)
      self.curr_buffer.delete_selection(True, self.sourceview.get_editable())
      self.sourceview.grab_focus()
      
   def on_mouse_button_clicked_image(self, widget, event, anchor):
      """Catches mouse buttons clicks upon images"""
      self.curr_image_anchor = anchor
      self.object_set_selection(self.curr_image_anchor)
      if event.button == 3:
         self.ui.get_widget("/ImageMenu").popup(None, None, None, event.button, event.time)
      elif event.type == gtk.gdk._2BUTTON_PRESS: self.image_edit()
         
   def on_mouse_button_clicked_anchor(self, widget, event, anchor):
      """Catches mouse buttons clicks upon images"""
      self.curr_anchor_anchor = anchor
      iter_anchor = self.curr_buffer.get_iter_at_child_anchor(self.curr_anchor_anchor)
      iter_bound = iter_anchor.copy()
      iter_bound.forward_char()
      self.curr_buffer.select_range(iter_anchor, iter_bound)
      if event.button == 3:
         self.ui.get_widget("/AnchorMenu").popup(None, None, None, event.button, event.time)
      elif event.type == gtk.gdk._2BUTTON_PRESS: self.anchor_edit()
      
   def image_load_into_dialog(self):
      """Load/Reload the Image Under Editing"""
      self.user_active = False
      self.glade.spinbutton_image_width.set_value(self.temp_image_width)
      self.glade.spinbutton_image_height.set_value(self.temp_image_height)
      if self.temp_image_width <= 900 and self.temp_image_height <= 600:
         # original size into the dialog
         pixbuf = self.original_pixbuf.scale_simple(int(self.temp_image_width),
                                                    int(self.temp_image_height),
                                                    gtk.gdk.INTERP_BILINEAR)
      else:
         # reduced size visible into the dialog
         if self.temp_image_width > 900:
            temp_image_width = 900
            temp_image_height = temp_image_width / self.image_w_h_ration
         else:
            temp_image_height = 600
            temp_image_width = temp_image_height * self.image_w_h_ration
         pixbuf = self.original_pixbuf.scale_simple(int(temp_image_width),
                                                    int(temp_image_height),
                                                    gtk.gdk.INTERP_BILINEAR)
      self.glade.image_under_editing.set_from_pixbuf(pixbuf)
      self.user_active = True
      
   def on_button_rotate_90_cw_clicked(self, *args):
      """Image Edit - Rotate 90 ClockWise"""
      self.original_pixbuf = self.original_pixbuf.rotate_simple(270)
      self.image_w_h_ration = 1/self.image_w_h_ration
      new_width = self.temp_image_height # new width is the former height and vice versa
      self.temp_image_height = self.temp_image_width
      self.temp_image_width = new_width
      self.image_load_into_dialog()
   
   def on_button_rotate_90_ccw_clicked(self, *args):
      """Image Edit - Rotate 90 CounterClockWise"""
      self.original_pixbuf = self.original_pixbuf.rotate_simple(90)
      self.image_w_h_ration = 1/self.image_w_h_ration
      new_width = self.temp_image_height # new width is the former height and vice versa
      self.temp_image_height = self.temp_image_width
      self.temp_image_width = new_width
      self.image_load_into_dialog()
   
   def on_spinbutton_image_width_value_changed(self, spinbutton):
      """Image Edit - Width Change Handling"""
      if self.user_active:
         self.temp_image_width = self.glade.spinbutton_image_width.get_value()
         self.temp_image_height = self.temp_image_width / self.image_w_h_ration
         self.image_load_into_dialog()
   
   def on_spinbutton_image_height_value_changed(self, spinbutton):
      """Image Edit - Height Change Handling"""
      if self.user_active:
         self.temp_image_height = self.glade.spinbutton_image_height.get_value()
         self.temp_image_width = self.temp_image_height * self.image_w_h_ration
         self.image_load_into_dialog()
      
   def apply_tag_foreground(self, *args):
      """The Foreground Color Chooser Button was Pressed"""
      self.apply_tag("foreground")
      
   def apply_tag_background(self, *args):
      """The Background Color Chooser Button was Pressed"""
      self.apply_tag("background")
      
   def apply_tag_link(self, *args):
      """The Link Insert Button was Pressed"""
      self.apply_tag("link")
      
   def apply_tag_bold(self, *args):
      """The Bold Button was Pressed"""
      self.apply_tag("weight", "heavy")
      
   def apply_tag_italic(self, *args):
      """The Italic Button was Pressed"""
      self.apply_tag("style", "italic")
      
   def apply_tag_underline(self, *args):
      """The Underline Button was Pressed"""
      self.apply_tag("underline", "single")
      
   def apply_tag_strikethrough(self, *args):
      """The Strikethrough Button was Pressed"""
      self.apply_tag("strikethrough", "true")
      
   def apply_tag_small(self, *args):
      """The Small Button was Pressed"""
      self.apply_tag("scale", "small")
   
   def apply_tag_large(self, *args):
      """The H1 Button was Pressed"""
      self.apply_tag("scale", "h1")
      
   def apply_tag_large2(self, *args):
      """The H2 Button was Pressed"""
      self.apply_tag("scale", "h2")
      
   def apply_tag_justify_right(self, *args):
      """The Justify Right Button was Pressed"""
      iter_start, iter_end = self.lists_handler.get_paragraph_iters()
      self.apply_tag("justification", "right", iter_sel_start=iter_start, iter_sel_end=iter_end)
   
   def apply_tag_justify_left(self, *args):
      """The Justify Left Button was Pressed"""
      iter_start, iter_end = self.lists_handler.get_paragraph_iters()
      self.apply_tag("justification", "left", iter_sel_start=iter_start, iter_sel_end=iter_end)
   
   def apply_tag_justify_center(self, *args):
      """The Justify Center Button was Pressed"""
      iter_start, iter_end = self.lists_handler.get_paragraph_iters()
      self.apply_tag("justification", "center", iter_sel_start=iter_start, iter_sel_end=iter_end)
      
   def apply_tag_try_automatic_bounds(self):
      """Try to Select a Word Forward/Backward the Cursor"""
      iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
      iter_end = iter_start.copy()
      end_moved = False
      while iter_end != None:
         char = iter_end.get_char()
         match = re.match('\w', char) # alphanumeric char
         if not match: break # we got it
         elif not iter_end.forward_char(): break # we reached the buffer end
         end_moved = True
      if not end_moved: iter_start.backward_char() # we could be at the end of a word
      while iter_start != None:
         char = iter_start.get_char()
         match = re.match('\w', char) # alphanumeric char
         if not match: # we got it
            iter_start.forward_char() # step forward to the beginning of the word
            break
         elif not iter_start.backward_char(): break # we reached the buffer start
      if iter_start.equal(iter_end): return False
      else:
         self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_end)
         self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), iter_start)
         return True
      
   def list_bulleted_handler(self, *args):
      """Handler of the Bulleted List"""
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
         support.dialog_warning(_("Automatic Syntax Highlighting Must be Disabled in order to Use This Feature"), self.window)
         return
      self.lists_handler.list_bulleted_handler()
      
   def list_numbered_handler(self, *args):
      """Handler of the Numbered List"""
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
         support.dialog_warning(_("Automatic Syntax Highlighting Must be Disabled in order to Use This Feature"), self.window)
         return
      self.lists_handler.list_numbered_handler()
      
   def apply_tag_latest(self, *args):
      """The Iterate Tagging Button was Pressed"""
      if self.latest_tag[0] == "": support.dialog_warning(_("No Previous Text Format Was Performed During This Session"), self.window)
      else: self.apply_tag(*self.latest_tag)
      
   def apply_tag(self, tag_property, property_value=None, iter_sel_start=None, iter_sel_end=None):
      """Apply a tag"""
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID and self.user_active:
         support.dialog_warning(_("Automatic Syntax Highlighting Must be Disabled in order to Use This Feature"), self.window)
         return
      if iter_sel_start == None and iter_sel_end == None:
         if tag_property != "justification":
            if self.curr_tree_iter == None:
               support.dialog_warning(_("No Node is Selected"), self.window)
               return
            if tag_property == "link": link_node_id = None
            if not self.curr_buffer.get_has_selection():
               if tag_property != "link":
                  if not self.apply_tag_try_automatic_bounds():
                     support.dialog_warning(_("No Text is Selected"), self.window)
                     return
               else:
                  tag_property_value = self.link_check_around_cursor()
                  if tag_property_value == "":
                     support.dialog_warning(_("No Text is Selected"), self.window)
                     return
                  vector = tag_property_value.split()
                  self.link_type = vector[0]
                  if self.link_type == "webs": self.glade.link_website_entry.set_text(vector[1])
                  elif self.link_type == "file": self.glade.entry_file_to_link_to.set_text(base64.b64decode(vector[1]))
                  elif self.link_type == "node":
                     link_node_id = int(vector[1])
                     if len(vector) == 3: self.glade.link_anchor_entry.set_text(vector[2])
                  else:
                     support.dialog_error("Tag Name Not Recognized! (%s)" % self.link_type, self.window)
                     self.link_type = "webs"
                     return
                  self.glade.radiobutton_link_website.set_active(self.link_type == "webs")
                  self.glade.radiobutton_link_node_anchor.set_active(self.link_type == "node")
                  self.glade.radiobutton_link_file.set_active(self.link_type == "file")
            iter_sel_start, iter_sel_end = self.curr_buffer.get_selection_bounds()
         else:
            support.dialog_warning(_("The Cursor is Not into a Paragraph"), self.window)
            return
      if property_value == None:
         if tag_property == "link":
            if self.next_chars_from_iter_are(iter_sel_start, 7, "http://")\
            or self.next_chars_from_iter_are(iter_sel_start, 8, "https://"):
               self.link_type == "webs"
               self.glade.link_website_entry.set_text(self.curr_buffer.get_text(iter_sel_start, iter_sel_end))
            self.node_choose_view_exist_or_create(link_node_id)
            self.glade.choosenodedialog.set_title(_("Insert/Edit a Link"))
            self.glade.link_dialog_top_vbox.show()
            self.glade.frame_link_anchor.show()
            self.glade.frame_link_website_url.set_sensitive(self.link_type == "webs")
            self.glade.hbox_link_node_anchor.set_sensitive(self.link_type == "node")
            self.glade.frame_link_filepath.set_sensitive(self.link_type == "file")
            response = self.glade.choosenodedialog.run()
            self.glade.choosenodedialog.hide()
            if response != 1: return # the user aborted the operation
            if self.link_type == "webs":
               link_url = self.glade.link_website_entry.get_text().strip()
               if len(link_url) < 8\
               or (link_url[0:7] != "http://" and link_url[0:8] != "https://"):
                  link_url = "http://" + link_url
               property_value = "webs" + cons.CHAR_SPACE + link_url
            elif self.link_type == "file":
               link_uri = self.glade.entry_file_to_link_to.get_text().strip()
               link_uri = base64.b64encode(link_uri)
               property_value = "file" + cons.CHAR_SPACE + link_uri
            elif self.link_type == "node":
               model, tree_iter = self.treeviewselection_2.get_selected()
               link_anchor = self.glade.link_anchor_entry.get_text().strip()
               property_value = "node" + cons.CHAR_SPACE + str(self.treestore[tree_iter][3])
               if len(link_anchor) > 0: property_value += cons.CHAR_SPACE + link_anchor
         else:
            response = self.glade.colorselectiondialog.run()
            self.glade.colorselectiondialog.hide()
            if response == 2: return # cancel was clicked
            property_value = self.glade.colorselection.get_current_color().to_string()
      if tag_property != "link":
         self.latest_tag = [tag_property, property_value]
      curr_tags = iter_sel_start.get_tags()
      # if there's already a tag about this property, we remove it before apply the new one
      for curr_tag in curr_tags:
         tag_name = curr_tag.get_property("name")
         if (tag_property == "weight" and tag_name[0:7] == "weight_")\
         or (tag_property == "style" and tag_name[0:6] == "style_")\
         or (tag_property == "underline" and tag_name[0:10] == "underline_")\
         or (tag_property == "strikethrough" and tag_name[0:14] == "strikethrough_"):
            self.curr_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
            return # just tag removal
         elif tag_property == "scale" and tag_name[0:6] == "scale_":
            self.curr_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
            #print property_value, tag_name[-2:]
            if property_value == tag_name[-2:]: return # just tag removal
         elif tag_property == "justification" and tag_name[0:14] == "justification_":
            self.curr_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
         elif (tag_property == "foreground" and tag_name[0:11] == "foreground_")\
         or (tag_property == "background" and tag_name[0:11] == "background_")\
         or (tag_property == "link" and tag_name[0:5] == "link_"):
            self.curr_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
      self.curr_buffer.apply_tag_by_name(self.apply_tag_exist_or_create(tag_property, property_value),
                                         iter_sel_start, iter_sel_end)
      if self.user_active:
         if self.file_update == False: self.update_window_save_needed() # file save needed
         self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      
   def next_chars_from_iter_are(self, iter_start, num, chars):
      """Returns True if the Given Chars are the next 'num' after iter"""
      iter = iter_start.copy()
      for i in range(num):
         if iter.get_char() != chars[i]: return False
         if i != num-1 and not iter.forward_char(): return False
      return True
      
   def apply_tag_exist_or_create(self, tag_property, property_value):
      """Check into the Tags Table whether the Tag Exists, if Not Creates it"""
      if property_value == "large": property_value = "h1"
      elif property_value == "largo": property_value = "h2"
      tag_name = tag_property + "_" + property_value
      tag = self.tag_table.lookup(str(tag_name))
      if tag == None:
         tag = gtk.TextTag(str(tag_name))
         if property_value == "heavy": tag.set_property(tag_property, pango.WEIGHT_HEAVY)
         elif property_value == "small": tag.set_property(tag_property, pango.SCALE_X_SMALL)
         elif property_value == "h1": tag.set_property(tag_property, pango.SCALE_XX_LARGE)
         elif property_value == "h2": tag.set_property(tag_property, pango.SCALE_X_LARGE)
         elif property_value == "italic": tag.set_property(tag_property, pango.STYLE_ITALIC)
         elif property_value == "single": tag.set_property(tag_property, pango.UNDERLINE_SINGLE)
         elif property_value == "true": tag.set_property(tag_property, True)
         elif property_value == "left": tag.set_property(tag_property, gtk.JUSTIFY_LEFT)
         elif property_value == "right": tag.set_property(tag_property, gtk.JUSTIFY_RIGHT)
         elif property_value == "center": tag.set_property(tag_property, gtk.JUSTIFY_CENTER)
         elif property_value[0:4] == "webs":
            tag.set_property("underline", pango.UNDERLINE_SINGLE)
            tag.set_property("foreground", "#00000000ffff")
         elif property_value[0:4] == "node":
            tag.set_property("underline", pango.UNDERLINE_SINGLE)
            tag.set_property("foreground", "#071c838e071c")
         elif property_value[0:4] == "file":
            tag.set_property("underline", pango.UNDERLINE_SINGLE)
            tag.set_property("foreground", "#8b8b69691414")
         else: tag.set_property(tag_property, property_value)
         self.tag_table.add(tag)
      return str(tag_name)
      
   def link_check_around_cursor(self):
      """Check if the cursor is on a link, in this case select the link and return the tag_property_value"""
      iter = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
      tags = iter.get_tags()
      for tag in tags:
         tag_name = tag.get_property("name")
         if tag_name[0:4] == "link": break
      else: return ""
      iter_end = iter.copy()
      if not iter_end.forward_to_tag_toggle(tag): return ""
      if not iter.backward_to_tag_toggle(tag): return ""
      self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_end)
      self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), iter)
      return tag_name[5:]
      
   def link_clicked(self, tag_property_value):
      """Function Called at Every Link Click"""
      vector = tag_property_value.split()
      if vector[0] == "webs":
         # link to webpage
         if self.weblink_custom_action[0]:
            subprocess.call(self.weblink_custom_action[1] % vector[1], shell=True)
         else: webbrowser.open(vector[1])
      elif vector[0] == "file":
         # link to file
         filepath = base64.b64decode(vector[1])
         if not os.path.isfile(filepath):
            support.dialog_error(_("The File Link '%s' is Not Valid") % filepath, self.window)
            return
         if sys.platform[0:3] == "win": os.startfile(filepath)
         else: subprocess.call("xdg-open %s" % re.escape(filepath), shell=True)
      elif vector[0] == "node":
         # link to a tree node
         tree_iter = self.get_tree_iter_from_node_id(int(vector[1]))
         if tree_iter == None:
            support.dialog_error(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)") % vector[1], self.window)
            return
         self.treeview_safe_set_cursor(tree_iter)
         self.sourceview.grab_focus()
         self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(None)
         self.sourceview.set_tooltip_text(None)
         if len(vector) == 3:
            iter_anchor = self.link_seek_for_anchor(vector[2])
            if iter_anchor == None: support.dialog_warning(_("No anchor named '%s' found") % vector[2], self.window)
            else:
               self.curr_buffer.place_cursor(iter_anchor)
               self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
      else: support.dialog_error("Tag Name Not Recognized! (%s)" % vector[0], self.window)
      
   def on_button_browse_for_file_to_link_to_clicked(self, *args):
      """The Button to browse for a file path on the links dialog was pressed"""
      filepath = support.dialog_file_select(curr_folder=self.pick_dir, parent=self.window)
      if filepath == None: return
      self.pick_dir = os.path.dirname(filepath)
      self.glade.entry_file_to_link_to.set_text(filepath)
      
   def link_seek_for_anchor(self, anchor_name):
      """Given an Anchor Name, Seeks for it in the Current Node and Returns the Iter or None"""
      curr_iter = self.curr_buffer.get_start_iter()
      while 1:
         anchor = curr_iter.get_child_anchor()
         if anchor != None:
            if "pixbuf" in dir(anchor) and "anchor" in dir(anchor.pixbuf) and anchor.pixbuf.anchor == anchor_name:
               return curr_iter
         if not curr_iter.forward_char(): break
      return None
      
   def on_browse_anchors_button_clicked(self, *args):
      """Browse for Existing Anchors on the Selected Node"""
      self.anchors_liststore_exist_or_create()
      self.anchors_liststore.clear()
      model, tree_iter = self.treeviewselection_2.get_selected()
      curr_iter = self.treestore[tree_iter][2].get_start_iter()
      while 1:
         anchor = curr_iter.get_child_anchor()
         if anchor != None:
            if "pixbuf" in dir(anchor) and "anchor" in dir(anchor.pixbuf):
               self.anchors_liststore.append([anchor.pixbuf.anchor])
         if not curr_iter.forward_char(): break
      anchor_first_iter = self.anchors_liststore.get_iter_first()
      if anchor_first_iter == None:
         support.dialog_info(_("There are No Anchors in the Selected Node"), self.window)
         return
      else: self.anchors_treeview.set_cursor(self.anchors_liststore.get_path(anchor_first_iter))
      self.glade.anchor_enter_name_hbox.hide()
      self.glade.scrolledwindow_anchors_list.show()
      self.glade.anchorhandledialog.set_title(_("Choose Existing Anchor"))
      response = self.glade.anchorhandledialog.run()
      self.glade.anchorhandledialog.hide()
      if response != 1: return # the user aborted the operation
      listmodel, listiter = self.anchors_treeviewselection.get_selected()
      self.glade.link_anchor_entry.set_text(self.anchors_liststore[listiter][0])
      
   def anchors_liststore_exist_or_create(self):
      """If Does Not Exist => Create Anchors Browser Liststore"""
      if not "anchors_liststore" in dir(self):
         self.anchors_liststore = gtk.ListStore(str)
         self.anchors_treeview = gtk.TreeView(self.anchors_liststore)
         self.anchors_renderer_text = gtk.CellRendererText()
         self.anchors_column = gtk.TreeViewColumn(_("Anchor Name"), self.anchors_renderer_text, text=0)
         self.anchors_treeview.append_column(self.anchors_column)
         self.anchors_treeviewselection = self.anchors_treeview.get_selection()
         self.anchors_treeview.connect('button-press-event', self.on_mouse_button_clicked_anchors_list)
         self.glade.scrolledwindow_anchors_list.add(self.anchors_treeview)
         self.glade.scrolledwindow_anchors_list.show_all()
      
   def on_mouse_button_clicked_anchors_list(self, widget, event):
      """Catches mouse buttons clicks"""
      if event.button != 1: return
      if event.type == gtk.gdk._2BUTTON_PRESS: self.glade.anchorhandledialog_button_ok.clicked()
      
   def sourceview_cursor_and_tooltips_handler(self, x, y):
      """Looks at all tags covering the position (x, y) in the text view,
         and if one of them is a link, change the cursor to the HAND2 cursor"""
      hovering_link = False
      hovering_anchor = False
      iter = self.sourceview.get_iter_at_location(x, y)
      tags = iter.get_tags()
      for tag in tags:
         tag_name = tag.get_property("name")
         if tag_name[0:4] == "link":
            hovering_link = True
            vector = tag_name[5:].split()
            if vector[0] == "file": tooltip = base64.b64decode(vector[1])
            else:
               tooltip = vector[1]
               if len(vector) == 3: tooltip += "#" + vector[2]
            break
      else:
         iter_anchor = iter.copy()
         pixbuf = iter_anchor.get_pixbuf()
         if pixbuf != None and "anchor" in dir(pixbuf): hovering_anchor = True
         else:
            iter_anchor.backward_char()
            pixbuf = iter_anchor.get_pixbuf()
            if pixbuf != None and "anchor" in dir(pixbuf): hovering_anchor = True
      if hovering_link != self.hovering_over_link: self.hovering_over_link = hovering_link
      if hovering_anchor:
         self.sourceview.set_tooltip_text(pixbuf.anchor)
         return
      if self.hovering_over_link:
         self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(gtk.gdk.Cursor(gtk.gdk.HAND2))
         self.sourceview.set_tooltip_text(tooltip)
      else:
         self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(None)
         self.sourceview.set_tooltip_text(None)
   
   def on_sourceview_event_after(self, text_view, event):
      """Called after every event on the SourceView"""
      if event.type == gtk.gdk._2BUTTON_PRESS and event.button == 1:
         x, y = text_view.window_to_buffer_coords(gtk.TEXT_WINDOW_WIDGET, int(event.x), int(event.y))
         iter_end = text_view.get_iter_at_location(x, y)
         iter_start = iter_end.copy()
         match = re.match('\w', iter_end.get_char()) # alphanumeric char
         if not match: return # double-click was not upon alphanumeric
         while match:
            if not iter_end.forward_char(): break # end of buffer
            match = re.match('\w', iter_end.get_char()) # alphanumeric char
         iter_start.backward_char()
         match = re.match('\w', iter_start.get_char()) # alphanumeric char
         while match:
            if not iter_start.backward_char(): break # start of buffer
            match = re.match('\w', iter_start.get_char()) # alphanumeric char
         if not match: iter_start.forward_char()
         self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_start)
         self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), iter_end)
      elif self.syntax_highlighting != cons.CUSTOM_COLORS_ID: return
      if event.type == gtk.gdk.BUTTON_PRESS:
         if event.button == 1:
            x, y = text_view.window_to_buffer_coords(gtk.TEXT_WINDOW_WIDGET, int(event.x), int(event.y))
            iter = self.sourceview.get_iter_at_location(x, y)
            tags = iter.get_tags()
            # check whether we are hovering a link
            for tag in tags:
               tag_name = tag.get_property("name")
               if tag_name[0:4] == "link":
                  self.link_clicked(tag_name[5:])
                  break
         elif event.button == 3 and not self.curr_buffer.get_has_selection():
            x, y = text_view.window_to_buffer_coords(gtk.TEXT_WINDOW_WIDGET, int(event.x), int(event.y))
            iter = self.sourceview.get_iter_at_location(x, y)
            self.curr_buffer.place_cursor(iter)
      elif event.type == gtk.gdk.KEY_PRESS:
         keyname = gtk.gdk.keyval_name(event.keyval)
         if (event.state & gtk.gdk.SHIFT_MASK): # Shift held down
            if keyname == "Return":
               self.curr_buffer.insert(self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert()), '   ')
         elif keyname == "Return":
            iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
            if iter_insert == None: return False
            iter_start = iter_insert.copy()
            if iter_start.backward_chars(2) and iter_start.get_char() == cons.CHAR_NEWLINE: return False # former was an empty row
            list_info = self.lists_handler.get_paragraph_list_info(iter_start)
            if list_info[0] == None: return False # former was not a list
            # possible list quit
            iter_list_quit = iter_insert.copy()
            if (list_info[0] == 0 and iter_list_quit.backward_chars(3) and iter_list_quit.get_char() == cons.CHAR_LISTBUL):
               self.curr_buffer.delete(iter_list_quit, iter_insert)
               return False # former was an empty paragraph => list quit
            elif (list_info[0] > 0 and iter_list_quit.backward_chars(2) and iter_list_quit.get_char() == cons.CHAR_SPACE\
            and iter_list_quit.backward_char() and iter_list_quit.get_char() == '.'):
               iter_list_quit.backward_chars(len(str(list_info[0])))
               self.curr_buffer.delete(iter_list_quit, iter_insert)
               return False # former was an empty paragraph => list quit
            if list_info[1]: # multiple line paragraph
               iter_start = iter_insert.copy()
               iter_start.backward_chars(3)
               self.curr_buffer.delete(iter_start, iter_insert)
            if list_info[0] == 0: self.curr_buffer.insert(iter_insert, cons.CHAR_LISTBUL + cons.CHAR_SPACE)
            else:
               curr_num = list_info[0] + 1
               self.curr_buffer.insert(iter_insert, '%s. ' % curr_num)
               self.lists_handler.list_adjust_ahead(curr_num, iter_insert.get_offset(), "num2num")
      return False
   
   def on_sourceview_motion_notify_event(self, text_view, event):
      """Update the cursor image if the pointer moved"""
      if not self.sourceview.get_cursor_visible(): self.sourceview.set_cursor_visible(True)
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID: return
      x, y = self.sourceview.window_to_buffer_coords(gtk.TEXT_WINDOW_WIDGET, int(event.x), int(event.y))
      self.sourceview_cursor_and_tooltips_handler(x, y)
      return False
   
   def on_sourceview_visibility_notify_event(self, text_view, event):
      """Update the cursor image if the window becomes visible (e.g. when a window covering it got iconified)"""
      if self.syntax_highlighting != cons.CUSTOM_COLORS_ID: return
      wx, wy, mod = self.sourceview.window.get_pointer()
      bx, by = self.sourceview.window_to_buffer_coords(gtk.TEXT_WINDOW_WIDGET, wx, wy)
      self.sourceview_cursor_and_tooltips_handler(bx, by)
      return False
   
   def on_window_n_tree_size_allocate_event(self, widget, allocation):
      """New Size Allocated"""
      if not self.curr_window_n_tree_width:
         self.curr_window_n_tree_width = {'window_width': self.window.get_allocation().width,
                                          'tree_width': self.scrolledwindow_tree.get_allocation().width}
      else:
         if not self.curr_buffer: return
         if self.curr_window_n_tree_width['window_width'] != self.window.get_allocation().width\
         or self.curr_window_n_tree_width['tree_width'] != self.scrolledwindow_tree.get_allocation().width:
            self.curr_window_n_tree_width['window_width'] = self.window.get_allocation().width
            self.curr_window_n_tree_width['tree_width'] = self.scrolledwindow_tree.get_allocation().width
            curr_iter = self.curr_buffer.get_start_iter()
            while 1:
               anchor = curr_iter.get_child_anchor()
               if anchor != None and "width_in_pixels" in dir(anchor) and not anchor.width_in_pixels:
                  self.codeboxes_handler.codebox_apply_width_height(anchor)
               if not curr_iter.forward_char(): break
            
   def get_text_window_width(self):
      """Get the Text Window Width"""
      return (self.window.get_allocation().width -\
              self.scrolledwindow_tree.get_allocation().width-\
              cons.MAIN_WIN_TO_TEXT_WIN_NORMALIZER)
   
   def remove_text_formatting(self, *args):
      """Cleans the Selected Text from All Formatting Tags"""
      if not self.node_sel_and_rich_text(): return
      if not self.curr_buffer.get_has_selection() and not self.apply_tag_try_automatic_bounds():
         support.dialog_warning(_("No Text is Selected"), self.window)
         return
      iter_sel_start, iter_sel_end = self.curr_buffer.get_selection_bounds()
      self.curr_buffer.remove_all_tags(iter_sel_start, iter_sel_end)
      self.update_window_save_needed()
      self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
      
   def tree_is_empty(self):
      """Return True if the treestore is empty"""
      if self.treestore.get_iter_first() == None: return True
      else: return False
