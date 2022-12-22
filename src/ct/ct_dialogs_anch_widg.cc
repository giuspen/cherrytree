/*
 * ct_dialogs_anch_widg.cc
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

#include "ct_dialogs.h"
#include "ct_main_win.h"
#include "ct_text_view.h"

Glib::ustring CtDialogs::latex_handle_dialog(CtMainWin* pCtMainWin,
                                             const Glib::ustring& latex_text)
{
    CtTextView textView{pCtMainWin};
    Glib::RefPtr<Gsv::Buffer> rBuffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(textView.get_buffer());
    rBuffer->set_text(latex_text);
    textView.setup_for_syntax("latex");
    pCtMainWin->apply_syntax_highlighting(rBuffer, "latex", false/*forceReApply*/);
    auto scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow{});
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow->add(textView);
    Gtk::Dialog dialog{_("Latex Text"),
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 250);
    Gtk::Box* pContentArea = dialog.get_content_area();
    auto hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 2/*spacing*/});
    auto vbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 2/*spacing*/});
    hbox->pack_start(*scrolledwindow);
    hbox->pack_start(*vbox, false, false);
    auto pCtConfig = pCtMainWin->get_ct_config();
    auto label_latex_size = Gtk::manage(new Gtk::Label{_("Image Size dpi")});
    Glib::RefPtr<Gtk::Adjustment> adj_latex_size = Gtk::Adjustment::create(pCtConfig->latexSizeDpi, 1, 10000, 10);
    auto spinbutton_latex_size = Gtk::manage(new Gtk::SpinButton{adj_latex_size});
    vbox->pack_end(*spinbutton_latex_size, false, false);
    vbox->pack_end(*label_latex_size, false, false);
    auto button_latex_tutorial = Gtk::manage(new Gtk::Button{});
    auto button_latex_reference = Gtk::manage(new Gtk::Button{});
    button_latex_tutorial->set_label(_("Tutorial"));
    button_latex_reference->set_label(_("Reference"));
    button_latex_tutorial->set_image_from_icon_name("ct_link_website", Gtk::ICON_SIZE_MENU);
    button_latex_reference->set_image_from_icon_name("ct_link_website", Gtk::ICON_SIZE_MENU);
    button_latex_tutorial->set_tooltip_text(_("LaTeX Math and Equations Tutorial"));
    button_latex_reference->set_tooltip_text(_("LaTeX Math Symbols Reference"));
    button_latex_tutorial->set_always_show_image(true);
    button_latex_reference->set_always_show_image(true);
    vbox->pack_start(*button_latex_tutorial, false, false);
    vbox->pack_start(*button_latex_reference, false, false);
    Glib::ustring error_msg = CtImageLatex::getRenderingErrorMessage();
    if (not error_msg.empty()) {
        auto p_label_error_msg = Gtk::manage(new Gtk::Label{error_msg});
        p_label_error_msg->set_use_markup(true);
        pContentArea->pack_start(*p_label_error_msg);
    }
    pContentArea->pack_start(*hbox);
    spinbutton_latex_size->signal_value_changed().connect([pCtMainWin, spinbutton_latex_size](){
        pCtMainWin->get_ct_config()->latexSizeDpi = spinbutton_latex_size->get_value_as_int();
    });
    button_latex_tutorial->signal_clicked().connect([](){
        fs::open_weblink("https://latex-tutorial.com/tutorials/amsmath/");
    });
    button_latex_reference->signal_clicked().connect([](){
        fs::open_weblink("https://latex-tutorial.com/symbols/math-symbols/");
    });
    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Escape == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_REJECT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);
    pContentArea->show_all();
    return Gtk::RESPONSE_ACCEPT == dialog.run() ? rBuffer->get_text() : "";
}

Glib::RefPtr<Gdk::Pixbuf> CtDialogs::image_handle_dialog(Gtk::Window& parent_win,
                                                         Glib::RefPtr<Gdk::Pixbuf> rOriginalPixbuf)
{
    int width = rOriginalPixbuf->get_width();
    int height = rOriginalPixbuf->get_height();
    double image_w_h_ration = static_cast<double>(width)/height;

    Gtk::Dialog dialog{_("Image Properties"),
                       parent_win,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);
    Gtk::Button button_rotate_90_ccw;
    button_rotate_90_ccw.set_image_from_icon_name("ct_rotate-left", Gtk::ICON_SIZE_DND);
    Gtk::Button button_rotate_90_cw;
    button_rotate_90_cw.set_image_from_icon_name("ct_rotate-right", Gtk::ICON_SIZE_DND);
    Gtk::ScrolledWindow scrolledwindow;
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Glib::RefPtr<Gtk::Adjustment> rHAdj = Gtk::Adjustment::create(width, 1, height, 1);
    Glib::RefPtr<Gtk::Adjustment> rVAdj = Gtk::Adjustment::create(width, 1, width, 1);
    Gtk::Viewport viewport(rHAdj, rVAdj);
    Gtk::Image image{rOriginalPixbuf};
    scrolledwindow.add(viewport);
    viewport.add(image);
    Gtk::Box hbox_1{Gtk::ORIENTATION_HORIZONTAL, 2/*spacing*/};
    hbox_1.pack_start(button_rotate_90_ccw, false, false);
    hbox_1.pack_start(scrolledwindow);
    hbox_1.pack_start(button_rotate_90_cw, false, false);
    Gtk::Button button_flip_horizontal;
    button_flip_horizontal.set_image_from_icon_name("ct_flip-horizontal", Gtk::ICON_SIZE_DND);
    Gtk::Button button_flip_vertical;
    button_flip_vertical.set_image_from_icon_name("ct_flip-vertical", Gtk::ICON_SIZE_DND);
    Gtk::Box hbox_2{Gtk::ORIENTATION_HORIZONTAL, 2/*spacing*/};
    hbox_2.pack_start(button_flip_horizontal, true, true);
    hbox_2.pack_start(button_flip_vertical, true, true);
    Gtk::Label label_width{_("Width")};
    Glib::RefPtr<Gtk::Adjustment> rAdj_width = Gtk::Adjustment::create(width, 1, 10000, 1);
    Gtk::SpinButton spinbutton_width{rAdj_width};
    Gtk::Label label_height{_("Height")};
    Glib::RefPtr<Gtk::Adjustment> rAdj_height = Gtk::Adjustment::create(height, 1, 10000, 1);
    Gtk::SpinButton spinbutton_height{rAdj_height};
    Gtk::Box hbox_3{Gtk::ORIENTATION_HORIZONTAL};
    hbox_3.pack_start(label_width);
    hbox_3.pack_start(spinbutton_width);
    hbox_3.pack_start(label_height);
    hbox_3.pack_start(spinbutton_height);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(hbox_1);
    pContentArea->pack_start(hbox_2, false, false);
    pContentArea->pack_start(hbox_3, false, false);
    pContentArea->set_spacing(6);

    bool stop_update = false;
    auto image_load_into_dialog = [&]() {
        stop_update = true;
        spinbutton_width.set_value(width);
        spinbutton_height.set_value(height);
        Glib::RefPtr<Gdk::Pixbuf> rPixbuf;
        if (width <= 900 && height <= 600) {
            // original size into the dialog
            rPixbuf = rOriginalPixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR);
        }
        else {
            // reduced size visible into the dialog
            if (width > 900) {
                int img_parms_width = 900;
                int img_parms_height = (int)(img_parms_width / image_w_h_ration);
                rPixbuf = rOriginalPixbuf->scale_simple(img_parms_width, img_parms_height, Gdk::INTERP_BILINEAR);
            }
            else {
                int img_parms_height = 600;
                int img_parms_width = (int)(img_parms_height * image_w_h_ration);
                rPixbuf = rOriginalPixbuf->scale_simple(img_parms_width, img_parms_height, Gdk::INTERP_BILINEAR);
            }
        }
        image.set(rPixbuf);
        stop_update = false;
    };
    button_rotate_90_cw.signal_clicked().connect([&](){
        rOriginalPixbuf = rOriginalPixbuf->rotate_simple(Gdk::PixbufRotation::PIXBUF_ROTATE_CLOCKWISE);
        image_w_h_ration = 1./image_w_h_ration;
        std::swap(width, height); // new width is the former height and vice versa
        image_load_into_dialog();
    });
    button_rotate_90_ccw.signal_clicked().connect([&](){
        rOriginalPixbuf = rOriginalPixbuf->rotate_simple(Gdk::PixbufRotation::PIXBUF_ROTATE_COUNTERCLOCKWISE);
        image_w_h_ration = 1./image_w_h_ration;
        std::swap(width, height); // new width is the former height and vice versa
        image_load_into_dialog();
    });
    button_flip_horizontal.signal_clicked().connect([&](){
        rOriginalPixbuf = rOriginalPixbuf->flip(true);
        image_load_into_dialog();
    });
    button_flip_vertical.signal_clicked().connect([&](){
        rOriginalPixbuf = rOriginalPixbuf->flip(false);
        image_load_into_dialog();
    });
    spinbutton_width.signal_value_changed().connect([&](){
        if (stop_update) return;
        width = spinbutton_width.get_value_as_int();
        height = (int)(width/image_w_h_ration);
        image_load_into_dialog();
    });
    spinbutton_height.signal_value_changed().connect([&](){
        if (stop_update) return;
        height = spinbutton_height.get_value_as_int();
        width = (int)(height*image_w_h_ration);
        image_load_into_dialog();
    });
    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        if (GDK_KEY_Escape == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_REJECT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);
    image_load_into_dialog();
    pContentArea->show_all();
    return Gtk::RESPONSE_ACCEPT == dialog.run() ? rOriginalPixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR) : Glib::RefPtr<Gdk::Pixbuf>{};
}

bool CtDialogs::codeboxhandle_dialog(CtMainWin* pCtMainWin,
                                     const Glib::ustring& title)
{
    Gtk::Dialog dialog{title,
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(300, -1);
    dialog.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);

    CtConfig* pConfig = pCtMainWin->get_ct_config();

    Gtk::Button button_prog_lang;
    const Glib::ustring syntax_hl_id = pConfig->codeboxSynHighl != CtConst::PLAIN_TEXT_ID ? pConfig->codeboxSynHighl : pConfig->autoSynHighl;
    const std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
    button_prog_lang.set_label(syntax_hl_id);
    button_prog_lang.set_image(*pCtMainWin->new_managed_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
    Gtk::RadioButton radiobutton_plain_text{_("Plain Text")};
    Gtk::RadioButton radiobutton_auto_syntax_highl{_("Automatic Syntax Highlighting")};
    radiobutton_auto_syntax_highl.join_group(radiobutton_plain_text);
    if (pConfig->codeboxSynHighl == CtConst::PLAIN_TEXT_ID) {
        radiobutton_plain_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    }
    else {
        radiobutton_auto_syntax_highl.set_active(true);
    }
    Gtk::Box type_vbox{Gtk::ORIENTATION_VERTICAL};
    type_vbox.pack_start(radiobutton_plain_text);
    type_vbox.pack_start(radiobutton_auto_syntax_highl);
    type_vbox.pack_start(button_prog_lang);
    Gtk::Frame type_frame{Glib::ustring("<b>")+_("Type")+"</b>"};
    dynamic_cast<Gtk::Label*>(type_frame.get_label_widget())->set_use_markup(true);
    type_frame.set_shadow_type(Gtk::SHADOW_NONE);
    type_frame.add(type_vbox);

    Gtk::Label label_width{_("Width")};
    Glib::RefPtr<Gtk::Adjustment> rAdj_width = Gtk::Adjustment::create(pConfig->codeboxWidth, 1, 10000);
    Gtk::SpinButton spinbutton_width{rAdj_width};
    spinbutton_width.set_value(pConfig->codeboxWidth);
    Gtk::Label label_height{_("Height")};
    Glib::RefPtr<Gtk::Adjustment> rAdj_height = Gtk::Adjustment::create(pConfig->codeboxHeight, 1, 10000);
    Gtk::SpinButton spinbutton_height{rAdj_height};
    spinbutton_height.set_value(pConfig->codeboxHeight);

    Gtk::RadioButton radiobutton_codebox_pixels{_("pixels")};
    Gtk::RadioButton radiobutton_codebox_percent{"%"};
    radiobutton_codebox_percent.join_group(radiobutton_codebox_pixels);
    radiobutton_codebox_pixels.set_active(pConfig->codeboxWidthPixels);
    radiobutton_codebox_percent.set_active(!pConfig->codeboxWidthPixels);

    Gtk::Box vbox_pix_perc{Gtk::ORIENTATION_VERTICAL};
    vbox_pix_perc.pack_start(radiobutton_codebox_pixels);
    vbox_pix_perc.pack_start(radiobutton_codebox_percent);
    Gtk::Box hbox_width{Gtk::ORIENTATION_HORIZONTAL, 5/*spacing*/};
    hbox_width.pack_start(label_width, false, false);
    hbox_width.pack_start(spinbutton_width, false, false);
    hbox_width.pack_start(vbox_pix_perc);
    Gtk::Box hbox_height{Gtk::ORIENTATION_HORIZONTAL, 5/*spacing*/};
    hbox_height.pack_start(label_height, false, false);
    hbox_height.pack_start(spinbutton_height, false, false);
    Gtk::Box vbox_size{Gtk::ORIENTATION_VERTICAL};
    vbox_size.pack_start(hbox_width);
    vbox_size.pack_start(hbox_height);
    Gtk::Alignment size_align;
    size_align.set_padding(0, 6, 6, 6);
    size_align.add(vbox_size);

    Gtk::Frame size_frame{Glib::ustring("<b>")+_("Size")+"</b>"};
    dynamic_cast<Gtk::Label*>(size_frame.get_label_widget())->set_use_markup(true);
    size_frame.set_shadow_type(Gtk::SHADOW_NONE);
    size_frame.add(size_align);

    Gtk::CheckButton checkbutton_codebox_linenumbers{_("Show Line Numbers")};
    checkbutton_codebox_linenumbers.set_active(pConfig->codeboxLineNum);
    Gtk::CheckButton checkbutton_codebox_matchbrackets{_("Highlight Matching Brackets")};
    checkbutton_codebox_matchbrackets.set_active(pConfig->codeboxMatchBra);
    Gtk::Box vbox_options{Gtk::ORIENTATION_VERTICAL};
    vbox_options.pack_start(checkbutton_codebox_linenumbers);
    vbox_options.pack_start(checkbutton_codebox_matchbrackets);
    Gtk::Alignment opt_align;
    opt_align.set_padding(6, 6, 6, 6);
    opt_align.add(vbox_options);

    Gtk::Frame options_frame{Glib::ustring("<b>")+_("Options")+"</b>"};
    dynamic_cast<Gtk::Label*>(options_frame.get_label_widget())->set_use_markup(true);
    options_frame.set_shadow_type(Gtk::SHADOW_NONE);
    options_frame.add(opt_align);

    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->set_spacing(5);
    pContentArea->pack_start(type_frame);
    pContentArea->pack_start(size_frame);
    pContentArea->pack_start(options_frame);
    pContentArea->show_all();

    button_prog_lang.signal_clicked().connect([&button_prog_lang, &dialog, pCtMainWin](){
        Glib::RefPtr<CtChooseDialogListStore> rItemStore = CtChooseDialogListStore::create();
        unsigned pathSelectIdx{0};
        unsigned pathCurrIdx{0};
        const auto currSyntaxHighl = button_prog_lang.get_label();
        for (const std::string& lang : pCtMainWin->get_language_manager()->get_language_ids()) {
            rItemStore->add_row(pCtMainWin->get_code_icon_name(lang), "", lang);
            if (lang == currSyntaxHighl) {
                pathSelectIdx = pathCurrIdx;
            }
            ++pathCurrIdx;
        }
        Gtk::TreeIter res = CtDialogs::choose_item_dialog(dialog,
                                                          _("Automatic Syntax Highlighting"),
                                                          rItemStore,
                                                          nullptr/*single_column_name*/,
                                                          std::to_string(pathSelectIdx));
        if (res) {
            const Glib::ustring syntax_hl_id = res->get_value(rItemStore->columns.desc);
            const std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
            button_prog_lang.set_label(syntax_hl_id);
            button_prog_lang.set_image(*pCtMainWin->new_managed_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
        }
    });
    radiobutton_auto_syntax_highl.signal_toggled().connect([&radiobutton_auto_syntax_highl, &button_prog_lang](){
        button_prog_lang.set_sensitive(radiobutton_auto_syntax_highl.get_active());
    });
    dialog.signal_key_press_event().connect([&](GdkEventKey* pEventKey){
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            spinbutton_width.update();
            spinbutton_height.update();
            dialog.response(Gtk::RESPONSE_ACCEPT);
            return true;
        }
        return false;
    });
    radiobutton_codebox_pixels.signal_toggled().connect([&radiobutton_codebox_pixels, &spinbutton_width](){
        if (radiobutton_codebox_pixels.get_active()) {
            spinbutton_width.set_value(700);
        }
        else if (spinbutton_width.get_value() > 100) {
            spinbutton_width.set_value(90);
        }
    });
    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        if (GDK_KEY_Escape == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_REJECT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    const int response = dialog.run();
    dialog.hide();

    if (response == Gtk::RESPONSE_ACCEPT) {
        pConfig->codeboxWidth = spinbutton_width.get_value_as_int();
        pConfig->codeboxWidthPixels = radiobutton_codebox_pixels.get_active();
        pConfig->codeboxHeight = spinbutton_height.get_value();
        pConfig->codeboxLineNum = checkbutton_codebox_linenumbers.get_active();
        pConfig->codeboxMatchBra = checkbutton_codebox_matchbrackets.get_active();
        if (radiobutton_plain_text.get_active()) {
            pConfig->codeboxSynHighl = CtConst::PLAIN_TEXT_ID;
        }
        else {
            pConfig->codeboxSynHighl = button_prog_lang.get_label();
        }
        return true;
    }
    return false;
}

CtDialogs::TableHandleResp CtDialogs::table_handle_dialog(CtMainWin* pCtMainWin,
                                                          const Glib::ustring& title,
                                                          const bool is_insert,
                                                          bool& is_light)
{
    Gtk::Dialog dialog{title,
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(*pCtMainWin);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);

    auto pCtConfig = pCtMainWin->get_ct_config();
    auto label_rows = Gtk::Label{_("Rows")};
    label_rows.set_halign(Gtk::Align::ALIGN_START);
    label_rows.set_margin_left(10);
    auto adj_rows = Gtk::Adjustment::create(pCtConfig->tableRows, 1, 10000, 1);
    auto spinbutton_rows = Gtk::SpinButton{adj_rows};
    spinbutton_rows.set_value(pCtConfig->tableRows);
    auto label_columns = Gtk::Label{_("Columns")};
    label_columns.set_halign(Gtk::Align::ALIGN_START);
    auto adj_columns = Gtk::Adjustment::create(pCtConfig->tableColumns, 1, 10000, 1);
    auto spinbutton_columns = Gtk::SpinButton{adj_columns};
    spinbutton_columns.set_value(pCtConfig->tableColumns);

    auto label_col_width = Gtk::Label{_("Default Width")};
    label_col_width.set_halign(Gtk::Align::ALIGN_START);
    auto adj_col_width = Gtk::Adjustment::create(pCtConfig->tableColWidthDefault, 1, 10000, 1);
    auto spinbutton_col_width = Gtk::SpinButton{adj_col_width};
    spinbutton_col_width.set_value(pCtConfig->tableColWidthDefault);

    auto label_size = Gtk::Label{std::string("<b>")+_("Table Size")+"</b>"};
    label_size.set_use_markup();
    label_size.set_halign(Gtk::Align::ALIGN_START);
    auto label_col = Gtk::Label{std::string("<b>")+_("Column Properties")+"</b>"};
    label_col.set_use_markup();
    label_col.set_halign(Gtk::Align::ALIGN_START);

    Gtk::Grid grid;
    grid.property_margin() = 6;
    grid.set_row_spacing(4);
    grid.set_column_spacing(8);
    grid.set_row_homogeneous(true);

    if (is_insert) {
        grid.attach(label_size,         0, 0, 2, 1);
        grid.attach(label_rows,         0, 1, 1, 1);
        grid.attach(spinbutton_rows,    1, 1, 1, 1);
        grid.attach(label_columns,      2, 1, 1, 1);
        grid.attach(spinbutton_columns, 3, 1, 1, 1);
    }
    grid.attach(label_col,             0, 2, 2, 1);
    grid.attach(label_col_width,       0, 3, 1, 1);
    grid.attach(spinbutton_col_width,  1, 3, 1, 1);

    auto checkbutton_is_light = Gtk::CheckButton(_("Lightweight (Much faster for large tables)"));
    checkbutton_is_light.set_active(is_light);
    auto checkbutton_table_ins_from_file = Gtk::CheckButton(_("Import from CSV File"));

    auto content_area = dialog.get_content_area();
    content_area->set_spacing(5);
    content_area->pack_start(grid);
    content_area->pack_start(checkbutton_is_light);
    if (is_insert) content_area->pack_start(checkbutton_table_ins_from_file);
    content_area->show_all();

    checkbutton_table_ins_from_file.signal_toggled().connect([&](){
        grid.set_sensitive(not checkbutton_table_ins_from_file.get_active());
    });

    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        if (GDK_KEY_Escape == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_REJECT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    const auto resp = dialog.run();
    if (Gtk::RESPONSE_ACCEPT == resp) {
        is_light = checkbutton_is_light.get_active();
        pCtConfig->tableRows = spinbutton_rows.get_value_as_int();
        pCtConfig->tableColumns = spinbutton_columns.get_value_as_int();
        pCtConfig->tableColWidthDefault = spinbutton_col_width.get_value_as_int();
        if (checkbutton_table_ins_from_file.get_active()) {
            return TableHandleResp::OkFromFile;
        }
        return TableHandleResp::Ok;
    }
    return TableHandleResp::Cancel;
}
