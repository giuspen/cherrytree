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
    struct ExpectedTag {
        Glib::ustring text_slot;
        bool found{false};
        CtTextIterUtil::CurrAttributesMap attr_map;
    };
private:
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;
    void _assert_tree_data(CtMainWin* pWin);
    void _assert_node_text(CtTreeIter& ctTreeIter, const Glib::ustring& expectedText);
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

void TestCtApp::_assert_node_text(CtTreeIter& ctTreeIter, const Glib::ustring& expectedText)
{
    const Glib::RefPtr<Gsv::Buffer> rTextBuffer = ctTreeIter.get_node_text_buffer();
    CHECK(static_cast<bool>(rTextBuffer));
    STRCMP_EQUAL(expectedText.c_str(), rTextBuffer->get_text().c_str());
}

#define _NL "\n"

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
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("йцукенгшщз", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("plain-text", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "ciao plain" _NL
            "йцукенгшщз"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("b");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("custom-colors", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "ciao rich" _NL
            "fore" _NL
            "back" _NL
            "bold" _NL
            "italic" _NL
            "under" _NL
            "strike" _NL
            "h1" _NL
            "h2" _NL
            "h3" _NL
            "small" _NL
            "asuper" _NL
            "asub" _NL
            "mono" _NL
        };
        _assert_node_text(ctTreeIter, expectedText);
        std::list<ExpectedTag> expectedTags = {
            ExpectedTag{
                .text_slot="ciao rich",
                .attr_map=CtTextIterUtil::CurrAttributesMap{}},
            ExpectedTag{
                .text_slot="fore",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_FOREGROUND, "#ffff00000000"}}},
            ExpectedTag{
                .text_slot="back",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_BACKGROUND, "#e6e6e6e6fafa"}}},
            ExpectedTag{
                .text_slot="bold",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY}}},
            ExpectedTag{
                .text_slot="italic",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC}}},
            ExpectedTag{
                .text_slot="under",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE}}},
            ExpectedTag{
                .text_slot="strike",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE}}},
            ExpectedTag{
                .text_slot="h1",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H1}}},
            ExpectedTag{
                .text_slot="h2",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H2}}},
            ExpectedTag{
                .text_slot="h3",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3}}},
            ExpectedTag{
                .text_slot="small",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SMALL}}},
            ExpectedTag{
                .text_slot="super",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP}}},
            ExpectedTag{
                .text_slot="sub",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB}}},
            ExpectedTag{
                .text_slot="mono",
                .attr_map=CtTextIterUtil::CurrAttributesMap{{CtConst::TAG_FAMILY, CtConst::TAG_PROP_VAL_MONOSPACE}}},
        };
        CtTextIterUtil::SerializeFunc test_slot = [&](Gtk::TextIter& start_iter,
                                                      Gtk::TextIter& end_iter,
                                                      CtTextIterUtil::CurrAttributesMap& curr_attributes)
        {
            const Glib::ustring slot_text = start_iter.get_text(end_iter);
            for (auto& expTag : expectedTags) {
                if (slot_text.find(expTag.text_slot) != std::string::npos) {
                    expTag.found = true;
                    for (const auto& currPair : curr_attributes) {
                        if (expTag.attr_map.count(currPair.first) != 0) {
                            // we defined it
                            STRCMP_EQUAL(expTag.attr_map[currPair.first].c_str(), currPair.second.c_str());
                        }
                        else {
                            // we haven't defined, expect empty!
                            STRCMP_EQUAL("", currPair.second.c_str());
                        }
                    }
                    break;
                }
            }
        };
        CtTextIterUtil::generic_process_slot(0, -1, ctTreeIter.get_node_text_buffer(), test_slot);
        for (auto& expTag : expectedTags) {
            CHECK(expTag.found);
        }
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("c");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("c", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "int main(int argc, char *argv[])" _NL
            "{" _NL
            "    return 0;" _NL
            "}" _NL
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("sh");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("sh", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "echo \"ciao!\""
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("html");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:1:0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("html", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "<head>" _NL
            "<title>NO</title>" _NL
            "</head>"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("xml");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:1:1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("xml", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("py");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("1:2", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("python3", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "print(\"ciao!\")"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("d");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("2", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK(ctTreeIter.get_node_is_bold());
        CHECK(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(45, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("ciao", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("#ff0000", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("custom-colors", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "second rich" _NL
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("e");
        CHECK(ctTreeIter);
        STRCMP_EQUAL("3", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        CHECK_FALSE(ctTreeIter.get_node_is_bold());
        CHECK_FALSE(ctTreeIter.get_node_read_only());
        CHECK_EQUAL(0, ctTreeIter.get_node_custom_icon_id());
        STRCMP_EQUAL("", ctTreeIter.get_node_tags().c_str());
        STRCMP_EQUAL("", ctTreeIter.get_node_foreground().c_str());
        STRCMP_EQUAL("custom-colors", ctTreeIter.get_node_syntax_highlighting().c_str());
        CHECK_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "anchored widgets:" _NL
            _NL
            "codebox:" _NL
            _NL
            _NL
            "anchor:" _NL
            _NL
            _NL
            "table:" _NL
            _NL
            _NL
            "image:" _NL
            _NL
            _NL
            "embedded file:" _NL
            _NL
            _NL
            "link to web" _NL
            "link to node" _NL
            "link to node+anchor" _NL
            "link to folder" _NL
            "link to file" _NL
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
}

TEST_GROUP(CtDocRWGroup)
{
};

#ifndef __APPLE__ // TestCtApp causes crash on macos

TEST(CtDocRWGroup, CtDocRW_all_variants)
{
    for (const std::string& in_doc_path : UT::testAllDocTypes) {
        for (const std::string& out_doc_path : UT::testAllDocTypes) {
            const std::vector<std::string> vec_args{"cherrytree", in_doc_path, "-t", out_doc_path};
            gchar** pp_args = CtStrUtil::vector_to_array(vec_args);
            TestCtApp testCtApp{};
            testCtApp.run(vec_args.size(), pp_args);
            g_strfreev(pp_args);
        }
    }
}

#endif // __APPLE__
