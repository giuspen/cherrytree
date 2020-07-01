// ExtractingFilePath.cpp

#include "StdAfx.h"

#include "../../../Common/Wildcard.h"

#include "../../../Windows/FileName.h"

#include "ExtractingFilePath.h"

static void ReplaceIncorrectChars(UString &s)
{
  {
    for (unsigned i = 0; i < s.Len(); i++)
    {
      wchar_t c = s[i];
      if (
          c == WCHAR_PATH_SEPARATOR)
        s.ReplaceOneCharAtPos(i, '_');
    }
  }
}

#if 1

void Correct_AltStream_Name(UString &s)
{
  unsigned len = s.Len();
  const unsigned kPostfixSize = 6;
  if (s.Len() >= kPostfixSize
      && StringsAreEqualNoCase_Ascii(s.RightPtr(kPostfixSize), ":$DATA"))
    len -= kPostfixSize;
  for (unsigned i = 0; i < len; i++)
  {
    wchar_t c = s[i];
    if (c == ':' || c == '\\' || c == '/'
        || c == 0x202E // RLO
        )
      s.ReplaceOneCharAtPos(i, '_');
  }
  if (s.IsEmpty())
    s = L'_';
}

static const unsigned g_ReservedWithNum_Index = 4;

static const char * const g_ReservedNames[] =
{
  "CON", "PRN", "AUX", "NUL",
  "COM", "LPT"
};

static bool IsSupportedName(const UString &name)
{
  for (unsigned i = 0; i < ARRAY_SIZE(g_ReservedNames); i++)
  {
    const char *reservedName = g_ReservedNames[i];
    unsigned len = MyStringLen(reservedName);
    if (name.Len() < len)
      continue;
    if (!name.IsPrefixedBy_Ascii_NoCase(reservedName))
      continue;
    if (i >= g_ReservedWithNum_Index)
    {
      wchar_t c = name[len];
      if (c < L'0' || c > L'9')
        continue;
      len++;
    }
    for (;;)
    {
      wchar_t c = name[len++];
      if (c == 0 || c == '.')
        return false;
      if (c != ' ')
        break;
    }
  }
  return true;
}

static void CorrectUnsupportedName(UString &name)
{
  if (!IsSupportedName(name))
    name.InsertAtFront(L'_');
}

#endif

static void Correct_PathPart(UString &s)
{
  // "." and ".."
  if (s[0] == '.' && (s[1] == 0 || (s[1] == '.' && s[2] == 0)))
    s.Empty();
}

// static const wchar_t *k_EmptyReplaceName = L"[]";
static const wchar_t k_EmptyReplaceName = L'_';

UString Get_Correct_FsFile_Name(const UString &name)
{
  UString res = name;
  Correct_PathPart(res);
  
  if (res.IsEmpty())
    res = k_EmptyReplaceName;
  return res;
}


void Correct_FsPath(bool absIsAllowed, UStringVector &parts, bool isDir)
{
  unsigned i = 0;

  if (absIsAllowed)
  {
    if (parts[0].IsEmpty())
    {
      i = 1;
    }
  }

  for (; i < parts.Size();)
  {
    UString &s = parts[i];

    #if 1
    Correct_PathPart(s);
    #endif

    if (s.IsEmpty())
    {
      if (isDir || i != parts.Size() - 1)
      {
        parts.Delete(i);
        continue;
      }
      s = k_EmptyReplaceName;
    }
    else
    {
    }
    
    i++;
  }

  if (!isDir)
  {
    if (parts.IsEmpty())
      parts.Add(k_EmptyReplaceName);
    else
    {
      UString &s = parts.Back();
      if (s.IsEmpty())
        s = k_EmptyReplaceName;
    }
  }
}

UString MakePathFromParts(const UStringVector &parts)
{
  UString s;
  FOR_VECTOR (i, parts)
  {
    if (i != 0)
      s.Add_PathSepar();
    s += parts[i];
  }
  return s;
}
