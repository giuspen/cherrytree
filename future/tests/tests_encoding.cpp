/*
 * tests_encoding.cpp
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

#include "ct_logging.h"
#include "test_consts.h"
#include "CppUTest/CommandLineTestRunner.h"


TEST_GROUP(EncodingGroup)
{
};

TEST(EncodingGroup, ustring_format)
{
    // on win32 this could throw an exception due to locale
    Glib::ustring str = "привет こんにちは";
    STRCMP_EQUAL(str.c_str(), fmt::format("{}", str).c_str());
}
