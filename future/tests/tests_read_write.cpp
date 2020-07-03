/*
 * tests_read_write.cpp
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
#include "ct_misc_utils.h"
#include "tests_common.h"
#include "CppUTest/CommandLineTestRunner.h"

class TestCtApp : public CtApp
{
public:
    TestCtApp() : CtApp{} {}
private:
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;
    void _assert_tree_data(CtMainWin* pWin);
};

void TestCtApp::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint)
{
    CHECK_EQUAL(1, files.size());
    const fs::path doc_filepath_from{files.front()->get_path()};
    // NOTE: we use the trick of the [-t export_to_txt_dir] argument to pass the target file type
    const fs::path doc_filepath_to{_export_to_txt_dir};
    const CtDocEncrypt docEncrypt_from = fs::get_doc_encrypt(doc_filepath_from);
    const CtDocEncrypt docEncrypt_to = fs::get_doc_encrypt(doc_filepath_to);

    CtMainWin* pWin = _create_window(true/*start_hidden*/);
    // tree empty
    CHECK_FALSE(pWin->get_tree_store().get_iter_first());
    // load file
    CHECK(pWin->file_open(doc_filepath_from, "", docEncrypt_from != CtDocEncrypt::True ? "" : UT::testPassword));
    // do not check/walk the tree before calling the save_as to test that
    // even without visiting each node we save it all

    // save to temporary filepath
    fs::path tmp_dirpath = pWin->get_ct_tmp()->getHiddenDirPath("UT");
    fs::path tmp_filepath = tmp_dirpath / doc_filepath_to.filename();
    pWin->file_save_as(tmp_filepath.string(), docEncrypt_to != CtDocEncrypt::True ? "" : UT::testPasswordBis);

    // close this window/tree
    pWin->force_exit() = true;
    remove_window(*pWin);

    // new empty window/tree
    CtMainWin* pWin2 = _create_window(true/*start_hidden*/);
    // tree empty
    CHECK_FALSE(pWin2->get_tree_store().get_iter_first());
    // load file previously saved
    CHECK(pWin2->file_open(tmp_filepath, "", docEncrypt_to != CtDocEncrypt::True ? "" : UT::testPasswordBis));
    // check tree
    _assert_tree_data(pWin2);

    // close this window/tree
    pWin2->force_exit() = true;
    remove_window(*pWin2);
}

void TestCtApp::_assert_tree_data(CtMainWin* pWin)
{
    CtSummaryInfo summaryInfo{};
    pWin->get_tree_store().populateSummaryInfo(summaryInfo);
    CHECK_EQUAL(3, summaryInfo.nodes_rich_text_num);
    CHECK_EQUAL(1, summaryInfo.nodes_plain_text_num);
    CHECK_EQUAL(5, summaryInfo.nodes_code_num);
    CHECK_EQUAL(1, summaryInfo.images_num);
    CHECK_EQUAL(1, summaryInfo.embfile_num);
    CHECK_EQUAL(1, summaryInfo.tables_num);
    CHECK_EQUAL(1, summaryInfo.codeboxes_num);
    CHECK_EQUAL(1, summaryInfo.anchors_num);
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("йцукенгшщз");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("b");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("c");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("sh");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("html");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:1:0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("xml");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:1:1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("py");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:2", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("d");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("2", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("e");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("3", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
    }
}

TEST_GROUP(CtDocRWGroup)
{
};

#ifndef __APPLE__ // TestCtApp causes crash on macos

void test_read_write(const std::vector<std::string>& vec_args)
{
    TestCtApp testCtApp{};
    gchar** pp_args = CtStrUtil::vector_to_array(vec_args);
    testCtApp.run(vec_args.size(), pp_args);
    g_strfreev(pp_args);
}

TEST(CtDocRWGroup, CtDocRW_all_variants)
{
    for (const std::string& in_doc_path : UT::testAllDocTypes) {
        for (const std::string& out_doc_path : UT::testAllDocTypes) {
            const std::vector<std::string> vecArgs{"cherrytree", in_doc_path, "-t", out_doc_path};
            test_read_write(vecArgs);
        }
    }
}

#endif // __APPLE__
