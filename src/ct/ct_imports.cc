/*
 * ct_imports.cc
 *
 * Copyright 2009-2021
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

#include "ct_imports.h"
#include "ct_parser.h"
#include "ct_misc_utils.h"
#include "ct_main_win.h"
#include "ct_export2html.h"
#include "ct_logging.h"
#include <libxml2/libxml/SAX.h>
#include <fstream>
#include <sstream>

namespace {

xmlpp::Element* create_root_plaintext_text_el(xmlpp::Document& doc, const Glib::ustring& text) {
    xmlpp::Element* el = doc.create_root_node("root")->add_child("slot")->add_child("rich_text");
    el->set_child_text(text);
    return el;
}

} // namespace (anonymous)

namespace CtXML {

xmlpp::Element* codebox_to_xml(xmlpp::Element* parent, const Glib::ustring& justification, int char_offset, int frame_width, int frame_height, int width_in_pixels, const Glib::ustring& syntax_highlighting, bool highlight_brackets, bool show_line_numbers)
{
    xmlpp::Element* p_codebox_node = parent->add_child("codebox");
    p_codebox_node->set_attribute("char_offset", std::to_string(char_offset));
    p_codebox_node->set_attribute(CtConst::TAG_JUSTIFICATION, justification);
    p_codebox_node->set_attribute("frame_width", std::to_string(frame_width));
    p_codebox_node->set_attribute("frame_height", std::to_string(frame_height));
    p_codebox_node->set_attribute("width_in_pixels", std::to_string(width_in_pixels));
    p_codebox_node->set_attribute("syntax_highlighting", syntax_highlighting);
    p_codebox_node->set_attribute("highlight_brackets", std::to_string(highlight_brackets));
    p_codebox_node->set_attribute("show_line_numbers", std::to_string(show_line_numbers));
    return p_codebox_node;
}

void table_row_to_xml(const std::vector<std::string>& row, xmlpp::Element* parent)
{
    xmlpp::Element* row_element = parent->add_child("row");
    for (const auto& cell : row) {
        xmlpp::Element* cell_element = row_element->add_child("cell");
        cell_element->set_child_text(cell);
    }
}

xmlpp::Element *image_to_xml(xmlpp::Element *parent, const std::string &path, int char_offset, const Glib::ustring &justification, CtStatusBar* status_bar /* = nullptr */)
{
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    // Get uri type
    CtMiscUtil::URI_TYPE path_type = CtMiscUtil::get_uri_type(path);
    if (path_type == CtMiscUtil::URI_TYPE::UNKNOWN) {
        throw std::runtime_error(fmt::format("Could not determine type for path: {}", path));
    }
    if (path_type == CtMiscUtil::URI_TYPE::WEB_URL) {

        if (status_bar) {
            status_bar->update_status(std::string(_("Downloading")) + " " + path + " ...");
            while (gtk_events_pending()) gtk_main_iteration();
        }

        // Download
        try {
            std::string file_buffer = fs::download_file(path);
            if (!file_buffer.empty()) {
                Glib::RefPtr<Gdk::PixbufLoader> pixbuf_loader = Gdk::PixbufLoader::create();
                pixbuf_loader->write(reinterpret_cast<const guint8 *>(file_buffer.c_str()), file_buffer.size());
                pixbuf_loader->close();
                pixbuf = pixbuf_loader->get_pixbuf();

            }
        }
        catch (std::exception &e) {
            spdlog::error("Exception occurred while downloading image at url: '{}'; Message: {}", e.what());
            throw;
        }
    } else if (path_type == CtMiscUtil::URI_TYPE::LOCAL_FILEPATH) {

        // Load from local
        try {
            if (Glib::file_test(path, Glib::FILE_TEST_IS_REGULAR)) {
                pixbuf = Gdk::Pixbuf::create_from_file(path);
                if (!pixbuf) throw std::runtime_error("Failed to create pixbuf from file");
            }

        }
        catch (std::exception& e) {
            spdlog::error("Exception occured while loading image from disk: {}", e.what());
            throw;
        }
    } else {
        throw std::logic_error("Unknown uri in image_to_xml");
    }
    if (!pixbuf) throw std::runtime_error("pixbuf is invalid, this should not have happened");

    g_autofree gchar* pBuffer{NULL};
    gsize buffer_size;
    pixbuf->save_to_buffer(pBuffer, buffer_size, "png");
    const std::string rawBlob = std::string(pBuffer, buffer_size);
    const std::string encodedBlob = Glib::Base64::encode(rawBlob);

    xmlpp::Element* image_element = parent->add_child("encoded_png");
    image_element->set_attribute("char_offset", std::to_string(char_offset));
    image_element->set_attribute(CtConst::TAG_JUSTIFICATION, justification);
    image_element->set_attribute("link", std::string(CtConst::LINK_TYPE_WEBS) + " " + path);
    image_element->add_child_text(encodedBlob);

    if (status_bar) status_bar->update_status("");
    return image_element;
}

} // namespace CtXML

// Parse plain text for possible web links
std::vector<std::pair<size_t, size_t>> CtImports::get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text)
{
    std::vector<std::pair<size_t, size_t>> web_links;
    size_t max_end_offset = plain_text.size();
    if (max_end_offset < 7) {
        return web_links;
    }
    size_t max_start_offset = max_end_offset - 7;
    size_t start_offset = 0;
    unsigned lastCharBeforeURL = 0;
    while (start_offset < max_start_offset)
    {
        if (str::startswith_any(plain_text.substr(start_offset), CtConst::WEB_LINK_STARTERS))
        {
            size_t end_offset = start_offset + 3;
            unsigned closingParenthesisChar = 0;
            if (lastCharBeforeURL == '(') closingParenthesisChar = ')';
            else if (lastCharBeforeURL == '[') closingParenthesisChar = ']';
            else if (lastCharBeforeURL == '{') closingParenthesisChar = '}';
            while (end_offset < max_end_offset and
                   plain_text[end_offset] != ' ' and
                   plain_text[end_offset] != '\n' and
                   plain_text[end_offset] != closingParenthesisChar) {
                ++end_offset;
            }
            web_links.push_back(std::make_pair(start_offset, end_offset));
            start_offset = end_offset + 1;
        }
        else {
            lastCharBeforeURL = plain_text.at(start_offset);
            ++start_offset;
        }
    }
    return web_links;
}

std::unique_ptr<ct_imported_node> CtImports::traverse_dir(const fs::path& dir, CtImporterInterface* importer)
{
    auto dir_node = std::make_unique<ct_imported_node>(dir, dir.filename().string());
    for (const auto& dir_item: fs::get_dir_entries(dir))
    {
        if (fs::is_directory(dir_item))
        {
            if (auto node = traverse_dir(dir_item, importer))
                dir_node->children.emplace_back(std::move(node));
        }
        else if (auto node = importer->import_file(dir_item))
            dir_node->children.emplace_back(std::move(node));
    }

    // skip empty dirs
    if (dir_node->children.empty())
        return nullptr;

    // not the best place but
    // two cases:
    // 1. children with the same names, one with content and other as dir, join them
    // 2. dir contains  note with the same name, join them (from keepnote)

    std::function<void(std::unique_ptr<ct_imported_node>&)> join_subdir_subnote;
    join_subdir_subnote = [&](std::unique_ptr<ct_imported_node>& node) {
        for (auto iter1 = node->children.begin(); iter1 != node->children.end(); ++iter1)
        {
            if ((*iter1)->has_content() && (*iter1)->children.empty()) // node with content
            {
                for (auto iter2 = node->children.begin(); iter2 != node->children.end(); ++iter2)
                {
                    if (!(*iter2)->has_content()) // dir node
                    {
                        if (iter1->get() == iter2->get()) continue; // same node?
                        if ((*iter1)->node_name == (*iter2)->node_name)
                        {
                            std::swap((*iter1)->children, (*iter2)->children);
                            node->children.erase(iter2);
                            break;
                        }
                    }
                }
            }
        }
        for (auto& child: node->children)
            join_subdir_subnote(child);
    };

    std::function<void(std::unique_ptr<ct_imported_node>&)> join_parent_dir_subnote;
    join_parent_dir_subnote = [&](std::unique_ptr<ct_imported_node>& node) {
        if (!node->has_content())
        {
            for (auto iter = node->children.begin(); iter != node->children.end(); ++iter)
            {
                if ((*iter)->has_content() && (*iter)->children.empty() && node->node_name == (*iter)->node_name)
                {
                    node->copy_content((*iter));
                    node->children.erase(iter);
                    break;
                }
            }
        }
        for (auto& child: node->children)
            join_parent_dir_subnote(child);
    };

    join_subdir_subnote(dir_node);
    join_parent_dir_subnote(dir_node);


    return dir_node;
}

CtHtmlImport::CtHtmlImport(CtConfig* config) : _config(config)
{
}

std::unique_ptr<ct_imported_node> CtHtmlImport::import_file(const fs::path& file)
{
    if (file.extension() != ".html" && file.extension() != ".htm")
        return nullptr;

    std::ifstream infile;
    infile.exceptions(std::ios_base::failbit);
    infile.open(file.string());
    std::ostringstream ss;
    ss << infile.rdbuf();

    auto imported_node = std::make_unique<ct_imported_node>(file, file.stem().string());
    CtHtml2Xml html2xml(_config);
    html2xml.set_local_dir(file.parent_path().string());
    html2xml.set_outter_xml_doc(imported_node->xml_content.get());
    html2xml.feed(ss.str());

    return imported_node;
}

CtTomboyImport::CtTomboyImport(CtConfig* config) : _config(config)
{
}

std::unique_ptr<ct_imported_node> CtTomboyImport::import_file(const fs::path& file)
{
    xmlpp::DomParser tomboy_doc;
    try { tomboy_doc.parse_file(file.string());}
    catch (std::exception& ex) {
        spdlog::error("CtTomboyImport: cannot parse xml file ({}): {}", ex.what(), file);
        return nullptr;
    }

    // find note
    xmlpp::Node* note_el = tomboy_doc.get_document()->get_root_node()->get_first_child("note");

    // find note name
    Glib::ustring node_name = "???";
    if (xmlpp::Node* el = note_el->get_first_child("title")) {
        if (auto title_el = dynamic_cast<xmlpp::Element*>(el)->get_child_text()) {
            node_name = title_el->get_content();
            if (node_name.size() > 18 && str::endswith(node_name, " Notebook Template"))
                return nullptr;
        }
    }

    // find note's parent
    Glib::ustring parent_name;
    if (xmlpp::Node* tags_el = note_el->get_first_child("tags"))
        if (xmlpp::Node* tag_el = tags_el->get_first_child("tag")) {
            Glib::ustring tag_name = dynamic_cast<xmlpp::Element*>(tag_el)->get_child_text()->get_content();
            if (tag_name.size() > 16 && str::startswith(tag_name, "system:notebook:"))
                parent_name = tag_name.substr(16);
        }
    if (parent_name.empty())
        parent_name = "ORPHANS";

    // parse note's content
    if (xmlpp::Node* text_el = note_el->get_first_child("text"))
        if (xmlpp::Node* content_el = text_el->get_first_child("note-content"))
        {
            auto parent_node = std::make_unique<ct_imported_node>(file, parent_name);
            auto node = std::make_unique<ct_imported_node>(file, node_name);

            _current_node = node->xml_content->create_root_node("root")->add_child("slot");
            _curr_attributes.clear();
            _chars_counter = 0;
            _is_list_item = false;
            _is_link_to_node = false;
            _iterate_tomboy_note(dynamic_cast<xmlpp::Element*>(content_el), node);

            parent_node->children.emplace_back(std::move(node));
            return parent_node;
        }
    return nullptr;
}

void CtTomboyImport::_iterate_tomboy_note(xmlpp::Element* iter, std::unique_ptr<ct_imported_node>& node)
{
    for (auto dom_iter: iter->get_children())
    {
        auto dom_iter_el = dynamic_cast<xmlpp::Element*>(dom_iter);
        if (dom_iter->get_name() == "#text")
        {
            Glib::ustring text_data = dynamic_cast<xmlpp::TextNode*>(dom_iter)->get_content();
            if (_curr_attributes[CtConst::TAG_LINK] == "webs ")
                _curr_attributes[CtConst::TAG_LINK] += text_data;
            else if (_is_list_item)
                text_data = _config->charsListbul[0] + CtConst::CHAR_SPACE + text_data;

            xmlpp::Element* el = _rich_text_serialize(text_data);
             if (_is_link_to_node)
                node->add_broken_link(text_data, el);

            _chars_counter += text_data.size();
        }
        else if (dom_iter->get_name() == "bold") {
            _curr_attributes[CtConst::TAG_WEIGHT] = CtConst::TAG_PROP_VAL_HEAVY;
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_WEIGHT] = "";
        } else if (dom_iter->get_name() == CtConst::TAG_PROP_VAL_ITALIC) {
            _curr_attributes[CtConst::TAG_STYLE] = CtConst::TAG_PROP_VAL_ITALIC;
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_STYLE] = "";
        } else if (dom_iter->get_name() == CtConst::TAG_STRIKETHROUGH) {
            _curr_attributes[CtConst::TAG_STRIKETHROUGH] = CtConst::TAG_PROP_VAL_TRUE;
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_STRIKETHROUGH] = "";
        } else if (dom_iter->get_name() == "highlight") {
            _curr_attributes[CtConst::TAG_BACKGROUND] = CtConst::COLOR_48_YELLOW;
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_BACKGROUND] = "";
        } else if (dom_iter->get_name() == CtConst::TAG_PROP_VAL_MONOSPACE) {
            _curr_attributes[CtConst::TAG_FAMILY] = dom_iter->get_name();
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_FAMILY] = "";
        } else if (dom_iter->get_name() == "size:small") {
            _curr_attributes[CtConst::TAG_SCALE] = CtConst::TAG_PROP_VAL_SMALL;
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_SCALE] = "";
        } else if (dom_iter->get_name() == "size:large") {
            _curr_attributes[CtConst::TAG_SCALE] = CtConst::TAG_PROP_VAL_H2;
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_SCALE] = "";
        } else if (dom_iter->get_name() == "size:huge") {
            _curr_attributes[CtConst::TAG_SCALE] = CtConst::TAG_PROP_VAL_H1;
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_SCALE] = "";
        } else if (dom_iter->get_name() == "link:url") {
            _curr_attributes[CtConst::TAG_LINK] = "webs ";
            _iterate_tomboy_note(dom_iter_el, node);
            _curr_attributes[CtConst::TAG_LINK] = "";
        } else if (dom_iter->get_name() == "list-item") {
            _is_list_item = true;
            _iterate_tomboy_note(dom_iter_el, node);
            _is_list_item = false;
        } else if (dom_iter->get_name() == "link:internal") {
            _is_link_to_node = true;
            _iterate_tomboy_note(dom_iter_el, node);
            _is_link_to_node = false;
        } else {
            spdlog::debug(dom_iter->get_name());
            _iterate_tomboy_note(dom_iter_el, node);
        }
    }
}

xmlpp::Element* CtTomboyImport::_rich_text_serialize(const Glib::ustring& text_data)
{
    auto dom_iter = _current_node->add_child("rich_text");
    for (auto atr: _curr_attributes)
        if (!atr.second.empty())
            dom_iter->set_attribute(atr.first, atr.second);
    dom_iter->add_child_text(text_data);
    return dom_iter;
}

CtZimImport::CtZimImport(CtConfig* config) : _zim_parser{std::make_unique<CtZimParser>(config)} {}

std::unique_ptr<ct_imported_node> CtZimImport::import_file(const fs::path& file)
{
    if (file.extension() != ".txt") return nullptr;

    _ensure_notebook_file_in_dir(file.parent_path());

    std::unique_ptr<ct_imported_node> node = std::make_unique<ct_imported_node>(file, file.stem().string());

    std::ifstream stream;
    stream.exceptions(std::ios::failbit);
    stream.open(file.string());

    _zim_parser->wipe_doc();
    _zim_parser->feed(stream);

    node->xml_content = _zim_parser->doc().document();
    node->content_broken_links = _zim_parser->doc().broken_links();

    return node;
}

CtZimImport::~CtZimImport() = default;

void CtZimImport::_ensure_notebook_file_in_dir(const fs::path& dir)
{
    if (_has_notebook_file) return;
    for (auto dir_item: fs::get_dir_entries(dir))
        if (dir_item.filename() == "notebook.zim")
        {
            _has_notebook_file = true;
            break;
        }

    if (!_has_notebook_file) {
        throw CtImportException(fmt::format("Directory: {} does not contain a notebook.zim file", dir));
    }
}

CtMDImport::~CtMDImport() = default;

std::unique_ptr<ct_imported_node> CtPlainTextImport::import_file(const fs::path& file)
{
    if (!CtMiscUtil::mime_type_contains(file.string(), "text/"))
        return nullptr;

    try
    {
        std::ifstream infile;
        infile.exceptions(std::ios_base::failbit);
        infile.open(file.string());
        std::ostringstream data;
        data << infile.rdbuf();
        std::string converted = data.str();
        const std::string codeset = CtStrUtil::get_encoding(converted.c_str(), converted.size());
        if (CtStrUtil::is_codeset_not_utf8(codeset)) {
            converted = Glib::convert_with_fallback(converted, "UTF-8", codeset);
        }
        std::unique_ptr<ct_imported_node> node = std::make_unique<ct_imported_node>(file, file.stem().string());
        node->xml_content->create_root_node("root")->add_child("slot")->add_child("rich_text")->add_child_text(converted);
        node->node_syntax = CtConst::PLAIN_TEXT_ID;
        return node;
    }
    catch (std::exception& ex)
    {
        spdlog::error("CtPlainTextImport, what: , file: {}", ex.what(), file);
    }
    return nullptr;
}

CtMDImport::CtMDImport(CtConfig* config) : _parser{std::make_unique<CtMDParser>(config)}
{
}

std::unique_ptr<ct_imported_node> CtMDImport::import_file(const fs::path& file)
{
    if (file.extension() != ".md")
        return nullptr;

    std::ifstream infile(file.string());
    if (!infile) throw std::runtime_error(fmt::format("CtMDImport: cannot open file, what: {}, file: {}", strerror(errno), file));
    _parser->wipe_doc();
    _parser->feed(infile);

    std::unique_ptr<ct_imported_node> node = std::make_unique<ct_imported_node>(file, file.stem().string());
    node->xml_content = _parser->doc().document();

    return node;
}

CtPandocImport::CtPandocImport(CtConfig* config): _config(config)
{
}

std::unique_ptr<ct_imported_node> CtPandocImport::import_file(const fs::path& file)
{
    std::stringstream html_buff;
    CtPandoc::to_html(file, html_buff);

    std::unique_ptr<ct_imported_node> node = std::make_unique<ct_imported_node>(file, file.stem().string());

    CtHtml2Xml parser(_config);
    parser.set_outter_xml_doc(node->xml_content.get());
    parser.feed(html_buff.str());

    return node;
}

std::unique_ptr<ct_imported_node> CtKeepnoteImport::import_file(const fs::path& file)
{
    for (auto ignore: {"__TRASH__", "__NOTEBOOK__"})
        if (file.string().find(ignore) != std::string::npos)
            return nullptr;
    if (file.filename().string() != "page.html")
        return nullptr;

    std::ifstream infile;
    infile.exceptions(std::ios::failbit);
    infile.open(file.string());

    std::ostringstream buff;
    buff << infile.rdbuf();

    CtHtml2Xml parser(_config);
    parser.set_local_dir(file.parent_path().string());
    parser.feed(buff.str());

    auto node = std::make_unique<ct_imported_node>(file, file.parent_path().stem().string());
    node->xml_content->create_root_node_by_import(parser.doc().get_root_node());
    spdlog::debug(buff.str());
    spdlog::debug(node->xml_content->write_to_string());

    return node;
}

namespace {

std::unique_ptr<ct_imported_node> mempad_page_to_node(const CtMempadParser::page& page, const fs::path& path)
{
    auto node = std::make_unique<ct_imported_node>(path, page.name);
    auto& doc = node->xml_content;
    create_root_plaintext_text_el(*doc, page.contents);

    return node;
}

template<typename ITER>
ITER up_to_same_level(ITER start, ITER upper_bound, int level)
{
    while (start != upper_bound) {
        if (start->level == level) {
            return start;
        }
        ++start;
    }
    return start;
}

std::unique_ptr<ct_imported_node> mempad_pages_to_nodes(const CtMempadParser::page& page,
                                                        const std::vector<CtMempadParser::page>& child_pages,
                                                        const fs::path& path)
{
    auto node = mempad_page_to_node(page, path);
    for (auto iter = child_pages.begin(); iter != child_pages.end(); ++iter) {
        if (iter->level == page.level + 1) {
            // Direct child
            auto last_child_child = up_to_same_level(iter + 1, child_pages.end(), iter->level);
            std::vector<CtMempadParser::page> child_children;
            if (iter + 1 != last_child_child) {
                child_children.insert(child_children.end(), iter + 1, last_child_child);
            }
            node->children.emplace_back(mempad_pages_to_nodes(*iter, child_children, path));
        }
    }
    return node;
}

std::unique_ptr<ct_imported_node> mempad_tree_to_node(const std::vector<CtMempadParser::page>& pages, const fs::path& path)
{
    CtMempadParser::page dummy_page{
        .level = 0,
        .name = "Root",
        .contents = ""
    };
    auto node = mempad_pages_to_nodes(dummy_page, pages, path);

    return node;
}

} // namespace (anonymous)

std::unique_ptr<ct_imported_node> CtMempadImporter::import_file(const fs::path& file)
{
    std::ifstream infile{file.string()};

    CtMempadParser parser;

    parser.feed(infile);

    std::vector<CtMempadParser::page> pages = parser.parsed_pages();

    auto node = mempad_tree_to_node(pages, file);

    return node;
}

std::unique_ptr<ct_imported_node> CtTreepadImporter::import_file(const fs::path& file)
{
    std::ifstream infile{file.string()};

    CtTreepadParser parser;

    parser.feed(infile);

    std::vector<CtMempadParser::page> pages = parser.parsed_pages();

    auto node = mempad_tree_to_node(pages, file);

    return node;
}

namespace {

std::unique_ptr<ct_imported_node> to_ct_node(const CtLeoParser::leo_node& leo_node, const fs::path& path) {
    auto node = std::make_unique<ct_imported_node>(path, leo_node.name);
    node->xml_content = std::make_shared<xmlpp::Document>();
    create_root_plaintext_text_el(*node->xml_content, leo_node.content);

    for (const auto& l_node : leo_node.children) {
        node->children.emplace_back(to_ct_node(l_node, path));
    }

    return node;
}

std::unique_ptr<ct_imported_node> generate_leo_root_node(std::vector<CtLeoParser::leo_node> leo_nodes, const fs::path& path) {
    CtLeoParser::leo_node dummy_node;
    dummy_node.name = "Leo Root";
    dummy_node.children = std::move(leo_nodes);

    return to_ct_node(dummy_node, path);
}

std::unique_ptr<ct_imported_node> node_to_ct_node(const CtRedNotebookParser::node& node, const fs::path& path)
{
    auto ct_node = std::make_unique<ct_imported_node>(path, node.name);

    if (!node.doc) {
        ct_node->xml_content = std::make_shared<xmlpp::Document>();
        create_root_plaintext_text_el(*ct_node->xml_content, "");
    } else {
        ct_node->xml_content = node.doc;
    }

    return ct_node;
}

std::unique_ptr<ct_imported_node> generate_ct_node_hierarchy(std::string&& root_name, const std::vector<CtRedNotebookParser::node>& nodes, const fs::path& path)
{
    CtRedNotebookParser::node root { std::move(root_name) };
    auto ct_root = node_to_ct_node(root, path);

    for (const auto& node : nodes) {
        ct_root->children.emplace_back(node_to_ct_node(node, path));
    }
    return ct_root;
}

} // namespace (anonymous)

std::unique_ptr<ct_imported_node> CtLeoImporter::import_file(const fs::path& path)
{
    std::ifstream in{path.string()};
    if (!in) {
        throw std::runtime_error(fmt::format("Failed to initalise input file, path: <{}>", path));
    }

    CtLeoParser parser;
    parser.feed(in);

    return generate_leo_root_node(parser.nodes(), path);
}

std::unique_ptr<ct_imported_node> CtRedNotebookImporter::import_file(const fs::path& path)
{
    std::ifstream in{path.string()};
    if (!in) {
        throw std::runtime_error("Failed to initalise import file");
    }

    return _parse_input(in, path);
}

std::unique_ptr<ct_imported_node> CtRedNotebookImporter::_parse_input(std::ifstream& infile, const fs::path& path)
{
    CtRedNotebookParser p{_ct_config};

    p.feed(infile);
    const auto& nodes = p.nodes();

    return generate_ct_node_hierarchy("RedNotebook Root", nodes, path);
}

std::unique_ptr<ct_imported_node> CtNoteCaseHTMLImporter::import_file(const fs::path& path)
{
    std::ifstream in{path.string()};
    if (!in) {
        throw std::runtime_error("Failed to setup input file for reading");
    }

    CtNoteCaseHTMLParser parser{_ct_config};
    parser.feed(in);
    return generate_ct_node_hierarchy("NoteCase Root", parser.nodes(), path);
}
