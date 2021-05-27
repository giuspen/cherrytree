/*
 * tests_common.h
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

#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "config.h"

#include <string>
#include <list>
#include <glib/gstdio.h>
#include <glibmm/miscutils.h>

namespace UT {

const std::string unitTestsDataDir{Glib::build_filename(_CMAKE_SOURCE_DIR, "tests", "data")};
const std::string ctzInputPath{Glib::build_filename(unitTestsDataDir, "7zr.ctz")};
const std::string ctxInputPath{Glib::build_filename(unitTestsDataDir, "7zr.ctx")};
const std::string testCtbDocPath{Glib::build_filename(unitTestsDataDir, "test.ctb")};
const std::string testCtdDocPath{Glib::build_filename(unitTestsDataDir, "test.ctd")};
const std::string testCtxDocPath{Glib::build_filename(unitTestsDataDir, "test.ctx")};
const std::string testCtzDocPath{Glib::build_filename(unitTestsDataDir, "test.ctz")};
const std::string testBomUtf8Path{Glib::build_filename(unitTestsDataDir, "bom_utf8.txt")};
const std::string testBomUtf32BEPath{Glib::build_filename(unitTestsDataDir, "bom_utf32be.txt")};
const std::string testBomUtf32LEPath{Glib::build_filename(unitTestsDataDir, "bom_utf32le.txt")};
const std::string testBomUtf16BEPath{Glib::build_filename(unitTestsDataDir, "bom_utf16be.txt")};
const std::string testBomUtf16LEPath{Glib::build_filename(unitTestsDataDir, "bom_utf16le.txt")};
const gchar testPassword[]{"7zr"};
const gchar testPasswordBis[]{"7zr2"};

} // namespace UT
