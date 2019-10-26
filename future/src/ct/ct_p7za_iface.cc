/*
 * ct_p7za_iface.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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

int CtP7zaIface::p7za_extract(const gchar* input_path, const gchar* out_dir, const gchar* passwd)
{
    g_autofree gchar* p_args = g_strdup_printf("7za e -p%s -w%s -bd -y -o%s %s", passwd, g_get_tmp_dir(), out_dir, input_path);
    gchar** pp_args = g_strsplit(p_args, " ", 0);
    int ret_val = p7za_exec(8, pp_args);
    g_strfreev(pp_args);
    return ret_val;
}

int CtP7zaIface::p7za_archive(const gchar* input_path, const gchar* output_path, const gchar* passwd)
{
    g_autofree gchar* p_args = g_strdup_printf("7za a -p%s -w%s -mx1 -bd -y %s %s", passwd, g_get_tmp_dir(), output_path, input_path);
    gchar** pp_args = g_strsplit(p_args, " ", 0);
    int ret_val = p7za_exec(9, pp_args);
    g_strfreev(pp_args);
    return ret_val;
}
