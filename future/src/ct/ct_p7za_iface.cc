/*
 * ct_p7za_iface.cc
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

#include "ct_p7za_iface.h"
#include <glib/gstdio.h>
#include <string>
#include <vector>
#include <algorithm>

extern int p7za_exec(int numArgs, char *args[]);
extern void cherrytree_register_7zaes();
extern void cherrytree_register_crc32();
extern void cherrytree_register_crc_table();
extern void cherrytree_register_crc64();
extern void cherrytree_register_7z();
extern void cherrytree_register_lzma2();
extern void cherrytree_register_lzma();

static void register_codecs()
{
    // to fix linker and remove '-whole-archive'
    // call dummy functions in 7z obj files
    // they could be called once, but make it simpler, call every time
    cherrytree_register_7zaes();
    cherrytree_register_crc32();
    cherrytree_register_crc_table();
    cherrytree_register_crc64();
    cherrytree_register_7z();
    cherrytree_register_lzma2();
    cherrytree_register_lzma();
}

#ifdef _WIN32
static void _slashes_convert(std::string& path)
{
    std::replace(path.begin(), path.end(), '\\', '/');
}
#endif // _WIN32

static gchar** vector_to_array(const std::vector<std::string>& vec)
{
    gchar **array = g_new(gchar *, vec.size()+1);
    for (size_t i = 0; i < vec.size(); ++i)
        array[i] = g_strdup(vec[i].c_str());
    array[vec.size()] = nullptr;
    return array;
}

int CtP7zaIface::p7za_extract(const gchar* input_path, const gchar* out_dir, const gchar* passwd)
{
    std::vector<std::string> args {
                "7za",
                "e",
                "-p" + std::string(passwd),
                "-w" + std::string(g_get_tmp_dir()),
                "-bd",   // Disable progress indicator
                "-bso0", // Disable standard output, error output is turn on
                "-bsp0", // Disable progress output
                "-bse1", // redirect error output into standard output
                "-y",
                "-o" + std::string(out_dir),
                input_path
    };
#ifdef _WIN32
    for (size_t i = 0; i < args.size(); ++i)
        _slashes_convert(args[i]);
#endif // _WIN32
    gchar** pp_args = vector_to_array(args);

    register_codecs();
    int ret_val = p7za_exec((int)args.size(), pp_args);
    g_strfreev(pp_args);
    return ret_val;
}

int CtP7zaIface::p7za_archive(const gchar* input_path, const gchar* output_path, const gchar* passwd)
{
    g_autofree gchar* p_workspace_dir = g_path_get_dirname(output_path);
    std::vector<std::string> args {
                "7za",
                "a",
                "-p" + std::string(passwd),
                "-w" + std::string(p_workspace_dir),
                "-mx1",
                "-bd",   // Disable progress indicator
                "-bso0", // Disable standard output, error output is turn on
                "-bsp0", // Disable progress output
                "-y",
                output_path,
                input_path
    };
#ifdef _WIN32
    for (size_t i = 0; i < args.size(); ++i)
        _slashes_convert(args[i]);
#endif // _WIN32
    gchar** pp_args = vector_to_array(args);

    register_codecs();
    int ret_val = p7za_exec(args.size(), pp_args);
    g_strfreev(pp_args);
    return ret_val;
}
