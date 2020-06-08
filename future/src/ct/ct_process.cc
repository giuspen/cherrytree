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
#include <fmt/format.h>
#include <unistd.h>


pid_t io_fork(int fds[2], const std::function<void(int, int)>& func) {
    int pipes[4];
    
    if (pipe(&pipes[0]) != 0 || pipe(&pipes[2]) != 0) {
        throw CtProcess::CtProcessError("Failed to open pipes");
    }
    
    auto pid = fork();
    if (pid > 0) {
        // Parent
        fds[0] = pipes[0];
        fds[1] = pipes[3];
        
        close(pipes[1]);
        close(pipes[2]);
        return pid;
    } else if (pid == 0) {
        // Child
        close(pipes[3]);
        close(pipes[0]);
        
        func(pipes[2], pipes[1]);
        _exit(0);
    }
    throw CtProcess::CtProcessError("fork() returned error code");
}


void CtProcess::run(const std::vector<std::string>& args, std::ostream &output) {
    
    // The casts here are safe because the exec family do not modify their args, its just for compatibility
    std::vector<char*> process_args;
    process_args.emplace_back(const_cast<char*>(_process_name.c_str()));
    for (const auto& arg : args) {
        process_args.emplace_back(const_cast<char*>(arg.c_str()));
    }
    process_args.emplace_back(nullptr);
    
   
   auto func = [process_args, this](int read_fs, int write_fs){
    
       dup2(write_fs, STDOUT_FILENO);
       dup2(write_fs, STDERR_FILENO);
       dup2(read_fs, STDIN_FILENO);
       
       close(write_fs);
       close(read_fs);
       
       execvp(_process_name.c_str(), process_args.data());
       spdlog::error("execlp() failed");
       _exit(1);
   };
   
   int fds[2];
    io_fork(fds, func);
    
   if (_input_data) {
       std::streamsize       pos;
       std::array<char, 256> buffer{};
       _input_data->read(buffer.data(), buffer.size());
       while (*_input_data || (pos = _input_data->gcount()) != 0) {
           if (write(fds[1], buffer.data(), buffer.size()) < 0) throw CtProcessError("Error while writing to output pipe");
        
           if (pos != 0) _input_data->read(buffer.data(), buffer.size());
       }
   }
    close(fds[1]);
    
    std::array<char, 257> buff{};
    while(true) {
        auto read_retr = read(fds[0], buff.data(), buff.size() - 1);
        if (read_retr > 0) {
            output << buff.data();
        } else if (read_retr == 0) {
            // EOF
            break;
        } else {
            throw CtProcessError(fmt::format("Error while reading from process; CODE: {}", errno));
        }
    }
}



void operator>>(CtProcess &process, std::ostream &output) {
    process.run(output);
}

void CtProcess::run(std::ostream &output) {
    run(_args, output);
}



