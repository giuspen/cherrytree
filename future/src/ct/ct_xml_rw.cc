/*
 * ct_xml_rw.cc
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

#include <iostream>
#include <assert.h>
#include "ct_doc_rw.h"
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_app.h"
#include "ct_codebox.h"
#include "ct_image.h"
#include "ct_table.h"

CtXmlRead::CtXmlRead(const char* filepath, const char* textContent)
{
    if (nullptr != filepath)
    {
        parse_file(filepath);
    }
    else if (nullptr != textContent)
    {
        parse_memory(textContent);
    }
}

CtXmlRead::~CtXmlRead()
{
}

void CtXmlRead::treeWalk(const Gtk::TreeIter* pParentIter)
{
    xmlpp::Document* pDocument = get_document();
    assert(nullptr != pDocument);
    xmlpp::Element* pRoot = pDocument->get_root_node();
    assert(CtConst::APP_NAME == pRoot->get_name());
    for (xmlpp::Node* pNode : pRoot->get_children())
    {
        if ("node" == pNode->get_name())
        {
            _xmlTreeWalkIter(static_cast<xmlpp::Element*>(pNode), pParentIter);
        }
        else if ("bookmarks" == pNode->get_name())
        {
            Glib::ustring bookmarks_csv = static_cast<xmlpp::Element*>(pNode)->get_attribute_value("list");
            for (gint64& nodeId : CtStrUtil::gstringSplit2int64(bookmarks_csv.c_str(), ","))
            {
                signalAddBookmark.emit(nodeId);
            }
        }
    }
}

void CtXmlRead::_xmlTreeWalkIter(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter)
{
    Gtk::TreeIter newIter = _xmlNodeProcess(pNodeElement, pParentIter);

    for (xmlpp::Node* pNode : pNodeElement->get_children())
    {
        if ("node" == pNode->get_name())
        {
            _xmlTreeWalkIter(static_cast<xmlpp::Element*>(pNode), &newIter);
        }
    }
}

Gtk::TreeIter CtXmlRead::_xmlNodeProcess(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter)
{
    CtNodeData nodeData;
    nodeData.nodeId = CtStrUtil::gint64FromGstring(pNodeElement->get_attribute_value("unique_id").c_str());
    nodeData.name = pNodeElement->get_attribute_value("name");
    nodeData.syntax = pNodeElement->get_attribute_value("prog_lang");
    nodeData.tags = pNodeElement->get_attribute_value("tags");
    nodeData.isRO = Glib::str_has_prefix(pNodeElement->get_attribute_value("readonly"), "T");
    nodeData.customIconId = CtStrUtil::gint64FromGstring(pNodeElement->get_attribute_value("custom_icon_id").c_str());
    nodeData.isBold = Glib::str_has_prefix(pNodeElement->get_attribute_value("is_bold"), "T");
    nodeData.foregroundRgb24 = pNodeElement->get_attribute_value("foreground");
    nodeData.tsCreation = CtStrUtil::gint64FromGstring(pNodeElement->get_attribute_value("ts_creation").c_str());
    nodeData.tsLastSave = CtStrUtil::gint64FromGstring(pNodeElement->get_attribute_value("ts_lastSave").c_str());
    nodeData.rTextBuffer = getTextBuffer(nodeData.syntax, nodeData.anchoredWidgets, pNodeElement);

    Gtk::TreeIter newIter = signalAppendNode.emit(&nodeData, pParentIter);
    return newIter;
}

CtXmlNodeType CtXmlRead::_xmlNodeGetTypeFromName(const Glib::ustring& xmlNodeName)
{
    CtXmlNodeType retXmlNodeType{CtXmlNodeType::None};
    if ("rich_text" == xmlNodeName)
    {
        retXmlNodeType = CtXmlNodeType::RichText;
    }
    else if ("encoded_png" == xmlNodeName)
    {
        retXmlNodeType = CtXmlNodeType::EncodedPng;
    }
    else if ("table" == xmlNodeName)
    {
        retXmlNodeType = CtXmlNodeType::Table;
    }
    else if ("codebox" == xmlNodeName)
    {
        retXmlNodeType = CtXmlNodeType::CodeBox;
    }
    return retXmlNodeType;
}

void CtXmlRead::_getTextBufferIter(Glib::RefPtr<Gsv::Buffer>& rTextBuffer,
                                   std::list<CtAnchoredWidget*>& anchoredWidgets,
                                   xmlpp::Node* pNodeParent)
{
    CtXmlNodeType xmlNodeType = _xmlNodeGetTypeFromName(pNodeParent->get_name());
    if (CtXmlNodeType::RichText == xmlNodeType)
    {
        xmlpp::Element* pNodeElement = static_cast<xmlpp::Element*>(pNodeParent);
        xmlpp::TextNode* pTextNode = pNodeElement->get_child_text();
        if (pTextNode)
        {
            const Glib::ustring textContent = pTextNode->get_content();
            if (!textContent.empty())
            {
                const std::list<xmlpp::Attribute*> attributeList = pNodeElement->get_attributes();
                std::vector<Glib::ustring> tagsNames;
                for (const xmlpp::Attribute* pAttribute : attributeList)
                {
                    if (CtStrUtil::isPgcharInPgcharSet(pAttribute->get_name().c_str(), CtConst::TAG_PROPERTIES))
                    {
                        Glib::ustring tagName = CtMiscUtil::getTextTagNameExistOrCreate(pAttribute->get_name(), pAttribute->get_value());
                        tagsNames.push_back(tagName);
                    }
                }
                if (tagsNames.size() > 0)
                {
                    rTextBuffer->insert_with_tags_by_name(rTextBuffer->end(), textContent, tagsNames);
                }
                else
                {
                    rTextBuffer->insert(rTextBuffer->end(), textContent);
                }
            }
        }
    }
    else if (CtXmlNodeType::None != xmlNodeType)
    {
        xmlpp::Element* pNodeElement = static_cast<xmlpp::Element*>(pNodeParent);
        const int charOffset = std::stoi(pNodeElement->get_attribute_value("char_offset"));
        Glib::ustring justification = pNodeElement->get_attribute_value(CtConst::TAG_JUSTIFICATION);
        if (justification.empty())
        {
            justification = CtConst::TAG_PROP_VAL_LEFT;
        }
        CtAnchoredWidget* pAnchoredWidget{nullptr};
        if (CtXmlNodeType::EncodedPng == xmlNodeType)
        {
            const Glib::ustring anchorName = pNodeElement->get_attribute_value("anchor");
            if (!anchorName.empty())
            {
                pAnchoredWidget = new CtImageAnchor(anchorName, charOffset, justification);
            }
            else
            {
                const Glib::ustring fileName = pNodeElement->get_attribute_value("filename");
                xmlpp::TextNode* pTextNode = pNodeElement->get_child_text();
                const std::string encodedBlob = pTextNode ? pTextNode->get_content() : "";
                const std::string rawBlob = Glib::Base64::decode(encodedBlob);
                if (!fileName.empty())
                {
                    std::string timeStr = pNodeElement->get_attribute_value("time");
                    if (timeStr.empty())
                    {
                        timeStr = "0";
                    }
                    double timeDouble = std::stod(timeStr);
                    pAnchoredWidget = new CtImageEmbFile(fileName, rawBlob, timeDouble, charOffset, justification);
                }
                else
                {
                    const Glib::ustring link = pNodeElement->get_attribute_value("link");
                    pAnchoredWidget = new CtImagePng(rawBlob, link, charOffset, justification);
                }
            }
        }
        else if (CtXmlNodeType::Table == xmlNodeType)
        {
            const int colMin = std::stoi(pNodeElement->get_attribute_value("col_min"));
            const int colMax = std::stoi(pNodeElement->get_attribute_value("col_max"));
            CtTableMatrix tableMatrix;
            populateTableMatrix(tableMatrix, pNodeElement);
            pAnchoredWidget = new CtTable(tableMatrix, colMin, colMax, charOffset, justification);
        }
        else if (CtXmlNodeType::CodeBox == xmlNodeType)
        {
            xmlpp::TextNode* pTextNode = pNodeElement->get_child_text();
            const Glib::ustring textContent = pTextNode ? pTextNode->get_content() : "";
            const Glib::ustring syntaxHighlighting = pNodeElement->get_attribute_value("syntax_highlighting");
            const int frameWidth = std::stoi(pNodeElement->get_attribute_value("frame_width"));
            const int frameHeight = std::stoi(pNodeElement->get_attribute_value("frame_height"));
            const bool widthInPixels = CtStrUtil::isStrTrue(pNodeElement->get_attribute_value("width_in_pixels"));
            const bool highlightBrackets = CtStrUtil::isStrTrue(pNodeElement->get_attribute_value("highlight_brackets"));
            const bool showLineNumbers = CtStrUtil::isStrTrue(pNodeElement->get_attribute_value("show_line_numbers"));

            CtCodebox* pCtCodebox = new CtCodebox(textContent,
                                                  syntaxHighlighting,
                                                  frameWidth,
                                                  frameHeight,
                                                  charOffset,
                                                  justification);
            pCtCodebox->setWidthInPixels(widthInPixels);
            pCtCodebox->setHighlightBrackets(highlightBrackets);
            pCtCodebox->setShowLineNumbers(showLineNumbers);
            pAnchoredWidget = pCtCodebox;
        }
        if (nullptr != pAnchoredWidget)
        {
            pAnchoredWidget->insertInTextBuffer(rTextBuffer);
            anchoredWidgets.push_back(pAnchoredWidget);
        }
    }
}

void CtXmlRead::populateTableMatrix(CtTableMatrix& tableMatrix, xmlpp::Element* pNodeElement)
{
    if (nullptr == pNodeElement)
    {
        xmlpp::Document *pDocument = get_document();
        assert(nullptr != pDocument);
        pNodeElement = pDocument->get_root_node();
    }
    for (xmlpp::Node* pNodeRow : pNodeElement->get_children())
    {
        if ("row" == pNodeRow->get_name())
        {
            tableMatrix.push_back(CtTableRow{});
            for (xmlpp::Node* pNodeCell : pNodeRow->get_children())
            {
                if ("cell" == pNodeCell->get_name())
                {
                    xmlpp::TextNode* pTextNode = static_cast<xmlpp::Element*>(pNodeCell)->get_child_text();
                    const Glib::ustring textContent = pTextNode ? pTextNode->get_content() : "";
                    tableMatrix.back().push_back(new CtTableCell(textContent, CtConst::PLAIN_TEXT_ID));
                }
            }
        }
    }
}

Glib::RefPtr<Gsv::Buffer> CtXmlRead::getTextBuffer(const std::string& syntax,
                                                   std::list<CtAnchoredWidget*>& anchoredWidgets,
                                                   xmlpp::Element* pNodeElement)
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer = CtMiscUtil::getNewTextBuffer(syntax);
    if (nullptr == pNodeElement)
    {
        xmlpp::Document *pDocument = get_document();
        assert(nullptr != pDocument);
        pNodeElement = pDocument->get_root_node();
    }
    rRetTextBuffer->begin_not_undoable_action();
    for (xmlpp::Node* pNode : pNodeElement->get_children())
    {
        _getTextBufferIter(rRetTextBuffer, anchoredWidgets, pNode);
    }
    rRetTextBuffer->end_not_undoable_action();
    rRetTextBuffer->set_modified(false);
    return rRetTextBuffer;
}


CtXmlWrite::CtXmlWrite(const char* filepath)
{
    create_root_node(CtConst::APP_NAME);
}

CtXmlWrite::~CtXmlWrite()
{
}

void CtXmlWrite::append_bookmarks(const std::list<gint64>& bookmarks)
{
    xmlpp::Element* p_bookmarks_node = get_root_node()->add_child("bookmarks");
    Glib::ustring rejoined;
    str::join_numbers(bookmarks, rejoined, ",");
    p_bookmarks_node->set_attribute("list", rejoined);
}

void CtXmlWrite::append_dom_node(CtTreeIter& ct_tree_iter,
                                 xmlpp::Element* p_node_parent,
                                 bool to_disk,
                                 bool skip_children,
                                 const std::pair<int,int>& offset_range)
{
    if (nullptr == p_node_parent)
    {
        p_node_parent = get_root_node();
    }
    xmlpp::Element* p_node_node = p_node_parent->add_child("node");
    p_node_node->set_attribute("name", ct_tree_iter.get_node_name());
    p_node_node->set_attribute("unique_id", std::to_string(ct_tree_iter.get_node_id()));
    p_node_node->set_attribute("prog_lang", ct_tree_iter.get_node_syntax_highlighting());
    p_node_node->set_attribute("tags", ct_tree_iter.get_node_tags());
    p_node_node->set_attribute("readonly", std::to_string(ct_tree_iter.get_node_read_only()));
    p_node_node->set_attribute("custom_icon_id", std::to_string(ct_tree_iter.get_node_custom_icon_id()));
    p_node_node->set_attribute("is_bold", std::to_string(ct_tree_iter.get_node_is_bold()));
    p_node_node->set_attribute("foreground", ct_tree_iter.get_node_foreground());
    p_node_node->set_attribute("ts_creation", std::to_string(ct_tree_iter.get_node_creating_time()));
    p_node_node->set_attribute("ts_lastsave", std::to_string(ct_tree_iter.get_node_modification_time()));

    Glib::RefPtr<Gsv::Buffer> rTextBuffer = ct_tree_iter.get_node_text_buffer();
    Gtk::TextIter start_iter = offset_range.first >= 0 ? rTextBuffer->get_iter_at_offset(offset_range.first) : rTextBuffer->begin();
    Gtk::TextIter end_iter = offset_range.second >= 0 ? rTextBuffer->get_iter_at_offset(offset_range.second) : rTextBuffer->end();

    std::map<const gchar*, std::string> curr_attributes;
    if (CtConst::RICH_TEXT_ID == ct_tree_iter.get_node_syntax_highlighting())
    {
        Gtk::TextIter curr_iter{start_iter};
        CtTextIterUtil::rich_text_attributes_update(curr_iter, curr_attributes);
        while (curr_iter.forward_to_tag_toggle(Glib::RefPtr<Gtk::TextTag>{nullptr}))
        {
            if (!CtTextIterUtil::tag_richtext_toggling_on_or_off(curr_iter))
            {
                if (!curr_iter.forward_char())
                {
                    break;
                }
                continue;
            }
            _rich_txt_serialize(p_node_node, start_iter, curr_iter, curr_attributes);
            if (curr_iter.compare(end_iter) >= 0)
            {
                break;
            }
            CtTextIterUtil::rich_text_attributes_update(curr_iter, curr_attributes);
            start_iter.set_offset(curr_iter.get_offset());
        }
        if (curr_iter.compare(end_iter) < 0)
        {
            _rich_txt_serialize(p_node_node, start_iter, curr_iter, curr_attributes);
        }
        if (to_disk)
        {
            
        }
    }
    else
    {
        _rich_txt_serialize(p_node_node, start_iter, end_iter, curr_attributes);
    }

    if (!skip_children)
    {
        CtTreeIter ct_tree_iter_child = ct_tree_iter.first_child();
        while (ct_tree_iter_child)
        {
            append_dom_node(ct_tree_iter_child, p_node_node, to_disk, skip_children, offset_range);
            ct_tree_iter_child++;
        }
    }
}

void CtXmlWrite::_rich_txt_serialize(xmlpp::Element* p_node_parent,
                                     Gtk::TextIter start_iter,
                                     Gtk::TextIter end_iter,
                                     std::map<const gchar*, std::string>& curr_attributes,
                                     const gchar change_case)
{
    xmlpp::Element* p_rich_text_node = p_node_parent->add_child("rich_text");
    for (const auto& map_iter : curr_attributes)
    {
        p_rich_text_node->set_attribute(map_iter.first, map_iter.second);
    }
    Glib::ustring slot_text = start_iter.get_text(end_iter);
    if ('n' != change_case)
    {
        if ('l' == change_case) slot_text = slot_text.lowercase();
        else if ('u' == change_case) slot_text = slot_text.uppercase();
        else if (('t' == change_case) && (slot_text.size() > 0))
        {
            if (std::isupper(slot_text.at(0))) slot_text = slot_text.lowercase();
            else slot_text = slot_text.uppercase();
        }
    }
    xmlpp::TextNode* p_text_node = p_rich_text_node->add_child_text(slot_text);
}
