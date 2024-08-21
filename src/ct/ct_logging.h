/*
 * ct_logging.h
 *
 * Copyright 2009-2024
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

#ifdef SHARED_FMT_SPDLOG
#include <spdlog/spdlog.h>
#include <fmt/core.h>
#include <fmt/printf.h>
#else // not SHARED_FMT_SPDLOG
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/bundled/printf.h"
#endif // not SHARED_FMT_SPDLOG
