/*
 * ct_actions_import.cc
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

#include "ct_actions.h"
#include "ct_clipboard.h"
#include "ct_imports.h"
#include "ct_storage_control.h"
#include "ct_export2html.h"
#include "ct_storage_xml.h"

#include "ct_logging.h"

// Import a node from a html file
void CtActions::import_node_from_html_file() noexcept
{
    CtHtmlImport importer(_pCtMainWin->get_ct_config());
    _import_from_file(&importer);
}

// Import a directory of html files - non recursive
void CtActions::import_node_from_html_directory() noexcept
{
    CtHtmlImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, "");
}

void CtActions::import_nodes_from_ct_file() noexcept
{
    try
    {
        CtDialogs::file_select_args args(_pCtMainWin);
        args.curr_folder = _pCtMainWin->get_ct_config()->pickDirImport;
        args.filter_name = _("CherryTree Document");
        args.filter_pattern.push_back("*.ctb"); // macos doesn't understand *.ct*
        args.filter_pattern.push_back("*.ctx");
        args.filter_pattern.push_back("*.ctd");
        args.filter_pattern.push_back("*.ctz");

        auto fpath = CtDialogs::file_select_dialog(args);
        if (fpath.empty()) return; // No file selected
        _pCtMainWin->get_ct_config()->pickDirImport = Glib::path_get_dirname(fpath);

        // Add the nodes through the storage type
        _pCtMainWin->get_ct_storage()->add_nodes_from_storage(fpath);

    } catch(std::exception& e) {
        spdlog::error("Exception caught while importing node from CT file: {}", e.what());
    }
}

void CtActions::import_node_from_plaintext_file() noexcept
{
    CtPlainTextImport importer(_pCtMainWin->get_ct_config());
    _import_from_file(&importer);
}

void CtActions::import_nodes_from_plaintext_directory() noexcept
{
    CtPlainTextImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, "");
}

void CtActions::import_node_from_md_file() noexcept
{
    CtMDImport importer(_pCtMainWin->get_ct_config());
    _import_from_file(&importer);
}

void CtActions::import_nodes_from_md_directory() noexcept
{
    CtMDImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, "");
}

void CtActions::import_nodes_from_zim_directory() noexcept
{
    CtZimImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, "");
}

void CtActions::import_node_from_pandoc() noexcept
{
    if (!CtPandoc::has_pandoc()) {
        CtDialogs::warning_dialog(_("Pandoc executable could not be found, please ensure it is in your path"), *_pCtMainWin);
        return;
    }

    CtPandocImport importer(_pCtMainWin->get_ct_config());
    _import_from_file(&importer);
}

void CtActions::import_directory_from_pandoc() noexcept
{
    if (!CtPandoc::has_pandoc()) {
        CtDialogs::warning_dialog(_("Pandoc executable could not be found, please ensure it is in your path"), *_pCtMainWin);
        return;
    }

    CtPandocImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, "");
};

void CtActions::import_nodes_from_gnote_directory() noexcept
{
    CtTomboyImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, Glib::build_filename(g_get_user_data_dir(), "gnote"));
}

void CtActions::import_nodes_from_tomboy_directory() noexcept
{
    CtTomboyImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, Glib::build_filename(g_get_user_data_dir(), "tomboy"));
}

void CtActions::import_nodes_from_keepnote_directory() noexcept {
    try {
        CtKeepnoteImport importer(_pCtMainWin->get_ct_config());
        _import_from_dir(&importer, "");
    } catch(const std::exception& e) {
        spdlog::error("Exception caught while importing from keepnote: {}", e.what());
    }
}

void CtActions::import_nodes_from_treepad_file() noexcept
{
    try {
        CtTreepadImporter importer;
        _import_from_file(&importer, true/*dummy_root*/);
    } catch(const std::exception& e) {
        spdlog::error("Exception caught while importing from Treepad: {}", e.what());
    }
}

void CtActions::import_nodes_from_mempad_file() noexcept
{
    try {
        CtMempadImporter importer;
        _import_from_file(&importer, true/*dummy_root*/);
    } catch(const std::exception& e) {
        spdlog::error("Exception caught while importing from Mempad: {}", e.what());
    }
}

void CtActions::import_nodes_from_leo_file() noexcept
{
    try {
        CtLeoImporter importer;
        _import_from_file(&importer);
    } catch(const std::exception& e) {
        spdlog::error("Exception caught while importing from Leo: {}", e.what());
    }
}

void CtActions::import_nodes_from_rednotebook_html() noexcept
{
    try {
        CtRedNotebookImporter importer{_pCtMainWin->get_ct_config()};
        _import_from_file(&importer);
    } catch(const std::exception& e) {
        spdlog::error("Exception caught while importing from Leo: {}", e.what());
    }
}

void CtActions::import_nodes_from_notecase_html() noexcept
{
    CtNoteCaseHTMLImporter importer{_pCtMainWin->get_ct_config()};
    _import_from_file(&importer);
}

void CtActions::_import_from_file(CtImporterInterface* importer, const bool dummy_root) noexcept
{
    CtDialogs::file_select_args args(_pCtMainWin);
    args.curr_folder = _pCtMainWin->get_ct_config()->pickDirImport;
    args.filter_name = importer->file_pattern_name();
    args.filter_pattern = importer->file_patterns();
    auto filepath = CtDialogs::file_select_dialog(args);
    if (filepath.empty()) return;
    _pCtMainWin->get_ct_config()->pickDirImport = Glib::path_get_dirname(filepath);

    try {
        std::unique_ptr<ct_imported_node> node = importer->import_file(filepath);
        if (!node) return;
        _create_imported_nodes(node.get(), dummy_root);
    }
    catch (std::exception& ex) {
        spdlog::error("import exception: {}", ex.what());
    }
}

void CtActions::_import_from_dir(CtImporterInterface* importer, const std::string& custom_dir) noexcept
{
    std::string start_dir = custom_dir.empty() ? _pCtMainWin->get_ct_config()->pickDirImport : custom_dir;
    std::string import_dir = CtDialogs::folder_select_dialog(start_dir, _pCtMainWin);
    if (import_dir.empty()) return;
    if (custom_dir.empty())
        _pCtMainWin->get_ct_config()->pickDirImport = import_dir;

    try
    {
        auto dir_node = CtImports::traverse_dir(import_dir, importer);
        _create_imported_nodes(dir_node.get());
    }
    catch (std::exception& ex)
    {
        spdlog::error("import exception: {}", ex.what());
    }
}

static Gtk::TreeIter select_parent_dialog(CtMainWin* pCtMainWin)
{
    if (!pCtMainWin->curr_tree_iter())
        return Gtk::TreeIter();

    Gtk::Dialog dialog(_("Who is the Parent?"), *pCtMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(*pCtMainWin);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(350, -1);

    auto radiobutton_root = Gtk::RadioButton(_("The Tree Root"));
    auto radiobutton_curr_node = Gtk::RadioButton(_("The Selected Node"));
    radiobutton_curr_node.join_group(radiobutton_root);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(radiobutton_root);
    content_area->pack_start(radiobutton_curr_node);
    content_area->show_all();
    if (dialog.run() == Gtk::RESPONSE_ACCEPT)
        if (radiobutton_curr_node.get_active())
           return pCtMainWin->curr_tree_iter();

    return Gtk::TreeIter();
}

void CtActions::_create_imported_nodes(ct_imported_node* imported_nodes, const bool dummy_root)
{
    // to apply functions to nodes
    std::function<void(ct_imported_node*, std::function<void(ct_imported_node*)>)> foreach_nodes;
    foreach_nodes = [&](ct_imported_node* imported_node, std::function<void(ct_imported_node*)> fun_apply) {
        fun_apply(imported_node);
        for (auto& node : imported_node->children)
            foreach_nodes(node.get(), fun_apply);
    };

    // setup node id
    std::map<Glib::ustring, gint64> node_ids;
    gint64 max_node_id = _pCtMainWin->get_tree_store().node_id_get();
    foreach_nodes(imported_nodes, [&](ct_imported_node* node) {
        node->node_id = max_node_id++;
        node_ids[node->node_name] = node->node_id;
    });

    // fix broken links, node name -> node id
    foreach_nodes(imported_nodes, [&](ct_imported_node* node) {
        for (auto& broken_link : node->content_broken_links)
            if (node_ids.count(broken_link.first))
                for (xmlpp::Element* link_el : broken_link.second)
                    link_el->set_attribute(CtConst::TAG_LINK, "node " + std::to_string(node_ids[broken_link.first]));
    });

    auto create_node = [&](ct_imported_node* imported_node, Gtk::TreeIter curr_iter, bool is_child) {
        CtNodeData node_data;
        node_data.name = imported_node->node_name;
        node_data.nodeId = imported_node->node_id;
        node_data.isBold = false;
        node_data.customIconId = 0;
        node_data.isRO = false;
        node_data.syntax = imported_node->node_syntax;
        node_data.tsCreation = std::time(nullptr);
        node_data.tsLastSave = node_data.tsCreation;
        node_data.sequence = -1;
        if (imported_node->has_content())
        {
            Glib::RefPtr<Gsv::Buffer> buffer = _pCtMainWin->get_new_text_buffer();
            buffer->begin_not_undoable_action();
            for (xmlpp::Node* xml_slot : imported_node->xml_content->get_root_node()->get_children("slot"))
                for (xmlpp::Node* child: xml_slot->get_children())
                {
                    Gtk::TextIter insert_iter = buffer->get_insert()->get_iter();
                    CtStorageXmlHelper(_pCtMainWin).get_text_buffer_one_slot_from_xml(buffer, child, node_data.anchoredWidgets, &insert_iter, insert_iter.get_offset());
                }
            buffer->end_not_undoable_action();
            buffer->set_modified(false);
            node_data.rTextBuffer = buffer;
        }
        else
            node_data.rTextBuffer = _pCtMainWin->get_new_text_buffer();

        Gtk::TreeIter node_iter;
        if (is_child && curr_iter)
            node_iter = _pCtMainWin->get_tree_store().append_node(&node_data, &curr_iter /* as parent */);
        else if (curr_iter)
            node_iter = _pCtMainWin->get_tree_store().insert_node(&node_data, curr_iter /* after */);
        else
            node_iter = _pCtMainWin->get_tree_store().append_node(&node_data);

        _pCtMainWin->get_tree_store().to_ct_tree_iter(node_iter).pending_new_db_node();
        _pCtMainWin->get_tree_store().update_node_aux_icon(node_iter);
        return node_iter;
    };

    // just create nodes
    std::function<void(Gtk::TreeIter, ct_imported_node*)> create_nodes;
    create_nodes = [&](Gtk::TreeIter curr_iter, ct_imported_node* imported_node) {
        auto iter = create_node(imported_node, curr_iter, true);
        for (auto& child : imported_node->children)
            create_nodes(iter, child.get());
    };

    Gtk::TreeIter parent_iter = select_parent_dialog(_pCtMainWin);
    if (not dummy_root and imported_nodes->has_content()) {
        create_nodes(parent_iter, imported_nodes);
    }
    else // skip top if it's dir
    {
        for (auto& child : imported_nodes->children)
            create_nodes(parent_iter, child.get());
    }

    _pCtMainWin->get_tree_store().nodes_sequences_fix(parent_iter, true);
    _pCtMainWin->update_window_save_needed();
}
