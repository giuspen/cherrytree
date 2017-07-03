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
// g++ xmldom.cc -o xmldom `pkg-config libxml++-2.6 --cflags --libs` -Wno-deprecated

#include <assert.h>
#include <iostream>
#include <libxml++/libxml++.h>
#include <glibmm.h>


std::list<Glib::ustring> ustring_split(const gchar *in_str, const gchar *delimiter, gint max_tokens=-1)
{
    std::list<Glib::ustring> ret_list;
    gchar** array_of_strings = g_strsplit(in_str, delimiter, max_tokens);
    for(gchar** ptr = array_of_strings; *ptr; ptr++)
    {
        ret_list.push_back(*ptr);
    }
    g_strfreev(array_of_strings);
    return ret_list;
}


class CherryTreeXML : public xmlpp::DomParser
{
public:
    void tree_walk();
    virtual void handle_cherry();
    virtual void handle_bookmarks(std::list<Glib::ustring>& bookmarks_list);
private:
    void _tree_walk_iter(const xmlpp::Element* node);
};


void CherryTreeXML::tree_walk()
{
    xmlpp::Document* document = get_document();
    assert(document != nullptr);
    xmlpp::Element* root = document->get_root_node();
    assert(root->get_name() == "cherrytree");
    for(xmlpp::Node* p_node : root->get_children())
    {
        if(p_node->get_name() == "node")
        {
            //_tree_walk_iter(p_node);
        }
        else if(p_node->get_name() == "bookmarks")
        {
            xmlpp::Attribute* p_attribute = static_cast<xmlpp::Element*>(p_node)->get_attribute("list");
            Glib::ustring bookmarks_csv = p_attribute->get_value();
            std::list<Glib::ustring> bookmarks_list = ustring_split(bookmarks_csv.c_str(), ",");
            handle_bookmarks(bookmarks_list);
        }
    }
}


void CherryTreeXML::_tree_walk_iter(const xmlpp::Element* p_node)
{
    
}


void CherryTreeXML::handle_cherry()
{
    
}


void CherryTreeXML::handle_bookmarks(std::list<Glib::ustring>& bookmarks_list)
{
    for(Glib::ustring str_elem : bookmarks_list)
    {
        std::cout << str_elem << std::endl;
    }
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
    CherryTreeXML ct_xml;
    ct_xml.parse_file(filepath);
    ct_xml.tree_walk();
}
