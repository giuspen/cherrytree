/*
 * tests_tmp_n_p7zip.cpp
 *
 * Copyright 2009-2020
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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
#include "ct_p7za_iface.h"
#include "config.h"
#include "ct_filesystem.h"
#include "tests_common.h"

#include <glib/gstdio.h>
#include <libxml++/libxml++.h>
#include "CppUTest/CommandLineTestRunner.h"


TEST_GROUP(TmpP7zipGroup)
{
};

TEST(TmpP7zipGroup, CTTmp_misc)
{
    CtTmp* pCTmp = new CtTmp();

    // temporary directories are created when their path is first queried
    std::string tempDirCtz{pCTmp->getHiddenDirPath(UT::ctzInputPath).string()};
    std::string tempDirCtx{pCTmp->getHiddenDirPath(UT::ctxInputPath).string()};
    std::string tempFileCtz{pCTmp->getHiddenFilePath(UT::ctzInputPath).string()};
    std::string tempFileCtx{pCTmp->getHiddenFilePath(UT::ctxInputPath).string()};

    CHECK(Glib::file_test(tempDirCtz, Glib::FILE_TEST_IS_DIR));
    CHECK(Glib::file_test(tempDirCtx, Glib::FILE_TEST_IS_DIR));

    // different temporary directoryes per different extracted file
    CHECK(0 != tempDirCtz.compare(tempDirCtx));

    const std::string ctdTmpPath{Glib::build_filename(tempDirCtz, "7zr.ctd")};
    const std::string ctbTmpPath{Glib::build_filename(tempDirCtx, "7zr.ctb")};

    // check for extracted filepaths correctness for CTZ and CTX
    STRCMP_EQUAL(tempFileCtz.c_str(), ctdTmpPath.c_str());
    STRCMP_EQUAL(tempFileCtx.c_str(), ctbTmpPath.c_str());

    // check for dirpaths non existence after the class is deleted
    delete pCTmp;
    CHECK_FALSE(Glib::file_test(tempDirCtz, Glib::FILE_TEST_IS_DIR));
    CHECK_FALSE(Glib::file_test(tempDirCtx, Glib::FILE_TEST_IS_DIR));
}

TEST(TmpP7zipGroup, P7zaIfaceMisc)
{
    // extract our test archive
    CtTmp ctTmp;
    CHECK_EQUAL(0, CtP7zaIface::p7za_extract(UT::ctzInputPath.c_str(), ctTmp.getHiddenDirPath(UT::ctzInputPath).c_str(), UT::testPassword, false));
    CHECK(Glib::file_test(ctTmp.getHiddenFilePath(UT::ctzInputPath).string(), Glib::FILE_TEST_EXISTS));

    // read and parse xml of extracted archive
    std::string xml_txt = Glib::file_get_contents(ctTmp.getHiddenFilePath(UT::ctzInputPath).string());
    xmlpp::DomParser dom_parser;
    dom_parser.parse_memory(xml_txt);
    xmlpp::Document* p_document = dom_parser.get_document();
    xmlpp::Element* p_element = p_document->get_root_node();
    STRCMP_EQUAL("cherrytree", p_element->get_name().c_str());
    STRCMP_EQUAL("NodeName", static_cast<xmlpp::Element*>(p_element->find("node")[0])->get_attribute_value("name").c_str());
    STRCMP_EQUAL("NodeContent", static_cast<xmlpp::Element*>(p_element->find("node/rich_text")[0])->get_child_text()->get_content().c_str());

    // try and archive again the extracted xml
    const std::string ctzTmpPathBis{Glib::build_filename(ctTmp.getHiddenDirPath(UT::ctzInputPath).string(), "7zr2.ctz")};
    CHECK_EQUAL(0, CtP7zaIface::p7za_archive(ctTmp.getHiddenFilePath(UT::ctzInputPath).c_str(), ctzTmpPathBis.c_str(), UT::testPasswordBis));

    CHECK(Glib::file_test(ctzTmpPathBis, Glib::FILE_TEST_EXISTS));

    // remove originally extracted
    CHECK_EQUAL(0, g_remove(ctTmp.getHiddenFilePath(UT::ctzInputPath).c_str()));
    CHECK_FALSE(Glib::file_test(ctTmp.getHiddenFilePath(UT::ctzInputPath).string(), Glib::FILE_TEST_EXISTS));

    // extract again from the archive that we created
    CHECK_EQUAL(0, CtP7zaIface::p7za_extract(ctzTmpPathBis.c_str(), ctTmp.getHiddenDirPath(UT::ctzInputPath).c_str(), UT::testPasswordBis, false));
    CHECK(Glib::file_test(ctTmp.getHiddenFilePath(UT::ctzInputPath).string(), Glib::FILE_TEST_EXISTS));
    std::string xml_txt_bis = Glib::file_get_contents(ctTmp.getHiddenFilePath(UT::ctzInputPath).string());
    STRCMP_EQUAL(xml_txt.c_str(), xml_txt_bis.c_str());

    // remove alien/unexpected files in temp directory
    for (auto tmpFilepath : std::list<std::string>{ctzTmpPathBis})
    {
        if (Glib::file_test(tmpFilepath, Glib::FILE_TEST_EXISTS))
        {
            CHECK_EQUAL(0, g_remove(tmpFilepath.c_str()));
        }
    }

}

TEST(TmpP7zipGroup, P7zaExtravtWrongPasswd)
{
    CtTmp ctTmp;
    const std::string ctdTmpPath{(ctTmp.getHiddenDirPath(UT::ctzInputPath) /"7zr.ctd").string()};

    // wrong password
    CHECK(0 != CtP7zaIface::p7za_extract(UT::ctzInputPath.c_str(), ctTmp.getHiddenDirPath(UT::ctzInputPath).c_str(), "wrongpassword", true));
    CHECK_FALSE(Glib::file_test(ctdTmpPath, Glib::FILE_TEST_EXISTS));

    // correct password
    CHECK_EQUAL(0, CtP7zaIface::p7za_extract(UT::ctzInputPath.c_str(), ctTmp.getHiddenDirPath(UT::ctzInputPath).c_str(), UT::testPassword, false));
    CHECK_TRUE(Glib::file_test(ctdTmpPath, Glib::FILE_TEST_EXISTS));
    g_remove(ctTmp.getHiddenFilePath(UT::ctzInputPath).string().c_str());
}
