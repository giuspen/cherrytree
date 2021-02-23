/*
 * tests_clipboard.cpp
 *
 * Copyright 2009-2021
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
// this test is for WIN32 and doesn't work on TRAVIS
#if (defined(_WIN32) && !defined(_TRAVIS))
    std::string inputClip_path{Glib::build_filename(UT::unitTestsDataDir, "clipboard_ms_input.txt")};
    std::string resultClip_path{Glib::build_filename(UT::unitTestsDataDir, "clipboard_ms_result.txt")};
    std::string inputClip = Glib::file_get_contents(inputClip_path);
    std::string resultClip = Glib::file_get_contents(resultClip_path);
    std::string converted = Win32HtmlFormat().convert_from_ms_clipboard(inputClip);

    ASSERT_FALSE(inputClip.empty());
    ASSERT_FALSE(resultClip.empty());
    ASSERT_STREQ(resultClip.c_str(), converted.c_str());

    std::string html = "<html></html>";
    std::string converted_2 = Win32HtmlFormat().convert_from_ms_clipboard(html);
    ASSERT_STREQ(html.c_str(), converted_2.c_str());
#endif
}
