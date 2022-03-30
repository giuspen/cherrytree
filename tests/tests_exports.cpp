/*
 * tests_exports.cpp
 *
 * Copyright 2009-2022
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
     : CtApp{"_test_exports"}
    {
        _no_gui = true;
        _on_startup(); // so that _uCtTmp is ready straight away
        _uCtCfg->usePandoc = false; // ensure html from gtksourceview syntax highlighting
    }
    CtTmp* getCtTmp() { return _uCtTmp.get(); }
    void register_args(const std::vector<std::string>* pVecArgs) { _pVecArgs = pVecArgs; }

private:
    void on_activate() final;

    const std::vector<std::string>* _pVecArgs{nullptr};
};

void TestCtApp::on_activate()
{
    // NOTE: on windows/msys2 unit tests the passed arguments do not work so we end up here
    ASSERT_TRUE(_pVecArgs);
    if (_pVecArgs->at(2) == "--export_to_txt_dir") {
        _export_to_txt_dir = _pVecArgs->at(3);
    }
    else if (_pVecArgs->at(2) == "--export_to_pdf_dir") {
        _export_to_pdf_dir = _pVecArgs->at(3);
    }
    else if (_pVecArgs->at(2) == "--export_to_html_dir") {
        _export_to_html_dir = _pVecArgs->at(3);
    }
    if (std::find(_pVecArgs->begin(), _pVecArgs->end(), "--export_single_file") != _pVecArgs->end()) {
        _export_single_file = true;
    }
    Glib::RefPtr<Gio::File> rFile =  Gio::File::create_for_path(_pVecArgs->at(1));
    Gio::Application::type_vec_files files{rFile};
    on_open(files, "");
}

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
        tmpFilepath = tmpDirpath / (Glib::path_get_basename(inDocPath)+"_HTML") / "index.html";
    }
    ASSERT_FALSE(ExportType::None == exportType);
    const std::vector<std::string> vec_args{"cherrytree", inDocPath, exportSwitch, tmpDirpath.string(), "--export_single_file"};
    testCtApp.register_args(&vec_args);
    gchar** pp_args = CtStrUtil::vector_to_array(vec_args);
    testCtApp.run(vec_args.size(), pp_args);
    ASSERT_TRUE(fs::is_regular_file(tmpFilepath));
    if (ExportType::Txt == exportType) {
        std::string expectTxt_path{Glib::build_filename(UT::unitTestsDataDir, "test.export.txt")};
        std::string expectTxt = Glib::file_get_contents(expectTxt_path);
        std::string resultTxt = Glib::file_get_contents(tmpFilepath.string());
        ASSERT_FALSE(resultTxt.empty());
        //g_file_set_contents(Glib::build_filename(UT::unitTestsDataDir, "test.export.txtt").c_str(), resultTxt.c_str(), -1, NULL);
#if defined(_WIN32)
        ASSERT_STREQ(str::replace(expectTxt, "\n", "\r\n").c_str(), resultTxt.c_str());
#else
        ASSERT_STREQ(expectTxt.c_str(), resultTxt.c_str());
#endif
    }
    else if (ExportType::Pdf == exportType) {
        ASSERT_NE(0, fs::file_size(tmpFilepath));
    }
    else if (ExportType::Html == exportType) {
        std::string expectHtml_path{Glib::build_filename(UT::unitTestsDataDir, "test.export.html")};
        std::string expectHtml = str::replace(Glib::file_get_contents(expectHtml_path),
                                              "_REPLACE_TITLE_",
                                              Glib::path_get_basename(inDocPath));
        std::string resultHtml = Glib::file_get_contents(tmpFilepath.string());
        ASSERT_FALSE(resultHtml.empty());
        //g_file_set_contents(Glib::build_filename(UT::unitTestsDataDir, "test.export.htmll").c_str(), resultHtml.c_str(), -1, NULL);
        ASSERT_STREQ(expectHtml.c_str(), resultHtml.c_str());
    }
    if (ExportType::Html != exportType) {
        ASSERT_TRUE(fs::remove(tmpFilepath));
        ASSERT_FALSE(fs::is_regular_file(tmpFilepath));
    }
    g_strfreev(pp_args);
}

INSTANTIATE_TEST_CASE_P(
        ExportsTests,
        ExportsMultipleParametersTests,
        ::testing::Values(std::make_tuple(UT::testCtbDocPath, "--export_to_txt_dir"),
                          std::make_tuple(UT::testCtdDocPath, "--export_to_txt_dir"),
                          std::make_tuple(UT::testCtbDocPath, "--export_to_pdf_dir"),
                          std::make_tuple(UT::testCtdDocPath, "--export_to_pdf_dir"),
                          std::make_tuple(UT::testCtbDocPath, "--export_to_html_dir"),
                          std::make_tuple(UT::testCtdDocPath, "--export_to_html_dir"))
);
