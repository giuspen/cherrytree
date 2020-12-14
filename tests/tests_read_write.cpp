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

class TestCtApp : public CtApp
{
public:
    TestCtApp(const std::vector<std::string>& vec_args)
     : CtApp{},
       _vec_args{vec_args}
    {}

    struct ExpectedTag {
        Glib::ustring text_slot;
        bool found{false};
        CtCurrAttributesMap attr_map;
    };

private:
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) final;
    void on_activate() final;

    void _run_test(const fs::path doc_filepath_from, const fs::path doc_filepath_to);
    void _assert_tree_data(CtMainWin* pWin);
    void _assert_node_text(CtTreeIter& ctTreeIter, const Glib::ustring& expectedText);
    void _process_rich_text_buffer(std::list<ExpectedTag>& expectedTags, Glib::RefPtr<Gsv::Buffer> rTextBuffer);

    const std::vector<std::string>& _vec_args;
};

void TestCtApp::on_activate()
{
    _on_startup();
    ASSERT_EQ(4, _vec_args.size());
    // NOTE: on windows/msys2 unit tests the passed arguments do not work so we end up here
    _run_test(_vec_args.at(1), _vec_args.at(3));
}

void TestCtApp::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& /*hint*/)
{
    _on_startup();
    ASSERT_EQ(1, files.size());
    // NOTE: we use the trick of the [-t export_to_txt_dir] argument to pass the target file type
    _run_test(files.front()->get_path(), _export_to_txt_dir);
}

void TestCtApp::_run_test(const fs::path doc_filepath_from, const fs::path doc_filepath_to)
{
    const CtDocEncrypt docEncrypt_from = fs::get_doc_encrypt(doc_filepath_from);
    const CtDocEncrypt docEncrypt_to = fs::get_doc_encrypt(doc_filepath_to);

    CtMainWin* pWin = _create_window(true/*start_hidden*/);
    // tree empty
    ASSERT_FALSE(pWin->get_tree_store().get_iter_first());
    // load file
    ASSERT_TRUE(pWin->file_open(doc_filepath_from, "", docEncrypt_from != CtDocEncrypt::True ? "" : UT::testPassword));
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
    ASSERT_FALSE(pWin2->get_tree_store().get_iter_first());
    // load file previously saved
    ASSERT_TRUE(pWin2->file_open(tmp_filepath, "", docEncrypt_to != CtDocEncrypt::True ? "" : UT::testPasswordBis));
    // check tree
    _assert_tree_data(pWin2);

    // close this window/tree
    pWin2->force_exit() = true;
    remove_window(*pWin2);
}

void TestCtApp::_process_rich_text_buffer(std::list<ExpectedTag>& expectedTags, Glib::RefPtr<Gsv::Buffer> rTextBuffer)
{
    CtTextIterUtil::SerializeFunc test_slot = [&expectedTags](Gtk::TextIter& start_iter,
                                                              Gtk::TextIter& end_iter,
                                                              CtCurrAttributesMap& curr_attributes)
    {
        const Glib::ustring slot_text = start_iter.get_text(end_iter);
        for (auto& expTag : expectedTags) {
            if (slot_text.find(expTag.text_slot) != std::string::npos) {
                expTag.found = true;
                for (const auto& currPair : curr_attributes) {
                    if (expTag.attr_map.count(currPair.first) != 0) {
                        // we defined it
                        ASSERT_STREQ(expTag.attr_map[currPair.first].c_str(), currPair.second.c_str());
                    }
                    else {
                        // we haven't defined, expect empty!
                        ASSERT_STREQ("", currPair.second.c_str());
                    }
                }
                break;
            }
        }
    };
    CtTextIterUtil::generic_process_slot(0, -1, rTextBuffer, test_slot);
}

void TestCtApp::_assert_node_text(CtTreeIter& ctTreeIter, const Glib::ustring& expectedText)
{
    const Glib::RefPtr<Gsv::Buffer> rTextBuffer = ctTreeIter.get_node_text_buffer();
    ASSERT_TRUE(static_cast<bool>(rTextBuffer));
    ASSERT_STREQ(expectedText.c_str(), rTextBuffer->get_text().c_str());
}

#define _NL "\n"

void TestCtApp::_assert_tree_data(CtMainWin* pWin)
{
    CtSummaryInfo summaryInfo{};
    pWin->get_tree_store().populateSummaryInfo(summaryInfo);
    ASSERT_EQ(3, summaryInfo.nodes_rich_text_num);
    ASSERT_EQ(1, summaryInfo.nodes_plain_text_num);
    ASSERT_EQ(5, summaryInfo.nodes_code_num);
    ASSERT_EQ(1, summaryInfo.images_num);
    ASSERT_EQ(1, summaryInfo.embfile_num);
    ASSERT_EQ(1, summaryInfo.tables_num);
    ASSERT_EQ(1, summaryInfo.codeboxes_num);
    ASSERT_EQ(1, summaryInfo.anchors_num);
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("йцукенгшщз");
        ASSERT_TRUE(ctTreeIter);
        ASSERT_STREQ("0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("йцукенгшщз", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("plain-text", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_TRUE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "ciao plain" _NL
            "йцукенгшщз"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("b");
        ASSERT_TRUE(ctTreeIter);
        // assert node properties
        ASSERT_STREQ("1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("custom-colors", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_TRUE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        // assert text
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
        // assert rich text tags
        std::list<ExpectedTag> expectedTags = {
            ExpectedTag{
                .text_slot="ciao rich",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_FILL}}},
            ExpectedTag{
                .text_slot="fore",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_FOREGROUND, "#ffff00000000"}}},
            ExpectedTag{
                .text_slot="back",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_BACKGROUND, "#e6e6e6e6fafa"}}},
            ExpectedTag{
                .text_slot="bold",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY},
                                              {CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_CENTER}}},
            ExpectedTag{
                .text_slot="italic",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC}}},
            ExpectedTag{
                .text_slot="under",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE},
                                              {CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_RIGHT}}},
            ExpectedTag{
                .text_slot="strike",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE}}},
            ExpectedTag{
                .text_slot="h1",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H1}}},
            ExpectedTag{
                .text_slot="h2",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H2}}},
            ExpectedTag{
                .text_slot="h3",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3}}},
            ExpectedTag{
                .text_slot="small",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SMALL}}},
            ExpectedTag{
                .text_slot="super",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP},
                                              {CtConst::TAG_INDENT, "1"}}},
            ExpectedTag{
                .text_slot="sub",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB},
                                              {CtConst::TAG_INDENT, "2"}}},
            ExpectedTag{
                .text_slot="mono",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_FAMILY, CtConst::TAG_PROP_VAL_MONOSPACE}}},
        };
        _process_rich_text_buffer(expectedTags, ctTreeIter.get_node_text_buffer());
        for (auto& expTag : expectedTags) {
            ASSERT_TRUE(expTag.found);
        }
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("c");
        ASSERT_TRUE(ctTreeIter);
        ASSERT_STREQ("1:0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("c", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
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
        ASSERT_TRUE(ctTreeIter);
        ASSERT_STREQ("1:1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("sh", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "echo \"ciao!\""
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("html");
        ASSERT_TRUE(ctTreeIter);
        ASSERT_STREQ("1:1:0", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("html", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "<head>" _NL
            "<title>NO</title>" _NL
            "</head>"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("xml");
        ASSERT_TRUE(ctTreeIter);
        ASSERT_STREQ("1:1:1", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("xml", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("py");
        ASSERT_TRUE(ctTreeIter);
        ASSERT_STREQ("1:2", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("python3", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "print(\"ciao!\")"
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    gint64 node_d_id{0};
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("d");
        ASSERT_TRUE(ctTreeIter);
        node_d_id = ctTreeIter.get_node_id();
        ASSERT_TRUE(node_d_id > 0);
        ASSERT_STREQ("2", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_TRUE(ctTreeIter.get_node_is_bold());
        ASSERT_TRUE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(45, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("ciao", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("#ff0000", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("custom-colors", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        const Glib::ustring expectedText{
            "second rich" _NL
        };
        _assert_node_text(ctTreeIter, expectedText);
    }
    {
        CtTreeIter ctTreeIter = pWin->get_tree_store().get_node_from_node_name("e");
        ASSERT_TRUE(ctTreeIter);
        // assert node properties
        const gint64 node_e_id = ctTreeIter.get_node_id();
        ASSERT_TRUE(node_e_id > 0);
        ASSERT_STREQ("3", pWin->get_tree_store().get_path(ctTreeIter).to_string().c_str());
        ASSERT_FALSE(ctTreeIter.get_node_is_bold());
        ASSERT_FALSE(ctTreeIter.get_node_read_only());
        ASSERT_EQ(0, ctTreeIter.get_node_custom_icon_id());
        ASSERT_STREQ("", ctTreeIter.get_node_tags().c_str());
        ASSERT_STREQ("", ctTreeIter.get_node_foreground().c_str());
        ASSERT_STREQ("custom-colors", ctTreeIter.get_node_syntax_highlighting().c_str());
        ASSERT_FALSE(pWin->get_tree_store().is_node_bookmarked(ctTreeIter.get_node_id()));
        // assert text
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
            "link to web ansa.it" _NL
            "link to node ‘d’" _NL
            "link to node ‘e’ + anchor" _NL
            "link to folder /etc" _NL
            "link to file /etc/fstab" _NL
        };
        _assert_node_text(ctTreeIter, expectedText);
        // assert rich text tags
        std::list<ExpectedTag> expectedTags = {
            ExpectedTag{
                .text_slot="link to web ansa.it",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_LINK, "webs http://www.ansa.it"}}},
            ExpectedTag{
                .text_slot="link to node ‘d’",
                .attr_map=CtCurrAttributesMap{{
                    CtConst::TAG_LINK,
                    std::string{"node "} + std::to_string(node_d_id)
                }}},
            ExpectedTag{
                .text_slot="link to node ‘e’ + anchor",
                .attr_map=CtCurrAttributesMap{{
                    CtConst::TAG_LINK,
                    std::string{"node "} + std::to_string(node_e_id) + " йцукенгшщз"
                }}},
            ExpectedTag{
                .text_slot="link to folder /etc",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_LINK, "fold L2V0Yw=="}}},
            ExpectedTag{
                .text_slot="link to file /etc/fstab",
                .attr_map=CtCurrAttributesMap{{CtConst::TAG_LINK, "file L2V0Yy9mc3RhYg=="}}},
        };
        _process_rich_text_buffer(expectedTags, ctTreeIter.get_node_text_buffer());
        for (auto& expTag : expectedTags) {
            ASSERT_TRUE(expTag.found);
        }
        // assert anchored widgets
        std::list<CtAnchoredWidget*> anchoredWidgets = ctTreeIter.get_anchored_widgets();
        ASSERT_EQ(5, anchoredWidgets.size());
        for (CtAnchoredWidget* pAnchWidget : anchoredWidgets) {
            switch (pAnchWidget->get_type()) {
                case CtAnchWidgType::CodeBox: {
                    ASSERT_EQ(28, pAnchWidget->getOffset());
                    ASSERT_STREQ(CtConst::TAG_PROP_VAL_LEFT, pAnchWidget->getJustification().c_str());
                    auto pCodebox = dynamic_cast<CtCodebox*>(pAnchWidget);
                    ASSERT_TRUE(pCodebox);
                    ASSERT_STREQ(
                        "def test_function:" _NL
                        "    print \"hi there йцукенгшщз\"",
                        pCodebox->get_text_content().c_str());
                    ASSERT_STREQ("python", pCodebox->get_syntax_highlighting().c_str());
                    ASSERT_TRUE(pCodebox->get_width_in_pixels());
                    ASSERT_EQ(280,  pCodebox->get_frame_width());
                    ASSERT_EQ(50,  pCodebox->get_frame_height());
                    ASSERT_TRUE(pCodebox->get_highlight_brackets());
                    ASSERT_FALSE(pCodebox->get_show_line_numbers());
                } break;
                case CtAnchWidgType::Table: {
                    ASSERT_EQ(49, pAnchWidget->getOffset());
                    ASSERT_STREQ(CtConst::TAG_PROP_VAL_LEFT, pAnchWidget->getJustification().c_str());
                    auto pTable = dynamic_cast<CtTable*>(pAnchWidget);
                    ASSERT_TRUE(pTable);
                    ASSERT_EQ(60, pTable->get_col_width_default());
                    const CtTableColWidths expected_column_widths{105, 75};
                    const CtTableColWidths actual_column_widths = pTable->get_col_widths();
                    ASSERT_EQ(expected_column_widths.size(), actual_column_widths.size());
                    for (size_t i = 0; i < expected_column_widths.size(); ++i) {
                        ASSERT_EQ(expected_column_widths.at(i), actual_column_widths.at(i));
                    }
                    const CtTableMatrix& tableMatrix = pTable->get_table_matrix();
                    // three rows
                    ASSERT_EQ(3, tableMatrix.size());
                    // two columns
                    ASSERT_EQ(2, tableMatrix.at(0).size());
                    {
                        CtTableCell* pCell = tableMatrix.at(0).at(0);
                        ASSERT_STREQ("h1", pCell->get_text_content().c_str());
                        ASSERT_STREQ(CtConst::TABLE_CELL_TEXT_ID, pCell->get_syntax_highlighting().c_str());
                    }
                    {
                        CtTableCell* pCell = tableMatrix.at(0).at(1);
                        ASSERT_STREQ("h2", pCell->get_text_content().c_str());
                        ASSERT_STREQ(CtConst::TABLE_CELL_TEXT_ID, pCell->get_syntax_highlighting().c_str());
                    }
                    {
                        CtTableCell* pCell = tableMatrix.at(1).at(0);
                        ASSERT_STREQ("йцукенгшщз", pCell->get_text_content().c_str());
                        ASSERT_STREQ(CtConst::TABLE_CELL_TEXT_ID, pCell->get_syntax_highlighting().c_str());
                    }
                    {
                        CtTableCell* pCell = tableMatrix.at(1).at(1);
                        ASSERT_STREQ("2", pCell->get_text_content().c_str());
                        ASSERT_STREQ(CtConst::TABLE_CELL_TEXT_ID, pCell->get_syntax_highlighting().c_str());
                    }
                    {
                        CtTableCell* pCell = tableMatrix.at(2).at(0);
                        ASSERT_STREQ("3", pCell->get_text_content().c_str());
                        ASSERT_STREQ(CtConst::TABLE_CELL_TEXT_ID, pCell->get_syntax_highlighting().c_str());
                    }
                    {
                        CtTableCell* pCell = tableMatrix.at(2).at(1);
                        ASSERT_STREQ("4", pCell->get_text_content().c_str());
                        ASSERT_STREQ(CtConst::TABLE_CELL_TEXT_ID, pCell->get_syntax_highlighting().c_str());
                    }
                } break;
                case CtAnchWidgType::ImagePng: {
                    ASSERT_EQ(59, pAnchWidget->getOffset());
                    ASSERT_STREQ(CtConst::TAG_PROP_VAL_LEFT, pAnchWidget->getJustification().c_str());
                    auto pImagePng = dynamic_cast<CtImagePng*>(pAnchWidget);
                    ASSERT_TRUE(pImagePng);
                    ASSERT_STREQ("webs http://www.ansa.it", pImagePng->get_link().c_str());
                    static const std::string embedded_png = Glib::Base64::decode("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");
                    ASSERT_EQ(embedded_png.size(), pImagePng->get_raw_blob().size());
                    ASSERT_EQ(embedded_png, pImagePng->get_raw_blob());
                } break;
                case CtAnchWidgType::ImageAnchor: {
                    ASSERT_EQ(39, pAnchWidget->getOffset());
                    ASSERT_STREQ(CtConst::TAG_PROP_VAL_LEFT, pAnchWidget->getJustification().c_str());
                    auto pImageAnchor = dynamic_cast<CtImageAnchor*>(pAnchWidget);
                    ASSERT_TRUE(pImageAnchor);
                    ASSERT_STREQ("йцукенгшщз", pImageAnchor->get_anchor_name().c_str());
                } break;
                case CtAnchWidgType::ImageEmbFile: {
                    ASSERT_EQ(77, pAnchWidget->getOffset());
                    ASSERT_STREQ(CtConst::TAG_PROP_VAL_LEFT, pAnchWidget->getJustification().c_str());
                    auto pImageEmbFile = dynamic_cast<CtImageEmbFile*>(pAnchWidget);
                    ASSERT_TRUE(pImageEmbFile);
                    ASSERT_STREQ("йцукенгшщз.txt", pImageEmbFile->get_file_name().c_str());
                    static const std::string embedded_file = Glib::Base64::decode("0LnRhtGD0LrQtdC90LPRiNGJ0LcK");
                    ASSERT_EQ(embedded_file.size(), pImageEmbFile->get_raw_blob().size());
                    ASSERT_EQ(embedded_file, pImageEmbFile->get_raw_blob());
                    ASSERT_EQ(1565442560, pImageEmbFile->get_time());
                } break;
            }
        }
    }
}

#if !defined(__APPLE__) // TestCtApp causes crash on macos

class ReadWriteMultipleParametersTests : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
};

TEST_P(ReadWriteMultipleParametersTests, ChecksReadWrite)
{
    const std::string in_doc_path = std::get<0>(GetParam());
    const std::string out_doc_path = std::get<1>(GetParam());
    const std::vector<std::string> vec_args{"cherrytree", in_doc_path, "-t", out_doc_path};
    gchar** pp_args = CtStrUtil::vector_to_array(vec_args);
    TestCtApp testCtApp{vec_args};
    testCtApp.run(vec_args.size(), pp_args);
    g_strfreev(pp_args);
}

INSTANTIATE_TEST_CASE_P(
        ReadWriteTests,
        ReadWriteMultipleParametersTests,
        ::testing::Values(
                std::make_tuple(UT::testCtbDocPath, UT::testCtbDocPath),
                std::make_tuple(UT::testCtbDocPath, UT::testCtdDocPath),
                std::make_tuple(UT::testCtbDocPath, UT::testCtxDocPath),
                std::make_tuple(UT::testCtbDocPath, UT::testCtzDocPath),
                std::make_tuple(UT::testCtdDocPath, UT::testCtbDocPath),
                std::make_tuple(UT::testCtdDocPath, UT::testCtdDocPath),
                std::make_tuple(UT::testCtdDocPath, UT::testCtxDocPath),
                std::make_tuple(UT::testCtdDocPath, UT::testCtzDocPath),
                std::make_tuple(UT::testCtxDocPath, UT::testCtbDocPath),
                std::make_tuple(UT::testCtxDocPath, UT::testCtdDocPath),
                std::make_tuple(UT::testCtxDocPath, UT::testCtxDocPath),
                std::make_tuple(UT::testCtxDocPath, UT::testCtzDocPath),
                std::make_tuple(UT::testCtzDocPath, UT::testCtbDocPath),
                std::make_tuple(UT::testCtzDocPath, UT::testCtdDocPath),
                std::make_tuple(UT::testCtzDocPath, UT::testCtxDocPath),
                std::make_tuple(UT::testCtzDocPath, UT::testCtzDocPath))
);

#endif // __APPLE__
