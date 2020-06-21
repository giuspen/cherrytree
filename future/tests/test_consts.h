/*
 * test_consts.h
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

#include "config.h"

#include <string>
#include <glib/gstdio.h>
#include <glibmm/miscutils.h>

const std::string unitTestsDataDir{Glib::build_filename(_CMAKE_SOURCE_DIR, "tests", "data")};
const std::string ctzInputPath{Glib::build_filename(unitTestsDataDir, "7zr.ctz")};
const std::string ctxInputPath{Glib::build_filename(unitTestsDataDir, "7zr.ctx")};
const gchar testPassword[]{"7zr"};
const gchar testPasswordBis[]{"7zr2"};