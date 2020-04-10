/*
 * ct_storage_xml.h
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

#pragma once

#include "ct_types.h"
#include <glibmm/refptr.h>
#include <gtksourceviewmm/buffer.h>
#include <gtkmm/treeiter.h>

namespace xmlpp {
    class Element;
}

class CtAnchoredWidget;
class CtMainWin;
class CtTreeIter;
class CtTableCell;

class CtStorageXml : public CtStorageEntity
{
public:
    CtStorageXml(CtMainWin* pCtMainWin);
    ~CtStorageXml() = default;

    void close_connect() override;
    void reopen_connect() override;
    void test_connection() override;

    bool populate_treestore(const Glib::ustring& file_path, Glib::ustring& error) override;
    bool save_treestore(const Glib::ustring& file_path, const CtStorageSyncPending& syncPending, Glib::ustring& error) override;

    Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                      const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& widgets) const override;
private:
    Gtk::TreeIter  _node_from_xml(xmlpp::Element* xml_element, Gtk::TreeIter parent_iter);
    void           _node_to_xml(CtTreeIter* ct_tree_iter, xmlpp::Element* p_node_parent);

private:
    CtMainWin*        _pCtMainWin{nullptr};
};


class CtStorageXmlHelper
{
public:
    CtStorageXmlHelper(CtMainWin* pCtMainWin);

    Glib::RefPtr<Gsv::Buffer> create_buffer_and_widgets(xmlpp::Element* parent_xml_element, const Glib::ustring& syntax,
                                                        std::list<CtAnchoredWidget*>& widgets, Gtk::TextIter* text_iter, int force_offset);

    Glib::RefPtr<Gsv::Buffer> create_buffer_no_widgets(const Glib::ustring& syntax, const char* xml_content);

    bool populate_table_matrix(std::vector<std::vector<CtTableCell*>>& tableMatrix, const char* xml_content);

    static void save_buffer_no_widgets(xmlpp::Element* p_node_parent, Glib::RefPtr<Gtk::TextBuffer> buffer,
                                       int start_offset, int end_offset, const gchar change_case);

private:
    void              _add_content(Glib::RefPtr<Gsv::Buffer> buffer, xmlpp::Element* xml_element, Gtk::TextIter* text_insert_pos);
    CtAnchoredWidget* _create_image(xmlpp::Element* xml_element, int charOffset, const Glib::ustring& justification);
    CtAnchoredWidget* _create_codebox(xmlpp::Element* xml_element, int charOffset, const Glib::ustring& justification);
    CtAnchoredWidget* _create_table(xmlpp::Element* xml_element, int charOffset, const Glib::ustring& justification);
    void              _populate_table_matrix(xmlpp::Element* xml_element, std::vector<std::vector<CtTableCell*>>& tableMatrix);

private:
    CtMainWin* _pCtMainWin;
};
