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

enum class ExportType { None, Txt, Pdf, Html };

class ExportsMultipleParametersTests : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
};

TEST_P(ExportsMultipleParametersTests, ChecksExports)
{
    TestCtApp testCtApp{};
    const std::string inDocPath = std::get<0>(GetParam());
    const std::string exportSwitch = std::get<1>(GetParam());
    fs::path tmpDirpath = testCtApp.getCtTmp()->getHiddenDirPath("UT");
    fs::path tmpFilepath;
    ExportType exportType{ExportType::None};
    if (exportSwitch.find("txt") != std::string::npos) {
        exportType = ExportType::Txt;
        tmpFilepath = tmpDirpath / (Glib::path_get_basename(inDocPath)+".txt");
    }
    else if (exportSwitch.find("pdf") != std::string::npos) {
        exportType = ExportType::Pdf;
        tmpFilepath = tmpDirpath / (Glib::path_get_basename(inDocPath)+".pdf");
    }
    else if (exportSwitch.find("html") != std::string::npos) {
        exportType = ExportType::Html;
        
    }
    ASSERT_FALSE(ExportType::None == exportType);
    const std::vector<std::string> vec_args{"cherrytree", inDocPath, exportSwitch, tmpDirpath.string(), "--export_single_file"};
    gchar** pp_args = CtStrUtil::vector_to_array(vec_args);
    testCtApp.run(vec_args.size(), pp_args);
    ASSERT_TRUE(fs::is_regular_file(tmpFilepath));
    if (ExportType::Txt == exportType) {
        std::string expTxt_path{Glib::build_filename(UT::unitTestsDataDir, "test.export.txt")};
        std::string expTxt = Glib::file_get_contents(expTxt_path);
        std::string resultTxt = Glib::file_get_contents(tmpFilepath.string());
        ASSERT_FALSE(resultTxt.empty());
        ASSERT_STREQ(expTxt.c_str(), resultTxt.c_str());
    }
    else if (ExportType::Pdf == exportType) {
        ASSERT_NE(0, fs::file_size(tmpFilepath));
    }
    ASSERT_TRUE(fs::remove(tmpFilepath));
    ASSERT_FALSE(fs::is_regular_file(tmpFilepath));
    g_strfreev(pp_args);
}

INSTANTIATE_TEST_CASE_P(
        ExportsTests,
        ExportsMultipleParametersTests,
        ::testing::Values(std::make_tuple(UT::testCtbDocPath, "--export_to_txt_dir"),
                          std::make_tuple(UT::testCtdDocPath, "--export_to_txt_dir"),
                          std::make_tuple(UT::testCtbDocPath, "--export_to_pdf_dir"),
                          std::make_tuple(UT::testCtdDocPath, "--export_to_pdf_dir"))
);
