/*
 * tests_filesystem.cpp
 *
 * Copyright 2009-2023
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
#include "tests_common.h"
#include <glibmm.h>

TEST(FileSystemGroup, path_stem)
{
#ifndef _WIN32
    ASSERT_STREQ("", fs::path("").stem().c_str());
    ASSERT_STREQ("file", fs::path("/root/file").stem().c_str());
    ASSERT_STREQ("file", fs::path("/root/file.txt").stem().c_str());
    ASSERT_STREQ("file.2", fs::path("/root/file.2.txt").stem().c_str());
    ASSERT_STREQ(".txt", fs::path("/root/.txt").stem().c_str());
#endif
}

TEST(FileSystemGroup, path_extension)
{
    ASSERT_STREQ("", fs::path("").extension().c_str());
    ASSERT_STREQ("", fs::path(".").extension().c_str());
    ASSERT_STREQ(".txt", fs::path("file.txt").extension().c_str());
    ASSERT_STREQ("", fs::path("file").extension().c_str());
    ASSERT_STREQ(".txt", fs::path("root/file.txt").extension().c_str());
    ASSERT_STREQ(".txt", fs::path("/root/file.txt").extension().c_str());
    ASSERT_STREQ(".txt", fs::path(".local/file.txt").extension().c_str());
    ASSERT_STREQ(".my_bar", fs::path(".local/file.txt/.....my_bar").extension().c_str());
    ASSERT_STREQ(".txt", fs::path("/home/foo/.local/file.txt/.....my_bar/txt.txt").extension().c_str());
    ASSERT_STREQ("", fs::path("/home/foo/.local/file.txt/.....my_bar/txt.").extension().c_str());
    ASSERT_STREQ(".txt", fs::path("foo..txt").extension().c_str());
    ASSERT_STREQ("", fs::path("/home/foo/.local").extension().c_str());
    ASSERT_STREQ("", fs::path("/home/foo/.config").extension().c_str());
}

TEST(FileSystemGroup, get_cherrytree_datadir)
{
    // we expect the unit test to be run from the built sources
    ASSERT_STREQ(_CMAKE_SOURCE_DIR, fs::get_cherrytree_datadir().c_str());
}

TEST(FileSystemGroup, get_cherrytree_localedir)
{
    // we expect the unit test to be run from the built sources
    ASSERT_STREQ(fs::canonical(Glib::build_filename(_CMAKE_SOURCE_DIR, "po")).c_str(), fs::get_cherrytree_localedir().c_str());
}

TEST(FileSystemGroup, is_regular_file)
{
    ASSERT_TRUE(fs::is_regular_file(UT::testCtdDocPath));
    ASSERT_TRUE(fs::is_regular_file(UT::testCtbDocPath));
    ASSERT_TRUE(fs::is_regular_file(UT::unitTestsDataDir + "/md_testfile.md"));
    ASSERT_FALSE(fs::is_regular_file(UT::unitTestsDataDir));
    ASSERT_FALSE(fs::is_regular_file(fs::absolute(UT::unitTestsDataDir)));
    ASSERT_FALSE(fs::is_regular_file(fs::absolute(UT::unitTestsDataDir).parent_path()));
}

TEST(FileSystemGroup, is_directory)
{
    ASSERT_TRUE(fs::is_directory(UT::unitTestsDataDir));
    ASSERT_FALSE(fs::is_directory(UT::testCtdDocPath));
    ASSERT_TRUE(fs::is_directory(fs::path(UT::unitTestsDataDir).parent_path()));
    ASSERT_FALSE(fs::is_directory(fs::path(UT::unitTestsDataDir).parent_path() / "test_consts.h"));
}

TEST(FileSystemGroup, get_doc_type_from_file_ext)
{
    ASSERT_TRUE(CtDocType::SQLite == fs::get_doc_type_from_file_ext(UT::testCtbDocPath));
    ASSERT_TRUE(CtDocType::XML == fs::get_doc_type_from_file_ext(UT::testCtdDocPath));
    ASSERT_TRUE(CtDocType::None == fs::get_doc_type_from_file_ext(UT::unitTestsDataDir + "/md_testfile.md"));
    ASSERT_TRUE(CtDocType::SQLite == fs::get_doc_type_from_file_ext(UT::unitTestsDataDir + "/mimetype_ctb.ctb"));
    ASSERT_TRUE(CtDocType::XML == fs::get_doc_type_from_file_ext(UT::unitTestsDataDir + "/7zr.ctz"));
    ASSERT_TRUE(CtDocType::SQLite == fs::get_doc_type_from_file_ext("test.ctx")); // Doesnt actually exist
}

TEST(FileSystemGroup, get_doc_encrypt_from_file_ext)
{
    ASSERT_TRUE(fs::get_doc_encrypt_from_file_ext(UT::testCtbDocPath) == CtDocEncrypt::False);
    ASSERT_TRUE(fs::get_doc_encrypt_from_file_ext(UT::testCtzDocPath) == CtDocEncrypt::True);
    ASSERT_TRUE(fs::get_doc_encrypt_from_file_ext(UT::unitTestsDataDir + "/mimetype_txt.txt") == CtDocEncrypt::None);
}

TEST(FileSystemGroup, path_is_absolute)
{
    ASSERT_TRUE(fs::path("/home/foo").is_absolute());
    ASSERT_FALSE(fs::path("/home/foo").is_relative());
    ASSERT_TRUE(fs::path("home/foo").is_relative());
    ASSERT_FALSE(fs::path("home/foo").is_absolute());
}

TEST(FileSystemGroup, path_parent_path)
{
    ASSERT_STREQ(fs::path("/home").c_str(), fs::path("/home/foo").parent_path().c_str());
    ASSERT_STREQ(fs::path("/home/foo").c_str(), fs::path("/home/foo/bar.txt").parent_path().c_str());
    ASSERT_STREQ(fs::path("home/foo").c_str(), fs::path("home/foo/bar.txt").parent_path().c_str());
}

TEST(FileSystemGroup, path_concat)
{
    fs::path p1("/home/foo/bar");
    fs::path p2(".txt");
    p1 += p2;
    ASSERT_STREQ("/home/foo/bar.txt", p1.c_str());
}

TEST(FileSystemGroup, path_native)
{
#ifndef _WIN32
    std::string first = "/foo";
#else
    std::string first = "\\foo";
#endif
    fs::path p("/foo");
    ASSERT_STREQ(p.string_native().c_str(), first.c_str());
}

TEST(FileSystemGroup, path_unix)
{
    fs::path p("C:\\foo\\bar");
    std::string first = "C:/foo/bar";
    ASSERT_STREQ(p.string_unix().c_str(), first.c_str());
}

TEST(FileSystemGroup, remove)
{
    // file remove
    fs::path test_file_path = fs::path{UT::unitTestsDataDir} / fs::path{"test_file.txt"};
    Glib::file_set_contents(test_file_path.string(), "blabla");
    ASSERT_TRUE(fs::exists(test_file_path));

    ASSERT_TRUE(fs::remove(test_file_path));
    ASSERT_FALSE(fs::exists(test_file_path));
    ASSERT_FALSE(fs::remove(test_file_path));

    // empty dir remove
    fs::path test_dir_path = fs::path{UT::unitTestsDataDir} / fs::path{"test_dir"};
    ASSERT_EQ(0, g_mkdir_with_parents(test_dir_path.c_str(), 0755));
    ASSERT_TRUE(fs::exists(test_dir_path));
    ASSERT_TRUE(fs::remove(test_dir_path));
    ASSERT_FALSE(fs::exists(test_dir_path));
    ASSERT_FALSE(fs::remove(test_dir_path));

    // non empty dir remove
    ASSERT_EQ(0, g_mkdir_with_parents(test_dir_path.c_str(), 0755));
    fs::path test_file_in_dir = test_dir_path / fs::path{"test_file.txt"};
    Glib::file_set_contents(test_file_in_dir.string(), "blabla");
    ASSERT_TRUE(fs::exists(test_file_in_dir));
    ASSERT_EQ(2, fs::remove_all(test_dir_path));
    ASSERT_FALSE(fs::exists(test_dir_path));
}

TEST(FileSystemGroup, move)
{
    fs::path test_file_path = fs::path{UT::unitTestsDataDir} / fs::path{"test_mv.txt"};
    fs::path test_dir_path1 = fs::path{UT::unitTestsDataDir} / fs::path{"test_mv_dir1"};
    fs::path test_dir_path2 = fs::path{UT::unitTestsDataDir} / fs::path{"test_mv_dir2"};
    if (fs::exists(test_dir_path1)) fs::remove_all(test_dir_path1);
    if (fs::exists(test_dir_path2)) fs::remove_all(test_dir_path2);

    Glib::file_set_contents(test_file_path.string(), "blabla");
    ASSERT_EQ(0, g_mkdir_with_parents(test_dir_path1.c_str(), 0755));
    ASSERT_EQ(0, g_mkdir_with_parents(test_dir_path2.c_str(), 0755));

    ASSERT_TRUE(fs::move_file(test_file_path, test_dir_path1 / fs::path{"test_mv.txt"}));
    ASSERT_TRUE(fs::is_regular_file(test_dir_path1 / fs::path{"test_mv.txt"}));
    ASSERT_FALSE(fs::exists(test_file_path));

    ASSERT_TRUE(fs::move_file(test_dir_path1, test_dir_path2 / test_dir_path1.filename()));
    ASSERT_TRUE(fs::is_directory(test_dir_path2 / test_dir_path1.filename()));
    ASSERT_TRUE(fs::is_regular_file(test_dir_path2 / test_dir_path1.filename() / fs::path{"test_mv.txt"}));
    ASSERT_FALSE(fs::is_directory(test_dir_path1));
    ASSERT_EQ(1, fs::get_dir_entries(test_dir_path2).size());
    ASSERT_EQ(1, fs::get_dir_entries(test_dir_path2 / test_dir_path1.filename()).size());
    ASSERT_EQ(3, fs::remove_all(test_dir_path2));
}

TEST(FileSystemGroup, relative)
{
#ifdef _WIN32
    ASSERT_STREQ("test.txt", fs::relative("C:\\tmp\\test.txt", "C:\\tmp").c_str());
    ASSERT_STREQ("tmp\\test.txt", fs::relative("C:\\tmp\\test.txt", "C:\\").c_str());
    ASSERT_STREQ("..\\test.txt", fs::relative("C:\\tmp\\test.txt", "C:\\tmp\\one").c_str());
    ASSERT_STREQ("..\\..\\test.txt", fs::relative("C:\\tmp\\test.txt", "C:\\tmp\\one\\two").c_str());
    ASSERT_STREQ("C:\\tmp\\test.txt", fs::relative("C:\\tmp\\test.txt", "D:\\tmp").c_str());
#else
    ASSERT_STREQ("test.txt", fs::relative("/tmp/test.txt", "/tmp").c_str());
    ASSERT_STREQ("tmp/test.txt", fs::relative("/tmp/test.txt", "/").c_str());
    ASSERT_STREQ("../test.txt", fs::relative("/tmp/test.txt", "/tmp/one").c_str());
    ASSERT_STREQ("../../test.txt", fs::relative("/tmp/test.txt", "/tmp/one/two").c_str());
#endif // _WIN32
}

TEST(FileSystemGroup, canonicalize_filename)
{
#ifdef _WIN32
    ASSERT_STREQ("C:\\opt\\one\\two\\three.txt", fs_canonicalize_filename("two\\three.txt", "C:\\opt\\one").c_str());
    ASSERT_STREQ("C:\\opt\\one\\two\\three.txt", fs_canonicalize_filename(".\\two\\three.txt", "C:\\opt\\one").c_str());
    ASSERT_STREQ("C:\\opt\\one\\two\\three.txt", fs_canonicalize_filename("..\\two\\three.txt", "C:\\opt\\one\\four").c_str());
    ASSERT_STREQ("C:\\opt\\one\\two\\three.txt", fs_canonicalize_filename("C:\\opt\\one\\two\\three.txt", "C:\\four\\five").c_str());
#else
    ASSERT_STREQ("/opt/one/two/three.txt", fs_canonicalize_filename("two/three.txt", "/opt/one").c_str());
    ASSERT_STREQ("/opt/one/two/three.txt", fs_canonicalize_filename("./two/three.txt", "/opt/one").c_str());
    ASSERT_STREQ("/opt/one/two/three.txt", fs_canonicalize_filename("../two/three.txt", "/opt/one/four").c_str());
    ASSERT_STREQ("/opt/one/two/three.txt", fs_canonicalize_filename("/opt/one/two/three.txt", "/four/five").c_str());
#endif
}
