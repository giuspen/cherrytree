/*
 * ct_main.cc
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

#include "ct_app.h"
#include "ct_misc_utils.h"
#include "config.h"

int main(int argc, char *argv[])
{
    std::locale::global(std::locale("")); // Set the global C++ locale to the user-specified locale

    const std::string ct_lang = CtMiscUtil::get_ct_language();
    if (ct_lang != CtConst::LANG_DEFAULT)
    {
        if (Glib::setenv("LANGUAGE", ct_lang, true/*overwrite*/))
        {
            g_message("Language overwrite = %s", ct_lang.c_str());
        }
        else
        {
            g_critical("Couldn't set language %s", ct_lang.c_str());
        }
    }

    bindtextdomain(GETTEXT_PACKAGE, CtFileSystem::get_cherrytree_localedir().c_str());
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    auto p_app = CtApp::create();

    return p_app->run(argc, argv);
}
