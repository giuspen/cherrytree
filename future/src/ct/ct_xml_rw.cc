/*
 * ct_xml_rw.cc
 * 
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
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
    xmlpp::Document *pDocument = get_document();
    assert(nullptr != pDocument);
    xmlpp::Element *pRoot = pDocument->get_root_node();
    assert("cherrytree" == pRoot->get_name());
    for (xmlpp::Node* pNode : pRoot->get_children())
    {
        if ("node" == pNode->get_name())
        {
            _xmlTreeWalkIter(static_cast<xmlpp::Element*>(pNode), pParentIter);
        }
        else if ("bookmarks" == pNode->get_name())
        {
            Glib::ustring bookmarks_csv = static_cast<xmlpp::Element*>(pNode)->get_attribute_value("list");
            for (gint64 &nodeId : CtStrUtil::gstringSplit2int64(bookmarks_csv.c_str(), ","))
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
    nodeData.tags = pNodeElement->get_attribute_value("name");
    nodeData.isRO = Glib::str_has_prefix(pNodeElement->get_attribute_value("readonly"), "T");
    nodeData.customIconId = CtStrUtil::gint64FromGstring(pNodeElement->get_attribute_value("custom_icon_id").c_str());
    nodeData.isBold = Glib::str_has_prefix(pNodeElement->get_attribute_value("is_bold"), "T");
    Glib::ustring foregroundRgb24 = pNodeElement->get_attribute_value("foreground");
    nodeData.fgOverride = !foregroundRgb24.empty();
    if (nodeData.fgOverride)
    {
        g_strlcpy(nodeData.foregroundRgb24, foregroundRgb24.c_str(), 8);
    }
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
            const Glib::ustring fileName = pNodeElement->get_attribute_value("filename");
            if (!anchorName.empty())
            {
                pAnchoredWidget = new CtImageAnchor(anchorName, charOffset, justification);
            }
            else if (!fileName.empty())
            {
                xmlpp::TextNode* pTextNode = pNodeElement->get_child_text();
                const std::string encodedFile = pTextNode ? pTextNode->get_content() : "";
                const std::string rawFileStr = Glib::Base64::decode(encodedFile);
                std::string timeStr = pNodeElement->get_attribute_value("time");
                if (timeStr.empty())
                {
                    timeStr = "0";
                }
                double timeDouble = std::stod(timeStr);
                pAnchoredWidget = new CtImageEmbFile(fileName, rawFileStr, timeDouble, charOffset, justification);
            }
            else
            {
                xmlpp::TextNode* pTextNode = pNodeElement->get_child_text();
                const std::string encodedPng = pTextNode ? pTextNode->get_content() : "";
                const std::string rawPngStr = Glib::Base64::decode(encodedPng);
                Glib::RefPtr<Gdk::PixbufLoader> rPixbufLoader = Gdk::PixbufLoader::create("image/png", true);
                rPixbufLoader->write(reinterpret_cast<const guint8*>(rawPngStr.c_str()), rawPngStr.size());
                rPixbufLoader->close();
                const Glib::RefPtr<Gdk::Pixbuf> rPixbuf = rPixbufLoader->get_pixbuf();
                const Glib::ustring link = pNodeElement->get_attribute_value("link");
                pAnchoredWidget = new CtImagePng(rPixbuf, charOffset, justification, link);
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
