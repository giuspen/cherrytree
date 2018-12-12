/*
 * ct_image.cc
 * 
 * Copyright 2018 Giuseppe Penone <giuspen@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "ct_image.h"
#include "ct_app.h"

CtImage::CtImage(const Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
                 const int& charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(charOffset, justification)
{
    _rPixbuf = rPixbuf;
    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}


CtImagePng::CtImagePng(const Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
                       const int& charOffset,
                       const std::string& justification,
                       const Glib::ustring& link)
 : CtImage(rPixbuf, charOffset, justification),
   _link(link)
{
}


CtImageAnchor::CtImageAnchor(const int& charOffset,
                             const std::string& justification,
                             const Glib::ustring& anchorName)
 : CtImage(CtApp::R_icontheme->load_icon("anchor", CtApp::P_ctCfg->anchorSize), charOffset, justification),
   _anchorName(anchorName)
{
    updateTooltip();
}

void CtImageAnchor::updateTooltip()
{
    set_tooltip_text(_anchorName);
}


CtImageEmbFile::CtImageEmbFile(const int& charOffset,
                               const std::string& justification,
                               const std::string& rawFileStr,
                               const double& timeSeconds)
 : CtImage(CtApp::R_icontheme->load_icon("file_icon", CtApp::P_ctCfg->embfileSize), charOffset, justification),
   _rawFileStr(rawFileStr)
{
    updateTooltip();
}

void CtImageEmbFile::updateTooltip()
{
    
}
