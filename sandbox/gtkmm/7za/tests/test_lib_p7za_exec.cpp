
// https://cpputest.github.io/
// sudo apt install cpputest
// g++ test_lib_p7za_exec.cpp ../.libs/libp7za.so -o bin_test_lib_p7za_exec `pkg-config cpputest glibmm-2.4 --cflags --libs` -lpthread

#include <glib/gstdio.h>
#include <glibmm.h>
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
}


int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
