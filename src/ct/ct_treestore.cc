/*
 * ct_treestore.cc
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

#include "ct_main_win.h"
#include <algorithm>
#include "ct_treestore.h"
#include "ct_misc_utils.h"
#include "ct_storage_control.h"
#include "ct_actions.h"
#include "ct_logging.h"

CtTreeIter::CtTreeIter(Gtk::TreeIter iter, const CtTreeModelColumns* pColumns, CtMainWin* pCtMainWin)
 : Gtk::TreeIter(iter),
   _pColumns(pColumns),
   _pCtMainWin(pCtMainWin)
{
}

CtTreeIter CtTreeIter::parent() const
{
    return CtTreeIter((*this)->parent(), _pColumns, _pCtMainWin);
}

CtTreeIter CtTreeIter::first_child() const
{
    return CtTreeIter((*this)->children().begin(), _pColumns, _pCtMainWin);
}

bool CtTreeIter::get_node_read_only() const
{
    return (*this) and (*this)->get_value(_pColumns->colNodeRO);
}

void CtTreeIter::set_node_read_only(bool val)
{
    (*this)->set_value(_pColumns->colNodeRO, val);
}

gint64 CtTreeIter::get_node_id() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeUniqueId) : -1;
}

void CtTreeIter::set_node_id(const gint64 new_id)
{
    if (*this) {
        (*this)->set_value(_pColumns->colNodeUniqueId, new_id);
    }
}

std::vector<gint64> CtTreeIter::get_children_node_ids() const
{
    std::vector<gint64> retVec;
    CtTreeIter iterChild = first_child();
    while (iterChild) {
        retVec.push_back(iterChild.get_node_id());
        std::vector<gint64> children_node_ids = iterChild.get_children_node_ids();
        if (not children_node_ids.empty()) {
            vec::vector_extend(retVec, children_node_ids);
        }
        iterChild++;
    }
    return retVec;
}

gint64 CtTreeIter::get_node_sequence() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeSequence) : -1;
}

bool CtTreeIter::get_node_is_bold() const
{
    return (*this) and get_is_bold_from_pango_weight((*this)->get_value(_pColumns->colWeight));
}

guint16 CtTreeIter::get_node_custom_icon_id() const
{
    return (*this) ? (*this)->get_value(_pColumns->colCustomIconId) : 0;
}

Glib::ustring CtTreeIter::get_node_name() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeName) : "";
}

void CtTreeIter::set_node_name(const Glib::ustring& node_name)
{
    (*this)->set_value(_pColumns->colNodeName, node_name);
}

Glib::ustring CtTreeIter::get_node_tags() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeTags) : "";
}

std::string CtTreeIter::get_node_foreground() const
{
    return (*this) ? (*this)->get_value(_pColumns->colForeground) : "";
}

std::string CtTreeIter::get_node_syntax_highlighting() const
{
    return (*this) ? (*this)->get_value(_pColumns->colSyntaxHighlighting) : "";
}

bool CtTreeIter::get_node_is_rich_text() const
{
    return get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID;
}

gint64 CtTreeIter::get_node_creating_time() const
{
    return (*this) ? (*this)->get_value(_pColumns->colTsCreation) : 0;
}

gint64 CtTreeIter::get_node_modification_time() const
{
    return (*this) ? (*this)->get_value(_pColumns->colTsLastSave) : 0;
}

void CtTreeIter::set_node_modification_time(const gint64 modification_time)
{
    if (*this) (*this)->set_value(_pColumns->colTsLastSave, modification_time);
}

void CtTreeIter::set_node_aux_icon(Glib::RefPtr<Gdk::Pixbuf> rPixbuf)
{
    (*this)->set_value(_pColumns->rColPixbufAux, rPixbuf);
}

void CtTreeIter::set_node_sequence(gint64 num)
{
    (*this)->set_value(_pColumns->colNodeSequence, num);
}


void CtTreeIter::set_node_text_buffer(Glib::RefPtr<Gsv::Buffer> new_buffer, const std::string& new_syntax_hilighting)
{
    remove_all_embedded_widgets();
    (*this)->set_value(_pColumns->rColTextBuffer, new_buffer);
    (*this)->set_value(_pColumns->colSyntaxHighlighting, new_syntax_hilighting);
    pending_edit_db_node_buff();
    pending_edit_db_node_prop();
}

Glib::RefPtr<Gsv::Buffer> CtTreeIter::get_node_text_buffer() const
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};
    const Gtk::TreeIter& self = *this;
    if (static_cast<bool>(self)) {
        Gtk::TreeRow row = *self;
        rRetTextBuffer = row.get_value(_pColumns->rColTextBuffer);
        if (not rRetTextBuffer) {
            // SQLite text buffer not yet populated
            std::list<CtAnchoredWidget*> anchoredWidgetList{};
            const auto nodeId = get_node_id();
            const auto nodeSyntaxHighl = get_node_syntax_highlighting();
            rRetTextBuffer = _pCtMainWin->get_ct_storage()->get_delayed_text_buffer(nodeId,
                                                                                    nodeSyntaxHighl,
                                                                                    anchoredWidgetList);
            row.set_value(_pColumns->colAnchoredWidgets, anchoredWidgetList);
            row.set_value(_pColumns->rColTextBuffer, rRetTextBuffer);
        }
    }
    return rRetTextBuffer;
}

bool CtTreeIter::get_node_buffer_already_loaded() const
{
    return static_cast<bool>((*this)->get_value(_pColumns->rColTextBuffer));
}

int CtTreeIter::get_pango_weight_from_is_bold(bool isBold)
{
    return isBold ? PANGO_WEIGHT_HEAVY : PANGO_WEIGHT_NORMAL;
}

bool CtTreeIter::get_is_bold_from_pango_weight(int pangoWeight)
{
    return pangoWeight == PANGO_WEIGHT_HEAVY;
}

void CtTreeIter::remove_all_embedded_widgets()
{
    if (*this) {
        get_node_text_buffer(); // to load buffer\widgets if not loaded
        for (auto widget: (*this)->get_value(_pColumns->colAnchoredWidgets))
            delete widget;
        (*this)->set_value(_pColumns->colAnchoredWidgets, std::list<CtAnchoredWidget*>());
    }
}

std::list<CtAnchoredWidget*> CtTreeIter::get_anchored_widgets_fast(const char doSort)
{
    if (*this) {
        get_node_text_buffer(); // to load buffer\widgets if not loaded
        // removes invalid widgets (if they were deleted from buffer)
        std::list<CtAnchoredWidget*> retAnchoredWidgetsList;
        bool resave_widgets = false;
        for (CtAnchoredWidget* pCtAnchoredWidget : (*this)->get_value(_pColumns->colAnchoredWidgets))
        {
            Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = pCtAnchoredWidget->getTextChildAnchor();
            if (rChildAnchor && !rChildAnchor->get_deleted()) {
                retAnchoredWidgetsList.push_back(pCtAnchoredWidget);
            } else {
                delete pCtAnchoredWidget;
                resave_widgets = true;
            }
        }
        if (resave_widgets) {
            (*this)->set_value(_pColumns->colAnchoredWidgets, retAnchoredWidgetsList);
        }

        if (doSort != 'n') {
            if (doSort == 'd') {
                // desc
                struct {
                    bool operator()(CtAnchoredWidget* a, CtAnchoredWidget* b) const {
                        return *a > *b;
                    }
                } customCompare;
                retAnchoredWidgetsList.sort(customCompare);
            }
            else {
                // asc
                struct {
                    bool operator()(CtAnchoredWidget* a, CtAnchoredWidget* b) const {
                        return *a < *b;
                    }
                } customCompare;
                retAnchoredWidgetsList.sort(customCompare);
            }
        }

        return retAnchoredWidgetsList;
    }
    return std::list<CtAnchoredWidget*>{};
}

std::list<CtAnchoredWidget*> CtTreeIter::get_anchored_widgets(int start_offset/*= -1*/, int end_offset/*= -1*/)
{
    get_node_text_buffer(); // to load buffer\widgets if not loaded
    std::list<CtAnchoredWidget*> retAnchoredWidgetsList;
    if ((*this) and (*this)->get_value(_pColumns->colAnchoredWidgets).size() > 0) {
        Glib::RefPtr<Gsv::Buffer> rTextBuffer = get_node_text_buffer();
        Gtk::TextIter curr_iter = start_offset >= 0 ? rTextBuffer->get_iter_at_offset(start_offset) : rTextBuffer->begin();
        do {
            if (end_offset >= 0 and curr_iter.get_offset() > end_offset) {
                break;
            }
            Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = curr_iter.get_child_anchor();
            if (rChildAnchor) {
                auto pCtAnchoredWidget = get_anchored_widget(rChildAnchor);
                if (pCtAnchoredWidget) {
                    pCtAnchoredWidget->updateOffset(curr_iter.get_offset());
                    pCtAnchoredWidget->updateJustification(curr_iter);
                    retAnchoredWidgetsList.push_back(pCtAnchoredWidget);
                }
            }
        }
        while (curr_iter.forward_char());
    }
    return retAnchoredWidgetsList;
}

CtAnchoredWidget* CtTreeIter::get_anchored_widget(Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor)
{
    if (*this) {
        for (CtAnchoredWidget* pCtAnchoredWidget : (*this)->get_value(_pColumns->colAnchoredWidgets)) {
            if (rChildAnchor == pCtAnchoredWidget->getTextChildAnchor()) {
                return pCtAnchoredWidget;
            }
        }
    }
    return nullptr;
}

void CtTreeIter::pending_edit_db_node_prop()
{
    _pCtMainWin->get_ct_storage()->pending_edit_db_node_prop(get_node_id());
}

void CtTreeIter::pending_edit_db_node_buff()
{
    _pCtMainWin->get_ct_storage()->pending_edit_db_node_buff(get_node_id());
}

void CtTreeIter::pending_edit_db_node_hier()
{
    _pCtMainWin->get_ct_storage()->pending_edit_db_node_hier(get_node_id());
}

void CtTreeIter::pending_new_db_node()
{
    _pCtMainWin->get_ct_storage()->pending_new_db_node(get_node_id());
}

CtTreeStore::CtTreeStore(CtMainWin* pCtMainWin)
 : _pCtMainWin{pCtMainWin}
{
    _rTreeStore = Gtk::TreeStore::create(_columns);
}

CtTreeStore::~CtTreeStore()
{
    _iter_delete_anchored_widgets(_rTreeStore->children());
}

void CtTreeStore::pending_rm_db_nodes(const std::vector<gint64>& node_ids)
{
    _pCtMainWin->get_ct_storage()->pending_rm_db_nodes(node_ids);
}

void CtTreeStore::pending_edit_db_bookmarks()
{
    _pCtMainWin->get_ct_storage()->pending_edit_db_bookmarks();
}

void CtTreeStore::_iter_delete_anchored_widgets(const Gtk::TreeModel::Children& children)
{
    for (Gtk::TreeIter treeIter = children.begin(); treeIter != children.end(); ++treeIter) {
        Gtk::TreeRow row = *treeIter;
        for (CtAnchoredWidget* pCtAnchoredWidget : row.get_value(_columns.colAnchoredWidgets)) {
            delete pCtAnchoredWidget;
            //printf("~pCtAnchoredWidget\n");
        }
        row.get_value(_columns.colAnchoredWidgets).clear();

        _iter_delete_anchored_widgets(row.children());
    }
}

void CtTreeStore::treeview_set_tree_path_n_text_cursor(CtTreeView* pTreeView,
                                                       const std::string& node_path,
                                                       const int cursor_pos)
{
    bool treeSelFromConfig{false};
    if (not node_path.empty()) {
        Gtk::TreeIter treeIter = _rTreeStore->get_iter(node_path);
        if (static_cast<bool>(treeIter)) {
            pTreeView->set_cursor_safe(treeIter);
            treeSelFromConfig = true;
            if (_pCtMainWin->get_ct_config()->treeClickExpand) {
                pTreeView->expand_row(_rTreeStore->get_path(treeIter), false/*open_all*/);
            }
            CtTreeIter ctTreeIter = to_ct_tree_iter(treeIter);
            _pCtMainWin->text_view_apply_cursor_position(ctTreeIter, cursor_pos);
        }
    }
    if (not treeSelFromConfig) {
        const Gtk::TreeIter treeIter = get_iter_first();
        if (static_cast<bool>(treeIter)) {
            pTreeView->set_cursor(_rTreeStore->get_path(treeIter));
        }
    }
}

std::string CtTreeStore::treeview_get_tree_expanded_collapsed_string(Gtk::TreeView& treeView)
{
    std::vector<std::string> expanded_collapsed_vec;
    _rTreeStore->foreach(
        [this, &treeView, &expanded_collapsed_vec](const Gtk::TreePath& path, const Gtk::TreeIter& iter)->bool{
            expanded_collapsed_vec.push_back(std::to_string(iter->get_value(_columns.colNodeUniqueId))
                                             + ","
                                             + (treeView.row_expanded(path) ? "True" : "False"));
            return false; /* false for continue */
        }
    );
    return str::join(expanded_collapsed_vec, "_");
}

void CtTreeStore::treeview_set_tree_expanded_collapsed_string(const std::string& expanded_collapsed_string, Gtk::TreeView& treeView, bool nodes_bookm_exp)
{
    std::map<gint64, bool> expanded_collapsed_dict;
    auto expand_collapsed_vec = str::split(expanded_collapsed_string, "_");
    for (const std::string& element: expand_collapsed_vec) {
        auto couple = str::split(element, ",");
        if (couple.size() == 2) {
            expanded_collapsed_dict[std::stoll(couple[0])] = CtStrUtil::is_str_true(couple[1]);
        }
    }
    treeView.collapse_all();
    _rTreeStore->foreach(
        [this, &treeView, &expanded_collapsed_dict, &nodes_bookm_exp](const Gtk::TreePath& path, const Gtk::TreeIter& iter)->bool{
            gint64 node_id = iter->get_value(_columns.colNodeUniqueId);
            if (map::exists(expanded_collapsed_dict, node_id) and expanded_collapsed_dict.at(node_id)) {
                treeView.expand_row(path, false);
            }
            else if (nodes_bookm_exp and vec::exists(_bookmarks, node_id) and iter->parent()) {
                treeView.expand_to_path(_rTreeStore->get_path(iter->parent()));
            }
            return false; /* false for continue */
        }
    );
}

void CtTreeStore::tree_view_connect(Gtk::TreeView* pTreeView)
{
    pTreeView->set_model(_rTreeStore);

    // if change column num, then change CtTreeView::TITLE_COL_NUM
    Gtk::TreeView::Column* pColumns = Gtk::manage(new Gtk::TreeView::Column(""));
    pColumns->pack_start(_columns.rColPixbuf, /*expand=*/false);
    pColumns->pack_start(_columns.colNodeName);
    pColumns->set_expand(true);
    pTreeView->append_column(*pColumns);
    pTreeView->append_column("", _columns.rColPixbufAux);

    Gtk::TreeViewColumn* pTVCol0 = pTreeView->get_column(CtTreeView::TITLE_COL_NUM);
    std::vector<Gtk::CellRenderer*> cellRenderers0 = pTVCol0->get_cells();
    if (cellRenderers0.size() > 1) {
        Gtk::CellRendererText *pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(cellRenderers0[1]);
        if (nullptr != pCellRendererText) {
            pTVCol0->add_attribute(pCellRendererText->property_weight(), _columns.colWeight);
            pTVCol0->set_cell_data_func(
                *pCellRendererText,
                [this](Gtk::CellRenderer* pCell, const Gtk::TreeIter& treeIter){
                    Gtk::TreeRow row = *treeIter;
                    if (row.get_value(_columns.colForeground).empty())
                    {
                        dynamic_cast<Gtk::CellRendererText*>(pCell)->property_foreground() = _pCtMainWin->get_ct_config()->ttDefFg;
                    }
                    else
                    {
                        dynamic_cast<Gtk::CellRendererText*>(pCell)->property_foreground() = row.get_value(_columns.colForeground);
                    }
                }
            );
        }
    }
}

void CtTreeStore::text_view_apply_textbuffer(CtTreeIter& treeIter, CtTextView* pTextView)
{
    if (not static_cast<bool>(treeIter)) {
        pTextView->set_buffer(Glib::RefPtr<Gsv::Buffer>{});
        pTextView->set_spell_check(false);
        pTextView->set_sensitive(false);
        return;
    }

    // disconnect signals connected to prev node
    for (sigc::connection& sigc_conn : _curr_node_sigc_conn) {
        sigc_conn.disconnect();
    }
    _curr_node_sigc_conn.clear();

    spdlog::debug("Node name: {}", treeIter.get_node_name());

    Glib::RefPtr<Gsv::Buffer> rTextBuffer = treeIter.get_node_text_buffer();
    _pCtMainWin->apply_syntax_highlighting(rTextBuffer, treeIter.get_node_syntax_highlighting(), false/*forceReApply*/);
    pTextView->setup_for_syntax(treeIter.get_node_syntax_highlighting());
    pTextView->set_buffer(rTextBuffer);
    pTextView->set_spell_check(treeIter.get_node_is_rich_text());
    pTextView->set_sensitive(true);
    pTextView->set_editable(not treeIter.get_node_read_only());

    for (CtAnchoredWidget* pCtAnchoredWidget : treeIter.get_anchored_widgets_fast())
    {
        Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = pCtAnchoredWidget->getTextChildAnchor();
        if (rChildAnchor) {
            if (0 == rChildAnchor->get_widgets().size()) {
                // Gtk::TextIter textIter = rTextBuffer->get_iter_at_child_anchor(rChildAnchor);
                pTextView->add_child_at_anchor(*pCtAnchoredWidget, rChildAnchor);
                pCtAnchoredWidget->apply_width_height(pTextView->get_allocation().get_width());
                pCtAnchoredWidget->apply_syntax_highlighting(false/*forceReApply*/);
            }
            else {
                // this happens if we click on a node that is already selected, not an issue
                // we simply must not add the same widget to the anchor again
            }
        }
        else {
            spdlog::error("!! rChildAnchor");
        }
    }

    pTextView->show_all();
    // we shouldn't lose focus from TREE because TREE shortcuts/arrays movement stop working
    // pTextView->grab_focus();

    // connect signals
    _curr_node_sigc_conn.push_back(
        rTextBuffer->signal_modified_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TextBuffer>>(
            sigc::mem_fun(*this, &CtTreeStore::_on_textbuffer_modified_changed), rTextBuffer
        ))
    );
    _curr_node_sigc_conn.push_back(
        rTextBuffer->signal_insert().connect(sigc::mem_fun(*this, &CtTreeStore::_on_textbuffer_insert), false)
    );
    _curr_node_sigc_conn.push_back(
        rTextBuffer->signal_erase().connect(sigc::mem_fun(*this, &CtTreeStore::_on_textbuffer_erase), false)
    );
    _curr_node_sigc_conn.push_back(
        rTextBuffer->signal_mark_set().connect(sigc::mem_fun(*this, &CtTreeStore::_on_textbuffer_mark_set), false)
    );
}

Glib::RefPtr<Gdk::Pixbuf> CtTreeStore::_get_node_icon(int nodeDepth, const std::string &syntax, guint32 customIconId)
{
    Glib::RefPtr<Gdk::Pixbuf> rPixbuf;
    if (0 != customIconId) {
        // customIconId
        rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODE_CUSTOM_ICONS.at((int)customIconId), CtConst::NODE_ICON_SIZE);
    }
    else if (CtConst::NODE_ICON_TYPE_NONE == _pCtMainWin->get_ct_config()->nodesIcons) {
        // NODE_ICON_TYPE_NONE
        rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODE_CUSTOM_ICONS.at(CtConst::NODE_ICON_NO_ICON_ID), CtConst::NODE_ICON_SIZE);
    }
    else if (CtStrUtil::contains(std::array<const gchar*, 2>{CtConst::RICH_TEXT_ID, CtConst::PLAIN_TEXT_ID}, syntax.c_str())) {
        // text node
        if (CtConst::NODE_ICON_TYPE_CHERRY == _pCtMainWin->get_ct_config()->nodesIcons) {
            if (nodeDepth >= static_cast<int>(CtConst::NODE_CHERRY_ICONS.size())) {
                nodeDepth %= CtConst::NODE_CHERRY_ICONS.size();
            }
            rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODE_CHERRY_ICONS.at(nodeDepth), CtConst::NODE_ICON_SIZE);
        }
        else {
            // NODE_ICON_TYPE_CUSTOM
            rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODE_CUSTOM_ICONS.at(_pCtMainWin->get_ct_config()->defaultIconText), CtConst::NODE_ICON_SIZE);
        }
    }
    else {
        // code node
        rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(_pCtMainWin->get_code_icon_name(syntax), CtConst::NODE_ICON_SIZE);
    }
    return rPixbuf;
}

void CtTreeStore::get_node_data(const Gtk::TreeIter& treeIter, CtNodeData& nodeData)
{
    Gtk::TreeRow row = *treeIter;

    if (!treeIter->get_value(_columns.rColTextBuffer)) {
        to_ct_tree_iter(treeIter).get_node_text_buffer();
    }

    nodeData.name =  row[_columns.colNodeName];
    nodeData.rTextBuffer = row[_columns.rColTextBuffer];
    nodeData.nodeId = row[_columns.colNodeUniqueId];
    nodeData.syntax = row[_columns.colSyntaxHighlighting];
    nodeData.sequence = row[_columns.colNodeSequence];
    nodeData.tags = row[_columns.colNodeTags];
    nodeData.isRO = row[_columns.colNodeRO];
    //row[_columns.rColPixbufAux] = ;
    nodeData.customIconId = row[_columns.colCustomIconId];
    nodeData.isBold = CtTreeIter::get_is_bold_from_pango_weight(row[_columns.colWeight]);
    nodeData.foregroundRgb24 = row[_columns.colForeground];
    nodeData.tsCreation = row[_columns.colTsCreation];
    nodeData.tsLastSave = row[_columns.colTsLastSave];
    nodeData.anchoredWidgets = row[_columns.colAnchoredWidgets];
}

void CtTreeStore::update_node_data(const Gtk::TreeIter& treeIter, const CtNodeData& nodeData)
{
    Gtk::TreeRow row = *treeIter;
    row[_columns.rColPixbuf] = _get_node_icon(_rTreeStore->iter_depth(treeIter), nodeData.syntax, nodeData.customIconId);
    row[_columns.colNodeName] = nodeData.name;
    row[_columns.rColTextBuffer] = nodeData.rTextBuffer;
    row[_columns.colNodeUniqueId] = nodeData.nodeId;
    row[_columns.colSyntaxHighlighting] = nodeData.syntax;
    row[_columns.colNodeSequence] = nodeData.sequence;
    row[_columns.colNodeTags] = nodeData.tags;
    row[_columns.colNodeRO] = nodeData.isRO;
    //row[_columns.rColPixbufAux] = ;  // will be updated by update_node_aux_icon
    row[_columns.colCustomIconId] = (guint16)nodeData.customIconId;
    row[_columns.colWeight] = CtTreeIter::get_pango_weight_from_is_bold(nodeData.isBold);
    row[_columns.colForeground] = nodeData.foregroundRgb24;
    row[_columns.colTsCreation] = nodeData.tsCreation;
    row[_columns.colTsLastSave] = nodeData.tsLastSave;
    row[_columns.colAnchoredWidgets] = nodeData.anchoredWidgets;

    update_node_aux_icon(treeIter);
    add_used_tags(nodeData.tags);
    _nodes_names_dict[nodeData.nodeId] = nodeData.name;
}

void CtTreeStore::update_node_icon(const Gtk::TreeIter& treeIter)
{
    auto icon = _get_node_icon(_rTreeStore->iter_depth(treeIter),
                               treeIter->get_value(_columns.colSyntaxHighlighting),
                               treeIter->get_value(_columns.colCustomIconId));
    treeIter->set_value(_columns.rColPixbuf, icon);
}

void CtTreeStore::update_nodes_icon(Gtk::TreeIter father_iter, bool cherry_only)
{
    if (cherry_only)
        if (CtConst::NODE_ICON_TYPE_CHERRY != _pCtMainWin->get_ct_config()->nodesIcons)
            return;
    if (father_iter)
        update_node_icon(father_iter);
    for (auto& child: father_iter ? father_iter->children() : _rTreeStore->children())
        update_nodes_icon(child, cherry_only);
}

void CtTreeStore::update_node_aux_icon(const Gtk::TreeIter& treeIter)
{
    bool is_ro = treeIter->get_value(_columns.colNodeRO);
    bool is_bookmark = vec::exists(_bookmarks, treeIter->get_value(_columns.colNodeUniqueId));
    std::string stock_id;
    if (is_ro and is_bookmark) stock_id = "ct_lockpin";
    else if (is_ro)           stock_id = "ct_locked";
    else if (is_bookmark)     stock_id = "ct_pin";

    if (stock_id.empty())
        treeIter->set_value(_columns.rColPixbufAux, Glib::RefPtr<Gdk::Pixbuf>());
    else
        treeIter->set_value(_columns.rColPixbufAux, _pCtMainWin->get_icon_theme()->load_icon(stock_id, CtConst::NODE_ICON_SIZE));
}

Gtk::TreeIter CtTreeStore::append_node(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter)
{
    Gtk::TreeIter newIter;
    //std::cout << pNodeData->name << std::endl;

    if (nullptr == pParentIter) {
        newIter = _rTreeStore->append();
    }
    else {
        newIter = _rTreeStore->append(static_cast<Gtk::TreeRow>(**pParentIter).children());
    }
    update_node_data(newIter, *pNodeData);
    return newIter;
}

Gtk::TreeIter CtTreeStore::insert_node(CtNodeData* pNodeData, const Gtk::TreeIter& afterIter)
{
    Gtk::TreeIter newIter = _rTreeStore->insert_after(afterIter);
    update_node_data(newIter, *pNodeData);
    return newIter;
}

void CtTreeStore::_on_textbuffer_modified_changed(Glib::RefPtr<Gtk::TextBuffer> rTextBuffer)
{
    if (_pCtMainWin->user_active() and rTextBuffer->get_modified()) {
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
    }
}

void CtTreeStore::_on_textbuffer_insert(const Gtk::TextBuffer::iterator& pos, const Glib::ustring& text, int /*bytes*/)
{
    if (_pCtMainWin->user_active() and not _pCtMainWin->get_text_view().own_insert_delete_active()) {
        _pCtMainWin->get_text_view().text_inserted(pos, text);
        CtTreeIter currTreeIter = _pCtMainWin->curr_tree_iter();
        if (currTreeIter and currTreeIter.get_node_is_rich_text()) {
            _pCtMainWin->get_state_machine().text_variation(currTreeIter.get_node_id(), text);
        }
    }
}

void CtTreeStore::_on_textbuffer_erase(const Gtk::TextBuffer::iterator& range_start, const Gtk::TextBuffer::iterator& range_end)
{
    if (_pCtMainWin->user_active() and not _pCtMainWin->get_text_view().own_insert_delete_active()) {
       _pCtMainWin->get_text_view().text_removed(range_start, range_end);
        CtTreeIter currTreeIter = _pCtMainWin->curr_tree_iter();
        if (currTreeIter and currTreeIter.get_node_is_rich_text()) {
            _pCtMainWin->get_state_machine().text_variation(currTreeIter.get_node_id(), range_start.get_text(range_end));
        }
    }
}

void CtTreeStore::_on_textbuffer_mark_set(const Gtk::TextIter& /*iter*/, const Glib::RefPtr<Gtk::TextMark>& rMark)
{
    if (_pCtMainWin->user_active()) {
        if (rMark->get_name() == "insert") {
            _pCtMainWin->get_text_view().selection_update();
        }
    }
}

void CtTreeStore::addAnchoredWidgets(Gtk::TreeIter treeIter,
                                     std::list<CtAnchoredWidget*> anchoredWidgetList,
                                     Gtk::TextView* pTextView)
{
    auto widgets = treeIter->get_value(_columns.colAnchoredWidgets);
    for (auto new_widget : anchoredWidgetList) {
        widgets.push_back(new_widget);
    }
    treeIter->set_value(_columns.colAnchoredWidgets, widgets);

    for (CtAnchoredWidget* pCtAnchoredWidget : anchoredWidgetList) {
        Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = pCtAnchoredWidget->getTextChildAnchor();
        if (rChildAnchor) {
            if (0 == rChildAnchor->get_widgets().size()) {
                pTextView->add_child_at_anchor(*pCtAnchoredWidget, rChildAnchor);
                pCtAnchoredWidget->apply_width_height(pTextView->get_allocation().get_width());
                pCtAnchoredWidget->apply_syntax_highlighting(false/*forceReApply*/);
            }
        }
    }
}

gint64 CtTreeStore::node_id_get(gint64 original_id, std::unordered_map<gint64,gint64> remapping_ids)
{
    // check if remapping was set
    if ((original_id > 0) and (1 == remapping_ids.count(original_id))) {
        return remapping_ids[original_id];
    }

    // prepare sets of ids not to be used
    std::set<gint64> allocated_for_remapping_ids;
    for (const auto& curr_pair : remapping_ids) {
        allocated_for_remapping_ids.insert(curr_pair.second);
    }
    std::set<gint64> nodes_pending_rm = _pCtMainWin->get_ct_storage()->get_nodes_pending_rm();

    // (@txe) this function works differently from python code
    // it's easer to find max than check every id is not used through all tree
    gint64 max_node_id{0};
    _rTreeStore->foreach_iter([&max_node_id, this](const Gtk::TreeIter& iter){
        if (iter->get_value(_columns.colNodeUniqueId) > max_node_id)
        {
            max_node_id = iter->get_value(_columns.colNodeUniqueId);
        }
        return false; /* continue */
    });
    for (const gint64 curr_id : allocated_for_remapping_ids)
    {
        if (curr_id > max_node_id) {
            max_node_id = curr_id;
        }
    }
    for (const gint64 curr_id : nodes_pending_rm) {
        if (curr_id > max_node_id) {
            max_node_id = curr_id;
        }
    }
    const gint64 new_node_id = max_node_id+1;

    // remapping set up
    if (original_id > 0) {
        remapping_ids[original_id] = new_node_id;
    }

    return new_node_id;
}

void CtTreeStore::add_used_tags(const Glib::ustring& tags)
{
    std::vector<Glib::ustring> tagVec = str::split(tags, CtConst::CHAR_SPACE);
    for (auto& tag : tagVec) {
        tag = str::trim(tag);
        if (not tag.empty()) {
            _usedTags.insert(tag);
        }
    }
}

bool CtTreeStore::is_node_bookmarked(const gint64 node_id)
{
    return vec::exists(_bookmarks, node_id);
}

std::string CtTreeStore::get_node_name_from_node_id(const gint64 node_id)
{
    auto iter = _nodes_names_dict.find(node_id); // node_id from link can be invalid
    if (iter != _nodes_names_dict.end()) return iter->second;
    return "";
}

CtTreeIter CtTreeStore::get_node_from_node_id(const gint64 node_id)
{
    Gtk::TreeIter find_iter;
    _rTreeStore->foreach_iter([&node_id, &find_iter, this](const Gtk::TreeIter& iter) {
        if (iter->get_value(_columns.colNodeUniqueId) != node_id) return false; /* continue */
        find_iter = iter;
        return true;
    });
    return to_ct_tree_iter(find_iter);
}

CtTreeIter CtTreeStore::get_node_from_node_name(const Glib::ustring& node_name)
{
    Gtk::TreeIter find_iter;
    _rTreeStore->foreach_iter([&node_name, &find_iter, this](const Gtk::TreeIter& iter) {
        if (iter->get_value(_columns.colNodeName) != node_name) return false; /* continue */
        find_iter = iter;
        return true;
    });
    return to_ct_tree_iter(find_iter);
}

bool CtTreeStore::bookmarks_add(gint64 nodeId)
{
    if (vec::exists(_bookmarks, nodeId)) {
        return false;
    }
    _bookmarks.push_back(nodeId);
    return true;
}

bool CtTreeStore::bookmarks_remove(gint64 nodeId)
{
    if (not vec::exists(_bookmarks, nodeId)) {
        return false;
    }
    vec::remove(_bookmarks, nodeId);
    return true;
}

const std::list<gint64>& CtTreeStore::bookmarks_get()
{
    return _bookmarks;
}

void CtTreeStore::bookmarks_set(const std::list<gint64>& bookmarks)
{
    _bookmarks = bookmarks;
}

Gtk::TreeIter CtTreeStore::get_iter_first()
{
    return _rTreeStore->get_iter("0");
}

CtTreeIter CtTreeStore::get_ct_iter_first()
{
    return to_ct_tree_iter(get_iter_first());
}

Gtk::TreeIter CtTreeStore::get_tree_iter_last_sibling(const Gtk::TreeNodeChildren& children)
{
    if (children.empty()) {
        return Gtk::TreeIter{};
    }
    return --children.end();
}

Gtk::TreePath CtTreeStore::get_path(Gtk::TreeIter tree_iter)
{
    return _rTreeStore->get_path(tree_iter);
}

CtTreeIter CtTreeStore::get_iter(Gtk::TreePath& path)
{
    return to_ct_tree_iter(_rTreeStore->get_iter(path));
}

CtTreeIter CtTreeStore::to_ct_tree_iter(Gtk::TreeIter tree_iter)
{
    return CtTreeIter{tree_iter, &get_columns(), _pCtMainWin};
}

void CtTreeStore::nodes_sequences_fix(Gtk::TreeIter father_iter,  bool process_children)
{
    auto children = father_iter ? father_iter->children() : _rTreeStore->children();
    gint64 node_sequence = 0;
    for (auto& child : children) {
        ++node_sequence;
        auto ct_child = to_ct_tree_iter(child);
        if (ct_child.get_node_sequence() != node_sequence) {
            ct_child.set_node_sequence(node_sequence);
            ct_child.pending_edit_db_node_hier();
        }
        if (process_children) {
            nodes_sequences_fix(child, process_children);
        }
    }
}

void CtTreeStore::populateSummaryInfo(CtSummaryInfo& summaryInfo)
{
    _rTreeStore->foreach(
        [&](const Gtk::TreePath& /*treePath*/, const Gtk::TreeIter& treeIter)->bool{
            auto ctTreeIter = to_ct_tree_iter(treeIter);
            const auto nodeSyntax = ctTreeIter.get_node_syntax_highlighting();
            if (nodeSyntax == CtConst::RICH_TEXT_ID) {
                ++summaryInfo.nodes_rich_text_num;
            }
            else if (nodeSyntax == CtConst::PLAIN_TEXT_ID) {
                ++summaryInfo.nodes_plain_text_num;
            }
            else {
                ++summaryInfo.nodes_code_num;
            }
            (void)ctTreeIter.get_node_text_buffer(); // ensure the node content is populated
            for (CtAnchoredWidget* pAnchoredWidget : ctTreeIter.get_anchored_widgets_fast()) {
                switch (pAnchoredWidget->get_type()) {
                    case CtAnchWidgType::CodeBox: ++summaryInfo.codeboxes_num; break;
                    case CtAnchWidgType::ImageAnchor: ++summaryInfo.anchors_num; break;
                    case CtAnchWidgType::ImageEmbFile: ++summaryInfo.embfile_num; break;
                    case CtAnchWidgType::ImagePng: ++summaryInfo.images_num; break;
                    case CtAnchWidgType::Table: ++summaryInfo.tables_num; break;
                }
            }
            return false; /* false for continue */
        }
    );
}
