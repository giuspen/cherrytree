/*
 * tests_exports.cpp
 *
 * Copyright 2009-2021
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

class TestCtApp : public CtApp
{
public:
    TestCtApp()
     : CtApp{"com.giuspen.cherrytree_test_exports"}
    {
        _on_startup(); // so that _uCtTmp is ready straight away
    }
    CtTmp* getCtTmp() { return _uCtTmp.get(); }
};

class ExportToTxtMultipleParametersTests : public ::testing::TestWithParam<std::string>
{
};

TEST_P(ExportToTxtMultipleParametersTests, ChecksExportToTxt)
{
    TestCtApp testCtApp{};
    const std::string inDocPath = GetParam();
    fs::path tmpDirpath = testCtApp.getCtTmp()->getHiddenDirPath("UT");
    const std::vector<std::string> vec_args{"cherrytree", inDocPath, "--export_to_txt_dir", tmpDirpath.string(), "--export_single_file"};
    gchar** pp_args = CtStrUtil::vector_to_array(vec_args);
    testCtApp.run(vec_args.size(), pp_args);
    fs::path tmpFilepath = tmpDirpath / (Glib::path_get_basename(inDocPath)+".txt");
    ASSERT_TRUE(fs::is_regular_file(tmpFilepath));
    std::string expTxt_path{Glib::build_filename(UT::unitTestsDataDir, "test.export.txt")};
    std::string expTxt = Glib::file_get_contents(expTxt_path);
    std::string resultTxt = Glib::file_get_contents(tmpFilepath.string());
    ASSERT_FALSE(resultTxt.empty());
    ASSERT_STREQ(expTxt.c_str(), resultTxt.c_str());
    ASSERT_TRUE(fs::remove(tmpFilepath));
    ASSERT_FALSE(fs::is_regular_file(tmpFilepath));
    g_strfreev(pp_args);
}

INSTANTIATE_TEST_CASE_P(
        ExportToTxtTests,
        ExportToTxtMultipleParametersTests,
        ::testing::Values(UT::testCtbDocPath,
                          UT::testCtdDocPath)
);
