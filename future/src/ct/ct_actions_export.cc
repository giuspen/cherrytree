/*
 * ct_actions_export.cc
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

#include "ct_actions.h"

// Print Page Setup Operations
void CtActions::export_print_page_setup()
{
    _pCtMainWin->get_ct_print().run_page_setup_dialog(_pCtMainWin);
}

void CtActions::export_print()
{
    if (!_is_there_selected_node_or_error()) return;
    auto export_type = CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_last_include_node_name, &_last_new_node_page, nullptr);
    if (export_type == CtDialogs::CtProcessNode::NONE) return;


}

void CtActions::export_to_pdf()
{

}

void CtActions::export_to_html()
{

}

void CtActions::export_to_txt_multiple()
{

}

void CtActions::export_to_txt_single()
{

}

void CtActions::export_to_ctd()
{

}
