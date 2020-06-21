/*
 * ct_image.cc
 *
 * Copyright 2009-2020
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

#include "ct_image.h"
#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_storage_sqlite.h"
#include "ct_logging.h"
#include "ct_storage_control.h"




CtImage::CtImage(CtMainWin* pCtMainWin,
                 const std::string& rawBlob,
                 const char* mimeType,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(pCtMainWin, charOffset, justification)
{
    Glib::RefPtr<Gdk::PixbufLoader> rPixbufLoader = Gdk::PixbufLoader::create(mimeType, true);
    rPixbufLoader->write(reinterpret_cast<const guint8*>(rawBlob.c_str()), rawBlob.size());
    rPixbufLoader->close();
    _rPixbuf = rPixbufLoader->get_pixbuf();

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}

CtImage::CtImage(CtMainWin* pCtMainWin,
                 const char* stockImage,
                 const int size,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(pCtMainWin, charOffset, justification)
{
    _rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(stockImage, size);

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}

CtImage::CtImage(CtMainWin* pCtMainWin,
                 Glib::RefPtr<Gdk::Pixbuf> pixBuf,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(pCtMainWin, charOffset, justification)
{
    _rPixbuf = pixBuf;

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}

CtImage::~CtImage()
{
}

void CtImage::save(const fs::path& file_name, const Glib::ustring& type)
{
    _rPixbuf->save(file_name.string(), type);
}


CtImagePng::CtImagePng(CtMainWin* pCtMainWin,
                       const std::string& rawBlob,
                       const Glib::ustring& link,
                       const int charOffset,
                       const std::string& justification)
 : CtImage(pCtMainWin, rawBlob, "image/png", charOffset, justification),
   _link(link)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImagePng::_on_button_press_event), false);
    // todo: DEPRECATED signal_visibility_notify_event().connect([this](){ this->queue_draw(); return false; });    // Problem of image colored frame disappearing
    update_label_widget();
}

CtImagePng::CtImagePng(CtMainWin* pCtMainWin,
                       Glib::RefPtr<Gdk::Pixbuf> pixBuf,
                       const Glib::ustring& link,
                       const int charOffset,
                       const std::string& justification)
 : CtImage(pCtMainWin, pixBuf, charOffset, justification),
   _link(link)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImagePng::_on_button_press_event), false);
    // todo: DEPRECATED signal_visibility_notify_event().connect([this](){ this->queue_draw(); return false; });    // Problem of image colored frame disappearing
    update_label_widget();
}

const std::string CtImagePng::get_raw_blob()
{
    g_autofree gchar* pBuffer{NULL};
    gsize buffer_size;
    _rPixbuf->save_to_buffer(pBuffer, buffer_size, "png");
    const std::string rawBlob = std::string(pBuffer, buffer_size);
    return rawBlob;
}

void CtImagePng::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* storage_cache)
{
    xmlpp::Element* p_image_node = p_node_parent->add_child("encoded_png");
    p_image_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_image_node->set_attribute("link", _link);
    std::string encodedBlob;
    if (!storage_cache || !storage_cache->get_cached_image(this, encodedBlob))
         encodedBlob = Glib::Base64::encode(get_raw_blob());
    p_image_node->add_child_text(encodedBlob);
}

bool CtImagePng::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* storage_cache)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else
    {
        std::string rawBlob;
        if (!storage_cache || !storage_cache->get_cached_image(this, rawBlob))
           rawBlob = get_raw_blob();
        const std::string link = _link;

        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, "", -1, SQLITE_STATIC); // anchor name
        sqlite3_bind_blob(p_stmt, 5, rawBlob.c_str(), rawBlob.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 6, "", -1, SQLITE_STATIC); // filename
        sqlite3_bind_text(p_stmt, 7, link.c_str(), link.size(), SQLITE_STATIC);
        sqlite3_bind_int64(p_stmt, 8, 0); // time
        if (sqlite3_step(p_stmt) != SQLITE_DONE)
        {
            spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtImagePng::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_ImagePng(this));
}

void CtImagePng::update_label_widget()
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

bool CtImagePng::_on_button_press_event(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_image_anchor = this;
    _pCtMainWin->get_ct_actions()->object_set_selection(this);
    if (event->button == 1 || event->button == 2)
    {
        if (event->type == GDK_2BUTTON_PRESS)
            _pCtMainWin->get_ct_actions()->image_edit();
        else if(!_link.empty())
            _pCtMainWin->get_ct_actions()->link_clicked(_link, event->button == 2);
    }
    else if (event->button == 3)
    {
        _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().find_action("img_link_dismiss")->signal_set_visible.emit(!_link.empty());
        _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Image)->popup(event->button, event->time);
    }
    return true; // do not propagate the event
}


CtImageAnchor::CtImageAnchor(CtMainWin* pCtMainWin,
                             const Glib::ustring& anchorName,
                             const int charOffset,
                             const std::string& justification)
 : CtImage(pCtMainWin, "ct_anchor", pCtMainWin->get_ct_config()->anchorSize, charOffset, justification),
   _anchorName(anchorName)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImageAnchor::_on_button_press_event), false);
    update_tooltip();
}

void CtImageAnchor::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache*)
{
    xmlpp::Element* p_image_node = p_node_parent->add_child("encoded_png");
    p_image_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_image_node->set_attribute("anchor", _anchorName);
}

bool CtImageAnchor::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache*)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
         spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else
    {
        const std::string anchor_name = _anchorName;
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, anchor_name.c_str(), anchor_name.size(), SQLITE_STATIC);
        sqlite3_bind_blob(p_stmt, 5, nullptr, 0, SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 6, "", -1, SQLITE_STATIC); // filename
        sqlite3_bind_text(p_stmt, 7, "", -1, SQLITE_STATIC); // link
        sqlite3_bind_int64(p_stmt, 8, 0); // time
        if (sqlite3_step(p_stmt) != SQLITE_DONE)
        {
             spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtImageAnchor::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_Anchor(this));
}

void CtImageAnchor::update_tooltip()
{
    set_tooltip_text(_anchorName);
}

// Catches mouse buttons clicks upon anchor images
bool CtImageAnchor::_on_button_press_event(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_anchor_anchor = this;
    _pCtMainWin->get_ct_actions()->object_set_selection(this);
    if (event->button == 3)
        _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Anchor)->popup(event->button, event->time);
    else if (event->type == GDK_2BUTTON_PRESS)
        _pCtMainWin->get_ct_actions()->anchor_edit();

    return true; // do not propagate the event
}

CtImageEmbFile::CtImageEmbFile(CtMainWin* pCtMainWin,
                               const fs::path& fileName,
                               const std::string& rawBlob,
                               const double& timeSeconds,
                               const int charOffset,
                               const std::string& justification)
 : CtImage(pCtMainWin, "ct_file_icon", pCtMainWin->get_ct_config()->embfileSize, charOffset, justification),
   _fileName(fileName),
   _rawBlob(rawBlob),
   _timeSeconds(timeSeconds)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImageEmbFile::_on_button_press_event), false);
    update_tooltip();
    update_label_widget();
}

void CtImageEmbFile::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache*)
{
    xmlpp::Element* p_image_node = p_node_parent->add_child("encoded_png");
    p_image_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_image_node->set_attribute("filename", _fileName.string());
    p_image_node->set_attribute("time", std::to_string(_timeSeconds));
    const std::string encodedBlob = Glib::Base64::encode(_rawBlob);
    p_image_node->add_child_text(encodedBlob);
}

bool CtImageEmbFile::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache*)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
         spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else
    {
        const std::string file_name = _fileName.string();
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, "", -1, SQLITE_STATIC); // anchor
        sqlite3_bind_blob(p_stmt, 5, _rawBlob.c_str(), _rawBlob.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 6, file_name.c_str(), file_name.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 7, "", -1, SQLITE_STATIC); // link
        sqlite3_bind_int64(p_stmt, 8, _timeSeconds);
        if (sqlite3_step(p_stmt) != SQLITE_DONE)
        {
             spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtImageEmbFile::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_EmbFile(this));
}

void CtImageEmbFile::update_label_widget()
{
    if (_pCtMainWin->get_ct_config()->embfileShowFileName)
    {
        _labelWidget.set_markup("<b><small>"+_fileName.string()+"</small></b>");
        _labelWidget.show();
        _frame.set_label_widget(_labelWidget);
    }
    else
    {
        _labelWidget.hide();
    }
}

void CtImageEmbFile::update_tooltip()
{
    char humanReadableSize[16];
    const size_t embfileBytes{_rawBlob.size()};
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
    const Glib::ustring strDateTime = dateTime.format(_pCtMainWin->get_ct_config()->timestampFormat);
    char buffTooltip[128];
    snprintf(buffTooltip, 128, "%s\n%s (%ld Bytes)\n%s", _fileName.c_str(), humanReadableSize, embfileBytes, strDateTime.c_str());
    set_tooltip_text(buffTooltip);
}

// Catches mouse buttons clicks upon files images
bool CtImageEmbFile::_on_button_press_event(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_file_anchor = this;
    _pCtMainWin->get_ct_actions()->object_set_selection(this);
    if (event->button == 3)
        _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::EmbFile)->popup(event->button, event->time);
    else if (event->type == GDK_2BUTTON_PRESS)
        _pCtMainWin->get_ct_actions()->embfile_open();

    return true; // do not propagate the event
}
