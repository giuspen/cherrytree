/*
 * sqlite3.cc
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

// https://sqlite.org/cintro.html
// http://zetcode.com/db/sqlitec
// g++ sqlite3.cc -o sqlite3 `pkg-config sqlite3 glibmm-2.4 --cflags --libs`

#include <assert.h>
#include <iostream>
#include <sqlite3.h>
#include <glibmm.h>


int on_sqlite3_exec_result_row(void *data, int num_columns, char **col_values, char **col_names)
{
    for(int i = 0; i < num_columns; i++)
    {
        std::cout << col_names[i] << "=" << (col_values[i] ? col_values[i] : "NULL") << ";";
    }
    std::cout << std::endl << "----" << std::endl;
    return 0; // invoke callback again for subsequent result rows
}


void parse_ctb(const Glib::ustring& filepath)
{
    std::cout << filepath << std::endl;
    sqlite3 *p_db;
    int ret_code = sqlite3_open(filepath.c_str(), &p_db);
    if(ret_code != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_open: " << sqlite3_errmsg(p_db) << std::endl;
    }
    else
    {
        char *p_err_msg = 0;
        ret_code = sqlite3_exec(p_db, "SELECT name,syntax,level FROM node", on_sqlite3_exec_result_row, 0, &p_err_msg);
        if(ret_code != SQLITE_OK)
        {
            std::cerr << "!! sqlite3_exec: " << p_err_msg << std::endl;
            sqlite3_free(p_err_msg);
        }
        else
        {

        }
    }
    sqlite3_close(p_db);
}


int main(int argc, char *argv[])
{
    std::locale::global(std::locale("")); // Set the global C++ locale to the user-specified locale
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " FILEPATH.CTB" << std::endl;
        return 1;
    }
    Glib::ustring filepath(argv[1]);
    assert(Glib::file_test(filepath, Glib::FILE_TEST_EXISTS));
    parse_ctb(filepath);
}
