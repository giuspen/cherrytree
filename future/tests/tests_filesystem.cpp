/*
 * tests_filesystem.cpp
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

#include "ct_filesystem.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "test_consts.h"



TEST_GROUP(FileSystemGroup)
{
};


TEST(FileSystemGroup, path_stem)
{
#ifndef _WIN32
    STRCMP_EQUAL("", fs::path("").stem().c_str());
    STRCMP_EQUAL("file", fs::path("/root/file").stem().c_str());
    STRCMP_EQUAL("file", fs::path("/root/file.txt").stem().c_str());
    STRCMP_EQUAL("file.2", fs::path("/root/file.2.txt").stem().c_str());
    STRCMP_EQUAL(".txt", fs::path("/root/.txt").stem().c_str());
#endif
}

TEST(FileSystemGroup, path_extension)
{
    STRCMP_EQUAL("", fs::path("").extension().c_str());
    STRCMP_EQUAL("", fs::path(".").extension().c_str());
    STRCMP_EQUAL(".txt", fs::path("file.txt").extension().c_str());
    STRCMP_EQUAL("", fs::path("file").extension().c_str());
    STRCMP_EQUAL(".txt", fs::path("root/file.txt").extension().c_str());
    STRCMP_EQUAL(".txt", fs::path("/root/file.txt").extension().c_str());
    STRCMP_EQUAL(".txt", fs::path(".local/file.txt").extension().c_str());
    STRCMP_EQUAL(".my_bar", fs::path(".local/file.txt/.....my_bar").extension().c_str());
    STRCMP_EQUAL(".txt", fs::path("/home/foo/.local/file.txt/.....my_bar/txt.txt").extension().c_str());
    STRCMP_EQUAL("", fs::path("/home/foo/.local/file.txt/.....my_bar/txt.").extension().c_str());
    STRCMP_EQUAL(".txt", fs::path("foo..txt").extension().c_str());
    STRCMP_EQUAL("", fs::path("/home/foo/.local").extension().c_str());
    STRCMP_EQUAL("", fs::path("/home/foo/.config").extension().c_str());
}

TEST(FileSystemGroup, get_cherrytree_datadir)
{
    // we expect the unit test to be run from the built sources
    STRCMP_EQUAL(_CMAKE_SOURCE_DIR, fs::get_cherrytree_datadir().c_str());
}

TEST(FileSystemGroup, get_cherrytree_localedir)
{
    // we expect the unit test to be run from the built sources
    STRCMP_EQUAL(fs::canonical(Glib::build_filename(_CMAKE_SOURCE_DIR, "po")).c_str(), fs::get_cherrytree_localedir().c_str());
}

TEST(FileSystemGroup, is_regular_file)
{
    CHECK_EQUAL(true, fs::is_regular_file(unitTestsDataDir + "/test.ctd"));
    CHECK_EQUAL(true, fs::is_regular_file(unitTestsDataDir + "/test.ctb"));
    CHECK_EQUAL(true, fs::is_regular_file(unitTestsDataDir + "/md_testfile.md"));
    CHECK_EQUAL(false, fs::is_regular_file(unitTestsDataDir));
    CHECK_EQUAL(false, fs::is_regular_file(fs::absolute(unitTestsDataDir)));
    CHECK_EQUAL(false, fs::is_regular_file(fs::absolute(unitTestsDataDir).parent_path()));
}

TEST(FileSystemGroup, is_directory)
{
    CHECK_EQUAL(true, fs::is_directory(unitTestsDataDir));
    CHECK_EQUAL(false, fs::is_directory(unitTestsDataDir + "/test.ctd"));
    CHECK_EQUAL(true, fs::is_directory(fs::path(unitTestsDataDir).parent_path()));
    CHECK_EQUAL(false, fs::is_directory(fs::path(unitTestsDataDir).parent_path() / "test_consts.h"));
}

TEST(FileSystemGroup, get_doc_type)
{
    CHECK(CtDocType::SQLite == fs::get_doc_type(unitTestsDataDir + "/test.ctb"));
    CHECK(CtDocType::XML == fs::get_doc_type(unitTestsDataDir + "/test.ctd"));
    CHECK(CtDocType::None == fs::get_doc_type(unitTestsDataDir + "/md_testfile.md"));
    CHECK(CtDocType::SQLite == fs::get_doc_type(unitTestsDataDir + "/mimetype_ctb.ctb"));
    CHECK(CtDocType::XML == fs::get_doc_type(unitTestsDataDir + "/7zr.ctz"));
    CHECK(CtDocType::SQLite == fs::get_doc_type("test.ctx")); // Doesnt actually exist
}

TEST(FileSystemGroup, get_doc_encrypt)
{
    CHECK(fs::get_doc_encrypt(unitTestsDataDir + "/test.ctb") == CtDocEncrypt::False);
    CHECK(fs::get_doc_encrypt(unitTestsDataDir + "/test.ctz") == CtDocEncrypt::True);
    CHECK(fs::get_doc_encrypt(unitTestsDataDir + "/mimetype_txt.txt") == CtDocEncrypt::None);
}

TEST(FileSystemGroup, path_is_absolute)
{
    CHECK_EQUAL(true, fs::path("/home/foo").is_absolute());
    CHECK_EQUAL(false, fs::path("/home/foo").is_relative());
    CHECK_EQUAL(true, fs::path("home/foo").is_relative());
    CHECK_EQUAL(false, fs::path("home/foo").is_absolute());
}

TEST(FileSystemGroup, path_parent_path)
{

    STRCMP_EQUAL(fs::path("/home").c_str(), fs::path("/home/foo").parent_path().c_str());
    STRCMP_EQUAL(fs::path("/home/foo").c_str(), fs::path("/home/foo/bar.txt").parent_path().c_str());
    STRCMP_EQUAL(fs::path("home/foo").c_str(), fs::path("home/foo/bar.txt").parent_path().c_str());
}

TEST(FileSystemGroup, path_concat)
{
    fs::path p1("/home/foo/bar");
    fs::path p2(".txt");
    p1 += p2;
    STRCMP_EQUAL("/home/foo/bar.txt", p1.c_str());
}

TEST(FileSystemGroup, path_native)
{
#ifndef _WIN32
    std::string first = "/foo";
#else
    std::string first = "\\foo";
#endif
    fs::path path("/foo");

    STRCMP_EQUAL(path.native().c_str(), first.c_str());
}

TEST(FileSystemGroup, remove) {
    fs::path test_dir(unitTestsDataDir + "/test_dir");
    CHECK_EQUAL(0, g_mkdir_with_parents(test_dir.c_str(), 0777));
    CHECK(fs::exists(test_dir));
    fs::remove(test_dir);
    CHECK(!fs::exists(test_dir));

    CHECK_EQUAL( 0, g_mkdir_with_parents(test_dir.c_str(), 0777));
    CHECK_EQUAL(1, fs::remove_all(test_dir));
    CHECK(!fs::exists(test_dir));
}
