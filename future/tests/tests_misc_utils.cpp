/*
 * tests_misc_utils.cpp
 * 
 * Copyright 2018 Giuseppe Penone <giuspen@gmail.com>
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

TEST(MiscUtilsGroup, isStrTrue)
{
    CHECK(CtStrUtil::isStrTrue("true"));
    CHECK(CtStrUtil::isStrTrue("True"));
    CHECK(CtStrUtil::isStrTrue("TRUE"));
    CHECK(CtStrUtil::isStrTrue("1"));
    CHECK(!CtStrUtil::isStrTrue("false"));
    CHECK(!CtStrUtil::isStrTrue("False"));
    CHECK(!CtStrUtil::isStrTrue("FALSE"));
    CHECK(!CtStrUtil::isStrTrue("0"));
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

TEST(MiscUtilsGroup, gint64FromGstring)
{
    CHECK_EQUAL(0, CtStrUtil::gint64FromGstring(""));
    CHECK_EQUAL(0x7fffffffffffffff, CtStrUtil::gint64FromGstring("0x7fffffffffffffff"));
    CHECK_EQUAL(-0x8000000000000000, CtStrUtil::gint64FromGstring("-0x8000000000000000"));
    CHECK_EQUAL(1234, CtStrUtil::gint64FromGstring("1234"));
    CHECK_EQUAL(-1234, CtStrUtil::gint64FromGstring("-1234"));
    CHECK_EQUAL(0x123f, CtStrUtil::gint64FromGstring("123f", true));
}

TEST(MiscUtilsGroup, getUint32FromHexChars)
{
    CHECK_EQUAL(0xaf, CtStrUtil::getUint32FromHexChars("aff", 2));
    CHECK_EQUAL(0xa, CtStrUtil::getUint32FromHexChars("aff", 1));
}

TEST(MiscUtilsGroup, str__split)
{
    {
        std::vector<std::string> splittedVec = str::split(":a:bc::d:", ":");
        CHECK(std::vector<std::string>({"", "a", "bc", "", "d", ""}) == splittedVec);
    }
    {
        std::vector<Glib::ustring> splittedVec = str::split<Glib::ustring>(" a bc  d ", " ");
        CHECK(std::vector<Glib::ustring>({"", "a", "bc", "", "d", ""}) == splittedVec);
    }
}

TEST(MiscUtilsGroup, gstringSplit2int64)
{
    std::vector<gint64> splittedVec = CtStrUtil::gstringSplit2int64("-1, 1,0, 1000", ",");
    CHECK(std::vector<gint64>({-1, 1, 0, 1000}) == splittedVec);
}

TEST(MiscUtilsGroup, isPgcharInPgcharSet)
{
    CHECK(CtStrUtil::isPgcharInPgcharSet(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROPERTIES));
    CHECK(!CtStrUtil::isPgcharInPgcharSet("something surely missing", CtConst::TAG_PROPERTIES));
}

TEST(MiscUtilsGroup, getFontMisc)
{
    CHECK("Sans" == CtFontUtil::getFontFamily("Sans 9"));
    CHECK("9" == CtFontUtil::getFontSizeStr("Sans 9"));
}


TEST(MiscUtilsGroup, setRgb24StrFromRgb24Int)
{
    guint32 int24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    char rgb24Str[8];
    CtRgbUtil::setRgb24StrFromRgb24Int(int24, rgb24Str);
    STRCMP_EQUAL("#f1ab57", rgb24Str);
}

TEST(MiscUtilsGroup, getRgb24IntFromRgb24Str)
{
    guint32 expectedInt24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    CHECK_EQUAL(expectedInt24, CtRgbUtil::getRgb24IntFromRgb24Str("#f1ab57"));
    CHECK_EQUAL(expectedInt24, CtRgbUtil::getRgb24IntFromRgb24Str("f1ab57"));
}

TEST(MiscUtilsGroup, setRgb24StrFromStrAny)
{
    char rgb24Str[8];
    STRCMP_EQUAL("#f1ab57", CtRgbUtil::setRgb24StrFromStrAny("#f1ab57", rgb24Str));
    STRCMP_EQUAL("#ffaabb", CtRgbUtil::setRgb24StrFromStrAny("#fab", rgb24Str));
    STRCMP_EQUAL("#123456", CtRgbUtil::setRgb24StrFromStrAny("#122334455667", rgb24Str));
    STRCMP_EQUAL("#123456", CtRgbUtil::setRgb24StrFromStrAny("122334455667", rgb24Str));
}

TEST(MiscUtilsGroup, getRgb24IntFromStrAny)
{
    guint32 expectedInt24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    CHECK_EQUAL(expectedInt24, CtRgbUtil::getRgb24IntFromStrAny("#f1ab57"));
    CHECK_EQUAL(expectedInt24, CtRgbUtil::getRgb24IntFromStrAny("f1ab57"));
    CHECK_EQUAL(expectedInt24, CtRgbUtil::getRgb24IntFromStrAny("f122ab335744"));
}

TEST(MiscUtilsGroup, str__endswith)
{
    CHECK(str::endswith("", ""));
    CHECK(str::endswith("123", ""));
    CHECK(str::endswith("123", "23"));
    CHECK(!str::endswith("123", "1"));
    CHECK(!str::endswith("", "1"));
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
