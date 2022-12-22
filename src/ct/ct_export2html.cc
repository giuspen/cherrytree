/*
 * ct_export2html.cc
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

#include "ct_export2html.h"
#include "ct_misc_utils.h"
#include "ct_main_win.h"
#include "ct_dialogs.h"
#include "ct_storage_control.h"
#include "ct_logging.h"
#include "ct_filesystem.h"

CtExport2Html::CtExport2Html(CtMainWin* pCtMainWin)
 : _pCtMainWin(pCtMainWin)
{
}

//Prepare the website folder
bool CtExport2Html::prepare_html_folder(fs::path dir_place, fs::path new_folder, bool export_overwrite, fs::path& export_path)
{
    if (dir_place.empty())
    {
        dir_place = CtDialogs::folder_select_dialog(_pCtMainWin->get_ct_config()->pickDirExport, _pCtMainWin);
        if (dir_place.empty())
            return false;
    }
    new_folder = CtMiscUtil::clean_from_chars_not_for_filename(new_folder.string()) + "_HTML";
    new_folder = fs::prepare_export_folder(dir_place, new_folder, export_overwrite);
    _export_dir = dir_place / new_folder;
    _images_dir = _export_dir / "images";
    _embed_dir = _export_dir / "EmbeddedFiles";
    _res_dir = _export_dir / "res";
    g_mkdir_with_parents(_export_dir.c_str(), 0777);
    g_mkdir_with_parents(_images_dir.c_str(), 0777);
    g_mkdir_with_parents(_embed_dir.c_str(), 0777);
    g_mkdir_with_parents(_res_dir.c_str(), 0777);

    fs::path config_dir = fs::get_cherrytree_configdir();
    fs::path styles_css_filepath = config_dir / "styles4.css";
    if (!fs::is_regular_file(styles_css_filepath))
    {
        fs::path styles_css_original = fs::path(fs::get_cherrytree_datadir()) / fs::path("data") / "styles4.css";
        fs::copy_file(styles_css_original, styles_css_filepath);
    }
    fs::copy_file(styles_css_filepath, _res_dir / "styles4.css");

    fs::path styles_js_filepath = config_dir / "script3.js";
    if (!fs::is_regular_file(styles_js_filepath))
    {
        fs::path script_js_original = fs::get_cherrytree_datadir() / "data" / "script3.js";
        fs::copy_file(script_js_original, styles_js_filepath);
    }
    fs::copy_file(styles_js_filepath, _res_dir / "script3.js");

    export_path = _export_dir;

    return true;
}

// Export a Node To HTML
void CtExport2Html::node_export_to_html(CtTreeIter tree_iter, const CtExportOptions& options, const Glib::ustring& index, int sel_start, int sel_end)
{
    Glib::ustring html_text = str::format(HTML_HEADER, tree_iter.get_node_name());
    if (not index.empty() and options.index_in_page) {
        auto script = R"HTML(
            <script type='text/javascript'>
                function in_frame () { try { return window.self !== window.top; } catch (e) { return true; } }
                if (!in_frame()) {
                    var page = location.pathname.substring(location.pathname.lastIndexOf("/") + 1);
                    window.location = 'index.html#' + page;
                }
            </script>)HTML";
        html_text = str::replace(html_text, "<script></script>", script);
    }
    html_text += "<div class='page'>";
    if (options.include_node_name) {
        html_text += "<h1 class='title'>" + tree_iter.get_node_name() + "</h1><br/>";
    }
    std::vector<Glib::ustring> html_slots;
    std::vector<CtAnchoredWidget*> widgets;
    if (tree_iter.get_node_is_text()) {
        Glib::ustring node_html_text;
        _html_get_from_treestore_node(tree_iter, sel_start, sel_end, html_slots, widgets);
        int images_count{0};
        for (size_t i = 0; i < html_slots.size(); ++i) {
            node_html_text += html_slots[i];
            if (i < widgets.size()) {
                try {
                    if (auto embfile = dynamic_cast<CtImageEmbFile*>(widgets[i]))
                        node_html_text += _get_embfile_html(embfile, tree_iter, _embed_dir);
                    else if (auto image = dynamic_cast<CtImage*>(widgets[i]))
                        node_html_text += _get_image_html(image, _images_dir, images_count, &tree_iter);
                    else if (auto table = dynamic_cast<CtTableCommon*>(widgets[i]))
                        node_html_text += _get_table_html(table);
                    else if (auto codebox = dynamic_cast<CtCodebox*>(widgets[i]))
                        node_html_text += _get_codebox_html(codebox);
                }
                catch (std::exception& ex) {
                    spdlog::debug("caught ex: {}", ex.what());
                }
                catch (...) {
                    spdlog::debug("unknown ex");
                }
            }
        }
        auto rTextBuffer = tree_iter.get_node_text_buffer();
        Gtk::TextIter start_iter = rTextBuffer->get_iter_at_offset(sel_start == -1 ? 0 : sel_start);
        Gtk::TextIter end_iter = sel_end == -1 ? rTextBuffer->end() : rTextBuffer->get_iter_at_offset(sel_end);
        std::vector<Glib::ustring> node_lines = str::split(node_html_text, CtConst::CHAR_NEWLINE);
        if (node_lines.size() > 0) {
            std::vector<bool> rtl_for_lines = CtStrUtil::get_rtl_for_lines(start_iter.get_text(end_iter));
            while (rtl_for_lines.size() < node_lines.size()) { rtl_for_lines.push_back(false); }
            const size_t lastIdx = node_lines.size() - 1;
            for (size_t i = 0; i <= lastIdx; ++i) {
                if (i < lastIdx or not node_lines.at(i).empty()) {
                    if (rtl_for_lines.at(i)) html_text += "<p dir=\"rtl\">" + node_lines.at(i) + "</p>";
                    else html_text += "<p>" + node_lines.at(i) + "</p>";
                }
            }
        }
    }
    else {
        html_text += _html_get_from_code_buffer(tree_iter.get_node_text_buffer(), sel_start, sel_end, tree_iter.get_node_syntax_highlighting());
    }
    if (not index.empty() and not options.index_in_page) {
        html_text += Glib::ustring("<p align=\"center\">") + "<img src=\"" + Glib::build_filename("images", "home.svg") + "\" height=\"22\" width=\"22\">" +
                CtConst::CHAR_SPACE + CtConst::CHAR_SPACE + "<a href=\"index.html\">" + _("Index") + "</a></p>";
    }
    html_text += "</div>"; // div class='page'
    html_text += HTML_FOOTER;

    fs::path node_html_filepath = _export_dir / _get_html_filename(tree_iter);
    g_file_set_contents(node_html_filepath.c_str(), html_text.c_str(), (gssize)html_text.bytes(), nullptr);
}

// Export All Nodes To HTML
void CtExport2Html::nodes_all_export_to_multiple_html(bool all_tree, const CtExportOptions& options)
{
    fs::path home_svg = fs::get_cherrytree_datadir() / fs::path("icons") / "ct_home.svg";
    fs::copy_file(home_svg, _images_dir / "home.svg");

    // create tree links text
    Glib::ustring tree_links_text = // dont' use R"HTML, it gives unnecessary " "
          "<div class='tree'>\n"
          "<p>\n"
          "<strong>Index</strong></br>\n"
          "<button onclick='expandAllSubtrees()'>Expand All</button> <button onclick='collapseAllSubtrees()'>Collapse All</button>\n"
          "</p>\n"
          "<ul class='outermost'>\n";
    CtTreeIter tree_iter = all_tree ? _pCtMainWin->get_tree_store().get_ct_iter_first() : _pCtMainWin->curr_tree_iter();
    for (;tree_iter; ++tree_iter)
    {
        _tree_links_text_iter(tree_iter, tree_links_text, 1, options.index_in_page);
        if (!all_tree) break;
    }
    tree_links_text += "</ul>\n";
    tree_links_text += "</div>\n";

    // create index html page
    Glib::ustring html_text = str::format(HTML_HEADER, _pCtMainWin->get_ct_storage()->get_file_name());
    if (options.index_in_page)
    {
        html_text += "<div class='two-panels'>\n<div class='tree-panel'>\n";
        html_text += tree_links_text;
        html_text += "</div>\n";
        html_text += "<div class='page-panel'><iframe src='' id='page_frame'></iframe></div>";
        html_text += "</div>"; // two-panels
    }
    else
        html_text += "<div class='page'>" + tree_links_text + "</div>";
    html_text += "<script src='res/script3.js'></script>\n";
    html_text += HTML_FOOTER;
    fs::path node_html_filepath = _export_dir / "index.html";
    g_file_set_contents(node_html_filepath.c_str(), html_text.c_str(), (gssize)html_text.bytes(), nullptr);

    // create html pages
    // function to iterate nodes
    std::function<void(CtTreeIter)> traverseFunc;
    traverseFunc = [this, &traverseFunc, &options, &tree_links_text](CtTreeIter tree_iter) {
        node_export_to_html(tree_iter, options, tree_links_text, -1, -1);
        for (auto& child: tree_iter->children())
            traverseFunc(_pCtMainWin->get_tree_store().to_ct_tree_iter(child));
    };
    // start to iterarte nodes
    tree_iter = all_tree ? _pCtMainWin->get_tree_store().get_ct_iter_first() : _pCtMainWin->curr_tree_iter();
    for (;tree_iter; ++tree_iter)
    {
        traverseFunc(tree_iter);
        if (!all_tree) break;
    }
}

// Export All Nodes To Single HTML
void CtExport2Html::nodes_all_export_to_single_html(bool all_tree, const CtExportOptions&)
{
    fs::path index_html_filepath = _export_dir / "index.html";
    Glib::RefPtr<Gio::File> rFile = Gio::File::create_for_path(index_html_filepath.string());
    Glib::RefPtr<Gio::FileOutputStream> rFileStream = rFile->append_to();

    // create html pages
    // function to iterate nodes
    Glib::ustring tree_links_text = "";
    std::function<void(CtTreeIter, int)> traverseFunc;
    traverseFunc = [this, &traverseFunc, rFileStream](CtTreeIter tree_iter, int node_level) {
        Glib::ustring html_text = "<div class='page'>";
        html_text += "<h1 class='title level-" + std::to_string(node_level) + "'>" + tree_iter.get_node_name() + "</h1><br/>";
        std::vector<Glib::ustring> html_slots;
        std::vector<CtAnchoredWidget*> widgets;
        if (tree_iter.get_node_is_text()) {
            Glib::ustring node_html_text;
            _html_get_from_treestore_node(tree_iter, -1, -1, html_slots, widgets);
            int images_count = 0;
            for (size_t i = 0; i < html_slots.size(); ++i) {
                node_html_text += html_slots[i];
                if (i < widgets.size()) {
                    try {
                        if (auto embfile = dynamic_cast<CtImageEmbFile*>(widgets[i]))
                            node_html_text += _get_embfile_html(embfile, tree_iter, _embed_dir);
                        else if (auto image = dynamic_cast<CtImage*>(widgets[i]))
                            node_html_text += _get_image_html(image, _images_dir, images_count, &tree_iter);
                        else if (auto table = dynamic_cast<CtTableCommon*>(widgets[i]))
                            node_html_text += _get_table_html(table);
                        else if (auto codebox = dynamic_cast<CtCodebox*>(widgets[i]))
                            node_html_text += _get_codebox_html(codebox);
                    }
                    catch (std::exception& ex) {
                        spdlog::debug("caught ex: {}", ex.what());
                    }
                    catch (...) {
                        spdlog::debug("unknown ex");
                    }
                }
            }
            std::vector<Glib::ustring> node_lines = str::split(node_html_text, CtConst::CHAR_NEWLINE);
            if (node_lines.size() > 0) {
                std::vector<bool> rtl_for_lines = CtStrUtil::get_rtl_for_lines(tree_iter.get_node_text_buffer()->get_text());
                while (rtl_for_lines.size() < node_lines.size()) { rtl_for_lines.push_back(false); }
                const size_t lastIdx = node_lines.size() - 1;
                for (size_t i = 0; i <= lastIdx; ++i) {
                    if (i < lastIdx or not node_lines.at(i).empty()) {
                        if (rtl_for_lines.at(i)) html_text += "<p dir=\"rtl\">" + node_lines.at(i) + "</p>";
                        else html_text += "<p>" + node_lines.at(i) + "</p>";
                    }
                }
            }
        }
        else {
            html_text += _html_get_from_code_buffer(tree_iter.get_node_text_buffer(), -1, -1, tree_iter.get_node_syntax_highlighting());
        }
        html_text += "</div>"; // div class='page'
        rFileStream->write(html_text.c_str(), html_text.bytes());
        html_text.clear();

        for (auto& child : tree_iter->children()) {
            traverseFunc(_pCtMainWin->get_tree_store().to_ct_tree_iter(child), node_level + 1);
        }
    };

    Glib::ustring html_header = str::format(HTML_HEADER, _pCtMainWin->get_ct_storage()->get_file_name());
    rFileStream->write(html_header.c_str(), html_header.bytes());

    // start to iterarte nodes
    CtTreeIter tree_iter = all_tree ? _pCtMainWin->get_tree_store().get_ct_iter_first() : _pCtMainWin->curr_tree_iter();
    for (;tree_iter; ++tree_iter)
    {
        traverseFunc(tree_iter, 1);
        if (!all_tree) break;
    }
    rFileStream->write(HTML_FOOTER.c_str(), HTML_FOOTER.bytes());
    rFileStream->flush();
    rFileStream->close();
}

// Creating the Tree Links Text - iter
void CtExport2Html::_tree_links_text_iter(CtTreeIter tree_iter, Glib::ustring& tree_links_text, int tree_count_level, bool index_in_page)
{
    Glib::ustring href = str::replace(_get_html_filename(tree_iter), "'", "\\'");
    Glib::ustring node_name = tree_iter.get_node_name();
    if (tree_iter->children().empty())
    {
        if (index_in_page)
            tree_links_text += "<li class='leaf'><a href='#' onclick=\"changeFrame('" + href + "')\">" + node_name + "</a></li>\n";
        else
            tree_links_text += "<li class='leaf'><a href='" + href + "'>" + node_name + "</a></li>\n";
    }
    else
    {
        if (index_in_page)
            tree_links_text += "<li><button onclick='toggleSubTree(this)'>-</button> <a href='#' onclick=\"changeFrame('" + href + "')\">" + node_name + "</a></li>";
        else
            tree_links_text += "<li><button onclick='toggleSubTree(this)'>-</button> <a href='" + href + "'>" + node_name +"</a></li>";
        tree_links_text += "<ul class='subtree'>\n";
        for (auto& child: tree_iter->children())
            _tree_links_text_iter(_pCtMainWin->get_tree_store().to_ct_tree_iter(child), tree_links_text, tree_count_level + 1, index_in_page);
        tree_links_text += "</ul>\n";
    }
}

// Returns the HTML given the text buffer and iter bounds
Glib::ustring CtExport2Html::selection_export_to_html(Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                                      Gtk::TextIter start_iter,
                                                      Gtk::TextIter end_iter,
                                                      const Glib::ustring& syntax_highlighting)
{
    Glib::ustring html_text = str::format(HTML_HEADER, "");
    if (syntax_highlighting == CtConst::RICH_TEXT_ID) {
        Glib::ustring node_html_text;
        int images_count{0};
        fs::path tempFolder = _pCtMainWin->get_ct_tmp()->getHiddenDirPath("IMAGE_TEMP_FOLDER");
        int start_offset = start_iter.get_offset();
        std::list<CtAnchoredWidget*> widgets = _pCtMainWin->curr_tree_iter().get_anchored_widgets(start_iter.get_offset(), end_iter.get_offset());
        for (CtAnchoredWidget* widget : widgets) {
            int end_offset = widget->getOffset();
            node_html_text +=_html_process_slot(start_offset, end_offset, text_buffer);
            if (CtImage* image = dynamic_cast<CtImage*>(widget)) node_html_text += _get_image_html(image, tempFolder, images_count, nullptr);
            else if (auto table = dynamic_cast<CtTableCommon*>(widget)) node_html_text += _get_table_html(table);
            else if (auto codebox = dynamic_cast<CtCodebox*>(widget)) node_html_text += _get_codebox_html(codebox);
            start_offset = end_offset;
        }
        node_html_text += _html_process_slot(start_offset, end_iter.get_offset(), text_buffer);

        std::vector<Glib::ustring> node_lines = str::split(node_html_text, CtConst::CHAR_NEWLINE);
        if (node_lines.size() > 0) {
            std::vector<bool> rtl_for_lines = CtStrUtil::get_rtl_for_lines(start_iter.get_text(end_iter));
            while (rtl_for_lines.size() < node_lines.size()) { rtl_for_lines.push_back(false); }
            const size_t lastIdx = node_lines.size() - 1;
            for (size_t i = 0; i <= lastIdx; ++i) {
                if (i < lastIdx or not node_lines.at(i).empty()) {
                    if (rtl_for_lines.at(i)) html_text += "<p dir=\"rtl\">" + node_lines.at(i) + "</p>";
                    else html_text += "<p>" + node_lines.at(i) + "</p>";
                }
            }
        }
    }
    else {
        Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(text_buffer);
        html_text += _html_get_from_code_buffer(gsv_buffer, start_iter.get_offset(), end_iter.get_offset(), syntax_highlighting);
    }
    html_text += HTML_FOOTER;
    return html_text;
}

// Returns the HTML given the table
Glib::ustring CtExport2Html::table_export_to_html(CtTableCommon* table)
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

// Returns the HTML embedded file
Glib::ustring CtExport2Html::_get_embfile_html(CtImageEmbFile* embfile, CtTreeIter tree_iter, fs::path embed_dir)
{
    Glib::ustring embfile_align_text = _get_object_alignment_string(embfile->getJustification());
    fs::path embfile_name = std::to_string(tree_iter.get_node_id()) + "-" +  embfile->get_file_name().string();
    fs::path embfile_rel_path = "EmbeddedFiles" / embfile_name;
    Glib::ustring embfile_html = "<table style=\"" + embfile_align_text + "\"><tr><td><a href=\"" +
            embfile_rel_path.string_unix() + "\">Linked file: " + embfile->get_file_name().string() + " </a></td></tr></table>";

    g_file_set_contents((embed_dir / embfile_name).c_str(), embfile->get_raw_blob().c_str(), (gssize)embfile->get_raw_blob().size(), nullptr);

    return embfile_html;
}

// Returns the HTML Image
Glib::ustring CtExport2Html::_get_image_html(CtImage* image, const fs::path& images_dir, int& images_count, CtTreeIter* tree_iter)
{
    if (CtImageAnchor* imageAnchor = dynamic_cast<CtImageAnchor*>(image)) {
        return "<a name=\"" + imageAnchor->get_anchor_name() + "\"></a>";
    }
    images_count += 1;
    Glib::ustring image_name, image_rel_path;
    if (tree_iter) {
        image_name = std::to_string(tree_iter->get_node_id()) + "-" + std::to_string(images_count) + ".png";
        image_rel_path = (fs::path{"images"} / image_name).string_unix();
    }
    else {
        image_name = std::to_string(images_count) + ".png";
        image_rel_path = "file://" + (images_dir / image_name).string_unix();
    }

    Glib::ustring image_html = "<img src=\"" + image_rel_path + "\" alt=\"" + image_rel_path + "\" />";
    CtImagePng* png = dynamic_cast<CtImagePng*>(image);
    if (png and not png->get_link().empty()) {
        Glib::ustring href = _get_href_from_link_prop_val(png->get_link());
        image_html = "<a href=\"" + href + "\">" + image_html + "</a>";
    }

    image->save(images_dir / image_name, "png");
    return image_html;
}

// Returns the HTML CodeBox
Glib::ustring CtExport2Html::_get_codebox_html(CtCodebox* codebox)
{
    Glib::ustring codebox_html = "<div class=\"codebox\">";
    codebox_html += _html_get_from_code_buffer(codebox->get_buffer(), -1, -1, codebox->get_syntax_highlighting());
    codebox_html += "</div>";
    return codebox_html;
}

// Returns the HTML Table
Glib::ustring CtExport2Html::_get_table_html(CtTableCommon* table)
{
    std::vector<std::vector<Glib::ustring>> rows;
    table->write_strings_matrix(rows);
    Glib::ustring table_html = "<table class=\"table\">";
    bool first{true};
    for (const auto& row : rows) {
        table_html += "<tr>";
        for (const Glib::ustring& cell : row) {
            Glib::ustring content = str::xml_escape(cell);
            if (content.empty()) {
                content = " "; // Otherwise the table will render with squashed cells
            }
            if (first) {
                table_html += "<th>" + content + "</th>";
            } else {
                table_html += "<td>" + content + "</td>";
            }
        }
        if (first) {
            first = false;
        }
        table_html += "</tr>";
    }
    table_html += "</table>";
    return table_html;
}

// Get rich text from syntax highlighted code node
Glib::ustring CtExport2Html::_html_get_from_code_buffer(const Glib::RefPtr<Gsv::Buffer>& code_buffer, int sel_start, int sel_end, const std::string& syntax_highlighting)
{
    Gtk::TextIter curr_iter = sel_start >= 0 ? code_buffer->get_iter_at_offset(sel_start) : code_buffer->begin();
    Gtk::TextIter end_iter = sel_end >= 0 ? code_buffer->get_iter_at_offset(sel_end) : code_buffer->end();

    _pCtMainWin->apply_syntax_highlighting(code_buffer, syntax_highlighting, false/*forceReApply*/);
    code_buffer->ensure_highlight(curr_iter, end_iter);

    Glib::ustring html_text;
    Glib::ustring former_tag_str = CtConst::COLOR_48_BLACK;
    bool span_opened = false;
    for (;;) {
        auto curr_tags = curr_iter.get_tags();
        if (curr_tags.size() > 0) {
            Glib::ustring curr_tag_str = curr_tags[0]->property_foreground_gdk().get_value().to_string();
            int font_weight = curr_tags[0]->property_weight().get_value();
            if (curr_tag_str == CtConst::COLOR_48_BLACK) {
                if (former_tag_str != curr_tag_str) {
                    former_tag_str = curr_tag_str;
                    // end of tag
                    html_text += "</span>";
                    span_opened = false;
                }
            }
            else {
                if (former_tag_str != curr_tag_str) {
                    former_tag_str = curr_tag_str;
                    if (span_opened) html_text += "</span>";
                    // start of tag
                    Glib::ustring color = CtRgbUtil::rgb_to_no_white(curr_tag_str);
                    color = CtRgbUtil::get_rgb24str_from_str_any(color);
                    html_text += "<span style=\"color:" + color + ";font-weight:" + std::to_string(font_weight) + "\">";
                    span_opened = true;
                }
            }
        }
        else if (span_opened) {
            span_opened = false;
            former_tag_str = CtConst::COLOR_48_BLACK;
            html_text += "</span>";
        }
        Glib::ustring sym = str::xml_escape(Glib::ustring(1, curr_iter.get_char()));
        //html_text += str::replace(sym, " ", "&nbsp;");
        // let's try and use <pre></pre> instead of '&nbsp;' to preserve the spaces
        html_text += sym;
        if (!curr_iter.forward_char() || (sel_end >= 0 && curr_iter.get_offset() >= sel_end)) {
            if (span_opened) html_text += "</span>";
            break;
        }
    }

    html_text = str::replace(html_text, CtConst::CHAR_NEWLINE, "<br />");
    return "<pre>" + html_text + "</pre>";
}

// Given a treestore iter returns the HTML rich text
void CtExport2Html::_html_get_from_treestore_node(CtTreeIter node_iter,
                                                  int sel_start,
                                                  int sel_end,
                                                  std::vector<Glib::ustring>& out_slots,
                                                  std::vector<CtAnchoredWidget*>& out_widgets)
{
    auto curr_buffer = node_iter.get_node_text_buffer();
    auto widgets = node_iter.get_anchored_widgets(sel_start, sel_end);
    out_widgets = std::vector<CtAnchoredWidget*>(widgets.begin(), widgets.end()); // copy from list to vector

    out_slots.clear();
    int start_offset = sel_start == -1 ? 0 : sel_start;
    for (auto widget : out_widgets) {
        int end_offset = widget->getOffset();
        out_slots.push_back(_html_process_slot(start_offset, end_offset, curr_buffer));
        start_offset = end_offset;
    }
    if (sel_end == -1)
        out_slots.push_back(_html_process_slot(start_offset, -1, curr_buffer));
    else
        out_slots.push_back(_html_process_slot(start_offset, sel_end, curr_buffer));
}

// Process a Single HTML Slot
Glib::ustring CtExport2Html::_html_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer)
{
    Glib::ustring curr_html_text = "";
    CtTextIterUtil::SerializeFunc f_html_serialise = [&](Gtk::TextIter& start_iter,
                                                         Gtk::TextIter& curr_iter,
                                                         CtCurrAttributesMap& curr_attributes)
    {
        curr_html_text += _html_text_serialize(start_iter, curr_iter, curr_attributes);
    };
    CtTextIterUtil::generic_process_slot(start_offset, end_offset, curr_buffer, f_html_serialise);

    for (auto header : {CtConst::TAG_PROP_VAL_H1, CtConst::TAG_PROP_VAL_H2, CtConst::TAG_PROP_VAL_H3,
                        CtConst::TAG_PROP_VAL_H4, CtConst::TAG_PROP_VAL_H5, CtConst::TAG_PROP_VAL_H6}) {
        curr_html_text = str::replace(curr_html_text, ("</" + Glib::ustring{header} + "><" + Glib::ustring{header} + " >").c_str(), "");
    }
    return curr_html_text;
}

// Adds a slice to the HTML Text
Glib::ustring CtExport2Html::_html_text_serialize(Gtk::TextIter start_iter,
                                                  Gtk::TextIter end_iter,
                                                  const CtCurrAttributesMap& curr_attributes)
{
    Glib::ustring html_attrs;
    bool superscript_active{false};
    bool subscript_active{false};
    bool monospace_active{false};
    bool bold_active{false};
    bool italic_active{false};
    Glib::ustring hN_active;
    Glib::ustring href;
    for (auto tag_property : CtConst::TAG_PROPERTIES) {
        if (curr_attributes.at(tag_property).empty()) {
            continue;
        }
        Glib::ustring property_value = curr_attributes.at(tag_property);
        if (tag_property == CtConst::TAG_WEIGHT) {
            // font-weight:bolder
            // tag_property = "font-weight"
            // property_value = "bolder"
            bold_active = true;
            continue;
        }
        else if (tag_property == CtConst::TAG_FOREGROUND) {
            // color:#FFFF00
            tag_property = "color";
            Glib::ustring color_no_white = CtRgbUtil::rgb_to_no_white(property_value);
            property_value = CtRgbUtil::get_rgb24str_from_str_any(color_no_white);
        }
        else if (tag_property == CtConst::TAG_BACKGROUND) {
            // background-color:#FFFF00
            tag_property = "background-color";
            property_value = CtRgbUtil::get_rgb24str_from_str_any(property_value);
        }
        else if (tag_property == CtConst::TAG_STYLE) {
            // font-style:italic
            // tag_property = "font-style"
            // property_value = cons.TAG_PROP_ITALIC
            italic_active = true;
            continue;
        }
        else if (tag_property == CtConst::TAG_UNDERLINE) {
            // text-decoration:underline
            tag_property = "text-decoration";
            property_value = CtConst::TAG_UNDERLINE;
        }
        else if (tag_property == CtConst::TAG_STRIKETHROUGH) {
            // text-decoration:line-through
            tag_property = "text-decoration";
            property_value = "line-through";
        }
        else if (tag_property == CtConst::TAG_SCALE) {
            if (property_value == CtConst::TAG_PROP_VAL_SUP) {
                superscript_active = true;
                continue;
            }
            else if (property_value == CtConst::TAG_PROP_VAL_SUB) {
                subscript_active = true;
                continue;
            }
            else {
                // font-size:xx-large/x-large/x-small
                tag_property = "font-size";
                if (property_value == CtConst::TAG_PROP_VAL_SMALL) {
                    property_value = "x-small";
                }
                else if (property_value == CtConst::TAG_PROP_VAL_H1 or
                         property_value == CtConst::TAG_PROP_VAL_H2 or
                         property_value == CtConst::TAG_PROP_VAL_H3 or
                         property_value == CtConst::TAG_PROP_VAL_H4 or
                         property_value == CtConst::TAG_PROP_VAL_H5 or
                         property_value == CtConst::TAG_PROP_VAL_H6)
                {
                    // TODO apply user defined scalable tag properies
                    hN_active = property_value;
                }
            }
        }
        else if (tag_property == CtConst::TAG_FAMILY) {
            monospace_active = true;
            continue;
        }
        else if (tag_property == CtConst::TAG_JUSTIFICATION) {
            // text-align:center/left/right
            // tag_property = "text-align"
            continue;
        }
        else if (tag_property == CtConst::TAG_LINK) {
            // <a href="http://www.example.com/">link-text goes here</a>
            href = _get_href_from_link_prop_val(property_value);
            continue;
        }
        html_attrs += Glib::ustring{tag_property.data()} + ":" + property_value + ";";
    }

    // split by \n to support RTL lines
    Glib::ustring html_text;
    std::vector<Glib::ustring> lines = str::split(start_iter.get_text(end_iter), CtConst::CHAR_NEWLINE);
    const size_t lastIdx = lines.size() - 1;
    for (size_t i = 0; i < lines.size(); ++i) {
        Glib::ustring tagged_text = str::xml_escape(lines[i]);

        if (not hN_active.empty()) {
            tagged_text = Glib::ustring{"<"} + hN_active + ">" + tagged_text + Glib::ustring{"</"} + hN_active + ">";
        }
        else if (not href.empty()) {
            tagged_text = "<a href=\"" + href + "\">" + tagged_text + "</a>";
        }
        else if (not html_attrs.empty()) {
            if (html_attrs.find("x-small") != Glib::ustring::npos) {
                tagged_text = "<small>" + tagged_text + "</small>";
            }
            else {
                tagged_text = "<span style=\"" + html_attrs + "\">" + tagged_text + "</span>";
            }
        }
        if (superscript_active) tagged_text = "<sup>" + tagged_text + "</sup>";
        if (subscript_active) tagged_text = "<sub>" + tagged_text + "</sub>";
        if (monospace_active) tagged_text = "<code>" + tagged_text + "</code>";
        if (bold_active) tagged_text = "<strong>" + tagged_text + "</strong>";
        if (italic_active) tagged_text = "<em>" + tagged_text + "</em>";
        html_text += tagged_text;

        // add '\n' between lines
        if (i < lastIdx) {
            html_text += CtConst::CHAR_NEWLINE;
        }
    }
    return html_text;
}

std::string CtExport2Html::_get_href_from_link_prop_val(Glib::ustring link_prop_val)
{
    CtLinkEntry link_entry = CtMiscUtil::get_link_entry(link_prop_val);
    if (link_entry.type == "")
        return "";

    std::string href = "";
    if (link_entry.type == CtConst::LINK_TYPE_WEBS)
        href = link_entry.webs;
    else if (link_entry.type == CtConst::LINK_TYPE_FILE)
        href = "file://" + link_process_filepath(link_entry.file, _pCtMainWin->get_ct_storage()->get_file_path().parent_path().string(), true/*forHtml*/);
    else if (link_entry.type == CtConst::LINK_TYPE_FOLD)
        href = "file://" + link_process_folderpath(link_entry.fold, _pCtMainWin->get_ct_storage()->get_file_path().parent_path().string(), true/*forHtml*/);
    else if (link_entry.type == CtConst::LINK_TYPE_NODE)
    {
        CtTreeIter node = _pCtMainWin->get_tree_store().get_node_from_node_id(link_entry.node_id);
        if (node)
        {
            href = _get_html_filename(node);
            if (!link_entry.anch.empty())
                href += "#" + link_entry.anch;
        }
    }
    return href;
}

std::string CtExport2Html::link_process_filepath(const std::string& filepath_raw, const std::string& relative_to, const bool forHtml)
{
    fs::path filepath = filepath_raw;
    fs::path abs_filepath = fs::canonical(filepath, relative_to);
    if (!fs::is_regular_file(filepath) && fs::is_regular_file(abs_filepath))
        filepath = abs_filepath;
    return forHtml ? filepath.string_unix() : filepath.string();
}

std::string CtExport2Html::link_process_folderpath(const std::string& folderpath_raw, const std::string& relative_to, const bool forHtml)
{
    fs::path folderpath = folderpath_raw;
    fs::path abs_folderpath = fs::canonical(folderpath, relative_to);
    if (!fs::is_directory(folderpath) && fs::is_directory(abs_folderpath))
        folderpath = abs_folderpath;
    return forHtml ? folderpath.string_unix() : folderpath.string();
}

// Returns the style attribute(s) according to the alignment
Glib::ustring CtExport2Html::_get_object_alignment_string(Glib::ustring alignment)
{
    if (alignment == CtConst::TAG_PROP_VAL_CENTER) return "margin-left:auto;margin-right:auto";
    if (alignment == CtConst::TAG_PROP_VAL_RIGHT) return "margin-left:auto";
    return "display:inline-table";
}

// Get the HTML page filename given the tree iter
Glib::ustring CtExport2Html::_get_html_filename(CtTreeIter tree_iter)
{
    Glib::ustring name = CtMiscUtil::get_node_hierarchical_name(tree_iter, "--"/*separator*/,
        true/*for_filename*/, true/*root_to_leaf*/, true/*trail_node_id*/, ".html"/*trailer*/);
    return str::replace(name, "#", "~");
}
