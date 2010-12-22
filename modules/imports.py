# -*- coding: UTF-8 -*-
#
#       imports.py
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

import HTMLParser, htmlentitydefs
import gtk, os, xml.dom.minidom, re, base64
import cons, machines


class TuxCardsHandler(HTMLParser.HTMLParser):
   """The Handler of the TuxCards File Parsing"""
   
   def __init__(self):
      """Machine boot"""
      HTMLParser.HTMLParser.__init__(self)
      self.xml_handler = machines.XMLHandler(self)
      
   def rich_text_serialize(self, text_data):
      """Appends a new part to the XML rich text"""
      dom_iter = self.dom.createElement("rich_text")
      for tag_property in cons.TAG_PROPERTIES:
         if self.curr_attributes[tag_property] != "":
            dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
      self.nodes_list[-1].appendChild(dom_iter)
      text_iter = self.dom.createTextNode(text_data)
      dom_iter.appendChild(text_iter)

   def start_parsing(self, tuxcards_string):
      """Start the Parsing"""
      dom = xml.dom.minidom.parseString(tuxcards_string)
      dom_iter = dom.firstChild
      while dom_iter:
         if dom_iter.nodeName == "InformationCollection": break
         dom_iter = dom_iter.nextSibling
      child_dom_iter = dom_iter.firstChild
      while child_dom_iter:
         if child_dom_iter.nodeName == "InformationElement": break
         child_dom_iter = child_dom_iter.nextSibling
      self.node_add(child_dom_iter)
   
   def node_add(self, dom_iter):
      """Add a Node"""
      child_dom_iter = dom_iter.firstChild
      self.nodes_list.append(self.dom.createElement("node"))
      while child_dom_iter:
         if child_dom_iter.nodeName == "Description":
            if child_dom_iter.firstChild: node_name = child_dom_iter.firstChild.data
            else: node_name = ""
            self.nodes_list[-1].setAttribute("name", node_name)
            self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
            self.nodes_list[-2].appendChild(self.nodes_list[-1])
         elif child_dom_iter.nodeName == "Information":
            if child_dom_iter.firstChild: node_string = child_dom_iter.firstChild.data
            else: node_string = ""
            self.curr_state = 0
            # curr_state 0: standby, taking no data
            # curr_state 1: waiting for node content, take many data
            self.pixbuf_vector = []
            self.chars_counter = 0
            self.feed(node_string.decode("utf-8", "ignore"))
            for pixbuf_element in self.pixbuf_vector:
               self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
         elif child_dom_iter.nodeName == "InformationElement":
            self.node_add(child_dom_iter)
         child_dom_iter = child_dom_iter.nextSibling
      self.nodes_list.pop()
   
   def handle_starttag(self, tag, attrs):
      """Encountered the beginning of a tag"""
      if self.curr_state == 0:
         if tag == "body": self.curr_state = 1
      else: # self.curr_state == 1
         if tag == "span" and attrs[0][0] == "style":
            if "font-weight" in attrs[0][1]: self.curr_attributes["weight"] = "heavy"
            elif "font-style" in attrs[0][1] and "italic" in attrs[0][1]: self.curr_attributes["style"] = "italic"
            elif "text-decoration" in attrs[0][1] and "underline" in attrs[0][1]: self.curr_attributes["underline"] = "single"
            elif "color" in attrs[0][1]:
               match = re.match("(?<=^).+:(.+);(?=$)", attrs[0][1])
               if match != None: self.curr_attributes["foreground"] = match.group(1).strip()
         elif tag == "a" and len(attrs) > 0:
            link_url = attrs[0][1]
            if len(link_url) > 7:
               if link_url[0:4] == "http": self.curr_attributes["link"] = "webs %s" % link_url
               elif link_url[0:7] == "file://": self.curr_attributes["link"] = "file %s" % base64.b64encode(link_url[7:])
               else: self.curr_attributes["link"] = "webs %s" % ("http://" + link_url)
         elif tag == "img" and len(attrs) > 0:
            img_path = attrs[0][1]
            if os.path.isfile(img_path):
               pixbuf = gtk.gdk.pixbuf_new_from_file(img_path)
               self.pixbuf_vector.append([self.chars_counter, pixbuf, "left"])
               self.chars_counter += 1
            else: print "%s not found" % img_path
         elif tag == "br":
            # this is a data block composed only by an endline
            self.rich_text_serialize("\n")
            self.chars_counter += 1
         elif tag == "hr":
            # this is a data block composed only by an horizontal rule
            self.rich_text_serialize(cons.HORIZONTAL_RULE)
            self.chars_counter += len(cons.HORIZONTAL_RULE)
         elif tag == "li":
            self.rich_text_serialize("\n• ")
            self.chars_counter += 3
   
   def handle_endtag(self, tag):
      """Encountered the end of a tag"""
      if self.curr_state == 0: return
      if tag == "p":
         # this is a data block composed only by an endline
         self.rich_text_serialize("\n")
         self.chars_counter += 1
      elif tag == "span":
         self.curr_attributes["weight"] = ""
         self.curr_attributes["style"] = ""
         self.curr_attributes["underline"] = ""
         self.curr_attributes["foreground"] = ""
      elif tag == "a": self.curr_attributes["link"] = ""
      
   def handle_data(self, data):
      """Found Data"""
      if self.curr_state == 0 or data in ['\n', '\n\n']: return
      data = data.replace("\n", "")
      self.rich_text_serialize(data)
      self.chars_counter += len(data)
      
   def handle_entityref(self, name):
      """Found Entity Reference like &name;"""
      if self.curr_state == 0: return
      if name in htmlentitydefs.name2codepoint:
         unicode_char = unichr(htmlentitydefs.name2codepoint[name])
         self.rich_text_serialize(unicode_char)
         self.chars_counter += 1
   
   def get_cherrytree_xml(self, tuxcards_string):
      """Returns a CherryTree string Containing the TuxCards Nodes"""
      self.dom = xml.dom.minidom.Document()
      self.nodes_list = [self.dom.createElement("cherrytree")]
      self.dom.appendChild(self.nodes_list[0])
      self.curr_attributes = {}
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      self.latest_span = ""
      self.start_parsing(tuxcards_string)
      return self.dom.toxml()


class KeepnoteHandler(HTMLParser.HTMLParser):
   """The Handler of the KeepNote Folder Parsing"""
   
   def __init__(self, folderpath):
      """Machine boot"""
      HTMLParser.HTMLParser.__init__(self)
      self.folderpath = folderpath
      self.xml_handler = machines.XMLHandler(self)
   
   def rich_text_serialize(self, text_data):
      """Appends a new part to the XML rich text"""
      dom_iter = self.dom.createElement("rich_text")
      for tag_property in cons.TAG_PROPERTIES:
         if self.curr_attributes[tag_property] != "":
            dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
      self.nodes_list[-1].appendChild(dom_iter)
      text_iter = self.dom.createTextNode(text_data)
      dom_iter.appendChild(text_iter)
   
   def start_parsing(self):
      """Start the Parsing"""
      for element in reversed(os.listdir(self.folderpath)):
         if os.path.isdir(os.path.join(self.folderpath, element))\
         and element not in ["__TRASH__", "__NOTEBOOK__"]:
            self.node_add(os.path.join(self.folderpath, element))
   
   def node_add(self, node_folder):
      """Add a Node"""
      self.nodes_list.append(self.dom.createElement("node"))
      self.nodes_list[-1].setAttribute("name", os.path.basename(node_folder))
      self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
      self.nodes_list[-2].appendChild(self.nodes_list[-1])
      filepath = os.path.join(node_folder, "page.html")
      if os.path.isfile(filepath):
         try:
            file_descriptor = open(filepath, 'r')
            node_string = file_descriptor.read()
            file_descriptor.close()
         except:
            print "Error opening the file %s" % filepath
            return
      else: node_string = "" # empty node
      self.curr_state = 0
      # curr_state 0: standby, taking no data
      # curr_state 1: waiting for node content, take many data
      self.pixbuf_vector = []
      self.curr_folder = node_folder
      self.chars_counter = 0
      self.feed(node_string.decode("utf-8", "ignore"))
      for pixbuf_element in self.pixbuf_vector:
         self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
      # check if the node has children
      for element in reversed(os.listdir(node_folder)):
         if os.path.isdir(os.path.join(node_folder, element)):
            self.node_add(os.path.join(node_folder, element))
      self.nodes_list.pop()
   
   def handle_starttag(self, tag, attrs):
      """Encountered the beginning of a tag"""
      if self.curr_state == 0:
         if tag == "body": self.curr_state = 1
      else: # self.curr_state == 1
         if tag == "b": self.curr_attributes["weight"] = "heavy"
         elif tag == "i": self.curr_attributes["style"] = "italic"
         elif tag == "u": self.curr_attributes["underline"] = "single"
         elif tag == "strike": self.curr_attributes["strikethrough"] = "true"
         elif tag == "span" and attrs[0][0] == "style":
            match = re.match("(?<=^)(.+):(.+)(?=$)", attrs[0][1])
            if match != None:
               if match.group(1) == "color":
                  self.curr_attributes["foreground"] = match.group(2).strip()
                  self.latest_span = "foreground"
               elif match.group(1) == "background-color":
                  self.curr_attributes["background"] = match.group(2).strip()
                  self.latest_span = "background"
         elif tag == "a" and len(attrs) > 0:
            link_url = attrs[0][1]
            if len(link_url) > 7:
               if link_url[0:4] == "http": self.curr_attributes["link"] = "webs %s" % link_url
               elif link_url[0:7] == "file://": self.curr_attributes["link"] = "file %s" % base64.b64encode(link_url[7:])
               else: self.curr_attributes["link"] = "webs %s" % ("http://" + link_url)
         elif tag == "img" and len(attrs) > 0:
            img_name = attrs[0][1]
            img_path = os.path.join(self.curr_folder, img_name)
            if os.path.isfile(img_path):
               pixbuf = gtk.gdk.pixbuf_new_from_file(img_path)
               self.pixbuf_vector.append([self.chars_counter, pixbuf, "left"])
               self.chars_counter += 1
            else: print "%s not found" % img_path
         elif tag == "br":
            # this is a data block composed only by an endline
            self.rich_text_serialize("\n")
            self.chars_counter += 1
         elif tag == "hr":
            # this is a data block composed only by an horizontal rule
            self.rich_text_serialize(cons.HORIZONTAL_RULE)
            self.chars_counter += len(cons.HORIZONTAL_RULE)
         elif tag == "li":
            self.rich_text_serialize("\n• ")
            self.chars_counter += 3
   
   def handle_endtag(self, tag):
      """Encountered the end of a tag"""
      if self.curr_state == 0: return
      if tag == "b": self.curr_attributes["weight"] = ""
      elif tag == "i": self.curr_attributes["style"] = ""
      elif tag == "u": self.curr_attributes["underline"] = ""
      elif tag == "strike": self.curr_attributes["strikethrough"] = ""
      elif tag == "span":
         if self.latest_span == "foreground": self.curr_attributes["foreground"] = ""
         elif self.latest_span == "background": self.curr_attributes["background"] = ""
      elif tag == "a": self.curr_attributes["link"] = ""
      
   def handle_data(self, data):
      """Found Data"""
      if self.curr_state == 0 or data in ['\n', '\n\n']: return
      data = data.replace("\n", "")
      self.rich_text_serialize(data)
      self.chars_counter += len(data)
      
   def handle_entityref(self, name):
      """Found Entity Reference like &name;"""
      if self.curr_state == 0: return
      if name in htmlentitydefs.name2codepoint:
         unicode_char = unichr(htmlentitydefs.name2codepoint[name])
         self.rich_text_serialize(unicode_char)
         self.chars_counter += 1
   
   def get_cherrytree_xml(self):
      """Returns a CherryTree string Containing the KeepNote Nodes"""
      self.dom = xml.dom.minidom.Document()
      self.nodes_list = [self.dom.createElement("cherrytree")]
      self.dom.appendChild(self.nodes_list[0])
      self.curr_attributes = {}
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      self.latest_span = ""
      self.start_parsing()
      return self.dom.toxml()


class BasketHandler(HTMLParser.HTMLParser):
   """The Handler of the Basket Folder Parsing"""
   
   def __init__(self, folderpath):
      """Machine boot"""
      HTMLParser.HTMLParser.__init__(self)
      self.folderpath = folderpath
      self.xml_handler = machines.XMLHandler(self)
   
   def check_basket_structure(self):
      """Check the Selected Folder to be a Basket Folder"""
      self.baskets_xml_filepath = os.path.join(self.folderpath, "baskets.xml")
      return os.path.isfile(self.baskets_xml_filepath)
   
   def rich_text_serialize(self, text_data):
      """Appends a new part to the XML rich text"""
      dom_iter = self.dom.createElement("rich_text")
      for tag_property in cons.TAG_PROPERTIES:
         if self.curr_attributes[tag_property] != "":
            dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
      self.nodes_list[-1].appendChild(dom_iter)
      text_iter = self.dom.createTextNode(text_data)
      dom_iter.appendChild(text_iter)
   
   def start_parsing(self):
      """Start the Parsing"""
      file_descriptor = open(self.baskets_xml_filepath, 'r')
      baskets_xml_string = file_descriptor.read()
      file_descriptor.close()
      dom = xml.dom.minidom.parseString(baskets_xml_string)
      dom_iter = dom.firstChild
      while dom_iter.firstChild == None:
         dom_iter = dom_iter.nextSibling
      child_dom_iter = dom_iter.firstChild
      while child_dom_iter:
         if child_dom_iter.nodeName == "basket": self.node_add(child_dom_iter)
         child_dom_iter = child_dom_iter.nextSibling
   
   def node_add(self, top_dom_iter):
      """Add a Node"""
      self.pixbuf_vector = []
      self.chars_counter = 0
      folder_name = top_dom_iter.attributes["folderName"].value[0:-1]
      node_name = "?"
      child_dom_iter = top_dom_iter.firstChild
      while child_dom_iter:
         if child_dom_iter.nodeName == "properties":
            nephew_dom_iter = child_dom_iter.firstChild
            while nephew_dom_iter:
               if nephew_dom_iter.nodeName == "name":
                  if nephew_dom_iter.firstChild: node_name = nephew_dom_iter.firstChild.data
               nephew_dom_iter = nephew_dom_iter.nextSibling
         child_dom_iter = child_dom_iter.nextSibling
      self.subfolder_path = os.path.join(self.folderpath, folder_name)
      node_xml_filepath = os.path.join(self.subfolder_path, ".basket")
      file_descriptor = open(node_xml_filepath, 'r')
      node_xml_string = file_descriptor.read()
      file_descriptor.close()
      dom = xml.dom.minidom.parseString(node_xml_string)
      dom_iter = dom.firstChild
      child_dom_iter = dom_iter.firstChild
      while not child_dom_iter:
         dom_iter = dom_iter.nextSibling
         child_dom_iter = dom_iter.firstChild
      while child_dom_iter:
         if child_dom_iter.nodeName == "properties":
            self.nodes_list.append(self.dom.createElement("node"))
            self.nodes_list[-1].setAttribute("name", node_name)
            self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
            self.nodes_list[-2].appendChild(self.nodes_list[-1])
         elif child_dom_iter.nodeName == "notes":
            self.notes_parse(child_dom_iter)
         child_dom_iter = child_dom_iter.nextSibling
      for pixbuf_element in self.pixbuf_vector:
         self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
      # check if the node has children
      child_dom_iter = top_dom_iter.firstChild
      while child_dom_iter:
         if child_dom_iter.nodeName == "basket": self.node_add(child_dom_iter)
         child_dom_iter = child_dom_iter.nextSibling
      self.nodes_list.pop()
   
   def notes_parse(self, notes_dom_iter):
      """Parse a 'notes'"""
      nephew_dom_iter = notes_dom_iter.firstChild
      while nephew_dom_iter:
         if nephew_dom_iter.nodeName == "group":
            self.notes_parse(nephew_dom_iter)
         elif nephew_dom_iter.nodeName == "note":
            self.note_parse(nephew_dom_iter)
         nephew_dom_iter = nephew_dom_iter.nextSibling
   
   def note_parse(self, note_dom_iter):
      """Parse a 'note'"""
      if note_dom_iter.attributes['type'].value == "html":
         # curr_state 0: standby, taking no data
         # curr_state 1: waiting for node content, take many data
         self.curr_state = 0
         content_dom_iter = note_dom_iter.firstChild
         while content_dom_iter:
            if content_dom_iter.nodeName == "content":
               content_path = os.path.join(self.subfolder_path, content_dom_iter.firstChild.data)
               if os.path.isfile(content_path):
                  file_descriptor = open(content_path, 'r')
                  node_string = file_descriptor.read()
                  file_descriptor.close()
               else: node_string = "" # empty node
               self.feed(node_string.decode("utf-8", "ignore"))
               self.rich_text_serialize("\n")
               self.chars_counter += 1
               break
            content_dom_iter = content_dom_iter.nextSibling
      elif note_dom_iter.attributes['type'].value == "image":
         content_dom_iter = note_dom_iter.firstChild
         while content_dom_iter:
            if content_dom_iter.nodeName == "content":
               content_path = os.path.join(self.subfolder_path, content_dom_iter.firstChild.data)
               if os.path.isfile(content_path):
                  pixbuf = gtk.gdk.pixbuf_new_from_file(content_path)
                  self.pixbuf_vector.append([self.chars_counter, pixbuf, "left"])
                  self.chars_counter += 1
                  self.rich_text_serialize("\n")
                  self.chars_counter += 1
               break
            content_dom_iter = content_dom_iter.nextSibling
      elif note_dom_iter.attributes['type'].value == "link":
         content_dom_iter = note_dom_iter.firstChild
         while content_dom_iter:
            if content_dom_iter.nodeName == "content":
               content = content_dom_iter.firstChild.data
               if content[:4] in ["http", "file"]:
                  if content[:4] == "http":
                     self.curr_attributes["link"] = "webs %s" % content
                  elif content[:4] == "file":
                     self.curr_attributes["link"] = "file %s" % base64.b64encode(content[7:])
                  content_title = content_dom_iter.attributes['title'].value
                  self.rich_text_serialize(content_title)
                  self.curr_attributes["link"] = ""
                  self.rich_text_serialize("\n")
                  self.chars_counter += len(content_title) + 1
               break
            content_dom_iter = content_dom_iter.nextSibling
   
   def handle_starttag(self, tag, attrs):
      """Encountered the beginning of a tag"""
      if self.curr_state == 0:
         if tag == "body": self.curr_state = 1
      else: # self.curr_state == 1
         if tag == "span" and attrs[0][0] == "style":
            if "font-weight" in attrs[0][1]: self.curr_attributes["weight"] = "heavy"
            elif "font-style" in attrs[0][1] and "italic" in attrs[0][1]: self.curr_attributes["style"] = "italic"
            elif "text-decoration" in attrs[0][1] and "underline" in attrs[0][1]: self.curr_attributes["underline"] = "single"
            elif "color" in attrs[0][1]:
               match = re.match("(?<=^).+:(.+);(?=$)", attrs[0][1])
               if match != None: self.curr_attributes["foreground"] = match.group(1).strip()
         elif tag == "a" and len(attrs) > 0:
            link_url = attrs[0][1]
            if len(link_url) > 7:
               if link_url[0:4] == "http": self.curr_attributes["link"] = "webs %s" % link_url
               elif link_url[0:7] == "file://": self.curr_attributes["link"] = "file %s" % base64.b64encode(link_url[7:])
               else: self.curr_attributes["link"] = "webs %s" % ("http://" + link_url)
         elif tag == "br":
            # this is a data block composed only by an endline
            self.rich_text_serialize("\n")
            self.chars_counter += 1
         elif tag == "hr":
            # this is a data block composed only by an horizontal rule
            self.rich_text_serialize(cons.HORIZONTAL_RULE)
            self.chars_counter += len(cons.HORIZONTAL_RULE)
         elif tag == "li":
            self.rich_text_serialize("\n• ")
            self.chars_counter += 3
   
   def handle_endtag(self, tag):
      """Encountered the end of a tag"""
      if self.curr_state == 0: return
      if tag == "p":
         # this is a data block composed only by an endline
         self.rich_text_serialize("\n")
         self.chars_counter += 1
      elif tag == "span":
         self.curr_attributes["weight"] = ""
         self.curr_attributes["style"] = ""
         self.curr_attributes["underline"] = ""
         self.curr_attributes["foreground"] = ""
      elif tag == "a": self.curr_attributes["link"] = ""
      elif tag == "body":
         self.rich_text_serialize("\n")
         self.chars_counter += 1
      
   def handle_data(self, data):
      """Found Data"""
      if self.curr_state == 0 or data in ['\n', '\n\n']: return
      data = data.replace("\n", "")
      self.rich_text_serialize(data)
      self.chars_counter += len(data)
      
   def handle_entityref(self, name):
      """Found Entity Reference like &name;"""
      if self.curr_state == 0: return
      if name in htmlentitydefs.name2codepoint:
         unicode_char = unichr(htmlentitydefs.name2codepoint[name])
         self.rich_text_serialize(unicode_char)
         self.chars_counter += 1
   
   def get_cherrytree_xml(self):
      """Returns a CherryTree string Containing the KeepNote Nodes"""
      self.dom = xml.dom.minidom.Document()
      self.nodes_list = [self.dom.createElement("cherrytree")]
      self.dom.appendChild(self.nodes_list[0])
      self.curr_attributes = {}
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      self.latest_span = ""
      self.start_parsing()
      return self.dom.toxml()


class TreepadHandler:
   """The Handler of the Treepad File Parsing"""
   
   def __init__(self):
      """Machine boot"""
      self.xml_handler = machines.XMLHandler(self)
      
   def parse_string_lines(self, file_descriptor):
      """Parse the string line by line"""
      self.curr_title = ""
      for text_line in file_descriptor:
         if len(text_line) > 0 and text_line[0] == "<": print text_line
      
   def get_cherrytree_xml(self, file_descriptor):
      """Parses the Given Notecase HTML String feeding the CherryTree XML dom"""
      self.dom = xml.dom.minidom.Document()
      self.nodes_list = [self.dom.createElement("cherrytree")]
      self.dom.appendChild(self.nodes_list[0])
      self.parse_string_lines(file_descriptor)
      return self.dom.toxml()


class NotecaseHandler(HTMLParser.HTMLParser):
   """The Handler of the Notecase Files Parsing"""
   
   def __init__(self):
      """Machine boot"""
      HTMLParser.HTMLParser.__init__(self)
      self.xml_handler = machines.XMLHandler(self)
   
   def rich_text_serialize(self, text_data):
      """Appends a new part to the XML rich text"""
      dom_iter = self.dom.createElement("rich_text")
      for tag_property in cons.TAG_PROPERTIES:
         if self.curr_attributes[tag_property] != "":
            dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
      self.nodes_list[-1].appendChild(dom_iter)
      text_iter = self.dom.createTextNode(text_data)
      dom_iter.appendChild(text_iter)
   
   def handle_starttag(self, tag, attrs):
      """Encountered the beginning of a tag"""
      if self.curr_state == 0:
         if tag == "dt":
            # waiting for the title
            # got dt, we go state 0->1
            self.curr_state = 1
      elif self.curr_state == 2:
         if tag == "dl":
            # the current node becomes father
            for pixbuf_element in self.pixbuf_vector:
               self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
            self.pixbuf_vector = []
            self.chars_counter = 0
            # got dl, we go state 2->0 and wait for the child
            self.curr_state = 0
         elif tag == "dt":
            # the current node has no more job to do
            for pixbuf_element in self.pixbuf_vector:
               self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
            self.pixbuf_vector = []
            self.chars_counter = 0
            self.nodes_list.pop()
            # waiting for the title
            # got dt, we go state 2->1
            self.curr_state = 1
         elif tag == "b": self.curr_attributes["weight"] = "heavy"
         elif tag == "i": self.curr_attributes["style"] = "italic"
         elif tag == "u": self.curr_attributes["underline"] = "single"
         elif tag == "s": self.curr_attributes["strikethrough"] = "true"
         elif tag == "span" and attrs[0][0] == "style":
            match = re.match("(?<=^)(.+):(.+)(?=$)", attrs[0][1])
            if match != None:
               if match.group(1) == "color":
                  self.curr_attributes["foreground"] = match.group(2).strip()
                  self.latest_span = "foreground"
               elif match.group(1) == "background-color":
                  self.curr_attributes["background"] = match.group(2).strip()
                  self.latest_span = "background"
         elif tag == "a" and len(attrs) > 0:
            link_url = attrs[0][1]
            if len(link_url) > 7:
               if link_url[0:4] == "http": self.curr_attributes["link"] = "webs %s" % link_url
               elif link_url[0:7] == "file://": self.curr_attributes["link"] = "file %s" % base64.b64encode(link_url[7:])
         elif tag == "br":
            # this is a data block composed only by an endline
            self.rich_text_serialize("\n")
            self.chars_counter += 1
         elif tag == "li":
            self.rich_text_serialize("\n• ")
            self.chars_counter += 3
         elif tag == "img" and len(attrs) > 0:
            for attribute in attrs:
               if attribute[0] == "src":
                  if attribute[1][:23] == "data:image/jpeg;base64,":
                     jpeg_data = attribute[1][23:]
                     pixbuf_loader = gtk.gdk.pixbuf_loader_new_with_mime_type("image/jpeg")
                     try: pixbuf_loader.write(base64.b64decode(jpeg_data))
                     except:
                        try: pixbuf_loader.write(base64.b64decode(jpeg_data + "="))
                        except: pixbuf_loader.write(base64.b64decode(jpeg_data + "=="))
                     pixbuf_loader.close()
                     pixbuf = pixbuf_loader.get_pixbuf()
                     self.pixbuf_vector.append([self.chars_counter, pixbuf, "left"])
                     self.chars_counter += 1
                  elif attribute[1][:22] == "data:image/png;base64,":
                     png_data = attribute[1][22:]
                     pixbuf_loader = gtk.gdk.pixbuf_loader_new_with_mime_type("image/png")
                     try: pixbuf_loader.write(base64.b64decode(png_data))
                     except:
                        try: pixbuf_loader.write(base64.b64decode(png_data + "="))
                        except: pixbuf_loader.write(base64.b64decode(png_data + "=="))
                     pixbuf_loader.close()
                     pixbuf = pixbuf_loader.get_pixbuf()
                     self.pixbuf_vector.append([self.chars_counter, pixbuf, "left"])
                     self.chars_counter += 1
   
   def handle_endtag(self, tag):
      """Encountered the end of a tag"""
      if self.curr_state == 1:
         if tag == "dt":
            # title reception complete
            self.nodes_list.append(self.dom.createElement("node"))
            self.nodes_list[-1].setAttribute("name", self.curr_title)
            self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
            self.nodes_list[-2].appendChild(self.nodes_list[-1])
            self.curr_title = ""
            # waiting for data
            if self.chars_counter > 0:
               # this means the new node is son of the previous, so we did not pop
               for pixbuf_element in self.pixbuf_vector:
                  self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-2], self.dom)
               self.pixbuf_vector = []
               self.chars_counter = 0
            # got dd, we go state 1->2
            self.curr_state = 2
      elif self.curr_state == 2:
         if tag == "dd":
            # the current node has no more job to do
            for pixbuf_element in self.pixbuf_vector:
               self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
            self.pixbuf_vector = []
            self.chars_counter = 0
            self.nodes_list.pop()
            # got /dd, we go state 2->0 and wait for a brother
            self.curr_state = 0
         elif tag == "dl":
            # the current node has no more job to do
            for pixbuf_element in self.pixbuf_vector:
               self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
            self.pixbuf_vector = []
            self.chars_counter = 0
            self.nodes_list.pop()
            self.nodes_list.pop()
            # got /dl, we go state 2->0 and wait for a father's brother
            self.curr_state = 0
         elif tag == "b": self.curr_attributes["weight"] = ""
         elif tag == "i": self.curr_attributes["style"] = ""
         elif tag == "u": self.curr_attributes["underline"] = ""
         elif tag == "s": self.curr_attributes["strikethrough"] = ""
         elif tag == "span":
            if self.latest_span == "foreground": self.curr_attributes["foreground"] = ""
            elif self.latest_span == "background": self.curr_attributes["background"] = ""
         elif tag == "a": self.curr_attributes["link"] = ""
      elif tag == "dl":
         # backward one level in nodes list
         for pixbuf_element in self.pixbuf_vector:
            self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
         self.pixbuf_vector = []
         self.chars_counter = 0
         self.nodes_list.pop()
      
   def handle_data(self, data):
      """Found Data"""
      if self.curr_state == 0 or data in ['\n', '\n\n']: return
      if self.curr_state == 1:
         # state 1 got title
         self.curr_title += data
      elif self.curr_state == 2:
         # state 2 got data
         clean_data = data.replace("\n", "")
         self.rich_text_serialize(clean_data)
         self.chars_counter += len(clean_data)
      
   def handle_entityref(self, name):
      """Found Entity Reference like &name;"""
      if name in htmlentitydefs.name2codepoint:
         unicode_char = unichr(htmlentitydefs.name2codepoint[name])
      else: return
      if self.curr_state == 1:
         # state 1 got title
         self.curr_title += unicode_char
      elif self.curr_state == 2:
         # state 2 got data
         self.rich_text_serialize(unicode_char)
         self.chars_counter += 1
      
   def get_cherrytree_xml(self, input_string):
      """Parses the Given Notecase HTML String feeding the CherryTree XML dom"""
      self.dom = xml.dom.minidom.Document()
      self.nodes_list = [self.dom.createElement("cherrytree")]
      self.dom.appendChild(self.nodes_list[0])
      self.curr_state = 0
      self.curr_title = ""
      self.curr_attributes = {}
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      self.latest_span = ""
      # curr_state 0: standby, taking no data
      # curr_state 1: waiting for node title, take one data
      # curr_state 2: waiting for node content, take many data
      self.pixbuf_vector = []
      self.chars_counter = 0
      self.feed(input_string.decode("utf-8", "ignore"))
      return self.dom.toxml()


class HTMLFromClipboardHandler(HTMLParser.HTMLParser):
   """The Handler of the HTML received from clipboard"""
   
   def __init__(self, dad):
      """Machine boot"""
      self.dad = dad
      self.monitored_tags = ["p", "b", "i", "u", "s", "h1", "h2", "span", "font"]
      HTMLParser.HTMLParser.__init__(self)
      self.xml_handler = machines.XMLHandler(self)
   
   def rich_text_serialize(self, text_data):
      """Appends a new part to the XML rich text"""
      dom_iter = self.dom.createElement("rich_text")
      for tag_property in cons.TAG_PROPERTIES:
         if self.curr_attributes[tag_property] != "":
            dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
      self.curr_dom_slot.appendChild(dom_iter)
      text_iter = self.dom.createTextNode(text_data)
      dom_iter.appendChild(text_iter)
   
   def get_rgb_gtk_attribute(self, html_attribute):
      """Get RGB GTK attribute from HTML attribute"""
      if html_attribute[0] == "#":
         return html_attribute
      elif "rgb" in html_attribute:
         match = re.match(".*rgb\((\d+), (\d+), (\d+)\).*", html_attribute)
         if match:
            html_attribute = "#%.2x%.2x%.2x" % (int(match.group(1)), int(match.group(2)), int(match.group(3)))
            return html_attribute
      return None
   
   def handle_starttag(self, tag, attrs):
      """Encountered the beginning of a tag"""
      if tag in self.monitored_tags: self.in_a_tag += 1
      if self.curr_state == 0:
         if tag == "body": self.curr_state = 1
      elif self.curr_state == 1:
         if tag == "table":
            self.curr_state = 2
            self.curr_table = []
            self.curr_table_header = False
         elif tag == "b": self.curr_attributes["weight"] = "heavy"
         elif tag == "i": self.curr_attributes["style"] = "italic"
         elif tag == "u": self.curr_attributes["underline"] = "single"
         elif tag == "s": self.curr_attributes["strikethrough"] = "true"
         elif tag == "style": self.curr_state = 0
         elif tag == "span":
            for attr in attrs:
               if attr[0] == "style":
                  match = re.match("(?<=^)(.+):(.+)(?=$)", attr[1])
                  if match != None:
                     if match.group(1) == "color":
                        attribute = self.get_rgb_gtk_attribute(match.group(2).strip())
                        if attribute:
                           self.curr_attributes["foreground"] = attribute
                           self.latest_span = "foreground"
                     elif match.group(1) in ["background", "background-color"]:
                        attribute = self.get_rgb_gtk_attribute(match.group(2).strip())
                        if attribute:
                           self.curr_attributes["background"] = attribute
                           self.latest_span = "background"
                     elif match.group(1) == "text-decoration":
                        if match.group(2).strip() in ["underline", "underline;"]:
                           self.curr_attributes["underline"] = "single"
                           self.latest_span = "underline"
                     elif match.group(1) == "font-weight":
                        if match.group(2).strip() in ["bolder", "bolder;"]:
                           self.curr_attributes["weight"] = "heavy"
                           self.latest_span = "weight"
         elif tag == "font":
            for attr in attrs:
               if attr[0] == "color":
                  attribute = self.get_rgb_gtk_attribute(attr[1].strip())
                  if attribute:
                     self.curr_attributes["foreground"] = attribute
                     self.latest_font = "foreground"
         elif tag in ["h1", "h2"]:
            self.rich_text_serialize(cons.CHAR_NEWLINE)
            if tag == "h1": self.curr_attributes["scale"] = "h1"
            else: self.curr_attributes["scale"] = "h2"
            for attr in attrs:
               if attr[0] == "align": self.curr_attributes["justification"] = attr[1].strip().lower()
         elif tag == "a" and len(attrs) > 0:
            link_url = attrs[0][1]
            if len(link_url) > 7:
               if link_url[0:4] == "http": self.curr_attributes["link"] = "webs %s" % link_url
               elif link_url[0:7] == "file://": self.curr_attributes["link"] = "file %s" % base64.b64encode(link_url[7:])
         elif tag == "br": self.rich_text_serialize(cons.CHAR_NEWLINE)
         elif tag == "ol": self.curr_list_type = ["o", 1]
         elif tag == "ul": self.curr_list_type = ["u", 0]
         elif tag == "li":
            if self.curr_list_type[0] == "u": self.rich_text_serialize("• ")
            else:
               self.rich_text_serialize("%s. " % self.curr_list_type[1])
               self.curr_list_type[1] += 1
         elif tag == "img" and len(attrs) > 0:
            pass # cross clipboard images not handled yet
      elif self.curr_state == 2:
         if tag == "tr": self.curr_table.append([])
         elif tag in ["td", "th"]:
            self.curr_cell = ""
            if tag == "th": self.curr_table_header = True
   
   def handle_endtag(self, tag):
      """Encountered the end of a tag"""
      if tag in self.monitored_tags: self.in_a_tag -= 1
      if self.curr_state == 0:
         if tag == "style": self.curr_state = 1
      if self.curr_state == 1:
         if tag == "p": self.rich_text_serialize(cons.CHAR_NEWLINE)
         elif tag == "b": self.curr_attributes["weight"] = ""
         elif tag == "i": self.curr_attributes["style"] = ""
         elif tag == "u": self.curr_attributes["underline"] = ""
         elif tag == "s": self.curr_attributes["strikethrough"] = ""
         elif tag == "span":
            if self.latest_span == "foreground": self.curr_attributes["foreground"] = ""
            elif self.latest_span == "background": self.curr_attributes["background"] = ""
            elif self.latest_span == "underline": self.curr_attributes["underline"] = ""
            elif self.latest_span == "weight": self.curr_attributes["weight"] = ""
         elif tag == "font":
            if self.latest_font == "foreground": self.curr_attributes["foreground"] = ""
         elif tag in ["h1", "h2"]:
            self.curr_attributes["scale"] = ""
            self.curr_attributes["justification"] = ""
            self.rich_text_serialize(cons.CHAR_NEWLINE)
         elif tag == "a": self.curr_attributes["link"] = ""
         elif tag == "li": self.rich_text_serialize(cons.CHAR_NEWLINE)
      elif self.curr_state == 2:
         if tag in ["td", "th"]: self.curr_table[-1].append(self.curr_cell)
         elif tag == "table":
            self.curr_state = 1
            if len(self.curr_table) == 1 and len(self.curr_table[0]) == 1:
               # it's a codebox
               codebox_dict = {
               'frame_width': 300,
               'frame_height': 150,
               'width_in_pixels': True,
               'syntax_highlighting': cons.CUSTOM_COLORS_ID,
               'fill_text': self.curr_table[0][0]
               }
               self.dad.xml_handler.codebox_element_to_xml([0, codebox_dict, "left"], self.curr_dom_slot)
            else:
               # it's a table
               if not self.curr_table_header: self.curr_table.append([_("click me")]*len(self.curr_table[0]))
               else: self.curr_table.append(self.curr_table.pop(0))
               table_dict = {'col_min': 40,
                             'col_max': 1000,
                             'matrix': self.curr_table}
               self.dad.xml_handler.table_element_to_xml([0, table_dict, "left"], self.curr_dom_slot)
            self.rich_text_serialize(cons.CHAR_NEWLINE)
      
   def handle_data(self, data):
      """Found Data"""
      if self.curr_state == 0: return
      if self.in_a_tag: clean_data = data.replace(cons.CHAR_NEWLINE, cons.CHAR_SPACE)
      else: clean_data = data.replace(cons.CHAR_NEWLINE, "")
      if not clean_data or clean_data == cons.CHAR_TAB: return
      if self.curr_state == 1: self.rich_text_serialize(clean_data.replace(cons.CHAR_TAB, cons.CHAR_SPACE))
      elif self.curr_state == 2: self.curr_cell += clean_data.replace(cons.CHAR_TAB, "")
      
   def handle_entityref(self, name):
      """Found Entity Reference like &name;"""
      if self.curr_state == 0: return
      if name in htmlentitydefs.name2codepoint:
         unicode_char = unichr(htmlentitydefs.name2codepoint[name])
      else: return
      if self.curr_state == 1: self.rich_text_serialize(unicode_char)
      elif self.curr_state == 2: self.curr_cell += unicode_char
      
   def get_clipboard_selection_xml(self, input_string):
      """Parses the Given HTML String feeding the XML dom"""
      self.dom = xml.dom.minidom.Document()
      root = self.dom.createElement("root")
      self.dom.appendChild(root)
      self.curr_dom_slot = self.dom.createElement("slot")
      root.appendChild(self.curr_dom_slot)
      self.curr_state = 0
      self.curr_attributes = {}
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      self.latest_span = ""
      self.latest_font = ""
      self.curr_cell = ""
      self.in_a_tag = 0
      self.curr_list_type = ["u", 0]
      # curr_state 0: standby, taking no data
      # curr_state 1: receiving rich text
      # curr_state 2: receiving table or codebox data
      input_string = input_string.decode("utf-8", "ignore")
      if not HTMLCheck().is_html_ok(input_string):
         input_string = cons.HTML_HEADER % "" + input_string + cons.HTML_FOOTER
      self.feed(input_string)
      return self.dom.toxml()


class HTMLCheck(HTMLParser.HTMLParser):
   """Check for Minimal Tags"""
   
   def __init__(self):
      """Machine boot"""
      HTMLParser.HTMLParser.__init__(self)
      
   def handle_starttag(self, tag, attrs):
      """Encountered the beginning of a tag"""
      if tag == "html" and self.steps == 0: self.steps = 1
      elif tag == "head" and self.steps == 1: self.steps = 2
      elif tag == "title" and self.steps == 2: self.steps = 3
      elif tag == "body" and self.steps == 5: self.steps = 6
      
   def handle_endtag(self, tag):
      """Encountered the end of a tag"""
      if tag == "title" and self.steps == 3: self.steps = 4
      elif tag == "head" and self.steps == 4: self.steps = 5
      elif tag == "body" and self.steps == 6: self.steps = 7
      if tag == "html" and self.steps == 7: self.steps = 8
      
   def is_html_ok(self, input_string):
      """Checks for the minimal html tags"""
      self.steps = 0
      self.feed(input_string)
      return self.steps == 8
