/*
 * ct_image.h
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

#pragma once

#include <gtkmm.h>
#include "ct_const.h"
#include "ct_codebox.h"
#include "ct_widgets.h"

class CtImage : public CtAnchoredWidget
{
public:
    CtImage(CtMainWin* pCtMainWin,
            const std::string& rawBlob,
            const char* mimeType,
            const int charOffset,
            const std::string& justification);
    CtImage(CtMainWin* pCtMainWin,
            const char* stockImage,
            const int size,
            const int charOffset,
            const std::string& justification);
    CtImage(CtMainWin* pCtMainWin,
            Glib::RefPtr<Gdk::Pixbuf> pixBuf,
            const int charOffset,
            const std::string& justification);
    virtual ~CtImage() override;

    void apply_width_height(const int /*parentTextWidth*/) override {}
    void apply_syntax_highlighting() override {}
    void set_modified_false() override {}

    void save(const fs::path& file_name, const Glib::ustring& type);
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf() { return _rPixbuf; }

protected:
    Gtk::Image _image;
    Glib::RefPtr<Gdk::Pixbuf> _rPixbuf;
};

class CtImagePng : public CtImage
{
public:
    CtImagePng(CtMainWin* pCtMainWin,
               const std::string& rawBlob,
               const Glib::ustring& link,
               const int charOffset,
               const std::string& justification);
    CtImagePng(CtMainWin* pCtMainWin,
               Glib::RefPtr<Gdk::Pixbuf> pixBuf,
               const Glib::ustring& link,
               const int charOffset,
               const std::string& justification);
    virtual ~CtImagePng() override {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::ImagePng; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

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
    CtImageAnchor(CtMainWin* pCtMainWin,
                  const Glib::ustring& anchorName,
                  const int charOffset,
                  const std::string& justification);
    virtual ~CtImageAnchor() override {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::ImageAnchor; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

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
    CtImageEmbFile(CtMainWin* pCtMainWin,
                   const fs::path& fileName,
                   const std::string& rawBlob,
                   const double& timeSeconds,
                   const int charOffset,
                   const std::string& justification);
    virtual ~CtImageEmbFile() override {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::ImageEmbFile; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    const fs::path& get_file_name() { return _fileName; }
    const std::string&   get_raw_blob() { return _rawBlob; }
    void                 set_raw_blob(const std::string& buffer) { _rawBlob = buffer; }
    double               get_time() { return _timeSeconds; }
    void                 set_time(time_t time) { _timeSeconds = time; }

    void update_tooltip();
    void update_label_widget();

private:
    bool _on_button_press_event(GdkEventButton* event);

protected:
    fs::path      _fileName;
    std::string   _rawBlob;      // raw data, not a string
    double        _timeSeconds;
};
