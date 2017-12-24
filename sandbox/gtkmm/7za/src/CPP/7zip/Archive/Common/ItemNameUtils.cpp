// Archive/Common/ItemNameUtils.cpp

#include "StdAfx.h"

#include "ItemNameUtils.h"

namespace NArchive {
namespace NItemName {

static const wchar_t kOSDirDelimiter = WCHAR_PATH_SEPARATOR;
static const wchar_t kDirDelimiter = L'/';

void ReplaceToOsPathSeparator(wchar_t *s)
{
}

UString MakeLegalName(const UString &name)
{
  UString zipName = name;
  zipName.Replace(kOSDirDelimiter, kDirDelimiter);
  return zipName;
}

UString GetOSName(const UString &name)
{
  UString newName = name;
  newName.Replace(kDirDelimiter, kOSDirDelimiter);
  return newName;
}

UString GetOSName2(const UString &name)
{
  if (name.IsEmpty())
    return UString();
  UString newName = GetOSName(name);
  if (newName.Back() == kOSDirDelimiter)
    newName.DeleteBack();
  return newName;
}

void ConvertToOSName2(UString &name)
{
  if (!name.IsEmpty())
  {
    name.Replace(kDirDelimiter, kOSDirDelimiter);
    if (name.Back() == kOSDirDelimiter)
      name.DeleteBack();
  }
}

bool HasTailSlash(const AString &name, UINT
  )
{
  if (name.IsEmpty())
    return false;
  LPCSTR prev =
    (LPCSTR)(name) + (name.Len() - 1);
  return (*prev == '/');
}

UString WinNameToOSName(const UString &name)
{
  UString newName = name;
  newName.Replace(L'\\', kOSDirDelimiter);
  return newName;
}

}}
