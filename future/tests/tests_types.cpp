/*
 * tests_types.cpp
 *
 * Copyright 2019-2020 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_types.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "ct_filesystem.h"

TEST_GROUP(TestTypesGroup)
{
};

TEST(TestTypesGroup, ctMaxSizedList)
{
    CtMaxSizedList<int> maxSizedList{3};
    maxSizedList.push_back(1);
    maxSizedList.push_back(2);
    maxSizedList.push_back(3);
    CHECK_EQUAL(3, maxSizedList.size());
    CHECK_EQUAL(1, maxSizedList.front());
    CHECK_EQUAL(3, maxSizedList.back());
    maxSizedList.move_or_push_back(1);
    CHECK_EQUAL(3, maxSizedList.size());
    CHECK_EQUAL(2, maxSizedList.front());
    CHECK_EQUAL(1, maxSizedList.back());
    maxSizedList.move_or_push_front(3);
    CHECK_EQUAL(3, maxSizedList.size());
    CHECK_EQUAL(3, maxSizedList.front());
    CHECK_EQUAL(1, maxSizedList.back());
    maxSizedList.move_or_push_front(4);
    CHECK_EQUAL(3, maxSizedList.size());
    CHECK_EQUAL(4, maxSizedList.front());
    CHECK_EQUAL(2, maxSizedList.back());
}
