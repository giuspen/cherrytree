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
    STRCMP_EQUAL("", CtFileSystem::path("").stem().c_str());
    STRCMP_EQUAL("file", CtFileSystem::path("/root/file").stem().c_str());
    STRCMP_EQUAL("file", CtFileSystem::path("/root/file.txt").stem().c_str());
    STRCMP_EQUAL("file.2", CtFileSystem::path("/root/file.2.txt").stem().c_str());
    STRCMP_EQUAL(".txt", CtFileSystem::path("/root/.txt").stem().c_str());
#endif
}

TEST(FileSystemGroup, path_extension)
{
    STRCMP_EQUAL("", CtFileSystem::path("").extension().c_str());
    STRCMP_EQUAL(".", CtFileSystem::path(".").extension().c_str());
    STRCMP_EQUAL(".txt", CtFileSystem::path("file.txt").extension().c_str());
    STRCMP_EQUAL("", CtFileSystem::path("file").extension().c_str());
    STRCMP_EQUAL(".txt", CtFileSystem::path("root/file.txt").extension().c_str());
    STRCMP_EQUAL(".txt", CtFileSystem::path("/root/file.txt").extension().c_str());
}

TEST(FileSystemGroup, get_cherrytree_datadir)
{
    // we expect the unit test to be run from the built sources
    STRCMP_EQUAL(_CMAKE_SOURCE_DIR, CtFileSystem::get_cherrytree_datadir().c_str());
}

TEST(FileSystemGroup, get_cherrytree_localedir)
{
    // we expect the unit test to be run from the built sources
    STRCMP_EQUAL(CtFileSystem::canonical(Glib::build_filename(_CMAKE_SOURCE_DIR, "po")).c_str(), CtFileSystem::get_cherrytree_localedir().c_str());
}

TEST(FileSystemGroup, is_regular_file)
{
    CHECK_EQUAL(true, CtFileSystem::is_regular_file(unitTestsDataDir + "/test.ctd"));
    CHECK_EQUAL(true, CtFileSystem::is_regular_file(unitTestsDataDir + "/test.ctb"));
    CHECK_EQUAL(true, CtFileSystem::is_regular_file(unitTestsDataDir + "/md_testfile.md"));
}

TEST(FileSystemGroup, is_directory)
{
    CHECK_EQUAL(true, CtFileSystem::is_directory(unitTestsDataDir));
    CHECK_EQUAL(true, CtFileSystem::is_directory(CtFileSystem::path(unitTestsDataDir).parent_path()));
}

TEST(FileSystemGroup, get_doc_type)
{
    CHECK(CtDocType::SQLite == CtFileSystem::get_doc_type(unitTestsDataDir + "/test.ctb"));
    CHECK(CtDocType::XML == CtFileSystem::get_doc_type(unitTestsDataDir + "/test.ctd"));
    CHECK(CtDocType::None == CtFileSystem::get_doc_type(unitTestsDataDir + "/md_testfile.md"));
}

TEST(FileSystemGroup, get_doc_encrypt)
{
    CHECK(CtFileSystem::get_doc_encrypt(unitTestsDataDir + "/test.ctb") == CtDocEncrypt::False);
    CHECK(CtFileSystem::get_doc_encrypt(unitTestsDataDir + "/test.ctz") == CtDocEncrypt::True);
    CHECK(CtFileSystem::get_doc_encrypt(unitTestsDataDir + "/mimetype_txt.txt") == CtDocEncrypt::None);
}

TEST(FileSystemGroup, path_is_absolute)
{
    CHECK_EQUAL(true, CtFileSystem::path("/home/foo").is_absolute());
    CHECK_EQUAL(false, CtFileSystem::path("/home/foo").is_relative());
    CHECK_EQUAL(true, CtFileSystem::path("home/foo").is_relative());
    CHECK_EQUAL(false, CtFileSystem::path("home/foo").is_absolute());
}

TEST(FileSystemGroup, path_parent_path)
{
    STRCMP_EQUAL("/home", CtFileSystem::path("/home/foo").parent_path().c_str());
    STRCMP_EQUAL("/home/foo", CtFileSystem::path("/home/foo/bar.txt").parent_path().c_str());
    STRCMP_EQUAL("home/foo", CtFileSystem::path("home/foo/bar.txt").parent_path().c_str());
}

TEST(FileSystemGroup, path_concat)
{
    CtFileSystem::path p1("/home/foo/bar");
    CtFileSystem::path p2(".txt");
    p1 += p2;
    STRCMP_EQUAL("/home/foo/bar.txt", p1.c_str());
}

TEST(FileSystemGroup, path_string)
{
#ifndef _WIN32
    std::string first = "/foo";
    std::string second = "/foo/bar.txt";
#else
    std::string first = "\\foo";
    std::string second = "\\foo\\bar.txt";
#endif
    CtFileSystem::path path(first.c_str());

    STRCMP_EQUAL(path.c_str(), "/foo");

    path /= "bar.txt";
    STRCMP_EQUAL(path.c_str(), second.c_str());
}

TEST(FileSystemGroup, remove) {
    CtFileSystem::path test_dir(unitTestsDataDir + "/test_dir");
    CHECK_EQUAL(0, g_mkdir_with_parents(test_dir.c_str(), 0777));
    CHECK(CtFileSystem::exists(test_dir));
    CtFileSystem::remove(test_dir);
    CHECK(!CtFileSystem::exists(test_dir));

    CHECK_EQUAL( 0, g_mkdir_with_parents(test_dir.c_str(), 0777));
    CHECK_EQUAL( 1, CtFileSystem::remove_all(test_dir));
    CHECK(!CtFileSystem::exists(test_dir));
}
