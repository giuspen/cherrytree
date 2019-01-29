#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include "ct_image.h"
#include "ct_app.h"
#include "ct_dialogs.h"
#include "ct_doc_rw.h"


CtNodeData dialog_node_prop(std::string title, Gtk::Window& parent, std::string name,
                      bool is_bold, std::string fg, guint32 c_icon_id, std::string syntax_highl,
                      bool ro, std::string tags, const std::set<std::string>& tags_set);


bool CtActions::is_there_selected_node_or_error()
{
    if (_ctMainWin->curr_tree_iter()) return true;
    ct_dialogs::warning_dialog(_("No Node is Selected"), *_ctMainWin);
    return false;
}

void CtActions::_node_add(bool duplicate)
{
    CtNodeData node;
    if (duplicate)
     {
        if (!is_there_selected_node_or_error()) return;
        _ctTreestore->getNodeData(_ctMainWin->curr_tree_iter(), node);
        node.anchoredWidgets.clear();
        node.rTextBuffer.clear();
    }
    else
    {
        node = dialog_node_prop(_("New Node Properties"), *_ctMainWin, "", false, "", 0, CtConst::RICH_TEXT_ID, false, "", _ctTreestore->get_used_tags());
        if (node.name.empty()) return;
    }

    node.tsCreation = std::time(nullptr);
    node.tsLastSave = node.tsCreation;
    node.nodeId = _ctTreestore->node_id_get();

    _ctMainWin->update_window_save_needed();
    CtApp::P_ctCfg->syntaxHighlighting = node.syntax;

    Gtk::TreeIter nodeIter;
    if (_ctMainWin->curr_tree_iter())
        nodeIter = _ctTreestore->insertNode(&node, _ctMainWin->curr_tree_iter());
    else
        nodeIter = _ctTreestore->appendNode(&node);

    _ctTreestore->ctdb_handler()->pending_new_db_node(node.nodeId);
    _ctTreestore->nodes_sequences_fix(_ctMainWin->curr_tree_iter()->parent(), false);
    _ctTreestore->updateNodeAuxIcon(nodeIter);
    /* todo
    self.nodes_names_dict[new_node_id] = ret_name
    if self.node_add_is_duplication:
        if self.syntax_highlighting != cons.RICH_TEXT_ID:
            text_buffer_from = self.treestore[tree_iter_from][2]
            text_buffer_to = self.treestore[new_node_iter][2]
            content = text_buffer_from.get_text(*text_buffer_from.get_bounds())
            text_buffer_to.begin_not_undoable_action()
            text_buffer_to.set_text(content)
            text_buffer_to.end_not_undoable_action()
        else:
            state = self.state_machine.requested_state_previous(self.treestore[tree_iter_from][3])
            self.load_buffer_from_state(state, given_tree_iter=new_node_iter)
    */
    _ctMainWin->get_tree_view().set_cursor(_ctTreestore->get_path(nodeIter));
    _ctMainWin->get_text_view().grab_focus();
}

void CtActions::node_child_add()
{

}

void CtActions::node_edit()
{
    if (!is_there_selected_node_or_error()) return;
    CtNodeData cur_data;
    _ctTreestore->getNodeData(_ctMainWin->curr_tree_iter(), cur_data);
    CtNodeData new_data = dialog_node_prop(_("Node Properties"), *_ctMainWin, cur_data.name, cur_data.isBold,
                                           cur_data.foregroundRgb24, cur_data.customIconId, cur_data.syntax,
                                           cur_data.isRO, cur_data.tags, _ctTreestore->get_used_tags());
    if (new_data.name.empty()) return;

    CtApp::P_ctCfg->syntaxHighlighting = new_data.syntax;
    if (cur_data.syntax !=  new_data.syntax) {
        if (cur_data.syntax == CtConst::RICH_TEXT_ID) {
            // leaving rich text
            if (!ct_dialogs::question_dialog(_("Leaving the Node Type Rich Text you will Lose all Formatting for This Node, Do you want to Continue?"), *_ctMainWin)) {
                return;
            }
            // todo:
            // SWITCH TextBuffer -> SourceBuffer
            //self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
            //self.curr_buffer = self.treestore[self.curr_tree_iter][2]
            //self.state_machine.delete_states(self.get_node_id_from_tree_iter(self.curr_tree_iter))
        } else if (new_data.syntax == CtConst::RICH_TEXT_ID) {
            // going to rich text
            // SWITCH SourceBuffer -> TextBuffer
            //self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
            //self.curr_buffer = self.treestore[self.curr_tree_iter][2]
        } else if (cur_data.syntax == CtConst::PLAIN_TEXT_ID) {
            // plain text to code
            //self.sourceview.modify_font(pango.FontDescription(self.code_font))
        } else if (new_data.syntax == CtConst::PLAIN_TEXT_ID) {
            // code to plain text
            // self.sourceview.modify_font(pango.FontDescription(self.pt_font))
        }
        _ctTreestore->updateNodeData(_ctMainWin->curr_tree_iter(), new_data);
        //if self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
        //  self.set_sourcebuffer_syntax_highlight(self.curr_buffer, self.syntax_highlighting)
        _ctMainWin->get_text_view().set_editable(!new_data.isRO);
        //self.update_selected_node_statusbar_info()
        _ctTreestore->updateNodeAuxIcon(_ctMainWin->curr_tree_iter());
        //self.treeview_set_colors()
        //self.update_node_name_header()
        _ctMainWin->update_window_save_needed("npro");
        _ctMainWin->get_text_view().grab_focus();
    }

}

CtNodeData dialog_node_prop(std::string title, Gtk::Window& parent, std::string name,
                      bool is_bold, std::string fg, guint32 c_icon_id, std::string syntax_highl,
                      bool ro, std::string tags, const std::set<std::string>& tags_set)
{
    auto dialog = Gtk::Dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(300, -1);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    auto name_entry = Gtk::Entry();
    name_entry.set_text(name);
    auto is_bold_checkbutton = Gtk::CheckButton(_("Bold"));
    is_bold_checkbutton.set_active(is_bold);
    auto fg_checkbutton = Gtk::CheckButton(_("Use Selected Color"));
    fg_checkbutton.set_active(fg != "");
    std::string real_fg = fg != "" ? fg : (CtApp::P_ctCfg->currColors.at('n') != "" ? CtApp::P_ctCfg->currColors.at('n') : "red");
    auto fg_colorbutton = Gtk::ColorButton(Gdk::RGBA(real_fg));
    fg_colorbutton.set_sensitive(fg != "");
    auto fg_hbox = Gtk::HBox();
    fg_hbox.set_spacing(2);
    fg_hbox.pack_start(fg_checkbutton, false, false);
    fg_hbox.pack_start(fg_colorbutton, false, false);
    auto c_icon_checkbutton = Gtk::CheckButton(_("Use Selected Icon"));
    c_icon_checkbutton.set_active(map::exists(CtConst::NODES_STOCKS, c_icon_id));
    auto c_icon_button = Gtk::Button();
    if (c_icon_checkbutton.get_active())
        c_icon_button.set_image(*CtImage::new_image_from_stock(CtConst::NODES_STOCKS.at(c_icon_id), Gtk::ICON_SIZE_BUTTON));
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
    std::string syntax_hl_id = syntax_highl;
    if (syntax_highl == CtConst::RICH_TEXT_ID || syntax_highl == CtConst::PLAIN_TEXT_ID)
        syntax_hl_id = CtApp::P_ctCfg->autoSynHighl;
    std::string button_stock_id = CtConst::getStockIdForCodeType(syntax_hl_id);
    button_prog_lang.set_label(syntax_hl_id);
    button_prog_lang.set_image(*CtImage::new_image_from_stock(button_stock_id, Gtk::ICON_SIZE_MENU));
    if (syntax_highl == CtConst::RICH_TEXT_ID) {
        radiobutton_rich_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    } else if (syntax_highl == CtConst::PLAIN_TEXT_ID) {
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
    type_frame.set_sensitive(ro == false);
    auto tags_hbox = Gtk::HBox();
    tags_hbox.set_spacing(2);
    auto tags_entry = Gtk::Entry();
    tags_entry.set_text(tags);
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
    ro_checkbutton.set_active(ro);
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
    c_icon_button.signal_clicked().connect([&parent, &c_icon_button, &c_icon_id](){
        auto itemStore = ct_dialogs::CtChooseDialogListStore::create();
        for (auto& pair: CtConst::NODES_ICONS)
            itemStore->add_row(pair.second, std::to_string(pair.first), "");
        auto res = ct_dialogs::choose_item_dialog(parent, _("Select Node Icon"), itemStore);
        if (res) {
            c_icon_id = std::stoi(res->get_value(itemStore->columns.key));
            c_icon_button.set_label("");
            c_icon_button.set_image(*CtImage::new_image_from_stock(res->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
        }
    });

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return CtNodeData();

    CtNodeData node;
    node.name = name_entry.get_text();
    if (node.name.empty())
        node.name = CtConst::CHAR_QUESTION;
    if (radiobutton_rich_text.get_active())
        node.syntax = CtConst::RICH_TEXT_ID;
    else if (radiobutton_plain_text.get_active())
        node.syntax = CtConst::PLAIN_TEXT_ID;
    else {
        node.syntax = button_prog_lang.get_label();
        CtApp::P_ctCfg->autoSynHighl = node.syntax;
    }
    node.tags = tags_entry.get_text();
    node.isRO = ro_checkbutton.get_active();
    node.customIconId = c_icon_checkbutton.get_active() ? c_icon_id : 0;
    node.isBold = is_bold_checkbutton.get_active();
    if (fg_checkbutton.get_active()) {
        std::string foregroundRgb24 = CtRgbUtil::getRgb24StrFromStrAny(fg_colorbutton.get_color().to_string());
        node.fgOverride = true;
        g_strlcpy(node.foregroundRgb24, foregroundRgb24.c_str(), 8);
        CtApp::P_ctCfg->currColors['n'] = foregroundRgb24;
    }
    return node;
}
