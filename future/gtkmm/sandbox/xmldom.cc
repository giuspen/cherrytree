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

using namespace std;


void parse_ctd(const Glib::ustring& filepath)
{
    cout << filepath << endl;
    xmlpp::DomParser  parser;
    parser.parse_file(filepath);
    xmlpp::Document* document = parser.get_document();
    assert(document != nullptr);
    xmlpp::Element* root = document->get_root_node();
    cout << root->get_name() << endl;
}


int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        cerr << "Usage: " << argv[0] << " FILEPATH.CTD" << endl;
        return 1;
    }
    Glib::ustring filepath(argv[1]);
    assert(Glib::file_test(filepath, Glib::FILE_TEST_EXISTS));
    parse_ctd(filepath);
}
