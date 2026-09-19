/*
 * ct_xml_compat.h
 *
 * Copyright 2009-2026
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

#pragma once

#include <libxml++/libxml++.h>

#if defined(CT_USE_LIBXMLPP5)
#define add_child add_child_element
#define get_child_text get_first_child_text
#define set_child_text set_first_child_text
#define CT_XML_CONST
#define CT_XML_TEXT(value) (value).raw()
#define CT_XML_STRING(value) value
#define CT_XML_ADD_CHILD(element, name) (element)->add_child_element(name)
#define CT_XML_GET_CHILD_TEXT(element) (element)->get_first_child_text()
#define CT_XML_SET_CHILD_TEXT(element, value) (element)->set_first_child_text(value)
#else
#define CT_XML_CONST const
#define CT_XML_TEXT(value) value
#define CT_XML_STRING(value) (value).raw()
#define CT_XML_ADD_CHILD(element, name) (element)->add_child(name)
#define CT_XML_GET_CHILD_TEXT(element) (element)->get_child_text()
#define CT_XML_SET_CHILD_TEXT(element, value) (element)->set_child_text(value)
#endif
