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
protected:
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;
};

void TestCtApp::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint)
{
    CHECK_EQUAL(1, files.size());
    CtMainWin* pWin = _create_window(true/*start_hidden*/);
    CHECK(pWin->file_open(files.front()->get_path(), ""));

    CtSummaryInfo summaryInfo{};
    pWin->get_tree_store().populateSummaryInfo(summaryInfo);
    CHECK_EQUAL(3, summaryInfo.nodes_rich_text_num);
    CHECK_EQUAL(1, summaryInfo.nodes_plain_text_num);
    CHECK_EQUAL(1, summaryInfo.nodes_code_num);
    CHECK_EQUAL(1, summaryInfo.images_num);
    CHECK_EQUAL(1, summaryInfo.embfile_num);
    CHECK_EQUAL(1, summaryInfo.tables_num);
    CHECK_EQUAL(1, summaryInfo.codeboxes_num);
    CHECK_EQUAL(1, summaryInfo.anchors_num);

    pWin->force_exit() = true;
    remove_window(*pWin);
}

TEST_GROUP(CtDocRWGroup)
{
};

TEST(CtDocRWGroup, CtDocRWCtb)
{
    TestCtApp testCtApp{};
    const std::vector<std::string> vecArgs{"cherrytree", testCtbDocPath};
    gchar** pp_args = CtStrUtil::vector_to_array(vecArgs);
    testCtApp.run(vecArgs.size(), pp_args);
    g_strfreev(pp_args);
}

TEST(CtDocRWGroup, CtDocRWCtd)
{
    TestCtApp testCtApp{};
    const std::vector<std::string> vecArgs{"cherrytree", testCtdDocPath};
    gchar** pp_args = CtStrUtil::vector_to_array(vecArgs);
    testCtApp.run(vecArgs.size(), pp_args);
    g_strfreev(pp_args);
}
