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
#include "ct_doc_rw.h"
#include "ct_app.h"

CtImage::CtImage(const std::string& rawBlob,
                 const char* mimeType,
                 const int charOffset,
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
                 const int size,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(charOffset, justification)
{
    _rPixbuf = CtApp::R_icontheme->load_icon(stockImage, size);

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}

CtImage::CtImage(Glib::RefPtr<Gdk::Pixbuf> pixBuf,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(charOffset, justification)
{
    _rPixbuf = pixBuf;

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
}

CtImage::~CtImage()
{

}

void CtImage::save(const Glib::ustring& file_name, const Glib::ustring& type)
{
    _rPixbuf->save(file_name, type);
}

Glib::RefPtr<Gdk::Pixbuf> CtImage::get_icon(const std::string& name, int size)
{
    if (CtApp::R_icontheme->has_icon(name))
        return CtApp::R_icontheme->load_icon(name, size);
    return Glib::RefPtr<Gdk::Pixbuf>();
}

Gtk::Image* CtImage::new_image_from_stock(const std::string& stockImage, Gtk::BuiltinIconSize size)
{
    Gtk::Image* image = Gtk::manage(new Gtk::Image());
    image->set_from_icon_name(stockImage, size);
    return image;
}


CtImagePng::CtImagePng(const std::string& rawBlob,
                       const Glib::ustring& link,
                       const int charOffset,
                       const std::string& justification)
 : CtImage(rawBlob, "image/png", charOffset, justification),
   _link(link)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImagePng::_onButtonPressEvent), false);
    // todo: DEPRECATED signal_visibility_notify_event().connect([this](){ this->queue_draw(); return false; });    // Problem of image colored frame disappearing
    updateLabelWidget();
}

CtImagePng::CtImagePng(Glib::RefPtr<Gdk::Pixbuf> pixBuf,
                       const Glib::ustring& link,
                       const int charOffset,
                       const std::string& justification)
 : CtImage(pixBuf, charOffset, justification),
   _link(link)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImagePng::_onButtonPressEvent), false);
    // todo: DEPRECATED signal_visibility_notify_event().connect([this](){ this->queue_draw(); return false; });    // Problem of image colored frame disappearing
    updateLabelWidget();
}

const std::string CtImagePng::get_raw_blob()
{
    gchar* pBuffer{nullptr};
    gsize  buffer_size;
    _rPixbuf->save_to_buffer(pBuffer, buffer_size, "png");
    const std::string rawBlob = std::string(pBuffer, buffer_size);
    g_free(pBuffer);
    return rawBlob;
}

void CtImagePng::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment)
{
    xmlpp::Element* p_image_node = p_node_parent->add_child("encoded_png");
    p_image_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_image_node->set_attribute("link", _link);
    const std::string encodedBlob = Glib::Base64::encode(get_raw_blob());
    p_image_node->add_child_text(encodedBlob);
}

bool CtImagePng::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtSQLite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(pDb) << std::endl;
        retVal = false;
    }
    else
    {
        const std::string rawBlob = get_raw_blob();
        const std::string link = Glib::locale_from_utf8(_link);
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
            std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(pDb) << std::endl;
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
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

bool CtImagePng::_onButtonPressEvent(GdkEventButton* event)
{
    CtApp::P_ctActions->curr_image_anchor = this;
    CtApp::P_ctActions->object_set_selection(this);
    if (event->button == 1 || event->button == 2)
    {
        if (event->type == GDK_2BUTTON_PRESS)
            CtApp::P_ctActions->image_edit();
        else if(!_link.empty())
            CtApp::P_ctActions->link_clicked(_link, event->button == 2);
    }
    else if (event->button == 3)
    {
        CtApp::P_ctActions->getCtMainWin()->get_ct_menu().find_action("img_link_dismiss")->signal_set_visible.emit(!_link.empty());
        CtApp::P_ctActions->getCtMainWin()->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Image)->popup(event->button, event->time);
    }
    return true; // do not propagate the event
}


CtImageAnchor::CtImageAnchor(const Glib::ustring& anchorName,
                             const int charOffset,
                             const std::string& justification)
 : CtImage("anchor", CtApp::P_ctCfg->anchorSize, charOffset, justification),
   _anchorName(anchorName)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImageAnchor::_onButtonPressEvent), false);
    updateTooltip();
}

void CtImageAnchor::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment)
{
    xmlpp::Element* p_image_node = p_node_parent->add_child("encoded_png");
    p_image_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_image_node->set_attribute("anchor", _anchorName);
}

bool CtImageAnchor::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtSQLite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(pDb) << std::endl;
        retVal = false;
    }
    else
    {
        const std::string anchor_name = Glib::locale_from_utf8(_anchorName);
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
            std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(pDb) << std::endl;
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

void CtImageAnchor::updateTooltip()
{
    set_tooltip_text(_anchorName);
}

// Catches mouse buttons clicks upon anchor images
bool CtImageAnchor::_onButtonPressEvent(GdkEventButton* event)
{
    CtApp::P_ctActions->curr_anchor_anchor = this;
    CtApp::P_ctActions->object_set_selection(this);
    if (event->button == 3)
        CtApp::P_ctActions->getCtMainWin()->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Anchor)->popup(event->button, event->time);
    else if (event->type == GDK_2BUTTON_PRESS)
        CtApp::P_ctActions->anchor_edit();

    return true; // do not propagate the event
}

CtImageEmbFile::CtImageEmbFile(const Glib::ustring& fileName,
                               const std::string& rawBlob,
                               const double& timeSeconds,
                               const int charOffset,
                               const std::string& justification)
 : CtImage("file_icon", CtApp::P_ctCfg->embfileSize, charOffset, justification),
   _fileName(fileName),
   _rawBlob(rawBlob),
   _timeSeconds(timeSeconds)
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImageEmbFile::_onButtonPressEvent), false);
    updateTooltip();
    updateLabelWidget();
}

void CtImageEmbFile::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment)
{
    xmlpp::Element* p_image_node = p_node_parent->add_child("encoded_png");
    p_image_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_image_node->set_attribute("filename", _fileName);
    p_image_node->set_attribute("time", std::to_string(_timeSeconds));
    const std::string encodedBlob = Glib::Base64::encode(_rawBlob);
    p_image_node->add_child_text(encodedBlob);
}

bool CtImageEmbFile::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtSQLite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(pDb) << std::endl;
        retVal = false;
    }
    else
    {
        const std::string file_name = Glib::locale_from_utf8(_fileName);
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
            std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(pDb) << std::endl;
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

void CtImageEmbFile::updateLabelWidget()
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

// Catches mouse buttons clicks upon files images
bool CtImageEmbFile::_onButtonPressEvent(GdkEventButton* event)
{
    CtApp::P_ctActions->curr_file_anchor = this;
    CtApp::P_ctActions->object_set_selection(this);
    if (event->button == 3)
        CtApp::P_ctActions->getCtMainWin()->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::EmbFile)->popup(event->button, event->time);
    else if (event->type == GDK_2BUTTON_PRESS)
        CtApp::P_ctActions->embfile_open();

    return true; // do not propagate the event
}
