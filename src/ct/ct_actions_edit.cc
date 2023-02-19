/*
 * ct_actions_find.cc
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

#include "ct_actions.h"
#include "ct_clipboard.h"
#include "ct_list.h"
#include "ct_image.h"
#include "ct_logging.h"
#include "ct_storage_control.h"
#include <gtkmm/dialog.h>

// Step Back for the Current Node, if Possible
void CtActions::requested_step_back()
{
    CtTreeIter currTreeIter = _pCtMainWin->curr_tree_iter();
    if (not currTreeIter) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    if (currTreeIter.get_node_is_rich_text()) {
        auto step_back = _pCtMainWin->get_state_machine().requested_state_previous(currTreeIter.get_node_id());
        if (step_back) {
            _pCtMainWin->load_buffer_from_state(step_back, currTreeIter);
        }
    }
    else if (currTreeIter.get_node_text_buffer()->can_undo()) {
        currTreeIter.get_node_text_buffer()->undo();
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
    }
}

// Step Ahead for the Current Node, if Possible
void CtActions::requested_step_ahead()
{
    CtTreeIter currTreeIter = _pCtMainWin->curr_tree_iter();
    if (not currTreeIter) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    if (currTreeIter.get_node_is_rich_text()) {
        auto step_ahead = _pCtMainWin->get_state_machine().requested_state_subsequent(currTreeIter.get_node_id());
        if (step_ahead) {
            _pCtMainWin->load_buffer_from_state(step_ahead, currTreeIter);
        }
    }
    else if (currTreeIter.get_node_text_buffer()->can_redo()) {
        currTreeIter.get_node_text_buffer()->redo();
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
    }
}

void CtActions::latex_insert()
{
    if (not _node_sel_and_rich_text()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    CtImageLatex::ensureRenderingBinariesTested();
    _latex_edit_dialog(CtImageLatex::LatexTextDefault, _curr_buffer()->get_insert()->get_iter(), nullptr/*pIterBound*/);
}

void CtActions::image_insert()
{
    if (not _node_sel_and_rich_text()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtConfig->pickDirImg;

    std::string filename = CtDialogs::file_select_dialog(args);
    if (filename.empty()) return;
    _pCtConfig->pickDirImg = Glib::path_get_dirname(filename);

    Glib::RefPtr<Gdk::Pixbuf> rPixbuf;
    try {
        rPixbuf = Gdk::Pixbuf::create_from_file(filename);
    }
    catch (Glib::Error& error) {
        spdlog::error("{} {}", __FUNCTION__, error.what());
    }
    if (rPixbuf)
        _image_edit_dialog(rPixbuf, _curr_buffer()->get_insert()->get_iter(), nullptr/*pIterBound*/);
    else
        CtDialogs::error_dialog(_("Image Format Not Recognized"), *_pCtMainWin);
}

void CtActions::table_insert()
{
    if (not _node_sel_and_rich_text()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    bool is_light{_pCtConfig->tableColumns*_pCtConfig->tableRows > _pCtConfig->tableCellsGoLight};
    CtDialogs::TableHandleResp res = CtDialogs::table_handle_dialog(
        _pCtMainWin,
        _("Insert Table"),
        true/*is_insert*/,
        is_light);
    if (res == CtDialogs::TableHandleResp::Cancel) return;

    std::list<std::vector<std::string>> rows;
    if (res == CtDialogs::TableHandleResp::Ok) {
        std::vector<std::string> empty_row(_pCtConfig->tableColumns, "");
        while ((int)rows.size() < _pCtConfig->tableRows) {
            rows.push_back(empty_row);
        }
    }
    CtTableMatrix tbl_matrix;
    const auto charOffset = _curr_buffer()->get_insert()->get_iter().get_offset();
    int col_width = _pCtConfig->tableColWidthDefault;
    if (res == CtDialogs::TableHandleResp::OkFromFile) {
        CtDialogs::FileSelectArgs args{_pCtMainWin};
        args.curr_folder = _pCtConfig->pickDirCsv;
        args.curr_file_name.clear();
        args.filter_name = _("CSV File");
        args.filter_pattern = {"*.csv"};

        std::string filepath = CtDialogs::file_select_dialog(args);
        if (filepath.empty()) return;
        _pCtConfig->pickDirCsv = Glib::path_get_dirname(filepath);
        CtTableCommon::populate_table_matrix_from_csv(filepath, _pCtMainWin, is_light, tbl_matrix);
        col_width = 60;
    }
    else {
        for (auto& row : rows) {
            tbl_matrix.push_back(CtTableRow{});
            for (auto& cell : row) {
                if (is_light) {
                    tbl_matrix.back().push_back(new Glib::ustring{});
                } else {
                    tbl_matrix.back().push_back(new CtTextCell{_pCtMainWin, cell, CtConst::TABLE_CELL_TEXT_ID});    
                }
            }
        }
    }
    CtTableCommon* pCtTable{nullptr};
    if (is_light) {
        pCtTable = new CtTableLight{_pCtMainWin, tbl_matrix, col_width, charOffset, "", CtTableColWidths{}};
    }
    else {
        pCtTable = new CtTableHeavy{_pCtMainWin, tbl_matrix, col_width, charOffset, "", CtTableColWidths{}};
    }
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pCtTable->insertInTextBuffer(gsv_buffer);

    _pCtMainWin->get_tree_store().addAnchoredWidgets(_pCtMainWin->curr_tree_iter(),
        {pCtTable}, &_pCtMainWin->get_text_view());
    //pCtTable->get_text_view().grab_focus();
}

void CtActions::codebox_insert()
{
    if (not _node_sel_and_rich_text()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    Glib::ustring textContent, justification;
    Gtk::TextIter iter_sel_start, iter_sel_end;
    if (_curr_buffer()->get_has_selection()) {
        _curr_buffer()->get_selection_bounds(iter_sel_start, iter_sel_end);
        textContent = iter_sel_start.get_text(iter_sel_end);
    }
    if (not CtDialogs::codeboxhandle_dialog(_pCtMainWin, _("Insert a CodeBox"))) {
        return;
    }
    if (not textContent.empty()) {
        _curr_buffer()->erase(iter_sel_start, iter_sel_end);
    }
    Gtk::TextIter iter_insert = _curr_buffer()->get_insert()->get_iter();

    CtCodebox* pCtCodebox = new CtCodebox{_pCtMainWin,
                                          textContent,
                                          _pCtConfig->codeboxSynHighl,
                                          (int)_pCtConfig->codeboxWidth,
                                          (int)_pCtConfig->codeboxHeight,
                                          iter_insert.get_offset(),
                                          justification,
                                          _pCtConfig->codeboxWidthPixels,
                                          _pCtConfig->codeboxMatchBra,
                                          _pCtConfig->codeboxLineNum};
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pCtCodebox->insertInTextBuffer(gsv_buffer);

    _pCtMainWin->get_tree_store().addAnchoredWidgets(_pCtMainWin->curr_tree_iter(),
                                                     {pCtCodebox},
                                                     &_pCtMainWin->get_text_view());
    pCtCodebox->get_text_view().grab_focus();
}

void CtActions::embfile_insert_path(const std::string& filepath)
{
    if (!_node_sel_and_rich_text()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;

    if (fs::file_size(filepath) > static_cast<uintmax_t>(_pCtConfig->embfileMaxSize * 1024 * 1024)) {
        bool is_sqlite = fs::get_doc_type(_pCtMainWin->get_ct_storage()->get_file_path()) == CtDocType::SQLite;
        auto message = str::format(_("The Maximum Size for Embedded Files is %s MB."), _pCtConfig->embfileMaxSize);
        if (is_sqlite) {
            if (!CtDialogs::question_dialog(message + "\n" + _("Do you want to Continue?"), *_pCtMainWin))
                return;
        }
        else {
            CtDialogs::error_dialog(message, *_pCtMainWin);
            return;
        }
    }

    std::string blob = Glib::file_get_contents(filepath);
    std::string name = Glib::path_get_basename(filepath);
    CtAnchoredWidget* pAnchoredWidget = new CtImageEmbFile{_pCtMainWin,
                                                           name,
                                                           blob,
                                                           std::time(nullptr),
                                                           _curr_buffer()->get_insert()->get_iter().get_offset(),
                                                           "",
                                                           CtImageEmbFile::get_next_unique_id()};
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);

    _pCtMainWin->get_tree_store().addAnchoredWidgets(_pCtMainWin->curr_tree_iter(),
                                                     {pAnchoredWidget},
                                                     &_pCtMainWin->get_text_view());
}

void CtActions::embfile_insert()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtConfig->pickDirFile;

    std::string filepath = CtDialogs::file_select_dialog(args);
    if (filepath.empty()) return;

    _pCtConfig->pickDirFile = Glib::path_get_dirname(filepath);

    embfile_insert_path(filepath);
}

// The Link Insert Button was Pressed
void CtActions::apply_tag_link()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (_curr_buffer()->get_has_selection()) {
        Gtk::TextIter iter_sel_start, iter_sel_end;
        _curr_buffer()->get_selection_bounds(iter_sel_start, iter_sel_end);
        if (1 == (iter_sel_end.get_offset() - iter_sel_start.get_offset())) {
            auto widgets = _pCtMainWin->curr_tree_iter().get_anchored_widgets(iter_sel_start.get_offset(), iter_sel_start.get_offset());
            if (not widgets.empty()) {
                if (CtImagePng* image = dynamic_cast<CtImagePng*>(widgets.front())) {
                    curr_image_anchor = image;
                    image_link_edit();
                    return;
                }
            }
        }
    }
    apply_tag(CtConst::TAG_LINK);
}

// Insert an Anchor
void CtActions::anchor_handle()
{
    if (!_node_sel_and_rich_text()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    _anchor_edit_dialog(nullptr, _curr_buffer()->get_insert()->get_iter(), nullptr);
}

struct TocEntry
{
    std::string anchor_link;
    std::string text;
    bool is_node = false;
    unsigned int depth = 0;
    unsigned int h_level = 0;
    std::list<TocEntry> children;
    TocEntry(std::string a_link, bool is_n, std::string txt, unsigned int dep, unsigned int h_lvl = 0)
     : anchor_link(std::move(a_link))
     , text(std::move(txt))
     , is_node(is_n)
     , depth(dep)
     , h_level(h_lvl)
    {}
};

std::optional<Glib::ustring> iter_in_tag(const Gtk::TextIter& iter, const Glib::ustring& tag)
{
    for (const auto& iter_tag : iter.get_tags()) {
        Glib::ustring tag_name;
        iter_tag->get_property("name", tag_name);
        //spdlog::debug("TAG: {}", tag_name);
        if (str::startswith(tag_name, tag)) {
            return tag_name;
        }
    }
    return std::nullopt;
}

TocEntry find_toc_entries(CtActions& actions, CtTreeIter& node, unsigned depth)
{
    int node_id = node.get_node_id();
    TocEntry entry{fmt::format("node {}", node_id), true, node.get_node_name(), depth};
    std::string scale_tag{"scale_"};
    std::unordered_map<int, int> encountered_headers;
    auto text_buffer = node.get_node_text_buffer();
    Gtk::TextIter text_iter = text_buffer->begin();

    do {
        std::optional<Glib::ustring> tag_name = iter_in_tag(text_iter, scale_tag);
        if (tag_name and not Glib::str_has_suffix(tag_name.value(), CtConst::TAG_PROP_VAL_SMALL)) {
            auto h_start = tag_name->find(scale_tag) + scale_tag.length() + 1;
            auto begin = tag_name->begin();
            std::advance(begin, h_start);
            Glib::ustring h_level_str{begin, tag_name->end()};
            try {
                int h_lvl = std::stoi(h_level_str);
                encountered_headers[h_lvl] += 1;

                Gtk::TextIter start_iter{text_iter};
                Gtk::TextIter end_iter{text_iter};
                while (not start_iter.starts_line()) {
                    if (not start_iter.backward_word_start()) break;
                }

                while (not end_iter.ends_line() and not end_iter.get_child_anchor()) {
                    if (not end_iter.forward_char()) break;
                }

                Glib::ustring txt{start_iter, end_iter};
                //spdlog::debug("{} - {}", txt, txt.size());

                auto mark = text_buffer->create_mark(end_iter, false);

                Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = end_iter.get_child_anchor();
                if (rChildAnchor) {
                    CtAnchoredWidget* pCtAnchoredWidget = node.get_anchored_widget(rChildAnchor);
                    if (pCtAnchoredWidget) {
                        auto pCtImageAnchor = dynamic_cast<CtImageAnchor*>(pCtAnchoredWidget);
                        static Glib::RefPtr<Glib::Regex> rRegExpAnchorName = Glib::Regex::create("h\\d+-\\d+");
                        if (pCtImageAnchor and rRegExpAnchorName->match(pCtImageAnchor->get_anchor_name())) {
                            const int endOffset = end_iter.get_offset();
                            auto iter_bound = end_iter;
                            iter_bound.forward_char();
                            text_buffer->erase(end_iter, iter_bound);
                            end_iter = text_buffer->get_iter_at_offset(endOffset);
                        }
                    }
                }
                const std::string anchor_txt = fmt::format("h{}-{}", h_lvl, encountered_headers[h_lvl]);
                actions.image_insert_anchor(end_iter, anchor_txt, "right");

                text_iter = mark->get_iter();
                text_buffer->delete_mark(mark);
                //spdlog::debug("INSERT DONE");
                entry.children.emplace_back(fmt::format("node {} {}", node_id, anchor_txt), false, txt, depth + 1, h_lvl);
            }
            catch(std::invalid_argument&) {
                spdlog::error("Could not convert [{}] to an integer", h_level_str);
            }
        }

    } while (text_iter.forward_line());

    return entry;
}

void CtActions::_insert_toc_at_pos(Glib::RefPtr<Gtk::TextBuffer> text_buffer, const std::list<TocEntry>& entries)
{
    for (const auto& entry : entries) {
        Glib::ustring bullet_char;
        CtStringSplittable& bullets_list = _pCtConfig->charsToc;
        auto nb_indents = entry.depth;
        if (entry.is_node) {
            bullet_char = bullets_list[0];
        }
        else {
            size_t bullet_index = entry.h_level + 1;
            if (bullet_index >= bullets_list.size()) {
                bullet_index = bullets_list.size() - 1;
            }
            bullet_char = bullets_list[bullet_index];
            nb_indents += entry.h_level - 1;
        }
        std::string indents(nb_indents, '\t');

        text_buffer->insert_at_cursor("\n" + indents + bullet_char + " ");

        auto mark = Gtk::TextMark::create();
        text_buffer->add_mark(mark, text_buffer->get_insert()->get_iter());
        text_buffer->insert_at_cursor(entry.text);

        auto mark_iter = mark->get_iter();
        std::string tag_name = _pCtMainWin->get_text_tag_name_exist_or_create(CtConst::TAG_LINK, entry.anchor_link);
        text_buffer->apply_tag_by_name(tag_name, mark_iter, text_buffer->get_insert()->get_iter());
        text_buffer->delete_mark(mark);

        _insert_toc_at_pos(text_buffer, entry.children);
    }
}

void find_toc_entries_and_children(std::list<TocEntry>& entries,
                                   CtActions& actions,
                                   CtMainWin& main_win,
                                   CtTreeIter& node,
                                   unsigned depth)
{
    main_win.get_tree_view().set_cursor_safe(node);
    TocEntry entry = find_toc_entries(actions, node, depth);
    entries.emplace_back(entry);

    CtTreeIter child = node.first_child();
    while (child) {
        find_toc_entries_and_children(entries, actions, main_win, child, depth + 1);
        ++child;
    }
}

void CtActions::toc_insert()
{
    if (!_is_there_selected_node_or_error()) return;
    if (!_node_sel_and_rich_text()) return;

    auto toc_type = CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, false, nullptr, nullptr, nullptr, nullptr);

    if (toc_type == CtExporting::NONE) return;

    std::list<TocEntry> entries;
    CtTreeIter curr_node = _pCtMainWin->curr_tree_iter();
    if (toc_type == CtExporting::CURRENT_NODE) {
        auto txt_buff = curr_node.get_node_text_buffer();
        _pCtMainWin->get_tree_view().set_cursor_safe(curr_node);

        TocEntry entry = find_toc_entries(*this, curr_node, 0);
        entries.emplace_back(std::move(entry));
    }
    else if (toc_type == CtExporting::CURRENT_NODE_AND_SUBNODES) {
        find_toc_entries_and_children(entries, *this, *_pCtMainWin, curr_node, 0);
    }
    else if (toc_type == CtExporting::ALL_TREE) {
        CtTreeStore& tree_store = _pCtMainWin->get_tree_store();
        CtTreeIter top_node = tree_store.get_ct_iter_first();
        CtTreeIter sib = top_node;
        while (sib) {
            find_toc_entries_and_children(entries, *this, *_pCtMainWin, sib, 0);
            ++sib;
        }
    }
    else {
        return;
    }

    _insert_toc_at_pos(curr_node.get_node_text_buffer(), entries);

    _pCtMainWin->get_tree_view().set_cursor_safe(curr_node);
}

// Insert Timestamp
void CtActions::timestamp_insert()
{
    if (not _is_curr_node_not_read_only_or_error()) return;

    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;

    time_t time = std::time(nullptr);
    Glib::ustring timestamp = str::time_format(_pCtConfig->timestampFormat, time);
    proof.text_view->get_buffer()->insert_at_cursor(timestamp);
}

void CtActions::special_char_insert()
{
    if (not _is_curr_node_not_read_only_or_error()) return;

    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;

    auto itemStore = CtChooseDialogListStore::create();
    unsigned pathSelectIdx{0};
    unsigned pathCurrIdx{0};
    const Glib::ustring lastSpecialChar = _pCtConfig->lastSpecialChar;
    for (gunichar ch : _pCtConfig->specialChars.item()) {
        const Glib::ustring specialChar{1, ch};
        itemStore->add_row("", "", specialChar);
        if (lastSpecialChar == specialChar) {
            pathSelectIdx = pathCurrIdx;
        }
        ++pathCurrIdx;
    }
    const Gtk::TreeIter treeIter = CtDialogs::choose_item_dialog(*_pCtMainWin,
                                                                 _("Special Characters"),
                                                                 itemStore,
                                                                 nullptr/*single_column_name*/,
                                                                 std::to_string(pathSelectIdx));
    if (treeIter) {
        const Glib::ustring specialChar = treeIter->get_value(itemStore->columns.desc);
        proof.text_view->get_buffer()->insert_at_cursor(specialChar);
        _pCtConfig->lastSpecialChar = specialChar;
    }
}

// Insert a Horizontal Line
void CtActions::horizontal_rule_insert()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    proof.text_view->get_buffer()->insert_at_cursor(_pCtConfig->hRule + CtConst::CHAR_NEWLINE);
}

// Lowers the Case of the Selected Text/the Underlying Word
void CtActions::text_selection_lower_case()
{
    _text_selection_change_case('l');
}

// Uppers the Case of the Selected Text/the Underlying Word
void CtActions::text_selection_upper_case()
{
    _text_selection_change_case('u');
}

// Toggles the Case of the Selected Text/the Underlying Word
void CtActions::text_selection_toggle_case()
{
    _text_selection_change_case('t');
}

void CtActions::toggle_ena_dis_spellcheck()
{
    _pCtConfig->enableSpellCheck = not _pCtConfig->enableSpellCheck;
    _validate_enable_spell_check();
    _pCtMainWin->get_text_view().set_spell_check(_pCtMainWin->curr_tree_iter().get_node_is_text());
}

void CtActions::_validate_enable_spell_check()
{
    if (_pCtConfig->enableSpellCheck and not gspell_language_get_available()) {
        _pCtConfig->enableSpellCheck = false;
        spdlog::debug("disabled spell check as no languages available");
    }
}

// Copy as Plain Text
void CtActions::cut_as_plain_text()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtClipboard::force_plain_text();
    auto proof = _get_text_view_n_buffer_codebox_proof();
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "cut-clipboard");
}

// Copy as Plain Text
void CtActions::copy_as_plain_text()
{
#if defined(HAVE_VTE)
    Gtk::Widget* pVte = _pCtMainWin->get_vte();
    if (pVte and pVte->has_focus()) {
        terminal_copy();
        return;
    }
#endif // HAVE_VTE
    CtClipboard::force_plain_text();
    auto proof = _get_text_view_n_buffer_codebox_proof();
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "copy-clipboard");
}

// Paste as Plain Text
void CtActions::paste_as_plain_text()
{
#if defined(HAVE_VTE)
    Gtk::Widget* pVte = _pCtMainWin->get_vte();
    if (pVte and pVte->has_focus()) {
        terminal_paste();
        return;
    }
#endif // HAVE_VTE
    auto proof = _get_text_view_n_buffer_codebox_proof();
    CtClipboard::force_plain_text();
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "paste-clipboard");
}

// Cut a Whole Row
void CtActions::text_row_cut()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    CtTextRange range = CtList{_pCtMainWin, proof.text_view->get_buffer()}.get_paragraph_iters();
    if (not range.iter_end.forward_char() and !range.iter_start.backward_char()) return;
    proof.text_view->get_buffer()->select_range(range.iter_start, range.iter_end);
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "cut-clipboard");
}

// Copy a Whole Row
void CtActions::text_row_copy()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;

    CtTextRange range = CtList{_pCtMainWin, proof.text_view->get_buffer()}.get_paragraph_iters();
    if (not range.iter_end.forward_char() and !range.iter_start.backward_char()) return;
    proof.text_view->get_buffer()->select_range(range.iter_start, range.iter_end);
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "copy-clipboard");
}

// Deletes the Whole Row
void CtActions::text_row_delete()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    CtTextRange range = CtList{_pCtMainWin, proof.text_view->get_buffer()}.get_paragraph_iters();
    if (not range.iter_end.forward_char() and !range.iter_start.backward_char()) return;
    proof.text_view->get_buffer()->erase(range.iter_start, range.iter_end);
    _pCtMainWin->get_state_machine().update_state();
}

// Duplicates the Whole Row or a Selection
void CtActions::text_row_selection_duplicate()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    auto text_buffer = proof.text_view->get_buffer();
    if (proof.text_view->get_buffer()->get_has_selection()) {
        Gtk::TextIter iter_start, iter_end;
        text_buffer->get_selection_bounds(iter_start, iter_end);
        int sel_start_offset = iter_start.get_offset();
        int sel_end_offset = iter_end.get_offset();
        if (proof.codebox or proof.table or proof.syntax_highl != CtConst::RICH_TEXT_ID) {
            Glib::ustring text_to_duplicate = text_buffer->get_text(iter_start, iter_end);
            if (text_to_duplicate.find(CtConst::CHAR_NEWLINE) != Glib::ustring::npos)
                text_to_duplicate = CtConst::CHAR_NEWLINE + text_to_duplicate;
            text_buffer->insert(iter_end, text_to_duplicate);
        }
        else{
            Glib::ustring rich_text = CtClipboard{_pCtMainWin}.rich_text_get_from_text_buffer_selection(
                _pCtMainWin->curr_tree_iter(),
                text_buffer,
                iter_start,
                iter_end);
            if (rich_text.find(CtConst::CHAR_NEWLINE) != Glib::ustring::npos) {
                text_buffer->insert(iter_end, CtConst::CHAR_NEWLINE);
                iter_end = proof.text_view->get_buffer()->get_iter_at_offset(sel_end_offset+1);
                text_buffer->move_mark(proof.text_view->get_buffer()->get_insert(), iter_end);
            }
            CtClipboard{_pCtMainWin}.from_xml_string_to_buffer(text_buffer, rich_text);
        }
        text_buffer->select_range(text_buffer->get_iter_at_offset(sel_start_offset),
                                  text_buffer->get_iter_at_offset(sel_end_offset));
    }
    else {
        int cursor_offset = text_buffer->get_iter_at_mark(text_buffer->get_insert()).get_offset();
        CtTextRange range = CtList{_pCtMainWin, proof.text_view->get_buffer()}.get_paragraph_iters();
        if (range.iter_start.get_offset() == range.iter_end.get_offset()) {
            Gtk::TextIter iter_start = text_buffer->get_iter_at_mark(text_buffer->get_insert());
            text_buffer->insert(iter_start, CtConst::CHAR_NEWLINE);
        }
        else {
            if (proof.codebox or proof.table or proof.syntax_highl != CtConst::RICH_TEXT_ID) {
                Glib::ustring text_to_duplicate = text_buffer->get_text(range.iter_start, range.iter_end);
                text_buffer->insert(range.iter_end, CtConst::CHAR_NEWLINE + text_to_duplicate);
            }
            else {
                Glib::ustring rich_text = CtClipboard{_pCtMainWin}.rich_text_get_from_text_buffer_selection(
                    _pCtMainWin->curr_tree_iter(),
                    text_buffer,
                    range.iter_start,
                    range.iter_end);
                int sel_end_offset = range.iter_end.get_offset();
                text_buffer->insert(range.iter_end, CtConst::CHAR_NEWLINE);
                range.iter_end = text_buffer->get_iter_at_offset(sel_end_offset+1);
                text_buffer->move_mark(text_buffer->get_insert(), range.iter_end);
                CtClipboard{_pCtMainWin}.from_xml_string_to_buffer(proof.text_view->get_buffer(), rich_text);
                text_buffer->place_cursor(text_buffer->get_iter_at_offset(cursor_offset));
            }
        }
    }
    _pCtMainWin->get_state_machine().update_state();
}

// Moves Up the Current Row/Selected Rows
void CtActions::text_row_up()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_view->get_buffer();
    CtTextRange range = CtList{_pCtMainWin, text_buffer}.get_paragraph_iters();
    range.iter_end.forward_char();
    bool missing_leading_newline = false;
    Gtk::TextIter destination_iter = range.iter_start;

    if (not destination_iter.backward_char()) return;
    if (not destination_iter.backward_char()) {
        missing_leading_newline = true;
    }
    else {
        while (destination_iter.get_char() != '\n') {
            if (not destination_iter.backward_char()) {
                missing_leading_newline = true;
                break;
            }
        }
    }
    if (not missing_leading_newline) destination_iter.forward_char();
    int destination_offset = destination_iter.get_offset();
    int start_offset = range.iter_start.get_offset();
    int end_offset = range.iter_end.get_offset();
    //#print "iter_start %s %s '%s'" % (start_offset, ord(iter_start.get_char()), iter_start.get_char())
    //#print "iter_end %s %s '%s'" % (end_offset, ord(iter_end.get_char()), iter_end.get_char())
    //#print "destination_iter %s %s '%s'" % (destination_offset, ord(destination_iter.get_char()), destination_iter.get_char())
    Glib::ustring text_to_move = text_buffer->get_text(range.iter_start, range.iter_end);
    int diff_offsets = end_offset - start_offset;
    if (proof.codebox or proof.table or proof.syntax_highl != CtConst::RICH_TEXT_ID) {
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (text_to_move.empty() or text_to_move[text_to_move.length()-1] != '\n') {
            diff_offsets += 1;
            text_to_move += CtConst::CHAR_NEWLINE;
        }
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        text_buffer->insert(destination_iter, text_to_move);
        proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
    }
    else {
        Glib::ustring rich_text = CtClipboard{_pCtMainWin}.rich_text_get_from_text_buffer_selection(
            _pCtMainWin->curr_tree_iter(),
            text_buffer,
            range.iter_start,
            range.iter_end,
            'n',
            true/*exclude_iter_sel_end*/);
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (destination_offset > 0) {
            // clear the newline from any tag
            Gtk::TextIter clr_start_iter = text_buffer->get_iter_at_offset(destination_offset-1);
            text_buffer->remove_all_tags(clr_start_iter, destination_iter);
        }
        bool append_newline = false;
        if (text_to_move.empty() or text_to_move[text_to_move.length()-1] != '\n') {
            diff_offsets += 1;
            append_newline = true;
        }
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        // trick of space to prevent subsequent text to take pasted text tag(s)
        text_buffer->insert_at_cursor(CtConst::CHAR_SPACE);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        // write moved line
        CtClipboard{_pCtMainWin}.from_xml_string_to_buffer(text_buffer, rich_text);
        if (append_newline)
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        // clear space trick
        Gtk::TextIter cursor_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        text_buffer->erase(cursor_iter, text_buffer->get_iter_at_offset(cursor_iter.get_offset()+1));
        // selection
        proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
    }
    _pCtMainWin->get_state_machine().update_state();
}

// Moves Down the Current Row/Selected Rows
void CtActions::text_row_down()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_view->get_buffer();
    CtTextRange range = CtList{_pCtMainWin, text_buffer}.get_paragraph_iters();
    if (not range.iter_end.forward_char()) return;
    int missing_leading_newline = false;
    Gtk::TextIter destination_iter = range.iter_end;
    while (destination_iter.get_char() != '\n') {
        if (not destination_iter.forward_char()) {
            missing_leading_newline = true;
            break;
        }
    }
    destination_iter.forward_char();
    int destination_offset = destination_iter.get_offset();
    int start_offset = range.iter_start.get_offset();
    int end_offset = range.iter_end.get_offset();
    //#print "iter_start %s %s '%s'" % (start_offset, ord(iter_start.get_char()), iter_start.get_char())
    //#print "iter_end %s %s '%s'" % (end_offset, ord(iter_end.get_char()), iter_end.get_char())
    //#print "destination_iter %s %s '%s'" % (destination_offset, ord(destination_iter.get_char()), destination_iter.get_char())
    Glib::ustring text_to_move = text_buffer->get_text(range.iter_start, range.iter_end);
    int diff_offsets = end_offset - start_offset;
    if (proof.codebox or proof.table or proof.syntax_highl != CtConst::RICH_TEXT_ID) {
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_offset -= diff_offsets;
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (text_to_move.empty() or text_to_move[text_to_move.length() - 1] != '\n') {
            diff_offsets += 1;
            text_to_move += CtConst::CHAR_NEWLINE;
        }
        if (missing_leading_newline) {
            diff_offsets += 1;
            text_to_move = CtConst::CHAR_NEWLINE + text_to_move;
        }
        text_buffer->insert(destination_iter, text_to_move);
        if (not missing_leading_newline)
            proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
        else
            proof.text_view->set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2);
    }
    else {
        Glib::ustring rich_text = CtClipboard{_pCtMainWin}.rich_text_get_from_text_buffer_selection(
            _pCtMainWin->curr_tree_iter(),
            text_buffer,
            range.iter_start,
            range.iter_end,
            'n',
            true/*exclude_iter_sel_end*/);
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_offset -= diff_offsets;
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (destination_offset > 0) {
            // clear the newline from any tag
            Gtk::TextIter clr_start_iter = text_buffer->get_iter_at_offset(destination_offset-1);
            text_buffer->remove_all_tags(clr_start_iter, destination_iter);
        }
        bool append_newline = false;
        if (text_to_move.empty() or text_to_move[text_to_move.length()-1] != '\n') {
            diff_offsets += 1;
            append_newline = true;
        }
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        if (missing_leading_newline) {
            diff_offsets += 1;
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        }
        // trick of space to prevent subsequent text to take pasted text tag(s)
        text_buffer->insert_at_cursor(CtConst::CHAR_SPACE);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        // write moved line
        CtClipboard{_pCtMainWin}.from_xml_string_to_buffer(text_buffer, rich_text);
        if (append_newline)
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        // clear space trick
        Gtk::TextIter cursor_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        text_buffer->erase(cursor_iter, text_buffer->get_iter_at_offset(cursor_iter.get_offset()+1));
        // selection
        if (not missing_leading_newline)
            proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
        else
            proof.text_view->set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2);
    }
    _pCtMainWin->get_state_machine().update_state();
}

void CtActions::replace_tabs_with_spaces()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_view->get_buffer();

    int replaced_tabs{0};
    Gtk::TextIter curr_iter = text_buffer->begin();
    const int tab_width = proof.text_view->get_tab_width();
    const Glib::ustring replaceStr = str::repeat(CtConst::CHAR_SPACE, tab_width);

    text_buffer->begin_user_action();
    while (curr_iter) {
        const gunichar curr_char = curr_iter.get_char();
        if ('\t' == curr_char) {
            int curr_offset = curr_iter.get_offset();
            text_buffer->erase(curr_iter, text_buffer->get_iter_at_offset(curr_offset+1));
            text_buffer->insert(text_buffer->get_iter_at_offset(curr_offset), replaceStr);
            curr_iter = text_buffer->get_iter_at_offset(curr_offset + tab_width);
            ++replaced_tabs;
        }
        else if (not curr_iter.forward_char()) {
            break;
        }
    }
    text_buffer->end_user_action();
    CtDialogs::info_dialog(std::to_string(replaced_tabs) + " " + _("Tabs Replaced"), *_pCtMainWin);
}

void CtActions::strip_trailing_spaces()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_view->get_buffer()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_view->get_buffer();

    int cleaned_lines{0};
    bool removed_something{true};
    while (removed_something) {
        removed_something = false;
        Gtk::TextIter curr_iter = text_buffer->begin();
        int curr_state = 0;
        int start_offset = 0;
        while (curr_iter) {
            gunichar curr_char = curr_iter.get_char();
            if (curr_state == 0) {
                if (curr_char == ' ' or curr_char == '\t') {
                    start_offset = curr_iter.get_offset();
                    curr_state = 1;
                }
            }
            else if (curr_state == 1) {
                if (curr_char == '\n') {
                    text_buffer->erase(text_buffer->get_iter_at_offset(start_offset), curr_iter);
                    removed_something = true;
                    cleaned_lines += 1;
                    break;
                }
                else if (curr_char != ' ' and curr_char != '\t') {
                    curr_state = 0;
                }
            }
            if (not curr_iter.forward_char()) {
                if (curr_state == 1) {
                    text_buffer->erase(text_buffer->get_iter_at_offset(start_offset), curr_iter);
                    cleaned_lines += 1;
                }
                break;
            }
        }
    }
    CtDialogs::info_dialog(std::to_string(cleaned_lines) + " " + _("Lines Stripped"), *_pCtMainWin);
}

// Insert/Edit LatexBox Dialog
void CtActions:: _latex_edit_dialog(const Glib::ustring& latex_text,
                                    Gtk::TextIter insert_iter,
                                    Gtk::TextIter* pIterBound)
{
    const Glib::ustring ret_latex_text = CtDialogs::latex_handle_dialog(_pCtMainWin, latex_text);
    if (ret_latex_text.empty()) return;
    Glib::ustring image_justification;
    if (pIterBound) { // only in case of modify
        image_justification = CtTextIterUtil::get_text_iter_alignment(insert_iter, _pCtMainWin);
        int image_offset = insert_iter.get_offset();
        _curr_buffer()->erase(insert_iter, *pIterBound);
        insert_iter = _curr_buffer()->get_iter_at_offset(image_offset);
    }
    image_insert_latex(insert_iter, ret_latex_text, image_justification);
}

void CtActions::image_insert_latex(Gtk::TextIter iter_insert,
                                   const Glib::ustring& latex_text,
                                   const Glib::ustring& justification)
{
    if (latex_text.empty()) return;
    const int charOffset = iter_insert.get_offset();
    CtAnchoredWidget* pAnchoredWidget = new CtImageLatex{_pCtMainWin, latex_text, charOffset, justification, CtImageEmbFile::get_next_unique_id()};
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);
    _pCtMainWin->get_tree_store().addAnchoredWidgets(_pCtMainWin->curr_tree_iter(),
                                                     {pAnchoredWidget},
                                                     &_pCtMainWin->get_text_view());
}

// Insert/Edit Image Dialog
void CtActions::_image_edit_dialog(Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
                                   Gtk::TextIter insert_iter,
                                   Gtk::TextIter* pIterBound)
{
    Glib::RefPtr<Gdk::Pixbuf> ret_pixbuf = CtDialogs::image_handle_dialog(*_pCtMainWin, rPixbuf);
    if (not ret_pixbuf) return;
    Glib::ustring image_justification;
    if (pIterBound) { // only in case of modify
        image_justification = CtTextIterUtil::get_text_iter_alignment(insert_iter, _pCtMainWin);
        int image_offset = insert_iter.get_offset();
        _curr_buffer()->erase(insert_iter, *pIterBound);
        insert_iter = _curr_buffer()->get_iter_at_offset(image_offset);
    }
    image_insert_png(insert_iter, ret_pixbuf, ""/*link*/, image_justification);
}

void CtActions::image_insert_png(Gtk::TextIter iter_insert,
                                 Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
                                 const Glib::ustring& link,
                                 const Glib::ustring& image_justification)
{
    if (not rPixbuf) return;
    const int charOffset = iter_insert.get_offset();
    CtAnchoredWidget* pAnchoredWidget = new CtImagePng{_pCtMainWin, rPixbuf, link, charOffset, image_justification};
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);
    _pCtMainWin->get_tree_store().addAnchoredWidgets(_pCtMainWin->curr_tree_iter(),
                                                     {pAnchoredWidget},
                                                     &_pCtMainWin->get_text_view());
}

void CtActions::image_insert_anchor(Gtk::TextIter iter_insert, const Glib::ustring &name, const Glib::ustring &image_justification)
{
    int charOffset = iter_insert.get_offset();
    CtAnchoredWidget* pAnchoredWidget = new CtImageAnchor{_pCtMainWin, name, charOffset, image_justification};
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);

    _pCtMainWin->get_tree_store().addAnchoredWidgets(_pCtMainWin->curr_tree_iter(),
                                                     {pAnchoredWidget},
                                                     &_pCtMainWin->get_text_view());
}

// Change the Case of the Selected Text/the Underlying Word"""
void CtActions::_text_selection_change_case(gchar change_type)
{
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    Glib::RefPtr<Gtk::TextBuffer> text_buffer = proof.text_view->get_buffer();
    if (not text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) {
        return;
    }
    if (not text_buffer->get_has_selection() and
        not _pCtMainWin->apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter()))
    {
        CtDialogs::warning_dialog(_("No Text is Selected."), *_pCtMainWin);
        return;
    }

    Gtk::TextIter iter_start, iter_end;
    text_buffer->get_selection_bounds(iter_start, iter_end);
    Glib::ustring text_to_change_case, rich_text;
    if (proof.codebox or proof.table or proof.syntax_highl != CtConst::RICH_TEXT_ID) {
        text_to_change_case = text_buffer->get_text(iter_start, iter_end);
        if (change_type == 'l')      text_to_change_case = text_to_change_case.lowercase();
        else if (change_type == 'u') text_to_change_case = text_to_change_case.uppercase();
        else if (change_type == 't') text_to_change_case = str::swapcase(text_to_change_case);
    }
    else {
        rich_text = CtClipboard{_pCtMainWin}.rich_text_get_from_text_buffer_selection(
            _pCtMainWin->curr_tree_iter(),
            text_buffer,
            iter_start,
            iter_end,
            change_type);
    }

    int start_offset = iter_start.get_offset();
    int end_offset = iter_end.get_offset();
    text_buffer->erase(iter_start, iter_end);
    Gtk::TextIter iter_insert = text_buffer->get_iter_at_offset(start_offset);
    if (proof.codebox or proof.table or proof.syntax_highl != CtConst::RICH_TEXT_ID) {
        text_buffer->insert(iter_insert, text_to_change_case);
    }
    else {
        text_buffer->move_mark(text_buffer->get_insert(), iter_insert);
        CtClipboard{_pCtMainWin}.from_xml_string_to_buffer(text_buffer, rich_text);
    }
    text_buffer->select_range(text_buffer->get_iter_at_offset(start_offset),
                              text_buffer->get_iter_at_offset(end_offset));
}
