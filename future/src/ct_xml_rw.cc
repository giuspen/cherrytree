/*
 * ct_xml_rw.cc
 * 
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <iostream>
#include <assert.h>
#include "ct_doc_rw.h"
#include "str_utils.h"


CherryTreeXMLRead::CherryTreeXMLRead(const char* filepath)
{
    parse_file(filepath);
}


CherryTreeXMLRead::~CherryTreeXMLRead()
{
}


void CherryTreeXMLRead::treeWalk(const Gtk::TreeIter *pParentIter)
{
    xmlpp::Document *p_document = get_document();
    assert(p_document != nullptr);
    xmlpp::Element *p_root = p_document->get_root_node();
    assert(p_root->get_name() == "cherrytree");
    for(xmlpp::Node *p_node : p_root->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xmlTreeWalkIter(static_cast<xmlpp::Element*>(p_node), pParentIter);
        }
        else if(p_node->get_name() == "bookmarks")
        {
            Glib::ustring bookmarks_csv = static_cast<xmlpp::Element*>(p_node)->get_attribute_value("list");
            for(gint64 &nodeId : gstring_split2int64(bookmarks_csv.c_str(), ","))
            {
                m_signal_add_bookmark.emit(nodeId);
            }
        }
    }
}


void CherryTreeXMLRead::_xmlTreeWalkIter(xmlpp::Element *p_node_element, const Gtk::TreeIter *pParentIter)
{
    Gtk::TreeIter new_iter = _xmlNodeProcess(p_node_element, pParentIter);

    for(xmlpp::Node *p_node : p_node_element->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xmlTreeWalkIter(static_cast<xmlpp::Element*>(p_node), &new_iter);
        }
    }
}


CtNodeData CherryTreeXMLRead::_xmlGetNodeProperties(xmlpp::Element *p_node_element)
{
    CtNodeData nodeData;
    nodeData.nodeId = gint64_from_gstring(p_node_element->get_attribute_value("unique_id").c_str());
    nodeData.name = p_node_element->get_attribute_value("name");
    nodeData.syntax = p_node_element->get_attribute_value("prog_lang");
    nodeData.tags = p_node_element->get_attribute_value("name");
    nodeData.isRO = Glib::str_has_prefix(p_node_element->get_attribute_value("readonly"), "T");
    nodeData.customIconId = gint64_from_gstring(p_node_element->get_attribute_value("customIconId").c_str());
    nodeData.isBold = Glib::str_has_prefix(p_node_element->get_attribute_value("isBold"), "T");
    Glib::ustring foregroundRgb24 = p_node_element->get_attribute_value("foreground");
    nodeData.fgOverride = !foregroundRgb24.empty();
    if(nodeData.fgOverride)
    {
        g_strlcpy(nodeData.foregroundRgb24, foregroundRgb24.c_str(), 8);
    }
    nodeData.tsCreation = gint64_from_gstring(p_node_element->get_attribute_value("tsCreation").c_str());
    nodeData.tsLastSave = gint64_from_gstring(p_node_element->get_attribute_value("tsLastSave").c_str());
    return nodeData;
}


Gtk::TreeIter CherryTreeXMLRead::_xmlNodeProcess(xmlpp::Element *p_node_element, const Gtk::TreeIter *pParentIter)
{
    CtNodeData nodeData = _xmlGetNodeProperties(p_node_element);
    Gtk::TreeIter new_iter = m_signal_append_node.emit(&nodeData, pParentIter);
    return new_iter;
}
