/*
 * tests_clipboard.cpp
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

#include "ct_clipboard.h"
#include "tests_common.h"

TEST(ClipboardGroup, ms_clip_convert)
{
// test doesn't work on TRAVIS with WIN32
#if !(defined(_TRAVIS) && defined(_WIN32))
    std::string inputClip_path{Glib::build_filename(UT::unitTestsDataDir, "clipboard_ms_input.txt")};
    std::string resultClip_path{Glib::build_filename(UT::unitTestsDataDir, "clipboard_ms_result.txt")};
    gchar* inputClip{nullptr}, *resultClip{nullptr};
    g_file_get_contents(inputClip_path.c_str(), &inputClip, nullptr, nullptr);
    g_file_get_contents(resultClip_path.c_str(), &resultClip, nullptr, nullptr);
    Glib::ustring converted = Win32HtmlFormat().convert_from_ms_clipboard(inputClip);

    ASSERT_TRUE(inputClip != nullptr);
    ASSERT_TRUE(resultClip != nullptr);
    ASSERT_STREQ(resultClip, converted.data());
    g_free(inputClip);
    g_free(resultClip);

    Glib::ustring html = "<html></html>";
    Glib::ustring converted_2 = Win32HtmlFormat().convert_from_ms_clipboard(html);
    ASSERT_STREQ(html.data(), converted_2.data());
#endif
}
