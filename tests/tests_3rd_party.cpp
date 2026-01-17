/*
 * tests_read_write.cpp
 *
 * Copyright 2009-2025
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
#include "ct_storage_control.h"
#include "tests_common.h"

class TestCtAppSC : public CtApp
{
public:
    TestCtAppSC()
     : CtApp{"_test_exports", Gio::APPLICATION_NON_UNIQUE}

    {
        _no_gui = true;
    }

private:
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) final;
    void _run_test(const fs::path doc_filepath_from, const fs::path doc_filepath_to);
};

void TestCtAppSC::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& /*hint*/)
{
    _on_startup();
    ASSERT_EQ(1, files.size());
    _run_test(files.front()->get_path(), _export_to_txt_dir);
}

enum class ExportType { None, Txt, Pdf, Html };

class ExportsMultipleParametersTests : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
};

TEST_P(ExportsMultipleParametersTests, ChecksExports)
{
    const std::string in_doc_path = std::get<0>(GetParam());
    const std::string out_doc_path = std::get<1>(GetParam());
    const std::vector<std::string> vec_args{"cherrytree", in_doc_path, "-t", out_doc_path};
    gchar** pp_args = CtStrUtil::vector_to_array(vec_args);
    TestCtAppSC testCtApp{};
    testCtApp.run(vec_args.size(), pp_args);
    g_strfreev(pp_args);
}

void TestCtAppSC::_run_test(const fs::path doc_filepath_from, const fs::path doc_filepath_to)
{
    const CtDocEncrypt docEncrypt_from = fs::get_doc_encrypt_from_file_ext(doc_filepath_from);
    const CtDocEncrypt docEncrypt_to = fs::get_doc_encrypt_from_file_ext(doc_filepath_to);

    CtMainWin* pWin = _create_window(true/*start_hidden*/);
    // tree empty
    ASSERT_FALSE(pWin->get_tree_store().get_iter_first());

    // load file
    ASSERT_TRUE(pWin->file_open(doc_filepath_from, ""/*node_to_focus*/, ""/*anchor_to_focus*/, docEncrypt_from != CtDocEncrypt::True ? "" : UT::testPassword));

    // Check if the node is present, with error text and read-only mode
    CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("ChatGPT");
    auto textContent = ctTreeIter.get_node_text_buffer()->get_text();
    auto isReadOnly = ctTreeIter.get_node_read_only();
    ASSERT_TRUE(textContent == "PARSING ERROR");
    ASSERT_TRUE(isReadOnly);
}

INSTANTIATE_TEST_CASE_P(
        ExportsTests,
        ExportsMultipleParametersTests,
        ::testing::Values(
            std::make_tuple(UT::testMultiFileSourCherry, UT::testCtzDocPath))
);
