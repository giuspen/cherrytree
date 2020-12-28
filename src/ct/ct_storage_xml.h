/*
 * ct_storage_xml.h
 *
 * Copyright 2009-2020
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

#include "ct_types.h"
#include "ct_filesystem.h"
#include <glibmm/refptr.h>
#include <gtksourceviewmm/buffer.h>
#include <gtkmm/treeiter.h>
#include <libxml++/libxml++.h>

namespace xmlpp {
    class Element;
    class Node;
}

class CtAnchoredWidget;
class CtMainWin;
class CtTreeIter;
class CtStorageCache;

class CtStorageXml : public CtStorageEntity
{
public:
    CtStorageXml(CtMainWin* pCtMainWin);
    ~CtStorageXml() = default;

    void close_connect() override;
    void reopen_connect() override;
    void test_connection() override;

    bool populate_treestore(const fs::path& file_path, Glib::ustring& error) override;
    bool save_treestore(const fs::path& file_path,
                        const CtStorageSyncPending& syncPending,
                        Glib::ustring& error,
                        const CtExporting exporting = CtExporting::NONE,
                        const int start_offset = 0,
                        const int end_offset = -1) override;
    void vacuum() override;
    void import_nodes(const fs::path& path) override;

    Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                      const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& widgets) const override;
private:
    Gtk::TreeIter _node_from_xml(xmlpp::Element* xml_element, gint64 sequence, Gtk::TreeIter parent_iter, gint64 new_id, bool* has_duplicated_id);
    void _nodes_to_xml(CtTreeIter* ct_tree_iter,
                       xmlpp::Element* p_node_parent,
                       CtStorageCache* storage_cache,
                       const CtExporting exporting = CtExporting::NONE,
                       const int start_offset = 0,
                       const int end_offset =-1);
    std::unique_ptr<xmlpp::DomParser> _get_parser(const fs::path& file_path);

private:
    CtMainWin* _pCtMainWin{nullptr};
    mutable std::map<gint64, std::shared_ptr<xmlpp::Document>> _delayed_text_buffers;
};


class CtStorageXmlHelper
{
public:
    CtStorageXmlHelper(CtMainWin* pCtMainWin);

    xmlpp::Element* node_to_xml(CtTreeIter* ct_tree_iter,
                                xmlpp::Element* p_node_parent,
                                bool with_widgets,
                                CtStorageCache* storage_cache,
                                const int start_offset = 0,
                                const int end_offset = -1);

    Glib::RefPtr<Gsv::Buffer> create_buffer_and_widgets_from_xml(
        xmlpp::Element* parent_xml_element,
        const Glib::ustring& syntax,
        std::list<CtAnchoredWidget*>& widgets,
        Gtk::TextIter* text_insert_pos,
        int force_offset);

    void get_text_buffer_one_slot_from_xml(Glib::RefPtr<Gsv::Buffer> buffer,
                                           xmlpp::Node* slot_node,
                                           std::list<CtAnchoredWidget*>& widgets,
                                           Gtk::TextIter* text_insert_pos,
                                           int force_offset);

    Glib::RefPtr<Gsv::Buffer> create_buffer_no_widgets(const Glib::ustring& syntax, const char* xml_content);

    bool populate_table_matrix(CtTableMatrix& tableMatrix, const char* xml_content, CtTableColWidths& tableColWidths);
    void populate_table_matrix(CtTableMatrix& tableMatrix, xmlpp::Element* xml_element, CtTableColWidths& tableColWidths);

    static void save_buffer_no_widgets_to_xml(xmlpp::Element* p_node_parent, Glib::RefPtr<Gtk::TextBuffer> buffer,
                                              int start_offset, int end_offset, const gchar change_case);

private:
    void              _add_rich_text_from_xml(Glib::RefPtr<Gsv::Buffer> buffer, xmlpp::Element* xml_element, Gtk::TextIter* text_insert_pos);
    CtAnchoredWidget* _create_image_from_xml(xmlpp::Element* xml_element, int charOffset, const Glib::ustring& justification);
    CtAnchoredWidget* _create_codebox_from_xml(xmlpp::Element* xml_element, int charOffset, const Glib::ustring& justification);
    CtAnchoredWidget* _create_table_from_xml(xmlpp::Element* xml_element, int charOffset, const Glib::ustring& justification);

private:
    CtMainWin* _pCtMainWin;
};

namespace CtXmlHelper
{
    void table_to_xml(xmlpp::Element* parent, const std::vector<std::vector<Glib::ustring>>& rows, int char_offset, Glib::ustring justification, int defaultWidth, Glib::ustring colWidths);
};
