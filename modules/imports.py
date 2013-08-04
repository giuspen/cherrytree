# -*- coding: UTF-8 -*-
#
#       imports.py
#
#       Copyright 2009-2013 Giuseppe Penone <giuspen@gmail.com>
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
import gtk, os, xml.dom.minidom, re, base64, urllib2
import cons, machines


def get_internal_link_from_http_url(link_url):
    """Get internal cherrytree link attribute from HTTP link url"""
    if link_url[0:4] == "http": return "webs %s" % link_url
    elif link_url[0:7] == "file://": return "file %s" % base64.b64encode(link_url[7:])
    else: return "webs %s" % ("http://" + link_url)

def get_web_links_offsets_from_plain_text(plain_text):
    """Parse plain text for possible web links"""
    web_links = []
    max_end_offset = len(plain_text)
    max_start_offset = max_end_offset - 7
    start_offset = 0
    while start_offset < max_start_offset:
        is_link = False
        if plain_text[start_offset] == "h":
            if plain_text[start_offset:start_offset+4] == "http":
                is_link = True
        elif plain_text[start_offset] == "f":
            if plain_text[start_offset:start_offset+3] == "ftp":
                is_link = True
        elif plain_text[start_offset] == "w":
            if plain_text[start_offset:start_offset+4] == "www.":
                is_link = True
        if is_link:
            end_offset = start_offset + 3
            while (end_offset < max_end_offset)\
            and (plain_text[end_offset] not in [cons.CHAR_SPACE, cons.CHAR_NEWLINE]):
                end_offset += 1
            web_links.append([start_offset, end_offset])
            start_offset = end_offset + 1
        else: start_offset += 1
    return web_links


class LeoHandler:
    """The Handler of the Leo File Parsing"""

    def __init__(self):
        """Machine boot"""
        self.xml_handler = machines.XMLHandler(self)

    def parse_leo_xml(self, leo_string):
        """Leo XML Parsing"""
        self.tnodes_dict = {}
        dom = xml.dom.minidom.parseString(leo_string)
        dom_iter = dom.firstChild
        while dom_iter:
            if dom_iter.nodeName == "leo_file": break
            dom_iter = dom_iter.nextSibling
        child_dom_iter = dom_iter.firstChild
        while child_dom_iter:
            if child_dom_iter.nodeName == "vnodes":
                vnode_dom_iter = child_dom_iter.firstChild
            elif child_dom_iter.nodeName == "tnodes":
                tnode_dom_iter = child_dom_iter.firstChild
                while tnode_dom_iter:
                    if tnode_dom_iter.nodeName == "t":
                        if tnode_dom_iter.firstChild: fill_text = tnode_dom_iter.firstChild.data
                        else: fill_text = ""
                        self.tnodes_dict[tnode_dom_iter.attributes['tx'].value] = fill_text
                    tnode_dom_iter = tnode_dom_iter.nextSibling
            child_dom_iter = child_dom_iter.nextSibling
        while vnode_dom_iter:
            if vnode_dom_iter.nodeName == "v": self.append_leo_node(vnode_dom_iter)
            vnode_dom_iter = vnode_dom_iter.nextSibling

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        #for tag_property in cons.TAG_PROPERTIES:
        #   if self.curr_attributes[tag_property] != "":
        #      dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
        self.nodes_list[-1].appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)

    def append_leo_node(self, vnode_dom_iter):
        """Append a Leo Node"""
        self.nodes_list.append(self.dom.createElement("node"))
        node_name = "?"
        child_dom_iter = vnode_dom_iter.firstChild
        while child_dom_iter:
            if child_dom_iter.nodeName == "vh":
                if child_dom_iter.firstChild: node_name = child_dom_iter.firstChild.data
                self.nodes_list[-1].setAttribute("name", node_name)
                self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
                self.nodes_list[-2].appendChild(self.nodes_list[-1])
                self.rich_text_serialize(self.tnodes_dict[vnode_dom_iter.attributes['t'].value])
            elif child_dom_iter.nodeName == "v": self.append_leo_node(child_dom_iter)
            child_dom_iter = child_dom_iter.nextSibling
        self.nodes_list.pop()

    def get_cherrytree_xml(self, leo_string):
        """Returns a CherryTree string Containing the Leo Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.parse_leo_xml(leo_string)
        return self.dom.toxml()


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
                self.feed(node_string.decode(cons.STR_UTF8, cons.STR_IGNORE))
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
            if tag == "span" and attrs[0][0] == cons.TAG_STYLE:
                if "font-weight" in attrs[0][1]: self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                elif "font-style" in attrs[0][1] and cons.TAG_PROP_ITALIC in attrs[0][1]: self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                elif "text-decoration" in attrs[0][1] and cons.TAG_UNDERLINE in attrs[0][1]: self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                elif "color" in attrs[0][1]:
                    match = re.match("(?<=^).+:(.+);(?=$)", attrs[0][1])
                    if match != None: self.curr_attributes[cons.TAG_FOREGROUND] = match.group(1).strip()
            elif tag == "a" and len(attrs) > 0:
                link_url = attrs[0][1]
                if len(link_url) > 7:
                    self.curr_attributes[cons.TAG_LINK] = get_internal_link_from_http_url(link_url)
            elif tag == "img" and len(attrs) > 0:
                img_path = attrs[0][1]
                if os.path.isfile(img_path):
                    pixbuf = gtk.gdk.pixbuf_new_from_file(img_path)
                    self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                    self.chars_counter += 1
                else: print "%s not found" % img_path
            elif tag == "br":
                # this is a data block composed only by an endline
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                self.chars_counter += 1
            elif tag == "hr":
                # this is a data block composed only by an horizontal rule
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.h_rule+cons.CHAR_NEWLINE)
                self.chars_counter += len(self.h_rule)+2
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+cons.CHAR_LISTBUL+cons.CHAR_SPACE)
                self.chars_counter += 3

    def handle_endtag(self, tag):
        """Encountered the end of a tag"""
        if self.curr_state == 0: return
        if tag == "p":
            # this is a data block composed only by an endline
            self.rich_text_serialize(cons.CHAR_NEWLINE)
            self.chars_counter += 1
        elif tag == "span":
            self.curr_attributes[cons.TAG_WEIGHT] = ""
            self.curr_attributes[cons.TAG_STYLE] = ""
            self.curr_attributes[cons.TAG_UNDERLINE] = ""
            self.curr_attributes[cons.TAG_FOREGROUND] = ""
        elif tag == "a": self.curr_attributes[cons.TAG_LINK] = ""

    def handle_data(self, data):
        """Found Data"""
        if self.curr_state == 0 or data in [cons.CHAR_NEWLINE, cons.CHAR_NEWLINE*2]: return
        data = data.replace(cons.CHAR_NEWLINE, "")
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
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
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
        self.feed(node_string.decode(cons.STR_UTF8, cons.STR_IGNORE))
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
            if tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
            elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
            elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
            elif tag == "strike": self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
            elif tag == "span" and attrs[0][0] == cons.TAG_STYLE:
                match = re.match("(?<=^)(.+):(.+)(?=$)", attrs[0][1])
                if match != None:
                    if match.group(1) == "color":
                        self.curr_attributes[cons.TAG_FOREGROUND] = match.group(2).strip()
                        self.latest_span = cons.TAG_FOREGROUND
                    elif match.group(1) == "background-color":
                        self.curr_attributes[cons.TAG_BACKGROUND] = match.group(2).strip()
                        self.latest_span = cons.TAG_BACKGROUND
            elif tag == "a" and len(attrs) > 0:
                link_url = attrs[0][1]
                if len(link_url) > 7:
                    self.curr_attributes[cons.TAG_LINK] = get_internal_link_from_http_url(link_url)
            elif tag == "img" and len(attrs) > 0:
                img_name = attrs[0][1]
                img_path = os.path.join(self.curr_folder, img_name)
                if os.path.isfile(img_path):
                    pixbuf = gtk.gdk.pixbuf_new_from_file(img_path)
                    self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                    self.chars_counter += 1
                else: print "%s not found" % img_path
            elif tag == "br":
                # this is a data block composed only by an endline
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                self.chars_counter += 1
            elif tag == "hr":
                # this is a data block composed only by an horizontal rule
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.h_rule+cons.CHAR_NEWLINE)
                self.chars_counter += len(self.h_rule)+2
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+cons.CHAR_LISTBUL+cons.CHAR_SPACE)
                self.chars_counter += 3

    def handle_endtag(self, tag):
        """Encountered the end of a tag"""
        if self.curr_state == 0: return
        if tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = ""
        elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = ""
        elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = ""
        elif tag == "strike": self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
        elif tag == "span":
            if self.latest_span == cons.TAG_FOREGROUND: self.curr_attributes[cons.TAG_FOREGROUND] = ""
            elif self.latest_span == cons.TAG_BACKGROUND: self.curr_attributes[cons.TAG_BACKGROUND] = ""
        elif tag == "a": self.curr_attributes[cons.TAG_LINK] = ""

    def handle_data(self, data):
        """Found Data"""
        if self.curr_state == 0 or data in [cons.CHAR_NEWLINE, cons.CHAR_NEWLINE*2]: return
        data = data.replace(cons.CHAR_NEWLINE, "")
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
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.latest_span = ""
        self.start_parsing()
        return self.dom.toxml()


class ZimHandler():
    """The Handler of the Zim Folder Parsing"""
    
    def __init__(self, folderpath):
        """Machine boot"""
        self.folderpath = folderpath
        self.xml_handler = machines.XMLHandler(self)
    
    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        for tag_property in cons.TAG_PROPERTIES:
            if self.curr_attributes[tag_property] != "":
                dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
        self.nodes_list[-1].appendChild(dom_iter)
        if self.curr_attributes[cons.TAG_SCALE] in [cons.TAG_PROP_H1, cons.TAG_PROP_H2, cons.TAG_PROP_H3]:
            try: text_data = text_data[1:-1]
            except: pass
        self.chars_counter += len(text_data)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)
    
    def parse_folder(self, curr_folder):
        """Start the Parsing"""
        for element in os.listdir(curr_folder):
            if len(element) > 4 and element[-4:] == ".txt" \
            and os.path.isfile(os.path.join(curr_folder, element)):
                file_descriptor = open(os.path.join(curr_folder, element), 'r')
                wiki_string = file_descriptor.read()
                file_descriptor.close()
                #
                node_name = os.path.splitext(element)[0]
                self.node_add(wiki_string, node_name, curr_folder)
                # check if the node has children
                children_folder = os.path.join(curr_folder, node_name)
                if os.path.isdir(children_folder):
                    self.parse_folder(children_folder)
                self.nodes_list.pop()
    
    def node_add(self, wiki_string, node_name, curr_folder):
        """Add a node"""
        self.nodes_list.append(self.dom.createElement("node"))
        self.nodes_list[-1].setAttribute("name", node_name)
        self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
        self.nodes_list[-2].appendChild(self.nodes_list[-1])
        #
        self.pixbuf_vector = []
        self.chars_counter = 0
        self.node_wiki_parse(wiki_string, node_name, curr_folder)
        for pixbuf_element in self.pixbuf_vector:
            self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
    
    def node_wiki_parse(self, wiki_string, node_name, curr_folder):
        """Parse the node wiki content"""
        curr_pos = 0
        wiki_string = wiki_string.replace(cons.CHAR_NEWLINE+cons.CHAR_STAR+cons.CHAR_SPACE, cons.CHAR_NEWLINE+cons.CHAR_LISTBUL+cons.CHAR_SPACE)
        max_pos = len(wiki_string)
        newline_count = 0
        wiki_slot = ""
        while curr_pos < max_pos:
            curr_char = wiki_string[curr_pos:curr_pos+1]
            next_char = wiki_string[curr_pos+1:curr_pos+2] if curr_pos+1 < max_pos else cons.CHAR_SPACE
            if newline_count < 4:
                if curr_char == cons.CHAR_NEWLINE: newline_count += 1
            else:
                if curr_char == cons.CHAR_STAR and next_char == cons.CHAR_STAR:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    if self.curr_attributes[cons.TAG_WEIGHT]: self.curr_attributes[cons.TAG_WEIGHT] = ""
                    else: self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                    curr_pos += 1
                elif curr_char == cons.CHAR_SLASH and next_char == cons.CHAR_SLASH:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    if self.curr_attributes[cons.TAG_STYLE]: self.curr_attributes[cons.TAG_STYLE] = ""
                    else: self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                    curr_pos += 1
                elif curr_char == cons.CHAR_USCORE and next_char == cons.CHAR_USCORE:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    if self.curr_attributes[cons.TAG_BACKGROUND]: self.curr_attributes[cons.TAG_BACKGROUND] = ""
                    else: self.curr_attributes[cons.TAG_BACKGROUND] = cons.COLOR_48_YELLOW
                    curr_pos += 1
                elif curr_char == cons.CHAR_TILDE and next_char == cons.CHAR_TILDE:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    if self.curr_attributes[cons.TAG_STRIKETHROUGH]: self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
                    else: self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
                    curr_pos += 1
                elif curr_char == cons.CHAR_SQUOTE and next_char == cons.CHAR_SQUOTE:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    if self.curr_attributes[cons.TAG_FAMILY]: self.curr_attributes[cons.TAG_FAMILY] = ""
                    else: self.curr_attributes[cons.TAG_FAMILY] = "monospace"
                    curr_pos += 1
                elif curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    if curr_pos+5 < max_pos and wiki_string[curr_pos+2:curr_pos+6] == cons.CHAR_EQUAL*4:
                        if self.curr_attributes[cons.TAG_SCALE]: self.curr_attributes[cons.TAG_SCALE] = ""
                        else: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H1
                        curr_pos += 5
                    elif curr_pos+4 < max_pos and wiki_string[curr_pos+2:curr_pos+5] == cons.CHAR_EQUAL*3:
                        if self.curr_attributes[cons.TAG_SCALE]: self.curr_attributes[cons.TAG_SCALE] = ""
                        else: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H2
                        curr_pos += 4
                    elif curr_pos+3 < max_pos and wiki_string[curr_pos+2:curr_pos+4] == cons.CHAR_EQUAL*2:
                        if self.curr_attributes[cons.TAG_SCALE]: self.curr_attributes[cons.TAG_SCALE] = ""
                        else: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                        curr_pos += 3
                    elif curr_pos+2 < max_pos and wiki_string[curr_pos+2:curr_pos+3] == cons.CHAR_EQUAL:
                        if self.curr_attributes[cons.TAG_SCALE]: self.curr_attributes[cons.TAG_SCALE] = ""
                        else: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                        curr_pos += 2
                    else:
                        if self.curr_attributes[cons.TAG_SCALE]: self.curr_attributes[cons.TAG_SCALE] = ""
                        else: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                        curr_pos += 1
                elif curr_char == cons.CHAR_CARET and next_char == cons.CHAR_BR_OPEN:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    self.curr_attributes[cons.TAG_SCALE] = "sup"
                    curr_pos += 1
                elif curr_char == cons.CHAR_USCORE and next_char == cons.CHAR_BR_OPEN:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    self.curr_attributes[cons.TAG_SCALE] = "sub"
                    curr_pos += 1
                elif curr_char == cons.CHAR_BR_CLOSE and self.curr_attributes[cons.TAG_SCALE] in ["sup", "sub"]:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    self.curr_attributes[cons.TAG_SCALE] = ""
                elif curr_char == cons.CHAR_BR_OPEN and next_char == cons.CHAR_BR_OPEN \
                  or curr_char == cons.CHAR_SQ_BR_OPEN and next_char == cons.CHAR_SQ_BR_OPEN:
                    if wiki_slot:
                        self.rich_text_serialize(wiki_slot)
                        wiki_slot = ""
                    curr_pos += 1
                elif curr_char == cons.CHAR_BR_CLOSE and next_char == cons.CHAR_BR_CLOSE:
                    valid_image = False
                    if cons.CHAR_QUESTION in wiki_slot:
                        splitted_wiki_slot = wiki_slot.split(cons.CHAR_QUESTION)
                        wiki_slot = splitted_wiki_slot[0]
                    if wiki_slot.startswith("./"): wiki_slot = os.path.join(curr_folder, node_name, wiki_slot[2:])
                    if os.path.isfile(wiki_slot):
                        try:
                            pixbuf = gtk.gdk.pixbuf_new_from_file(wiki_slot)
                            self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                            valid_image = True
                        except: pass
                    if not valid_image: print "! error: '%s' is not a valid image" % wiki_slot
                    wiki_slot = ""
                    curr_pos += 1
                elif curr_char == cons.CHAR_SQ_BR_CLOSE and next_char == cons.CHAR_SQ_BR_CLOSE:
                    if cons.CHAR_PIPE in wiki_slot:
                        target_n_label = wiki_slot.split(cons.CHAR_PIPE)
                    else:
                        target_n_label = [wiki_slot, wiki_slot]
                    exp_filepath = target_n_label[0]
                    if exp_filepath.startswith("./"): exp_filepath = os.path.join(curr_folder, node_name, exp_filepath[2:])
                    if cons.CHAR_SLASH in exp_filepath:
                        self.curr_attributes[cons.TAG_LINK] = "file %s" % base64.b64encode(exp_filepath)
                        self.rich_text_serialize(target_n_label[1])
                        self.curr_attributes[cons.TAG_LINK] = ""
                    else:
                        self.links_to_node_list.append({'name_dest': target_n_label[0],
                                                        'node_source': node_name,
                                                        'char_start': self.chars_counter,
                                                        'char_end': self.chars_counter+len(target_n_label[1])})
                        self.rich_text_serialize(target_n_label[1])
                    wiki_slot = ""
                    curr_pos += 1
                else: wiki_slot += curr_char
            curr_pos += 1
        if wiki_slot: self.rich_text_serialize(wiki_slot)
    
    def get_cherrytree_xml(self):
        """Returns a CherryTree string Containing the Zim Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.links_to_node_list = []
        self.parse_folder(self.folderpath)
        return self.dom.toxml()

    def set_links_to_nodes(self, dad):
        """After the node import, set the links to nodes on the new tree"""
        for link_to_node in self.links_to_node_list:
            node_dest = dad.get_tree_iter_from_node_name(link_to_node['name_dest'])
            node_source = dad.get_tree_iter_from_node_name(link_to_node['node_source'])
            if not node_dest:
                #print "node_dest not found"
                continue
            if not node_source:
                #print "node_source not found"
                continue
            source_buffer = dad.get_textbuffer_from_tree_iter(node_source)
            if source_buffer.get_char_count() < link_to_node['char_end']:
                continue
            property_value = "node" + cons.CHAR_SPACE + str(dad.treestore[node_dest][3])
            source_buffer.apply_tag_by_name(dad.apply_tag_exist_or_create(cons.TAG_LINK, property_value),
                                            source_buffer.get_iter_at_offset(link_to_node['char_start']),
                                            source_buffer.get_iter_at_offset(link_to_node['char_end']))


class TomboyHandler():
    """The Handler of the Tomboy Folder Parsing"""

    def __init__(self, folderpath):
        """Machine boot"""
        self.folderpath = folderpath
        self.xml_handler = machines.XMLHandler(self)

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        for tag_property in cons.TAG_PROPERTIES:
            if self.curr_attributes[tag_property] != "":
                dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
        self.dest_dom_new.appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)

    def start_parsing(self):
        """Start the Parsing"""
        for element in reversed(os.listdir(self.folderpath)):
            if os.path.isfile(os.path.join(self.folderpath, element)):
                file_descriptor = open(os.path.join(self.folderpath, element), 'r')
                xml_string = file_descriptor.read()
                file_descriptor.close()
                self.doc_parse(xml_string, element)

    def doc_parse(self, xml_string, file_name):
        """Parse an xml file"""
        dest_dom_father = self.dest_orphans_dom_node
        try: dom = xml.dom.minidom.parseString(xml_string)
        except:
            print "? non xml file:", file_name
            return
        dom_iter = dom.firstChild
        while dom_iter:
            if dom_iter.nodeName == "note": break
            dom_iter = dom_iter.nextSibling
        child_dom_iter = dom_iter.firstChild
        self.node_title = "???"
        while child_dom_iter:
            if child_dom_iter.nodeName == "title":
                self.node_title = child_dom_iter.firstChild.data if child_dom_iter.firstChild else "???"
                if len(self.node_title) > 18 and self.node_title[-18:] == " Notebook Template":
                    return
            elif child_dom_iter.nodeName == "text":
                text_dom_iter = child_dom_iter
            elif child_dom_iter.nodeName == "tags":
                tag_dom_iter = child_dom_iter.firstChild
                while tag_dom_iter:
                    if tag_dom_iter.nodeName == "tag":
                        if tag_dom_iter.firstChild:
                            tag_text = tag_dom_iter.firstChild.data
                            if len(tag_text) > 16 and tag_text[:16] == "system:notebook:":
                                dest_dom_father = self.notebook_exist_or_create(tag_text[16:])
                    tag_dom_iter = tag_dom_iter.nextSibling
            child_dom_iter = child_dom_iter.nextSibling
        nephew_dom_iter = text_dom_iter.firstChild
        while nephew_dom_iter:
            if nephew_dom_iter.nodeName == "note-content":
                self.node_add(nephew_dom_iter, dest_dom_father)
                break
            nephew_dom_iter = nephew_dom_iter.nextSibling

    def node_add(self, content_iter, dest_dom_father):
        """Add a Node"""
        self.dest_dom_new = self.dom.createElement("node")
        self.dest_dom_new.setAttribute("name", self.node_title)
        self.dest_dom_new.setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
        dest_dom_father.appendChild(self.dest_dom_new)
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.chars_counter = 0
        self.node_add_iter(content_iter.firstChild)

    def node_add_iter(self, dom_iter):
        """Recursively parse nodes"""
        while dom_iter:
            if dom_iter.nodeName == "#text":
                text_data = dom_iter.data
                if self.curr_attributes[cons.TAG_LINK] == "webs ":
                    self.curr_attributes[cons.TAG_LINK] += dom_iter.data
                elif self.is_list_item:
                    text_data = cons.CHAR_LISTBUL+cons.CHAR_SPACE + text_data
                elif self.is_link_to_node:
                    self.links_to_node_list.append({'name_dest':text_data,
                                                    'node_source':self.node_title,
                                                    'char_start':self.chars_counter,
                                                    'char_end':self.chars_counter+len(text_data)})
                self.rich_text_serialize(text_data)
                self.chars_counter += len(text_data)
            elif dom_iter.nodeName == "bold":
                self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_WEIGHT] = ""
            elif dom_iter.nodeName == cons.TAG_PROP_ITALIC:
                self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_STYLE] = ""
            elif dom_iter.nodeName == cons.TAG_STRIKETHROUGH:
                self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
            elif dom_iter.nodeName == "highlight":
                self.curr_attributes[cons.TAG_BACKGROUND] = cons.COLOR_48_YELLOW
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_BACKGROUND] = ""
            elif dom_iter.nodeName == "monospace":
                self.curr_attributes[cons.TAG_FAMILY] = dom_iter.nodeName
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_FAMILY] = ""
            elif dom_iter.nodeName == "size:small":
                self.curr_attributes[cons.TAG_SCALE] = "small"
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_SCALE] = ""
            elif dom_iter.nodeName == "size:large":
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H2
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_SCALE] = ""
            elif dom_iter.nodeName == "size:huge":
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H1
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_SCALE] = ""
            elif dom_iter.nodeName == "link:url":
                self.curr_attributes[cons.TAG_LINK] = "webs "
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_LINK] = ""
            elif dom_iter.nodeName == "list-item":
                self.is_list_item = True
                self.node_add_iter(dom_iter.firstChild)
                self.is_list_item = False
            elif dom_iter.nodeName == "link:internal":
                self.is_link_to_node = True
                self.node_add_iter(dom_iter.firstChild)
                self.is_link_to_node = False
            elif dom_iter.firstChild:
                #print dom_iter.nodeName
                self.node_add_iter(dom_iter.firstChild)
            dom_iter = dom_iter.nextSibling

    def notebook_exist_or_create(self, notebook_title):
        """Check if there's already a notebook with this title"""
        if not notebook_title in self.dest_notebooks_dom_nodes:
            self.dest_notebooks_dom_nodes[notebook_title] = self.dom.createElement("node")
            self.dest_notebooks_dom_nodes[notebook_title].setAttribute("name", notebook_title)
            self.dest_notebooks_dom_nodes[notebook_title].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
            self.dest_top_dom.appendChild(self.dest_notebooks_dom_nodes[notebook_title])
        return self.dest_notebooks_dom_nodes[notebook_title]

    def get_cherrytree_xml(self):
        """Returns a CherryTree string Containing the Tomboy Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.dest_top_dom = self.dom.createElement(cons.APP_NAME)
        self.dom.appendChild(self.dest_top_dom)
        self.curr_attributes = {}
        self.is_list_item = False
        self.is_link_to_node = False
        self.links_to_node_list = []
        # orphans node
        self.dest_orphans_dom_node = self.dom.createElement("node")
        self.dest_orphans_dom_node.setAttribute("name", "ORPHANS")
        self.dest_orphans_dom_node.setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
        self.dest_top_dom.appendChild(self.dest_orphans_dom_node)
        # notebooks nodes
        self.dest_notebooks_dom_nodes = {}
        # start parsing
        self.start_parsing()
        return self.dom.toxml()

    def set_links_to_nodes(self, dad):
        """After the node import, set the links to nodes on the new tree"""
        for link_to_node in self.links_to_node_list:
            node_dest = dad.get_tree_iter_from_node_name(link_to_node['name_dest'])
            node_source = dad.get_tree_iter_from_node_name(link_to_node['node_source'])
            if not node_dest:
                #print "node_dest not found"
                continue
            if not node_source:
                #print "node_source not found"
                continue
            source_buffer = dad.get_textbuffer_from_tree_iter(node_source)
            if source_buffer.get_char_count() < link_to_node['char_end']:
                continue
            property_value = "node" + cons.CHAR_SPACE + str(dad.treestore[node_dest][3])
            source_buffer.apply_tag_by_name(dad.apply_tag_exist_or_create(cons.TAG_LINK, property_value),
                                            source_buffer.get_iter_at_offset(link_to_node['char_start']),
                                            source_buffer.get_iter_at_offset(link_to_node['char_end']))


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
                    self.feed(node_string.decode(cons.STR_UTF8, cons.STR_IGNORE))
                    self.rich_text_serialize(cons.CHAR_NEWLINE)
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
                        self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                        self.chars_counter += 1
                        self.rich_text_serialize(cons.CHAR_NEWLINE)
                        self.chars_counter += 1
                    break
                content_dom_iter = content_dom_iter.nextSibling
        elif note_dom_iter.attributes['type'].value == cons.TAG_LINK:
            content_dom_iter = note_dom_iter.firstChild
            while content_dom_iter:
                if content_dom_iter.nodeName == "content":
                    content = content_dom_iter.firstChild.data
                    if content[:4] in ["http", "file"]:
                        if content[:4] == "http":
                            self.curr_attributes[cons.TAG_LINK] = "webs %s" % content
                        elif content[:4] == "file":
                            self.curr_attributes[cons.TAG_LINK] = "file %s" % base64.b64encode(content[7:])
                        content_title = content_dom_iter.attributes['title'].value
                        self.rich_text_serialize(content_title)
                        self.curr_attributes[cons.TAG_LINK] = ""
                        self.rich_text_serialize(cons.CHAR_NEWLINE)
                        self.chars_counter += len(content_title) + 1
                    break
                content_dom_iter = content_dom_iter.nextSibling

    def handle_starttag(self, tag, attrs):
        """Encountered the beginning of a tag"""
        if self.curr_state == 0:
            if tag == "body": self.curr_state = 1
        else: # self.curr_state == 1
            if tag == "span" and attrs[0][0] == cons.TAG_STYLE:
                if "font-weight" in attrs[0][1]: self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                elif "font-style" in attrs[0][1] and cons.TAG_PROP_ITALIC in attrs[0][1]: self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                elif "text-decoration" in attrs[0][1] and cons.TAG_UNDERLINE in attrs[0][1]: self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                elif "color" in attrs[0][1]:
                    match = re.match("(?<=^).+:(.+);(?=$)", attrs[0][1])
                    if match != None: self.curr_attributes[cons.TAG_FOREGROUND] = match.group(1).strip()
            elif tag == "a" and len(attrs) > 0:
                link_url = attrs[0][1]
                if len(link_url) > 7:
                    self.curr_attributes[cons.TAG_LINK] = get_internal_link_from_http_url(link_url)
            elif tag == "br":
                # this is a data block composed only by an endline
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                self.chars_counter += 1
            elif tag == "hr":
                # this is a data block composed only by an horizontal rule
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.h_rule+cons.CHAR_NEWLINE)
                self.chars_counter += len(self.h_rule)+2
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+cons.CHAR_LISTBUL+cons.CHAR_SPACE)
                self.chars_counter += 3

    def handle_endtag(self, tag):
        """Encountered the end of a tag"""
        if self.curr_state == 0: return
        if tag == "p":
            # this is a data block composed only by an endline
            self.rich_text_serialize(cons.CHAR_NEWLINE)
            self.chars_counter += 1
        elif tag == "span":
            self.curr_attributes[cons.TAG_WEIGHT] = ""
            self.curr_attributes[cons.TAG_STYLE] = ""
            self.curr_attributes[cons.TAG_UNDERLINE] = ""
            self.curr_attributes[cons.TAG_FOREGROUND] = ""
        elif tag == "a": self.curr_attributes[cons.TAG_LINK] = ""
        elif tag == "body":
            self.rich_text_serialize(cons.CHAR_NEWLINE)
            self.chars_counter += 1

    def handle_data(self, data):
        """Found Data"""
        if self.curr_state == 0 or data in [cons.CHAR_NEWLINE, cons.CHAR_NEWLINE*2]: return
        data = data.replace(cons.CHAR_NEWLINE, "")
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
        """Returns a CherryTree string Containing the Basket Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.latest_span = ""
        self.start_parsing()
        return self.dom.toxml()


class KnowitHandler(HTMLParser.HTMLParser):
    """The Handler of the Knowit File Parsing"""

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

    def parse_string_lines(self, file_descriptor):
        """Parse the string line by line"""
        self.curr_xml_state = 0
        self.curr_node_name = ""
        self.curr_node_content = ""
        self.curr_node_level = 0
        self.former_node_level = -1
        # 0: waiting for \NewEntry or \CurrentEntry
        # 1: gathering node content
        for text_line in file_descriptor:
            text_line = text_line.decode(cons.STR_UTF8, cons.STR_IGNORE)
            if self.curr_xml_state == 0:
                if (len(text_line) > 10 and text_line[:10] == "\NewEntry ")\
                or (len(text_line) > 14 and text_line[:14] == "\CurrentEntry "):
                    if text_line[1] == "N": text_to_parse = text_line[10:-1]
                    else: text_to_parse = text_line[14:-1]
                    match = re.match("(\d+) (.*)$", text_to_parse)
                    if not match: print '%s' % text_to_parse
                    self.curr_node_level = int(match.group(1))
                    self.curr_node_name = match.group(2)
                    #print "node name = '%s', node level = %s" % (self.curr_node_name, self.curr_node_level)
                    if self.curr_node_level <= self.former_node_level:
                        for count in range(self.former_node_level - self.curr_node_level):
                            self.nodes_list.pop()
                        self.nodes_list.pop()
                    self.former_node_level = self.curr_node_level
                    self.curr_node_content = ""
                    self.curr_xml_state = 1
                    self.nodes_list.append(self.dom.createElement("node"))
                    self.nodes_list[-1].setAttribute("name", self.curr_node_name)
                    self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
                    self.nodes_list[-2].appendChild(self.nodes_list[-1])
                else: self.curr_node_name += text_line.replace(cons.CHAR_CR, "").replace(cons.CHAR_NEWLINE, "") + cons.CHAR_SPACE
            elif self.curr_xml_state == 1:
                if len(text_line) > 14 and text_line[:14] == "</body></html>":
                    # node content end
                    self.curr_xml_state = 0
                    self.curr_html_state = 0
                    self.feed(self.curr_node_content.decode(cons.STR_UTF8, cons.STR_IGNORE))
                elif self.curr_node_content == "" and text_line == cons.CHAR_NEWLINE:
                    # empty node
                    self.curr_xml_state = 0
                    self.curr_html_state = 0
                else: self.curr_node_content += text_line

    def handle_starttag(self, tag, attrs):
        """Encountered the beginning of a tag"""
        if self.curr_html_state == 0:
            if tag == "body": self.curr_html_state = 1
        else: # self.curr_html_state == 1
            if tag == "span" and attrs[0][0] == cons.TAG_STYLE:
                if "font-weight" in attrs[0][1]: self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                elif "font-style" in attrs[0][1] and cons.TAG_PROP_ITALIC in attrs[0][1]: self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                elif "text-decoration" in attrs[0][1] and cons.TAG_UNDERLINE in attrs[0][1]: self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                elif "color" in attrs[0][1]:
                    match = re.match("(?<=^).+:(.+)(?=$)", attrs[0][1])
                    if match != None: self.curr_attributes[cons.TAG_FOREGROUND] = match.group(1).strip()
            elif tag == "br":
                # this is a data block composed only by an endline
                self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+cons.CHAR_LISTBUL+cons.CHAR_SPACE)

    def handle_endtag(self, tag):
        """Encountered the end of a tag"""
        if self.curr_html_state == 0: return
        if tag == "p":
            # this is a data block composed only by an endline
            self.rich_text_serialize(cons.CHAR_NEWLINE)
        elif tag == "span":
            self.curr_attributes[cons.TAG_WEIGHT] = ""
            self.curr_attributes[cons.TAG_STYLE] = ""
            self.curr_attributes[cons.TAG_UNDERLINE] = ""
            self.curr_attributes[cons.TAG_FOREGROUND] = ""

    def handle_data(self, data):
        """Found Data"""
        if self.curr_html_state == 0 or data in [cons.CHAR_NEWLINE, cons.CHAR_NEWLINE*2]: return
        data = data.replace(cons.CHAR_NEWLINE, "")
        self.rich_text_serialize(data)

    def handle_entityref(self, name):
        """Found Entity Reference like &name;"""
        if self.curr_html_state == 0: return
        if name in htmlentitydefs.name2codepoint:
            unicode_char = unichr(htmlentitydefs.name2codepoint[name])
            self.rich_text_serialize(unicode_char)

    def get_cherrytree_xml(self, file_descriptor):
        """Returns a CherryTree string Containing the Knowit Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.latest_span = ""
        self.parse_string_lines(file_descriptor)
        return self.dom.toxml()


class KeynoteHandler:
    """The Handler of the Treepad File Parsing"""

    def __init__(self):
        """Machine boot"""
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

    def parse_string_lines(self, file_descriptor):
        """Parse the string line by line"""
        self.curr_state = 0
        self.curr_node_name = ""
        self.curr_node_content = ""
        self.curr_node_level = 0
        self.former_node_level = -1
        self.in_picture = False
        # 0: waiting for LV=
        # 1: waiting for ND=
        # 2: waiting for %:
        # 3: gathering node content
        for text_line in file_descriptor:
            text_line = text_line.decode(cons.STR_UTF8, cons.STR_IGNORE)
            if self.curr_state == 0:
                if text_line.startswith("LV="):
                    print "node level:", text_line[3:]
                    self.curr_state = 1
                    self.curr_node_level = int(text_line[3:])
                    if self.curr_node_level <= self.former_node_level:
                        for count in range(self.former_node_level - self.curr_node_level):
                            self.nodes_list.pop()
                        self.nodes_list.pop()
                    self.former_node_level = self.curr_node_level
            elif self.curr_state == 1:
                if text_line.startswith("ND="):
                    print "node name:", text_line[3:]
                    self.curr_state = 2
                    self.curr_node_name = text_line[3:]
            elif self.curr_state == 2:
                if text_line.startswith("%:"):
                    self.curr_state = 3
                    self.curr_node_content = ""
                    self.nodes_list.append(self.dom.createElement("node"))
                    self.nodes_list[-1].setAttribute("name", self.curr_node_name)
                    self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
                    self.nodes_list[-2].appendChild(self.nodes_list[-1])
                    self.node_br_ok = False
            elif self.curr_state == 3:
                if text_line.startswith("%-") or text_line.startswith("%%"):
                    self.curr_state = 0
                    self.rich_text_serialize(self.curr_node_content)
                else: self.write_line_text(text_line.replace(cons.CHAR_CR, "").replace(cons.CHAR_NEWLINE, ""))
    
    def check_pending_text_to_tag(self):
        """Check if there's text to process before opening tag"""
        if self.curr_node_content:
            self.rich_text_serialize(self.curr_node_content)
            self.curr_node_content = ""
    
    def write_line_text(self, text_line):
        """Write Stripped Line Content"""
        print "'%s'" % text_line
        #if text_line.endswith("}"): return
        curr_state = 0
        dummy_loop = 0
        in_br_num = 0
        in_br_read_data = False
        for i, curr_char in enumerate(text_line):
            if dummy_loop > 0:
                dummy_loop -= 1
                continue
            if self.in_picture:
                if curr_char == cons.CHAR_BR_CLOSE: self.in_picture = False
                else: continue
            if curr_char == cons.CHAR_BSLASH:
                if text_line[i+1:].startswith(cons.CHAR_BSLASH):
                    self.curr_node_content += cons.CHAR_BSLASH
                    dummy_loop = 1
                    curr_state = 0
                elif text_line[i+1:].startswith(cons.CHAR_BR_OPEN):
                    self.curr_node_content += cons.CHAR_BR_OPEN
                    dummy_loop = 1
                    curr_state = 0
                elif text_line[i+1:].startswith(cons.CHAR_BR_CLOSE):
                    self.curr_node_content += cons.CHAR_BR_CLOSE
                    dummy_loop = 1
                    curr_state = 0
                elif text_line[i+1:] == "par":
                    self.curr_node_content += cons.CHAR_NEWLINE
                    break
                elif text_line[i+1:].startswith("line"):
                    dummy_loop = 4
                    self.curr_node_content += cons.CHAR_NEWLINE + 3*cons.CHAR_SPACE
                elif text_line[i+1:].startswith("pntext"):
                    if text_line[i+8:i+10] == "f1":
                        dummy_loop = 10
                        curr_state = 0
                        in_br_read_data = True
                    elif text_line[i+8:i+10] == "f2":
                        dummy_loop = 9
                        self.curr_node_content += cons.CHAR_LISTBUL + cons.CHAR_SPACE
                    else: print text_line[i+8:i+10]
                elif (text_line[i+1:].startswith("b"+cons.CHAR_SPACE) or text_line[i+1:].startswith("b"+cons.CHAR_BSLASH)):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                    if text_line[i+2:i+3] == cons.CHAR_SPACE:
                        dummy_loop = 2
                    else:
                        dummy_loop = 1
                    curr_state = 0
                elif (text_line[i+1:].startswith("i"+cons.CHAR_SPACE) or text_line[i+1:].startswith("i"+cons.CHAR_BSLASH)):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                    if text_line[i+2:i+3] == cons.CHAR_SPACE:
                        dummy_loop = 2
                    else:
                        dummy_loop = 1
                    curr_state = 0
                elif (text_line[i+1:].startswith("ul"+cons.CHAR_SPACE) or text_line[i+1:].startswith("ul"+cons.CHAR_BSLASH)):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                    if text_line[i+3:i+4] == cons.CHAR_SPACE:
                        dummy_loop = 3
                    else:
                        dummy_loop = 2
                    curr_state = 0
                elif (text_line[i+1:].startswith("strike"+cons.CHAR_SPACE) or text_line[i+1:].startswith("strike"+cons.CHAR_BSLASH)):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
                    if text_line[i+7:i+8] == cons.CHAR_SPACE:
                        dummy_loop = 7
                    else:
                        dummy_loop = 6
                    curr_state = 0
                elif text_line[i+1:].startswith("b0"):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_WEIGHT] = ""
                    dummy_loop = 2
                    curr_state = 0
                elif text_line[i+1:].startswith("i0"):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_STYLE] = ""
                    dummy_loop = 2
                    curr_state = 0
                elif text_line[i+1:].startswith("ulnone"):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_UNDERLINE] = ""
                    dummy_loop = 6
                    curr_state = 0
                elif text_line[i+1:].startswith("strike0"):
                    self.check_pending_text_to_tag()
                    self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
                    dummy_loop = 7
                    curr_state = 0
                elif text_line[i+1:].startswith("pict"):
                    self.in_picture = True
                    curr_state = 1
                else:
                    curr_state = 1
            elif curr_char == cons.CHAR_BR_OPEN:
                if self.node_br_ok: in_br_num += 1
                else: self.node_br_ok = True
            elif curr_char == cons.CHAR_BR_CLOSE:
                in_br_num -= 1
                curr_state = 0
                in_br_read_data = False
            elif in_br_read_data and curr_char == cons.CHAR_PARENTH_CLOSE:
                self.curr_node_content += "." + cons.CHAR_SPACE
            else:
                if in_br_num == 0 or in_br_read_data:
                    if curr_state == 0:
                        self.curr_node_content += curr_char
                    elif curr_state == 1:
                        if curr_char == cons.CHAR_SPACE:
                            curr_state = 0
    
    def get_cherrytree_xml(self, file_descriptor):
        """Returns a CherryTree string Containing the Treepad Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.parse_string_lines(file_descriptor)
        return self.dom.toxml()


class TreepadHandler:
    """The Handler of the Treepad File Parsing"""

    def __init__(self):
        """Machine boot"""
        self.xml_handler = machines.XMLHandler(self)

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        self.nodes_list[-1].appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)

    def parse_string_lines(self, file_descriptor):
        """Parse the string line by line"""
        self.curr_state = 0
        self.curr_node_name = ""
        self.curr_node_content = ""
        self.curr_node_level = 0
        self.former_node_level = -1
        # 0: waiting for <node>
        # 1: waiting for node name
        # 2: waiting for node level
        # 3: gathering node content
        for text_line in file_descriptor:
            text_line = text_line.decode(cons.STR_UTF8, cons.STR_IGNORE)
            if self.curr_state == 0:
                if len(text_line) > 5 and text_line[:6] == "<node>": self.curr_state = 1
            elif self.curr_state == 1:
                #print "node name", text_line
                self.curr_node_name = text_line.replace(cons.CHAR_CR, "").replace(cons.CHAR_NEWLINE, "")
                self.curr_state = 2
            elif self.curr_state == 2:
                #print "node level", text_line
                if re.match("\d+", text_line):
                    self.curr_node_level = int(text_line)
                    #print self.curr_node_level
                    if self.curr_node_level <= self.former_node_level:
                        for count in range(self.former_node_level - self.curr_node_level):
                            self.nodes_list.pop()
                        self.nodes_list.pop()
                    self.former_node_level = self.curr_node_level
                    self.curr_node_content = ""
                    self.curr_state = 3
                    self.nodes_list.append(self.dom.createElement("node"))
                    self.nodes_list[-1].setAttribute("name", self.curr_node_name)
                    self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
                    self.nodes_list[-2].appendChild(self.nodes_list[-1])
                else: self.curr_node_name += text_line.replace(cons.CHAR_CR, "").replace(cons.CHAR_NEWLINE, "") + cons.CHAR_SPACE
            elif self.curr_state == 3:
                if len(text_line) > 9 and text_line[:10] == "<end node>":
                    self.curr_state = 0
                    self.rich_text_serialize(self.curr_node_content)
                else: self.curr_node_content += text_line.replace(cons.CHAR_CR, "").replace(cons.CHAR_NEWLINE, "") + cons.CHAR_NEWLINE

    def get_cherrytree_xml(self, file_descriptor):
        """Returns a CherryTree string Containing the Treepad Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.parse_string_lines(file_descriptor)
        return self.dom.toxml()


class MempadHandler:
    """The Handler of the Mempad File Parsing"""

    def __init__(self):
        """Machine boot"""
        self.xml_handler = machines.XMLHandler(self)

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        self.nodes_list[-1].appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)

    def parse_binary_bytes(self, file_descriptor):
        """Parse the binary bytes one by one"""
        self.curr_state = 0
        self.curr_node_name = ""
        self.curr_node_content = ""
        self.curr_node_level = 0
        self.former_node_level = -1
        # 0: waiting for first node level
        # 1: filling node name
        # 2: filling node content
        # 3: waiting for subsequent node level
        all_data = file_descriptor.read()
        for element in all_data:
            if self.curr_state == 0:
                if ord(element) == 1:
                    self.curr_node_level = 1
                    self.curr_state = 1
            elif self.curr_state == 1:
                if ord(element) == 0:
                    #print self.curr_node_name
                    self.curr_node_content = ""
                    self.curr_state = 2
                    if self.curr_node_level <= self.former_node_level:
                        for count in range(self.former_node_level - self.curr_node_level):
                            self.nodes_list.pop()
                        self.nodes_list.pop()
                    self.former_node_level = self.curr_node_level
                    self.nodes_list.append(self.dom.createElement("node"))
                    self.nodes_list[-1].setAttribute("name", self.curr_node_name)
                    self.nodes_list[-1].setAttribute("prog_lang", cons.CUSTOM_COLORS_ID)
                    self.nodes_list[-2].appendChild(self.nodes_list[-1])
                else: self.curr_node_name += element
            elif self.curr_state == 2:
                if ord(element) == 0:
                    #print self.curr_node_content
                    self.curr_state = 3
                    self.rich_text_serialize(self.curr_node_content)
                else: self.curr_node_content += element
            else:
                self.curr_node_level = ord(element)
                self.curr_node_name = ""
                self.curr_state = 1

    def get_cherrytree_xml(self, file_descriptor):
        """Returns a CherryTree string Containing the Mempad Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.parse_binary_bytes(file_descriptor)
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
            elif tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
            elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
            elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
            elif tag == "s": self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
            elif tag == "span" and attrs[0][0] == cons.TAG_STYLE:
                match = re.match("(?<=^)(.+):(.+)(?=$)", attrs[0][1])
                if match != None:
                    if match.group(1) == "color":
                        self.curr_attributes[cons.TAG_FOREGROUND] = match.group(2).strip()
                        self.latest_span = cons.TAG_FOREGROUND
                    elif match.group(1) == "background-color":
                        self.curr_attributes[cons.TAG_BACKGROUND] = match.group(2).strip()
                        self.latest_span = cons.TAG_BACKGROUND
            elif tag == "a" and len(attrs) > 0:
                link_url = attrs[0][1]
                if len(link_url) > 7:
                    self.curr_attributes[cons.TAG_LINK] = get_internal_link_from_http_url(link_url)
            elif tag == "br":
                # this is a data block composed only by an endline
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                self.chars_counter += 1
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+cons.CHAR_LISTBUL+cons.CHAR_SPACE)
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
                            self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
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
                            self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
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
            elif tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = ""
            elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = ""
            elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = ""
            elif tag == "s": self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
            elif tag == "span":
                if self.latest_span == cons.TAG_FOREGROUND: self.curr_attributes[cons.TAG_FOREGROUND] = ""
                elif self.latest_span == cons.TAG_BACKGROUND: self.curr_attributes[cons.TAG_BACKGROUND] = ""
            elif tag == "a": self.curr_attributes[cons.TAG_LINK] = ""
        elif tag == "dl":
            # backward one level in nodes list
            for pixbuf_element in self.pixbuf_vector:
                self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
            self.pixbuf_vector = []
            self.chars_counter = 0
            self.nodes_list.pop()

    def handle_data(self, data):
        """Found Data"""
        if self.curr_state == 0 or data in [cons.CHAR_NEWLINE, cons.CHAR_NEWLINE*2]: return
        if self.curr_state == 1:
            # state 1 got title
            self.curr_title += data
        elif self.curr_state == 2:
            # state 2 got data
            clean_data = data.replace(cons.CHAR_NEWLINE, "")
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
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
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
        self.feed(input_string.decode(cons.STR_UTF8, cons.STR_IGNORE))
        return self.dom.toxml()


class HTMLFromClipboardHandler(HTMLParser.HTMLParser):
    """The Handler of the HTML received from clipboard"""

    def __init__(self, dad):
        """Machine boot"""
        self.dad = dad
        self.monitored_tags = ["p", "b", "i", "u", "s", cons.TAG_PROP_H1, cons.TAG_PROP_H2, cons.TAG_PROP_H3, "span", "font"]
        HTMLParser.HTMLParser.__init__(self)

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
                self.curr_rows_span = []
                self.curr_table_header = False
            elif tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
            elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
            elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
            elif tag == "s": self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
            elif tag == cons.TAG_STYLE: self.curr_state = 0
            elif tag == "span":
                for attr in attrs:
                    if attr[0] == cons.TAG_STYLE:
                        attributes = attr[1].split(";")
                        for attribute in attributes:
                            #print "attribute", attribute
                            match = re.match("(?<=^)(.+):(.+)(?=$)", attribute)
                            if match != None:
                                if match.group(1) == "color":
                                    attribute = self.get_rgb_gtk_attribute(match.group(2).strip())
                                    if attribute:
                                        self.curr_attributes[cons.TAG_FOREGROUND] = attribute
                                        self.latest_span.append(cons.TAG_FOREGROUND)
                                elif match.group(1) in [cons.TAG_BACKGROUND, "background-color"]:
                                    attribute = self.get_rgb_gtk_attribute(match.group(2).strip())
                                    if attribute:
                                        self.curr_attributes[cons.TAG_BACKGROUND] = attribute
                                        self.latest_span.append(cons.TAG_BACKGROUND)
                                elif match.group(1) == "text-decoration":
                                    if match.group(2).strip() in [cons.TAG_UNDERLINE, "underline;"]:
                                        self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                                        self.latest_span.append(cons.TAG_UNDERLINE)
                                elif match.group(1) == "font-weight":
                                    if match.group(2).strip() in ["bolder", "700"]:
                                        self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                                        self.latest_span.append(cons.TAG_WEIGHT)
            elif tag == "font":
                for attr in attrs:
                    if attr[0] == "color":
                        attribute = self.get_rgb_gtk_attribute(attr[1].strip())
                        if attribute:
                            self.curr_attributes[cons.TAG_FOREGROUND] = attribute
                            self.latest_font = cons.TAG_FOREGROUND
            elif tag in [cons.TAG_PROP_H1, cons.TAG_PROP_H2, cons.TAG_PROP_H3]:
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                if tag == cons.TAG_PROP_H1: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H1
                elif tag == cons.TAG_PROP_H2: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H2
                else: self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                for attr in attrs:
                    if attr[0] == "align": self.curr_attributes[cons.TAG_JUSTIFICATION] = attr[1].strip().lower()
            elif tag == "a" and len(attrs) > 0:
                #print "attrs", attrs
                for attr in attrs:
                    if attr[0] == "href":
                        link_url = attr[1]
                        if len(link_url) > 7:
                            self.curr_attributes[cons.TAG_LINK] = get_internal_link_from_http_url(link_url)
                        break
            elif tag == "br": self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "ol": self.curr_list_type = ["o", 1]
            elif tag == "ul": self.curr_list_type = ["u", 0]
            elif tag == "li":
                if self.curr_list_type[0] == "u": self.rich_text_serialize(cons.CHAR_LISTBUL+cons.CHAR_SPACE)
                else:
                    self.rich_text_serialize("%s. " % self.curr_list_type[1])
                    self.curr_list_type[1] += 1
            elif tag == "img" and len(attrs) > 0:
                img_path = attrs[0][1]
                self.insert_image(img_path)
            elif tag == "pre": self.pre_tag = "p"
        elif self.curr_state == 2:
            if tag == "table": # nested tables
                self.curr_table = []
                self.curr_rows_span = []
                self.curr_table_header = False
            elif tag == "tr": self.curr_table.append([])
            elif tag in ["td", "th"]:
                self.curr_cell = ""
                self.curr_rowspan = 1
                for attr in attrs:
                    if attr[0] == "rowspan":
                        self.curr_rowspan = int(attr[1])
                        break
                if tag == "th": self.curr_table_header = True
            elif tag == "img" and len(attrs) > 0:
                img_path = attrs[0][1]
                self.insert_image(img_path, cons.CHAR_NEWLINE*2)
            elif tag == "br": self.curr_cell += cons.CHAR_NEWLINE
            elif tag == "ol": self.curr_list_type = ["o", 1]
            elif tag == "ul": self.curr_list_type = ["u", 0]
            elif tag == "li":
                if self.curr_list_type[0] == "u": self.curr_cell += cons.CHAR_LISTBUL+cons.CHAR_SPACE
                else:
                    self.curr_cell += "%s. " % self.curr_list_type[1]
                    self.curr_list_type[1] += 1

    def insert_image(self, img_path, trailing_chars=""):
        """Insert Image in Buffer"""
        try:
            self.dad.statusbar.push(self.dad.statusbar_context_id, _("Downloading") + " %s ..." % img_path)
            while gtk.events_pending(): gtk.main_iteration()
            url_desc = urllib2.urlopen(img_path, timeout=3)
            image_file = url_desc.read()
            pixbuf_loader = gtk.gdk.PixbufLoader()
            pixbuf_loader.write(image_file)
            pixbuf_loader.close()
            pixbuf = pixbuf_loader.get_pixbuf()
            self.dad.xml_handler.pixbuf_element_to_xml([0, pixbuf, cons.TAG_PROP_LEFT], self.curr_dom_slot, self.dom)
            self.dad.statusbar.pop(self.dad.statusbar_context_id)
            if trailing_chars: self.rich_text_serialize(trailing_chars)
        except:
            print "failed download of", img_path
            self.dad.statusbar.pop(self.dad.statusbar_context_id)

    def handle_endtag(self, tag):
        """Encountered the end of a tag"""
        if tag in self.monitored_tags: self.in_a_tag -= 1
        if self.curr_state == 0:
            if tag == cons.TAG_STYLE: self.curr_state = 1
        if self.curr_state == 1:
            if tag == "p": self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = ""
            elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = ""
            elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = ""
            elif tag == "s": self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
            elif tag == "span":
                if cons.TAG_FOREGROUND in self.latest_span: self.curr_attributes[cons.TAG_FOREGROUND] = ""
                if cons.TAG_BACKGROUND in self.latest_span: self.curr_attributes[cons.TAG_BACKGROUND] = ""
                if cons.TAG_UNDERLINE in self.latest_span: self.curr_attributes[cons.TAG_UNDERLINE] = ""
                if cons.TAG_WEIGHT in self.latest_span: self.curr_attributes[cons.TAG_WEIGHT] = ""
                self.latest_span = []
            elif tag == "font":
                if self.latest_font == cons.TAG_FOREGROUND: self.curr_attributes[cons.TAG_FOREGROUND] = ""
            elif tag in [cons.TAG_PROP_H1, cons.TAG_PROP_H2, cons.TAG_PROP_H3]:
                self.curr_attributes[cons.TAG_SCALE] = ""
                self.curr_attributes[cons.TAG_JUSTIFICATION] = ""
                self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "a": self.curr_attributes[cons.TAG_LINK] = ""
            elif tag == "li": self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "pre": self.pre_tag = ""
        elif self.curr_state == 2:
            if tag in ["td", "th"]:
                self.curr_table[-1].append(self.curr_cell)
                if len(self.curr_table) == 1: self.curr_rows_span.append(self.curr_rowspan)
                else:
                    index = len(self.curr_table[-1])-1
                    #print "self.curr_rows_span", self.curr_rows_span
                    while index >= len(self.curr_rows_span):
                        # rowspan in very first row
                        self.curr_rows_span.append(1)
                        self.curr_table[-2].append("")
                    if self.curr_rows_span[index] == 1: self.curr_rows_span[index] = self.curr_rowspan
                    else:
                        unos_found = 0
                        while unos_found < 2:
                            if not unos_found: self.curr_table[-1].insert(index, "")
                            else: self.curr_table[-1].append("")
                            self.curr_rows_span[index] -= 1
                            index += 1
                            if index == len(self.curr_rows_span): break
                            if self.curr_rows_span[index] == 1:
                                unos_found += 1
                                if unos_found < 2:
                                    index += 1
                                    if index == len(self.curr_rows_span): break
                                    if self.curr_rows_span[index] == 1: unos_found += 1
            elif tag == "table":
                self.curr_state = 1
                if len(self.curr_table) == 1 and len(self.curr_table[0]) == 1:
                    # it's a codebox
                    codebox_dict = {
                    'frame_width': 300,
                    'frame_height': 150,
                    'width_in_pixels': True,
                    'syntax_highlighting': cons.CUSTOM_COLORS_ID,
                    'highlight_brackets': False,
                    'show_line_numbers': False,
                    'fill_text': self.curr_table[0][0]
                    }
                    self.dad.xml_handler.codebox_element_to_xml([0, codebox_dict, cons.TAG_PROP_LEFT], self.curr_dom_slot)
                else:
                    # it's a table
                    if not self.curr_table_header: self.curr_table.append([_("click me")]*len(self.curr_table[0]))
                    else: self.curr_table.append(self.curr_table.pop(0))
                    table_dict = {'col_min': cons.TABLE_DEFAULT_COL_MIN,
                                  'col_max': cons.TABLE_DEFAULT_COL_MAX,
                                  'matrix': self.curr_table}
                    self.dad.xml_handler.table_element_to_xml([0, table_dict, cons.TAG_PROP_LEFT], self.curr_dom_slot)
                self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag in ["p", "li"]: self.curr_cell += cons.CHAR_NEWLINE

    def handle_data(self, data):
        """Found Data"""
        if self.curr_state == 0: return
        if self.pre_tag == "p":
            self.rich_text_serialize(data)
            return
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
        self.latest_span = []
        self.latest_font = ""
        self.curr_cell = ""
        self.in_a_tag = 0
        self.curr_list_type = ["u", 0]
        self.pre_tag = ""
        # curr_state 0: standby, taking no data
        # curr_state 1: receiving rich text
        # curr_state 2: receiving table or codebox data
        input_string = input_string.decode(cons.STR_UTF8, cons.STR_IGNORE)
        if not HTMLCheck().is_html_ok(input_string):
            input_string = cons.HTML_HEADER % "" + input_string + cons.HTML_FOOTER
        #print "###############"
        #print input_string
        #print "###############"
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
