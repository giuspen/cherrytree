/*
 * tests_imports.cpp
 *
 * Copyright 2009-2026
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

#include "ct_imports.h"
#include "ct_config.h"
#include "ct_filesystem.h"
#include "tests_common.h"

#include <glibmm.h>

namespace {

const std::string tomboyEmptyTagNote{R"(<?xml version="1.0" encoding="utf-8"?>
<note version="0.3">
  <title>Harmless Meeting Note</title>
  <tags>
    <tag></tag>
  </tags>
  <text>
    <note-content version="0.1">This is a normal looking note.</note-content>
  </text>
</note>
)"};

struct ScopedFileCleanup
{
  explicit ScopedFileCleanup(const fs::path& path) : filePath{path} {}

    ~ScopedFileCleanup() {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }
    }

    fs::path filePath;
};

} // namespace

TEST(ImportsGroup, TomboyEmptyTagDoesNotCrash)
{
    Glib::init();

    fs::path notePath = fs::path{UT::unitTestsDataDir} / "crash_empty_tag.note";
    ScopedFileCleanup scopedCleanup{notePath};

    Glib::file_set_contents(notePath.string(), tomboyEmptyTagNote);

    CtTomboyImport importer{CtConfig::GetCtConfig()};
    std::unique_ptr<CtImportedNode> importedNode;
    ASSERT_NO_FATAL_FAILURE(importedNode = importer.import_file(notePath));

    if (importedNode) {
      ASSERT_STREQ("Harmless Meeting Note", importedNode->node_name.c_str());
    }
}
