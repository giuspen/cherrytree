/*
 * ct_image.h
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

#pragma once

#include <gtkmm.h>
#include "ct_const.h"
#include "ct_main_win.h"

class CtImage : public CtAnchoredWidget
{
public:
    CtImage(const Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
            const int& charOffset,
            const std::string& justification);
    virtual ~CtImage() {}
    virtual void applyWidthHeight(int parentTextWidth) {}
protected:
    Gtk::Image _image;
    Glib::RefPtr<Gdk::Pixbuf> _rPixbuf;
};

class CtImagePng : public CtImage
{
public:
    CtImagePng(const Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
               const int& charOffset,
               const std::string& justification,
               const Glib::ustring& link);
    virtual ~CtImagePng() {}
    void updateLabelWidget();
protected:
    Glib::ustring _link;
};

class CtImageAnchor : public CtImage
{
public:
    CtImageAnchor(const Glib::ustring& anchorName,
                  const int& charOffset,
                  const std::string& justification);
    virtual ~CtImageAnchor() {}
    void updateTooltip();
protected:
    Glib::ustring _anchorName;
};

class CtImageEmbFile : public CtImage
{
public:
    CtImageEmbFile(const Glib::ustring& fileName,
                   const std::string& rawFileStr,
                   const double& timeSeconds,
                   const int& charOffset,
                   const std::string& justification);
    virtual ~CtImageEmbFile() {}
    void updateTooltip();
    void updateLabelWidget();
protected:
    Glib::ustring _fileName;
    std::string _rawFileStr;
    double _timeSeconds;
};
