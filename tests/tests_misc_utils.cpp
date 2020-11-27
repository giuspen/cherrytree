/*
 * tests_misc_utils.cpp
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

#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_filesystem.h"
#include "tests_common.h"

TEST(MiscUtilsGroup, files_encodings)
{
    auto _get_file_encoding = [](const std::string& inFilepath)->std::string{
        std::string inString = Glib::file_get_contents(inFilepath);
        return CtStrUtil::get_encoding(inString.c_str(), inString.size());
    };
    ASSERT_STREQ("UTF-8", _get_file_encoding(UT::testCtdDocPath).c_str());
}

TEST(MiscUtilsGroup, is_str_true)
{
    ASSERT_TRUE(CtStrUtil::is_str_true("true"));
    ASSERT_TRUE(CtStrUtil::is_str_true("True"));
    ASSERT_TRUE(CtStrUtil::is_str_true("TRUE"));
    ASSERT_TRUE(CtStrUtil::is_str_true("1"));
    ASSERT_TRUE(not CtStrUtil::is_str_true("false"));
    ASSERT_TRUE(not CtStrUtil::is_str_true("False"));
    ASSERT_TRUE(not CtStrUtil::is_str_true("FALSE"));
    ASSERT_TRUE(not CtStrUtil::is_str_true("0"));
}

TEST(MiscUtilsGroup, str__replace)
{
    {
        Glib::ustring testReplaceStr = "one two threetwo";
        ASSERT_STREQ("one four threefour", str::replace(testReplaceStr, "two", "four").c_str());
    }
    {
        std::string testReplaceStr = "one two threetwo";
        ASSERT_STREQ("one four threefour", str::replace(testReplaceStr, "two", "four").c_str());
    }
}

TEST(MiscUtilsGroup, str__trim)
{
    {
        Glib::ustring testTrimStr = "\t one two three ";
        ASSERT_STREQ("one two three", str::trim(testTrimStr).c_str());
    }
    {
        std::string testTrimStr = "\t one two three ";
        ASSERT_STREQ("one two three", str::trim(testTrimStr).c_str());
    }
}

TEST(MiscUtilsGroup, gint64_from_gstring)
{
    ASSERT_EQ(0, CtStrUtil::gint64_from_gstring(""));
    ASSERT_EQ(0x7fffffffffffffff, CtStrUtil::gint64_from_gstring("0x7fffffffffffffff"));
    ASSERT_EQ(-0x8000000000000000, CtStrUtil::gint64_from_gstring("-0x8000000000000000"));
    ASSERT_EQ(1234, CtStrUtil::gint64_from_gstring("1234"));
    ASSERT_EQ(-1234, CtStrUtil::gint64_from_gstring("-1234"));
    ASSERT_EQ(0x123f, CtStrUtil::gint64_from_gstring("123f", true));
}

TEST(MiscUtilsGroup, guint32_from_hex_chars)
{
    ASSERT_EQ(0xaf, CtStrUtil::guint32_from_hex_chars("aff", 2));
    ASSERT_EQ(0xa, CtStrUtil::guint32_from_hex_chars("aff", 1));
}

TEST(MiscUtilsGroup, str__split)
{
    {
        std::vector<std::string> splittedVec = str::split<std::string>(":a:bc::d:", ":");
        ASSERT_TRUE(std::vector<std::string>({"", "a", "bc", "", "d", ""}) == splittedVec);
    }
    {
        std::vector<Glib::ustring> splittedVec = str::split<Glib::ustring>(" a bc  d ", " ");
        ASSERT_TRUE(std::vector<Glib::ustring>({"", "a", "bc", "", "d", ""}) == splittedVec);
    }
}

TEST(MiscUtilsGroup, gstring_split_to_int)
{
    std::vector<int> splittedVec = CtStrUtil::gstring_split_to_int("-1, 1,0, 1000", ",");
    ASSERT_TRUE(std::vector<int>({-1, 1, 0, 1000}) == splittedVec);
}

TEST(MiscUtilsGroup, gstring_split_to_int64)
{
    std::vector<gint64> splittedVec = CtStrUtil::gstring_split_to_int64("-1, 1,0, 1000", ",");
    ASSERT_TRUE(std::vector<gint64>({-1, 1, 0, 1000}) == splittedVec);
}

TEST(MiscUtilsGroup, iter_util__startswith)
{
    Glib::init();
    auto buffer = Gsv::Buffer::create();
    buffer->set_text("Saitama さいたま市");
    ASSERT_TRUE(CtTextIterUtil::startswith(buffer->begin(), "Sai"));
    ASSERT_TRUE(not CtTextIterUtil::startswith(buffer->begin(), "さ"));
    buffer->set_text("さいたま市 Saitama");
    ASSERT_TRUE(CtTextIterUtil::startswith(buffer->begin(), "さ"));
    ASSERT_TRUE(not CtTextIterUtil::startswith(buffer->begin(), "Sai"));
}

TEST(MiscUtilsGroup, iter_util__startswith_any)
{
    Glib::init();
    auto buffer = Gsv::Buffer::create();
    buffer->set_text("Saitama さいたま市");

    ASSERT_TRUE(CtTextIterUtil::startswith_any(buffer->begin(), std::array<const gchar*, 3>{"M", "Sai", "さ"}));
    ASSERT_TRUE(not CtTextIterUtil::startswith_any(buffer->begin(), std::array<const gchar*, 3>{"M", "21", "さ"}));
    buffer->set_text("さいたま市 Saitama");
    ASSERT_TRUE(CtTextIterUtil::startswith_any(buffer->begin(), std::array<const gchar*, 3>{"M", "Sai", "さ"}));
    ASSERT_TRUE(not CtTextIterUtil::startswith_any(buffer->begin(), std::array<const gchar*, 3>{"M", "Sai", "123"}));
}

TEST(MiscUtilsGroup, contains)
{
    ASSERT_TRUE(CtStrUtil::contains(CtConst::TAG_PROPERTIES, CtConst::TAG_STRIKETHROUGH));
    ASSERT_TRUE(not CtStrUtil::contains(CtConst::TAG_PROPERTIES, "something surely missing"));
}

TEST(MiscUtilsGroup, getFontMisc)
{
    ASSERT_STREQ("Sans", CtFontUtil::get_font_family("Sans 9").c_str());
    ASSERT_EQ(9, CtFontUtil::get_font_size("Sans 9"));
    ASSERT_STREQ("Noto Sans", CtFontUtil::get_font_family("Noto Sans 9").c_str());
    ASSERT_EQ(9, CtFontUtil::get_font_size("Noto Sans 9"));
    ASSERT_STREQ("Noto Sans 9", CtFontUtil::get_font_str("Noto Sans", 9).c_str());
    ASSERT_STREQ("Noto Sans 9", CtFontUtil::get_font_str(Pango::FontDescription("Noto Sans 9")).c_str());
}

TEST(MiscUtilsGroup, set_rgb24str_from_rgb24int)
{
    guint32 int24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    char rgb24Str[8];
    CtRgbUtil::set_rgb24str_from_rgb24int(int24, rgb24Str);
    ASSERT_STREQ("#f1ab57", rgb24Str);
}

TEST(MiscUtilsGroup, get_rgb24int_from_rgb24str)
{
    guint32 expectedInt24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    ASSERT_EQ(expectedInt24, CtRgbUtil::get_rgb24int_from_rgb24str("#f1ab57"));
    ASSERT_EQ(expectedInt24, CtRgbUtil::get_rgb24int_from_rgb24str("f1ab57"));
}

TEST(MiscUtilsGroup, set_rgb24str_from_str_any)
{
    char rgb24Str[8];
    ASSERT_STREQ("#f1ab57", CtRgbUtil::set_rgb24str_from_str_any("#f1ab57", rgb24Str));
    ASSERT_STREQ("#ffaabb", CtRgbUtil::set_rgb24str_from_str_any("#fab", rgb24Str));
    ASSERT_STREQ("#123456", CtRgbUtil::set_rgb24str_from_str_any("#122334455667", rgb24Str));
    ASSERT_STREQ("#123456", CtRgbUtil::set_rgb24str_from_str_any("122334455667", rgb24Str));
}

TEST(MiscUtilsGroup, get_rgb24int_from_str_any)
{
    guint32 expectedInt24 = (0xf1 << 16) | (0xab << 8) | 0x57;
    ASSERT_EQ(expectedInt24, CtRgbUtil::get_rgb24int_from_str_any("#f1ab57"));
    ASSERT_EQ(expectedInt24, CtRgbUtil::get_rgb24int_from_str_any("f1ab57"));
    ASSERT_EQ(expectedInt24, CtRgbUtil::get_rgb24int_from_str_any("f122ab335744"));
}

TEST(MiscUtilsGroup, natural_compare)
{
    ASSERT_TRUE(CtStrUtil::natural_compare("","") == 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("","a") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a","") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a","a") == 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("","9") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("9","") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("1","1") == 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("1","2") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("3","2") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a1","a1") == 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a1","a2") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a2","a1") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a1a2","a1a3") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a1a2","a1a0") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("134","122") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("12a3","12a3") == 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("12a1","12a0") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("12a1","12a2") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("a","aa") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("aaa","aa") > 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("Alpha 2","Alpha 2") == 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("Alpha 2","Alpha 2A") < 0);
    ASSERT_TRUE(CtStrUtil::natural_compare("Alpha 2 B","Alpha 2") > 0);
}

TEST(MiscUtilsGroup, str__startswith)
{
    ASSERT_TRUE(str::startswith("", ""));
    ASSERT_TRUE(str::startswith("123", ""));
    ASSERT_TRUE(str::startswith("sさいた", "sさい"));
    ASSERT_TRUE(not str::startswith("sさいた", "1"));
    ASSERT_TRUE(not str::startswith("", "さ"));
}

TEST(MiscUtilsGroup, str__startswith_any)
{
    ASSERT_TRUE(str::startswith_any("", std::array<const gchar*, 3>{"1", "", "3"}));
    ASSERT_TRUE(str::startswith_any("123", std::array<const gchar*, 1>{""}));
    ASSERT_TRUE(str::startswith_any("sさいた", std::array<const gchar*, 3>{"1", "sさ", "3"}));
    ASSERT_TRUE(not str::startswith_any("sさいた", std::array<const gchar*, 3>{"1", "さ", "3"}));
    ASSERT_TRUE(not str::startswith_any("", std::array<const gchar*, 3>{"1", "さ", "3"}));
}

TEST(MiscUtilsGroup, str__endswith)
{
    ASSERT_TRUE(str::endswith("", ""));
    ASSERT_TRUE(str::endswith("123", ""));
    ASSERT_TRUE(str::endswith("123", "23"));
    ASSERT_TRUE(not str::endswith("123", "1"));
    ASSERT_TRUE(not str::endswith("", "1"));
}

TEST(MiscUtilsGroup, str__indexOf)
{
    gunichar uc = Glib::ustring("さ")[0];
    ASSERT_EQ(str::indexOf("Saitama さいたま市", "さい"), 8);
    ASSERT_EQ(str::indexOf("Saitama さいたま市", "S"), 0);
    ASSERT_EQ(str::indexOf("Saitama さいたま市", 'a'), 1);
    ASSERT_EQ(str::indexOf("Saitama さいたま市", uc), 8);
}

TEST(MiscUtilsGroup, str__xml_escape)
{
    ASSERT_STREQ("", str::xml_escape("").c_str());
    ASSERT_STREQ("Ру", str::xml_escape("Ру").c_str());
    ASSERT_STREQ("&lt;Ру&gt;", str::xml_escape("<Ру>").c_str());
    ASSERT_STREQ("1&lt;2&quot;3&quot;4&amp;&gt;", str::xml_escape("1<2\"3\"4&>").c_str());
}

TEST(MiscUtilsGroup, str__join)
{
    std::vector<std::string> empty_v;
    std::vector<std::string> v_1{"1"};
    std::vector<std::string> v_2{"1", "2"};

    ASSERT_TRUE(str::join(empty_v, ",") == "");
    ASSERT_TRUE(str::join(v_1, ",") == "1");
    ASSERT_TRUE(str::join(v_2, ",") == "1,2");

    {
        std::vector<Glib::ustring> vecToJoin({"", "a", "bc", "", "d", ""});
        Glib::ustring rejoined = str::join(vecToJoin, ":");
        ASSERT_STREQ(":a:bc::d:", rejoined.c_str());
    }
    {
        std::vector<std::string> vecToJoin({"", "a", "bc", "", "d", ""});
        std::string rejoined = str::join(vecToJoin, " ");
        ASSERT_STREQ(" a bc  d ", rejoined.c_str());
    }
}

TEST(MiscUtilsGroup, str__join_numbers)
{
    std::vector<gint64> vecToJoin({-1, 1, 0, 1000});
    ASSERT_STREQ("-1,1,0,1000", str::join_numbers(vecToJoin, ",").c_str());
}

TEST(MiscUtilsGroup, str__swapcase)
{
    ASSERT_STREQ("CheRrY", str::swapcase("cHErRy").c_str());
}

TEST(MiscUtilsGroup, str__repeat)
{
    ASSERT_STREQ("", str::repeat("**", 0).c_str());
    ASSERT_STREQ("**", str::repeat("**", 1).c_str());
    ASSERT_STREQ("******", str::repeat("**", 3).c_str());
}

TEST(MiscUtilsGroup, vec_remove)
{
    std::vector<int> empty_v;
    std::vector<int> v_3{1, 2, 3};

    vec::remove(empty_v, 1);
    vec::remove(v_3, 0);
    ASSERT_TRUE(v_3.size() == 3);
    vec::remove(v_3, 2);
    ASSERT_TRUE(v_3.size() == 2);
}

TEST(MiscUtilsGroup, get__uri_type)
{
    ASSERT_TRUE(CtMiscUtil::get_uri_type("https://google.com") == CtMiscUtil::URI_TYPE::WEB_URL);
    ASSERT_TRUE(CtMiscUtil::get_uri_type("http://google.com") == CtMiscUtil::URI_TYPE::WEB_URL);

    ASSERT_TRUE(CtMiscUtil::get_uri_type("/home") == CtMiscUtil::URI_TYPE::LOCAL_FILEPATH);
    ASSERT_TRUE(CtMiscUtil::get_uri_type("C:\\\\windows") == CtMiscUtil::URI_TYPE::LOCAL_FILEPATH);
    ASSERT_TRUE(CtMiscUtil::get_uri_type(UT::unitTestsDataDir) == CtMiscUtil::URI_TYPE::LOCAL_FILEPATH);

    ASSERT_TRUE(CtMiscUtil::get_uri_type("smb://localhost") == CtMiscUtil::URI_TYPE::UNKNOWN);
    ASSERT_TRUE(CtMiscUtil::get_uri_type("my invalid uri") == CtMiscUtil::URI_TYPE::UNKNOWN);
}

TEST(MiscUtilsGroup, mime__type_contains)
{
// some tests don't work on TRAVIS with WIN32
#if !(defined(_TRAVIS) && defined(_WIN32))
    ASSERT_TRUE(CtMiscUtil::mime_type_contains(UT::unitTestsDataDir+"/mimetype_txt.txt", "text/"));
    ASSERT_TRUE(CtMiscUtil::mime_type_contains(UT::unitTestsDataDir+"/mimetype_html.html", "text/"));
    ASSERT_TRUE(CtMiscUtil::mime_type_contains(UT::unitTestsDataDir+"/mimetype_html.html", "html"));

// test doesn't work on WIN32 and MacOS (Travis)
#if !(defined(_WIN32) || (defined(_TRAVIS) && defined(__APPLE__)))
    ASSERT_TRUE(CtMiscUtil::mime_type_contains(UT::unitTestsDataDir+"/mimetype_cpp.cpp", "text/"));
#endif

    ASSERT_TRUE(!CtMiscUtil::mime_type_contains(UT::unitTestsDataDir+"/mimetype_ctb.ctb", "text/"));
#endif
}

TEST(MiscUtilsGroup, parallel_for)
{
    auto check_range_in_vec = [](const std::vector<int>& vec, const size_t& first, const size_t& last) -> bool {
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i >= first && i < last) {
                if (vec[i] != 1)
                    return false;
            } else if (vec[i] != 0)
                return false;
        }
        return true;
    };

    // test check_range_in_vec
    std::vector<int> vec(4, 0);
    ASSERT_TRUE(check_range_in_vec(vec, 0, 0) == true);
    ASSERT_TRUE(check_range_in_vec(vec, 0, 1) == false);
    vec[1] = 1;
    ASSERT_TRUE(check_range_in_vec(vec, 1, 1) == false);
    ASSERT_TRUE(check_range_in_vec(vec, 1, 2) == true);
    vec[1] = 2;
    ASSERT_TRUE(check_range_in_vec(vec, 1, 1) == false);
    ASSERT_TRUE(check_range_in_vec(vec, 1, 2) == false);

    // parallel_for splits the given range on slices, one slice per thread.
    // The formula to calculate slices is not simple, so the unit test checks that every element
    // in the given range is processed, processed only once, no one is skipped,
    // and elements that are out of the range should not be touched.
    // To be sure, it is checked for different combinations of ranges, including empty range.
    size_t vec_len = 30;
    for (size_t first = 0; first < vec_len; ++first)
        for (size_t last = first; last < vec_len; ++last)
        {
            std::vector<int> vec(vec_len + 1, 0);
            CtMiscUtil::parallel_for(first, last, [&](size_t index){
                vec[index] += 1;
            });
            ASSERT_TRUE(check_range_in_vec(vec, first, last));
        }
}

TEST(MiscUtilsGroup, get_link_entry)
{
    ASSERT_STREQ(CtConst::LINK_TYPE_WEBS, CtMiscUtil::get_link_entry("webs https://example.com").type.c_str());
    ASSERT_STREQ("https://example.com", CtMiscUtil::get_link_entry("webs https://example.com").webs.c_str());
    ASSERT_STREQ(CtConst::LINK_TYPE_FILE, CtMiscUtil::get_link_entry("file L2hvbWUvZm9vL2Jhcgo=").type.c_str());
    ASSERT_STREQ("/home/foo/bar\n", CtMiscUtil::get_link_entry("file L2hvbWUvZm9vL2Jhcgo=").file.c_str());
    ASSERT_STREQ(CtConst::LINK_TYPE_FOLD, CtMiscUtil::get_link_entry("fold L2hvbWUvZm9vL2Jhcgo=").type.c_str());
    ASSERT_STREQ("/home/foo/bar\n", CtMiscUtil::get_link_entry("fold L2hvbWUvZm9vL2Jhcgo=").fold.c_str());
    ASSERT_STREQ(CtConst::LINK_TYPE_NODE, CtMiscUtil::get_link_entry("node 2 hi hi").type.c_str());
    ASSERT_TRUE(CtMiscUtil::get_link_entry("node 2 hi hi").node_id == 2);
    ASSERT_STREQ("hi hi", CtMiscUtil::get_link_entry("node 2 hi hi").anch.c_str());

    ASSERT_STREQ("", CtMiscUtil::get_link_entry("").type.c_str());
    ASSERT_STREQ("", CtMiscUtil::get_link_entry("https://example.com").type.c_str());
    ASSERT_STREQ("", CtMiscUtil::get_link_entry("/home/foo/bar").type.c_str());
    ASSERT_STREQ("", CtMiscUtil::get_link_entry("home https://example.com").type.c_str());
}

