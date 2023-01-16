/*
 * ct_pref_dlg_plain_text_n_code.cc
 *
 * Copyright 2009-2023
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

#include "ct_pref_dlg.h"
#include "ct_main_win.h"

Gtk::Widget* CtPrefDlg::build_tab_plain_text_n_code()
{
    auto vbox_syntax = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});

    auto checkbutton_pt_show_white_spaces = Gtk::manage(new Gtk::CheckButton{_("Show White Spaces")});
    checkbutton_pt_show_white_spaces->set_active(_pConfig->ptShowWhiteSpaces);
    auto checkbutton_pt_highl_curr_line = Gtk::manage(new Gtk::CheckButton{_("Highlight Current Line")});
    checkbutton_pt_highl_curr_line->set_active(_pConfig->ptHighlCurrLine);
    auto checkbutton_pt_highl_match_bra = Gtk::manage(new Gtk::CheckButton{_("Highlight Matching Brackets")});
    checkbutton_pt_highl_match_bra->set_active(_pConfig->ptHighlMatchBra);

    vbox_syntax->pack_start(*checkbutton_pt_show_white_spaces, false, false);
    vbox_syntax->pack_start(*checkbutton_pt_highl_curr_line, false, false);
    vbox_syntax->pack_start(*checkbutton_pt_highl_match_bra, false, false);

    Gtk::Frame* frame_syntax = new_managed_frame_with_align(_("Text Editor"), vbox_syntax);

    auto checkbutton_code_exec_confirm = Gtk::manage(new Gtk::CheckButton{_("Ask Confirmation Before Executing the Code")});
    checkbutton_code_exec_confirm->set_active(_pConfig->codeExecConfirm);
#if defined(HAVE_VTE)
    auto checkbutton_code_exec_vte = Gtk::manage(new Gtk::CheckButton{_("Use Internal Terminal")});
    checkbutton_code_exec_vte->set_active(_pConfig->codeExecVte);
    auto hbox_vte_shell = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_vte_shell = Gtk::manage(new Gtk::Label{_("Internal Terminal Shell")});
    auto entry_vte_shell = Gtk::manage(new Gtk::Entry{});
    entry_vte_shell->set_text(_pConfig->vteShell);
    entry_vte_shell->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
    hbox_vte_shell->pack_start(*label_vte_shell, false, false);
    hbox_vte_shell->pack_start(*entry_vte_shell, false, false);
    hbox_vte_shell->set_sensitive(_pConfig->codeExecVte);
#endif // HAVE_VTE
    Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(_commandModelColumns);
    _fill_custom_exec_commands_model(liststore);
    auto treeview = Gtk::manage(new Gtk::TreeView{liststore});
    treeview->set_headers_visible(false);
    treeview->set_size_request(300, 200);

    Gtk::CellRendererPixbuf pixbuf_renderer;
    pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
    const int col_num_pixbuf = treeview->append_column("", pixbuf_renderer) - 1;
    treeview->get_column(col_num_pixbuf)->add_attribute(pixbuf_renderer, "icon-name", _shortcutModelColumns.icon);

    treeview->append_column("", _commandModelColumns.key);
    const int col_num_ext = treeview->append_column_editable("", _commandModelColumns.ext) - 1;
    const int col_num_desc = treeview->append_column_editable("", _commandModelColumns.desc) - 1;

    auto scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow{});
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow->add(*treeview);

    auto button_add = Gtk::manage(new Gtk::Button{});
    button_add->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_add", Gtk::ICON_SIZE_BUTTON));
    button_add->set_tooltip_text(_("Add"));
    auto button_remove = Gtk::manage(new Gtk::Button{});
    button_remove->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_remove", Gtk::ICON_SIZE_BUTTON));
    button_remove->set_tooltip_text(_("Remove Selected"));
    auto button_reset_cmds = Gtk::manage(new Gtk::Button{});
    button_reset_cmds->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_cmds->set_tooltip_text(_("Reset to Default"));
    Gtk::VBox* vbox_buttons = Gtk::manage(new Gtk::VBox());
    vbox_buttons->pack_start(*button_add, false, false);
    vbox_buttons->pack_start(*button_remove, false, false);
    vbox_buttons->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    vbox_buttons->pack_start(*button_reset_cmds, false, false);

    auto vbox_codexec = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto hbox_term_run = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    auto entry_term_run = Gtk::manage(new Gtk::Entry{});
    entry_term_run->set_text(get_code_exec_term_run(_pCtMainWin));
    auto button_reset_term = Gtk::manage(new Gtk::Button{});
    button_reset_term->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_term->set_tooltip_text(_("Reset to Default"));
    hbox_term_run->pack_start(*entry_term_run, true, true);
    hbox_term_run->pack_start(*button_reset_term, false, false);
    auto hbox_cmd_per_type = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    hbox_cmd_per_type->pack_start(*scrolledwindow, true, true);
    hbox_cmd_per_type->pack_start(*vbox_buttons, false, false);

    auto label = Gtk::manage(new Gtk::Label{Glib::ustring{"<b>"}+_("Command per Node/CodeBox Type")+"</b>"});
    label->set_use_markup(true);
    vbox_codexec->pack_start(*checkbutton_code_exec_confirm, false, false);
#if defined(HAVE_VTE)
    vbox_codexec->pack_start(*checkbutton_code_exec_vte, false, false);
    vbox_codexec->pack_start(*hbox_vte_shell, false, false);
#endif // HAVE_VTE
    vbox_codexec->pack_start(*label, false, false);
    vbox_codexec->pack_start(*hbox_cmd_per_type, true, true);
    auto label2 = Gtk::manage(new Gtk::Label{Glib::ustring{"<b>"}+_("Terminal Command")+"</b>"});
    label2->set_use_markup(true);
    vbox_codexec->pack_start(*label2, false, false);
    vbox_codexec->pack_start(*hbox_term_run, false, false);

    Gtk::Frame* frame_codexec = new_managed_frame_with_align(_("Code Execution"), vbox_codexec);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_syntax, false, false);
    pMainBox->pack_start(*frame_codexec, true, true);

    checkbutton_pt_show_white_spaces->signal_toggled().connect([this, checkbutton_pt_show_white_spaces](){
        _pConfig->ptShowWhiteSpaces = checkbutton_pt_show_white_spaces->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('p'/*PlainTextNCode*/);
        });
    });
    checkbutton_pt_highl_curr_line->signal_toggled().connect([this, checkbutton_pt_highl_curr_line](){
        _pConfig->ptHighlCurrLine = checkbutton_pt_highl_curr_line->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('p'/*PlainTextNCode*/);
        });
    });
    checkbutton_pt_highl_match_bra->signal_toggled().connect([this, checkbutton_pt_highl_match_bra](){
        _pConfig->ptHighlMatchBra = checkbutton_pt_highl_match_bra->get_active();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
    });
    checkbutton_code_exec_confirm->signal_toggled().connect([this, checkbutton_code_exec_confirm](){
        _pConfig->codeExecConfirm = checkbutton_code_exec_confirm->get_active();
    });
#if defined(HAVE_VTE)
    checkbutton_code_exec_vte->signal_toggled().connect([this, checkbutton_code_exec_vte, hbox_vte_shell](){
        _pConfig->codeExecVte = checkbutton_code_exec_vte->get_active();
        hbox_vte_shell->set_sensitive(_pConfig->codeExecVte);
    });
    entry_vte_shell->signal_changed().connect([this, entry_vte_shell](){
        fs::path shell_path{entry_vte_shell->get_text()};
        if (fs::exists(fs::canonical(shell_path))) {
            _pConfig->vteShell = entry_vte_shell->get_text();
            entry_vte_shell->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
        else {
            entry_vte_shell->set_icon_from_icon_name("ct_urgent", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
    });
    entry_vte_shell->signal_icon_release().connect([entry_vte_shell](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
        entry_vte_shell->set_text(CtConst::VTE_SHELL_BASH);
    });
#endif // HAVE_VTE
    Gtk::CellRendererText* pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(treeview->get_column(col_num_desc)->get_cells()[0]);
    pCellRendererText->signal_edited().connect([this, liststore](const Glib::ustring& path, const Glib::ustring& new_command){
        auto row = liststore->get_iter(path);
        row->set_value(_commandModelColumns.desc, new_command);
        _pConfig->customCodexecType[row->get_value(_commandModelColumns.key)] = new_command;
    });
    pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(treeview->get_column(col_num_ext)->get_cells()[0]);
    pCellRendererText->signal_edited().connect([this, liststore](const Glib::ustring& path, const Glib::ustring& new_ext){
        auto row = liststore->get_iter(path);
        row->set_value(_commandModelColumns.ext, new_ext);
        _pConfig->customCodexecExt[row->get_value(_commandModelColumns.key)] = new_ext;
    });
    entry_term_run->signal_changed().connect([this, entry_term_run](){
        _pConfig->customCodexecTerm = entry_term_run->get_text();
    });
    button_add->signal_clicked().connect([this, treeview, liststore](){
        _add_new_command_in_model(treeview, liststore);
    });
    button_remove->signal_clicked().connect([this, treeview, liststore](){
        _remove_command_from_model(treeview, liststore);
    });
    auto button_remove_test_sensitive = [button_remove, treeview](){
        button_remove->set_sensitive(treeview->get_selection()->get_selected());
    };
    treeview->signal_cursor_changed().connect(button_remove_test_sensitive);
    button_remove_test_sensitive();
    button_reset_cmds->signal_clicked().connect([this, liststore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            _pConfig->customCodexecType.clear();
            _fill_custom_exec_commands_model(liststore);
        }
    });
    button_reset_term->signal_clicked().connect([this, entry_term_run](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            _pConfig->customCodexecTerm.clear();
            entry_term_run->set_text(get_code_exec_term_run(_pCtMainWin));
        }
    });

    return pMainBox;
}

std::string CtPrefDlg::get_code_exec_term_run(CtMainWin* pCtMainWin)
{
    if (!pCtMainWin->get_ct_config()->customCodexecTerm.empty())
    {
        return pCtMainWin->get_ct_config()->customCodexecTerm;
    }
    const std::string op_sys =
#ifdef _WIN32
        "win";
#else
        "linux";
#endif
    return CtConst::CODE_EXEC_TERM_RUN_DEFAULT.at(op_sys);
}

std::set<std::string> CtPrefDlg::_get_code_exec_type_keys()
{
    std::set<std::string> retSetCodexecTypeKeys;
    for (const auto& it : _pCtMainWin->get_ct_config()->customCodexecType) {
        retSetCodexecTypeKeys.insert(it.first);
    }
    for (const auto& it : CtConst::CODE_EXEC_TYPE_CMD_DEFAULT) {
        retSetCodexecTypeKeys.insert(it.first);
    }
    return retSetCodexecTypeKeys;
}

std::string CtPrefDlg::get_code_exec_ext(CtMainWin* pCtMainWin, const std::string code_type)
{
    for (const auto& it : pCtMainWin->get_ct_config()->customCodexecExt) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    for (const auto& it : CtConst::CODE_EXEC_TYPE_EXT_DEFAULT) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    return "txt";
}

std::string CtPrefDlg::get_code_exec_type_cmd(CtMainWin* pCtMainWin, const std::string code_type)
{
    for (const auto& it : pCtMainWin->get_ct_config()->customCodexecType) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    for (const auto& it : CtConst::CODE_EXEC_TYPE_CMD_DEFAULT) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    return std::string{};
}

void CtPrefDlg::_fill_custom_exec_commands_model(Glib::RefPtr<Gtk::ListStore> rModel)
{
    rModel->clear();
    for (const auto& code_type : _get_code_exec_type_keys())
    {
        Gtk::TreeModel::Row row = *(rModel->append());
        row[_commandModelColumns.icon] = _pCtMainWin->get_code_icon_name(code_type);
        row[_commandModelColumns.key] = code_type;
        row[_commandModelColumns.ext] = CtPrefDlg::get_code_exec_ext(_pCtMainWin, code_type);
        row[_commandModelColumns.desc] = CtPrefDlg::get_code_exec_type_cmd(_pCtMainWin, code_type);
    }
}

void CtPrefDlg::_add_new_command_in_model(Gtk::TreeView* pTreeview, Glib::RefPtr<Gtk::ListStore> rModel)
{
    const std::set<std::string> all_codexec_keys = _get_code_exec_type_keys();
    auto itemStore = CtChooseDialogListStore::create();
    for (const auto& lang : _pCtMainWin->get_language_manager()->get_language_ids())
    {
        if (0 == all_codexec_keys.count(lang)) {
            itemStore->add_row(_pCtMainWin->get_code_icon_name(lang), "", lang);
        }
    }
    const Gtk::TreeIter treeIterChosen = CtDialogs::choose_item_dialog(*this, _("Select Element to Add"), itemStore);
    if (treeIterChosen) {
        const auto code_type = treeIterChosen->get_value(itemStore->columns.desc);
        Gtk::TreeIter newTreeIter;
        Gtk::TreeIter loopPrevTreeIter;
        for (const auto& currTreeIter : rModel->children()) {
            const int result = currTreeIter->get_value(_commandModelColumns.key).compare(code_type);
            if (result > 0) {
                newTreeIter = loopPrevTreeIter ? rModel->insert_after(loopPrevTreeIter) : rModel->prepend();
                break;
            }
            loopPrevTreeIter = currTreeIter;
        }
        if (not newTreeIter) {
            newTreeIter = rModel->append();
        }
        Gtk::TreeModel::Row row = *newTreeIter;
        row[_commandModelColumns.icon] = _pCtMainWin->get_code_icon_name(code_type);
        row[_commandModelColumns.key] = code_type;
        row[_commandModelColumns.ext] = CtPrefDlg::get_code_exec_ext(_pCtMainWin, code_type);
        row[_commandModelColumns.desc] = std::string{"REPLACE_ME "} + CtConst::CODE_EXEC_TMP_SRC;
        pTreeview->set_cursor(rModel->get_path(newTreeIter));
    }
}

void CtPrefDlg::_remove_command_from_model(Gtk::TreeView* pTreeview, Glib::RefPtr<Gtk::ListStore> rModel)
{
    Gtk::TreeIter sel_iter = pTreeview->get_selection()->get_selected();
    if (sel_iter) {
        const auto code_type = sel_iter->get_value(_commandModelColumns.key);
        auto& customCodexecType = _pCtMainWin->get_ct_config()->customCodexecType;
        auto& customCodexecExt = _pCtMainWin->get_ct_config()->customCodexecExt;
        if (customCodexecType.count(code_type)) {
            customCodexecType.erase(code_type);
        }
        if (customCodexecExt.count(code_type)) {
            customCodexecExt.erase(code_type);
        }
        rModel->erase(sel_iter);
    }
}
