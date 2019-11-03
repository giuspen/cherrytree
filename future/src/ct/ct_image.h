/*
 * ct_image.h
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

#pragma once

#include <gtkmm.h>
#include "ct_const.h"
#include "ct_codebox.h"
#include "ct_widgets.h"

class CtImage : public CtAnchoredWidget
{
public:
    CtImage(const std::string& rawBlob,
            const char* mimeType,
            const int charOffset,
            const std::string& justification);
    CtImage(const char* stockImage,
            const int size,
            const int charOffset,
            const std::string& justification);
    CtImage(Glib::RefPtr<Gdk::Pixbuf> pixBuf,
            const int charOffset,
            const std::string& justification);
    virtual ~CtImage();

    void apply_width_height(const int /*parentTextWidth*/) override {}
    void set_modified_false() override {}

    void save(const Glib::ustring& file_name, const Glib::ustring& type);
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf() { return _rPixbuf; }

public:
    static Glib::RefPtr<Gdk::Pixbuf> get_icon(const std::string& name, int size);
    static Gtk::Image*               new_image_from_stock(const std::string& stockImage, Gtk::BuiltinIconSize size);

protected:
    Gtk::Image _image;
    Glib::RefPtr<Gdk::Pixbuf> _rPixbuf;
};

class CtImagePng : public CtImage
{
public:
    CtImagePng(const std::string& rawBlob,
               const Glib::ustring& link,
               const int charOffset,
               const std::string& justification);
    CtImagePng(Glib::RefPtr<Gdk::Pixbuf> pixBuf,
               const Glib::ustring& link,
               const int charOffset,
               const std::string& justification);
    virtual ~CtImagePng() {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment) override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::ImagePng; }

    const std::string get_raw_blob();
    void update_label_widget();
    const Glib::ustring& get_link() { return _link; }
    void set_link(const Glib::ustring& link) { _link = link; }

private:
    bool _on_button_press_event(GdkEventButton* event);

protected:
    Glib::ustring _link;
};

class CtImageAnchor : public CtImage
{
public:
    CtImageAnchor(const Glib::ustring& anchorName,
                  const int charOffset,
                  const std::string& justification);
    virtual ~CtImageAnchor() {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment) override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::ImageAnchor; }

    const Glib::ustring& get_anchor_name() { return _anchorName; }

    void update_tooltip();

private:
    bool _on_button_press_event(GdkEventButton* event);

protected:
    Glib::ustring _anchorName;
};

class CtImageEmbFile : public CtImage
{
public:
    CtImageEmbFile(const Glib::ustring& fileName,
                   const std::string& rawBlob,
                   const double& timeSeconds,
                   const int charOffset,
                   const std::string& justification);
    virtual ~CtImageEmbFile() {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment) override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::ImageEmbFile; }

    const Glib::ustring& get_file_name() { return _fileName; }
    const std::string&   get_raw_blob() { return _rawBlob; }
    void                 set_raw_blob(char* buffer, size_t size) { _rawBlob = std::string(buffer, size);}
    void                 set_time(time_t time) { _timeSeconds = time; }

    void update_tooltip();
    void update_label_widget();

private:
    bool _on_button_press_event(GdkEventButton* event);

protected:
    Glib::ustring _fileName;
    std::string   _rawBlob;      // raw data, not a string
    double        _timeSeconds;
};
