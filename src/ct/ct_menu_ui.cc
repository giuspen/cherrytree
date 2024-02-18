/*
 * ct_menu_ui.cc
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

#include "ct_menu.h"
#include "ct_const.h"
#include "ct_config.h"
#include "ct_misc_utils.h"

std::vector<std::string> CtMenu::_get_ui_str_toolbars()
{
    auto generate_ui = [&](size_t id, const std::vector<std::string>& items){
        std::string str;
        for (const std::string& element: items) {
            if (element == CtConst::TAG_SEPARATOR) {
                str += "<child><object class='GtkSeparatorToolItem'/></child>";
            }
            else {
                const bool isOpenRecent{element == CtConst::CHAR_STAR};
                CtMenuAction const* pAction = isOpenRecent ? find_action("ct_open_file") : find_action(element);
                if (pAction) {
                    if (isOpenRecent) str += "<child><object class='GtkMenuToolButton' id='RecentDocs'>";
                    else str += "<child><object class='GtkToolButton'>";
                    str += "<property name='action-name'>win." + pAction->id + "</property>"; // 'win.' is a default action group in Window
                    str += "<property name='icon-name'>" + pAction->image + "</property>";
                    str += "<property name='label'>" + pAction->name + "</property>";
                    std::string kb_shortcut = pAction->get_shortcut(_pCtConfig);
                    std::string tooltip;
                    if (not _pCtConfig->toolbarTooltips) {
                        // keep empty
                    }
                    else if (kb_shortcut.empty()) {
                        tooltip = pAction->desc;
                    }
                    else {
                        if (kb_shortcut.find(CtMenu::KB_CONTROL) != std::string::npos) {
                            kb_shortcut = str::replace(kb_shortcut, CtMenu::KB_CONTROL, "Ctrl+");
                        }
                        if (kb_shortcut.find(CtMenu::KB_SHIFT) != std::string::npos) {
                            kb_shortcut = str::replace(kb_shortcut, CtMenu::KB_SHIFT, "Shift+");
                        }
                        if (kb_shortcut.find(CtMenu::KB_ALT) != std::string::npos) {
                            kb_shortcut = str::replace(kb_shortcut, CtMenu::KB_ALT, "Alt+");
                        }
                        if (kb_shortcut.find(CtMenu::KB_META) != std::string::npos) {
                            kb_shortcut = str::replace(kb_shortcut, CtMenu::KB_META, "Meta+");
                        }
                        tooltip = pAction->desc + " (" + str::xml_escape(kb_shortcut).c_str() + ")";
                    }
                    if (not tooltip.empty()) {
                        str += "<property name='tooltip-text'>" + tooltip + "</property>";
                    }
                    str += "<property name='visible'>True</property>";
                    str += "<property name='use_underline'>True</property>";
                    str += "</object></child>";
                }
            }
        }
        str = "<interface><object class='GtkToolbar' id='ToolBar" + std::to_string(id) + "'>"
                "<property name='visible'>True</property>"
                "<property name='can_focus'>False</property>"
                + str +
                "</object></interface>";
        return str;
    };

    std::vector<std::string> toolbarUIstr;
    std::vector<std::string> vecToolbarElements = str::split(_pCtConfig->toolbarUiList, ",");
    std::vector<std::string> toolbar_accumulator;
    for (const std::string& element : vecToolbarElements) {
        if (element != CtConst::TOOLBAR_SPLIT)
            toolbar_accumulator.push_back(element);
        else if (!toolbar_accumulator.empty()) {
            toolbarUIstr.push_back(generate_ui(toolbarUIstr.size(), toolbar_accumulator));
            toolbar_accumulator.clear();
        }
    }

    if (!toolbar_accumulator.empty()) {
        toolbarUIstr.push_back(generate_ui(toolbarUIstr.size(), toolbar_accumulator));
        toolbar_accumulator.clear();
    }

    return toolbarUIstr;
}

const char* CtMenu::_get_ui_str_menu()
{
    return R"MARKUP(
<menubar name='MenuBar'>
  <menu action='FileMenu'>
    <menuitem action='ct_new_inst'/>
    <menuitem action='ct_open_folder'/>
    <menuitem action='ct_open_file'/>
    <menu action='RecentDocsSubMenu'>
    </menu>
    <separator/>
    <menu action='ImportSubMenu'>
      <menuitem action='import_ct_folder'/>
      <menuitem action='import_ct_file'/>
      <menuitem action='import_indented_list'/>
      <menuitem action='import_txt_file'/>
      <menuitem action='import_txt_folder'/>
      <menuitem action='import_html_file'/>
      <menuitem action='import_html_folder'/>
      <menuitem action='import_md_file'/>
      <menuitem action='import_md_folder'/>
      <menuitem action='import_gnote'/>
      <menuitem action='import_keepnote'/>
      <menuitem action='import_leo'/>
      <menuitem action='import_mempad'/>
      <menuitem action='import_notecase'/>
      <menuitem action='import_rednotebook'/>
      <menuitem action='import_tomboy'/>
      <menuitem action='import_treepad'/>
      <menuitem action='import_zim'/>
    </menu>
    <menu action='ExportSubMenu'>
      <menuitem action='export_pdf'/>
      <menuitem action='export_html'/>
      <menuitem action='export_txt'/>
      <menuitem action='export_ct'/>
    </menu>
    <separator/>
    <menuitem action='ct_vacuum'/>
    <menuitem action='ct_save'/>
    <menuitem action='ct_save_as'/>
    <separator/>
    <menuitem action='print_page_setup'/>
    <menuitem action='do_print'/>
    <separator/>
    <menu action='PrefSubMenu'>
      <menuitem action='preferences_dlg'/>
      <menuitem action='pref_import'/>
      <menuitem action='pref_export'/>
      <menuitem action='open_cfg_folder'/>
    </menu>
    <menuitem action='tree_parse_info'/>
    <separator/>
    <menuitem action='quit_app'/>
    <menuitem action='exit_app'/>
  </menu>

  <menu action='EditMenu'>
    <menuitem action='act_undo'/>
    <menuitem action='act_redo'/>
    <separator/>
    <menuitem action='cut_plain'/>
    <menuitem action='copy_plain'/>
    <menuitem action='paste_plain'/>
    <separator/>
    <menu action='RowSubMenu'>
      <menuitem action='cut_row'/>
      <menuitem action='copy_row'/>
      <menuitem action='dup_row'/>
      <menuitem action='mv_up_row'/>
      <menuitem action='mv_down_row'/>
      <menuitem action='del_row'/>
    </menu>
    <separator/>
    <menu action='TableSubMenu'>
      <menuitem action='table_cut'/>
      <menuitem action='table_copy'/>
      <menuitem action='table_delete'/>
      <menuitem action='table_column_add'/>
      <menuitem action='table_column_delete'/>
      <menuitem action='table_column_left'/>
      <menuitem action='table_column_right'/>
      <menuitem action='table_column_increase_width'/>
      <menuitem action='table_column_decrease_width'/>
      <menuitem action='table_row_add'/>
      <menuitem action='table_row_cut'/>
      <menuitem action='table_row_copy'/>
      <menuitem action='table_row_paste'/>
      <menuitem action='table_row_delete'/>
      <menuitem action='table_row_up'/>
      <menuitem action='table_row_down'/>
      <menuitem action='table_rows_sort_descending'/>
      <menuitem action='table_rows_sort_ascending'/>
      <menuitem action='table_export'/>
      <menuitem action='table_edit_properties'/>
    </menu>
    <menu action='CodeBoxSubMenu'>
      <menuitem action='codebox_cut'/>
      <menuitem action='codebox_copy'/>
      <menuitem action='codebox_delete'/>
      <menuitem action='codebox_delete_keeping_text'/>
      <menuitem action='codebox_increase_width'/>
      <menuitem action='codebox_decrease_width'/>
      <menuitem action='codebox_increase_height'/>
      <menuitem action='codebox_decrease_height'/>
      <menuitem action='codebox_load_from_file'/>
      <menuitem action='codebox_save_to_file'/>
      <menuitem action='codebox_change_properties'/>
    </menu>
  </menu>

  <menu action='InsertMenu'>
    <menuitem action='handle_image'/>
    <menuitem action='handle_table'/>
    <menuitem action='handle_codebox'/>
    <menuitem action='handle_latex'/>
    <menuitem action='handle_embfile'/>
    <menuitem action='handle_link'/>
    <menuitem action='handle_anchor'/>
    <menuitem action='insert_toc'/>
    <menuitem action='insert_timestamp'/>
    <menuitem action='insert_special_char'/>
    <menuitem action='insert_horiz_rule'/>
    <menu action='ListSubMenu'>
     <menuitem action='handle_bull_list'/>
     <menuitem action='handle_num_list'/>
     <menuitem action='handle_todo_list'/>
    </menu>
  </menu>

  <menu action='FormatMenu'>
    <menuitem action='fmt_clone'/>
    <menuitem action='fmt_latest'/>
    <menuitem action='fmt_rm'/>
    <separator/>
    <menuitem action='fmt_color_fg'/>
    <menuitem action='fmt_color_bg'/>
    <separator/>
    <menu action='FontSubMenu'>
      <menuitem action='fmt_bold'/>
      <menuitem action='fmt_italic'/>
      <menuitem action='fmt_underline'/>
      <menuitem action='fmt_strikethrough'/>
      <menuitem action='fmt_monospace'/>
      <menuitem action='fmt_small'/>
      <menuitem action='fmt_superscript'/>
      <menuitem action='fmt_subscript'/>
    </menu>
    <menu action='ChangeCaseSubMenu'>
      <menuitem action='case_down'/>
      <menuitem action='case_up'/>
      <menuitem action='case_tggl'/>
    </menu>
    <menu action='HeadingSubMenu'>
      <menuitem action='fmt_h1'/>
      <menuitem action='fmt_h2'/>
      <menuitem action='fmt_h3'/>
      <menuitem action='fmt_h4'/>
      <menuitem action='fmt_h5'/>
      <menuitem action='fmt_h6'/>
    </menu>
    <separator/>
    <menuitem action='fmt_indent'/>
    <menuitem action='fmt_unindent'/>
    <separator/>
    <menu action='JustifySubMenu'>
      <menuitem action='fmt_justify_left'/>
      <menuitem action='fmt_justify_center'/>
      <menuitem action='fmt_justify_right'/>
      <menuitem action='fmt_justify_fill'/>
    </menu>
  </menu>

  <menu action='ToolsMenu'>
    <menuitem action='spellcheck_toggle'/>
    <separator/>
    <menuitem action='exec_code_los'/>
    <menuitem action='exec_code_all'/>
    <menuitem action='strip_trail_spaces'/>
    <menuitem action='repl_tabs_spaces'/>
    <separator/>
    <menuitem action='command_palette'/>
  </menu>

  <menu action='TreeMenu'>
    <menuitem action='go_node_next'/>
    <menuitem action='go_node_prev'/>
    <separator/>
    <menuitem action='tree_add_node'/>
    <menuitem action='tree_add_subnode'/>
    <menuitem action='tree_dup_node'/>
    <menuitem action='tree_dup_node_subnodes'/>
    <menuitem action='tree_shared_node'/>
    <menuitem action='tree_copy_node_subnodes'/>
    <menuitem action='tree_paste_node_subnodes'/>
    <menuitem action='tree_node_date_root'/>
    <menuitem action='tree_node_date_sel'/>
    <separator/>
    <menuitem action='tree_node_prop'/>
    <menuitem action='tree_node_toggle_ro'/>
    <menuitem action='tree_node_link'/>
    <menuitem action='child_nodes_inherit_syntax'/>
    <separator/>
    <menu action='BookmarksSubMenu'>
    </menu>
    <menuitem action='node_bookmark'/>
    <menuitem action='node_unbookmark'/>
    <separator/>
    <menuitem action='nodes_all_expand'/>
    <menuitem action='nodes_all_collapse'/>
    <separator/>
    <menu action='TreeMoveSubMenu'>
      <menuitem action='tree_node_up'/>
      <menuitem action='tree_node_down'/>
      <menuitem action='tree_node_left'/>
      <menuitem action='tree_node_right'/>
      <menuitem action='tree_node_new_father'/>
    </menu>
    <menu action='TreeSortSubMenu'>
      <menuitem action='tree_sibl_sort_asc'/>
      <menuitem action='tree_sibl_sort_desc'/>
      <separator/>
      <menuitem action='tree_all_sort_asc'/>
      <menuitem action='tree_all_sort_desc'/>
    </menu>
    <separator/>
    <menuitem action='tree_node_del'/>
  </menu>

  <menu action='SearchMenu'>
    <menuitem action='find_in_node'/>
    <menuitem action='find_in_allnodes'/>
    <menuitem action='find_in_node_names'/>
    <menuitem action='find_iter_fw'/>
    <menuitem action='find_iter_bw'/>
    <separator/>
    <menuitem action='replace_in_node'/>
    <menuitem action='replace_in_allnodes'/>
    <menuitem action='replace_iter_fw'/>
    <separator/>
    <menuitem action='tree_clear_exclude_from_search'/>
    <separator/>
    <menuitem action='toggle_show_allmatches_dlg'/>
  </menu>

  <menu action='ViewMenu'>
    <menuitem action='toggle_show_tree'/>
    <menuitem action='toggle_show_treelines'/>
    <menuitem action='toggle_show_menubar'/>
    <menuitem action='toggle_show_toolbar'/>
    <menuitem action='toggle_show_statusbar'/>
    <menuitem action='toggle_show_node_name_head'/>
    <menuitem action='toggle_show_vte'/>
    <separator/>
    <menuitem action='menubar_in_titlebar'/>
    <menuitem action='toggle_fullscreen'/>
    <menuitem action='toggle_always_on_top'/>
    <separator/>
    <menuitem action='toggle_focus_tree_text'/>
    <menuitem action='toggle_focus_vte_text'/>
    <separator/>
    <menuitem action='toolbar_icons_size_p'/>
    <menuitem action='toolbar_icons_size_m'/>
  </menu>

  <menu action='BookmarksMenu'>
  </menu>

  <menu action='HelpMenu'>
    <menuitem action='ct_check_newer'/>
    <separator/>
    <menuitem action='ct_homepage'/>
    <menuitem action='ct_github'/>
    <menuitem action='ct_issues'/>
    <menuitem action='ct_help'/>
    <separator/>
    <menuitem action='ct_about'/>
  </menu>
</menubar>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_text()
{
    return R"MARKUP(
<popup>
  <separator/>
  <menuitem action='cut_plain'/>
  <menuitem action='copy_plain'/>
  <menuitem action='paste_plain'/>
  <separator/>
  <menu action='RowSubMenu'>
    <menuitem action='cut_row'/>
    <menuitem action='copy_row'/>
    <menuitem action='dup_row'/>
    <menuitem action='mv_up_row'/>
    <menuitem action='mv_down_row'/>
    <menuitem action='del_row'/>
  </menu>
  <menu action='FormattingSubMenu'>
    <menuitem action='fmt_clone'/>
    <menuitem action='fmt_latest'/>
    <menuitem action='fmt_rm'/>
    <separator/>
    <separator/>
    <menuitem action='fmt_color_fg'/>
    <menuitem action='fmt_color_bg'/>
    <separator/>
    <menu action='FontSubMenu'>
      <menuitem action='fmt_bold'/>
      <menuitem action='fmt_italic'/>
      <menuitem action='fmt_underline'/>
      <menuitem action='fmt_strikethrough'/>
      <menuitem action='fmt_monospace'/>
      <menuitem action='fmt_small'/>
      <menuitem action='fmt_superscript'/>
      <menuitem action='fmt_subscript'/>
    </menu>
      <menu action='ChangeCaseSubMenu'>
      <menuitem action='case_down'/>
      <menuitem action='case_up'/>
      <menuitem action='case_tggl'/>
    </menu>
    <menu action='HeadingSubMenu'>
      <menuitem action='fmt_h1'/>
      <menuitem action='fmt_h2'/>
      <menuitem action='fmt_h3'/>
      <menuitem action='fmt_h4'/>
      <menuitem action='fmt_h5'/>
      <menuitem action='fmt_h6'/>
    </menu>
    <separator/>
    <menuitem action='fmt_indent'/>
    <menuitem action='fmt_unindent'/>
    <separator/>
    <menu action='JustifySubMenu'>
      <menuitem action='fmt_justify_left'/>
      <menuitem action='fmt_justify_center'/>
      <menuitem action='fmt_justify_right'/>
      <menuitem action='fmt_justify_fill'/>
    </menu>
  </menu>
  <menu action='InsertSubMenu'>
    <menuitem action='handle_image'/>
    <menuitem action='handle_table'/>
    <menuitem action='handle_codebox'/>
    <menuitem action='handle_latex'/>
    <menuitem action='handle_embfile'/>
    <menuitem action='handle_link'/>
    <menuitem action='handle_anchor'/>
    <menuitem action='insert_toc'/>
    <menuitem action='insert_timestamp'/>
    <menuitem action='insert_special_char'/>
    <menuitem action='insert_horiz_rule'/>
    <menu action='ListSubMenu'>
      <menuitem action='handle_bull_list'/>
      <menuitem action='handle_num_list'/>
      <menuitem action='handle_todo_list'/>
    </menu>
  </menu>
  <menuitem action='exec_code_los'/>
  <menuitem action='exec_code_all'/>
  <menuitem action='strip_trail_spaces'/>
  <separator/>
  <menu action='FindSubMenu'>
    <menuitem action='find_in_node'/>
    <menuitem action='find_in_allnodes'/>
    <menuitem action='find_in_node_names'/>
    <menuitem action='find_iter_fw'/>
    <menuitem action='find_iter_bw'/>
  </menu>
  <menu action='ReplaceSubMenu'>
    <menuitem action='replace_in_node'/>
    <menuitem action='replace_in_allnodes'/>
    <menuitem action='replace_iter_fw'/>
  </menu>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_code()
{
    return R"MARKUP(
<popup>
  <separator/>
  <menuitem action='cut_plain'/>
  <menuitem action='copy_plain'/>
  <separator/>
  <menuitem action='exec_code_los'/>
  <menuitem action='exec_code_all'/>
  <menuitem action='strip_trail_spaces'/>
  <menuitem action='repl_tabs_spaces'/>
  <menu action='InsertSubMenu'>
    <menuitem action='insert_timestamp'/>
    <menuitem action='insert_special_char'/>
    <menuitem action='insert_horiz_rule'/>
  </menu>
  <menu action='ChangeCaseSubMenu'>
    <menuitem action='case_down'/>
    <menuitem action='case_up'/>
    <menuitem action='case_tggl'/>
  </menu>
  <menu action='RowSubMenu'>
    <menuitem action='cut_row'/>
    <menuitem action='copy_row'/>
    <menuitem action='del_row'/>
    <menuitem action='dup_row'/>
    <menuitem action='mv_up_row'/>
    <menuitem action='mv_down_row'/>
  </menu>
  <separator/>
  <menu action='FindSubMenu'>
    <menuitem action='find_in_node'/>
    <menuitem action='find_in_allnodes'/>
    <menuitem action='find_in_node_names'/>
    <menuitem action='find_iter_fw'/>
    <menuitem action='find_iter_bw'/>
  </menu>
  <menu action='ReplaceSubMenu'>
    <menuitem action='replace_in_node'/>
    <menuitem action='replace_in_allnodes'/>
    <menuitem action='replace_iter_fw'/>
  </menu>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_image()
{
    return R"MARKUP(
<popup>
  <menuitem action='img_cut'/>
  <menuitem action='img_copy'/>
  <menuitem action='img_del'/>
  <separator/>
  <menuitem action='img_edit'/>
  <menuitem action='img_save'/>
  <separator/>
  <menuitem action='img_link_edit'/>
  <menuitem action='img_link_dismiss'/>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_latex()
{
    return R"MARKUP(
<popup>
  <menuitem action='tex_cut'/>
  <menuitem action='tex_copy'/>
  <menuitem action='tex_del'/>
  <separator/>
  <menuitem action='tex_edit'/>
  <menuitem action='tex_save'/>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_anchor()
{
    return R"MARKUP(
<popup>
  <menuitem action='anch_cut'/>
  <menuitem action='anch_copy'/>
  <menuitem action='anch_del'/>
  <separator/>
  <menuitem action='anch_link'/>
  <menuitem action='anch_edit'/>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_embfile()
{
    return R"MARKUP(
<popup>
  <menuitem action='emb_file_cut'/>
  <menuitem action='emb_file_copy'/>
  <menuitem action='emb_file_del'/>
  <separator/>
  <menuitem action='emb_file_open'/>
  <menuitem action='emb_file_save'/>
  <separator/>
  <menuitem action='emb_file_rename'/>
</popup>
    )MARKUP";
}

const char* CtMenu::_get_popup_menu_ui_str_terminal()
{
    return R"MARKUP(
<popup>
  <menuitem action='term_copy'/>
  <menuitem action='term_paste'/>
  <menuitem action='term_reset'/>
</popup>
    )MARKUP";
}
