/*
 * tests_misc_utils.cpp
 *
 * Copyright 2018-2020 Giuseppe Penone <giuspen@gmail.com>
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
#include "ct_const.h"
#include "CppUTest/CommandLineTestRunner.h"


TEST_GROUP(MiscUtilsGroup)
{
};

TEST(MiscUtilsGroup, is_str_true)
{
    CHECK(CtStrUtil::is_str_true("true"));
    CHECK(CtStrUtil::is_str_true("True"));
    CHECK(CtStrUtil::is_str_true("TRUE"));
    CHECK(CtStrUtil::is_str_true("1"));
    CHECK(not CtStrUtil::is_str_true("false"));
    CHECK(not CtStrUtil::is_str_true("False"));
    CHECK(not CtStrUtil::is_str_true("FALSE"));
    CHECK(not CtStrUtil::is_str_true("0"));
}

TEST(MiscUtilsGroup, str__replace)
{
    {
        Glib::ustring testReplaceStr = "one two threetwo";
        STRCMP_EQUAL("one four threefour", str::replace(testReplaceStr, "two", "four").c_str());
    }
    {
        std::string testReplaceStr = "one two threetwo";
        STRCMP_EQUAL("one four threefour", str::replace(testReplaceStr, "two", "four").c_str());
    }
}

TEST(MiscUtilsGroup, str__trim)
{
    {
        Glib::ustring testTrimStr = "\t one two three ";
        STRCMP_EQUAL("one two three", str::trim(testTrimStr).c_str());
    }
    {
        std::string testTrimStr = "\t one two three ";
        STRCMP_EQUAL("one two three", str::trim(testTrimStr).c_str());
    }
}

TEST(MiscUtilsGroup, gint64_from_gstring)
{
    CHECK_EQUAL(0, CtStrUtil::gint64_from_gstring(""));
    CHECK_EQUAL(0x7fffffffffffffff, CtStrUtil::gint64_from_gstring("0x7fffffffffffffff"));
    CHECK_EQUAL(-0x8000000000000000, CtStrUtil::gint64_from_gstring("-0x8000000000000000"));
    CHECK_EQUAL(1234, CtStrUtil::gint64_from_gstring("1234"));
    CHECK_EQUAL(-1234, CtStrUtil::gint64_from_gstring("-1234"));
    CHECK_EQUAL(0x123f, CtStrUtil::gint64_from_gstring("123f", true));
}

TEST(MiscUtilsGroup, guint32_from_hex_chars)
{
    CHECK_EQUAL(0xaf, CtStrUtil::guint32_from_hex_chars("aff", 2));
    CHECK_EQUAL(0xa, CtStrUtil::guint32_from_hex_chars("aff", 1));
}

TEST(MiscUtilsGroup, str__split)
{
    {
        std::vector<std::string> splittedVec = str::split<std::string>(":a:bc::d:", ":");
        CHECK(std::vector<std::string>({"", "a", "bc", "", "d", ""}) == splittedVec);
    }
    {
        std::vector<Glib::ustring> splittedVec = str::split<Glib::ustring>(" a bc  d ", " ");
        CHECK(std::vector<Glib::ustring>({"", "a", "bc", "", "d", ""}) == splittedVec);
    }
}

TEST(MiscUtilsGroup, gstring_split_to_int64)
{
    std::vector<gint64> splittedVec = CtStrUtil::gstring_split_to_int64("-1, 1,0, 1000", ",");
    CHECK(std::vector<gint64>({-1, 1, 0, 1000}) == splittedVec);
}

TEST(MiscUtilsGroup, is_pgchar_in_pgchar_iterable)
{
    CHECK(CtStrUtil::is_pgchar_in_pgchar_iterable(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROPERTIES));
    CHECK(not CtStrUtil::is_pgchar_in_pgchar_iterable("something surely missing", CtConst::TAG_PROPERTIES));
}

TEST(MiscUtilsGroup, getFontMisc)
{
    STRCMP_EQUAL("Sans", CtFontUtil::get_font_family("Sans 9").c_str());
    STRCMP_EQUAL("9", CtFontUtil::get_font_size_str("Sans 9").c_str());
    STRCMP_EQUAL("Noto Sans", CtFontUtil::get_font_family("Noto Sans 9").c_str());
    STRCMP_EQUAL("9", CtFontUtil::get_font_size_str("Noto Sans 9").c_str());
}


TEST(MiscUtilsGroup, set_rgb24str_from_rgb24int)
{
    guint32 int24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    char rgb24Str[8];
    CtRgbUtil::set_rgb24str_from_rgb24int(int24, rgb24Str);
    STRCMP_EQUAL("#f1ab57", rgb24Str);
}

TEST(MiscUtilsGroup, get_rgb24int_from_rgb24str)
{
    guint32 expectedInt24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    CHECK_EQUAL(expectedInt24, CtRgbUtil::get_rgb24int_from_rgb24str("#f1ab57"));
    CHECK_EQUAL(expectedInt24, CtRgbUtil::get_rgb24int_from_rgb24str("f1ab57"));
}

TEST(MiscUtilsGroup, set_rgb24str_from_str_any)
{
    char rgb24Str[8];
    STRCMP_EQUAL("#f1ab57", CtRgbUtil::set_rgb24str_from_str_any("#f1ab57", rgb24Str));
    STRCMP_EQUAL("#ffaabb", CtRgbUtil::set_rgb24str_from_str_any("#fab", rgb24Str));
    STRCMP_EQUAL("#123456", CtRgbUtil::set_rgb24str_from_str_any("#122334455667", rgb24Str));
    STRCMP_EQUAL("#123456", CtRgbUtil::set_rgb24str_from_str_any("122334455667", rgb24Str));
}

TEST(MiscUtilsGroup, get_rgb24int_from_str_any)
{
    guint32 expectedInt24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    CHECK_EQUAL(expectedInt24, CtRgbUtil::get_rgb24int_from_str_any("#f1ab57"));
    CHECK_EQUAL(expectedInt24, CtRgbUtil::get_rgb24int_from_str_any("f1ab57"));
    CHECK_EQUAL(expectedInt24, CtRgbUtil::get_rgb24int_from_str_any("f122ab335744"));
}

TEST(MiscUtilsGroup, natural_compare)
{
    CHECK(CtStrUtil::natural_compare("","") == 0);
    CHECK(CtStrUtil::natural_compare("","a") < 0);
    CHECK(CtStrUtil::natural_compare("a","") > 0);
    CHECK(CtStrUtil::natural_compare("a","a") == 0);
    CHECK(CtStrUtil::natural_compare("","9") < 0);
    CHECK(CtStrUtil::natural_compare("9","") > 0);
    CHECK(CtStrUtil::natural_compare("1","1") == 0);
    CHECK(CtStrUtil::natural_compare("1","2") < 0);
    CHECK(CtStrUtil::natural_compare("3","2") > 0);
    CHECK(CtStrUtil::natural_compare("a1","a1") == 0);
    CHECK(CtStrUtil::natural_compare("a1","a2") < 0);
    CHECK(CtStrUtil::natural_compare("a2","a1") > 0);
    CHECK(CtStrUtil::natural_compare("a1a2","a1a3") < 0);
    CHECK(CtStrUtil::natural_compare("a1a2","a1a0") > 0);
    CHECK(CtStrUtil::natural_compare("134","122") > 0);
    CHECK(CtStrUtil::natural_compare("12a3","12a3") == 0);
    CHECK(CtStrUtil::natural_compare("12a1","12a0") > 0);
    CHECK(CtStrUtil::natural_compare("12a1","12a2") < 0);
    CHECK(CtStrUtil::natural_compare("a","aa") < 0);
    CHECK(CtStrUtil::natural_compare("aaa","aa") > 0);
    CHECK(CtStrUtil::natural_compare("Alpha 2","Alpha 2") == 0);
    CHECK(CtStrUtil::natural_compare("Alpha 2","Alpha 2A") < 0);
    CHECK(CtStrUtil::natural_compare("Alpha 2 B","Alpha 2") > 0);
}

TEST(MiscUtilsGroup, str__endswith)
{
    CHECK(str::endswith("", ""));
    CHECK(str::endswith("123", ""));
    CHECK(str::endswith("123", "23"));
    CHECK(not str::endswith("123", "1"));
    CHECK(not str::endswith("", "1"));
}

TEST(MiscUtilsGroup, str__join)
{
    std::vector<std::string> empty_v;
    std::vector<std::string> v_1{"1"};
    std::vector<std::string> v_2{"1", "2"};

    CHECK(str::join(empty_v, ",") == "");
    CHECK(str::join(v_1, ",") == "1");
    CHECK(str::join(v_2, ",") == "1,2");

    {
        std::vector<Glib::ustring> vecToJoin({"", "a", "bc", "", "d", ""});
        Glib::ustring rejoined = str::join(vecToJoin, ":");
        STRCMP_EQUAL(":a:bc::d:", rejoined.c_str());
    }
    {
        std::vector<std::string> vecToJoin({"", "a", "bc", "", "d", ""});
        std::string rejoined = str::join(vecToJoin, " ");
        STRCMP_EQUAL(" a bc  d ", rejoined.c_str());
    }
}

TEST(MiscUtilsGroup, str__join_numbers)
{
    std::vector<gint64> vecToJoin({-1, 1, 0, 1000});
    {
        Glib::ustring rejoined;
        str::join_numbers(vecToJoin, rejoined, ",");
        STRCMP_EQUAL("-1,1,0,1000", rejoined.c_str());
    }
    {
        std::string rejoined;
        str::join_numbers(vecToJoin, rejoined);
        STRCMP_EQUAL("-1 1 0 1000", rejoined.c_str());
    }
}

TEST(MiscUtilsGroup, str__swapcase)
{
    STRCMP_EQUAL("CheRrY", str::swapcase("cHErRy").c_str());
}

TEST(MiscUtilsGroup, str__repeat)
{
    STRCMP_EQUAL("", str::repeat("**", 0).c_str());
    STRCMP_EQUAL("**", str::repeat("**", 1).c_str());
    STRCMP_EQUAL("******", str::repeat("**", 3).c_str());
}

TEST(MiscUtilsGroup, vec_remove)
{
    std::vector<int> empty_v;
    std::vector<int> v_3{1, 2, 3};

    vec::remove(empty_v, 1);
    vec::remove(v_3, 0);
    CHECK(v_3.size() == 3);
    vec::remove(v_3, 2);
    CHECK(v_3.size() == 2);
}

int main(int ac, char** av)
{
    // libp7za has memory leaks
    MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();

    return CommandLineTestRunner::RunAllTests(ac, av);
}
