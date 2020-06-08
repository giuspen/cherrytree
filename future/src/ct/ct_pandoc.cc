/*
 * ct_pandoc.cc
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

#include <sstream>
#include "ct_pandoc.h"
#include "ct_process.h"
#include "ct_logging.h"

std::unique_ptr<CtProcess> pandoc_process() {
    auto p = std::make_unique<CtProcess>("pandoc");
    p->append_arg("-t");
    p->append_arg("html");
    return p;
}

namespace CtPandoc {

bool has_pandoc() {
    return true;
}


void to_html(std::istream& input, std::ostream& output) {
    auto process = pandoc_process();
    try {
        process->input(&input);
        process->run(output);
    } catch(std::exception& e) {
        spdlog::error("Exception in to_html: {}", e.what());
        throw;
    }
}

}
