/*
 * ct_logging.h
 *
 * Copyright 2009-2020
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#ifdef _WIN32
#include <fmt/printf.h>
#define SPDLOG_FMT_EXTERNAL
#else
#include <spdlog/fmt/bundled/printf.h>
#endif
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <glibmm/ustring.h>

// ostream works badly on Win32 due to locale encoding
template <>
struct fmt::formatter<Glib::ustring>: formatter<string_view> {
  // parse is inherited from formatter<string_view>.
  template <typename FormatContext>
  auto format(Glib::ustring c, FormatContext& ctx) {
    return formatter<string_view>::format(c.c_str(), ctx);
  }
};
