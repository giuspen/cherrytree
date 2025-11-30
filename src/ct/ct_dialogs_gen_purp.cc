/*
 * ct_dialogs_gen_purp.cc
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

#include "ct_dialogs.h"
#include "ct_main_win.h"

CtDialogTextEntry::CtDialogTextEntry(const Glib::ustring& title,
                                     const bool forPassword,
                                     Gtk::Window* pParentWin)
{
    set_title(title);
    set_transient_for(*pParentWin);
    property_destroy_with_parent() = true;
    set_modal();

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    (void)CtMiscUtil::dialog_add_button(this, _("Cancel"), Gtk::RESPONSE_CANCEL, "ct_cancel");
    (void)CtMiscUtil::dialog_add_button(this, _("OK"), Gtk::RESPONSE_OK, "ct_done", true/*isDefault*/);
    _entry.set_icon_from_gicon(Gio::ThemedIcon::create("ct_clear"), Gtk::ENTRY_ICON_SECONDARY);
    _entry.set_size_request(350, -1);
    if (forPassword) {
        _entry.set_visibility(false);
    }
    get_content_area()->pack_start(_entry, true, true, 0);
    _entry.signal_key_press_event().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_on_entry_key_press_event), false/*call me before other*/);
    _entry.signal_icon_press().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_on_entry_icon_press));
    get_content_area()->show_all();
#else
    add_button(_("Cancel"), 0);
    add_button(_("OK"), 1);
    // Entry icon position API removed in GTK4
    _entry.set_size_request(350, -1);
    if (forPassword) {
        _entry.set_visibility(false);
    }
    get_content_area()->append(_entry);
#endif
}

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
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
#endif

Gtk::TreeModel::iterator CtDialogs::choose_item_dialog(Gtk::Window& parent,
                                            const Glib::ustring& title,
                                            Glib::RefPtr<CtChooseDialogListStore> pModel,
                                            const gchar* single_column_name/*= nullptr*/,
                                            const std::string& pathToSelect/*= "0"*/,
                                            std::optional<std::pair<int,int>> use_size/*= std::nullopt*/,
                                            const bool column_is_colour/*= false*/)
{
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    Gtk::Dialog dialog{title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(parent);
    (void)CtMiscUtil::dialog_add_button(&dialog, _("Cancel"), Gtk::RESPONSE_REJECT, "ct_cancel");
    (void)CtMiscUtil::dialog_add_button(&dialog, _("OK"), Gtk::RESPONSE_ACCEPT, "ct_done", true/*isDefault*/);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    int use_width, use_height;
    if (use_size.has_value()) { use_width = use_size.value().first; use_height = use_size.value().second; }
    else { parent.get_size(use_width, use_height); use_width = 200; }
    dialog.set_default_size(use_width, use_height);
    auto pScrolledwindow = Gtk::manage(new Gtk::ScrolledWindow{});
    pScrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    auto pElementsTreeview = Gtk::manage(new Gtk::TreeView{pModel});
    pElementsTreeview->set_headers_visible(false);
    const auto treePathToSelect = Gtk::TreePath{pathToSelect};
    pElementsTreeview->get_selection()->select(treePathToSelect);
    pElementsTreeview->signal_row_activated().connect([&](const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*) {
        if (Gtk::TreeModel::iterator iter = pElementsTreeview->get_selection()->get_selected()) {
            static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT))->clicked();
        }
    });
    Gtk::CellRendererPixbuf pixbuf_renderer;
    if (nullptr == single_column_name) {
        pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
        const int col_num = pElementsTreeview->append_column("", pixbuf_renderer) - 1;
        pElementsTreeview->get_column(col_num)->add_attribute(pixbuf_renderer, "icon-name", pModel->columns.stock_id);
        pElementsTreeview->append_column("", pModel->columns.desc);
        pElementsTreeview->set_search_column(2);
    } else {
        const int col_num = pElementsTreeview->append_column(single_column_name, pModel->columns.desc) - 1;
        if (column_is_colour) {
            Gtk::TreeViewColumn* pTVCol = pElementsTreeview->get_column(col_num);
            std::vector<Gtk::CellRenderer*> cellRenderers = pTVCol->get_cells();
            if (cellRenderers.size() > 0) {
                auto pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(cellRenderers.front());
                if (pCellRendererText) {
                    pTVCol->add_attribute(pCellRendererText->property_foreground(), pModel->columns.desc);
                }
            }
        }
    }
    pScrolledwindow->add(*pElementsTreeview);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(*pScrolledwindow);
    pContentArea->show_all();
    pElementsTreeview->grab_focus();
    pElementsTreeview->scroll_to_row(treePathToSelect, 0.5);
    return Gtk::RESPONSE_ACCEPT == dialog.run() ? pElementsTreeview->get_selection()->get_selected() : Gtk::TreeModel::iterator{};
#else
    // GTK4 minimal stub: not implemented synchronously
    (void)single_column_name; (void)pathToSelect; (void)use_size; (void)column_is_colour;
    Gtk::TreeModel::iterator empty;
    return empty;
#endif
}

Glib::ustring CtDialogs::img_n_entry_dialog(Gtk::Window& parent,
                                            const Glib::ustring& title,
                                            const Glib::ustring& entry_content,
                                            const char* img_stock)
{
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    Gtk::Dialog dialog{title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    (void)CtMiscUtil::dialog_add_button(&dialog, _("Cancel"), Gtk::RESPONSE_REJECT, "ct_cancel");
    (void)CtMiscUtil::dialog_add_button(&dialog, _("OK"), Gtk::RESPONSE_ACCEPT, "ct_done", true/*isDefault*/);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);
    Gtk::Image image;
    image.set_from_icon_name(img_stock, Gtk::ICON_SIZE_BUTTON);
    Gtk::Entry entry;
    entry.set_text(entry_content);
    Gtk::Box hbox{Gtk::ORIENTATION_HORIZONTAL, 5/*spacing*/};
    hbox.pack_start(image, false, false);
    hbox.pack_start(entry);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(hbox);
    pContentArea->show_all();
    entry.grab_focus();
    entry.signal_activate().connect([&](){ if (not entry.get_text().empty()) dialog.response(Gtk::RESPONSE_ACCEPT); });
    return Gtk::RESPONSE_ACCEPT == dialog.run() ? str::trim(entry.get_text()) : "";
#else
    // GTK4 minimal: return provided content without UI
    return str::trim(entry_content);
#endif
}

std::time_t CtDialogs::date_select_dialog(Gtk::Window& parent,
                                          const Glib::ustring& title,
                                          const std::time_t& curr_time)
{
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    Gtk::Dialog dialog{title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(parent);
    (void)CtMiscUtil::dialog_add_button(&dialog, _("Cancel"), Gtk::RESPONSE_REJECT, "ct_cancel");
    (void)CtMiscUtil::dialog_add_button(&dialog, _("OK"), Gtk::RESPONSE_ACCEPT, "ct_done", true/*isDefault*/);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    std::tm struct_time = *std::localtime(&curr_time);
    Gtk::Box* pContentArea = dialog.get_content_area();
    Gtk::Calendar calendar;
    calendar.select_month((guint)(struct_time.tm_mon), (guint)(struct_time.tm_year + 1900));
    calendar.select_day((guint)struct_time.tm_mday);
    Glib::RefPtr<Gtk::Adjustment> rAdj_h = Gtk::Adjustment::create(struct_time.tm_hour, 0, 23, 1);
    Gtk::SpinButton spinbutton_h{rAdj_h};
    spinbutton_h.set_value(struct_time.tm_hour);
    Glib::RefPtr<Gtk::Adjustment> rAdj_m = Gtk::Adjustment::create(struct_time.tm_min, 0, 59, 1);
    Gtk::SpinButton spinbutton_m{rAdj_m};
    spinbutton_m.set_value(struct_time.tm_min);
    Gtk::Box hbox{Gtk::ORIENTATION_HORIZONTAL};
    hbox.pack_start(spinbutton_h);
    hbox.pack_start(spinbutton_m);
    pContentArea->pack_start(calendar);
    pContentArea->pack_start(hbox);
    pContentArea->show_all();
    if (dialog.run() != Gtk::RESPONSE_ACCEPT) return 0;
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
#else
    // GTK4 minimal stub: not implemented synchronously
    return 0;
#endif
}

CtDialogs::CtPickDlgState CtDialogs::colour_pick_dialog(CtMainWin* pCtMainWin,
                                                        const Glib::ustring& title,
                                                        Glib::ustring& ret_colour,
                                                        bool allow_remove_colour)
{
#if GTK_MAJOR_VERSION < 4
    Gtk::ColorChooserDialog dialog{title};
    dialog.set_transient_for(*pCtMainWin);
    dialog.set_modal(true);
    dialog.property_destroy_with_parent() = true;
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    if (allow_remove_colour) {
        dialog.add_button(_("Remove"), Gtk::RESPONSE_NONE);
    }
    // from gtk3 branch gtk-3-24 file gtk/gtk/gtkcolorchooserwidget.c function add_default_palette
    const gchar* default_colors[45]{
        "#99c1f1", "#62a0ea", "#3584e4", "#1c71d8", "#1a5fb4", /* Blue */
        "#8ff0a4", "#57e389", "#33d17a", "#2ec27e", "#26a269", /* Green */
        "#f9f06b", "#f8e45c", "#f6d32d", "#f5c211", "#e5a50a", /* Yellow */
        "#ffbe6f", "#ffa348", "#ff7800", "#e66100", "#c64600", /* Orange */
        "#f66151", "#ed333b", "#e01b24", "#c01c28", "#a51d2d", /* Red */
        "#dc8add", "#c061cb", "#9141ac", "#813d9c", "#613583", /* Purple */
        "#cdab8f", "#b5835a", "#986a44", "#865e3c", "#63452c", /* Brown */
        "#ffffff", "#f6f5f4", "#deddda", "#c0bfbc", "#9a9996", /* Light */
        "#77767b", "#5e5c64", "#3d3846", "#241f31", "#000000"};/* Dark */
    std::vector<Gdk::RGBA> default_colours;
    auto& coloursUserPalette = pCtMainWin->get_ct_config()->coloursUserPalette;
    auto f_add_palette = [&](){
        default_colours.clear();
        std::vector<Gdk::RGBA> palette_colours;
        size_t column_idx{0u};
        for (int i = 0; i < 45; ++i) {
            const Gdk::RGBA curr_colour{default_colors[i]};
            default_colours.push_back(curr_colour);
            palette_colours.push_back(curr_colour);
            if (coloursUserPalette.size() > 0u) {
                if (4 == i % 5) {
                    if (coloursUserPalette.size() > column_idx) palette_colours.push_back(*coloursUserPalette.at(column_idx));
                    else palette_colours.push_back(Gdk::RGBA{});
                    if (coloursUserPalette.size() > 9u) {
                        if (coloursUserPalette.size() > (9+column_idx)) palette_colours.push_back(*coloursUserPalette.at(9+column_idx));
                        else palette_colours.push_back(Gdk::RGBA{});
                    }
                    ++column_idx;
                }
            }
        }
        dialog.add_palette(Gtk::Orientation::ORIENTATION_VERTICAL, 5 + (coloursUserPalette.size() + 8)/9, palette_colours);
    };
    f_add_palette();
    dialog.set_rgba(Gdk::RGBA{ret_colour});

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

    Gtk::Box* pContentArea = dialog.get_content_area();
    std::vector<Gtk::Widget*> content_children = pContentArea->get_children();
    //spdlog::debug("{} content_children", content_children.size());
    if (content_children.size() > 0u) {
        auto pInnerBox = dynamic_cast<Gtk::Box*>(content_children.front());
        if (pInnerBox) {
            std::vector<Gtk::Widget*> inner_children = pInnerBox->get_children();
            //spdlog::debug("{} inner_children", inner_children.size());
            if (inner_children.size() > 0u) {
                auto pInner2Box = dynamic_cast<Gtk::Box*>(inner_children.front());
                if (pInner2Box) {
                    std::vector<Gtk::Widget*> inner2_children = pInner2Box->get_children();
                    spdlog::debug("{} inner2_children", inner2_children.size());
                    if (3u == inner2_children.size()) {
                        inner2_children.at(1)->hide();
                        inner2_children.at(2)->hide();
                        auto hbox_more_less = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
                        auto button_more = Gtk::manage(new Gtk::Button{});
                        button_more->set_image(*pCtMainWin->new_managed_image_from_stock("ct_add", Gtk::ICON_SIZE_BUTTON));
                        hbox_more_less->pack_start(*button_more, false, false);
                        auto button_less = Gtk::manage(new Gtk::Button{});
                        button_less->set_image(*pCtMainWin->new_managed_image_from_stock("ct_remove", Gtk::ICON_SIZE_BUTTON));
                        button_less->set_sensitive(coloursUserPalette.size() > 0u);
                        hbox_more_less->pack_start(*button_less, false, false);
                        hbox_more_less->show_all();
                        pInner2Box->pack_start(*hbox_more_less, false, false);
                        button_more->signal_clicked().connect([&](){
                            dialog.property_show_editor() = true;
                        });
                        button_less->signal_clicked().connect([&](){
                            auto itemStore = CtChooseDialogListStore::create();
                            for (const Gdk::RGBA& curr_rgba : coloursUserPalette) {
                                itemStore->add_row("", "", curr_rgba.to_string());
                            }
                            const Gtk::TreeModel::iterator treeIter = CtDialogs::choose_item_dialog(dialog,
                                                                                         _("Remove User Colour from Palette"),
                                                                                         itemStore,
                                                                                         _("Colour"),
                                                                                         "0"/*pathToSelect*/,
                                                                                         std::nullopt/*use_size*/,
                                                                                         true/*column_is_colour*/);
                            if (treeIter) {
                                const Glib::ustring colour_to_rm = treeIter->get_value(itemStore->columns.desc);
                                for (const Gdk::RGBA& curr_rgba : coloursUserPalette) {
                                    if (colour_to_rm == curr_rgba.to_string()) {
                                        coloursUserPalette.remove(curr_rgba);
                                        break;
                                    }
                                }
                                dialog.response(Gtk::RESPONSE_HELP);
                            }
                        });
                    }
                }
            }
        }
    }

    const int response = dialog.run();

    if (Gtk::RESPONSE_NONE == response) {
        return CtPickDlgState::REMOVE_COLOR;
    }
    if (Gtk::RESPONSE_HELP == response) {
        return CtPickDlgState::CALL_AGAIN;
    }
    if (Gtk::RESPONSE_OK != response) {
        return CtPickDlgState::CANCEL;
    }
    const Gdk::RGBA sel_colour = dialog.get_rgba();
    ret_colour = CtRgbUtil::rgb_to_string_24(sel_colour);
    if (not vec::exists(default_colours, sel_colour)) {
        coloursUserPalette.move_or_push_front(sel_colour);
    }
    return CtPickDlgState::SELECTED;
#else
    (void)pCtMainWin; (void)title; (void)ret_colour; (void)allow_remove_colour;
    // GTK4 minimal stub: not implemented
    return CtPickDlgState::CANCEL;
#endif
}

// Returns True if the user presses OK
bool CtDialogs::question_dialog(const Glib::ustring& message,
                                Gtk::Window& parent)
{
#if GTK_MAJOR_VERSION < 4
    Gtk::MessageDialog dialog{parent, message, true/*use_markup*/, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true/*modal*/};
    dialog.set_title(_("Question"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.property_destroy_with_parent() = true;
    static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_YES))->grab_focus();
    return (Gtk::RESPONSE_YES == dialog.run());
#else
    // GTK4 minimal stub
    (void)message; (void)parent;
    return false;
#endif
}

void CtDialogs::info_dialog(const Glib::ustring& message,
                            Gtk::Window& parent)
{
#if GTK_MAJOR_VERSION < 4
    Gtk::MessageDialog dialog{parent, message, true/*use_markup*/, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true/*modal*/};
    dialog.set_title(_("Info"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.property_destroy_with_parent() = true;
    dialog.run();
#else
    (void)message; (void)parent; // GTK4 minimal stub
#endif
}

void CtDialogs::warning_dialog(const Glib::ustring& message,
                               Gtk::Window& parent)
{
#if GTK_MAJOR_VERSION < 4
    Gtk::MessageDialog dialog{parent, message, true/*use_markup*/, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true/*modal*/};
    dialog.set_title(_("Warning"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.property_destroy_with_parent() = true;
    dialog.run();
#else
    (void)message; (void)parent; // GTK4 minimal stub
#endif
}

void CtDialogs::error_dialog(const Glib::ustring& message,
                             Gtk::Window& parent)
{
#if GTK_MAJOR_VERSION < 4
    Gtk::MessageDialog dialog{parent, message, true/*use_markup*/, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true/*modal*/};
    dialog.set_title(_("Error"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.property_destroy_with_parent() = true;
    dialog.run();
#else
    (void)message; (void)parent; // GTK4 minimal stub
#endif
}

std::string CtDialogs::file_select_dialog(Gtk::Window* pParentWin, const CtFileSelectArgs& args)
{
#if GTK_MAJOR_VERSION < 4
    auto chooser = Gtk::FileChooserNative::create(_("Select File"), *pParentWin, Gtk::FILE_CHOOSER_ACTION_OPEN);
#else
    // GTK4 minimal stub: not implemented
    (void)pParentWin; (void)args;
    return std::string{};
#endif
#if GTKMM_MAJOR_VERSION < 3
    auto chooser = std::make_unique<Gtk::FileChooserDialog>(*pParentWin, _("Select File"), Gtk::FILE_CHOOSER_ACTION_OPEN);
    chooser->add_button(Gtk::StockID{GTK_STOCK_CANCEL}, Gtk::RESPONSE_CANCEL);
    chooser->add_button(Gtk::StockID{GTK_STOCK_OPEN}, Gtk::RESPONSE_ACCEPT);
    chooser->property_destroy_with_parent() = true;
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

std::string CtDialogs::folder_select_dialog(Gtk::Window* pParentWin, const std::string& curr_folder)
{
#if GTK_MAJOR_VERSION < 4
    auto chooser = Gtk::FileChooserNative::create(_("Select Folder"), *pParentWin, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
#else
    // GTK4 minimal stub: not implemented
    (void)pParentWin; (void)curr_folder;
    return std::string{};
#endif
#if GTKMM_MAJOR_VERSION < 3
    auto chooser = std::make_unique<Gtk::FileChooserDialog>(*pParentWin, _("Select Folder"), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    chooser->add_button(Gtk::StockID{GTK_STOCK_CANCEL}, Gtk::RESPONSE_CANCEL);
    chooser->add_button(Gtk::StockID{GTK_STOCK_OPEN}, Gtk::RESPONSE_ACCEPT);
    chooser->property_destroy_with_parent() = true;
#endif
    if (curr_folder.empty() or not Glib::file_test(curr_folder, Glib::FILE_TEST_IS_DIR)) {
        chooser->set_current_folder(g_get_home_dir());
    }
    else {
        chooser->set_current_folder(curr_folder);
    }
        return chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "";
}

std::string CtDialogs::file_save_as_dialog(Gtk::Window* pParentWin, const CtFileSelectArgs& args)
{
#if GTK_MAJOR_VERSION < 4
    auto chooser = Gtk::FileChooserNative::create(_("Save File as"), *pParentWin, Gtk::FILE_CHOOSER_ACTION_SAVE);
#else
    // GTK4 minimal stub: not implemented
    (void)pParentWin; (void)args;
    return std::string{};
#endif
#if GTKMM_MAJOR_VERSION < 3
    auto chooser = std::make_unique<Gtk::FileChooserDialog>(*pParentWin, _("Save File as"), Gtk::FILE_CHOOSER_ACTION_SAVE);
    chooser->add_button(Gtk::StockID{GTK_STOCK_CANCEL}, Gtk::RESPONSE_CANCEL);
    chooser->add_button(Gtk::StockID{GTK_STOCK_SAVE_AS}, Gtk::RESPONSE_ACCEPT);
    chooser->property_destroy_with_parent() = true;
#endif
    chooser->set_do_overwrite_confirmation(args.overwrite_confirmation);
    if (args.curr_folder.empty() or not fs::is_directory(args.curr_folder)) {
        chooser->set_current_folder(g_get_home_dir());
    }
    else {
        chooser->set_current_folder(args.curr_folder.string());
    }
    if (not args.curr_file_name.empty()) {
        chooser->set_current_name(args.curr_file_name.string());
    }
    if (not args.filter_pattern.empty()) {
        Glib::RefPtr<Gtk::FileFilter> rFileFilter = Gtk::FileFilter::create();
        rFileFilter->set_name(args.filter_name);
        for (const Glib::ustring& element : args.filter_pattern) {
            rFileFilter->add_pattern(element);
        }
        chooser->add_filter(rFileFilter);
    }
        return chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "";
}

std::string CtDialogs::folder_save_as_dialog(Gtk::Window* pParentWin, const CtFileSelectArgs& args)
{
    // GTK4 minimal: use SELECT_FOLDER instead of create folder
#if GTK_MAJOR_VERSION < 4
    auto chooser = std::make_unique<Gtk::FileChooserDialog>(*pParentWin, _("Save To Folder"), Gtk::FILE_CHOOSER_ACTION_CREATE_FOLDER);
    (void)CtMiscUtil::dialog_add_button(chooser.get(), _("Cancel"), Gtk::RESPONSE_CANCEL, "ct_cancel");
    (void)CtMiscUtil::dialog_add_button(chooser.get(), _("Save"), Gtk::RESPONSE_ACCEPT, "ct_save-as");
    chooser->property_destroy_with_parent() = true;
#else
    // GTK4 minimal stub: not implemented
    (void)pParentWin; (void)args;
    return std::string{};
#endif
    //chooser->set_do_overwrite_confirmation(true); unfortunately works only with Gtk::FILE_CHOOSER_ACTION_SAVE
    if (args.curr_folder.empty() or not fs::is_directory(args.curr_folder)) {
        chooser->set_current_folder(g_get_home_dir());
    }
    else {
        chooser->set_current_folder(args.curr_folder.string());
    }
    if (not args.curr_file_name.empty()) {
        chooser->set_current_name(args.curr_file_name.string());
    }
        return chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "";
}
