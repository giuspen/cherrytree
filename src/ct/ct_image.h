/*
 * ct_image.h
 *
 * Copyright 2009-2025
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
    ~CtImage() override {}

    void apply_width_height(const int /*parentTextWidth*/) override {}
    void apply_syntax_highlighting(const bool /*forceReApply*/) override {}
    void set_modified_false() override {}

    void save(const fs::path& file_name, const Glib::ustring& type);
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf() const { return _rPixbuf; }

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
    ~CtImagePng() override {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache, const std::string& multifile_dir) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    CtAnchWidgType get_type() const override { return CtAnchWidgType::ImagePng; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    const std::string get_raw_blob();
    void update_label_widget();
    const Glib::ustring& get_link() const { return _link; }
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
                  const CtAnchorExpCollState expCollState,
                  const int charOffset,
                  const std::string& justification);
    ~CtImageAnchor() override {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache, const std::string& multifile_dir) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    CtAnchWidgType get_type() const override { return CtAnchWidgType::ImageAnchor; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    const Glib::ustring& get_anchor_name() const { return _anchorName; }
    CtAnchorExpCollState get_exp_coll_state() const { return _expCollState; }
    void toggle_exp_coll_state();

private:
    bool _on_button_press_event(GdkEventButton* event);
    void _set_exp_coll_state(const CtAnchorExpCollState expCollState);
    static const char* _get_stock_id_for_exp_coll_state(const CtAnchorExpCollState expCollState);

protected:
    Glib::ustring _anchorName;
    CtAnchorExpCollState _expCollState;
};

class CtImageLatex : public CtImage
{
public:
    CtImageLatex(CtMainWin* pCtMainWin,
                 const Glib::ustring& latexText,
                 const int charOffset,
                 const std::string& justification,
                 const size_t uniqueId);
    ~CtImageLatex() override {}

    static const std::string LatexSpecialFilename;
    static const Glib::ustring LatexTextDefault;
    static const int PrintZoom;

    static void ensureRenderingBinariesTested();
    static Glib::ustring getRenderingErrorMessage();

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache, const std::string& multifile_dir) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    CtAnchWidgType get_type() const override { return CtAnchWidgType::ImageLatex; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    const Glib::ustring& get_latex_text() { return _latexText; }
    size_t               get_unique_id() { return _uniqueId; }
    Glib::RefPtr<Gdk::Pixbuf> get_image_for_print() const {
        return _get_latex_image(_pCtMainWin, _latexText, _uniqueId, PrintZoom);
    }

    void update_tooltip();

private:
    static Glib::RefPtr<Gdk::Pixbuf> _get_latex_image(CtMainWin* pCtMainWin, const Glib::ustring& latexText, const size_t uniqueId, const int zoom = 1);

private:
    bool _on_button_press_event(GdkEventButton* event);

protected:
    static bool   _renderingBinariesTested;
    static bool   _renderingBinariesLatexOk;
    static bool   _renderingBinariesDviPngOk;
    Glib::ustring _latexText;
    const size_t  _uniqueId;
};

class CtImageEmbFile : public CtImage
{
public:
    CtImageEmbFile(CtMainWin* pCtMainWin,
                   const fs::path& fileName,
                   const std::string& rawBlob,
                   const time_t timeSeconds,
                   const int charOffset,
                   const std::string& justification,
                   const size_t uniqueId,
                   const fs::path& pathLastMultiFile);
    ~CtImageEmbFile() override {}

    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache, const std::string& multifile_dir) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    CtAnchWidgType get_type() const override { return CtAnchWidgType::ImageEmbFile; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    const fs::path&      get_file_name() const { return _fileName; }
    void                 set_file_name(const fs::path& path) { _fileName = path; }
    const std::string&   get_raw_blob() { return _rawBlob; }
    void                 set_raw_blob(const std::string& buffer) { _rawBlob = buffer; }
    time_t               get_time() { return _timeSeconds; }
    void                 set_time(const time_t time) { _timeSeconds = time; }
    size_t               get_unique_id() { return _uniqueId; }
    const fs::path&      get_pathLastMultiFile() { return _pathLastMultiFile; }

    static size_t        get_next_unique_id();

    void                 update_tooltip();
    void                 update_label_widget();

private:
    static Glib::RefPtr<Gdk::Pixbuf> _get_file_icon(CtMainWin* pCtMainWin, const fs::path& fileName);

private:
    bool _on_button_press_event(GdkEventButton* event);
    void _checkNonEmptyRawBlob(const char* multifile_dir);

protected:
    fs::path      _fileName;
    std::string   _rawBlob;      // raw data, not a string
    time_t        _timeSeconds;
    const size_t  _uniqueId;
    fs::path      _pathLastMultiFile;
};
