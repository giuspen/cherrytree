/*
 * ct_actions_import.cc
 *
 * Copyright 2009-2024
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
#include <optional>

static std::optional<Gtk::TreeIter> select_parent_dialog(CtMainWin* pCtMainWin)
{
    if (!pCtMainWin->curr_tree_iter()) {
        return Gtk::TreeIter{};
    }
    Gtk::Dialog dialog{_("Who is the Parent?"),
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(*pCtMainWin);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(350, -1);

    Gtk::RadioButton radiobutton_root{_("The Tree Root")};
    Gtk::RadioButton radiobutton_curr_node{_("The Selected Node")};
    radiobutton_curr_node.join_group(radiobutton_root);
    auto pContentArea = dialog.get_content_area();
    pContentArea->pack_start(radiobutton_root);
    pContentArea->pack_start(radiobutton_curr_node);
    pContentArea->show_all();

    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        if (GDK_KEY_Escape == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_REJECT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
        if (radiobutton_curr_node.get_active()) {
            return pCtMainWin->curr_tree_iter();
        }
        return Gtk::TreeIter{};
    }
    return std::nullopt;
}

// Import a node from a html file
void CtActions::import_node_from_html_file()
{
    CtHtmlImport importer(_pCtConfig);
    _import_from_file(&importer);
}

// Import a directory of html files - non recursive
void CtActions::import_node_from_html_directory()
{
    CtHtmlImport importer(_pCtConfig);
    _import_from_dir(&importer, "");
}

void CtActions::import_nodes_from_ct_folder()
{
    try {
        const std::string folder_path = CtDialogs::folder_select_dialog(_pCtMainWin, _pCtConfig->pickDirImport);
        if (folder_path.empty()) return; // No folder selected
        _pCtConfig->pickDirImport = Glib::path_get_dirname(folder_path);

        std::optional<Gtk::TreeIter> parent_iter = select_parent_dialog(_pCtMainWin);
        if (not parent_iter.has_value()) {
            return;
        }

        // Add the nodes through the storage type
        _pCtMainWin->get_ct_storage()->add_nodes_from_storage(folder_path, parent_iter.value(), true/*is_folder*/);
    }
    catch(std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_nodes_from_ct_file()
{
    try {
        CtDialogs::CtFileSelectArgs args{};
        args.curr_folder = _pCtConfig->pickDirImport;
        args.filter_name = _("CherryTree File");
        args.filter_pattern.push_back("*.ctb"); // macos doesn't understand *.ct*
        args.filter_pattern.push_back("*.ctx");
        args.filter_pattern.push_back("*.ctd");
        args.filter_pattern.push_back("*.ctz");

        const std::string file_path = CtDialogs::file_select_dialog(_pCtMainWin, args);
        if (file_path.empty()) return; // No file selected
        _pCtConfig->pickDirImport = Glib::path_get_dirname(file_path);

        std::optional<Gtk::TreeIter> parent_iter = select_parent_dialog(_pCtMainWin);
        if (not parent_iter.has_value()) {
            return;
        }

        // Add the nodes through the storage type
        _pCtMainWin->get_ct_storage()->add_nodes_from_storage(file_path, parent_iter.value(), false/*is_folder*/);
    }
    catch(std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_node_from_plaintext_file()
{
    CtPlainTextImport importer(_pCtConfig);
    _import_from_file(&importer);
}

void CtActions::import_nodes_from_plaintext_directory()
{
    CtPlainTextImport importer(_pCtConfig);
    _import_from_dir(&importer, "");
}

void CtActions::import_node_from_md_file()
{
    CtMDImport importer(_pCtConfig);
    _import_from_file(&importer);
}

void CtActions::import_nodes_from_md_directory()
{
    CtMDImport importer(_pCtConfig);
    _import_from_dir(&importer, "");
}

void CtActions::import_nodes_from_zim_directory()
{
    CtZimImport importer{_pCtConfig};
    _import_from_dir(&importer, Glib::build_filename(Glib::get_home_dir(), "Notebooks", "Notes"));
}

void CtActions::import_nodes_from_gnote_directory()
{
    CtTomboyImport importer(_pCtConfig);
    _import_from_dir(&importer, Glib::build_filename(g_get_user_data_dir(), "gnote"));
}

void CtActions::import_nodes_from_tomboy_directory()
{
    CtTomboyImport importer(_pCtConfig);
    _import_from_dir(&importer, Glib::build_filename(g_get_user_data_dir(), "tomboy"));
}

void CtActions::import_nodes_from_keepnote_directory() {
    try {
        CtKeepnoteImport importer(_pCtConfig);
        _import_from_dir(&importer, "");
    } catch(const std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_nodes_from_treepad_file()
{
    try {
        CtTreepadImporter importer;
        _import_from_file(&importer, true/*dummy_root*/);
    } catch(const std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_nodes_from_mempad_file()
{
    try {
        CtMempadImporter importer;
        _import_from_file(&importer, true/*dummy_root*/);
    } catch(const std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_nodes_from_indented_list_file()
{
    try {
        CtIndentedListImporter importer;
        _import_from_file(&importer, true/*dummy_root*/);
    } catch(const std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_nodes_from_leo_file()
{
    try {
        CtLeoImporter importer;
        _import_from_file(&importer);
    } catch(const std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_nodes_from_rednotebook_html()
{
    try {
        CtRedNotebookImporter importer{_pCtConfig};
        _import_from_file(&importer);
    } catch(const std::exception& e) {
        spdlog::error("{}: {}", __FUNCTION__, e.what());
    }
}

void CtActions::import_nodes_from_notecase_html()
{
    CtNoteCaseHTMLImporter importer{_pCtConfig};
    _import_from_file(&importer);
}

void CtActions::_import_from_file(CtImporterInterface* importer, const bool dummy_root)
{
    CtDialogs::CtFileSelectArgs args{};
    args.curr_folder = _pCtConfig->pickDirImport;
    args.filter_name = importer->file_pattern_name();
    args.filter_pattern = importer->file_patterns();
    args.filter_mime = importer->file_mime_types();
    const std::string filepath = CtDialogs::file_select_dialog(_pCtMainWin, args);
    if (filepath.empty()) return;
    spdlog::debug("{} {}", __FUNCTION__, filepath);
    _pCtConfig->pickDirImport = Glib::path_get_dirname(filepath);

    try {
        std::unique_ptr<CtImportedNode> pNode = importer->import_file(filepath);
        if (pNode) {
            _create_imported_nodes(pNode.get(), dummy_root);
        }
    }
    catch (std::exception& ex) {
        spdlog::error("import exception: {}", ex.what());
    }
}

void CtActions::_import_from_dir(CtImporterInterface* importer, const std::string& custom_dir)
{
    std::string start_dir = custom_dir.empty() or not fs::is_directory(custom_dir) ? _pCtConfig->pickDirImport : custom_dir;
    std::string import_dir = CtDialogs::folder_select_dialog(_pCtMainWin, start_dir);
    if (import_dir.empty()) return;
    if (custom_dir.empty()) {
        _pCtConfig->pickDirImport = import_dir;
    }
    try {
        auto dir_node = CtImports::traverse_dir(import_dir, importer);
        _create_imported_nodes(dir_node.get());
    }
    catch (std::exception& ex) {
        spdlog::error("import exception: {}", ex.what());
    }
}

void CtActions::_create_imported_nodes(CtImportedNode* imported_nodes, const bool dummy_root)
{
    if (not imported_nodes) {
        return;
    }

    // to apply functions to nodes
    std::function<void(CtImportedNode*, std::function<void(CtImportedNode*)>)> f_foreach_node;
    f_foreach_node = [&](CtImportedNode* imported_node, std::function<void(CtImportedNode*)> f_apply) {
        f_apply(imported_node);
        for (auto& node : imported_node->children) {
            f_foreach_node(node.get(), f_apply);
        }
    };

    // setup node id
    std::map<Glib::ustring, gint64> node_ids;
    CtTreeStore& ct_treestore = _pCtMainWin->get_tree_store();
    gint64 max_node_id = ct_treestore.node_id_get();
    f_foreach_node(imported_nodes, [&](CtImportedNode* node) {
        node->node_id = max_node_id++;
        node_ids[node->node_name] = node->node_id;
    });

    // fix broken links, node name -> node id
    f_foreach_node(imported_nodes, [&](CtImportedNode* node) {
        for (auto& broken_link : node->content_broken_links)
            if (node_ids.count(broken_link.first))
                for (xmlpp::Element* link_el : broken_link.second)
                    link_el->set_attribute(CtConst::TAG_LINK, "node " + std::to_string(node_ids[broken_link.first]));
    });

    auto f_create_node = [&](CtImportedNode* imported_node, Gtk::TreeIter curr_iter, bool is_child) {
        CtNodeData node_data{};
        node_data.name = imported_node->node_name;
        node_data.nodeId = imported_node->node_id;
        node_data.syntax = imported_node->node_syntax;
        node_data.tsCreation = std::time(nullptr);
        node_data.tsLastSave = node_data.tsCreation;
        node_data.sequence = -1;
        if (imported_node->has_content()) {
            Glib::RefPtr<Gtk::TextBuffer> pTextBuffer = _pCtMainWin->get_new_text_buffer();
            auto pGtkSourceBuffer = GTK_SOURCE_BUFFER(pTextBuffer->gobj());
            gtk_source_buffer_begin_not_undoable_action(pGtkSourceBuffer);
            for (xmlpp::Node* xml_slot : imported_node->xml_content->get_root_node()->get_children("slot")) {
                for (xmlpp::Node* child: xml_slot->get_children()) {
                    Gtk::TextIter insert_iter = pTextBuffer->get_insert()->get_iter();
                    CtStorageXmlHelper{_pCtMainWin}.get_text_buffer_one_slot_from_xml(pTextBuffer, child, node_data.anchoredWidgets, &insert_iter, -1, "");
                }
            }
            gtk_source_buffer_end_not_undoable_action(pGtkSourceBuffer);
            pTextBuffer->set_modified(false);
            node_data.rTextBuffer = pTextBuffer;
        }
        else {
            node_data.rTextBuffer = _pCtMainWin->get_new_text_buffer();
        }

        Gtk::TreeIter node_iter;
        if (is_child and curr_iter)
            node_iter = ct_treestore.append_node(&node_data, &curr_iter /* as parent */);
        else if (curr_iter)
            node_iter = ct_treestore.insert_node(&node_data, curr_iter /* after */);
        else
            node_iter = ct_treestore.append_node(&node_data);

        CtTreeIter ct_tree_iter = ct_treestore.to_ct_tree_iter(node_iter);
        ct_tree_iter.pending_new_db_node();
        ct_treestore.update_node_aux_icon(ct_tree_iter);
        return node_iter;
    };

    // just create nodes
    std::function<void(Gtk::TreeIter, CtImportedNode*)> f_create_nodes;
    f_create_nodes = [&](Gtk::TreeIter curr_iter, CtImportedNode* imported_node) {
        auto iter = f_create_node(imported_node, curr_iter, true);
        for (auto& child : imported_node->children)
            f_create_nodes(iter, child.get());
    };

    std::optional<Gtk::TreeIter> parent_iter = select_parent_dialog(_pCtMainWin);
    if (not parent_iter.has_value()) {
        return;
    }
    if (not dummy_root and imported_nodes->has_content()) {
        f_create_nodes(parent_iter.value(), imported_nodes);
    }
    else { // skip top if it's dir
        for (auto& child : imported_nodes->children) {
            f_create_nodes(parent_iter.value(), child.get());
        }
    }

    ct_treestore.nodes_sequences_fix(parent_iter.value(), true);
    _pCtMainWin->update_window_save_needed();
}
