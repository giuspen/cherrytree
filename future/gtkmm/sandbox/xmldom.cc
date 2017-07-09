/*
 * xmldom.cc
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

// https://developer.gnome.org/libxml++/2.40/
// g++ xmldom.cc -o xmldom `pkg-config libxml++-2.6 gtkmm-3.0 --cflags --libs` -Wno-deprecated

#include <assert.h>
#include <iostream>
#include <libxml++/libxml++.h>
#include <glibmm.h>
#include <gtkmm.h>


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
protected:
    virtual void tree_walk(Gtk::TreeModel::Row *p_parent_row)=0;
    std::list<gint64> *mp_bookmarks;
    Glib::RefPtr<Gtk::TreeStore>  mr_treestore;
};


class CherryTreeXMLRead : public CherryTreeDocRead, public xmlpp::DomParser
{
public:
    CherryTreeXMLRead(Glib::ustring& filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    ~CherryTreeXMLRead();
    void tree_walk(Gtk::TreeModel::Row *p_parent_row);
private:
    void _xml_tree_walk_iter(xmlpp::Element* p_node_element, Gtk::TreeModel::Row *p_parent_row);
    Gtk::TreeModel::Row *_xml_node_process(xmlpp::Element* p_node_element, Gtk::TreeModel::Row *p_parent_row);
};


CherryTreeDocRead::CherryTreeDocRead(std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore) : mp_bookmarks(p_bookmarks), mr_treestore(r_treestore)
{
}


CherryTreeXMLRead::CherryTreeXMLRead(Glib::ustring &filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore) : CherryTreeDocRead(p_bookmarks, r_treestore)
{
    parse_file(filepath);
}


CherryTreeXMLRead::~CherryTreeXMLRead()
{
}


void CherryTreeXMLRead::tree_walk(Gtk::TreeModel::Row *p_parent_row)
{
    xmlpp::Document* document = get_document();
    assert(document != nullptr);
    xmlpp::Element* root = document->get_root_node();
    assert(root->get_name() == "cherrytree");
    for(xmlpp::Node* p_node : root->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xml_tree_walk_iter(static_cast<xmlpp::Element*>(p_node), p_parent_row);
        }
        else if(p_node->get_name() == "bookmarks")
        {
            Glib::ustring bookmarks_csv = static_cast<xmlpp::Element*>(p_node)->get_attribute_value("list");
            *mp_bookmarks = gstring_split2int64(bookmarks_csv.c_str(), ",");
        }
    }
}


void CherryTreeXMLRead::_xml_tree_walk_iter(xmlpp::Element* p_node_element, Gtk::TreeModel::Row *p_parent_row)
{
    Gtk::TreeModel::Row *p_new_node = _xml_node_process(p_node_element, p_parent_row);

    for(xmlpp::Node* p_node : p_node_element->get_children())
    {
        if(p_node->get_name() == "node")
        {
            _xml_tree_walk_iter(static_cast<xmlpp::Element*>(p_node), p_new_node);
        }
    }
}


Gtk::TreeModel::Row *CherryTreeXMLRead::_xml_node_process(xmlpp::Element* p_node_element, Gtk::TreeModel::Row *p_parent_row)
{
    Gtk::TreeModel::Row *p_new_node = nullptr;

    std::cout << p_node_element->get_attribute_value("name") << std::endl;

    return p_new_node;
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
    Gtk::TreeModel::Row *p_parent_row;

    CherryTreeXMLRead ct_xml_read(filepath, &bookmarks, r_treestore);
    ct_xml_read.tree_walk(p_parent_row);
}
