/*
 * tests_lists.cpp
 *
 * Copyright 2009-2023
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

#include "ct_misc_utils.h"
#include "ct_list.h"
#include "ct_config.h"
#include "tests_common.h"
#include "ct_export2html.h"

static const CtConfig ct_config;

const Glib::ustring bufferContent_1{
    "- primo elemento" _NL              // 0
    "- secondo elemento" _NL            // 17
    "   su più righe" _NL               // 36
    "   → terzo elemento indentato" _NL // 52
    "   → quarto su" _NL                // 82
    "      più 1" _NL                   // 97
    "      più 2" _NL                   // 109
    "      più 3" _NL                   // 121
    "      ⇒ ancora un livello" _NL    // 133
    "         su più righe" _NL         // 159
    "      ⇒ stesso livello" _NL        // 181
    "         • ancora uno avanti" _NL  // 204
    _NL                                 // 233
    "1. primo elemento" _NL             // 234
    "2. secondo elemento" _NL           // 252
    "   su più righe" _NL               // 272
    "   1) terzo indentato" _NL         // 288
    "   2) quarto su" _NL               // 310
    "      più 1" _NL                   // 326
    "      più 2" _NL                   // 338
    "      1- ancora un livello" _NL    // 350
    "         1> ancora uno avanti"};   // 377

const Glib::ustring bufferContent_2{
    "ciao" _NL                          // 0
    _NL                                 // 5
    "- primo elemento con tag" _NL      // 6
    "- secondo elemento" _NL};          // 31

TEST(ListsGroup, CtListInfo_2)
{
    auto rTextTagTable = Gtk::TextTagTable::create();
    auto pBuffer = Gsv::Buffer::create(rTextTagTable);
    pBuffer->set_text(bufferContent_2);
    const std::string tagName{CtConst::TAG_WEIGHT + CtConst::CHAR_USCORE + CtConst::TAG_PROP_VAL_HEAVY};
    auto rTextTag = Gtk::TextTag::create(tagName);
    rTextTag->property_weight() = Pango::Weight::WEIGHT_HEAVY;
    rTextTagTable->add(rTextTag);
    pBuffer->apply_tag_by_name(tagName,
                               pBuffer->get_iter_at_offset(23),
                               pBuffer->get_iter_at_offset(26));

    CtList ct_list{&ct_config, pBuffer};

    CtListInfo curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(0));
    ASSERT_EQ(CtListType::None, curr_list_info.type);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(6));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(6, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(31));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(31, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    Glib::ustring out_html = CtExport2Html::html_process_slot(&ct_config,
                                                              nullptr/*pCtMainWin*/,
                                                              0, bufferContent_2.size()-1,
                                                              pBuffer);
    ASSERT_STREQ("ciao\n<ul><li>primo elemento <strong>con</strong> tag</li><li>secondo elemento</li></ul>", out_html.c_str());
}

TEST(ListsGroup, CtListInfo_1)
{
    auto pBuffer = Gsv::Buffer::create();
    pBuffer->set_text(bufferContent_1);
    CtList ct_list{&ct_config, pBuffer};

    CtListInfo curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(0));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(0, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(17));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(17, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(36));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(17, curr_list_info.startoffs);
    ASSERT_EQ(1, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(52));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(52, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(82));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(82, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(97));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(82, curr_list_info.startoffs);
    ASSERT_EQ(1, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(109));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(82, curr_list_info.startoffs);
    ASSERT_EQ(2, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(121));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(82, curr_list_info.startoffs);
    ASSERT_EQ(3, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(133));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(2, curr_list_info.level);
    ASSERT_EQ(133, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(159));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(2, curr_list_info.level);
    ASSERT_EQ(133, curr_list_info.startoffs);
    ASSERT_EQ(1, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(181));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(2, curr_list_info.level);
    ASSERT_EQ(181, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(204));
    ASSERT_EQ(CtListType::Bullet, curr_list_info.type);
    ASSERT_EQ(3, curr_list_info.level);
    ASSERT_EQ(204, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(233));
    ASSERT_EQ(CtListType::None, curr_list_info.type);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(234));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(1, curr_list_info.num_seq);
    ASSERT_EQ(234, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(252));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(2, curr_list_info.num_seq);
    ASSERT_EQ(252, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(272));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(0, curr_list_info.level);
    ASSERT_EQ(2, curr_list_info.num_seq);
    ASSERT_EQ(252, curr_list_info.startoffs);
    ASSERT_EQ(1, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(288));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(1, curr_list_info.num_seq);
    ASSERT_EQ(288, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(310));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(2, curr_list_info.num_seq);
    ASSERT_EQ(310, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(326));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(2, curr_list_info.num_seq);
    ASSERT_EQ(310, curr_list_info.startoffs);
    ASSERT_EQ(1, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(338));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(1, curr_list_info.level);
    ASSERT_EQ(2, curr_list_info.num_seq);
    ASSERT_EQ(310, curr_list_info.startoffs);
    ASSERT_EQ(2, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(350));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(2, curr_list_info.level);
    ASSERT_EQ(1, curr_list_info.num_seq);
    ASSERT_EQ(350, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    curr_list_info = ct_list.get_paragraph_list_info(pBuffer->get_iter_at_offset(377));
    ASSERT_EQ(CtListType::Number, curr_list_info.type);
    ASSERT_EQ(3, curr_list_info.level);
    ASSERT_EQ(1, curr_list_info.num_seq);
    ASSERT_EQ(377, curr_list_info.startoffs);
    ASSERT_EQ(0, curr_list_info.count_nl);

    Glib::ustring out_html = CtExport2Html::html_process_slot(&ct_config,
                                                              nullptr/*pCtMainWin*/,
                                                              0, bufferContent_1.size()-1,
                                                              pBuffer);
    ASSERT_STREQ("<ul><li>primo elemento</li><li>secondo elemento su pi\xC3\xB9 righe<ul><li>terzo elemento indentato</li><li>quarto su pi\xC3\xB9 1 pi\xC3\xB9 2 pi\xC3\xB9 3<ul><li>ancora un livello su pi\xC3\xB9 righe</li><li>stesso livello<ul><li>ancora uno avanti</li></ul></li></ul></li></ul></li></ul>\n<ol><li>primo elemento</li><li>secondo elemento su pi\xC3\xB9 righe<ol><li>terzo indentato</li><li>quarto su pi\xC3\xB9 1 pi\xC3\xB9 2<ol><li>ancora un livello<ol><li>ancora uno avant</li></ol></li></ol></li></ol></li></ol>", out_html.c_str());
}
