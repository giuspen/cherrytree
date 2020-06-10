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

#if defined(__WIN32)
#include <windows.h>


void throw_with_last_error(std::string_view msg) {
    auto err = GetLastError();
    throw CtProcess::CtProcessError(fmt::format("{}; ERROR: {}", msg, err));
}

void create_child_process(std::string_view process_name, void* write_handle, void* read_handle)
{
    PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));
    
    STARTUPINFO start_info;
    ZeroMemory(&start_info, sizeof(STARTUPINFO));
    
    start_info.cb = sizeof(STARTUPINFO);
    start_info.hStdError = write_handle;
    start_info.hStdOutput = write_handle;
    start_info.hStdInput = read_handle;
    start_info.dwFlags |= STARTF_USESTDHANDLES;
    
    
    std::vector<char> chs;
    chs.reserve(process_name.size());
    
    for (auto ch : process_name) {
        chs.emplace_back(ch);
    }
    
    bool did_succeed = CreateProcess(nullptr, chs.data(), nullptr, nullptr, true, 0, nullptr, nullptr, &start_info, &process_info);
    
    if (!did_succeed) {
        throw_with_last_error("Error occurred in CreateProcess");
    }
    
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
    
    CloseHandle(write_handle);
    CloseHandle(read_handle);
    
}



#else
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

#endif  // IFDEF __WIN32

void CtProcess::run(const std::vector<std::string>& args, std::ostream &output) {
    
    // The casts here are safe because the exec family do not modify their args, its just for compatibility
    std::vector<char *> process_args;
    process_args.emplace_back(const_cast<char *>(_process_name.c_str()));
    for (const auto &arg : args) {
        process_args.emplace_back(const_cast<char *>(arg.c_str()));
    }
    process_args.emplace_back(nullptr);

#if defined(__WIN32)
    SECURITY_ATTRIBUTES sec_attrs;
    
    spdlog::debug("Start of parent exe");
    
    sec_attrs.nLength              = sizeof(sec_attrs);
    sec_attrs.bInheritHandle       = true;
    sec_attrs.lpSecurityDescriptor = nullptr;
    
    HANDLE child_in_wd;
    HANDLE child_in_rd;
    HANDLE child_out_rd;
    HANDLE child_out_wd;
    
    
    if (!CreatePipe(&child_out_rd, &child_out_wd, &sec_attrs, 0)) {
        throw_with_last_error("Failed to create output pipes for child");
    }
    
    if (!SetHandleInformation(child_out_rd, HANDLE_FLAG_INHERIT, 0)) {
        throw_with_last_error("Failed to set read handle inheritance for child");
    }
    
    if (!CreatePipe(&child_in_rd, &child_in_wd, &sec_attrs, 0)) {
        throw_with_last_error("Failed to create input pipes for child");
    }
    
    if (!SetHandleInformation(child_in_wd, HANDLE_FLAG_INHERIT, 0)) {
        throw_with_last_error("Failed to set handle inheritance for child stdin");
    }
    
    create_child_process("echo", child_out_wd, child_in_rd);
    
    /*while(true) {
        bool success = WriteFile(child_in_wd, )
        
        
    }*/
    DWORD                 dw_read;
    std::array<char, 267> buff{};
    
    while (true) {
        bool success = ReadFile(child_out_rd, buff.data(), buff.size() - 1, &dw_read, nullptr);
        if (!success || dw_read == 0) break;
        
        spdlog::debug("GOT: {}", buff.data());
        output << buff.data();
    }
    
#else
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
#endif // IFDEF __WIN32
}


void operator>>(CtProcess &process, std::ostream &output) {
    process.run(output);
}

void CtProcess::run(std::ostream &output) {
    run(_args, output);
}



