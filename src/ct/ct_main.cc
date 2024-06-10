/*
 * ct_main.cc
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

#include "ct_app.h"
#include "ct_misc_utils.h"
#include "config.h"
#include "ct_logging.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#if defined(_WIN32)
#include <locale>
#include <codecvt>
#endif /* _WIN32 */

void glib_log_handler(const gchar*/*log_domain*/, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
{
    if (CtApp::inside_gsv_init or not message or not user_data) {
        return;
    }
    auto pGtkLogger = static_cast<spdlog::logger*>(user_data);
    switch (log_level) {
        case G_LOG_LEVEL_ERROR:    pGtkLogger->error(message);    break;
        case G_LOG_LEVEL_CRITICAL: pGtkLogger->critical(message); break;
        case G_LOG_LEVEL_WARNING:  pGtkLogger->warn(message);     break;
        case G_LOG_LEVEL_MESSAGE:  pGtkLogger->info(message);     break;
        case G_LOG_LEVEL_INFO:     pGtkLogger->info(message);     break;
        case G_LOG_LEVEL_DEBUG:
            // disable due to excessive output
            //pGtkLogger->debug(msg);
            break;
        default:                   pGtkLogger->info(message);
    }
}

int main(int argc, char *argv[])
{
    {
        const char* pExePath = argv[0];
#if defined(_WIN32)
        wchar_t path_buff[1024];
        std::wstring wtf = std::wstring(path_buff, GetModuleFileNameW(NULL, path_buff, 1023));
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string converted_str = converter.to_bytes(wtf);
        pExePath = converted_str.c_str();
#endif /* _WIN32 */
        g_message("exe_path = %s", pExePath);
        fs::register_exe_path_detect_if_portable(pExePath);
    }

#ifdef HAVE_NLS
    const std::string ct_lang = CtMiscUtil::get_ct_language();
    if (ct_lang != CtConst::LANG_DEFAULT) {
        const std::string ct_lang_utf8 = ct_lang + ".UTF-8";
        if ( fs::alter_locale_env_var("LANGUAGE", ct_lang + ":en") and
             fs::alter_locale_env_var("LANG", ct_lang_utf8) and
             fs::alter_locale_env_var("LC_ALL", ct_lang_utf8) ) {
            g_message("Language overwrite = %s (localedir = %s)", ct_lang.c_str(), fs::get_cherrytree_localedir().c_str());
        }
        else {
            g_critical("Couldn't set language %s", ct_lang.c_str());
        }
    }
    bindtextdomain(GETTEXT_PACKAGE, fs::get_cherrytree_localedir().c_str());
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif /* HAVE_NLS */

    // output logs into console and a log file
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    std::optional<fs::path> optLogdir = fs::get_cherrytree_logdir();
    if (optLogdir.has_value()) {
        try {
            // Create a file rotating logger with 5mb size max and 3 rotated files
            auto max_size = 1048576 * 5;
            auto max_files = 3;
            fs::path log_path = optLogdir.value() / "cherrytree.log";
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_path.c_str(), max_size, max_files));
        }
        catch (const spdlog::spdlog_ex &ex) {
            spdlog::debug("Log init failed: {}", ex.what());
        }
    }

    spdlog::drop(""); // remove the default logger (if you want, you can use its name)
    // these two loggers are the same, they just add "[che]" and "[gtk]" in their output
    auto cherrytree_logger = std::make_shared<spdlog::logger>("che", begin(sinks), end(sinks));
    auto gtk_logger = std::make_shared<spdlog::logger>("gtk", begin(sinks), end(sinks));

    spdlog::set_default_logger(cherrytree_logger);         // make our logger as a default logger
    spdlog::register_logger(gtk_logger);                   // register it, so we can access it in another place
    spdlog::flush_on(spdlog::level::debug);                // flush when "info" or higher message is logged on all loggers
    spdlog::set_level(spdlog::level::debug);               // Setup spdlog, use debug level by default for now

    g_log_set_default_handler(glib_log_handler, gtk_logger.get()); // Redirect Gtk log messages to spdlog

    bool is_secondary_session{false};
    for (int i = 1; i < argc; ++i) {
        if (0 == strcmp("-S", argv[i]) or
            0 == strcmp("--secondary_session", argv[i]))
        {
            is_secondary_session = true;
            break;
        }
    }

    Glib::RefPtr<CtApp> r_app = CtApp::create(is_secondary_session ? "_2" : "");
    return r_app->run(argc, argv);
}
