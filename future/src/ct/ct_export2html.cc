/*
 * ct_export2html.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_export2html.h"
#include "ct_misc_utils.h"
#include "ct_app.h"

CtExport2Html::CtExport2Html()
{

}

// Returns the HTML given the text buffer and iter bounds
Glib::ustring CtExport2Html::selection_export_to_html(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter start_iter,
                                                      Gtk::TextIter end_iter, const Glib::ustring& syntax_highlighting)
{
    Glib::ustring html_text = str::format(HTML_HEADER, "");
    if (syntax_highlighting == CtConst::RICH_TEXT_ID)
    {
        int images_count = 0;
        Glib::ustring tempFolder = CtApp::P_ctTmp->getHiddenDirPath("IMAGE_TEMP_FOLDER");

        int start_offset = start_iter.get_offset();
        std::list<CtAnchoredWidget*> widgets = CtApp::P_ctActions->getCtMainWin()->curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(std::make_pair(start_iter.get_offset(), end_iter.get_offset()));
        for (CtAnchoredWidget* widget: widgets)
        {
            int end_offset = widget->getOffset();
            Glib::ustring text_slot = _html_process_slot(start_offset, end_offset, text_buffer);
            html_text += text_slot;
            if (CtImage* image = dynamic_cast<CtImage*>(widget)) html_text += _get_image_html(image, tempFolder, images_count, nullptr);
            else if (CtTable* table = dynamic_cast<CtTable*>(widget)) html_text += _get_table_html(table);
            else if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widget)) html_text += _get_codebox_html(codebox);
            start_offset = end_offset;
        }
        _html_process_slot(start_offset, end_iter.get_offset(), text_buffer);
    }
    else
    {
        Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(text_buffer);
        html_text += _html_get_from_code_buffer(gsv_buffer, std::make_pair(start_iter.get_offset(), end_iter.get_offset()));
    }
    html_text += HTML_FOOTER;
    return html_text;
}

// Returns the HTML given the table
Glib::ustring CtExport2Html::table_export_to_html(CtTable* table)
{
    Glib::ustring html_text = str::format(HTML_HEADER, "");
    html_text += _get_table_html(table);
    html_text += HTML_FOOTER;
    return html_text;
}

// Returns the HTML given the codebox
Glib::ustring CtExport2Html::codebox_export_to_html(CtCodebox* codebox)
{
    Glib::ustring html_text = str::format(HTML_HEADER, "");
    html_text += _get_codebox_html(codebox);
    html_text += HTML_FOOTER;
    return html_text;
}

// Returns the HTML Image
Glib::ustring CtExport2Html::_get_image_html(CtImage* image, const Glib::ustring& images_dir, int& images_count, CtTreeIter* tree_iter)
{
    if (CtImageAnchor* imageAnchor = dynamic_cast<CtImageAnchor*>(image))
        return "<a name=\"" + imageAnchor->get_anchor_name() + "\"></a>";

    images_count += 1;
    Glib::ustring image_name, image_rel_path;
    if (tree_iter)
    {
        image_name = std::to_string(tree_iter->get_node_id()) + "-" + std::to_string(images_count) + ".png";
        image_rel_path = CtFileSystem::join("images", image_name);
    }
    else
    {
        image_name = std::to_string(images_count) + ".png";
        image_rel_path = "file://" + CtFileSystem::join(images_dir, image_name);
    }

    Glib::ustring image_html = "<img src=\"" + image_rel_path + "\" alt=\"" + image_rel_path + "\" />";
    if (CtImagePng* png = dynamic_cast<CtImagePng*>(image))
    {
        Glib::ustring href = _get_href_from_link_prop_val(png->get_link());
        image_html = "<a href=\"" + href + "\">" + image_html + "</a>";
    }

    image->save(CtFileSystem::join(images_dir, image_name), "png");
    return image_html;
}

// Returns the HTML CodeBox
Glib::ustring CtExport2Html::_get_codebox_html(CtCodebox* codebox)
{
    Glib::ustring codebox_html = "<div class=\"codebox\">";
    codebox_html += _html_get_from_code_buffer(codebox->get_buffer(), std::make_pair(-1, -1));
    codebox_html += "</div>";
    return codebox_html;
}

// Returns the HTML Table
Glib::ustring CtExport2Html::_get_table_html(CtTable* table)
{
    Glib::ustring table_html = "<table class=\"table\">";
    for (auto row: table->get_table_matrix())
    {
        table_html += "<tr>";
        bool first = true;
        for (auto cell: row)
            if (first) {
                table_html += "<th>" + str::xml_escape(cell->get_text_content()) + "</th>";
                first = false;
            } else {
                table_html += "<td>" + str::xml_escape(cell->get_text_content()) + "</td>";
            }
        table_html += "</tr>";
    }
    table_html += "</table>";
    return table_html;
}

// Get rich text from syntax highlighted code node
Glib::ustring CtExport2Html::_html_get_from_code_buffer(Glib::RefPtr<Gsv::Buffer> code_buffer, std::pair<int, int> sel_range)
{
    Gtk::TextIter curr_iter = sel_range.first >= 0 ? code_buffer->get_iter_at_offset(sel_range.first) : code_buffer->begin();
    code_buffer->ensure_highlight(curr_iter, code_buffer->end());
    Glib::ustring html_text = "";
    Glib::ustring former_tag_str = CtConst::COLOR_48_BLACK;
    bool span_opened = false;
    for (;;)
    {
        auto curr_tags = curr_iter.get_tags();
        if (curr_tags.size() > 0)
        {
            Glib::ustring curr_tag_str = curr_tags[0]->property_foreground_gdk().get_value().to_string();
            int font_weight = curr_tags[0]->property_weight().get_value();
            if (curr_tag_str == CtConst::COLOR_48_BLACK)
            {
                if (former_tag_str != curr_tag_str)
                {
                    former_tag_str = curr_tag_str;
                    // end of tag
                    html_text += "</span>";
                    span_opened = false;
                }
            }
            else
            {
                if (former_tag_str != curr_tag_str)
                {
                    former_tag_str = curr_tag_str;
                    if (span_opened) html_text += "</span>";
                    // start of tag
                    Glib::ustring color = CtRgbUtil::rgb_to_no_white(curr_tag_str);
                    color = CtRgbUtil::getRgb24StrFromStrAny(color);
                    html_text += "<span style=\"color:" + color + ";font-weight:" + std::to_string(font_weight) + "\">";
                    span_opened = true;
                }
            }
        }
        else if (span_opened)
        {
            span_opened = false;
            former_tag_str = CtConst::COLOR_48_BLACK;
            html_text += "</span>";
        }
        Glib::ustring sym = str::xml_escape(Glib::ustring(1, curr_iter.get_char()));
        html_text += str::replace(sym, " ", "&nbsp;");
        if (!curr_iter.forward_char() || (sel_range.second >= 0 && curr_iter.get_offset() > sel_range.second))
        {
            if (span_opened) html_text += "</span>";
            break;
        }
    }

    html_text = str::replace(html_text, CtConst::CHAR_NEWLINE, "<br />");
    return "<div class=\"codebox\">" + html_text + "</div>";
}

// Process a Single HTML Slot
Glib::ustring CtExport2Html::_html_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer)
{
    Glib::ustring curr_html_text = "";
    CtTextIterUtil::generic_process_slot(start_offset, end_offset, curr_buffer,
                                         [&](Gtk::TextIter& start_iter, Gtk::TextIter& curr_iter, std::map<const gchar*, std::string>& curr_attributes) {
        curr_html_text += _html_text_serialize(start_iter, curr_iter, curr_attributes);
    });

    curr_html_text = str::replace(curr_html_text, "<br/><p ", "<p ");
    curr_html_text = str::replace(curr_html_text, "</p><br/>", "</p>");
    for (auto header: {CtConst::TAG_PROP_VAL_H1, CtConst::TAG_PROP_VAL_H2, CtConst::TAG_PROP_VAL_H3})
        curr_html_text = str::replace(curr_html_text, ("</" + Glib::ustring(header) + "><" + Glib::ustring(header) + " >").c_str(), "");

    return curr_html_text;
}

// Adds a slice to the HTML Text
Glib::ustring CtExport2Html::_html_text_serialize(Gtk::TextIter start_iter, Gtk::TextIter end_iter, const std::map<const gchar*, std::string>& curr_attributes)
{
    Glib::ustring inner_text = str::xml_escape(start_iter.get_text(end_iter));
    if (inner_text == "") return "";
    inner_text = str::replace(inner_text, CtConst::CHAR_NEWLINE, "<br />");

    Glib::ustring html_attrs = "";
    bool superscript_active = false;
    bool subscript_active = false;
    bool monospace_active = false;
    bool bold_active = false;
    bool italic_active = false;
    for (auto tag_property: CtConst::TAG_PROPERTIES)
    {
        if (curr_attributes.at(tag_property) == "")
            continue;
        Glib::ustring property_value = curr_attributes.at(tag_property);
        if (tag_property == CtConst::TAG_WEIGHT)
        {
            // font-weight:bolder
            // tag_property = "font-weight"
            // property_value = "bolder"
            bold_active = true;
            continue;
        }
        else if (tag_property == CtConst::TAG_FOREGROUND)
        {
            // color:#FFFF00
            tag_property = "color";
            Glib::ustring color_no_white = CtRgbUtil::rgb_to_no_white(property_value);
            property_value = CtRgbUtil::getRgb24StrFromStrAny(color_no_white);
        }
        else if (tag_property == CtConst::TAG_BACKGROUND)
        {
            // background-color:#FFFF00
            tag_property = "background-color";
            property_value = CtRgbUtil::getRgb24StrFromStrAny(property_value);
        }
        else if (tag_property == CtConst::TAG_STYLE)
        {
            // font-style:italic
            // tag_property = "font-style"
            // property_value = cons.TAG_PROP_ITALIC
            italic_active = true;
            continue;
        }
        else if (tag_property == CtConst::TAG_UNDERLINE)
        {
            // text-decoration:underline
            tag_property = "text-decoration";
            property_value = CtConst::TAG_UNDERLINE;
        }
        else if (tag_property == CtConst::TAG_STRIKETHROUGH)
        {
            // text-decoration:line-through
            tag_property = "text-decoration";
            property_value = "line-through";
        }
        else if (tag_property == CtConst::TAG_SCALE)
        {
            if (property_value == CtConst::TAG_PROP_VAL_SUP)
            {
                superscript_active = true;
                continue;
            }
            else if (property_value == CtConst::TAG_PROP_VAL_SUB)
            {
                subscript_active = true;
                continue;
            }
            else
            {
                // font-size:xx-large/x-large/x-small
                tag_property = "font-size";
                if (property_value == CtConst::TAG_PROP_VAL_SMALL) property_value = "x-small";
                else if (property_value == CtConst::TAG_PROP_VAL_H1) property_value = "xx-large";
                else if (property_value == CtConst::TAG_PROP_VAL_H2) property_value = "x-large";
                else if (property_value == CtConst::TAG_PROP_VAL_H3) property_value = "large";
            }
        }
        else if (tag_property == CtConst::TAG_FAMILY)
        {
            monospace_active = true;
            continue;
        }
        else if (tag_property == CtConst::TAG_JUSTIFICATION)
        {
            // text-align:center/left/right
            // tag_property = "text-align"
            continue;
        }
        else if (tag_property == CtConst::TAG_LINK)
        {
            // <a href="http://www.example.com/">link-text goes here</a>
            Glib::ustring href = _get_href_from_link_prop_val(property_value);
            if (href == "")
                continue;
            Glib::ustring html_text = "<a href=\"" + href + "\">" + inner_text + "</a>";
            return html_text;
        }
        html_attrs += Glib::ustring(tag_property) + ":" + property_value + ";";
    }
    Glib::ustring tagged_text;
    if (html_attrs == "" || inner_text == "<br />")
        tagged_text = inner_text;
    else
    {
        if (html_attrs.find("xx-large") != Glib::ustring::npos)
            tagged_text = "<h1>" + inner_text + "</h1>";
        else if (html_attrs.find("x-large") != Glib::ustring::npos)
            tagged_text = "<h2>" + inner_text + "</h2>";
        else if (html_attrs.find("large") != Glib::ustring::npos)
            tagged_text = "<h3>" + inner_text + "</h3>";
        else if (html_attrs.find("x-small") != Glib::ustring::npos)
            tagged_text = "<small>" + inner_text + "</small>";
        else
            tagged_text = "<span style=\"" + html_attrs + "\">" + inner_text + "</span>";
    }
    if (superscript_active) tagged_text = "<sup>" + tagged_text + "</sup>";
    if (subscript_active) tagged_text = "<sub>" + tagged_text + "</sub>";
    if (monospace_active) tagged_text = "<code>" + tagged_text + "</code>";
    if (bold_active) tagged_text = "<strong>" + tagged_text + "</strong>";
    if (italic_active) tagged_text = "<em>" + tagged_text + "</em>";

    return tagged_text;
}

Glib::ustring CtExport2Html::_get_href_from_link_prop_val(Glib::ustring link_prop_val)
{
    // todo: I saw the same function before, we need to join them
    Glib::ustring href = "";
    auto vec = str::split(link_prop_val, " ");
    if (vec[0] == CtConst::LINK_TYPE_WEBS)
        href = vec[1];
    else if (vec[0] == CtConst::LINK_TYPE_FILE)
    {
        Glib::ustring filepath = _link_process_filepath(vec[1]);
        href = "file://" + filepath;
    }
    else if (vec[0] == CtConst::LINK_TYPE_FOLD)
    {
        Glib::ustring folderpath = _link_process_folderpath(vec[1]);
        href = "file://" + folderpath;
    }
    else if (vec[0] == CtConst::LINK_TYPE_NODE)
    {
        CtTreeIter node = CtApp::P_ctActions->getCtMainWin()->get_tree_store().get_node_from_node_id(std::stol(vec[1]));
        if (node)
        {
            href = _get_html_filename(node);
            if (vec.size() >= 3)
            {

                if (vec.size() == 3) href += "#" + vec[2];
                else href += "#" + link_prop_val.substr(vec[0].size() + vec[1].size() + 2);
            }
        }
    }
    return href;
}

Glib::ustring CtExport2Html::_link_process_filepath(const Glib::ustring& filepath_raw)
{
    Glib::ustring filepath_orig = Glib::Base64::decode(filepath_raw);
    Glib::ustring filepath = CtFileSystem::get_proper_platform_filepath(filepath_orig);
    // todo:
    //if not os.path.isabs(filepath) and os.path.isfile(os.path.join(self.file_dir, filepath)):
    //    filepath = os.path.join(self.file_dir, filepath)
    return filepath;
}

Glib::ustring CtExport2Html::_link_process_folderpath(const Glib::ustring& folderpath_raw)
{
    Glib::ustring folderpath_orig = Glib::Base64::decode(folderpath_raw);
    Glib::ustring folderpath = CtFileSystem::get_proper_platform_filepath(folderpath_orig);
    // todo:
    //if not os.path.isabs(folderpath) and os.path.isdir(os.path.join(self.file_dir, folderpath)):
    //    folderpath = os.path.join(self.file_dir, folderpath)
    return folderpath;
}

// Get the HTML page filename given the tree iter
Glib::ustring CtExport2Html::_get_html_filename(CtTreeIter tree_iter)
{
    Glib::ustring name = CtMiscUtil::get_node_hierarchical_name(tree_iter, "--", true, true, ".html");
    return str::replace(name, "#", "~");
}
