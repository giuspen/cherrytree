/*
 * ct_image.cc
 *
 * Copyright 2009-2022
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

#include "ct_main_win.h"
#include "ct_image.h"
#include "ct_actions.h"
#include "ct_storage_sqlite.h"
#include "ct_logging.h"
#include "ct_storage_control.h"

CtImage::CtImage(CtMainWin* pCtMainWin,
                 const std::string& rawBlob,
                 const char* mimeType,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget{pCtMainWin, charOffset, justification}
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
 : CtAnchoredWidget{pCtMainWin, charOffset, justification}
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
 : CtAnchoredWidget{pCtMainWin, charOffset, justification}
{
    _rPixbuf = pixBuf;

    _image.set(_rPixbuf);
    _frame.add(_image);
    show_all();
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
 : CtImage{pCtMainWin, rawBlob, "image/png", charOffset, justification}
 , _link{link}
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImagePng::_on_button_press_event), false);
    update_label_widget();
}

CtImagePng::CtImagePng(CtMainWin* pCtMainWin,
                       Glib::RefPtr<Gdk::Pixbuf> pixBuf,
                       const Glib::ustring& link,
                       const int charOffset,
                       const std::string& justification)
 : CtImage{pCtMainWin, pixBuf, charOffset, justification}
 , _link{link}
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImagePng::_on_button_press_event), false);
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
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK) {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else {
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
        if (sqlite3_step(p_stmt) != SQLITE_DONE) {
            spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtImagePng::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_ImagePng{this});
}

void CtImagePng::update_label_widget()
{
    if (!_link.empty()) {
        _labelWidget.set_markup("<b><small>▲</small></b>");
        _labelWidget.show();
        _frame.set_label_widget(_labelWidget);
    }
    else {
        _labelWidget.hide();
    }
}

bool CtImagePng::_on_button_press_event(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_image_anchor = this;
    _pCtMainWin->get_ct_actions()->object_set_selection(this);
    if (event->button == 1 || event->button == 2) {
        if (event->type == GDK_2BUTTON_PRESS)
            _pCtMainWin->get_ct_actions()->image_edit();
        else if(!_link.empty())
            _pCtMainWin->get_ct_actions()->link_clicked(_link, event->button == 2);
    }
    else if (event->button == 3) {
        _pCtMainWin->get_ct_menu().find_action("img_link_dismiss")->signal_set_visible.emit(!_link.empty());
        _pCtMainWin->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Image)->popup(event->button, event->time);
    }
    return true; // do not propagate the event
}

CtImageAnchor::CtImageAnchor(CtMainWin* pCtMainWin,
                             const Glib::ustring& anchorName,
                             const int charOffset,
                             const std::string& justification)
 : CtImage{pCtMainWin, "ct_anchor", pCtMainWin->get_ct_config()->anchorSize, charOffset, justification}
 , _anchorName{anchorName}
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
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK) {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else {
        const std::string anchor_name = _anchorName;
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, anchor_name.c_str(), anchor_name.size(), SQLITE_STATIC);
        sqlite3_bind_blob(p_stmt, 5, nullptr, 0, SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 6, "", -1, SQLITE_STATIC); // filename
        sqlite3_bind_text(p_stmt, 7, "", -1, SQLITE_STATIC); // link
        sqlite3_bind_int64(p_stmt, 8, 0); // time
        if (sqlite3_step(p_stmt) != SQLITE_DONE) {
            spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtImageAnchor::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_Anchor{this});
}

void CtImageAnchor::update_tooltip()
{
    set_tooltip_text(_anchorName);
}

bool CtImageAnchor::_on_button_press_event(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_anchor_anchor = this;
    _pCtMainWin->get_ct_actions()->object_set_selection(this);
    if (event->button == 3)
        _pCtMainWin->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Anchor)->popup(event->button, event->time);
    else if (event->type == GDK_2BUTTON_PRESS)
        _pCtMainWin->get_ct_actions()->anchor_edit();

    return true; // do not propagate the event
}

/*static*/const std::string CtImageLatex::LatexSpecialFilename{"__ct_special.tex"};
/*static*/const Glib::ustring CtImageLatex::LatexTextDefault{"\\documentclass{article}\n"
                                                             "\\pagestyle{empty}\n"
                                                             "\\usepackage{amsmath}\n"
                                                             "\\begin{document}\n"
                                                             "\\begin{align*}\n"
                                                             "f(x) &= x^2\\\\\n"
                                                             "g(x) &= \\frac{1}{x}\\\\\n"
                                                             "F(x) &= \\int^a_b \\frac{1}{3}x^3\n"
                                                             "\\end{align*}\n"
                                                             "\\end{document}"};
/*static*/bool CtImageLatex::_renderingBinariesTested{false};
/*static*/bool CtImageLatex::_renderingBinariesLatexOk{true};
/*static*/bool CtImageLatex::_renderingBinariesDviPngOk{true};

CtImageLatex::CtImageLatex(CtMainWin* pCtMainWin,
                           const Glib::ustring& latexText,
                           const int charOffset,
                           const std::string& justification,
                           const size_t uniqueId)
 : CtImage{pCtMainWin, _get_latex_image(pCtMainWin, latexText, uniqueId), charOffset, justification}
 , _latexText{latexText}
 , _uniqueId{uniqueId}
{
    signal_button_press_event().connect(sigc::mem_fun(*this, &CtImageLatex::_on_button_press_event), false);
    update_tooltip();
}

void CtImageLatex::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache*)
{
    xmlpp::Element* p_image_node = p_node_parent->add_child("encoded_png");
    p_image_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_image_node->set_attribute("filename", CtImageLatex::LatexSpecialFilename);
    p_image_node->add_child_text(_latexText);
}

bool CtImageLatex::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache*)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK) {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else {
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, "", -1, SQLITE_STATIC); // anchor
        sqlite3_bind_blob(p_stmt, 5, _latexText.c_str(), _latexText.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 6, CtImageLatex::LatexSpecialFilename.c_str(), CtImageLatex::LatexSpecialFilename.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 7, "", -1, SQLITE_STATIC); // link
        sqlite3_bind_int64(p_stmt, 8, 0); // time
        if (sqlite3_step(p_stmt) != SQLITE_DONE) {
            spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtImageLatex::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_Latex{this});
}

void CtImageLatex::update_tooltip()
{
    set_tooltip_text(_latexText);
}

#if defined(_WIN32)
#define CONSOLE_SILENCE_OUTPUT  " > nul"
#define CONSOLE_BIN_PREFIX      ""
#else // !_WIN32
#define CONSOLE_SILENCE_OUTPUT  " > /dev/null"
#if defined(_FLATPAK_BUILD)
#define CONSOLE_BIN_PREFIX      "cd /app/bin/.TinyTeX/bin/x86_64-linux && ./"
#else // !_FLATPAK_BUILD
#define CONSOLE_BIN_PREFIX      fs::get_latex_dvipng_console_bin_prefix()
#endif // !_FLATPAK_BUILD
#endif // !_WIN32
/*static*/Glib::RefPtr<Gdk::Pixbuf> CtImageLatex::_get_latex_image(CtMainWin* pCtMainWin, const Glib::ustring& latexText, const size_t uniqueId)
{
    if (not _renderingBinariesTested) {
        _renderingBinariesTested = true;
    }
    const fs::path filename = std::to_string(uniqueId) +
                              CtConst::CHAR_MINUS + std::to_string(getpid()) +
                              CtConst::CHAR_MINUS + CtImageLatex::LatexSpecialFilename;
    const fs::path tmp_filepath_tex = pCtMainWin->get_ct_tmp()->getHiddenFilePath(filename);
    Glib::file_set_contents(tmp_filepath_tex.string(), latexText);
    const fs::path tmp_dirpath = tmp_filepath_tex.parent_path();
    std::string cmd = fmt::sprintf("%slatex --interaction=batchmode -output-directory=%s %s" CONSOLE_SILENCE_OUTPUT,
                                   CONSOLE_BIN_PREFIX, tmp_dirpath.c_str(), tmp_filepath_tex.c_str());
#if defined(_WIN32)
    glong utf16text_len = 0;
    g_autofree gunichar2* utf16text = g_utf8_to_utf16(cmd.c_str(), (glong)Glib::ustring{cmd.c_str()}.bytes(), nullptr, &utf16text_len, nullptr);
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    bool success = CreateProcessW(NULL, (LPWSTR)utf16text, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (success) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    int retVal;
    if (not success) {
        spdlog::error("!! CreateProcessW({})", cmd);
#else
    int retVal = std::system(cmd.c_str());
    if (retVal != 0) {
        spdlog::error("system({}) returned {}", cmd, retVal);
#endif
        if (_renderingBinariesLatexOk) {
            _renderingBinariesLatexOk = false;
            if (_renderingBinariesDviPngOk and 0 != system(fmt::sprintf("%sdvipng --version" CONSOLE_SILENCE_OUTPUT, CONSOLE_BIN_PREFIX).c_str())) {
                _renderingBinariesDviPngOk = false;
            }
        }
    }
    else {
        std::string tmp_filepath_noext = tmp_filepath_tex.string();
        tmp_filepath_noext = tmp_filepath_noext.substr(0, tmp_filepath_noext.size() - 3);
        const fs::path tmp_filepath_dvi = tmp_filepath_noext + "dvi";
        const fs::path tmp_filepath_png = tmp_filepath_noext + "png";
        cmd = fmt::sprintf("%sdvipng -q -T tight -D %d %s -o %s" CONSOLE_SILENCE_OUTPUT,
                           CONSOLE_BIN_PREFIX, pCtMainWin->get_ct_config()->latexSizeDpi, tmp_filepath_dvi.c_str(), tmp_filepath_png.c_str());
        retVal = std::system(cmd.c_str());
        if (retVal != 0) {
            spdlog::error("system({}) returned {}", cmd, retVal);
            if (_renderingBinariesDviPngOk) {
                _renderingBinariesDviPngOk = false;
            }
        }
        else {
            Glib::RefPtr<Gdk::Pixbuf> rPixbuf;
            try {
                rPixbuf = Gdk::Pixbuf::create_from_file(tmp_filepath_png.c_str());
            }
            catch (Glib::Error& error) {
                spdlog::error("{} {}", __FUNCTION__, error.what());
            }
            if (rPixbuf) {
                // success
                return rPixbuf;
            }
        }
    }
    // fallback
    return pCtMainWin->get_icon_theme()->load_icon("ct_warning", 48);
}

/*static*/void CtImageLatex::ensureRenderingBinariesTested()
{
    if (_renderingBinariesTested) {
        return;
    }
    _renderingBinariesTested = true;
    _renderingBinariesLatexOk = 0 == system(fmt::sprintf("%slatex --version" CONSOLE_SILENCE_OUTPUT, CONSOLE_BIN_PREFIX).c_str());
    _renderingBinariesDviPngOk = 0 == system(fmt::sprintf("%sdvipng --version" CONSOLE_SILENCE_OUTPUT, CONSOLE_BIN_PREFIX).c_str());
}

/*static*/Glib::ustring CtImageLatex::getRenderingErrorMessage()
{
    if (not _renderingBinariesLatexOk and not _renderingBinariesDviPngOk) {
        return Glib::ustring{"<b><span foreground=\"red\">"} + _("Could not access the executables 'latex' and 'dvipng'") + Glib::ustring{"</span></b>\n"} +
               Glib::ustring{"* "} + _("For example, on Ubuntu the packages to install are:") +
               Glib::ustring{"\n  <tt>$sudo apt install texlive-latex-base</tt>\n  <tt>$sudo apt install dvipng</tt>\n"} +
               Glib::ustring{"* "} + _("For example, on Mac OS the packages to install are:") +
               Glib::ustring{"\n  <tt>$brew install --cask basictex</tt>\n  <tt>$sudo tlmgr update --self</tt>\n  <tt>$sudo tlmgr install dvipng</tt>\n"};
    }
    if (not _renderingBinariesLatexOk) {
        return Glib::ustring{"<b><span foreground=\"red\">"} + _("Could not access the executable 'latex'") + Glib::ustring{"</span></b>\n"} +
               Glib::ustring{"* "} + _("For example, on Ubuntu the packages to install are:") +
               Glib::ustring{"\n  <tt>$sudo apt install texlive-latex-base</tt>\n"} +
               Glib::ustring{"* "} + _("For example, on Mac OS the packages to install are:") +
               Glib::ustring{"\n  <tt>$brew install --cask basictex</tt>\n  <tt>$sudo tlmgr update --self</tt>\n  <tt>$sudo tlmgr install dvipng</tt>\n"};
    }
    if (not _renderingBinariesDviPngOk) {
        return Glib::ustring{"<b><span foreground=\"red\">"} + _("Could not access the executable 'dvipng'") + Glib::ustring{"</span></b>\n"} +
               Glib::ustring{"* "} + _("For example, on Ubuntu the packages to install are:") +
               Glib::ustring{"\n  <tt>$sudo apt install dvipng</tt>\n"} +
               Glib::ustring{"* "} + _("For example, on Mac OS the packages to install are:") +
               Glib::ustring{"\n  <tt>$brew install --cask basictex</tt>\n  <tt>$sudo tlmgr update --self</tt>\n  <tt>$sudo tlmgr install dvipng</tt>\n"};
    }
    return "";
}

bool CtImageLatex::_on_button_press_event(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_latex_anchor = this;
    _pCtMainWin->get_ct_actions()->object_set_selection(this);
    if (event->button == 3)
        _pCtMainWin->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::Latex)->popup(event->button, event->time);
    else if (event->type == GDK_2BUTTON_PRESS)
        _pCtMainWin->get_ct_actions()->latex_edit();

    return true; // do not propagate the event
}

/*static*/size_t CtImageEmbFile::get_next_unique_id()
{
    static size_t next_unique_id{1};
    return next_unique_id++;
}

CtImageEmbFile::CtImageEmbFile(CtMainWin* pCtMainWin,
                               const fs::path& fileName,
                               const std::string& rawBlob,
                               const time_t timeSeconds,
                               const int charOffset,
                               const std::string& justification,
                               const size_t uniqueId)
 : CtImage{pCtMainWin, _get_file_icon(pCtMainWin, fileName), charOffset, justification}
 , _fileName{fileName}
 , _rawBlob{rawBlob}
 , _timeSeconds{timeSeconds}
 , _uniqueId{uniqueId}
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
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_IMAGE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK) {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else {
        const std::string file_name = _fileName.string();
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, "", -1, SQLITE_STATIC); // anchor
        sqlite3_bind_blob(p_stmt, 5, _rawBlob.c_str(), _rawBlob.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 6, file_name.c_str(), file_name.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 7, "", -1, SQLITE_STATIC); // link
        sqlite3_bind_int64(p_stmt, 8, _timeSeconds);
        if (sqlite3_step(p_stmt) != SQLITE_DONE) {
            spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtImageEmbFile::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_EmbFile{this});
}

void CtImageEmbFile::update_label_widget()
{
    if (_pCtMainWin->get_ct_config()->embfileShowFileName) {
        _labelWidget.set_markup("<b><small>"+str::xml_escape(_fileName.string())+"</small></b>");
        _labelWidget.show();
        _frame.set_label_widget(_labelWidget);
    }
    else {
        _labelWidget.hide();
    }
}

void CtImageEmbFile::update_tooltip()
{
    char humanReadableSize[16];
    const size_t embfileBytes{_rawBlob.size()};
    const double embfileKbytes{static_cast<double>(embfileBytes)/1024};
    const double embfileMbytes{embfileKbytes/1024};
    if (embfileMbytes > 1) {
        snprintf(humanReadableSize, 16, "%.1f MB", embfileMbytes);
    }
    else {
        snprintf(humanReadableSize, 16, "%.1f KB", embfileKbytes);
    }
    const Glib::DateTime dateTime{Glib::DateTime::create_now_local(static_cast<gint64>(_timeSeconds))};
    const Glib::ustring strDateTime = dateTime.format(_pCtMainWin->get_ct_config()->timestampFormat);
    char buffTooltip[128];
    snprintf(buffTooltip, 128, "%s\n%s (%zu Bytes)\n%s", _fileName.c_str(), humanReadableSize, embfileBytes, strDateTime.c_str());
    set_tooltip_text(buffTooltip);
}

/*static*/Glib::RefPtr<Gdk::Pixbuf> CtImageEmbFile::_get_file_icon(CtMainWin* pCtMainWin, const fs::path& fileName)
{
    Glib::RefPtr<Gdk::Pixbuf> result;
#ifndef _WIN32
    g_autofree gchar* ctype = g_content_type_guess(fileName.c_str(), NULL, 0, NULL);
    if (ctype && !g_content_type_is_unknown(ctype)) {
        if (GIcon* icon = g_content_type_get_icon(ctype)) { // Glib::wrap will unref object
            Gtk::IconInfo info = pCtMainWin->get_icon_theme()->lookup_icon(Glib::wrap(icon), pCtMainWin->get_ct_config()->embfileIconSize, Gtk::ICON_LOOKUP_USE_BUILTIN);
            result = info.load_icon();
        }
    }
#else
    (void)fileName; // silence warning
#endif // _WIN32
    if (!result)
        result = pCtMainWin->get_icon_theme()->load_icon("ct_file_icon", pCtMainWin->get_ct_config()->embfileIconSize);
    return result;
}

// Catches mouse buttons clicks upon files images
bool CtImageEmbFile::_on_button_press_event(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_file_anchor = this;
    _pCtMainWin->get_ct_actions()->object_set_selection(this);
    if (event->button == 3)
        _pCtMainWin->get_ct_menu().get_popup_menu(CtMenu::POPUP_MENU_TYPE::EmbFile)->popup(event->button, event->time);
    else if (event->type == GDK_2BUTTON_PRESS)
        _pCtMainWin->get_ct_actions()->embfile_open();

    return true; // do not propagate the event
}
