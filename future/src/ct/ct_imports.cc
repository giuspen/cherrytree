/*
 * ct_imports.cc
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

#include "ct_imports.h"
#include "ct_misc_utils.h"
#include "ct_main_win.h"
#include "ct_export2html.h"
#include "ct_logging.h"
#include <libxml2/libxml/SAX.h>
#include <fstream>
#include <sstream>

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

xmlpp::Element* table_to_xml(const std::vector<std::vector<std::string>>& matrix, xmlpp::Element* parent, int char_offset, Glib::ustring justification, int col_min, int col_max)
{
    xmlpp::Element* tbl_node = parent->add_child("table");
    tbl_node->set_attribute("char_offset", std::to_string(char_offset));
    tbl_node->set_attribute(CtConst::TAG_JUSTIFICATION, justification);
    tbl_node->set_attribute("col_min", std::to_string(col_min));
    tbl_node->set_attribute("col_max", std::to_string(col_max));
    
    // Header goes at end
    for (auto row_iter = matrix.cbegin() + 1; row_iter != matrix.cend(); ++row_iter) {
        table_row_to_xml(*row_iter, tbl_node);
    }
    table_row_to_xml(matrix.front(), tbl_node);
    
    return tbl_node;
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
    image_element->set_attribute("link", path);
    image_element->add_child_text(encodedBlob);
    
    if (status_bar) status_bar->update_status("");
    return image_element;
}
}


const std::set<std::string> CtHtml2Xml::HTML_A_TAGS{"p", "b", "i", "u", "s", CtConst::TAG_PROP_VAL_H1,
            CtConst::TAG_PROP_VAL_H2, CtConst::TAG_PROP_VAL_H3, "span", "font"};

// Parse plain text for possible web links
std::vector<std::pair<int, int>> CtImports::get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text)
{
    std::vector<std::pair<int, int>> web_links;
    int max_end_offset = (int)plain_text.size();
    int max_start_offset = max_end_offset - 7;
    int start_offset = 0;
    while (start_offset < max_start_offset)
    {
        if (str::startswith_any(plain_text.substr(start_offset), CtConst::WEB_LINK_STARTERS))
        {
            int end_offset = start_offset + 3;
            while (end_offset < max_end_offset
                   && plain_text[(size_t)end_offset] != g_utf8_get_char(CtConst::CHAR_SPACE)
                   && plain_text[(size_t)end_offset] != g_utf8_get_char(CtConst::CHAR_NEWLINE))
                end_offset += 1;
            web_links.push_back(std::make_pair(start_offset, end_offset));
            start_offset = end_offset + 1;
        }
        else
            start_offset += 1;
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
    // if there are node (dir) with subnodes  and node with content, both with the same name, join them
    for (auto child_it = dir_node->children.begin(); child_it != dir_node->children.end(); ++child_it)
    {
        if ((*child_it)->has_content() && (*child_it)->children.empty()) // node with content
        {
            for (auto dir_it = dir_node->children.begin(); dir_it != dir_node->children.end(); ++dir_it)
            {
                if (!(*dir_it)->has_content()) // dir node
                {
                    if (child_it->get() == dir_it->get()) continue;
                    if ((*child_it)->node_name == (*dir_it)->node_name)
                    {
                        std::swap((*child_it)->children, (*dir_it)->children);
                        dir_node->children.erase(dir_it);
                        break;
                    }
                }
            }
        }
    }


    return dir_node;
}



CtHtml2Xml::CtHtml2Xml(CtConfig* config) : _config(config)
{

}

void CtHtml2Xml::feed(const std::string& html)
{
    _xml_doc = _outter_doc ? _outter_doc : &_temp_doc;

    _state = ParserState::WAIT_BODY;
    _tag_id_generator = 0;
    _tag_styles.clear();
    _html_pre_tag_open = false;
    _html_td_tag_open = false;
    _parsing_valid_tag = true;
    _html_a_tag_counter = 0;
    _list_type = 'u';
    _list_num = 0;
    _list_level = -1;
    _table.clear();

    _slot_root = _xml_doc->create_root_node("root")->add_child("slot");
    _char_offset = 0;
    _slot_text = "";
    _slot_style_id = -1;
    _slot_styles_cache.clear();

    if (str::startswith(html, "<!doctype html>"))
        CtHtmlParser::feed(html);
    else {
        // if not fixed, we can skip some items
        std::string fixed_html = "<!doctype html><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"</head><body>"
                + html + "</body></html>";
        CtHtmlParser::feed(fixed_html);
    }


    _rich_text_save_pending();
}

void CtHtml2Xml::handle_starttag(std::string_view tag, const char** atts)
{
    _start_adding_tag_styles();

    if (vec::exists(CtConst::INVALID_HTML_TAGS, tag)) {
        _parsing_valid_tag = false;
        return;
    }
    _parsing_valid_tag = true;
    
    if (HTML_A_TAGS.count(tag.begin())) _html_a_tag_counter += 1;
    if (_state == ParserState::WAIT_BODY)
    {
        if (tag == "body") {
            _state = ParserState::PARSING_BODY;
        }
    }
    else if (_state == ParserState::PARSING_BODY)
    {
        if (tag == "table")
        {
            _table.clear();
            _html_td_tag_open = false;
            _state = ParserState::PARSING_TABLE;
        }
        else if (tag == "strong") _add_tag_style(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
        else if (tag == "b") _add_tag_style(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
        else if (tag == "i") _add_tag_style(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
        else if (tag == "em") _add_tag_style(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
        else if (tag == "u") _add_tag_style(CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE);
        else if (tag == "s") _add_tag_style(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
        else if (tag == CtConst::TAG_STYLE) _state = ParserState::WAIT_BODY;
        else if (tag == "span")
        {
            for (auto& tag_attr: char2list_attrs(atts))
            {
                if (tag_attr.name != CtConst::TAG_STYLE)
                    continue;
                for (Glib::ustring& style_attribute: str::split(Glib::ustring(tag_attr.value.begin()), ";"))
                {
                    int colon_pos = str::indexOf(style_attribute, CtConst::CHAR_COLON);
                    if (colon_pos < 0) continue;
                    auto attr_name = str::trim(style_attribute.substr(0, colon_pos).lowercase());
                    Glib::ustring attr_value = str::trim(style_attribute.substr(colon_pos + 1, style_attribute.size() - colon_pos).lowercase());
                    if (attr_name == "color") {
                        auto color = _convert_html_color(attr_value);
                        if (!color.empty())
                            _add_tag_style(CtConst::TAG_FOREGROUND, color);
                    } else if (attr_name == CtConst::TAG_BACKGROUND || attr_name == "background-color") {
                        auto color = _convert_html_color(attr_value);
                        if (!color.empty())
                            _add_tag_style(CtConst::TAG_BACKGROUND, color);
                    } else if (attr_name == "text-decoration") {
                        if (attr_value == CtConst::TAG_UNDERLINE || str::startswith(attr_value, "underline"))
                            _add_tag_style(CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE);
                        else if (attr_value == "line-through")
                            _add_tag_style(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
                    } else if (attr_name == "font-weight") {
                        if (attr_value == "bold" || attr_value == "bolder" || attr_value == "700")
                            _add_tag_style(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
                    } else if (attr_name == "font-style") {
                        if (attr_value == CtConst::TAG_PROP_VAL_ITALIC)
                            _add_tag_style(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
                    } else if (attr_name == "font-size") {
                        try
                        {
                            attr_value = str::replace(attr_value, "pt", "");
                            // Can throw std::invalid_argument or std::out_of_range
                            int font_size = std::stoi(attr_value, nullptr); 
                            if (font_size > 0 && font_size < 11)
                                _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SMALL);
                            else if (font_size > 13 && font_size < 19)
                                _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3);
                            else if (font_size >= 19)
                                _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H2);

                        } catch (std::invalid_argument&) {}
                    }
                }
            }
        }
        else if (tag == "font")
        {
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "color") {
                    auto color = _convert_html_color(str::trim(Glib::ustring(tag_attr.value.begin())));
                    if (color != "")
                        _add_tag_style(CtConst::TAG_FOREGROUND, color);
                }
            }
        }
        else if (tag == "p")
        {
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "align")
                    _add_tag_style(CtConst::TAG_JUSTIFICATION, str::trim(Glib::ustring(tag_attr.value.begin()).lowercase()));
            }
        }
        else if (tag == CtConst::TAG_PROP_VAL_SUP || tag == CtConst::TAG_PROP_VAL_SUB)
        {
            _add_tag_style(CtConst::TAG_SCALE, tag.begin());
        }
        else if (tag  == CtConst::TAG_PROP_VAL_H1 || tag == CtConst::TAG_PROP_VAL_H2  || tag == CtConst::TAG_PROP_VAL_H3
                 || tag == CtConst::TAG_PROP_VAL_H4  || tag == CtConst::TAG_PROP_VAL_H5  || tag == CtConst::TAG_PROP_VAL_H6)
        {
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
            if (tag == CtConst::TAG_PROP_VAL_H1 || tag == CtConst::TAG_PROP_VAL_H2) _add_tag_style(CtConst::TAG_SCALE, tag.begin());
            else _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3);
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "align")
                    _add_tag_style(CtConst::TAG_JUSTIFICATION, str::trim(Glib::ustring(tag_attr.value.begin()).lowercase()));
            }
        }
        else if (tag == "a")
        {
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "href" && tag_attr.value.size() > 7)
                    _add_tag_style(CtConst::TAG_LINK, CtStrUtil::get_internal_link_from_http_url(tag_attr.value.begin()));
            }
        }
        else if (tag == "br") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "ol")  { _list_type = 'o'; _list_num = 1; }
        else if (tag == "ul")  { 
            _list_type = 'u'; 
            _list_num = 0;
            if (_list_level < static_cast<int>(_config->charsListbul.size()) - 1) _list_level++;
        }
        else if (tag == "li") {
            if (_list_type == 'u') {
                if (_list_level < 0) {
                    // A ul _should_ have appeared before this
                    throw std::runtime_error("List item appeared before list declaration");
                }
                _rich_text_serialize(_config->charsListbul[_list_level] + CtConst::CHAR_SPACE);

            }
            else {
                _rich_text_serialize(std::to_string(_list_num) + ". ");
                _list_num += 1;
            }
        }
        else if (tag  == "img" || tag == "v:imagedata") {
            for (auto& tag_attr: char2list_attrs(atts))
                if (tag_attr.name == "src")
                    _insert_image(tag_attr.value.begin(), "");
        }
        else if (tag == "pre") _html_pre_tag_open = true;
        else if (tag == "code") _add_tag_style(CtConst::TAG_FAMILY, CtConst::TAG_PROP_VAL_MONOSPACE);
        else if (tag == "dt") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "dd") _rich_text_serialize(CtConst::CHAR_NEWLINE + Glib::ustring(CtConst::CHAR_TAB));
    }
    else if (_state == ParserState::PARSING_TABLE)
    {
        if (tag == "div") { // table is used as layout
            if (_table.empty()) {
                _table.clear();
                _html_td_tag_open = false;
                _state = ParserState::PARSING_BODY;
            }
        }
        else if (tag == "table") { // nested tables
            _table.clear();
            _html_td_tag_open = false;
        }
        else if (tag == "tr") {
            _table.push_back(std::list<table_cell>());
            _html_td_tag_open = false;
        }
        else if (tag == "td" || tag == "th") {
            _html_td_tag_open = true;
            if (_table.empty()) // case of first missing <tr>, this is the header even if <td>
                _table.push_back(std::list<table_cell>());
            int rowspan = 1;
            for (auto& tag_attr: char2list_attrs(atts))
                if (tag_attr.name == "rowspan")
                    rowspan = std::atoi(tag_attr.value.begin());
            _table.back().push_back(table_cell{rowspan, ""});
        }
        else if (tag == "img" || tag == "v:imagedata") {
            for (auto& tag_attr: char2list_attrs(atts))
                if (tag_attr.name == "src")
                    _insert_image(tag_attr.value.begin(), str::repeat(CtConst::CHAR_NEWLINE, 2));
        }
        else if (tag == "br" && _html_td_tag_open) _table.back().back().text += CtConst::CHAR_NEWLINE;
        else if (tag == "ol" && _html_td_tag_open) { _list_type = 'o'; _list_num = 1; }
        else if (tag == "ul" && _html_td_tag_open) { _list_type = 'u'; _list_num = 0; }
        else if (tag == "li" && _html_td_tag_open) {
            if (_list_type == 'u') _table.back().back().text += _config->charsListbul[0] + CtConst::CHAR_SPACE;
            else {
                _table.back().back().text += std::to_string(_list_num) + ". ";
                _list_num += 1;
            }
        }
    }
    _end_adding_tag_styles();
}

void CtHtml2Xml::handle_endtag(std::string_view tag)
{
    _pop_tag_styles();
    if (HTML_A_TAGS.count(tag.begin())) _html_a_tag_counter -= 1;
    if (_state == ParserState::WAIT_BODY)
    {
        if (tag == CtConst::TAG_STYLE)
            _state = ParserState::PARSING_BODY;
    }
    else if (_state == ParserState::PARSING_BODY)
    {
        if (tag == "p") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "div") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "pre") _html_pre_tag_open = false;
        else if (tag == CtConst::TAG_PROP_VAL_H1 || tag == CtConst::TAG_PROP_VAL_H2 || tag == CtConst::TAG_PROP_VAL_H3
                 || tag == CtConst::TAG_PROP_VAL_H4 || tag == CtConst::TAG_PROP_VAL_H5 || tag == CtConst::TAG_PROP_VAL_H6) {
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
        }
        else if (tag == "li") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "ul") {
            // Move back up a list level
            if (_list_level > 0) _list_level--;
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
        }
    }
    else if (_state == ParserState::PARSING_TABLE)
    {
        if (tag == "p" || tag == "li")
        {
            if (_html_td_tag_open)
                _table.back().back().text += CtConst::CHAR_NEWLINE;
        }
        else if (tag == "td" || tag == "th")
        {
            _html_td_tag_open = false;
        }
        else if (tag == "table")
        {
            _state = ParserState::PARSING_BODY;
            if (_table.size() && _table.back().size() == 0) // case of latest <tr> without any <tr> afterwards
                _table.pop_back();
            if (_table.size() == 1 && _table.back().size() == 1) // it's a codebox
                _insert_codebox();
            else // it's a table
                _insert_table();
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
        }
    }
}

void CtHtml2Xml::handle_data(std::string_view text)
{
    if (_state == ParserState::WAIT_BODY || !_parsing_valid_tag)
        return;
    if (_html_pre_tag_open) {
         _rich_text_serialize(text.begin());
         return;
    }
    Glib::ustring clean_data(text.begin());
    if (_html_a_tag_counter > 0) clean_data = str::replace(clean_data, CtConst::CHAR_NEWLINE, CtConst::CHAR_SPACE);
    else                         clean_data = str::replace(clean_data, CtConst::CHAR_NEWLINE, "");
    if (clean_data.empty() || clean_data == CtConst::CHAR_TAB)
        return;
    clean_data = str::replace(clean_data, "\x20", CtConst::CHAR_SPACE); // replace non-breaking space
    // not a good idea, if it's UTF-16, it should be converted
    // clean_data = str::replace(clean_data, "\xfeff", "");
    if (_state == ParserState::PARSING_BODY) {
        clean_data = str::replace(clean_data, CtConst::CHAR_TAB, CtConst::CHAR_SPACE);
        _rich_text_serialize(clean_data);
    }
    if (_state == ParserState::PARSING_TABLE && _html_td_tag_open) {
        clean_data = str::replace(clean_data, CtConst::CHAR_TAB, "");
        _table.back().back().text += clean_data;
    }
}

// Found Entity Reference like &name;
void CtHtml2Xml::handle_charref(std::string_view /*name*/)
{
    // todo: test it
}

void CtHtml2Xml::set_status_bar(CtStatusBar* status_bar)
{
    _status_bar = status_bar;
}

void CtHtml2Xml::_start_adding_tag_styles()
{
    // every tag will have style, even if it's empty
    _tag_styles.push_back(tag_style{++_tag_id_generator, "", ""});
}

void CtHtml2Xml::_add_tag_style(const std::string& style, const std::string& value)
{
    auto& current = _tag_styles.back();
    if (current.style.empty()) {
        current.style = style;
        current.value = value;
    } else {
        _tag_styles.push_back(tag_style{current.tag_id, style, value});
    }
}

void CtHtml2Xml::_end_adding_tag_styles()
{
    // actually, nothing to do here
}

void CtHtml2Xml::_pop_tag_styles()
{
    // every tag has at least one style
    int tag_id = _tag_styles.back().tag_id;
    while (_tag_styles.back().tag_id == tag_id)
        _tag_styles.pop_back();
}

int CtHtml2Xml::_get_tag_style_id()
{
    // use tag id as style id because they are unique
    for (auto tag_style = _tag_styles.rbegin(); tag_style != _tag_styles.rend(); ++tag_style)
        if (!tag_style->style.empty()) // skip empty because they don't matter
            return tag_style->tag_id;
    return 0;
}

void CtHtml2Xml::_put_tag_styles_on_top_cache()
{
    int current_tag_style = _get_tag_style_id();

    // check cache first if it exist move it on top
    auto exists_in_cache = [&]() {
        for (auto& style: _slot_styles_cache)
            if (style.slot_style_id == current_tag_style)
                return true;
        return false;
    };
    if (exists_in_cache())
    {
        // remove others, they won't need
        while (_slot_styles_cache.front().slot_style_id != current_tag_style)
            _slot_styles_cache.pop_front();
        return;
    }

    // or create and put in cache top
    _slot_styles_cache.push_front(slot_styles());
    auto& style = _slot_styles_cache.front();
    style.slot_style_id = current_tag_style;
    for (auto& tag_style: _tag_styles)
        if (!tag_style.style.empty())
            style.styles[tag_style.style] = tag_style.value;

    // clean up cache
    if (_slot_styles_cache.size() > 10)
        _slot_styles_cache.pop_back();
}

std::string CtHtml2Xml::_convert_html_color(const std::string& html_color)
{
    Gdk::RGBA rgba;
    if (!rgba.set(html_color))
        return "";
    // r+g+b black is 0
    // r+g+b white is 3*1.0 = 3.0
    double black_level = 3.0 / 100. * 15;    // lower is black-ish
    double white_level = 3.0 - black_level;  // higher is white-ish
    double sum_rgb = rgba.get_blue() + rgba.get_green() + rgba.get_red();
    if (sum_rgb < black_level || sum_rgb > white_level)
        return "";
    return CtRgbUtil::rgb_any_to_24(rgba);
}

// Insert Image in Buffer
void CtHtml2Xml::_insert_image(std::string img_path, std::string trailing_chars)
{
    _rich_text_save_pending();

    // todo: remove this copy-paste (image.cc)
    auto insert_image = [&](Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
        g_autofree gchar* pBuffer{NULL};
        gsize buffer_size;
        pixbuf->save_to_buffer(pBuffer, buffer_size, "png");
        const std::string rawBlob = std::string(pBuffer, buffer_size);
        const std::string encodedBlob = Glib::Base64::encode(rawBlob);

        xmlpp::Element* p_image_node = _slot_root->add_child("encoded_png");
        p_image_node->set_attribute("char_offset", std::to_string(_char_offset));
        p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT);
        p_image_node->set_attribute("link", img_path);
        p_image_node->add_child_text(encodedBlob);
    };

    if (_status_bar) {
        _status_bar->update_status(std::string(_("Downloading")) + " " + img_path + " ...");
       while (gtk_events_pending()) gtk_main_iteration();
    }

    bool image_good = false;

    // trying to download
    try {
        std::string file_buffer = fs::download_file(img_path);
        if (!file_buffer.empty()) {
            Glib::RefPtr<Gdk::PixbufLoader> pixbuf_loader = Gdk::PixbufLoader::create();
            pixbuf_loader->write((const guint8*)file_buffer.c_str(), file_buffer.size());
            pixbuf_loader->close();
            auto pixbuf = pixbuf_loader->get_pixbuf();
            insert_image(pixbuf);
            image_good = true;
        }
    }  catch (...) { }

    // trying to load from disk
    try {
        if (!image_good) {
            std::string local_image = Glib::build_filename(_local_dir, img_path);
            if (Glib::file_test(local_image, Glib::FILE_TEST_IS_REGULAR)) {
                Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(local_image);
                if (pixbuf) {
                    insert_image(pixbuf);
                    image_good = true;
                }
            }
        }
    }
    catch (...) {}

    if (image_good) {
        _char_offset += 1;
        if (!trailing_chars.empty())
            _rich_text_serialize(trailing_chars);
    } else {
        spdlog::error("Failed to download {}", img_path);
    }

    if (_status_bar)
        _status_bar->update_status("");
}

void CtHtml2Xml::_insert_table()
{
    _rich_text_save_pending();

    // add more cells for rowspan > 1
    for (auto& row: _table)
        for (auto iter = row.begin(); iter != row.end(); ++ iter)
            if (iter->rowspan > 1)
                row.insert(std::next(iter), iter->rowspan - 1, {1, ""});
    // find bigger row size
    size_t row_len = 0;
    for (auto& row: _table)
        row_len  = std::max(row_len, row.size());
    // add more cell for rowspan = 0
    for (auto& row: _table)
        for (auto iter = row.begin(); iter != row.end(); ++ iter)
            if (iter->rowspan == 0 && row.size() < row_len)
                row.insert(std::next(iter), row_len - row.size(), {1, ""});
    // add more cell just in case
    for (auto& row: _table)
        if (row.size() < row_len)
            row.insert(row.end(), row_len - row.size(), {1, ""});

    // todo: remove this copy-paste (table.cc)
    xmlpp::Element* p_table_node = _slot_root->add_child("table");
    p_table_node->set_attribute("char_offset", std::to_string(_char_offset));
    p_table_node->set_attribute(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT);
    p_table_node->set_attribute("col_min", std::to_string(40));
    p_table_node->set_attribute("col_max", std::to_string(400));


    auto row_to_xml = [&](const std::list<table_cell>& row) {
        xmlpp::Element* p_row_node = p_table_node->add_child("row");
        for (const auto& cell: row)
        {
            xmlpp::Element* p_cell_node = p_row_node->add_child("cell");
            p_cell_node->add_child_text(str::trim(cell.text));
        }
    };
    // put header at the end
    bool is_header = true;
    for (const auto& row: _table)
    {
        if (is_header) { is_header = false; continue; }
        row_to_xml(row);
    }
    row_to_xml(_table.front());

    _char_offset += 1;
}

void CtHtml2Xml::_insert_codebox()
{
    _rich_text_save_pending();

    // todo: fix this copy-paste from codebox.cc
    xmlpp::Element* p_codebox_node = _slot_root->add_child("codebox");
    p_codebox_node->set_attribute("char_offset", std::to_string(_char_offset));
    p_codebox_node->set_attribute(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT);
    p_codebox_node->set_attribute("frame_width", std::to_string(300));
    p_codebox_node->set_attribute("frame_height", std::to_string(150));
    p_codebox_node->set_attribute("width_in_pixels", std::to_string(true));
    p_codebox_node->set_attribute("syntax_highlighting", CtConst::PLAIN_TEXT_ID);
    p_codebox_node->set_attribute("highlight_brackets", std::to_string(false));
    p_codebox_node->set_attribute("show_line_numbers", std::to_string(false));
    p_codebox_node->add_child_text(str::trim(_table.back().back().text));

    _char_offset += 1;
}

// Appends a new part to the XML rich text
void CtHtml2Xml::_rich_text_serialize(std::string text)
{
    if (text.empty()) return;
    int current_tag_style_id = _get_tag_style_id();

    // fist time -> put styles on cache top
    if (_slot_style_id == -1) {
        _put_tag_styles_on_top_cache();
        _slot_style_id = current_tag_style_id;
    }
    // same style, text in the same slot
    if (_slot_style_id == current_tag_style_id)
    {
        _slot_text += text;
        return;
    }
    // styles changed, so
    // create slot with prevous text
    _rich_text_save_pending();

    //
    _put_tag_styles_on_top_cache();
    _slot_text = text;
    _slot_style_id = current_tag_style_id;
}

void CtHtml2Xml::_rich_text_save_pending()
{
    // the style is always on cache top
    if (_slot_text != "")
    {
        auto& s_style = _slot_styles_cache.front();

        xmlpp::Element* s = _slot_root->add_child("rich_text");
        for (auto& attr: s_style.styles)
            s->set_attribute(attr.first, attr.second);
        s->set_child_text(_slot_text);
        _char_offset += _slot_text.size();
    }

    _slot_text = "";
    _slot_style_id = -1;
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
    html2xml.set_outter_xml_doc(&imported_node->xml_content);
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

            _current_node = node->xml_content.create_root_node("root")->add_child("slot");
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





CtZimImport::CtZimImport(CtConfig* config): CtParser(config), CtTextParser(config)
{

}

std::unique_ptr<ct_imported_node> CtZimImport::import_file(const fs::path& file)
{
    if (file.extension() != ".txt") return nullptr;

    _ensure_notebook_file_in_dir(file.parent_path());

    std::unique_ptr<ct_imported_node> node = std::make_unique<ct_imported_node>(file, file.stem().string());
    _current_node = node.get();
    _current_element = node->xml_content.create_root_node("root")->add_child("slot");
    _current_element = _current_element->add_child("rich_text");

    std::ifstream stream(file.string());
    feed(stream);
    return node;
}

void CtZimImport::feed(std::istream& data)
{
    _init_tokens();
    _build_token_maps();

    std::string line;
    while(std::getline(data, line, '\n')) {
        if (_parse_state == PARSING_STATE::HEAD) {
            // Creation-Date: .* is the final line of the header
            if (line.find("Creation-Date:") != std::string::npos) {
                // TODO: Read the creation date and use it for ts_creation
                _parse_state = PARSING_STATE::BODY;
            }
        } else if (_parse_state == PARSING_STATE::BODY) {
            _parse_body_line(line);
        }
    }
    // Reset
    _parse_state = PARSING_STATE::HEAD;
}

void CtZimImport::_init_tokens()
{
    if (_token_schemas.empty()) {
        _token_schemas = {
            // Bold
            {"**", true, true, [this](const std::string& data){
                _close_current_tag();
                _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
                _close_current_tag();
            }},
            // Indentation detection for lists
            {"\t", false, false, [this](const std::string& data) {
                _list_level++;
                // Did a double match for even number of \t tags
                if (data.empty()) _list_level++;
            }},
            {"https://", false, false, [this](const std::string& data) {
                _close_current_tag();
                _add_link("https://"+data);
                _add_text("https://"+data);
                _close_current_tag();
            }},
            {"http://", false, false, [this](const std::string& data) {
                _close_current_tag();
                _add_link("http://"+data);
                _add_text("http://"+data);
                _close_current_tag();
            }},
            // Bullet list
            {"* ", false, false, [this](const std::string& data) {
                _add_list(_list_level, data);
                _list_level = 0;
            }},
            // Italic
            {"//", true, true, [this](const std::string& data) {
                _close_current_tag();
                _add_italic_tag(data);
                _close_current_tag();
            }},
            // Strikethrough
            {"~~", true, true, [this](const std::string& data){
                _close_current_tag();
                _add_strikethrough_tag(data);
                _close_current_tag();
            }},
            // Headers
            {"==", false, false, [this](const std::string &data) {
                    int count = 5;

                    auto iter = data.begin();
                    while (*iter == '=') {
                        count--;
                        ++iter;
                    }

                    if (count < 0) {
                        throw CtImportException(fmt::format("Parsing error while parsing header data: {} - Too many '='", data));
                    }

                    if (count > 3) {
                        // Reset to smaller (h3 currently)
                        count = 3;
                    }

                    auto str = str::replace(data, "= ", "");
                    str = str::replace(str, "=", "");

                    _close_current_tag();
                    _add_scale_tag(count, str);
                    _close_current_tag();
            }, "==", true},
            // External link (e.g https://example.com)
            {"{{", true, false, [](const std::string&) {
                // Todo: Implement this (needs image importing)
            },"}}"},
            // Todo list
           // {"[", true, false, links_match_func, "] "},
            {"[*", true, false, [this](const std::string& data) {
                _add_todo_list(CHECKBOX_STATE::TICKED, data);
            }, "]"},
            {"[x", true, false, [this](const std::string& data) {
                _add_todo_list(CHECKBOX_STATE::MARKED, data);
            }, "]"},
            {"[>", true, false, [this](const std::string& data) {
                _add_todo_list(CHECKBOX_STATE::MARKED, data);
            }, "]"},
            {"[ ", true, false, [this](const std::string& data) {
                _add_todo_list(CHECKBOX_STATE::UNCHECKED, data);
            }, "]"},
            // Internal link (e.g MyPage) - This is just for removing the leftover ']'
            {"[[", true, false, [this](const std::string& data){
                _close_current_tag();
                 _current_node->add_broken_link(data, _current_element);
                 _add_text(data);
                _close_current_tag();
            }, "]]"},
            // Verbatum - captures all the tokens inside it and print without formatting
            {"''", true, true, [this](const std::string& data){
                _add_text(data);
            }, "''", true},
            // Suberscript
            {"^{", true, false, [this](const std::string& data){
                _close_current_tag();
                _add_superscript_tag(data);
                _close_current_tag();
            }, "}"},
            // Subscript
            {"_{", true, false, [this](const std::string& data){
                _close_current_tag();
                _add_subscript_tag(data);
                _close_current_tag();
            }, "}"}

        };
    }
}

void CtZimImport::_parse_body_line(const std::string& line)
{
    auto tokens_raw = _tokenize(line);
    auto tokens = _parse_tokens(tokens_raw);

    for (const auto& token : tokens) {
        if (token.first) {
            token.first->action(token.second);
        } else {
            _add_text(token.second);
        }

    }
    _add_newline();
}

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

        std::unique_ptr<ct_imported_node> node = std::make_unique<ct_imported_node>(file, file.stem().string());
        node->xml_content.create_root_node("root")->add_child("slot")->add_child("rich_text")->add_child_text(data.str());
        node->node_syntax = CtConst::PLAIN_TEXT_ID;
        return node;
    }
    catch (std::exception& ex)
    {
        spdlog::error("CtPlainTextImport, what: , file: {}", ex.what(), file);
    }
    return nullptr;
}



CtMDImport::CtMDImport(CtConfig* config) : _parser(config)
{

}

std::unique_ptr<ct_imported_node> CtMDImport::import_file(const fs::path& file)
{
    if (file.extension() != ".md")
        return nullptr;

    std::ifstream infile(file.string());
    if (!infile) throw std::runtime_error(fmt::format("CtMDImport: cannot open file, what: {}, file: {}", strerror(errno), file));
    _parser.wipe();
    _parser.feed(infile);

    std::unique_ptr<ct_imported_node> node = std::make_unique<ct_imported_node>(file, file.stem().string());
    node->xml_content.create_root_node_by_import(_parser.get_root_node());

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
    parser.set_outter_xml_doc(&node->xml_content);
    parser.feed(html_buff.str());

    return node;
}
