#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

from __future__ import absolute_import
from __future__ import print_function

# __author__ = 'Jakub Kolasa <jakub@arker.pl'>

from menustruct import UI_INFO_STRUCT_MENUBAR

# TODO: use direct building without middle xml format


def _menuBuild(item, tag="menu"):
    action, items = item
    # TODO: sanitize action
    s = "\n<{0} action=\"{1}\">".format(tag, action)

    for elem in items:
        s += "\n"
        islist = type(elem) is list
        if islist:
            s += _menuBuild(elem)
            continue

        if elem.startswith('-'):
            s += "<separator/>"
            continue
        s += '<menuitem action="{0}"/>'.format(elem)

    s += "\n</{0}>".format(tag)
    return s


def menuBuildMenuBars(ui_menubar_struct):
    s = ""
    for mb in ui_menubar_struct:
        # TODO: sanitize
        s += '<menubar name="{0}">'.format(mb[0])
        for m in mb[1]:
            s += "\n"
            s += _menuBuild(m)

        s += '</menubar>'

    return s


def menuBuildPopups(ui_popup_struct):
    s = ""
    for item in ui_popup_struct:
        s += _menuBuild(item, 'popup')

    return s


def menuBuildXML(ui_menubar_struct, ui_popup_struct):
    return "<ui>" + menuBuildMenuBars(ui_menubar_struct) + menuBuildPopups(ui_popup_struct) + "</ui>"


def menuBarInsert(where, what, after):
    placement = UI_INFO_STRUCT_MENUBAR
    for wh in where:
        for p in placement:
            if type(p) is not list:
                continue

            name = p[0]
            if name == wh:
                wh = None
                placement = p[1]
                break

        if wh:
            placement = None
            break

    if not placement or type(placement) is not list:
        print ("ERROR: Could not find menu placement for:", where)
        return

    if after:
        if after == "0":
            placement.insert(0, what)
        else:
            try:
                i = placement.index(after)
                placement.insert(i+1, what)
            except:
                placement.append(what)

