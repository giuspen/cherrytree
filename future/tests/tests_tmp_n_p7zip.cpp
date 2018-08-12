/*
 * tests_tmp_n_p7zip.cc
 * 
 * Copyright 2018 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_app.h"
#include "CppUTest/CommandLineTestRunner.h"


TEST_GROUP(TmpP7zipGroup)
{
};

TEST(TmpP7zipGroup, CTTmp_misc)
{
    CTTmp* pCTmp = new CTTmp();
    CHECK(g_file_test(pCTmp->getRootDirpath(), G_FILE_TEST_IS_DIR));
    delete pCTmp;
    CHECK(!g_file_test(pCTmp->getRootDirpath(), G_FILE_TEST_IS_DIR));
}
