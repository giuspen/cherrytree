/*
 * main.cc
 * 
 * Copyright 2017 giuspen <giuspen@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <iostream>
#include <glibmm/i18n.h>
#include <gtkmm.h>

class CherryTree : public Gtk::Window
{
public:
    CherryTree();
    virtual ~CherryTree();

protected:
    void on_button_clicked();

    Gtk::Button m_button;
};

CherryTree::CherryTree() : m_button(_("Hello World"))
{
    set_border_width(10);

    m_button.signal_clicked().connect(sigc::mem_fun(*this, &CherryTree::on_button_clicked));

    add(m_button);

    m_button.show();
}

CherryTree::~CherryTree()
{
}

void CherryTree::on_button_clicked()
{
    std::cout << _("Hello World") << std::endl;
}

int main(int argc, char *argv[])
{
    bindtextdomain(GETTEXT_PACKAGE, CHERRYTREE_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    auto app = Gtk::Application::create(argc, argv, "com.giuspen.cherrytree");

    CherryTree cherrytree;

    return app->run(cherrytree);
}
