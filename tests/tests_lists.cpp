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

static const CtConfig ct_config;

const Glib::ustring bufferContent{
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

TEST(ListsGroup, CtListInfo)
{
    Glib::init();
    auto pBuffer = Gsv::Buffer::create();
    pBuffer->set_text(bufferContent);
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
}
