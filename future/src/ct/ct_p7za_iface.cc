/*
 * ct_p7za_iface.cc
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

#include <glib/gstdio.h>
#include <glibmm.h>
#include "ct_p7za_iface.h"

extern int p7za_exec(int numArgs, char *args[]);

#ifdef _WIN32
static void _slashes_convert(gchar* pPath)
{
    const gchar from{'\\'};
    const gchar to{'/'};
    gchar* pChar{pPath};
    while (pChar)
    {
        pChar = strchr(pChar, from);
        if (pChar) *pChar = to;
    }
}
#endif // _WIN32

static gchar** vector_to_array(const std::vector<Glib::ustring>& vec)
{
    gchar **array = g_new (gchar *, vec.size()+1);
    for (size_t i = 0; i < vec.size(); ++i)
        array[i] = g_strdup(vec[i].c_str());
    array[vec.size()] = nullptr;
    return array;
}

int CtP7zaIface::p7za_extract(const gchar* input_path, const gchar* out_dir, const gchar* passwd)
{
    std::vector<Glib::ustring> args {
                "7za",
                "e",
                "-p" + Glib::ustring(passwd),
                "-w" + Glib::ustring(g_get_tmp_dir()),
                "-bd",   // Disable progress indicator
                "-bso0", // Disable standard output, error output is turn on
                "-bsp0", // Disable progress output
                "-y",
                "-o" + Glib::ustring(out_dir),
                input_path
    };
    gchar** pp_args = vector_to_array(args);

#ifdef _WIN32
    for (int i = 0; i < (int)args.size(); ++i)
        _slashes_convert(args[i]);
#endif // _WIN32

    int ret_val = p7za_exec((int)args.size(), pp_args);
    g_strfreev(pp_args);
    return ret_val;
}

int CtP7zaIface::p7za_archive(const gchar* input_path, const gchar* output_path, const gchar* passwd)
{
    g_autofree gchar* p_workspace_dir = g_path_get_dirname(output_path);
    std::vector<Glib::ustring> args {
                "7za",
                "a",
                "-p" + Glib::ustring(passwd),
                "-w" + Glib::ustring(p_workspace_dir),
                "-mx1",
                "-bd",   // Disable progress indicator
                "-bso0", // Disable standard output, error output is turn on
                "-bsp0", // Disable progress output
                "-y",
                output_path,
                input_path
    };
    gchar** pp_args = vector_to_array(args);

#ifdef _WIN32
    for (int i = 0; i < (int)args.size(); ++i)
        _slashes_convert(args[i]);
#endif // _WIN32

    int ret_val = p7za_exec(args.size(), pp_args);
    g_strfreev(pp_args);
    return ret_val;
}
