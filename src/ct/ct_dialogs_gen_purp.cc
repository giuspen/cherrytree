/*
 * ct_dialogs_gen_purp.cc
 *
 * Copyright 2009-2021
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

#include "ct_dialogs.h"
#include "ct_main_win.h"

CtDialogTextEntry::CtDialogTextEntry(const Glib::ustring& title,
                                     const bool forPassword,
                                     Gtk::Window* pParentWin)
{
    set_title(title);
    set_transient_for(*pParentWin);
    set_modal();

    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default_response(Gtk::RESPONSE_OK);

    _entry.set_icon_from_stock(Gtk::Stock::CLEAR, Gtk::ENTRY_ICON_SECONDARY);
    _entry.set_size_request(350, -1);
    if (forPassword) {
        _entry.set_visibility(false);
    }
    get_vbox()->pack_start(_entry, true, true, 0);

    // Special signals which correspond to the underlying X-Windows events are suffixed by _event.
    // By default, your signal handlers are called after any previously-connected signal handlers. However, this can be a problem with the X Event signals. For instance, the existing signal handlers, or the default signal handler, might return true to stop other signal handlers from being called. To specify that your signal handler should be called before the other signal handlers, so that it will always be called, you can specify false for the optional after parameter
    _entry.signal_key_press_event().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_on_entry_key_press_event), false/*call me before other*/);

    _entry.signal_icon_press().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_on_entry_icon_press));

    get_vbox()->show_all();
}

bool CtDialogTextEntry::_on_entry_key_press_event(GdkEventKey* pEventKey)
{
    if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
        Gtk::Button* pButton = static_cast<Gtk::Button*>(get_widget_for_response(Gtk::RESPONSE_OK));
        pButton->grab_focus();
        pButton->clicked();
        return true;
    }
    if (GDK_KEY_Escape == pEventKey->keyval) {
        Gtk::Button* pButton = static_cast<Gtk::Button*>(get_widget_for_response(Gtk::RESPONSE_CANCEL));
        pButton->grab_focus();
        pButton->clicked();
        return true;
    }
    return false;
}

Gtk::TreeIter CtDialogs::choose_item_dialog(Gtk::Window& parent,
                                            const Glib::ustring& title,
                                            Glib::RefPtr<CtChooseDialogListStore> rModel,
                                            const gchar* single_column_name/*= nullptr*/,
                                            const std::string& pathToSelect/*= "0"*/)
{
    Gtk::Dialog dialog{title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 300);
    auto pScrolledwindow = Gtk::manage(new Gtk::ScrolledWindow{});
    pScrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    auto pElementsTreeview = Gtk::manage(new Gtk::TreeView{rModel});
    pElementsTreeview->set_headers_visible(false);
    const auto treePathToSelect = Gtk::TreePath{pathToSelect};
    pElementsTreeview->get_selection()->select(treePathToSelect);
    pElementsTreeview->signal_row_activated().connect([&](const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) {
        if (Gtk::TreeIter iter = pElementsTreeview->get_selection()->get_selected()) {
            static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT))->clicked();
        }
    });

    Gtk::CellRendererPixbuf pixbuf_renderer;
    if (nullptr == single_column_name) {
        pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
        int col_num = pElementsTreeview->append_column("", pixbuf_renderer) - 1;
        pElementsTreeview->get_column(col_num)->add_attribute(pixbuf_renderer, "icon-name", rModel->columns.stock_id);
        pElementsTreeview->append_column("", rModel->columns.desc);
        pElementsTreeview->set_search_column(2);
    }
    else {
        pElementsTreeview->append_column(single_column_name, rModel->columns.desc);
    }
    pScrolledwindow->add(*pElementsTreeview);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(*pScrolledwindow);
    pContentArea->show_all();
    pElementsTreeview->grab_focus();
    pElementsTreeview->scroll_to_row(treePathToSelect, 0.5);

    return Gtk::RESPONSE_ACCEPT == dialog.run() ? pElementsTreeview->get_selection()->get_selected() : Gtk::TreeIter{};
}

Glib::ustring CtDialogs::img_n_entry_dialog(Gtk::Window& parent,
                                            const Glib::ustring& title,
                                            const Glib::ustring& entry_content,
                                            const char* img_stock)
{
    Gtk::Dialog dialog{title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);
    Gtk::Image image;
    image.set_from_icon_name(img_stock, Gtk::ICON_SIZE_BUTTON);
    Gtk::Entry entry;
    entry.set_text(entry_content);
    Gtk::HBox hbox;
    hbox.pack_start(image, false, false);
    hbox.pack_start(entry);
    hbox.set_spacing(5);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(hbox);
    pContentArea->show_all();
    entry.grab_focus();
    entry.signal_activate().connect([&](){
        if (!entry.get_text().empty()) {
            dialog.response(Gtk::RESPONSE_ACCEPT);
        }
    });

    return Gtk::RESPONSE_ACCEPT == dialog.run() ? str::trim(entry.get_text()) : "";
}

std::time_t CtDialogs::date_select_dialog(Gtk::Window& parent,
                                          const Glib::ustring& title,
                                          const std::time_t& curr_time)
{
    Gtk::Dialog dialog{title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);

    std::tm struct_time = *std::localtime(&curr_time);

    Gtk::Box* pContentArea = dialog.get_content_area();
    Gtk::Calendar calendar;
    calendar.select_month((guint)(struct_time.tm_mon), (guint)(struct_time.tm_year + 1900)); // month 0-11
    calendar.select_day((guint)struct_time.tm_mday); // day 1-31
    Glib::RefPtr<Gtk::Adjustment> rAdj_h = Gtk::Adjustment::create(struct_time.tm_hour, 0, 23, 1);
    Gtk::SpinButton spinbutton_h{rAdj_h};
    spinbutton_h.set_value(struct_time.tm_hour);
    Glib::RefPtr<Gtk::Adjustment> rAdj_m = Gtk::Adjustment::create(struct_time.tm_min, 0, 59, 1);
    Gtk::SpinButton spinbutton_m{rAdj_m};
    spinbutton_m.set_value(struct_time.tm_min);
    Gtk::HBox hbox;
    hbox.pack_start(spinbutton_h);
    hbox.pack_start(spinbutton_m);
    pContentArea->pack_start(calendar);
    pContentArea->pack_start(hbox);

    dialog.signal_button_press_event().connect([&dialog](GdkEventButton* event)->bool{
        if ( (event->button == 1) &&
             (event->type == GDK_2BUTTON_PRESS) )
        {
            return false;
        }
        dialog.response(Gtk::RESPONSE_ACCEPT);
        return true;
    });
    pContentArea->show_all();

    if (dialog.run() != Gtk::RESPONSE_ACCEPT) {
        return 0;
    }

    guint new_year, new_month, new_day;
    calendar.get_date(new_year, new_month, new_day);

    std::tm tmtime = {};
    tmtime.tm_year = (int)new_year - 1900;
    tmtime.tm_mon = (int)(new_month);
    tmtime.tm_mday = (int)new_day;
    tmtime.tm_hour = spinbutton_h.get_value_as_int();
    tmtime.tm_min = spinbutton_m.get_value_as_int();

    std::time_t new_time = std::mktime(&tmtime);
    return new_time;
}

CtDialogs::CtPickDlgState CtDialogs::color_pick_dialog(CtMainWin* pCtMainWin,
                                                       const Glib::ustring& title,
                                                       Gdk::RGBA& ret_color,
                                                       bool allow_remove_color)
{
    Gtk::ColorChooserDialog dialog{title};
    dialog.set_transient_for(*pCtMainWin);
    dialog.set_modal(true);
    dialog.property_destroy_with_parent() = true;
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    if (allow_remove_color) {
        dialog.add_button(_("Remove Color"), Gtk::RESPONSE_NONE);
    }
    dialog.set_rgba(ret_color);

    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_OK));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    const int response = dialog.run();

    if (Gtk::RESPONSE_NONE == response) {
        return CtPickDlgState::REMOVE_COLOR;
    }
    if (Gtk::RESPONSE_OK != response) {
        return CtPickDlgState::CANCEL;
    }
    ret_color = dialog.get_rgba();
    return CtPickDlgState::SELECTED;
}

// Returns True if the user presses OK
bool CtDialogs::question_dialog(const Glib::ustring& message,
                                Gtk::Window& parent)
{
    Gtk::MessageDialog dialog{parent,
                              message,
                              true/*use_markup*/,
                              Gtk::MESSAGE_QUESTION,
                              Gtk::BUTTONS_YES_NO,
                              true/*modal*/};
    dialog.set_title(_("Question"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_YES))->grab_focus();
    return (Gtk::RESPONSE_YES == dialog.run());
}

void CtDialogs::info_dialog(const Glib::ustring& message,
                            Gtk::Window& parent)
{
    Gtk::MessageDialog dialog{parent,
                              message,
                              true/*use_markup*/,
                              Gtk::MESSAGE_INFO,
                              Gtk::BUTTONS_OK,
                              true/*modal*/};
    dialog.set_title(_("Info"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

void CtDialogs::warning_dialog(const Glib::ustring& message,
                               Gtk::Window& parent)
{
    Gtk::MessageDialog dialog{parent,
                              message,
                              true/*use_markup*/,
                              Gtk::MESSAGE_WARNING,
                              Gtk::BUTTONS_OK,
                              true/*modal*/};
    dialog.set_title(_("Warning"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

void CtDialogs::error_dialog(const Glib::ustring& message,
                             Gtk::Window& parent)
{
    Gtk::MessageDialog dialog{parent,
                              message,
                              true/*use_markup*/,
                              Gtk::MESSAGE_ERROR,
                              Gtk::BUTTONS_OK,
                              true/*modal*/};
    dialog.set_title(_("Error"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// Returns the retrieved filepath or None
std::string CtDialogs::file_select_dialog(const FileSelectArgs& args)
{
#if GTKMM_MAJOR_VERSION > 3 || (GTKMM_MAJOR_VERSION == 3 && GTKMM_MINOR_VERSION >= 24)
    auto chooser = Gtk::FileChooserNative::create(_("Select File"), *args.pParentWin, Gtk::FILE_CHOOSER_ACTION_OPEN);
#else
    auto chooser = std::make_unique<Gtk::FileChooserDialog>(*args.pParentWin, _("Select File"), Gtk::FILE_CHOOSER_ACTION_OPEN);
    chooser->add_button(Gtk::StockID{GTK_STOCK_CANCEL}, Gtk::RESPONSE_CANCEL);
    chooser->add_button(Gtk::StockID{GTK_STOCK_OPEN}, Gtk::RESPONSE_ACCEPT);
#endif
    if (args.curr_folder.empty() or not fs::is_directory(args.curr_folder)) {
        chooser->set_current_folder(Glib::get_home_dir());
    }
    else {
        chooser->set_current_folder(args.curr_folder.string());
    }
    if (not args.filter_pattern.empty() or not args.filter_mime.empty()) {
        Glib::RefPtr<Gtk::FileFilter> rFileFilter = Gtk::FileFilter::create();
        rFileFilter->set_name(args.filter_name);
        for (const auto& element : args.filter_pattern) {
            rFileFilter->add_pattern(element);
        }
        for (const auto& element : args.filter_mime) {
            rFileFilter->add_mime_type(element);
        }
        chooser->add_filter(rFileFilter);
    }
    return chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "";
}

// Returns the retrieved folderpath or None
std::string CtDialogs::folder_select_dialog(const std::string& curr_folder, Gtk::Window* pParentWin)
{
#if GTKMM_MAJOR_VERSION > 3 || (GTKMM_MAJOR_VERSION == 3 && GTKMM_MINOR_VERSION >= 24)
    auto chooser = Gtk::FileChooserNative::create(_("Select Folder"), *pParentWin, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
#else
    auto chooser = std::make_unique<Gtk::FileChooserDialog>(*pParentWin, _("Select Folder"), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    chooser->add_button(Gtk::StockID{GTK_STOCK_CANCEL}, Gtk::RESPONSE_CANCEL);
    chooser->add_button(Gtk::StockID{GTK_STOCK_OPEN}, Gtk::RESPONSE_ACCEPT);
#endif
    if (curr_folder.empty() || !Glib::file_test(curr_folder, Glib::FILE_TEST_IS_DIR)) {
        chooser->set_current_folder(g_get_home_dir());
    }
    else {
        chooser->set_current_folder(curr_folder);
    }
    return chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "";
}

// Returns the retrieved filepath or None
std::string CtDialogs::file_save_as_dialog(const FileSelectArgs& args)
{
#if GTKMM_MAJOR_VERSION > 3 || (GTKMM_MAJOR_VERSION == 3 && GTKMM_MINOR_VERSION >= 24)
    auto chooser = Gtk::FileChooserNative::create(_("Save File as"), *args.pParentWin, Gtk::FILE_CHOOSER_ACTION_SAVE);
#else
    auto chooser = std::make_unique<Gtk::FileChooserDialog>(*args.pParentWin, _("Save File as"), Gtk::FILE_CHOOSER_ACTION_SAVE);
    chooser->add_button(Gtk::StockID{GTK_STOCK_CANCEL}, Gtk::RESPONSE_CANCEL);
    chooser->add_button(Gtk::StockID{GTK_STOCK_SAVE_AS}, Gtk::RESPONSE_ACCEPT);
#endif
    chooser->set_do_overwrite_confirmation(true);
    if (args.curr_folder.empty() || !fs::is_directory(args.curr_folder)) {
        chooser->set_current_folder(g_get_home_dir());
    }
    else {
        chooser->set_current_folder(args.curr_folder.string());
    }
    if (!args.curr_file_name.empty()) {
        chooser->set_current_name(args.curr_file_name.string());
    }
    if (!args.filter_pattern.empty()) {
        Glib::RefPtr<Gtk::FileFilter> rFileFilter = Gtk::FileFilter::create();
        rFileFilter->set_name(args.filter_name);
        for (const Glib::ustring& element : args.filter_pattern) {
            rFileFilter->add_pattern(element);
        }
        chooser->add_filter(rFileFilter);
    }
    return chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "";
}
