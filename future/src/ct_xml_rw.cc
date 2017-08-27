/*
 * ct_xml_rw.cc
 * 
 * Copyright 2017 giuspen <giuspen@gmail.com>
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


CherryTreeXMLRead::CherryTreeXMLRead(Glib::ustring &filepath)
{
    parse_file(filepath);
}


CherryTreeXMLRead::~CherryTreeXMLRead()
{
}


void CherryTreeXMLRead::tree_walk(Gtk::TreeIter *p_parent_iter)
{
    xmlpp::Document *p_document = get_document();
    assert(p_document != nullptr);
    xmlpp::Element *p_root = p_document->get_root_node();
    assert(p_root->get_name() == "cherrytree");
    for(xmlpp::Node *p_node : p_root->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xml_tree_walk_iter(static_cast<xmlpp::Element*>(p_node), p_parent_iter);
        }
        else if(p_node->get_name() == "bookmarks")
        {
            Glib::ustring bookmarks_csv = static_cast<xmlpp::Element*>(p_node)->get_attribute_value("list");
            for(gint64 &node_id : gstring_split2int64(bookmarks_csv.c_str(), ","))
            {
                m_signal_add_bookmark.emit(node_id);
            }
        }
    }
}


void CherryTreeXMLRead::_xml_tree_walk_iter(xmlpp::Element *p_node_element, Gtk::TreeIter *p_parent_iter)
{
    Gtk::TreeIter new_iter = _xml_node_process(p_node_element, p_parent_iter);

    for(xmlpp::Node *p_node : p_node_element->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xml_tree_walk_iter(static_cast<xmlpp::Element*>(p_node), &new_iter);
        }
    }
}


t_ct_node_data CherryTreeXMLRead::_xml_get_node_properties(xmlpp::Element *p_node_element)
{
    t_ct_node_data node_data;
    node_data.node_id = gint64_from_gstring(p_node_element->get_attribute_value("unique_id").c_str());
    node_data.name = p_node_element->get_attribute_value("name");
    node_data.syntax = p_node_element->get_attribute_value("prog_lang");
    node_data.tags = p_node_element->get_attribute_value("name");
    node_data.is_ro = Glib::str_has_prefix(p_node_element->get_attribute_value("readonly"), "T");
    node_data.custom_icon_id = gint64_from_gstring(p_node_element->get_attribute_value("custom_icon_id").c_str());
    node_data.is_bold = Glib::str_has_prefix(p_node_element->get_attribute_value("is_bold"), "T");
    Glib::ustring foreground_rgb24 = p_node_element->get_attribute_value("foreground");
    node_data.fg_override = !foreground_rgb24.empty();
    if(node_data.fg_override)
    {
        g_strlcpy(node_data.foreground_rgb24, foreground_rgb24.c_str(), 8);
    }
    node_data.ts_creation = gint64_from_gstring(p_node_element->get_attribute_value("ts_creation").c_str());
    node_data.ts_lastsave = gint64_from_gstring(p_node_element->get_attribute_value("ts_lastsave").c_str());
    return node_data;
}


Gtk::TreeIter CherryTreeXMLRead::_xml_node_process(xmlpp::Element *p_node_element, Gtk::TreeIter *p_parent_iter)
{
    t_ct_node_data node_data = _xml_get_node_properties(p_node_element);
    Gtk::TreeIter new_iter = m_signal_append_node.emit(&node_data, p_parent_iter);
    return new_iter;
}
