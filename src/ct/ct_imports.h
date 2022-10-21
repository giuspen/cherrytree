/*
 * ct_imports.h
 *
 * Copyright 2009-2022
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

#pragma once

#include "ct_const.h"
#include "ct_filesystem.h"

#include <glibmm/ustring.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml++/libxml++.h>
#include <queue>
#include <utility>
#include <glibmm/i18n.h>
#include <memory>

namespace {
using TableMatrx = std::queue<std::queue<std::string>>;
}

struct CtImportedNode
{
    CtImportedNode(fs::path _path, const Glib::ustring& _name) : path{std::move(_path)}, node_name{_name} {}

    void add_broken_link(const Glib::ustring& link, xmlpp::Element* el) {
        content_broken_links[link].push_back(el);
    }
    bool has_content() {
        return xml_content->get_root_node();
    }
    void copy_content(std::unique_ptr<CtImportedNode>& copy_node) {
        node_syntax = copy_node->node_syntax;
        xml_content = copy_node->xml_content;
        content_broken_links = copy_node->content_broken_links;
    }

    fs::path                         path;
    Glib::ustring                    node_name;
    gint64                           node_id{-1};     // generated at the end
    std::string                      node_syntax{CtConst::RICH_TEXT_ID};
    std::shared_ptr<xmlpp::Document> xml_content{std::make_shared<xmlpp::Document>()};
    std::map<Glib::ustring, std::vector<xmlpp::Element*>> content_broken_links;
    std::list<std::unique_ptr<CtImportedNode>> children;
};

class CtImporterInterface
{
public:
    virtual std::unique_ptr<CtImportedNode> import_file(const fs::path& file) = 0;
    virtual std::string                     file_pattern_name() { return ""; }
    virtual std::vector<Glib::ustring>      file_patterns() { return {}; }
    virtual std::vector<Glib::ustring>      file_mime_types() { return {}; }
};

/// Implementation of file patterns for HTML importers
class CtHtmlImporterInterface: public CtImporterInterface
{
public:
    std::string                 file_pattern_name() override { return _("HTML Document"); }
#ifdef _WIN32
    std::vector<Glib::ustring>  file_patterns() override { return {"*.html", "*.htm"}; }
#else
    std::vector<Glib::ustring>  file_mime_types() override { return {"text/html"}; }
#endif
};

namespace CtImports {

std::vector<std::pair<size_t, size_t>> get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text);
std::unique_ptr<CtImportedNode> traverse_dir(const fs::path& dir, CtImporterInterface* importer);

} // namespace CtImports

struct CtStatusBar;

namespace CtXML {

xmlpp::Element* codebox_to_xml(xmlpp::Element* parent, const Glib::ustring& justification, int char_offset, int frame_width, int frame_height, int width_in_pixels, const Glib::ustring& syntax_highlighting, bool highlight_brackets, bool show_line_numbers);
xmlpp::Element* image_to_xml(xmlpp::Element* parent, const std::string& path, int char_offset, const Glib::ustring& justification, CtStatusBar* status_bar = nullptr);

} // namespace CtXML

/**
 * @class CtImportException
 * @brief Thrown when an exception occures during importing
 */
class CtImportException: public std::runtime_error {
public:
    explicit CtImportException(const std::string& msg) : std::runtime_error("[Import Exception]: " + msg) {}
};

class CtConfig;

// pygtk: HTMLHandler
class CtHtmlImport : public CtHtmlImporterInterface
{
public:
    CtHtmlImport(CtConfig* config);

    // virtuals of CtImporterInterface
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;

private:
    CtConfig* _config;
};

class CtTomboyImport : public CtImporterInterface
{
public:
    CtTomboyImport(CtConfig* config);

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;

private:
    void            _iterate_tomboy_note(xmlpp::Element* iter, std::unique_ptr<CtImportedNode>& node);
    xmlpp::Element* _rich_text_serialize(const Glib::ustring& text_data);

private:
    CtConfig*       _config;

    xmlpp::Element* _current_node;
    int             _chars_counter;
    bool            _is_list_item;
    bool            _is_link_to_node;
    std::map<std::string, std::string> _curr_attributes;
};

class CtZimParser;
/**
 * @brief Import handler for Zim Wiki directories
 * @class CtZimImport
 */
class CtZimImport : public CtImporterInterface
{
public:
    explicit CtZimImport(CtConfig* config);

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;

    ~CtZimImport();

private:
    std::unique_ptr<CtZimParser> _zim_parser;
};

class CtPlainTextImport : public CtImporterInterface
{
public:
    CtPlainTextImport(CtConfig*) {}

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;
    std::string                     file_pattern_name() override { return _("Plain Text Document"); }
#ifdef _WIN32
    std::vector<Glib::ustring>      file_patterns() override { return {"*.txt"}; }
#else
    std::vector<Glib::ustring>      file_mime_types() override { return {"text/*"}; }
#endif
};

class CtMDParser;
class CtMDImport : public CtImporterInterface
{
public:
    CtMDImport(CtConfig* config);

    ~CtMDImport();
public:
    // virtuals of CtImporterInterface
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;
    std::vector<Glib::ustring>        file_patterns() override { return {"*.md"}; };
    std::string                       file_pattern_name() override { return _("Markdown Document"); }
private:
    std::unique_ptr<CtMDParser> _parser;
};

class CtKeepnoteImport : public CtImporterInterface
{
public:
    explicit CtKeepnoteImport(CtConfig* config) : _config(config) {}
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;

private:
    CtConfig* _config;
};

class CtMempadImporter : public CtImporterInterface
{
public:
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;

    std::vector<Glib::ustring> file_patterns() override { return {"*.lst"}; };
    std::string file_pattern_name() override { return _("Mempad Document"); }
};

class CtTreepadImporter : public CtImporterInterface
{
public:
    std::unique_ptr<CtImportedNode> import_file(const fs::path& file) override;

    std::vector<Glib::ustring> file_patterns() override { return {"*.hjt"}; };
    std::string file_pattern_name() override { return _("Treepad Document"); }
};

class CtLeoImporter : public CtImporterInterface
{
public:

    std::unique_ptr<CtImportedNode> import_file(const fs::path& path) override;

    std::vector<Glib::ustring> file_patterns() override { return {"*.leo"}; };
    std::string file_pattern_name() override { return _("Leo Document"); }
};

class CtRedNotebookImporter : public CtHtmlImporterInterface
{
public:
    explicit CtRedNotebookImporter(CtConfig* config) : _ct_config{config} {}

    std::unique_ptr<CtImportedNode> import_file(const fs::path& path) override;

private:
    std::unique_ptr<CtImportedNode> _parse_input(const fs::path& path);

    CtConfig* _ct_config;
};

class CtNoteCaseHTMLImporter: public CtHtmlImporterInterface
{
public:
    explicit CtNoteCaseHTMLImporter(CtConfig* config) : _ct_config{config} {}

    std::unique_ptr<CtImportedNode> import_file(const fs::path& path) override;

private:

    CtConfig* _ct_config;
};
