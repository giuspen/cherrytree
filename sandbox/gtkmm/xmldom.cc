/*
 * xmldom.cc
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

// https://developer.gnome.org/libxml++/2.40/
// g++ xmldom.cc -o xmldom `pkg-config libxml++-2.6 gtkmm-3.0 --cflags --libs` -Wno-deprecated

#include <assert.h>
#include <iostream>
#include <libxml++/libxml++.h>
#include <gtkmm.h>


struct t_node_properties
{
    gint64         node_id;
    Glib::ustring  name;
    Glib::ustring  syntax;
    Glib::ustring  tags;
    bool           is_ro;
    guint32        custom_icon_id;
    bool           is_bold;
    bool           fg_override;
    char           foreground_rgb24[8];
    gint64         ts_creation;
    gint64         ts_lastsave;
};


gint64 gint64_from_gstring(const gchar *in_gstring)
{
    return g_ascii_strtoll(in_gstring, NULL, 10);
}


std::list<gint64> gstring_split2int64(const gchar *in_str, const gchar *delimiter, gint max_tokens=-1)
{
    std::list<gint64> ret_list;
    gchar **array_of_strings = g_strsplit(in_str, delimiter, max_tokens);
    for(gchar **ptr = array_of_strings; *ptr; ptr++)
    {
        gint64 curr_int = g_ascii_strtoll(*ptr, NULL, 10);
        ret_list.push_back(curr_int);
    }
    g_strfreev(array_of_strings);
    return ret_list;
}


class CherryTreeDocRead
{
public:
    CherryTreeDocRead(std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    virtual ~CherryTreeDocRead();
    virtual void tree_walk(Gtk::TreeIter parent_iter)=0;
protected:
    std::list<gint64> *mp_bookmarks;
    Glib::RefPtr<Gtk::TreeStore>  mr_treestore;
};


class CherryTreeXMLRead : public CherryTreeDocRead, public xmlpp::DomParser
{
public:
    CherryTreeXMLRead(Glib::ustring& filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    virtual ~CherryTreeXMLRead();
    void tree_walk(Gtk::TreeIter parent_iter);
private:
    void _xml_tree_walk_iter(xmlpp::Element *p_node_element, Gtk::TreeIter parent_iter);
    t_node_properties _xml_get_node_properties(xmlpp::Element *p_node_element);
    Gtk::TreeIter _xml_node_process(xmlpp::Element *p_node_element, Gtk::TreeIter parent_iter);
};


CherryTreeDocRead::CherryTreeDocRead(std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore) : mp_bookmarks(p_bookmarks), mr_treestore(r_treestore)
{
}


CherryTreeDocRead::~CherryTreeDocRead()
{
}


CherryTreeXMLRead::CherryTreeXMLRead(Glib::ustring &filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore) : CherryTreeDocRead(p_bookmarks, r_treestore)
{
    parse_file(filepath);
}


CherryTreeXMLRead::~CherryTreeXMLRead()
{
}


void CherryTreeXMLRead::tree_walk(Gtk::TreeIter parent_iter)
{
    xmlpp::Document *p_document = get_document();
    assert(p_document != nullptr);
    xmlpp::Element *p_root = p_document->get_root_node();
    assert(p_root->get_name() == "cherrytree");
    for(xmlpp::Node *p_node : p_root->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xml_tree_walk_iter(static_cast<xmlpp::Element*>(p_node), parent_iter);
        }
        else if(p_node->get_name() == "bookmarks")
        {
            Glib::ustring bookmarks_csv = static_cast<xmlpp::Element*>(p_node)->get_attribute_value("list");
            for(gint64 &node_id : gstring_split2int64(bookmarks_csv.c_str(), ","))
            {
                mp_bookmarks->push_back(node_id);
            }
        }
    }
}


void CherryTreeXMLRead::_xml_tree_walk_iter(xmlpp::Element *p_node_element, Gtk::TreeIter parent_iter)
{
    Gtk::TreeIter new_iter = _xml_node_process(p_node_element, parent_iter);

    for(xmlpp::Node *p_node : p_node_element->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xml_tree_walk_iter(static_cast<xmlpp::Element*>(p_node), new_iter);
        }
    }
}


t_node_properties CherryTreeXMLRead::_xml_get_node_properties(xmlpp::Element *p_node_element)
{
    t_node_properties node_properties;
    node_properties.node_id = gint64_from_gstring(p_node_element->get_attribute_value("unique_id").c_str());
    node_properties.name = p_node_element->get_attribute_value("name");
    node_properties.syntax = p_node_element->get_attribute_value("prog_lang");
    node_properties.tags = p_node_element->get_attribute_value("name");
    node_properties.is_ro = Glib::str_has_prefix(p_node_element->get_attribute_value("readonly"), "T");
    node_properties.custom_icon_id = gint64_from_gstring(p_node_element->get_attribute_value("custom_icon_id").c_str());
    node_properties.is_bold = Glib::str_has_prefix(p_node_element->get_attribute_value("is_bold"), "T");
    Glib::ustring foreground_rgb24 = p_node_element->get_attribute_value("foreground");
    node_properties.fg_override = !foreground_rgb24.empty();
    if(node_properties.fg_override)
    {
        g_strlcpy(node_properties.foreground_rgb24, foreground_rgb24.c_str(), 8);
    }
    node_properties.ts_creation = gint64_from_gstring(p_node_element->get_attribute_value("ts_creation").c_str());
    node_properties.ts_lastsave = gint64_from_gstring(p_node_element->get_attribute_value("ts_lastsave").c_str());
    return node_properties;
}


Gtk::TreeIter CherryTreeXMLRead::_xml_node_process(xmlpp::Element *p_node_element, Gtk::TreeIter parent_iter)
{
    t_node_properties node_properties = _xml_get_node_properties(p_node_element);
    Gtk::TreeIter new_iter;

    std::cout << node_properties.name << std::endl;

    return new_iter;
}


int main(int argc, char *argv[])
{
    std::locale::global(std::locale("")); // Set the global C++ locale to the user-specified locale
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " FILEPATH.CTD" << std::endl;
        return 1;
    }
    Glib::ustring filepath(argv[1]);
    assert(Glib::file_test(filepath, Glib::FILE_TEST_EXISTS));

    std::list<gint64> bookmarks;
    Glib::RefPtr<Gtk::TreeStore> r_treestore;
    Gtk::TreeIter parent_iter;

    CherryTreeXMLRead ct_xml_read(filepath, &bookmarks, r_treestore);
    ct_xml_read.tree_walk(parent_iter);
}
