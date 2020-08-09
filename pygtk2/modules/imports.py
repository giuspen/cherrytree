# -*- coding: UTF-8 -*-
#
#       imports.py
#
#       Copyright 2009-2019 Giuseppe Penone <giuspen@gmail.com>
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

import HTMLParser
import htmlentitydefs
import gtk
import gio
import os
import xml.dom.minidom
import re
import base64
import urllib2
import binascii
import shutil
import glob
import time
import cons
import machines
import support
import exports


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
        if support.get_first_chars_of_string_at_offset_are(plain_text, start_offset, cons.WEB_LINK_STARTERS):
            end_offset = start_offset + 3
            while (end_offset < max_end_offset)\
            and (plain_text[end_offset] not in [cons.CHAR_SPACE, cons.CHAR_NEWLINE]):
                end_offset += 1
            web_links.append([start_offset, end_offset])
            start_offset = end_offset + 1
        else: start_offset += 1
    return web_links

def epim_html_file_to_hier_files(filepath):
    """From EPIM HTML File to Folder of HTML Files"""
    epim_dir = os.path.join(cons.TMP_FOLDER, "EPIM")
    if os.path.isdir(epim_dir): shutil.rmtree(epim_dir)
    curr_state = 0
    nodes_levels = []
    nodes_names = []
    nodes_content = []
    curr_node_idx = 0
    html_prefix = ""
    with open(filepath, 'r') as fd:
        for html_row in fd:
            if curr_state == 0:
                html_prefix += html_row
                if "<body>" in html_row: curr_state = 1
            elif curr_state == 1:
                if not "href" in html_row:
                    curr_state = 2
                    continue
                nodes_levels.append(html_row.count("&nbsp; &nbsp;"))
            elif curr_state == 2:
                if html_row.startswith("<span class=rvts1>"):
                    nodes_names.append(support.clean_from_chars_not_for_filename(html_row[18:-14]))
                    nodes_content.append("")
                    curr_state = 3
            elif curr_state == 3:
                if html_row.startswith("<ul class=list0>"):
                    curr_node_idx += 1
                    curr_state = 2
                elif html_row.startswith("</body></html>"):
                    curr_node_idx += 1
                    curr_state = 4
                else: nodes_content[curr_node_idx] += html_row
            else: break
    print len(nodes_levels), nodes_levels
    print len(nodes_names), nodes_names
    filedir, filename = os.path.split(filepath)
    dest_folder = [epim_dir]
    for i,node_name in enumerate(nodes_names):
        if i>0:
            if nodes_levels[i] > nodes_levels[i-1]:
                dest_folder.append(os.path.join(dest_folder[-1], nodes_names[i-1]))
                print dest_folder[-1]
            elif nodes_levels[i] < nodes_levels[i-1]:
                for back_jumps in range(nodes_levels[i-1] - nodes_levels[i]):
                    dest_folder.pop()
        if not os.path.isdir(dest_folder[-1]):
            os.makedirs(dest_folder[-1])
            for filepath_to_cpy in glob.glob(os.path.join(filedir, "*")):
                if filepath_to_cpy == filepath: continue
                shutil.copy(filepath_to_cpy, dest_folder[-1])
        dest_filepath = os.path.join(dest_folder[-1], support.clean_from_chars_not_for_filename(node_name)+".htm")
        with open(dest_filepath, 'w') as fd:
            fd.write(html_prefix+nodes_content[i]+"</body></html>")
    return epim_dir


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
                self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
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

    def __init__(self, dad):
        """Machine boot"""
        HTMLParser.HTMLParser.__init__(self)
        self.dad = dad
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
                self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
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
            elif tag in ["img", "v:imagedata"] and len(attrs) > 0:
                dic_attrs = dict(a for a in attrs)
                img_path = dic_attrs.get('src', "")
                pixbuf = None
                if os.path.isfile(img_path):
                    pixbuf = gtk.gdk.pixbuf_new_from_file(img_path)
                else:
                    try:
                        url_desc = urllib2.urlopen(img_path, timeout=3)
                        image_file = url_desc.read()
                        pixbuf_loader = gtk.gdk.PixbufLoader()
                        pixbuf_loader.write(image_file)
                        pixbuf_loader.close()
                        pixbuf = pixbuf_loader.get_pixbuf()
                    except: pass
                if pixbuf:
                    self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                    self.chars_counter += 1
                else: print "%s insert fail" % img_path
            elif tag == "br":
                # this is a data block composed only by an endline
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                self.chars_counter += 1
            elif tag == "hr":
                # this is a data block composed only by an horizontal rule
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.h_rule+cons.CHAR_NEWLINE)
                self.chars_counter += len(self.dad.h_rule)+2
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.chars_listbul[0]+cons.CHAR_SPACE)
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
        self.start_parsing(tuxcards_string)
        return self.dom.toxml()


class KeepnoteHandler(HTMLParser.HTMLParser):
    """The Handler of the KeepNote Folder Parsing"""

    def __init__(self, dad, folderpath):
        """Machine boot"""
        HTMLParser.HTMLParser.__init__(self)
        self.dad = dad
        self.folderpath = folderpath
        self.xml_handler = machines.XMLHandler(self)
        self.prev_attributes = {}
        self.prev_accumulator = []

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        # accumulate data (+= string is to slow) to udpate element lately
        if self.prev_attributes == self.curr_attributes:
            if self.nodes_list[-1].lastChild != None:
                self.prev_accumulator.append(text_data)
                return

        # attributes changes, so finally can update prev element
        if (self.prev_accumulator):
            self.nodes_list[-1].lastChild.firstChild.data += "".join(self.prev_accumulator)
            self.prev_accumulator = []

        dom_iter = self.dom.createElement("rich_text")
        if self.curr_attributes:
            for tag_property in cons.TAG_PROPERTIES:
                if tag_property in self.curr_attributes:
                    if self.curr_attributes[tag_property] != "":
                        dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
                        
        self.nodes_list[-1].appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)
        self.prev_attributes = self.curr_attributes.copy()

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
        self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
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
        #for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.curr_attributes = {}
        self.prev_attributes = {}
        self.prev_accumulator = []
        self.latest_span = []
        self.pixbuf_vector = []
        self.curr_folder = node_folder
        self.chars_counter = 0
        self.feed(node_string.decode(cons.STR_UTF8, cons.STR_IGNORE))
        # after parsing
        if (self.prev_accumulator):
            self.nodes_list[-1].lastChild.firstChild.data += "".join(self.prev_accumulator)

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
                        self.latest_span.append(cons.TAG_FOREGROUND)
                    elif match.group(1) == "background-color":
                        self.curr_attributes[cons.TAG_BACKGROUND] = match.group(2).strip()
                        self.latest_span.append(cons.TAG_BACKGROUND)
            elif tag == "a" and len(attrs) > 0:
                link_url = attrs[0][1]
                if len(link_url) > 7:
                    self.curr_attributes[cons.TAG_LINK] = get_internal_link_from_http_url(link_url)
            elif tag in ["img", "v:imagedata"] and len(attrs) > 0:
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
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.h_rule+cons.CHAR_NEWLINE)
                self.chars_counter += len(self.dad.h_rule)+2
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.chars_listbul[0]+cons.CHAR_SPACE)
                self.chars_counter += 3

    def handle_endtag(self, tag):
        """Encountered the end of a tag"""
        if self.curr_state == 0: return
        if tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = ""
        elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = ""
        elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = ""
        elif tag == "strike": self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
        elif tag == "span":
            if self.latest_span:
                if self.latest_span[-1] == cons.TAG_FOREGROUND: self.curr_attributes[cons.TAG_FOREGROUND] = ""
                elif self.latest_span[-1] == cons.TAG_BACKGROUND: self.curr_attributes[cons.TAG_BACKGROUND] = ""
                del self.latest_span[-1]
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
        self.start_parsing()
        return self.dom.toxml()


class RedNotebookHandler():
    """The Handler of the RedNotebook Folder Parsing"""

    def __init__(self, dad, folderpath):
        """Machine boot"""
        self.folderpath = folderpath
        self.dad = dad
        self.xml_handler = machines.XMLHandler(self)

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        for tag_property in cons.TAG_PROPERTIES:
            if self.curr_attributes[tag_property] != "":
                dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
        self.nodes_list[-1].appendChild(dom_iter)
        self.chars_counter += len(text_data)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)

    def node_day_add(self, node_name, wiki_string):
        """Add a new Day node"""
        self.nodes_list.append(self.dom.createElement("node"))
        self.nodes_list[-1].setAttribute("name", node_name)
        self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
        self.nodes_list[-2].appendChild(self.nodes_list[-1])
        #
        self.pixbuf_vector = []
        self.chars_counter = 0
        self.node_wiki_parse(wiki_string, node_name)
        for pixbuf_element in self.pixbuf_vector:
            self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)

    def node_wiki_parse(self, wiki_string, node_name):
        """Parse the node wiki content"""
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.in_link = False
        self.in_image = False
        self.in_plain_link = False
        in_numbered_list = 0
        curr_pos = 0
        wiki_string = wiki_string.replace(2*cons.CHAR_NEWLINE, cons.CHAR_NEWLINE)
        wiki_string = wiki_string.replace(2*cons.CHAR_BSLASH, cons.CHAR_NEWLINE)
        wiki_string = wiki_string.replace(2*cons.CHAR_SQUOTE, cons.CHAR_SQUOTE)
        wiki_string = wiki_string.replace(cons.CHAR_NEWLINE+cons.CHAR_MINUS+cons.CHAR_SPACE, cons.CHAR_NEWLINE+self.dad.chars_listbul[0]+cons.CHAR_SPACE)
        max_pos = len(wiki_string)
        self.wiki_slot = ""
        def wiki_slot_flush():
            if self.wiki_slot:
                #print self.wiki_slot
                self.rich_text_serialize(self.wiki_slot)
                self.wiki_slot = ""
        probably_url = False
        in_hN = [False, False, False, False, False]
        while curr_pos < max_pos:
            curr_char = wiki_string[curr_pos:curr_pos+1]
            next_char = wiki_string[curr_pos+1:curr_pos+2] if curr_pos+1 < max_pos else cons.CHAR_SPACE
            third_char = wiki_string[curr_pos+2:curr_pos+3] if curr_pos+2 < max_pos else cons.CHAR_SPACE
            fourth_char = wiki_string[curr_pos+3:curr_pos+4] if curr_pos+3 < max_pos else cons.CHAR_SPACE

            if curr_char == cons.CHAR_NEWLINE:
                if next_char == "+" and third_char == cons.CHAR_SPACE:
                    in_numbered_list += 1
                    leading_chars_num = self.dad.lists_handler.get_leading_chars_num(in_numbered_list)
                    self.wiki_slot += cons.CHAR_NEWLINE + "%s. " % in_numbered_list
                    curr_pos += 3
                    continue
                elif in_numbered_list:
                    in_numbered_list = 0
                self.in_link = False
                self.in_image = False

            if self.in_plain_link:
                if curr_char in [cons.CHAR_SPACE, cons.CHAR_NEWLINE]:
                    self.curr_attributes[cons.TAG_LINK] = "webs %s" % self.wiki_slot
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_LINK] = ""
                    self.in_plain_link = False
                self.wiki_slot += curr_char
            elif self.in_image:
                #[""file://...."".png]
                if curr_char == cons.CHAR_SQ_BR_CLOSE:
                    valid_image = False
                    if self.wiki_slot.startswith("./"): self.wiki_slot = os.path.join(self.folderpath, self.wiki_slot[2:])
                    if os.path.isfile(self.wiki_slot):
                        try:
                            pixbuf = gtk.gdk.pixbuf_new_from_file(self.wiki_slot)
                            self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                            self.chars_counter += 1
                            valid_image = True
                        except: pass
                    if not valid_image: print "! error: '%s' is not a valid image" % self.wiki_slot
                    self.wiki_slot = ""
                    curr_pos += 1
                    self.in_image = False
                elif not curr_char in [cons.CHAR_DQUOTE]:
                    self.wiki_slot += curr_char
            elif self.in_link:
                #[rednotebook-1.12.tar.gz ""file:///home/giuspen/Downloads/rednotebook-1.12.tar.gz""]
                #[Google ""http://google.com""]
                if curr_char == cons.CHAR_SQ_BR_CLOSE:
                    if ' ""' in self.wiki_slot:
                        label_n_target = self.wiki_slot.split(' ""')
                        if len(label_n_target) == 2:
                            exp_filepath = label_n_target[1].replace(cons.CHAR_DQUOTE, "").replace("file://", "")
                            if exp_filepath.startswith("./"): exp_filepath = os.path.join(self.folderpath, exp_filepath[2:])
                            if exp_filepath.startswith("http") or exp_filepath.startswith("ftp") or exp_filepath.startswith("www.")\
                            and not cons.CHAR_SPACE in exp_filepath:
                                self.curr_attributes[cons.TAG_LINK] = "webs %s" % exp_filepath
                                self.rich_text_serialize(label_n_target[0])
                                self.curr_attributes[cons.TAG_LINK] = ""
                            elif cons.CHAR_SLASH in exp_filepath:
                                self.curr_attributes[cons.TAG_LINK] = "file %s" % base64.b64encode(exp_filepath)
                                self.rich_text_serialize(label_n_target[0])
                                self.curr_attributes[cons.TAG_LINK] = ""
                            else:
                                print "?", label_n_target
                            self.wiki_slot = ""
                            curr_pos += 1
                    self.in_link = False
                else: self.wiki_slot += curr_char
            elif curr_char == cons.CHAR_STAR and next_char == cons.CHAR_STAR:
                wiki_slot_flush()
                if self.curr_attributes[cons.TAG_WEIGHT]: self.curr_attributes[cons.TAG_WEIGHT] = ""
                else: self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                curr_pos += 1
            elif curr_char == cons.CHAR_SLASH and next_char == cons.CHAR_SLASH:
                if probably_url:
                    self.wiki_slot += curr_char
                    curr_pos += 1
                    continue
                wiki_slot_flush()
                if self.curr_attributes[cons.TAG_STYLE]: self.curr_attributes[cons.TAG_STYLE] = ""
                else: self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                curr_pos += 1
            elif curr_char == cons.CHAR_USCORE and next_char == cons.CHAR_USCORE:
                wiki_slot_flush()
                if self.curr_attributes[cons.TAG_UNDERLINE]: self.curr_attributes[cons.TAG_UNDERLINE] = ""
                else: self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                curr_pos += 1
            elif curr_char == cons.CHAR_MINUS and next_char == cons.CHAR_MINUS:
                wiki_slot_flush()
                if self.curr_attributes[cons.TAG_STRIKETHROUGH]: self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
                else: self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
                curr_pos += 1
            elif curr_char == cons.CHAR_GRAVE and next_char == cons.CHAR_GRAVE:
                wiki_slot_flush()
                if self.curr_attributes[cons.TAG_FAMILY]: self.curr_attributes[cons.TAG_FAMILY] = ""
                else: self.curr_attributes[cons.TAG_FAMILY] = cons.TAG_PROP_MONOSPACE
                curr_pos += 1
            elif curr_char == 'h' and next_char == 't' and third_char == 't' and fourth_char == 'p':
                wiki_slot_flush()
                self.wiki_slot += curr_char
                self.in_plain_link = True
            # =
            elif not in_hN[0] and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_SPACE:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H1
                curr_pos += 1
                in_hN[0] = True
                #print "H1sta"
            elif in_hN[0] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = ""
                curr_pos += 1
                in_hN[0] = False
                #print "H1end"
            ## ==
            elif not in_hN[1] and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_SPACE:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H2
                curr_pos += 2
                in_hN[1] = True
                #print "H2sta"
            elif in_hN[1] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = ""
                curr_pos += 2
                in_hN[1] = False
                #print "H2end"
            ## ===
            elif not in_hN[2] and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_SPACE:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                curr_pos += 3
                in_hN[2] = True
                #print "H3sta"
            elif in_hN[2] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = ""
                curr_pos += 3
                in_hN[2] = False
                #print "H3end"
            ## ====
            elif not in_hN[3] and curr_pos+4 < max_pos and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+5] == cons.CHAR_SPACE:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                curr_pos += 4
                in_hN[3] = True
                #print "H4sta"
            elif curr_pos+4 < max_pos and in_hN[3] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+5] == cons.CHAR_EQUAL:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = ""
                curr_pos += 4
                in_hN[3] = False
                #print "H4end"
            # =====
            elif not in_hN[4] and curr_pos+5 < max_pos and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+6] == (cons.CHAR_EQUAL+cons.CHAR_SPACE):
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                curr_pos += 5
                in_hN[4] = True
                #print "H5sta"
            elif curr_pos+5 < max_pos and in_hN[4] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+6] == 2*cons.CHAR_EQUAL:
                wiki_slot_flush()
                self.curr_attributes[cons.TAG_SCALE] = ""
                curr_pos += 5
                in_hN[4] = False
                #print "H5end"
            #
            elif curr_char == cons.CHAR_SQ_BR_OPEN and next_char == cons.CHAR_DQUOTE and third_char == cons.CHAR_DQUOTE and curr_pos+9 < max_pos and wiki_string[curr_pos+3:curr_pos+10] == "file://":
                wiki_slot_flush()
                curr_pos += 9
                self.in_image = True
            elif curr_char == cons.CHAR_SQ_BR_OPEN:
                wiki_slot_flush()
                self.in_link = True
            else:
                self.wiki_slot += curr_char
                #print self.wiki_slot
                if curr_char == ":" and next_char == cons.CHAR_SLASH:
                    probably_url = True
                elif curr_char in [cons.CHAR_SPACE, cons.CHAR_NEWLINE]:
                    probably_url = False
            curr_pos += 1
        wiki_slot_flush()

    def node_month_add(self, filename):
        """Add a new Month node"""
        #print filename
        self.nodes_list.append(self.dom.createElement("node"))
        self.nodes_list[-1].setAttribute("name", filename[:-4])
        self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
        self.nodes_list[-2].appendChild(self.nodes_list[-1])
        with open(os.path.join(self.folderpath, filename), "r") as fd:
            month_list = []
            def clean_markdown_start():
                if month_list[-1]["md"].startswith(cons.CHAR_SQUOTE):
                    month_list[-1]["md"] = month_list[-1]["md"][1:]
            def clean_markdown_end():
                if month_list:
                    if month_list[-1]["md"].endswith(cons.CHAR_SQUOTE+cons.CHAR_BR_CLOSE):
                        month_list[-1]["md"] = month_list[-1]["md"][:-2]
                    elif month_list[-1]["md"].endswith(cons.CHAR_BR_CLOSE):
                        month_list[-1]["md"] = month_list[-1]["md"][:-1]
                    else:
                        print "!! unexpected md end"
                        print month_list[-1]["md"]
            for text_line in fd:
                text_line = text_line.replace(cons.CHAR_NEWLINE, "").replace(cons.CHAR_CR, "")
                ret_match = re.search("^(\d+):\s{text:\s(.+)$", text_line)
                if ret_match:
                    # start of new day
                    clean_markdown_end()
                    month_list.append({"day": ret_match.group(1), "md": ret_match.group(2)})
                    clean_markdown_start()
                elif text_line == "{}": pass
                else:
                    curr_line = text_line if not text_line.startswith(4*cons.CHAR_SPACE) else text_line[4:]
                    curr_line = curr_line.replace(2*cons.CHAR_SQUOTE, cons.CHAR_SQUOTE)
                    month_list[-1]["md"] += cons.CHAR_NEWLINE + curr_line
            clean_markdown_end()
            for month_element in month_list:
                self.node_day_add(month_element["day"], month_element["md"])
                self.nodes_list.pop()
        self.nodes_list.pop()

    def get_cherrytree_xml(self):
        """Returns a CherryTree string Containing the RedNotebook Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        if os.path.isdir(os.path.join(self.folderpath, "data")):
            self.folderpath = os.path.join(self.folderpath, "data")
        for element in sorted(os.listdir(self.folderpath)):
            if element.endswith(".txt") and len(element) == 11:
                self.node_month_add(element)
        return self.dom.toxml()


class ZimHandler():
    """The Handler of the Zim Folder Parsing"""

    def __init__(self, dad, folderpath):
        """Machine boot"""
        self.folderpath = folderpath
        self.dad = dad
        self.xml_handler = machines.XMLHandler(self)

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        for tag_property in cons.TAG_PROPERTIES:
            if self.curr_attributes[tag_property] != "":
                dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
        self.nodes_list[-1].appendChild(dom_iter)
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
                self.node_add(wiki_string.decode(cons.STR_UTF8), node_name, curr_folder)
                # check if the node has children
                children_folder = os.path.join(curr_folder, node_name)
                if os.path.isdir(children_folder):
                    self.parse_folder(children_folder)
                self.nodes_list.pop()

    def node_add(self, wiki_string, node_name, curr_folder):
        """Add a node"""
        self.nodes_list.append(self.dom.createElement("node"))
        self.nodes_list[-1].setAttribute("name", node_name)
        self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
        self.nodes_list[-2].appendChild(self.nodes_list[-1])
        #
        self.pixbuf_vector = []
        self.chars_counter = 0
        self.node_wiki_parse(wiki_string, node_name, curr_folder)
        for pixbuf_element in self.pixbuf_vector:
            self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)

    def node_wiki_parse(self, wiki_string, node_name, curr_folder):
        """Parse the node wiki content"""
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.in_block = False
        self.in_link = False
        self.in_plain_link = False
        self.in_table = False
        self.in_codebox = False
        curr_pos = 0
        wiki_string = wiki_string.replace(cons.CHAR_CR, "")
        wiki_string = wiki_string.replace(cons.CHAR_NEWLINE+cons.CHAR_STAR+cons.CHAR_SPACE, cons.CHAR_NEWLINE+self.dad.chars_listbul[0]+cons.CHAR_SPACE)
        wiki_string = wiki_string.replace(cons.CHAR_TAB+cons.CHAR_STAR+cons.CHAR_SPACE, cons.CHAR_TAB+self.dad.chars_listbul[0]+cons.CHAR_SPACE)
        max_pos = len(wiki_string)
        newline_count = 0
        self.wiki_slot = ""
        def wiki_slot_flush():
            if self.wiki_slot:
                #print self.wiki_slot
                self.rich_text_serialize(self.wiki_slot)
                self.wiki_slot = ""
        probably_url = False
        in_hN = [False, False, False, False, False]
        while curr_pos < max_pos:
            curr_char = wiki_string[curr_pos:curr_pos+1]
            next_char = wiki_string[curr_pos+1:curr_pos+2] if curr_pos+1 < max_pos else cons.CHAR_SPACE
            third_char = wiki_string[curr_pos+2:curr_pos+3] if curr_pos+2 < max_pos else cons.CHAR_SPACE
            fourth_char = wiki_string[curr_pos+3:curr_pos+4] if curr_pos+3 < max_pos else cons.CHAR_SPACE
            if newline_count < 4:
                if curr_char == cons.CHAR_NEWLINE: newline_count += 1
            else:
                if self.in_block:
                    if curr_char == cons.CHAR_SQUOTE and next_char == cons.CHAR_SQUOTE and third_char == cons.CHAR_SQUOTE:
                        wiki_slot_flush()
                        self.curr_attributes[cons.TAG_FAMILY] = ""
                        curr_pos += 2
                        self.in_block = False
                    else: self.wiki_slot += curr_char
                elif self.in_plain_link:
                    if curr_char in [cons.CHAR_SPACE, cons.CHAR_NEWLINE]:
                        self.curr_attributes[cons.TAG_LINK] = "webs %s" % self.wiki_slot
                        wiki_slot_flush()
                        self.curr_attributes[cons.TAG_LINK] = ""
                        self.in_plain_link = False
                    self.wiki_slot += curr_char
                elif self.in_codebox:
                    if curr_char == cons.CHAR_BR_CLOSE and next_char == cons.CHAR_BR_CLOSE and third_char == cons.CHAR_BR_CLOSE:
                        #print self.wiki_slot
                        syntax_highlighting = cons.PLAIN_TEXT_ID
                        show_line_numbers = False
                        if self.wiki_slot.startswith("code"):
                            newline_idx = self.wiki_slot.find("\n")
                            if newline_idx >= 0:
                                decl_line = self.wiki_slot[:newline_idx]
                                match_syn_highl = re.search(r'lang="([^"]+)"', decl_line)
                                if match_syn_highl:
                                    prog_lang = match_syn_highl.group(1)
                                    if prog_lang in self.dad.available_languages:
                                        syntax_highlighting = prog_lang
                                match_linenum = re.search(r'linenumbers="([^"]+)"', decl_line)
                                if match_linenum:
                                    show_line_numbers = bool(match_linenum.group(1))
                                self.wiki_slot = self.wiki_slot[newline_idx:]
                        codebox_dict = {
                        'frame_width': 300,
                        'frame_height': 150,
                        'width_in_pixels': True,
                        'syntax_highlighting': syntax_highlighting,
                        'highlight_brackets': False,
                        'show_line_numbers': show_line_numbers,
                        'fill_text': self.wiki_slot
                        }
                        self.dad.xml_handler.codebox_element_to_xml([self.chars_counter, codebox_dict, cons.TAG_PROP_LEFT],
                            self.nodes_list[-1], self.dom)
                        self.chars_counter += 1
                        curr_pos += 2
                        self.in_codebox = False
                        self.wiki_slot = ""
                    else:
                        self.wiki_slot += curr_char
                elif self.in_link:
                    if curr_char == cons.CHAR_BR_CLOSE and next_char == cons.CHAR_BR_CLOSE:
                        valid_image = False
                        if cons.CHAR_QUESTION in self.wiki_slot:
                            splitted_wiki_slot = self.wiki_slot.split(cons.CHAR_QUESTION)
                            self.wiki_slot = splitted_wiki_slot[0]
                        if self.wiki_slot.startswith("./"): self.wiki_slot = os.path.join(curr_folder, node_name, self.wiki_slot[2:])
                        if os.path.isfile(self.wiki_slot):
                            try:
                                pixbuf = gtk.gdk.pixbuf_new_from_file(self.wiki_slot)
                                self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                                self.chars_counter += 1
                                valid_image = True
                            except: pass
                        if not valid_image: print "! error: '%s' is not a valid image" % self.wiki_slot
                        self.wiki_slot = ""
                        curr_pos += 1
                        self.in_link = False
                    elif curr_char == cons.CHAR_SQ_BR_CLOSE and next_char == cons.CHAR_SQ_BR_CLOSE:
                        if cons.CHAR_PIPE in self.wiki_slot:
                            target_n_label = self.wiki_slot.split(cons.CHAR_PIPE)
                        else:
                            target_n_label = [self.wiki_slot, self.wiki_slot]
                        exp_filepath = target_n_label[0]
                        if exp_filepath.startswith("./"): exp_filepath = os.path.join(curr_folder, node_name, exp_filepath[2:])
                        if exp_filepath.startswith("http") or exp_filepath.startswith("ftp") or exp_filepath.startswith("www.")\
                        and not cons.CHAR_SPACE in exp_filepath:
                            self.curr_attributes[cons.TAG_LINK] = "webs %s" % exp_filepath
                            self.rich_text_serialize(target_n_label[1])
                            self.curr_attributes[cons.TAG_LINK] = ""
                        elif cons.CHAR_SLASH in exp_filepath:
                            self.curr_attributes[cons.TAG_LINK] = "file %s" % base64.b64encode(exp_filepath)
                            self.rich_text_serialize(target_n_label[1])
                            self.curr_attributes[cons.TAG_LINK] = ""
                        else:
                            self.links_to_node_list.append({'name_dest': target_n_label[0],
                                                            'node_source': node_name,
                                                            'char_start': self.chars_counter,
                                                            'char_end': self.chars_counter+len(target_n_label[1])})
                            self.rich_text_serialize(target_n_label[1])
                        self.wiki_slot = ""
                        curr_pos += 1
                        self.in_link = False
                    else: self.wiki_slot += curr_char
                elif curr_char == cons.CHAR_STAR and next_char == cons.CHAR_STAR:
                    wiki_slot_flush()
                    if self.curr_attributes[cons.TAG_WEIGHT]: self.curr_attributes[cons.TAG_WEIGHT] = ""
                    else: self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                    curr_pos += 1
                elif curr_char == cons.CHAR_SLASH and next_char == cons.CHAR_SLASH:
                    if probably_url:
                        self.wiki_slot += curr_char
                        curr_pos += 1
                        continue
                    wiki_slot_flush()
                    if self.curr_attributes[cons.TAG_STYLE]: self.curr_attributes[cons.TAG_STYLE] = ""
                    else: self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                    curr_pos += 1
                elif curr_char == cons.CHAR_USCORE and next_char == cons.CHAR_USCORE:
                    wiki_slot_flush()
                    if self.curr_attributes[cons.TAG_UNDERLINE]: self.curr_attributes[cons.TAG_UNDERLINE] = ""
                    else: self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                    curr_pos += 1
                elif curr_char == cons.CHAR_TILDE and next_char == cons.CHAR_TILDE:
                    wiki_slot_flush()
                    if self.curr_attributes[cons.TAG_STRIKETHROUGH]: self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
                    else: self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
                    curr_pos += 1
                elif curr_char == cons.CHAR_SQUOTE and next_char == cons.CHAR_SQUOTE:
                    wiki_slot_flush()
                    if self.curr_attributes[cons.TAG_FAMILY]: self.curr_attributes[cons.TAG_FAMILY] = ""
                    else: self.curr_attributes[cons.TAG_FAMILY] = cons.TAG_PROP_MONOSPACE
                    if third_char == cons.CHAR_SQUOTE:
                        curr_pos += 2
                        self.in_block = True if self.curr_attributes[cons.TAG_FAMILY] else False
                    else: curr_pos += 1
                elif curr_char == 'h' and next_char == 't' and third_char == 't' and fourth_char == 'p':
                    wiki_slot_flush()
                    self.wiki_slot += curr_char
                    self.in_plain_link = True
                # ==
                elif not in_hN[4] and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_SPACE:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                    curr_pos += 2
                    in_hN[4] = True
                    #print "H5sta"
                elif in_hN[4] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = ""
                    curr_pos += 2
                    in_hN[4] = False
                    #print "H5end"
                ## ===
                elif not in_hN[3] and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_SPACE:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                    curr_pos += 3
                    in_hN[3] = True
                    #print "H4sta"
                elif in_hN[3] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = ""
                    curr_pos += 3
                    in_hN[3] = False
                    #print "H4end"
                ## ====
                elif not in_hN[2] and curr_pos+4 < max_pos and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+5] == cons.CHAR_SPACE:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H3
                    curr_pos += 4
                    in_hN[2] = True
                    #print "H3sta"
                elif curr_pos+4 < max_pos and in_hN[2] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+5] == cons.CHAR_EQUAL:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = ""
                    curr_pos += 4
                    in_hN[2] = False
                    #print "H3end"
                ## =====
                elif not in_hN[1] and curr_pos+5 < max_pos and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+6] == (cons.CHAR_EQUAL+cons.CHAR_SPACE):
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H2
                    curr_pos += 5
                    in_hN[1] = True
                    #print "H2sta"
                elif curr_pos+5 < max_pos and in_hN[1] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+6] == 2*cons.CHAR_EQUAL:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = ""
                    curr_pos += 5
                    in_hN[1] = False
                    #print "H2end"
                # ======
                elif not in_hN[0] and curr_pos+6 < max_pos and curr_char == cons.CHAR_EQUAL and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+7] == (2*cons.CHAR_EQUAL+cons.CHAR_SPACE):
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_H1
                    curr_pos += 6
                    in_hN[0] = True
                    #print "H1sta"
                elif curr_pos+6 < max_pos and in_hN[0] and curr_char == cons.CHAR_SPACE and next_char == cons.CHAR_EQUAL and third_char == cons.CHAR_EQUAL and fourth_char == cons.CHAR_EQUAL and wiki_string[curr_pos+4:curr_pos+7] == 3*cons.CHAR_EQUAL:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = ""
                    curr_pos += 6
                    in_hN[0] = False
                    #print "H1end"
                #
                elif curr_char == cons.CHAR_CARET and next_char == cons.CHAR_BR_OPEN:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_SUP
                    curr_pos += 1
                elif curr_char == cons.CHAR_USCORE and next_char == cons.CHAR_BR_OPEN:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_SUB
                    curr_pos += 1
                elif curr_char == cons.CHAR_BR_CLOSE and self.curr_attributes[cons.TAG_SCALE] in [cons.TAG_PROP_SUP, cons.TAG_PROP_SUB]:
                    wiki_slot_flush()
                    self.curr_attributes[cons.TAG_SCALE] = ""
                elif curr_char == cons.CHAR_BR_OPEN and next_char == cons.CHAR_BR_OPEN and third_char == cons.CHAR_BR_OPEN:
                    wiki_slot_flush()
                    curr_pos += 2
                    self.in_codebox = True
                elif curr_char == cons.CHAR_BR_OPEN and next_char == cons.CHAR_BR_OPEN \
                  or curr_char == cons.CHAR_SQ_BR_OPEN and next_char == cons.CHAR_SQ_BR_OPEN:
                    wiki_slot_flush()
                    curr_pos += 1
                    self.in_link = True
                elif curr_char == cons.CHAR_SQ_BR_OPEN\
                and next_char in [cons.CHAR_SPACE, cons.CHAR_STAR, 'x']\
                and third_char == cons.CHAR_SQ_BR_CLOSE:
                    if next_char == cons.CHAR_SPACE: self.wiki_slot += self.dad.chars_todo[0]
                    elif next_char == cons.CHAR_STAR: self.wiki_slot += self.dad.chars_todo[1]
                    else: self.wiki_slot += self.dad.chars_todo[2]
                    self.wiki_slot += cons.CHAR_SPACE
                    curr_pos += 2
                elif self.in_table:
                    if curr_char == cons.CHAR_PIPE:
                        self.curr_table[-1].append(self.curr_cell.strip())
                        self.curr_cell = ""
                        print self.curr_table[-1][-1]
                        if next_char == cons.CHAR_NEWLINE:
                            if third_char == cons.CHAR_PIPE:
                                self.curr_table.append([])
                                curr_pos += 2
                            else:
                                self.in_table = False
                                self.curr_table.append(self.curr_table.pop(0))
                                if self.curr_table[0][0].startswith(":-")\
                                or self.curr_table[0][0].startswith("--"):
                                    del self.curr_table[0]
                                table_dict = {'col_min': cons.TABLE_DEFAULT_COL_MIN,
                                              'col_max': cons.TABLE_DEFAULT_COL_MAX,
                                              'matrix': self.curr_table}
                                self.dad.xml_handler.table_element_to_xml([self.chars_counter, table_dict, cons.TAG_PROP_LEFT], self.nodes_list[-1], self.dom)
                                self.chars_counter += 1
                    else:
                        self.curr_cell += curr_char
                else:
                    self.wiki_slot += curr_char
                    #print self.wiki_slot
                    if curr_char == ":" and next_char == cons.CHAR_SLASH:
                        probably_url = True
                    elif curr_char in [cons.CHAR_SPACE, cons.CHAR_NEWLINE]:
                        probably_url = False
                        if curr_char == cons.CHAR_NEWLINE and next_char == cons.CHAR_PIPE:
                            self.in_table = True
                            self.curr_cell = ""
                            self.curr_table = [[]]
                            curr_pos += 1
                            wiki_slot_flush()
            curr_pos += 1
        wiki_slot_flush()

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
                if cons.CHAR_COLON in link_to_node['name_dest']:
                    node_dest = dad.get_tree_iter_from_node_name(link_to_node['name_dest'].split(cons.CHAR_COLON)[-1])
                elif link_to_node['name_dest'].startswith("+"):
                    node_dest = dad.get_tree_iter_from_node_name(link_to_node['name_dest'][1:])
            if not node_dest:
                print "node_dest not found", link_to_node['name_dest']
                continue
            if not node_source:
                print "node_source not found", link_to_node['node_source']
                continue
            source_buffer = dad.get_textbuffer_from_tree_iter(node_source)
            if source_buffer.get_char_count() < link_to_node['char_end']:
                print "source_buffer less than %d chars" % link_to_node['char_end']
                continue
            property_value = cons.LINK_TYPE_NODE + cons.CHAR_SPACE + str(dad.treestore[node_dest][3])
            source_buffer.apply_tag_by_name(dad.apply_tag_exist_or_create(cons.TAG_LINK, property_value),
                                            source_buffer.get_iter_at_offset(link_to_node['char_start']),
                                            source_buffer.get_iter_at_offset(link_to_node['char_end']))


class TomboyHandler():
    """The Handler of the Tomboy Folder Parsing"""

    def __init__(self, dad, folderpath):
        """Machine boot"""
        self.folderpath = folderpath
        self.dad = dad
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
        self.dest_dom_new.setAttribute("prog_lang", cons.RICH_TEXT_ID)
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
                    text_data = self.dad.chars_listbul[0]+cons.CHAR_SPACE + text_data
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
            elif dom_iter.nodeName == cons.TAG_PROP_MONOSPACE:
                self.curr_attributes[cons.TAG_FAMILY] = dom_iter.nodeName
                self.node_add_iter(dom_iter.firstChild)
                self.curr_attributes[cons.TAG_FAMILY] = ""
            elif dom_iter.nodeName == "size:small":
                self.curr_attributes[cons.TAG_SCALE] = cons.TAG_PROP_SMALL
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
            self.dest_notebooks_dom_nodes[notebook_title].setAttribute("prog_lang", cons.RICH_TEXT_ID)
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
        self.dest_orphans_dom_node.setAttribute("prog_lang", cons.RICH_TEXT_ID)
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
            property_value = cons.LINK_TYPE_NODE + cons.CHAR_SPACE + str(dad.treestore[node_dest][3])
            source_buffer.apply_tag_by_name(dad.apply_tag_exist_or_create(cons.TAG_LINK, property_value),
                                            source_buffer.get_iter_at_offset(link_to_node['char_start']),
                                            source_buffer.get_iter_at_offset(link_to_node['char_end']))


class BasketHandler(HTMLParser.HTMLParser):
    """The Handler of the Basket Folder Parsing"""

    def __init__(self, dad, folderpath):
        """Machine boot"""
        HTMLParser.HTMLParser.__init__(self)
        self.folderpath = folderpath
        self.dad = dad
        self.xml_handler = machines.XMLHandler(dad)

    def check_basket_structure(self):
        """Check the Selected Folder to be a Basket Folder"""
        self.baskets_xml_filepath = os.path.join(self.folderpath, "baskets.xml")
        if os.path.isfile(self.baskets_xml_filepath): return True
        self.folderpath = os.path.join(self.folderpath, "baskets")
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
                self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
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
        elif note_dom_iter.attributes['type'].value == "file":
            content_dom_iter = note_dom_iter.firstChild
            while content_dom_iter:
                if content_dom_iter.nodeName == "content":
                    content_path = os.path.join(self.subfolder_path, content_dom_iter.firstChild.data)
                    if os.path.isfile(content_path):
                        pixbuf = gtk.gdk.pixbuf_new_from_file(cons.FILE_CHAR)
                        with open(content_path, 'rb') as fd:
                            pixbuf.filename = os.path.basename(content_path)
                            pixbuf.embfile = fd.read()
                            pixbuf.time = time.time()
                        self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                        self.chars_counter += 1
                        self.rich_text_serialize(cons.CHAR_NEWLINE)
                        self.chars_counter += 1
                    break
                content_dom_iter = content_dom_iter.nextSibling
        elif note_dom_iter.attributes['type'].value == "cross_reference":
            content_dom_iter = note_dom_iter.firstChild
            while content_dom_iter:
                if content_dom_iter.nodeName == "content":
                    if content_dom_iter.hasAttribute('title'):
                        title = "cross reference: "  + content_dom_iter.attributes['title'].value
                        self.feed(title.decode(cons.STR_UTF8, cons.STR_IGNORE))
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
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.h_rule+cons.CHAR_NEWLINE)
                self.chars_counter += len(self.dad.h_rule)+2
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.chars_listbul[0]+cons.CHAR_SPACE)
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
        self.start_parsing()
        return self.dom.toxml()


class KnowitHandler(HTMLParser.HTMLParser):
    """The Handler of the Knowit File Parsing"""

    def __init__(self, dad):
        """Machine boot"""
        HTMLParser.HTMLParser.__init__(self)
        self.dad = dad
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
        self.links_list = []
        # 0: waiting for \NewEntry or \CurrentEntry
        # 1: gathering node content
        for text_line in file_descriptor:
            text_line = text_line.decode(cons.STR_UTF8, cons.STR_IGNORE)
            if self.curr_xml_state == 0:
                if text_line.startswith("\NewEntry ")\
                or text_line.startswith("\CurrentEntry "):
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
                    self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
                    self.nodes_list[-2].appendChild(self.nodes_list[-1])
                else: self.curr_node_name += text_line.replace(cons.CHAR_CR, "").replace(cons.CHAR_NEWLINE, "") + cons.CHAR_SPACE
            elif self.curr_xml_state == 1:
                if text_line.startswith("\Link "):
                    link_uri = text_line[6:-1]
                    self.links_list.append([link_uri, ""])
                elif text_line.startswith("\Descr "):
                    link_desc = text_line[7:-1]
                    self.links_list[-1][1] = link_desc
                elif text_line.endswith("</body></html>"+cons.CHAR_NEWLINE):
                    # node content end
                    self.curr_xml_state = 0
                    self.curr_html_state = 0
                    self.feed(self.curr_node_content.decode(cons.STR_UTF8, cons.STR_IGNORE))
                    if self.links_list:
                        self.rich_text_serialize(cons.CHAR_NEWLINE)
                        for link_element in self.links_list:
                            if link_element[0][:4] in ["http", "file"]:
                                if link_element[0][:4] == "http":
                                    self.curr_attributes[cons.TAG_LINK] = "webs %s" % link_element[0]
                                elif link_element[0][:4] == "file":
                                    self.curr_attributes[cons.TAG_LINK] = "file %s" % base64.b64encode(link_element[0][7:])
                                self.rich_text_serialize(link_element[1])
                                self.curr_attributes[cons.TAG_LINK] = ""
                                self.rich_text_serialize(cons.CHAR_NEWLINE)
                            elif link_element[0].startswith("knowit://"):
                                name_dest = link_element[0][9:]
                                self.links_to_node_list.append({'name_dest':name_dest,
                                    'name_source':self.curr_node_name,
                                    'node_desc':link_element[1]})
                        self.links_list = []
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
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.chars_listbul[0]+cons.CHAR_SPACE)

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

    def set_links_to_nodes(self, dad):
        """After the node import, set the links to nodes on the new tree"""
        for link_to_node in self.links_to_node_list:
            node_dest = dad.get_tree_iter_from_node_name(link_to_node['name_dest'])
            node_source = dad.get_tree_iter_from_node_name(link_to_node['name_source'])
            if not node_dest:
                #print "node_dest not found"
                continue
            if not node_source:
                #print "node_source not found"
                continue
            source_buffer = dad.get_textbuffer_from_tree_iter(node_source)
            property_value = cons.LINK_TYPE_NODE + cons.CHAR_SPACE + str(dad.treestore[node_dest][3])
            node_desc = link_to_node['node_desc']
            char_start = source_buffer.get_end_iter().get_offset()
            char_end = char_start + len(node_desc)
            source_buffer.insert(source_buffer.get_end_iter(), node_desc)
            source_buffer.apply_tag_by_name(dad.apply_tag_exist_or_create(cons.TAG_LINK, property_value),
                                            source_buffer.get_iter_at_offset(char_start),
                                            source_buffer.get_iter_at_offset(char_end))

    def get_cherrytree_xml(self, file_descriptor):
        """Returns a CherryTree string Containing the Knowit Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.links_to_node_list = []
        self.parse_string_lines(file_descriptor)
        return self.dom.toxml()


class KeynoteHandler:
    """The Handler of the Keynote File Parsing"""

    def __init__(self, dad):
        """Machine boot"""
        self.xml_handler = machines.XMLHandler(self)
        self.dad = dad

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        for tag_property in cons.TAG_PROPERTIES:
            if self.curr_attributes[tag_property] != "":
                dom_iter.setAttribute(tag_property, self.curr_attributes[tag_property])
        self.nodes_list[-1].appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)
        self.chars_counter += len(text_data)

    def parse_string_lines(self, file_descriptor):
        """Parse the string line by line"""
        self.curr_state = 0
        self.curr_node_name = ""
        self.curr_node_id = None
        self.curr_node_content = ""
        self.curr_node_level = 0
        self.former_node_level = -1
        self.img_tmp_path = os.path.join(cons.TMP_FOLDER, "img_tmp")
        # 0: waiting for LV=
        # 1: waiting for ND=
        # 2: waiting for DI=
        # 3: waiting for %:
        # 4: parsing node content
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
                if text_line.startswith("DI="):
                    self.curr_state = 3
                    self.curr_node_id = text_line[3:]
            elif self.curr_state == 3:
                if text_line.startswith("%:"):
                    self.curr_state = 4
                    self.curr_node_content = ""
                    self.nodes_list.append(self.dom.createElement("node"))
                    self.nodes_list[-1].setAttribute("name", self.curr_node_name)
                    self.nodes_list[-1].setAttribute("unique_id", self.curr_node_id)
                    self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
                    self.nodes_list[-2].appendChild(self.nodes_list[-1])
                    self.node_br_ok = False
                    self.in_picture = False
                    self.in_object = False
                    self.pixbuf_vector = []
                    self.chars_counter = 0
                    self.in_br_num = 0
                    self.in_br_read_data = False
            elif self.curr_state == 4:
                if text_line.startswith("%-") or text_line.startswith("%%"):
                    self.curr_state = 0
                    self.rich_text_serialize(self.curr_node_content)
                    for pixbuf_element in self.pixbuf_vector:
                        self.xml_handler.pixbuf_element_to_xml(pixbuf_element, self.nodes_list[-1], self.dom)
                    self.pixbuf_vector = []
                    self.chars_counter = 0
                else: self.write_line_text(text_line.replace(cons.CHAR_CR, "").replace(cons.CHAR_NEWLINE, "") + cons.CHAR_NEWLINE)

    def check_pending_text_to_tag(self):
        """Check if there's text to process before opening tag"""
        if self.curr_node_content:
            self.rich_text_serialize(self.curr_node_content)
            self.curr_node_content = ""

    def write_line_text(self, text_line):
        """Write Stripped Line Content"""
        #print "'%s'" % text_line
        curr_state = 0
        dummy_loop = 0
        if self.in_picture or self.in_object:
            if text_line[0] == cons.CHAR_BR_CLOSE:
                self.img_fd.close()
                if self.in_picture: print "in_picture OFF"
                else: print "in_object OFF"
                with open(self.img_tmp_path, 'rb') as fd:
                    pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(cons.FILE_CHAR, self.dad.embfile_size, self.dad.embfile_size)
                    pixbuf.filename = "image.wmf" if self.in_picture else "file"
                    pixbuf.embfile = fd.read()
                    pixbuf.time = time.time()
                self.pixbuf_vector.append([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT])
                self.chars_counter += 1
                self.in_picture = False
                self.in_object = False
                self.in_br_num -= 1
                dummy_loop += 1
            else:
                if self.in_picture or self.in_object:
                    self.img_fd.write(binascii.a2b_hex(text_line))
                return
        elif "HYPERLINK" in text_line:
            hyp_idx_1 = text_line.find("HYPERLINK")
            hyp_idx_2 = text_line.find("}}}")
            text_line = text_line[hyp_idx_1+11:hyp_idx_2]
            #print text_line
            hyp_idx_1 = text_line.find(cons.CHAR_DQUOTE)
            text_target = text_line[:hyp_idx_1]
            text_line = text_line[hyp_idx_1:]
            hyp_idx_2 = text_line.find(cons.CHAR_SPACE)
            text_label = text_line[hyp_idx_2+1:]
            text_target = text_target.replace(4*cons.CHAR_BSLASH, cons.CHAR_BSLASH)
            text_label = text_label.replace(2*cons.CHAR_BSLASH, cons.CHAR_BSLASH)
            print text_target
            print text_label
            self.check_pending_text_to_tag()
            if text_target.startswith("http"):
                self.curr_attributes[cons.TAG_LINK] = "webs %s" % text_target
            elif text_target.startswith("file"):
                if text_target[8:9] != cons.CHAR_STAR:
                    self.curr_attributes[cons.TAG_LINK] = "file %s" % base64.b64encode(text_target[8:])
                else:
                    self.links_to_node_list.append({'name_dest':text_label,
                        'name_source':self.curr_node_name,
                        'node_desc':text_label,
                        'char_start':self.chars_counter,
                        'char_end':self.chars_counter+len(text_label)})
            self.rich_text_serialize(text_label)
            self.curr_attributes[cons.TAG_LINK] = ""
            return
        for i, curr_char in enumerate(text_line):
            if dummy_loop > 0:
                dummy_loop -= 1
                continue
            if curr_char == cons.CHAR_BSLASH:
                if (self.in_br_num == 0 or self.in_br_read_data) and text_line[i+1:].startswith(cons.CHAR_SQUOTE):
                    self.curr_node_content += unichr(int(text_line[i+2:i+4], 16))
                    dummy_loop = 3
                    curr_state = 0
                elif text_line[i+1:].startswith(cons.CHAR_BSLASH):
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
                    fN = text_line[i+8:i+11]
                    if fN == "f0 ":
                        curr_state = 1
                        self.in_br_read_data = True
                    elif fN == "f1 ":
                        dummy_loop = 10
                        curr_state = 0
                        self.in_br_read_data = True
                    else:
                        dummy_loop = 9
                        self.curr_node_content += self.dad.chars_listbul[0] + cons.CHAR_SPACE
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
                    print "in_picture ON"
                    curr_state = 1
                    self.img_fd = open(self.img_tmp_path, "wb")
                elif text_line[i+1:].startswith("object"):
                    self.in_object = True
                    print "in_object ON"
                    curr_state = 1
                    self.img_fd = open(self.img_tmp_path, "wb")
                elif text_line[i+1:].startswith("result"):
                    print "result"
                    dummy_loop = 6
                    curr_state = 0
                else:
                    curr_state = 1
            elif curr_char == cons.CHAR_BR_OPEN:
                if self.node_br_ok:
                    self.in_br_num += 1
                    print "self.in_br_num", self.in_br_num
                else: self.node_br_ok = True
            elif curr_char == cons.CHAR_BR_CLOSE:
                self.in_br_num -= 1
                print "self.in_br_num", self.in_br_num
                curr_state = 0
                self.in_br_read_data = False
            elif self.in_br_read_data and curr_char == cons.CHAR_PARENTH_CLOSE:
                self.curr_node_content += "." + cons.CHAR_SPACE
            else:
                if self.in_br_num == 0 or self.in_br_read_data:
                    if curr_state == 0:
                        self.curr_node_content += curr_char
                    elif curr_state == 1:
                        if curr_char == cons.CHAR_SPACE:
                            curr_state = 0

    def set_links_to_nodes(self, dad):
        """After the node import, set the links to nodes on the new tree"""
        for link_to_node in self.links_to_node_list:
            node_dest = dad.get_tree_iter_from_node_name(link_to_node['name_dest'])
            node_source = dad.get_tree_iter_from_node_name(link_to_node['name_source'])
            if not node_dest:
                #print "node_dest not found"
                continue
            if not node_source:
                #print "node_source not found"
                continue
            source_buffer = dad.get_textbuffer_from_tree_iter(node_source)
            if source_buffer.get_char_count() < link_to_node['char_end']:
                continue
            property_value = cons.LINK_TYPE_NODE + cons.CHAR_SPACE + str(dad.treestore[node_dest][3])
            source_buffer.apply_tag_by_name(dad.apply_tag_exist_or_create(cons.TAG_LINK, property_value),
                                            source_buffer.get_iter_at_offset(link_to_node['char_start']),
                                            source_buffer.get_iter_at_offset(link_to_node['char_end']))

    def get_cherrytree_xml(self, file_descriptor):
        """Returns a CherryTree string Containing the Treepad Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.links_to_node_list = []
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

    def parse_string_lines(self, treepad_string):
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
        treepad_vec = treepad_string.split(cons.CHAR_CR+cons.CHAR_NEWLINE)
        for text_line in treepad_vec:
            if self.curr_state == 0:
                if len(text_line) > 5 and text_line[:6] == "<node>": self.curr_state = 1
            elif self.curr_state == 1:
                #print "node name", text_line
                self.curr_node_name = text_line
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
                    self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
                    self.nodes_list[-2].appendChild(self.nodes_list[-1])
                else: self.curr_node_name += text_line + cons.CHAR_SPACE
            elif self.curr_state == 3:
                if len(text_line) > 9 and text_line[:10] == "<end node>":
                    self.curr_state = 0
                    self.rich_text_serialize(self.curr_node_content)
                else: self.curr_node_content += text_line + cons.CHAR_NEWLINE

    def get_cherrytree_xml(self, treepad_string):
        """Returns a CherryTree string Containing the Treepad Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        self.parse_string_lines(treepad_string)
        return self.dom.toxml()


class PlainTextHandler:
    """The Handler of the Plain Text Parsing"""

    def __init__(self, dad):
        """Machine boot"""
        self.dad = dad

    def rich_text_serialize(self, text_data):
        """Appends a new part to the XML rich text"""
        dom_iter = self.dom.createElement("rich_text")
        self.nodes_list[-1].appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)

    def add_folder(self, folderpath):
        """Add nodes from plain text files in a Folder"""
        for element in sorted(os.listdir(folderpath)):
            full_element = os.path.join(folderpath, element)
            if os.path.isfile(full_element):
                gio_file = gio.File(full_element)
                gio_file_info = gio_file.query_info("*")
                if not cons.IS_WIN_OS:
                    mime_types = str(gio_file_info.get_icon())
                    if "text-" in mime_types:
                        self.add_file(full_element)
                else:
                    mime_type = gio_file_info.get_content_type()
                    if mime_type in ["."+self.dad.ext_plain_import.lower(), "."+self.dad.ext_plain_import.upper()]:
                        self.add_file(full_element)
            elif os.path.isdir(full_element):
                self.add_node_with_content(full_element, "")
                self.add_folder(full_element)
                self.nodes_list.pop()

    def add_file(self, filepath):
        """Add node from one plain text File"""
        file_content = ""
        try:
            file_descriptor = open(filepath, 'r')
            file_content = file_descriptor.read()
            file_descriptor.close()
        except:
            print "skip import of", filepath
            return
        self.add_node_with_content(filepath, support.auto_decode_str(file_content))
        self.nodes_list.pop()

    def add_node_with_content(self, filepath, file_content):
        """Append Node and Fill Content"""
        self.nodes_list.append(self.dom.createElement("node"))
        node_name = os.path.basename(filepath)
        if node_name.lower().endswith("."+self.dad.ext_plain_import.lower()):
            len_ext = 1+len(self.dad.ext_plain_import)
            node_name = node_name[:-len_ext]
        self.nodes_list[-1].setAttribute("name", node_name)
        self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
        self.nodes_list[-2].appendChild(self.nodes_list[-1])
        self.rich_text_serialize(file_content)

    def get_cherrytree_xml(self, filepath="", folderpath=""):
        """Returns a CherryTree string Containing the Plain Text Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        if filepath: self.add_file(filepath)
        else: self.add_folder(folderpath)
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
                    self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
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

    def __init__(self, dad):
        """Machine boot"""
        HTMLParser.HTMLParser.__init__(self)
        self.dad = dad
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
                # the current node becomes parent
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
                        self.latest_span.append(cons.TAG_FOREGROUND)
                    elif match.group(1) == "background-color":
                        self.curr_attributes[cons.TAG_BACKGROUND] = match.group(2).strip()
                        self.latest_span.append(cons.TAG_BACKGROUND)
            elif tag == "a" and len(attrs) > 0:
                link_url = attrs[0][1]
                if len(link_url) > 7:
                    self.curr_attributes[cons.TAG_LINK] = get_internal_link_from_http_url(link_url)
            elif tag == "br":
                # this is a data block composed only by an endline
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                self.chars_counter += 1
            elif tag == "li":
                self.rich_text_serialize(cons.CHAR_NEWLINE+self.dad.chars_listbul[0]+cons.CHAR_SPACE)
                self.chars_counter += 3
            elif tag in ["img", "v:imagedata"] and len(attrs) > 0:
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
                self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
                self.nodes_list[-2].appendChild(self.nodes_list[-1])
                self.curr_title = ""
                # waiting for data
                if self.chars_counter > 0:
                    # this means the new node is child of the previous, so we did not pop
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
                # got /dl, we go state 2->0 and wait for a parent's sibling
                self.curr_state = 0
            elif tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = ""
            elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = ""
            elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = ""
            elif tag == "s": self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
            elif tag == "span":
                if self.latest_span:
                    if self.latest_span[-1] == cons.TAG_FOREGROUND: self.curr_attributes[cons.TAG_FOREGROUND] = ""
                    elif self.latest_span[-1] == cons.TAG_BACKGROUND: self.curr_attributes[cons.TAG_BACKGROUND] = ""
                    del self.latest_span[-1]
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
        self.latest_span = []
        # curr_state 0: standby, taking no data
        # curr_state 1: waiting for node title, take one data
        # curr_state 2: waiting for node content, take many data
        self.pixbuf_vector = []
        self.chars_counter = 0
        self.feed(input_string.decode(cons.STR_UTF8, cons.STR_IGNORE))
        return self.dom.toxml()


class HTMLHandler(HTMLParser.HTMLParser):
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
        self.nodes_list[-1].appendChild(dom_iter)
        text_iter = self.dom.createTextNode(text_data)
        dom_iter.appendChild(text_iter)
        self.chars_counter += len(text_data)

    def get_rgb_gtk_attribute(self, html_attribute):
        """Get RGB GTK attribute from HTML attribute"""
        html_attribute_key = html_attribute.strip().lower()
        #print "html_attribute_key", html_attribute_key
        if html_attribute_key[0] == "#":
            return html_attribute_key
        if html_attribute_key in cons.HTML_COLOR_NAMES:
            return cons.HTML_COLOR_NAMES[html_attribute_key]
        if "rgb" in html_attribute_key:
            rgb_tern = []
            for i in range(3):
                if i == 0: parenth_start = html_attribute_key.find(cons.CHAR_PARENTH_OPEN)
                else: parenth_start = html_attribute_key.find(cons.CHAR_COMMA)
                if i == 2: parenth_end = html_attribute_key[parenth_start+1:].find(cons.CHAR_PARENTH_CLOSE)
                else: parenth_end = html_attribute_key[parenth_start+1:].find(cons.CHAR_COMMA)
                if parenth_start < 0 or parenth_end < 0:
                    break
                rgb_tern.append(int(html_attribute_key[parenth_start+1:parenth_start+1+parenth_end]))
                html_attribute_key = html_attribute_key[parenth_start+1+parenth_end:]
            if len(rgb_tern) != 3:
                print rgb_tern
                return None
            html_attribute_key = "#%.2x%.2x%.2x" % (rgb_tern[0], rgb_tern[1], rgb_tern[2])
            return html_attribute_key
        return None

    def handle_starttag(self, tag, attrs):
        """Encountered the beginning of a tag"""
        if tag in self.monitored_tags: self.in_a_tag += 1
        if self.curr_state == 0:
            if tag == "body":
                self.num_bodies -= 1
                if self.num_bodies == 0: self.curr_state = 1
        elif self.curr_state == 1:
            if tag == "table":
                self.curr_state = 2
                self.curr_table = []
                self.curr_rows_span = []
                self.curr_table_header = False
                self.curr_cell = ""
            elif tag == "b": self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
            elif tag == "i": self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
            elif tag == "u": self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
            elif tag == "s": self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
            elif tag == cons.TAG_STYLE: self.curr_state = 0
            elif tag == "span":
                self.latest_span.append(set())
                for attr in attrs:
                    if attr[0] == cons.TAG_STYLE:
                        attributes = attr[1].split(";")
                        for attribute in attributes:
                            #print "attribute", attribute
                            colon_pos = attribute.find(cons.CHAR_COLON)
                            if colon_pos < 0: continue
                            attr_name = attribute[:colon_pos].strip().lower()
                            attr_value = attribute[colon_pos+1:].strip().lower()
                            #print attr_name, attr_value
                            if attr_name == "color":
                                attribute = self.get_rgb_gtk_attribute(attr_value)
                                if attribute and not exports.rgb_get_is_blackish_or_whiteish(attribute):
                                    self.curr_attributes[cons.TAG_FOREGROUND] = attribute
                                    self.latest_span[-1].add(cons.TAG_FOREGROUND)
                            elif attr_name in [cons.TAG_BACKGROUND, "background-color"]:
                                attribute = self.get_rgb_gtk_attribute(attr_value)
                                if attribute and not exports.rgb_get_is_blackish_or_whiteish(attribute):
                                    self.curr_attributes[cons.TAG_BACKGROUND] = attribute
                                    self.latest_span[-1].add(cons.TAG_BACKGROUND)
                            elif attr_name == "text-decoration":
                                if attr_value in [cons.TAG_UNDERLINE, "underline;"]:
                                    self.curr_attributes[cons.TAG_UNDERLINE] = cons.TAG_PROP_SINGLE
                                    self.latest_span[-1].add(cons.TAG_UNDERLINE)
                                elif attr_value in ["line-through"]:
                                    self.curr_attributes[cons.TAG_STRIKETHROUGH] = cons.TAG_PROP_TRUE
                                    self.latest_span[-1].add(cons.TAG_STRIKETHROUGH)
                            elif attr_name == "font-weight":
                                if attr_value in ["bold", "bolder", "700"]:
                                    self.curr_attributes[cons.TAG_WEIGHT] = cons.TAG_PROP_HEAVY
                                    self.latest_span[-1].add(cons.TAG_WEIGHT)
                            elif attr_name == "font-style":
                                if attr_value in [cons.TAG_PROP_ITALIC]:
                                    self.curr_attributes[cons.TAG_STYLE] = cons.TAG_PROP_ITALIC
                                    self.latest_span[-1].add(cons.TAG_STYLE)
            elif tag == "font":
                for attr in attrs:
                    if attr[0] == "color":
                        attribute = self.get_rgb_gtk_attribute(attr[1].strip())
                        if attribute and not exports.rgb_get_is_blackish_or_whiteish(attribute):
                            self.curr_attributes[cons.TAG_FOREGROUND] = attribute
                            self.latest_font = cons.TAG_FOREGROUND
            elif tag in (cons.TAG_PROP_SUP, cons.TAG_PROP_SUB):
                self.curr_attributes[cons.TAG_SCALE] = tag
            elif tag in (cons.TAG_PROP_H1, cons.TAG_PROP_H2, cons.TAG_PROP_H3, cons.TAG_PROP_H4, cons.TAG_PROP_H5, cons.TAG_PROP_H6):
                self.rich_text_serialize(cons.CHAR_NEWLINE)
                if tag in (cons.TAG_PROP_H1, cons.TAG_PROP_H2): self.curr_attributes[cons.TAG_SCALE] = tag
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
                if self.curr_list_type[0] == "u": self.rich_text_serialize(self.dad.chars_listbul[0]+cons.CHAR_SPACE)
                else:
                    self.rich_text_serialize("%s. " % self.curr_list_type[1])
                    self.curr_list_type[1] += 1
            elif tag in ["img", "v:imagedata"] and len(attrs) > 0:
                dic_attrs = dict(a for a in attrs)
                img_path = dic_attrs.get('src', "")
                self.insert_image(img_path)
            elif tag == "pre": self.pre_tag = "p"
            elif tag == "code": self.curr_attributes[cons.TAG_FAMILY] = cons.TAG_PROP_MONOSPACE
            elif tag == "dt":
                self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "dd":
                self.rich_text_serialize(cons.CHAR_NEWLINE+cons.CHAR_TAB)
        elif self.curr_state == 2:
            if tag == "table": # nested tables
                self.curr_table = []
                self.curr_rows_span = []
                self.curr_table_header = False
                self.curr_cell = ""
            elif tag == "tr":
                if self.curr_cell.strip() and self.curr_table:
                    # case of not closed <td>
                    self.curr_table[-1].append(self.curr_cell)
                    self.curr_cell = ""
                self.curr_table.append([])
            elif tag in ["td", "th"]:
                if not self.curr_table:
                    # case of first missing <tr>, this is the header even if <td>
                    self.curr_table.append([])
                    self.curr_table_header = True
                self.curr_cell = ""
                self.curr_rowspan = 1
                for attr in attrs:
                    if attr[0] == "rowspan":
                        self.curr_rowspan = int(attr[1])
                if tag == "th":
                    self.curr_table_header = True
            elif tag in ["img", "v:imagedata"] and len(attrs) > 0:
                dic_attrs = dict(a for a in attrs)
                img_path = dic_attrs.get('src', "")
                self.insert_image(img_path, cons.CHAR_NEWLINE*2)
            elif tag == "br": self.curr_cell += cons.CHAR_NEWLINE
            elif tag == "ol": self.curr_list_type = ["o", 1]
            elif tag == "ul": self.curr_list_type = ["u", 0]
            elif tag == "li":
                if self.curr_list_type[0] == "u": self.curr_cell += self.dad.chars_listbul[0]+cons.CHAR_SPACE
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
            self.dad.xml_handler.pixbuf_element_to_xml([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT], self.nodes_list[-1], self.dom)
            self.chars_counter += 1
            self.dad.statusbar.pop(self.dad.statusbar_context_id)
            if trailing_chars: self.rich_text_serialize(trailing_chars)
        except:
            if os.path.isfile(os.path.join(self.local_dir, img_path)):
                pixbuf = gtk.gdk.pixbuf_new_from_file(os.path.join(self.local_dir, img_path))
                self.dad.xml_handler.pixbuf_element_to_xml([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT], self.nodes_list[-1], self.dom)
                self.chars_counter += 1
            else: print "failed download of", img_path
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
                if self.latest_span:
                    if cons.TAG_FOREGROUND in self.latest_span[-1]: self.curr_attributes[cons.TAG_FOREGROUND] = ""
                    elif cons.TAG_BACKGROUND in self.latest_span[-1]: self.curr_attributes[cons.TAG_BACKGROUND] = ""
                    elif cons.TAG_UNDERLINE in self.latest_span[-1]: self.curr_attributes[cons.TAG_UNDERLINE] = ""
                    elif cons.TAG_STRIKETHROUGH in self.latest_span[-1]: self.curr_attributes[cons.TAG_STRIKETHROUGH] = ""
                    elif cons.TAG_WEIGHT in self.latest_span[-1]: self.curr_attributes[cons.TAG_WEIGHT] = ""
                    elif cons.TAG_STYLE in self.latest_span[-1]: self.curr_attributes[cons.TAG_STYLE] = ""
                    del self.latest_span[-1]
            elif tag == "font":
                if self.latest_font == cons.TAG_FOREGROUND: self.curr_attributes[cons.TAG_FOREGROUND] = ""
            elif tag in (cons.TAG_PROP_SUP, cons.TAG_PROP_SUB):
                self.curr_attributes[cons.TAG_SCALE] = ""
            elif tag in (cons.TAG_PROP_H1, cons.TAG_PROP_H2, cons.TAG_PROP_H3, cons.TAG_PROP_H4, cons.TAG_PROP_H5, cons.TAG_PROP_H6):
                self.curr_attributes[cons.TAG_SCALE] = ""
                self.curr_attributes[cons.TAG_JUSTIFICATION] = ""
                self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "a": self.curr_attributes[cons.TAG_LINK] = ""
            elif tag == "li": self.rich_text_serialize(cons.CHAR_NEWLINE)
            elif tag == "pre": self.pre_tag = ""
            elif tag == "code": self.curr_attributes[cons.TAG_FAMILY] = ""
        elif self.curr_state == 2:
            if tag in ["td", "th"]:
                self.curr_table[-1].append(self.curr_cell)
                self.curr_cell = ""
                if len(self.curr_table) == 1:
                    self.curr_rows_span.append(self.curr_rowspan)
                else:
                    index = len(self.curr_table[-1])-1
                    #print "self.curr_rows_span", self.curr_rows_span
                    while index >= len(self.curr_rows_span):
                        # rowspan in very first row
                        self.curr_rows_span.append(1)
                        self.curr_table[-2].append("")
                    if self.curr_rows_span[index] == 1:
                        self.curr_rows_span[index] = self.curr_rowspan
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
                if not self.curr_table[-1]:
                    # case of latest <tr> without any <tr> afterwards
                    del self.curr_table[-1]
                if len(self.curr_table) == 1 and len(self.curr_table[0]) == 1:
                    # it's a codebox
                    text_inside_codebox = self.curr_table[0][0].strip()
                    if text_inside_codebox:
                        codebox_dict = {
                        'frame_width': 300,
                        'frame_height': 150,
                        'width_in_pixels': True,
                        'syntax_highlighting': cons.PLAIN_TEXT_ID,
                        'highlight_brackets': False,
                        'show_line_numbers': False,
                        'fill_text': text_inside_codebox
                        }
                        self.dad.xml_handler.codebox_element_to_xml([self.chars_counter, codebox_dict, cons.TAG_PROP_LEFT],
                            self.nodes_list[-1], self.dom)
                        self.chars_counter += 1
                    else: print "empty codebox skip"
                else:
                    # it's a table
                    if len(self.curr_table) > 0:
                        num_columns = len(self.curr_table[0])
                        for curr_row in self.curr_table:
                            if len(curr_row) != num_columns:
                                print "!!", self.curr_table
                                break
                        else:
                            if not self.curr_table_header: self.curr_table.append([_("click me")]*num_columns)
                            else: self.curr_table.append(self.curr_table.pop(0))
                            table_dict = {'col_min': cons.TABLE_DEFAULT_COL_MIN,
                                          'col_max': cons.TABLE_DEFAULT_COL_MAX,
                                          'matrix': self.curr_table}
                            self.dad.xml_handler.table_element_to_xml([self.chars_counter, table_dict, cons.TAG_PROP_LEFT],
                                self.nodes_list[-1], self.dom)
                            self.chars_counter += 1
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
        replacements = False
        out_data = []
        for char in clean_data:
            ord_char = ord(char)
            #print ord_char
            if ord_char == 0xa0:
                out_data.append(cons.CHAR_SPACE)
                replacements = True
            elif ord_char == 0xfeff:
                replacements = True
            else:
                out_data.append(char)
        if replacements:
            clean_data = "".join(out_data)
        if self.curr_state == 1: self.rich_text_serialize(clean_data.replace(cons.CHAR_TAB, cons.CHAR_SPACE))
        elif self.curr_state == 2: self.curr_cell += clean_data.replace(cons.CHAR_TAB, "")

    def handle_entityref(self, name):
        """Found Entity Reference like &name;"""
        if self.curr_state == 0: return
        #print name
        if name == "nbsp":
            unicode_char = cons.CHAR_SPACE
        elif name in htmlentitydefs.name2codepoint:
            unicode_char = unichr(htmlentitydefs.name2codepoint[name])
        else: return
        if self.curr_state == 1: self.rich_text_serialize(unicode_char)
        elif self.curr_state == 2: self.curr_cell += unicode_char

    def handle_charref(self, name):
        """decimal and hexadecimal numeric character references of the form &#NNN; and &#xNNN;"""
        if self.curr_state == 0: return
        if name[0] in ['x', 'X']:
            unicode_num = int(name[1:], 16)
        else: unicode_num = int(name)
        #print unicode_num
        if unicode_num == 160: # nbsp
            unicode_num = 32 # space
        unicode_char = unichr(unicode_num)
        if self.curr_state == 1: self.rich_text_serialize(unicode_char)
        elif self.curr_state == 2: self.curr_cell += unicode_char

    def get_clipboard_selection_xml(self, input_string):
        """Parses the Given HTML String feeding the XML dom"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement("root")]
        self.dom.appendChild(self.nodes_list[0])
        self.nodes_list.append(self.dom.createElement("slot"))
        self.nodes_list[0].appendChild(self.nodes_list[-1])
        self.boot_n_feed(input_string, "")
        return self.dom.toxml()

    def boot_n_feed(self, input_string, local_dir):
        """Init variables and start feed"""
        self.curr_state = 0
        self.local_dir = local_dir
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        self.latest_span = []
        self.latest_font = ""
        self.curr_cell = ""
        self.in_a_tag = 0
        self.chars_counter = 0
        self.curr_list_type = ["u", 0]
        self.pre_tag = ""
        # curr_state 0: standby, taking no data
        # curr_state 1: receiving rich text
        # curr_state 2: receiving table or codebox data
        if not HTMLCheck().is_html_ok(input_string):
            input_string = cons.HTML_HEADER % "" + input_string + cons.HTML_FOOTER
        #print "###############"
        #print input_string
        #with open("clipboard.log", 'w') as fd:
            #fd.write(input_string)
        #print "###############"
        self.num_bodies = len(re.findall("<body[^>]*>", input_string, re.IGNORECASE))
        self.feed(input_string)

    def add_folder(self, folderpath):
        """Add nodes from HTML files in a Folder"""
        folder_file_same_name = ""
        for element in sorted(os.listdir(folderpath)):
            if folder_file_same_name and folder_file_same_name == element:
                folder_file_same_name = ""
                continue
            full_element = os.path.join(folderpath, element)
            if os.path.isfile(full_element):
                gio_file = gio.File(full_element)
                gio_file_info = gio_file.query_info("*")
                if not cons.IS_WIN_OS:
                    mime_types = str(gio_file_info.get_icon()).lower()
                    if "html" in mime_types:
                        self.add_file(full_element)
                else:
                    mime_type = gio_file_info.get_content_type()
                    if mime_type.lower() in [".html", ".htm"]:
                        self.add_file(full_element)
            elif os.path.isdir(full_element):
                if os.path.isfile(full_element+".htm"):
                    folder_file_same_name = element+".htm"
                    self.add_file(full_element+".htm", do_pop=False)
                else: self.add_node_with_content(full_element, "")
                self.add_folder(full_element)
                self.nodes_list.pop()

    def add_file(self, filepath, do_pop=True):
        """Add node from one HTML File"""
        file_content = ""
        try:
            file_descriptor = open(filepath, 'r')
            file_content = file_descriptor.read()
            file_descriptor.close()
        except:
            print "skip import of", filepath
            return
        self.add_node_with_content(filepath, "")
        self.boot_n_feed(support.auto_decode_str(file_content), os.path.dirname(filepath))
        if do_pop: self.nodes_list.pop()

    def add_node_with_content(self, filepath, file_content):
        """Append Node and Fill Content"""
        self.nodes_list.append(self.dom.createElement("node"))
        node_name = os.path.basename(filepath)
        if node_name.lower().endswith(".htm"): node_name = node_name[:-4]
        elif node_name.lower().endswith(".html"): node_name = node_name[:-5]
        #print node_name, len(self.nodes_list)
        self.nodes_list[-1].setAttribute("name", node_name)
        self.nodes_list[-1].setAttribute("prog_lang", cons.RICH_TEXT_ID)
        self.nodes_list[-2].appendChild(self.nodes_list[-1])
        if file_content: self.rich_text_serialize(file_content)

    def get_cherrytree_xml(self, filepath="", folderpath=""):
        """Returns a CherryTree string Containing the HTML Nodes"""
        self.dom = xml.dom.minidom.Document()
        self.nodes_list = [self.dom.createElement(cons.APP_NAME)]
        self.dom.appendChild(self.nodes_list[0])
        if filepath: self.add_file(filepath)
        else: self.add_folder(folderpath)
        return self.dom.toxml()


class HTMLCheck(HTMLParser.HTMLParser):
    """Check for Minimal Tags"""

    def __init__(self):
        """Machine boot"""
        HTMLParser.HTMLParser.__init__(self)

    def handle_starttag(self, tag, attrs):
        """Encountered the beginning of a tag"""
        if tag == "html" and self.steps == 0: self.steps = 1
        elif tag == "head" and self.steps == 1: self.steps = 4
        elif tag == "body" and self.steps == 5: self.steps = 6

    def handle_endtag(self, tag):
        """Encountered the end of a tag"""
        if tag == "head" and self.steps == 4: self.steps = 5
        elif tag == "body" and self.steps == 6: self.steps = 7
        if tag == "html" and self.steps == 7: self.steps = 8

    def is_html_ok(self, input_string):
        """Checks for the minimal html tags"""
        self.steps = 0
        self.feed(input_string)
        return self.steps == 8
