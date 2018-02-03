
// https://cpputest.github.io/
// sudo apt install cpputest
// g++ test_lib_p7za_exec.cpp -Wl,--whole-archive ../libp7za.a -Wl,--no-whole-archive `pkg-config cpputest glibmm-2.4 libxml++-2.6 --cflags --libs` -lpthread -o bin_test_lib_p7za_exec -Wno-deprecated

#include <glib/gstdio.h>
#include <glibmm.h>
#include <libxml++/libxml++.h>
#include <iostream>
#include "CppUTest/CommandLineTestRunner.h"


extern int p7za_exec(int numArgs, char *args[]);


const Glib::ustring TEST_DIR = Glib::path_get_dirname(__FILE__);
const Glib::ustring CTZ_INPUT_PATH = Glib::build_filename(TEST_DIR, "7zr.ctz");
const Glib::ustring CTD_TMP_PATH = Glib::build_filename(TEST_DIR, "7zr.ctd");
const Glib::ustring CTZ_TMP_PATH = Glib::build_filename(TEST_DIR, "7zr2.ctz");
const gchar PASSWORD[] = "7zr";


static int _extract(const gchar * input_path)
{
    gchar * p_args = g_strdup_printf("7za e -p%s -w%s -bd -y -o%s %s", PASSWORD, TEST_DIR.c_str(), TEST_DIR.c_str(), input_path);
    gchar **pp_args = g_strsplit(p_args, " ", 0);
    g_free(p_args);
    int ret_val = p7za_exec(8, pp_args);
    g_strfreev(pp_args);
    return ret_val;
}


static int _archive(const gchar * input_path, const gchar * output_path)
{
    gchar * p_args = g_strdup_printf("7za a -p%s -w%s -mx1 -bd -y %s %s", PASSWORD, TEST_DIR.c_str(), output_path, input_path);
    gchar **pp_args = g_strsplit(p_args, " ", 0);
    g_free(p_args);
    int ret_val = p7za_exec(9, pp_args);
    g_strfreev(pp_args);
    return ret_val;
}


static void _cleanup()
{
    for (auto filepath : std::list<Glib::ustring>{CTD_TMP_PATH, CTZ_TMP_PATH})
    {
        if (Glib::file_test(filepath, Glib::FILE_TEST_EXISTS))
        {
            g_remove(filepath.c_str());
        }
    }
}


TEST_GROUP(P7zaExecGroup)
{
    void setup()
    {
        _cleanup();
    }

    void teardown()
    {
        _cleanup();
    }
};


TEST(P7zaExecGroup, extract)
{
    _extract(CTZ_INPUT_PATH.c_str());
    CHECK(Glib::file_test(CTD_TMP_PATH, Glib::FILE_TEST_EXISTS));
    std::string xml_txt = Glib::file_get_contents(CTD_TMP_PATH);
    xmlpp::DomParser dom_parser;
    dom_parser.parse_memory(xml_txt);
    xmlpp::Document* p_document = dom_parser.get_document();
    xmlpp::Element* p_element = p_document->get_root_node();
    CHECK(0 == p_element->get_name().compare("cherrytree"));
    CHECK(0 == static_cast<xmlpp::Element*>(p_element->find("node")[0])->get_attribute_value("name").compare("NodeName"));
    CHECK(0 == static_cast<xmlpp::Element*>(p_element->find("node/rich_text")[0])->get_child_text()->get_content().compare("NodeContent"));
    _archive(CTD_TMP_PATH.c_str(), CTZ_TMP_PATH.c_str());
    CHECK(Glib::file_test(CTZ_TMP_PATH, Glib::FILE_TEST_EXISTS));
    g_remove(CTD_TMP_PATH.c_str());
    _extract(CTZ_TMP_PATH.c_str());
    CHECK(Glib::file_test(CTD_TMP_PATH, Glib::FILE_TEST_EXISTS));
    std::string xml_txt_bis = Glib::file_get_contents(CTD_TMP_PATH);
    CHECK(0 == xml_txt.compare(xml_txt_bis));
}


int main(int ac, char** av)
{
    MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
    return CommandLineTestRunner::RunAllTests(ac, av);
}
