
// https://cpputest.github.io/
// sudo apt install cpputest
// g++ tests_str_utils.cpp ../src/str_utils.cc -o bin_tests_str_utils `pkg-config cpputest glibmm-2.4 --cflags --libs`

#include "../src/str_utils.h"
#include "CppUTest/CommandLineTestRunner.h"


TEST_GROUP(StrUtilsGroup)
{
};


TEST(StrUtilsGroup, replace_in_string)
{
    Glib::ustring test_replaces_str = "one two threetwo";
    CHECK(replace_in_string(test_replaces_str, "two", "four") == "one four threefour");
}

TEST(StrUtilsGroup, trim_string)
{
    Glib::ustring test_trim_str = "\t one two three ";
    CHECK(trim_string(test_trim_str) == "one two three");
}

TEST(StrUtilsGroup, gint64_from_gstring)
{
    CHECK(gint64_from_gstring("") == 0);
    CHECK(gint64_from_gstring("0x7fffffffffffffff") == 0x7fffffffffffffff);
    CHECK(gint64_from_gstring("-0x8000000000000000") == -0x8000000000000000);
    CHECK(gint64_from_gstring("1234") == 1234);
    CHECK(gint64_from_gstring("-1234") == -1234);
    CHECK(gint64_from_gstring("ffff", true) == 0xffff);
}

TEST(StrUtilsGroup, gstring_split2ustring)
{
    const gchar str_orig[] = ":a:bc::d:";
    const gchar str_delimiter[] = ":";

    std::list<Glib::ustring> splitted_list = gstring_split2ustring(str_orig, str_delimiter);
    CHECK(splitted_list == std::list<Glib::ustring>({"", "a", "bc", "", "d", ""}));
}

TEST(StrUtilsGroup, gstring_split2int64)
{
    const gchar str_int64_orig[] = "-1, 1,0, 1000";
    const gchar str_delimiter_int64[] = ",";

    std::list<gint64> splitted_list = gstring_split2int64(str_int64_orig, str_delimiter_int64);
    CHECK(splitted_list == std::list<gint64>({-1, 1, 0, 1000}));
}


int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
