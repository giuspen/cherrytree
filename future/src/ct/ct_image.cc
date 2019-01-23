/*
 * ct_image.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_image.h"
#include "ct_app.h"

CtImage::CtImage(const std::string& rawBlob,
                 const char* mimeType,
                 const int& charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(charOffset, justification)
{
    Glib::RefPtr<Gdk::PixbufLoader> rPixbufLoader = Gdk::PixbufLoader::create(mimeType, true);
    rPixbufLoader->write(reinterpret_cast<const guint8*>(rawBlob.c_str()), rawBlob.size());
    rPixbufLoader->close();
    _rPixbuf = rPixbufLoader->get_pixbuf();

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}

CtImage::CtImage(const char* stockImage,
                 const int& size,
                 const int& charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(charOffset, justification)
{
    _rPixbuf = CtApp::R_icontheme->load_icon(stockImage, size);

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}

Gtk::Image* CtImage::new_image_from_stock(const std::string& stockImage, const int& size)
{
    Glib::RefPtr<Gdk::Pixbuf> pix_buf = CtApp::R_icontheme->load_icon(stockImage, size);
    Gtk::Image* image = Gtk::manage(new Gtk::Image());
    image->set(pix_buf);
    return image;
}


CtImagePng::CtImagePng(const std::string& rawBlob,
                       const Glib::ustring& link,
                       const int& charOffset,
                       const std::string& justification)
 : CtImage(rawBlob, "image/png", charOffset, justification),
   _link(link)
{
    updateLabelWidget();
}

void CtImagePng::updateLabelWidget()
{
    if (!_link.empty())
    {
        _labelWidget.set_markup("<b><small>▲</small></b>");
        _labelWidget.show();
        _frame.set_label_widget(_labelWidget);
    }
    else
    {
        _labelWidget.hide();
    }
}


CtImageAnchor::CtImageAnchor(const Glib::ustring& anchorName,
                             const int& charOffset,
                             const std::string& justification)
 : CtImage("anchor", CtApp::P_ctCfg->anchorSize, charOffset, justification),
   _anchorName(anchorName)
{
    updateTooltip();
}

void CtImageAnchor::updateTooltip()
{
    set_tooltip_text(_anchorName);
}


CtImageEmbFile::CtImageEmbFile(const Glib::ustring& fileName,
                               const std::string& rawBlob,
                               const double& timeSeconds,
                               const int& charOffset,
                               const std::string& justification)
 : CtImage("file_icon", CtApp::P_ctCfg->embfileSize, charOffset, justification),
   _fileName(fileName),
   _rawBlob(rawBlob),
   _timeSeconds(timeSeconds)
{
    updateTooltip();
    updateLabelWidget();
}

void  CtImageEmbFile::updateLabelWidget()
{
    if (CtApp::P_ctCfg->embfileShowFileName)
    {
        _labelWidget.set_markup("<b><small>"+_fileName+"</small></b>");
        _labelWidget.show();
        _frame.set_label_widget(_labelWidget);
    }
    else
    {
        _labelWidget.hide();
    }
}

void CtImageEmbFile::updateTooltip()
{
    char humanReadableSize[16];
    const long unsigned embfileBytes{_rawBlob.size()};
    const double embfileKbytes{static_cast<double>(embfileBytes)/1024};
    const double embfileMbytes{embfileKbytes/1024};
    if (embfileMbytes > 1)
    {
        snprintf(humanReadableSize, 16, "%.1f MB", embfileMbytes);
    }
    else
    {
        snprintf(humanReadableSize, 16, "%.1f KB", embfileKbytes);
    }
    const Glib::DateTime dateTime{Glib::DateTime::create_now_local(static_cast<gint64>(_timeSeconds))};
    const Glib::ustring strDateTime = dateTime.format(CtApp::P_ctCfg->timestampFormat);
    char buffTooltip[128];
    snprintf(buffTooltip, 128, "%s\n%s (%ld Bytes)\n%s", _fileName.c_str(), humanReadableSize, embfileBytes, strDateTime.c_str());
    set_tooltip_text(buffTooltip);
}
