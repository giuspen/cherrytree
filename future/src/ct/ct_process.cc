/*
 * ct_process.cc
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

#include "ct_process.h"
#include "ct_logging.h"
#include <gio/gio.h>
#include <memory>
#include <array>


std::unique_ptr<GError, decltype(&g_error_free)> gerror_factory(GError* err = nullptr) {
    return std::unique_ptr<GError, decltype(&g_error_free)>(err, g_error_free);
}

void wait_and_free_subprocess(GSubprocess* process) noexcept {
    if(process) g_subprocess_wait(process, nullptr, nullptr);
}

void CtProcess::run(const std::vector<std::string>& args, std::ostream &output)
{
    
    std::vector<const char *> process_args;
    process_args.emplace_back(_process_name.c_str());
    for (const auto &arg : args) {
        process_args.emplace_back(arg.c_str());
    }
    process_args.emplace_back(nullptr);


    int p_flags = G_SUBPROCESS_FLAGS_STDOUT_PIPE;
    if (_input_data) p_flags |= G_SUBPROCESS_FLAGS_STDIN_PIPE;
    auto p_err = gerror_factory();
    GError* err_raw = p_err.get();
    std::unique_ptr<GSubprocess, decltype(&wait_and_free_subprocess)> process(g_subprocess_newv(process_args.data(), static_cast<GSubprocessFlags>(p_flags), &err_raw), wait_and_free_subprocess);
    if (!process) {
        throw CtProcessError("Error occured during g_subprocess_new"); //fmt::format("Error occured during g_subprocess_new: {}", p_err->message));
    }
    
    if (_input_data) {
        auto* stdin_stream = g_subprocess_get_stdin_pipe(process.get());
        std::streamsize       pos;
        std::array<char, 256> buffer{};
        auto err = gerror_factory();
        auto* err_raw = err.get();
        gsize nb_written = 0;
        _input_data->read(buffer.data(), buffer.size());
        while (*_input_data || (pos = _input_data->gcount()) != 0) {
            if (!g_output_stream_write_all(stdin_stream, buffer.data(), buffer.size(), &nb_written, nullptr, &err_raw)) {
                throw CtProcessError("Error while writing to output pipe");//fmt::format("Error while writing to output pipe: {}", err->message));
            }
            
            
            if (pos != 0) _input_data->read(buffer.data(), buffer.size());
        }
        g_output_stream_close(stdin_stream, nullptr, nullptr);
    }
    
    auto* stdout_stream = g_subprocess_get_stdout_pipe(process.get());
    std::array<guint8, 257> buff{};
    while(true) {
        auto read_retr = g_input_stream_read(stdout_stream, buff.data(), buff.size() - 1, nullptr, nullptr);
        if (read_retr > 0) {
            output << buff.data();
        } else if (read_retr == 0) {
            // EOF
            break;
        } else {
            throw CtProcessError("Error while reading from process");//fmt::format("Error while reading from process"));
        }
    }   
}


void operator>>(CtProcess &process, std::ostream &output) 
{
    process.run(output);
}

void CtProcess::run(std::ostream &output) 
{
    run(_args, output);
}



