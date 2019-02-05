#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include "ct_image.h"
#include "ct_app.h"
#include "ct_dialogs.h"
#include "ct_doc_rw.h"
#include <time.h>

bool dialog_node_prop(std::string title, Gtk::Window& parent, CtNodeData& nodeData, const std::set<std::string>& tags_set);

bool CtActions::_is_there_selected_node_or_error()
{
    if (_ctMainWin->curr_tree_iter()) return true;
    ct_dialogs::warning_dialog(_("No Node is Selected"), *_ctMainWin);
    return false;
}

void CtActions::_node_add(bool duplicate, bool add_child)
{
    CtNodeData nodeData;
    if (duplicate)
     {
        if (!_is_there_selected_node_or_error()) return;
        _ctTreestore->getNodeData(_ctMainWin->curr_tree_iter(), nodeData);

        if (nodeData.syntax != CtConst::RICH_TEXT_ID) {
            nodeData.rTextBuffer = CtMiscUtil::getNewTextBuffer(nodeData.syntax, nodeData.rTextBuffer->get_text());
            nodeData.anchoredWidgets.clear();
        } else {
            //state = self.state_machine.requested_state_previous(self.treestore[tree_iter_from][3])
            //self.load_buffer_from_state(state, given_tree_iter=new_node_iter)

            // todo: temporary solution
            nodeData.anchoredWidgets.clear();
            nodeData.rTextBuffer = CtMiscUtil::getNewTextBuffer(nodeData.syntax, nodeData.rTextBuffer->get_text());
        }
    }
    else
    {
        if (add_child && !_is_there_selected_node_or_error()) return;
        std::string title = add_child ? _("New Child Node Properties") : _("New Node Properties");
        nodeData.isBold = false;
        nodeData.customIconId = 0;
        nodeData.syntax = CtConst::RICH_TEXT_ID;
        nodeData.isRO = false;
        if (!dialog_node_prop(title, *_ctMainWin, nodeData, _ctTreestore->get_used_tags()))
            return;
    }
    _node_add_with_data(_ctMainWin->curr_tree_iter(), nodeData, add_child);
}

void CtActions::_node_add_with_data(Gtk::TreeIter curr_iter, CtNodeData& nodeData, bool add_child)
{
    if (!nodeData.rTextBuffer)
        nodeData.rTextBuffer = CtMiscUtil::getNewTextBuffer(nodeData.syntax);
    nodeData.tsCreation = std::time(nullptr);
    nodeData.tsLastSave = nodeData.tsCreation;
    nodeData.nodeId = _ctTreestore->node_id_get();

    _ctMainWin->update_window_save_needed();
    CtApp::P_ctCfg->syntaxHighlighting = nodeData.syntax;

    Gtk::TreeIter nodeIter;
    if (add_child) {
        nodeIter = _ctTreestore->appendNode(&nodeData, &curr_iter /* as parent */);
    } else if (curr_iter)
        nodeIter = _ctTreestore->insertNode(&nodeData, curr_iter /* after */);
    else
        nodeIter = _ctTreestore->appendNode(&nodeData);

    _ctTreestore->ctdb_handler()->pending_new_db_node(nodeData.nodeId);
    _ctTreestore->nodes_sequences_fix(curr_iter ? curr_iter->parent() : Gtk::TreeIter(), false);
    _ctTreestore->updateNodeAuxIcon(nodeIter);
    _ctMainWin->get_tree_view().set_cursor_safe(nodeIter);
    _ctMainWin->get_text_view().grab_focus();
}

void CtActions::_node_child_exist_or_create(Gtk::TreeIter parentIter, const std::string& nodeName)
{
    Gtk::TreeIter childIter = parentIter ? parentIter->children().begin() : _ctTreestore->get_iter_first();
    for (; childIter; ++childIter)
        if (_ctTreestore->to_ct_tree_iter(childIter).get_node_name() == nodeName) {
            _ctMainWin->get_tree_view().set_cursor_safe(childIter);
            return;
        }
    CtNodeData nodeData;
    nodeData.name = nodeName;
    nodeData.isBold = false;
    nodeData.customIconId = 0;
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    nodeData.isRO = false;
    _node_add_with_data(parentIter, nodeData, true);
}

// Move a node to a parent and after a sibling
void CtActions::_node_move_after(Gtk::TreeIter iter_to_move, Gtk::TreeIter father_iter,
                                 Gtk::TreeIter brother_iter /*= Gtk::TreeIter()*/, bool set_first /*= false*/)
{
    Gtk::TreeIter new_node_iter;
    if (brother_iter)   new_node_iter = _ctTreestore->get_store()->insert_after(brother_iter);
    else if (set_first) new_node_iter = _ctTreestore->get_store()->prepend(father_iter->children());
    else                new_node_iter = _ctTreestore->get_store()->append(father_iter->children());

    // we move also all the children
    std::function<void(Gtk::TreeIter&,Gtk::TreeIter&)> node_move_data_and_children;
    node_move_data_and_children = [this, &node_move_data_and_children](Gtk::TreeIter& old_iter,Gtk::TreeIter& new_iter) {
        CtNodeData node_data;
        _ctTreestore->getNodeData(old_iter, node_data);
        _ctTreestore->updateNodeData(new_iter, node_data);
        for (Gtk::TreeIter child: old_iter->children()) {
            Gtk::TreeIter new_child = _ctTreestore->get_store()->append(new_iter->children());
            node_move_data_and_children(child, new_child);
        }
    };
    node_move_data_and_children(iter_to_move, new_node_iter);

    // now we can remove the old iter (and all children)
    _ctTreestore->get_store()->erase(iter_to_move);
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[new_node_iter][3])
    _ctTreestore->nodes_sequences_fix(Gtk::TreeIter(), true);
    if (father_iter)
        _ctMainWin->get_tree_view().expand_row(_ctTreestore->get_path(father_iter), false);
    else
        _ctMainWin->get_tree_view().expand_row(_ctTreestore->get_path(new_node_iter), false);
    Gtk::TreePath new_node_path = _ctTreestore->get_path(new_node_iter);
    _ctMainWin->get_tree_view().collapse_row(new_node_path);
    _ctMainWin->get_tree_view().set_cursor(new_node_path);
    _ctMainWin->update_window_save_needed();
}

bool CtActions::_need_node_swap(Gtk::TreeIter& leftIter, Gtk::TreeIter& rightIter, bool ascending)
{
    int cmp = _ctTreestore->to_ct_tree_iter(leftIter).get_node_name().compare(_ctTreestore->to_ct_tree_iter(rightIter).get_node_name());
    return ascending ? cmp > 0 : cmp < 0;
}

bool CtActions::_tree_sort_level_and_sublevels(const Gtk::TreeNodeChildren& children, bool ascending)
{
    auto need_swap = [this,&ascending](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, ascending); };
    bool swap_excecuted = CtMiscUtil::node_siblings_sort_iteration(_ctTreestore->get_store(), children, need_swap);
    for (auto& child: children)
        if (_tree_sort_level_and_sublevels(child.children(), ascending))
            swap_excecuted = true;
    return swap_excecuted;
}

void CtActions::node_edit()
{
    if (!_is_there_selected_node_or_error()) return;
    CtNodeData nodeData;
    _ctTreestore->getNodeData(_ctMainWin->curr_tree_iter(), nodeData);
    CtNodeData newData = nodeData;
    if (!dialog_node_prop(_("Node Properties"), *_ctMainWin, newData, _ctTreestore->get_used_tags()))
        return;

    CtApp::P_ctCfg->syntaxHighlighting = newData.syntax;
    if (nodeData.syntax !=  newData.syntax) {
        if (nodeData.syntax == CtConst::RICH_TEXT_ID) {
            // leaving rich text
            if (!ct_dialogs::question_dialog(_("Leaving the Node Type Rich Text you will Lose all Formatting for This Node, Do you want to Continue?"), *_ctMainWin)) {
                return;
            }
            // todo:
            // SWITCH TextBuffer -> SourceBuffer
            //self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
            //self.curr_buffer = self.treestore[self.curr_tree_iter][2]
            //self.state_machine.delete_states(self.get_node_id_from_tree_iter(self.curr_tree_iter))
        } else if (newData.syntax == CtConst::RICH_TEXT_ID) {
            // going to rich text
            // SWITCH SourceBuffer -> TextBuffer
            //self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
            //self.curr_buffer = self.treestore[self.curr_tree_iter][2]
        } else if (nodeData.syntax == CtConst::PLAIN_TEXT_ID) {
            // plain text to code
            //self.sourceview.modify_font(pango.FontDescription(self.code_font))
        } else if (newData.syntax == CtConst::PLAIN_TEXT_ID) {
            // code to plain text
            // self.sourceview.modify_font(pango.FontDescription(self.pt_font))
        }
    }
    _ctTreestore->updateNodeData(_ctMainWin->curr_tree_iter(), newData);
    //if self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
    //  self.set_sourcebuffer_syntax_highlight(self.curr_buffer, self.syntax_highlighting)
    _ctMainWin->get_text_view().set_editable(!newData.isRO);
    //self.update_selected_node_statusbar_info()
    _ctTreestore->updateNodeAuxIcon(_ctMainWin->curr_tree_iter());
    _ctMainWin->treeview_set_colors();
    _ctMainWin->window_header_update();
    _ctMainWin->window_header_update_lock_icon(newData.isRO);
    _ctMainWin->update_window_save_needed("npro");
    _ctMainWin->get_text_view().grab_focus();
}

void CtActions::node_toggle_read_only()
{
    if (!_is_there_selected_node_or_error()) return;
    bool node_is_ro = !_ctMainWin->curr_tree_iter().get_node_read_only();
    _ctMainWin->curr_tree_iter().set_node_read_only(node_is_ro);
    _ctMainWin->get_text_view().set_editable(!node_is_ro);
    _ctMainWin->window_header_update_lock_icon(node_is_ro);
    //self.update_selected_node_statusbar_info()
    _ctTreestore->updateNodeAuxIcon(_ctMainWin->curr_tree_iter());
    _ctMainWin->update_window_save_needed("npro");
    _ctMainWin->get_text_view().grab_focus();
}

void CtActions::node_date()
{
    std::string months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    std::string days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    time_t time = std::time(nullptr);
    tm* tmTime = std::localtime(&time);

    std::string year = std::to_string(tmTime->tm_year + 1900);
    std::string month = months[tmTime->tm_mon];
    std::string day = days[tmTime->tm_wday] + " " + std::to_string(tmTime->tm_mday);

    _node_child_exist_or_create(Gtk::TreeIter(), year);
    _node_child_exist_or_create(_ctMainWin->curr_tree_iter(), month);
    _node_child_exist_or_create(_ctMainWin->curr_tree_iter(), day);
}

void CtActions::node_up()
{
    if (!_is_there_selected_node_or_error()) return;
    auto prev_iter = --_ctMainWin->curr_tree_iter();
    if (!prev_iter) return;
    _ctTreestore->get_store()->iter_swap(_ctMainWin->curr_tree_iter(), prev_iter);
    //self.nodes_sequences_swap(self.curr_tree_iter, prev_iter)
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[self.curr_tree_iter][3])
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[prev_iter][3])
    _ctMainWin->get_tree_view().set_cursor(_ctTreestore->get_path(_ctMainWin->curr_tree_iter()));
    _ctMainWin->update_window_save_needed();
}

void CtActions::node_down()
{
    if (!_is_there_selected_node_or_error()) return;
    auto next_iter = ++_ctMainWin->curr_tree_iter();
    if (!next_iter) return;
    _ctTreestore->get_store()->iter_swap(_ctMainWin->curr_tree_iter(), next_iter);
    //self.nodes_sequences_swap(self.curr_tree_iter, subseq_iter)
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[self.curr_tree_iter][3])
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[subseq_iter][3])
    _ctMainWin->get_tree_view().set_cursor(_ctTreestore->get_path(_ctMainWin->curr_tree_iter()));
    _ctMainWin->update_window_save_needed();
}

void CtActions::node_right()
{
    if (!_is_there_selected_node_or_error()) return;
    auto prev_iter = --_ctMainWin->curr_tree_iter();
    if (!prev_iter) return;
    _node_move_after(_ctMainWin->curr_tree_iter(), prev_iter);
    //if (CtApp::P_ctCfg->nodesIcons == "c") self.treeview_refresh(change_icon=True)
}

void CtActions::node_left()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _ctMainWin->curr_tree_iter()->parent();
    if (!father_iter) return;
    _node_move_after(_ctMainWin->curr_tree_iter(), father_iter->parent(), father_iter);
    //if (CtApp::P_ctCfg->nodesIcons == "c") self.treeview_refresh(change_icon=True)
}

void CtActions::node_change_father()
{
    if (!_is_there_selected_node_or_error()) return;
    CtTreeIter old_father_iter = _ctMainWin->curr_tree_iter().parent();
    CtTreeIter father_iter = _ctTreestore->to_ct_tree_iter(ct_dialogs::choose_node_dialog(*_ctMainWin,
                                   _ctMainWin->get_tree_view(), _("Select the New Parent"), _ctTreestore, _ctMainWin->curr_tree_iter()));
    if (!father_iter) return;
    gint64 curr_node_id = _ctMainWin->curr_tree_iter().get_node_id();
    gint64 old_father_node_id = old_father_iter.get_node_id();
    gint64 new_father_node_id = father_iter.get_node_id();
    if (curr_node_id == new_father_node_id) {
        ct_dialogs::error_dialog(_("The new parent can't be the very node to move!"), *_ctMainWin);
        return;
    }
    if (old_father_node_id != -1 && new_father_node_id == old_father_node_id) {
        ct_dialogs::info_dialog(_("The new chosen parent is still the old parent!"), *_ctMainWin);
        return;
    }
    for (CtTreeIter move_towards_top_iter = father_iter.parent(); move_towards_top_iter; move_towards_top_iter = move_towards_top_iter.parent())
        if (move_towards_top_iter.get_node_id() == curr_node_id) {
            ct_dialogs::error_dialog(_("The new parent can't be one of his children!"), *_ctMainWin);
            return;
        }

    _node_move_after(_ctMainWin->curr_tree_iter(), father_iter);
    //if self.nodes_icons == "c": self.treeview_refresh(change_icon=True)
}

//"""Sorts the Tree Ascending"""
void CtActions::tree_sort_ascending()
{
    if (_tree_sort_level_and_sublevels(_ctTreestore->get_store()->children(), true)) {
        _ctTreestore->nodes_sequences_fix(Gtk::TreeIter(), true);
        _ctMainWin->update_window_save_needed();
    }
}

//"""Sorts the Tree Ascending"""
void CtActions::tree_sort_descending()
{
    if (_tree_sort_level_and_sublevels(_ctTreestore->get_store()->children(), false)) {
        _ctTreestore->nodes_sequences_fix(Gtk::TreeIter(), true);
        _ctMainWin->update_window_save_needed();
    }
}

//"""Sorts all the Siblings of the Selected Node Ascending"""
void CtActions::node_siblings_sort_ascending()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _ctMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _ctTreestore->get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, true); };
    if (CtMiscUtil::node_siblings_sort_iteration(_ctTreestore->get_store(), children, need_swap)) {
        _ctTreestore->nodes_sequences_fix(father_iter, true);
        _ctMainWin->update_window_save_needed();
    }
}

//"""Sorts all the Siblings of the Selected Node Descending"""
void CtActions::node_siblings_sort_descending()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _ctMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _ctTreestore->get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, false); };
    if (CtMiscUtil::node_siblings_sort_iteration(_ctTreestore->get_store(), children, need_swap)) {
        _ctTreestore->nodes_sequences_fix(father_iter, true);
        _ctMainWin->update_window_save_needed();
    }
}

bool dialog_node_prop(std::string title, Gtk::Window& parent, CtNodeData& nodeData, const std::set<std::string>& tags_set)
{
    auto dialog = Gtk::Dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(300, -1);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    auto name_entry = Gtk::Entry();
    name_entry.set_text(nodeData.name);
    auto is_bold_checkbutton = Gtk::CheckButton(_("Bold"));
    is_bold_checkbutton.set_active(nodeData.isBold);
    auto fg_checkbutton = Gtk::CheckButton(_("Use Selected Color"));
    fg_checkbutton.set_active(nodeData.foregroundRgb24 != "");
    Glib::ustring real_fg = nodeData.foregroundRgb24 != "" ? nodeData.foregroundRgb24 : (CtApp::P_ctCfg->currColors.at('n') != "" ? CtApp::P_ctCfg->currColors.at('n').c_str() : "red");
    auto fg_colorbutton = Gtk::ColorButton(Gdk::RGBA(real_fg));
    fg_colorbutton.set_sensitive(nodeData.foregroundRgb24 != "");
    auto fg_hbox = Gtk::HBox();
    fg_hbox.set_spacing(2);
    fg_hbox.pack_start(fg_checkbutton, false, false);
    fg_hbox.pack_start(fg_colorbutton, false, false);
    auto c_icon_checkbutton = Gtk::CheckButton(_("Use Selected Icon"));
    c_icon_checkbutton.set_active(map::exists(CtConst::NODES_STOCKS, nodeData.customIconId));
    auto c_icon_button = Gtk::Button();
    if (c_icon_checkbutton.get_active())
        c_icon_button.set_image(*CtImage::new_image_from_stock(CtConst::NODES_STOCKS.at(nodeData.customIconId), Gtk::ICON_SIZE_BUTTON));
    else {
        c_icon_button.set_label(_("click me"));
        c_icon_button.set_sensitive(false);
    }
    auto c_icon_hbox = Gtk::HBox();
    c_icon_hbox.set_spacing(2);
    c_icon_hbox.pack_start(c_icon_checkbutton, false, false);
    c_icon_hbox.pack_start(c_icon_button, false, false);
    auto name_vbox = Gtk::VBox();
    name_vbox.pack_start(name_entry);
    name_vbox.pack_start(is_bold_checkbutton);
    name_vbox.pack_start(fg_hbox);
    name_vbox.pack_start(c_icon_hbox);
    auto name_frame = Gtk::Frame(std::string("<b>")+_("Node Name")+"</b>");
    ((Gtk::Label*)name_frame.get_label_widget())->set_use_markup(true);
    name_frame.set_shadow_type(Gtk::SHADOW_NONE);
    name_frame.add(name_vbox);
    auto radiobutton_rich_text = Gtk::RadioButton(_("Rich Text"));
    auto radiobutton_plain_text = Gtk::RadioButton(_("Plain Text"));
    radiobutton_plain_text.join_group(radiobutton_rich_text);
    auto radiobutton_auto_syntax_highl = Gtk::RadioButton(_("Automatic Syntax Highlighting"));
    radiobutton_auto_syntax_highl.join_group(radiobutton_rich_text);
    auto button_prog_lang = Gtk::Button();
    std::string syntax_hl_id = nodeData.syntax;
    if (nodeData.syntax == CtConst::RICH_TEXT_ID || nodeData.syntax == CtConst::PLAIN_TEXT_ID)
        syntax_hl_id = CtApp::P_ctCfg->autoSynHighl;
    std::string button_stock_id = CtConst::getStockIdForCodeType(syntax_hl_id);
    button_prog_lang.set_label(syntax_hl_id);
    button_prog_lang.set_image(*CtImage::new_image_from_stock(button_stock_id, Gtk::ICON_SIZE_MENU));
    if (nodeData.syntax == CtConst::RICH_TEXT_ID) {
        radiobutton_rich_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    } else if (nodeData.syntax == CtConst::PLAIN_TEXT_ID) {
        radiobutton_plain_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    } else {
        radiobutton_auto_syntax_highl.set_active(true);
    }
    auto type_vbox = Gtk::VBox();
    type_vbox.pack_start(radiobutton_rich_text);
    type_vbox.pack_start(radiobutton_plain_text);
    type_vbox.pack_start(radiobutton_auto_syntax_highl);
    type_vbox.pack_start(button_prog_lang);
    auto type_frame = Gtk::Frame(std::string("<b>")+_("Node Type")+"</b>");
    ((Gtk::Label*)type_frame.get_label_widget())->set_use_markup(true);
    type_frame.set_shadow_type(Gtk::SHADOW_NONE);
    type_frame.add(type_vbox);
    type_frame.set_sensitive(nodeData.isRO == false);
    auto tags_hbox = Gtk::HBox();
    tags_hbox.set_spacing(2);
    auto tags_entry = Gtk::Entry();
    tags_entry.set_text(nodeData.tags);
    auto button_browse_tags = Gtk::Button();
    button_browse_tags.set_image(*CtImage::new_image_from_stock("find", Gtk::ICON_SIZE_BUTTON));
    button_browse_tags.set_sensitive(!tags_set.empty());
    tags_hbox.pack_start(tags_entry);
    tags_hbox.pack_start(button_browse_tags, false, false);
    auto tags_frame = Gtk::Frame(std::string("<b>")+_("Tags for Searching")+"</b>");
    ((Gtk::Label*)tags_frame.get_label_widget())->set_use_markup(true);
    tags_frame.set_shadow_type(Gtk::SHADOW_NONE);
    tags_frame.add(tags_hbox);
    auto ro_checkbutton = Gtk::CheckButton(_("Read Only"));
    ro_checkbutton.set_active(nodeData.isRO);
    auto content_area = dialog.get_content_area();
    content_area->set_spacing(5);
    content_area->pack_start(name_frame);
    content_area->pack_start(type_frame);
    content_area->pack_start(tags_frame);
    content_area->pack_start(ro_checkbutton);
    content_area->show_all();
    name_entry.grab_focus();

    button_prog_lang.signal_clicked().connect([&parent, &button_prog_lang](){
        auto itemStore = ct_dialogs::CtChooseDialogListStore::create();
        for (auto lang: CtApp::R_languageManager->get_language_ids())
            itemStore->add_row(CtConst::getStockIdForCodeType(lang), "", lang);
        auto res = ct_dialogs::choose_item_dialog(parent, _("Automatic Syntax Highlighting"), itemStore);
        if (res) {
            std::string stock_id = res->get_value(itemStore->columns.desc);
            button_prog_lang.set_label(stock_id);
            button_prog_lang.set_image(*CtImage::new_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
        }
    });
    radiobutton_auto_syntax_highl.signal_toggled().connect([&radiobutton_auto_syntax_highl, &button_prog_lang](){
       button_prog_lang.set_sensitive(radiobutton_auto_syntax_highl.get_active());
    });
    button_browse_tags.signal_clicked().connect([&parent, &tags_entry, &tags_set](){
        auto itemStore = ct_dialogs::CtChooseDialogListStore::create();
        for (const auto& tag: tags_set)
            itemStore->add_row("", "", tag);
        auto res = ct_dialogs::choose_item_dialog(parent, _("Choose Existing Tag"), itemStore, _("Tag Name"));
        if (res) {
            std::string cur_tag = tags_entry.get_text();
            if  (str::endswith(cur_tag, CtConst::CHAR_SPACE))
                tags_entry.set_text(cur_tag + res->get_value(itemStore->columns.desc));
            else
                tags_entry.set_text(cur_tag + CtConst::CHAR_SPACE + res->get_value(itemStore->columns.desc));
        }
    });
    ro_checkbutton.signal_toggled().connect([&ro_checkbutton, &type_frame](){
        type_frame.set_sensitive(ro_checkbutton.get_active());
    });
    fg_checkbutton.signal_toggled().connect([&fg_checkbutton, &fg_colorbutton](){
        fg_colorbutton.set_sensitive(fg_checkbutton.get_active());
    });
    fg_colorbutton.signal_pressed().connect([&parent, &fg_colorbutton](){
        Gdk::RGBA ret_color = fg_colorbutton.get_rgba();
        if (ct_dialogs::color_pick_dialog(parent, ret_color))
            fg_colorbutton.set_rgba(ret_color);
    });
    c_icon_checkbutton.signal_toggled().connect([&c_icon_checkbutton, &c_icon_button](){
        c_icon_button.set_sensitive(c_icon_checkbutton.get_active());
    });
    c_icon_button.signal_clicked().connect([&parent, &c_icon_button, &nodeData](){
        auto itemStore = ct_dialogs::CtChooseDialogListStore::create();
        for (auto& pair: CtConst::NODES_ICONS)
            itemStore->add_row(pair.second, std::to_string(pair.first), "");
        auto res = ct_dialogs::choose_item_dialog(parent, _("Select Node Icon"), itemStore);
        if (res) {
            nodeData.customIconId = std::stoi(res->get_value(itemStore->columns.key));
            c_icon_button.set_label("");
            c_icon_button.set_image(*CtImage::new_image_from_stock(res->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
        }
    });

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return false;

    nodeData.name = name_entry.get_text();
    if (nodeData.name.empty())
        nodeData.name = CtConst::CHAR_QUESTION;
    if (radiobutton_rich_text.get_active())
        nodeData.syntax = CtConst::RICH_TEXT_ID;
    else if (radiobutton_plain_text.get_active())
        nodeData.syntax = CtConst::PLAIN_TEXT_ID;
    else {
        nodeData.syntax = button_prog_lang.get_label();
        CtApp::P_ctCfg->autoSynHighl = nodeData.syntax;
    }
    nodeData.tags = tags_entry.get_text();
    nodeData.isRO = ro_checkbutton.get_active();
    nodeData.customIconId = c_icon_checkbutton.get_active() ? nodeData.customIconId : 0;
    nodeData.isBold = is_bold_checkbutton.get_active();
    if (fg_checkbutton.get_active()) {
        nodeData.foregroundRgb24 = CtRgbUtil::getRgb24StrFromStrAny(fg_colorbutton.get_color().to_string());
        CtApp::P_ctCfg->currColors['n'] = nodeData.foregroundRgb24;
    }
    return true;
}
