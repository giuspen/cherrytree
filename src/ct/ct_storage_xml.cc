/*
 * ct_storage_xml.cc
 *
 * Copyright 2009-2023
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

#include "ct_storage_xml.h"
#include "ct_const.h"
#include "ct_misc_utils.h"
#include <libxml++/libxml++.h>
#include <libxml2/libxml/parser.h>
#include "ct_image.h"
#include "ct_codebox.h"
#include "ct_table.h"
#include "ct_main_win.h"
#include "ct_storage_control.h"
#include "ct_storage_multifile.h"
#include "ct_logging.h"

bool CtStorageXml::populate_treestore(const fs::path& file_path, Glib::ustring& error)
{
    try {
        // open file
        std::unique_ptr<xmlpp::DomParser> parser = CtStorageXml::get_parser(file_path);

        CtTreeStore& ct_tree_store = _pCtMainWin->get_tree_store();

        // load bookmarks
        for (xmlpp::Node* xml_node : parser->get_document()->get_root_node()->get_children("bookmarks")) {
            Glib::ustring bookmarks_csv = static_cast<xmlpp::Element*>(xml_node)->get_attribute_value("list");
            for (const auto nodeId : CtStrUtil::gstring_split_to_int64(bookmarks_csv.c_str(), ",")) {
                if (not _isDryRun) {
                    ct_tree_store.bookmarks_add(nodeId);
                }
            }
        }

        // load node tree
        std::list<CtTreeIter> nodes_with_duplicated_id;
        std::function<void(xmlpp::Element*, const gint64, Gtk::TreeIter)> nodes_from_xml;
        nodes_from_xml = [&](xmlpp::Element* xml_element, const gint64 sequence, Gtk::TreeIter parent_iter) {
            bool has_duplicated_id{false};
            Gtk::TreeIter new_iter = CtStorageXmlHelper{_pCtMainWin}.node_from_xml(xml_element, sequence, parent_iter, -1, &has_duplicated_id, _delayed_text_buffers, _isDryRun, "");
            if (has_duplicated_id and not _isDryRun) {
                nodes_with_duplicated_id.push_back(ct_tree_store.to_ct_tree_iter(new_iter));
            }
            gint64 child_sequence{0};
            for (xmlpp::Node* xml_node : xml_element->get_children("node")) {
                nodes_from_xml(static_cast<xmlpp::Element*>(xml_node), ++child_sequence, new_iter);
            }
        };
        gint64 sequence{0};
        for (xmlpp::Node* xml_node : parser->get_document()->get_root_node()->get_children("node")) {
            nodes_from_xml(static_cast<xmlpp::Element*>(xml_node), ++sequence, Gtk::TreeIter{});
        }

        // fixes duplicated ids by setting new ids
        for (auto& node : nodes_with_duplicated_id) {
            node.set_node_id(ct_tree_store.node_id_get());
        }
        return true;
    }
    catch (std::exception& e) {
        error = e.what();
        return false;
    }
}

bool CtStorageXml::save_treestore(const fs::path& file_path,
                                  const CtStorageSyncPending&,
                                  Glib::ustring& error,
                                  const CtExporting exporting,
                                  const int start_offset/*= 0*/,
                                  const int end_offset/*=-1*/)
{
    try {
        xmlpp::Document xml_doc;
        xml_doc.create_root_node(CtConst::APP_NAME);

        if ( CtExporting::NONESAVE == exporting or
             CtExporting::NONESAVEAS == exporting or
             CtExporting::ALL_TREE == exporting )
        {
            // save bookmarks
            xmlpp::Element* p_bookmarks_node = xml_doc.get_root_node()->add_child("bookmarks");
            p_bookmarks_node->set_attribute("list", str::join_numbers(_pCtMainWin->get_tree_store().bookmarks_get(), ","));
        }

        CtStorageCache storage_cache;
        storage_cache.generate_cache(_pCtMainWin, nullptr, true/*for_xml*/);

        // save nodes
        if ( CtExporting::NONESAVE == exporting or
             CtExporting::NONESAVEAS == exporting or
             CtExporting::ALL_TREE == exporting )
        {
            auto ct_tree_iter = _pCtMainWin->get_tree_store().get_ct_iter_first();
            while (ct_tree_iter) {
                _nodes_to_xml(&ct_tree_iter, xml_doc.get_root_node(), &storage_cache, exporting, start_offset, end_offset);
                ++ct_tree_iter;
            }
        }
        else {
            CtTreeIter ct_tree_iter = _pCtMainWin->curr_tree_iter();
            _nodes_to_xml(&ct_tree_iter, xml_doc.get_root_node(), &storage_cache, exporting, start_offset, end_offset);
        }

        // write file
        xml_doc.write_to_file_formatted(file_path.string());

        return true;
    }
    catch (std::exception& e) {
        error = e.what();
        return false;
    }
}

void CtStorageXml::import_nodes(const fs::path& filepath, const Gtk::TreeIter& parent_iter)
{
    std::unique_ptr<xmlpp::DomParser> parser = CtStorageXml::get_parser(filepath);

    CtTreeStore& ct_tree_store = _pCtMainWin->get_tree_store();

    std::function<void(xmlpp::Element*, const gint64 sequence, Gtk::TreeIter)> recursive_import_func;
    recursive_import_func = [&](xmlpp::Element* xml_element, const gint64 sequence, Gtk::TreeIter parent_iter) {
        Gtk::TreeIter new_iter = CtStorageXmlHelper{_pCtMainWin}.node_from_xml(xml_element, sequence, parent_iter, ct_tree_store.node_id_get(), nullptr/*pHasDuplicatedId*/, _delayed_text_buffers, _isDryRun, "");
        CtTreeIter new_ct_iter = ct_tree_store.to_ct_tree_iter(new_iter);
        new_ct_iter.pending_new_db_node();

        gint64 child_sequence{0};
        for (xmlpp::Node* xml_node : xml_element->get_children("node")) {
            recursive_import_func(static_cast<xmlpp::Element*>(xml_node), ++child_sequence, new_iter);
        }
    };

    gint64 sequence{0};
    for (xmlpp::Node* xml_node : parser->get_document()->get_root_node()->get_children("node")) {
        recursive_import_func(static_cast<xmlpp::Element*>(xml_node), ++sequence, ct_tree_store.to_ct_tree_iter(parent_iter));
    }
}

Glib::RefPtr<Gsv::Buffer> CtStorageXml::get_delayed_text_buffer(const gint64& node_id,
                                                                const std::string& syntax,
                                                                std::list<CtAnchoredWidget*>& widgets) const
{
    if (_delayed_text_buffers.count(node_id) == 0) {
        spdlog::error("!! {} node_id {}", __FUNCTION__, node_id);
        return Glib::RefPtr<Gsv::Buffer>{};
    }
    std::shared_ptr<xmlpp::Document> node_buffer = _delayed_text_buffers[node_id];
    auto xml_element = dynamic_cast<xmlpp::Element*>(node_buffer->get_root_node()->get_first_child());
    auto ret_buffer = CtStorageXmlHelper{_pCtMainWin}.create_buffer_and_widgets_from_xml(xml_element, syntax, widgets, nullptr, -1, "");
    if (ret_buffer) {
        _delayed_text_buffers.erase(node_id);
    }
    return ret_buffer;
}

void CtStorageXml::_nodes_to_xml(CtTreeIter* ct_tree_iter,
                                 xmlpp::Element* p_node_parent,
                                 CtStorageCache* storage_cache,
                                 const CtExporting exporting,
                                 const int start_offset/*= 0*/,
                                 const int end_offset/*=-1*/)
{
    Glib::RefPtr<Gsv::Buffer> rTextBuffer = ct_tree_iter->get_node_text_buffer();
    if (not rTextBuffer) {
        throw std::runtime_error(str::format(_("Failed to retrieve the content of the node '%s'"), ct_tree_iter->get_node_name()));
    }
    xmlpp::Element* p_node_node =  CtStorageXmlHelper{_pCtMainWin}.node_to_xml(
        ct_tree_iter,
        p_node_parent,
        std::string{}/*multifile_dir*/,
        storage_cache,
        start_offset,
        end_offset
    );
    if ( CtExporting::CURRENT_NODE != exporting and
         CtExporting::SELECTED_TEXT != exporting )
    {
        CtTreeIter ct_tree_iter_child = ct_tree_iter->first_child();
        while (ct_tree_iter_child) {
            _nodes_to_xml(&ct_tree_iter_child, p_node_node, storage_cache, exporting, start_offset, end_offset);
            ++ct_tree_iter_child;
        }
    }
}

/*static*/std::unique_ptr<xmlpp::DomParser> CtStorageXml::get_parser(const fs::path& file_path)
{
    // open file
    auto parser = std::make_unique<xmlpp::DomParser>();
    bool parseOk{true};
    try {
        parser->set_parser_options(xmlParserOption::XML_PARSE_HUGE);
        parser->parse_file(file_path.string());
    }
    catch (xmlpp::exception& e) {
        spdlog::error("{} {} {}", __FUNCTION__, file_path.string(), e.what());

        std::string buffer = Glib::file_get_contents(file_path.string());
        CtStrUtil::convert_if_not_utf8(buffer, true/*sanitise*/);
        parseOk = CtXmlHelper::safe_parse_memory(*parser, buffer);
    }

    if (not parseOk) {
        throw std::runtime_error("xml parse fail");
    }
    if (not parser->get_document()) {
        throw std::runtime_error("document is null");
    }
    if (parser->get_document()->get_root_node()->get_name() != CtConst::APP_NAME) {
        throw std::runtime_error("document contains the wrong node root");
    }
    return parser;
}

xmlpp::Element* CtStorageXmlHelper::node_to_xml(const CtTreeIter* ct_tree_iter,
                                                xmlpp::Element* p_node_parent,
                                                const std::string& multifile_dir,
                                                CtStorageCache* storage_cache,
                                                const int start_offset/*= 0*/,
                                                const int end_offset/*=-1*/)
{
    xmlpp::Element* p_node_node = p_node_parent->add_child("node");
    p_node_node->set_attribute("name", ct_tree_iter->get_node_name());
    p_node_node->set_attribute("unique_id", std::to_string(ct_tree_iter->get_node_id()));
    p_node_node->set_attribute("prog_lang", ct_tree_iter->get_node_syntax_highlighting());
    p_node_node->set_attribute("tags", ct_tree_iter->get_node_tags());
    p_node_node->set_attribute("readonly", std::to_string(ct_tree_iter->get_node_read_only()));
    p_node_node->set_attribute("nosearch_me", std::to_string(ct_tree_iter->get_node_is_excluded_from_search()));
    p_node_node->set_attribute("nosearch_ch", std::to_string(ct_tree_iter->get_node_children_are_excluded_from_search()));
    p_node_node->set_attribute("custom_icon_id", std::to_string(ct_tree_iter->get_node_custom_icon_id()));
    p_node_node->set_attribute("is_bold", std::to_string(ct_tree_iter->get_node_is_bold()));
    p_node_node->set_attribute("foreground", ct_tree_iter->get_node_foreground());
    p_node_node->set_attribute("ts_creation", std::to_string(ct_tree_iter->get_node_creating_time()));
    p_node_node->set_attribute("ts_lastsave", std::to_string(ct_tree_iter->get_node_modification_time()));

    Glib::RefPtr<Gsv::Buffer> buffer = ct_tree_iter->get_node_text_buffer();
    save_buffer_no_widgets_to_xml(p_node_node, buffer, start_offset, end_offset, 'n');

    for (CtAnchoredWidget* pAnchoredWidget : ct_tree_iter->get_anchored_widgets(start_offset, end_offset)) {
        pAnchoredWidget->to_xml(p_node_node, start_offset > 0 ? -start_offset : 0, storage_cache, multifile_dir);
    }
    return p_node_node;
}

Gtk::TreeIter CtStorageXmlHelper::node_from_xml(const xmlpp::Element* xml_element,
                                                const gint64 sequence,
                                                const Gtk::TreeIter parent_iter,
                                                const gint64 new_id,
                                                bool* pHasDuplicatedId,
                                                CtDelayedTextBufferMap& delayed_text_buffers,
                                                const bool isDryRun,
                                                const std::string& multifile_dir)
{
    if (pHasDuplicatedId) {
        *pHasDuplicatedId = false;
    }
    CtNodeData node_data;
    if (-1 == new_id) {
        // use the id found in the xml
        node_data.nodeId = CtStrUtil::gint64_from_gstring(xml_element->get_attribute_value("unique_id").c_str());
    }
    else {
        // use the passed new_id
        node_data.nodeId = new_id;
    }
    node_data.name = xml_element->get_attribute_value("name");
    node_data.syntax = xml_element->get_attribute_value("prog_lang");
    node_data.tags = xml_element->get_attribute_value("tags");
    node_data.isReadOnly = CtStrUtil::is_str_true(xml_element->get_attribute_value("readonly"));
    node_data.excludeMeFromSearch = CtStrUtil::is_str_true(xml_element->get_attribute_value("nosearch_me"));
    node_data.excludeChildrenFromSearch = CtStrUtil::is_str_true(xml_element->get_attribute_value("nosearch_ch"));
    node_data.customIconId = (guint32)CtStrUtil::gint64_from_gstring(xml_element->get_attribute_value("custom_icon_id").c_str());
    node_data.isBold = CtStrUtil::is_str_true(xml_element->get_attribute_value("is_bold"));
    node_data.foregroundRgb24 = xml_element->get_attribute_value("foreground");
    node_data.tsCreation = CtStrUtil::gint64_from_gstring(xml_element->get_attribute_value("ts_creation").c_str());
    node_data.tsLastSave = CtStrUtil::gint64_from_gstring(xml_element->get_attribute_value("ts_lastsave").c_str());
    node_data.sequence = sequence;

    if (isDryRun) {
        return Gtk::TreeIter{};
    }

    if (-1 == new_id) {
        // use the id found in the xml
        if (delayed_text_buffers.count(node_data.nodeId) != 0) {
            spdlog::debug("node has duplicated id {}, will be fixed", node_data.nodeId);
            if (pHasDuplicatedId) *pHasDuplicatedId = true;
            // create buffer now because we cannot put a duplicate id in _delayed_text_buffers
            // the id will be fixed on top level code
            node_data.rTextBuffer = create_buffer_and_widgets_from_xml(xml_element, node_data.syntax, node_data.anchoredWidgets, nullptr, -1, multifile_dir);
        }
        else {
            // because of widgets which are slow to insert for now, delay creating buffers
            // save node data in a separate document
            auto node_buffer = std::make_shared<xmlpp::Document>();
            node_buffer->create_root_node("root")->import_node(xml_element);
            delayed_text_buffers[node_data.nodeId] = node_buffer;
        }
    }
    else {
        // (use the passed new_id)
        // create buffer now because imported document will be closed
        node_data.rTextBuffer = create_buffer_and_widgets_from_xml(xml_element, node_data.syntax, node_data.anchoredWidgets, nullptr, -1, multifile_dir);
    }
    return _pCtMainWin->get_tree_store().append_node(&node_data, &parent_iter);
}

Glib::RefPtr<Gsv::Buffer> CtStorageXmlHelper::create_buffer_and_widgets_from_xml(const xmlpp::Element* parent_xml_element,
                                                                                 const Glib::ustring&/*syntax*/,
                                                                                 std::list<CtAnchoredWidget*>& widgets,
                                                                                 Gtk::TextIter* text_insert_pos,
                                                                                 const int force_offset,
                                                                                 const std::string& multifile_dir)
{
    Glib::RefPtr<Gsv::Buffer> pBuffer = _pCtMainWin->get_new_text_buffer();
    bool error{false};
    pBuffer->begin_not_undoable_action();
    for (xmlpp::Node* xml_slot : parent_xml_element->get_children()) {
        if (not get_text_buffer_one_slot_from_xml(pBuffer, xml_slot, widgets, text_insert_pos, force_offset, multifile_dir)) {
            error = true;
            break;
        }
    }
    pBuffer->end_not_undoable_action();
    pBuffer->set_modified(false);
    return error ? Glib::RefPtr<Gsv::Buffer>{} : pBuffer;
}

bool CtStorageXmlHelper::get_text_buffer_one_slot_from_xml(Glib::RefPtr<Gsv::Buffer> buffer,
                                                           xmlpp::Node* slot_node,
                                                           std::list<CtAnchoredWidget*>& widgets,
                                                           Gtk::TextIter* text_insert_pos,
                                                           const int force_offset,
                                                           const std::string& multifile_dir)
{
    xmlpp::Element* slot_element = static_cast<xmlpp::Element*>(slot_node);
    Glib::ustring slot_element_name = slot_element->get_name();

    enum class SlotType {None, RichText, Image, Table, Codebox};
    SlotType slot_type{SlotType::None};
    if (slot_element_name == "rich_text")        slot_type = SlotType::RichText;
    else if (slot_element_name == "encoded_png") slot_type = SlotType::Image;
    else if (slot_element_name == "table")       slot_type = SlotType::Table;
    else if (slot_element_name == "codebox")     slot_type = SlotType::Codebox;

    if (SlotType::RichText == slot_type) {
        _add_rich_text_from_xml(buffer, slot_element, text_insert_pos);
    }
    else if (SlotType::None != slot_type) {
        const int char_offset = -1 != force_offset ? force_offset : std::stoi(slot_element->get_attribute_value("char_offset"));
        Glib::ustring justification = slot_element->get_attribute_value(CtConst::TAG_JUSTIFICATION);
        if (justification.empty()) justification = CtConst::TAG_PROP_VAL_LEFT;

        CtAnchoredWidget* widget{nullptr};
        if (SlotType::Image == slot_type) {
            widget = _create_image_from_xml(slot_element, char_offset, justification, multifile_dir);
            if (not widget) {
                return false;
            }
        }
        else if (SlotType::Table == slot_type) {
            widget = _create_table_from_xml(slot_element, char_offset, justification);
        }
        else if (SlotType::Codebox == slot_type) {
            widget = _create_codebox_from_xml(slot_element, char_offset, justification);
        }
        if (widget) {
            widget->insertInTextBuffer(buffer);
            widgets.push_back(widget);
        }
    }
    return true;
}

Glib::RefPtr<Gsv::Buffer> CtStorageXmlHelper::create_buffer_no_widgets(const Glib::ustring& syntax, const char* xml_content)
{
    xmlpp::DomParser parser;
    std::list<CtAnchoredWidget*> widgets;
    if (CtXmlHelper::safe_parse_memory(parser, xml_content)) {
        return create_buffer_and_widgets_from_xml(parser.get_document()->get_root_node(), syntax, widgets, nullptr, -1, "");
    }
    return Glib::RefPtr<Gsv::Buffer>{};
}

bool CtStorageXmlHelper::populate_table_matrix(CtTableMatrix& tableMatrix,
                                               const char* xml_content,
                                               CtTableColWidths& tableColWidths,
                                               bool& is_light)
{
    xmlpp::DomParser parser;
    if (CtXmlHelper::safe_parse_memory(parser, xml_content)) {
        populate_table_matrix(tableMatrix,
                              parser.get_document()->get_root_node(),
                              tableColWidths,
                              is_light);
        return true;
    }
    return false;
}

void CtStorageXmlHelper::populate_table_matrix(CtTableMatrix& tableMatrix,
                                               xmlpp::Element* xml_element,
                                               CtTableColWidths& tableColWidths,
                                               bool& is_light)
{
    const Glib::ustring isLightStr = xml_element->get_attribute_value("is_light");
    if (not isLightStr.empty()) {
        is_light = CtStrUtil::is_str_true(isLightStr);
    }
    for (xmlpp::Node* pNodeRow : xml_element->get_children("row")) {
        tableMatrix.push_back(CtTableRow{});
        for (xmlpp::Node* pNodeCell : pNodeRow->get_children("cell")) {
            xmlpp::TextNode* pTextNode = static_cast<xmlpp::Element*>(pNodeCell)->get_child_text();
            const Glib::ustring textContent = pTextNode ? pTextNode->get_content() : "";
            if (is_light) {
                tableMatrix.back().push_back(new Glib::ustring{textContent});
            }
            else {
                tableMatrix.back().push_back(new CtTextCell{_pCtMainWin, textContent, CtConst::TABLE_CELL_TEXT_ID});
            }
        }
    }
    tableMatrix.insert(tableMatrix.begin(), tableMatrix.back());
    tableMatrix.pop_back();
    const Glib::ustring colWidthsStr = xml_element->get_attribute_value("col_widths");
    if (not colWidthsStr.empty()) {
        tableColWidths = CtStrUtil::gstring_split_to_int(colWidthsStr.c_str(), ",");
    }
}

void CtStorageXmlHelper::save_buffer_no_widgets_to_xml(xmlpp::Element* p_node_parent,
                                                       Glib::RefPtr<Gtk::TextBuffer> rBuffer,
                                                       int start_offset,
                                                       int end_offset,
                                                       const gchar change_case)
{
    CtTextIterUtil::SerializeFunc rich_txt_serialize = [&](Gtk::TextIter& start_iter,
                                                           Gtk::TextIter& end_iter,
                                                           CtCurrAttributesMap& curr_attributes,
                                                           CtListInfo*/*pCurrListInfo*/)
    {
        xmlpp::Element* p_rich_text_node = p_node_parent->add_child("rich_text");
        for (const auto& map_iter : curr_attributes) {
            if (not map_iter.second.empty()) {
                p_rich_text_node->set_attribute(map_iter.first.data(), map_iter.second);
            }
        }
        Glib::ustring slot_text = start_iter.get_text(end_iter);
        if ('n' != change_case) {
            if ('l' == change_case) slot_text = slot_text.lowercase();
            else if ('u' == change_case) slot_text = slot_text.uppercase();
            else if ('t' == change_case) slot_text = str::swapcase(slot_text);
        }
        p_rich_text_node->add_child_text(slot_text);
    };

    CtTextIterUtil::generic_process_slot(_pCtMainWin->get_ct_config(), start_offset, end_offset, rBuffer, rich_txt_serialize);
}

void CtStorageXmlHelper::_add_rich_text_from_xml(Glib::RefPtr<Gsv::Buffer> buffer, xmlpp::Element* xml_element, Gtk::TextIter* text_insert_pos)
{
    xmlpp::TextNode* text_node = xml_element->get_child_text();
    if (not text_node) return;
    const Glib::ustring text_content = text_node->get_content();
    if (text_content.empty()) return;
    std::vector<Glib::ustring> tags;
    for (const xmlpp::Attribute* pAttribute : xml_element->get_attributes()) {
        if (CtStrUtil::contains(CtConst::TAG_PROPERTIES, pAttribute->get_name().c_str())) {
            tags.push_back(_pCtMainWin->get_text_tag_name_exist_or_create(pAttribute->get_name(), pAttribute->get_value()));
        }
    }
    Gtk::TextIter iter = text_insert_pos ? *text_insert_pos : buffer->end();
    if (tags.size() > 0)
        buffer->insert_with_tags_by_name(iter, text_content, tags);
    else
        buffer->insert(iter, text_content);
}

CtAnchoredWidget* CtStorageXmlHelper::_create_image_from_xml(xmlpp::Element* xml_element,
                                                             int charOffset,
                                                             const Glib::ustring& justification,
                                                             const std::string& multifile_dir)
{
    const Glib::ustring anchorName = xml_element->get_attribute_value("anchor");
    if (not anchorName.empty()) {
        return new CtImageAnchor{_pCtMainWin, anchorName, charOffset, justification};
    }
    fs::path file_name = static_cast<std::string>(xml_element->get_attribute_value("filename"));
    xmlpp::TextNode* pTextNode = xml_element->get_child_text();
    const std::string encodedBlob = pTextNode ? pTextNode->get_content() : "";
    if (file_name == CtImageLatex::LatexSpecialFilename) {
        return new CtImageLatex{_pCtMainWin, encodedBlob, charOffset, justification, CtImageEmbFile::get_next_unique_id()};
    }
    std::string rawBlob;
    if (multifile_dir.empty()) {
        rawBlob = Glib::Base64::decode(encodedBlob);
    }
    else {
        const std::string sha256sum = xml_element->get_attribute_value("sha256sum");
        if (not CtStorageMultiFile::read_blob(multifile_dir, sha256sum, rawBlob)) {
            spdlog::warn("!! unexp not found {} in {}", sha256sum, multifile_dir);
            return nullptr;
        }
    }
    if (not file_name.empty()) {
        std::string timeStr = xml_element->get_attribute_value("time");
        if (timeStr.empty()) {
            timeStr = "0";
        }
        const time_t timeInt = std::stoll(timeStr);
        return new CtImageEmbFile{_pCtMainWin, file_name, rawBlob, timeInt, charOffset, justification, CtImageEmbFile::get_next_unique_id()};
    }
    const Glib::ustring link = xml_element->get_attribute_value("link");
    return new CtImagePng{_pCtMainWin, rawBlob, link, charOffset, justification};
}

CtAnchoredWidget* CtStorageXmlHelper::_create_codebox_from_xml(xmlpp::Element* xml_element,
                                                               int charOffset,
                                                               const Glib::ustring& justification)
{
    xmlpp::TextNode* pTextNode = xml_element->get_child_text();
    const Glib::ustring textContent = pTextNode ? pTextNode->get_content() : "";
    const Glib::ustring syntaxHighlighting = xml_element->get_attribute_value("syntax_highlighting");
    const int frameWidth = std::stoi(xml_element->get_attribute_value("frame_width"));
    const int frameHeight = std::stoi(xml_element->get_attribute_value("frame_height"));
    const bool widthInPixels = CtStrUtil::is_str_true(xml_element->get_attribute_value("width_in_pixels"));
    const bool highlightBrackets = CtStrUtil::is_str_true(xml_element->get_attribute_value("highlight_brackets"));
    const bool showLineNumbers = CtStrUtil::is_str_true(xml_element->get_attribute_value("show_line_numbers"));

    return new CtCodebox{_pCtMainWin,
                         textContent,
                         syntaxHighlighting,
                         frameWidth,
                         frameHeight,
                         charOffset,
                         justification,
                         widthInPixels,
                         highlightBrackets,
                         showLineNumbers};
}

CtAnchoredWidget* CtStorageXmlHelper::_create_table_from_xml(xmlpp::Element* xml_element,
                                                             int charOffset,
                                                             const Glib::ustring& justification)
{
    const int colWidthDefault = std::stoi(xml_element->get_attribute_value("col_max"));

    CtTableMatrix tableMatrix;
    CtTableColWidths tableColWidths;
    bool is_light{false};
    populate_table_matrix(tableMatrix, xml_element, tableColWidths, is_light);
    if (is_light) {
        return new CtTableLight{_pCtMainWin, tableMatrix, colWidthDefault, charOffset, justification, tableColWidths};
    }
    return new CtTableHeavy{_pCtMainWin, tableMatrix, colWidthDefault, charOffset, justification, tableColWidths};
}

void CtXmlHelper::table_to_xml(xmlpp::Element* p_parent,
                               const std::vector<std::vector<Glib::ustring>>& rows,
                               const int char_offset,
                               const Glib::ustring justification,
                               const int defaultWidth,
                               const Glib::ustring colWidths,
                               const bool is_light)
{
    xmlpp::Element* p_table_node = p_parent->add_child("table");
    p_table_node->set_attribute("char_offset", std::to_string(char_offset));
    p_table_node->set_attribute(CtConst::TAG_JUSTIFICATION, justification);
    p_table_node->set_attribute("col_min", std::to_string(defaultWidth)); // todo get rid of column min
    p_table_node->set_attribute("col_max", std::to_string(defaultWidth));
    p_table_node->set_attribute("col_widths", colWidths);
    if (is_light) {
        p_table_node->set_attribute("is_light", "1");
    }

    auto row_to_xml = [&](const std::vector<Glib::ustring>& tableRow) {
        xmlpp::Element* row_element = p_table_node->add_child("row");
        for (const auto& cell : tableRow) {
            xmlpp::Element* cell_element = row_element->add_child("cell");
            cell_element->set_child_text(cell);
        }
    };

    // put header at the end
    bool is_header = true;
    for (auto& row : rows) {
        if (is_header) { is_header = false; }
        else row_to_xml(row);
    }
    row_to_xml(rows[0]);
}

bool CtXmlHelper::safe_parse_memory(xmlpp::DomParser& parser, const Glib::ustring& xml_content)
{
    bool parseOk{false};
    try {
        parser.parse_memory(xml_content);
        parseOk = true;
    }
    catch (xmlpp::parse_error& e) {
        spdlog::error("{} [1] {}", __FUNCTION__, e.what());
    }
    if (not parseOk) {
        g_autofree gchar* pMadeValid = g_utf8_make_valid(xml_content.c_str(), xml_content.bytes());
        try {
            parser.parse_memory(pMadeValid);
            parseOk = true;
        }
        catch (xmlpp::parse_error& e) {
            spdlog::error("{} [2] {}", __FUNCTION__, e.what());
        }
        if (not parseOk) {
            auto sanitised = str::sanitize_bad_symbols(pMadeValid);
            try {
                parser.parse_memory(sanitised);
                parseOk = true;
            }
            catch (xmlpp::parse_error& e) {
                spdlog::error("{} [3] {}", __FUNCTION__, e.what());
            }
            if (not parseOk) {
                return false;
            }
        }
    }
    return parser.get_document() and parser.get_document()->get_root_node();
}
