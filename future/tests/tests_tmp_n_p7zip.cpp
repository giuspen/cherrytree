/*
 * tests_tmp_n_p7zip.cc
 * 
 * Copyright 2018 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_app.h"
#include "p7za_iface.h"
#include <glib/gstdio.h>
#include <libxml++/libxml++.h>
#include "CppUTest/CommandLineTestRunner.h"


TEST_GROUP(TmpP7zipGroup)
{
};

TEST(TmpP7zipGroup, CTTmp_misc)
{
    std::string keyPath{"nomatter"};
    CTTmp* pCTmp = new CTTmp();
    std::string tempDir{pCTmp->getHiddenDirPath(keyPath)};
    CHECK(Glib::file_test(tempDir, Glib::FILE_TEST_IS_DIR));
    delete pCTmp;
    CHECK(!Glib::file_test(tempDir, Glib::FILE_TEST_IS_DIR));
}

TEST(TmpP7zipGroup, P7zaIfaceMisc)
{
    CTTmp ctTmp;
    const std::string ctzInputPath{Glib::build_filename(_UNITTEST_DATA_DIR, "7zr.ctz")};
    const std::string ctdTmpPath{Glib::build_filename(ctTmp.getHiddenDirPath(ctzInputPath), "7zr.ctd")};
    const std::string ctzTmpPath{Glib::build_filename(ctTmp.getHiddenDirPath(ctzInputPath), "7zr2.ctz")};
    const gchar testPassword[]{"7zr"};
    for (auto filepath : std::list<std::string>{ctdTmpPath, ctzTmpPath})
    {
        if (Glib::file_test(filepath, Glib::FILE_TEST_EXISTS))
        {
            g_remove(filepath.c_str());
        }
    }
    p7za_extract(ctzInputPath.c_str(), ctTmp.getHiddenDirPath(ctzInputPath), testPassword);
    CHECK(Glib::file_test(ctdTmpPath, Glib::FILE_TEST_EXISTS));
    std::string xml_txt = Glib::file_get_contents(ctdTmpPath);
    xmlpp::DomParser dom_parser;
    dom_parser.parse_memory(xml_txt);
    xmlpp::Document* p_document = dom_parser.get_document();
    xmlpp::Element* p_element = p_document->get_root_node();
    CHECK(0 == p_element->get_name().compare("cherrytree"));
    CHECK(0 == static_cast<xmlpp::Element*>(p_element->find("node")[0])->get_attribute_value("name").compare("NodeName"));
    CHECK(0 == static_cast<xmlpp::Element*>(p_element->find("node/rich_text")[0])->get_child_text()->get_content().compare("NodeContent"));
    p7za_archive(ctdTmpPath.c_str(), ctzTmpPath.c_str(), testPassword);
    CHECK(Glib::file_test(ctzTmpPath, Glib::FILE_TEST_EXISTS));
    g_remove(ctdTmpPath.c_str());
    p7za_extract(ctzTmpPath.c_str(), ctTmp.getHiddenDirPath(ctzInputPath), testPassword);
    CHECK(Glib::file_test(ctdTmpPath, Glib::FILE_TEST_EXISTS));
    std::string xml_txt_bis = Glib::file_get_contents(ctdTmpPath);
    CHECK(0 == xml_txt.compare(xml_txt_bis));
}
