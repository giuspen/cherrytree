/*
 * ct_p7za_iface.h
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
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
#include <glib.h>
#include <glib/gtypes.h>

namespace CtP7zaIface {

int p7za_extract(const gchar* input_path, const gchar* out_dir, const gchar* passwd, bool suppress_error);

int p7za_archive(const gchar* input_path, const gchar* output_path, const gchar* passwd);

} // namespace CtP7zaIface

