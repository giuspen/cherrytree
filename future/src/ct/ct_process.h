/*
 * ct_process.h
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

#pragma once


#include <string>
#include <memory>
#include <ostream>
#include <istream>
#include <vector>


/**
 * @brief Interface for executing external processes
 * @class CtProcess
 */
class CtProcess {
public:
    /**
     * @brief Thrown when an error occurs due to an external process
     * @class CtProcessError
     */
    class CtProcessError: public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };
    
    explicit CtProcess(std::string process_name) : _process_name(std::move(process_name)) {}

    void args(std::vector<std::string> args) { _args = std::move(args); }
    void append_arg(std::string arg) { _args.emplace_back(std::move(arg)); }
    
    constexpr void input(std::istream* input) { _input_data = input; }
    
    void run(const std::vector<std::string>&, std::ostream& output);
    void run(std::ostream& output);
    
    
    friend void operator>>(CtProcess& process, std::ostream& output);
private:
    std::string _process_name;
    std::istream* _input_data = nullptr;
    std::vector<std::string> _args;
};

