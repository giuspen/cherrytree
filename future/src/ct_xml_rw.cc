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


CtXMLRead::CtXMLRead(const char* filepath)
{
    parse_file(filepath);
}

CtXMLRead::~CtXMLRead()
{
}

void CtXMLRead::treeWalk(const Gtk::TreeIter* pParentIter)
{
    xmlpp::Document *pDocument = get_document();
    assert (nullptr != pDocument);
    xmlpp::Element *pRoot = pDocument->get_root_node();
    assert ("cherrytree" == pRoot->get_name());
    for (xmlpp::Node *pNode : pRoot->get_children())
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

void CtXMLRead::_xmlTreeWalkIter(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter)
{
    Gtk::TreeIter newIter = _xmlNodeProcess(pNodeElement, pParentIter);

    for (xmlpp::Node *pNode : pNodeElement->get_children())
    {
        if ("node" == pNode->get_name())
        {
            _xmlTreeWalkIter(static_cast<xmlpp::Element*>(pNode), &newIter);
        }
    }
}

Gtk::TreeIter CtXMLRead::_xmlNodeProcess(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter)
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
    if(nodeData.fgOverride)
    {
        g_strlcpy(nodeData.foregroundRgb24, foregroundRgb24.c_str(), 8);
    }
    nodeData.tsCreation = CtStrUtil::gint64FromGstring(pNodeElement->get_attribute_value("ts_creation").c_str());
    nodeData.tsLastSave = CtStrUtil::gint64FromGstring(pNodeElement->get_attribute_value("ts_lastSave").c_str());
    nodeData.rTextBuffer = getTextBuffer(nodeData.syntax, nodeData.anchoredWidgets, pNodeElement);

    Gtk::TreeIter newIter = signalAppendNode.emit(&nodeData, pParentIter);
    return newIter;
}

Glib::RefPtr<Gsv::Buffer> CtXMLRead::getTextBuffer(const std::string& syntax, std::list<CtAnchoredWidget*>& anchoredWidgets, xmlpp::Element* pNodeElement)
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};
    rRetTextBuffer = Gsv::Buffer::create(CtApp::R_textTagTable);
    rRetTextBuffer->set_max_undo_levels(CtApp::P_ctCfg->limitUndoableSteps);
    rRetTextBuffer->begin_not_undoable_action();
    if (0 != syntax.compare(CtConst::RICH_TEXT_ID))
    {
        rRetTextBuffer->set_style_scheme(CtApp::R_styleSchemeManager->get_scheme(CtApp::P_ctCfg->styleSchemeId));
        if (0 == syntax.compare(CtConst::PLAIN_TEXT_ID))
        {
            rRetTextBuffer->set_highlight_syntax(false);
        }
        else
        {
            rRetTextBuffer->set_language(CtApp::R_languageManager->get_language(syntax));
            rRetTextBuffer->set_highlight_syntax(true);
        }
        rRetTextBuffer->set_highlight_matching_brackets(true);
    }
    if (nullptr != pNodeElement)
    {
        for (xmlpp::Node *pNode : pNodeElement->get_children())
        {
            if ("rich_text" == pNode->get_name())
            {
                xmlpp::Element* pNodeElement = static_cast<xmlpp::Element*>(pNode);
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
                            rRetTextBuffer->insert_with_tags_by_name(rRetTextBuffer->end(), textContent, tagsNames);
                        }
                        else
                        {
                            rRetTextBuffer->insert(rRetTextBuffer->end(), textContent);
                        }
                    }
                }
            }
            else if ("encoded_png" == pNode->get_name())
            {
                xmlpp::Element* pNodeElement = static_cast<xmlpp::Element*>(pNode);
                const int charOffset = std::stoi(pNodeElement->get_attribute_value("char_offset"));
                Glib::ustring justification = pNodeElement->get_attribute_value(CtConst::TAG_JUSTIFICATION);
                if (justification.empty())
                {
                    justification = CtConst::TAG_PROP_VAL_LEFT;
                }
                const Glib::ustring anchor = pNodeElement->get_attribute_value("anchor");
                const Glib::ustring filename = pNodeElement->get_attribute_value("filename");
                CtImage* pCtImage{nullptr};
                if (!anchor.empty())
                {
                    pCtImage = new CtImageAnchor(charOffset, justification);
                }
                else if (!filename.empty())
                {
                    pCtImage = new CtImageEmbFile(charOffset, justification);
                }
                else
                {
                    pCtImage = new CtImagePng(charOffset, justification);
                }
                pCtImage->insertInTextBuffer(rRetTextBuffer);
                anchoredWidgets.push_back(pCtImage);
            }
            else if ("table" == pNode->get_name())
            {
                
            }
            else if ("codebox" == pNode->get_name())
            {
                xmlpp::Element* pNodeElement = static_cast<xmlpp::Element*>(pNode);
                xmlpp::TextNode* pTextNode = pNodeElement->get_child_text();
                const Glib::ustring textContent = pTextNode ? pTextNode->get_content() : "";
                const Glib::ustring syntaxHighlighting = pNodeElement->get_attribute_value("syntax_highlighting");
                const int frameWidth = std::stoi(pNodeElement->get_attribute_value("frame_width"));
                const int frameHeight = std::stoi(pNodeElement->get_attribute_value("frame_height"));
                const int charOffset = std::stoi(pNodeElement->get_attribute_value("char_offset"));
                Glib::ustring justification = pNodeElement->get_attribute_value(CtConst::TAG_JUSTIFICATION);
                if (justification.empty())
                {
                    justification = CtConst::TAG_PROP_VAL_LEFT;
                }
                const bool widthInPixels = CtStrUtil::isStrTrue(pNodeElement->get_attribute_value("width_in_pixels"));
                const bool highlightBrackets = CtStrUtil::isStrTrue(pNodeElement->get_attribute_value("highlight_brackets"));
                const bool showLineNumbers = CtStrUtil::isStrTrue(pNodeElement->get_attribute_value("show_line_numbers"));

                CtCodebox* pCtCodebox = new CtCodebox(textContent, syntaxHighlighting, frameWidth, frameHeight, charOffset, justification);
                pCtCodebox->setWidthInPixels(widthInPixels);
                pCtCodebox->setHighlightBrackets(highlightBrackets);
                pCtCodebox->setShowLineNumbers(showLineNumbers);
                pCtCodebox->insertInTextBuffer(rRetTextBuffer);
                anchoredWidgets.push_back(pCtCodebox);
            }
        }
    }
    rRetTextBuffer->end_not_undoable_action();
    rRetTextBuffer->set_modified(false);
    return rRetTextBuffer;
}
