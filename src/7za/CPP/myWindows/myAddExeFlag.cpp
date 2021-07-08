#include "StdAfx.h"

#include <glib/gstdio.h>

#include <windows.h>

#define NEED_NAME_WINDOWS_TO_UNIX
#include "myPrivate.h"

#include "Common/StringConvert.h"

void myAddExeFlag(const UString &u_name)
{
	AString filename = UnicodeStringToMultiByte(u_name, CP_ACP); // FIXME
	const char * name = nameWindowToUnix(filename);
	// printf("myAddExeFlag(%s)\n",name);
	g_chmod(name,0700);
}

