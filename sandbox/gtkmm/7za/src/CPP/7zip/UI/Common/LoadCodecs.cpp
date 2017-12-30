// LoadCodecs.cpp

/*
EXTERNAL_CODECS
---------------
  CCodecs::Load() tries to detect the directory with plugins.
  It stops the checking, if it can find any of the following items:
    - 7z.dll file
    - "Formats" subdir
    - "Codecs"  subdir
  The order of check:
    1) directory of client executable
    2) WIN32: directory for REGISTRY item [HKEY_*\Software\7-Zip\Path**]
       The order for HKEY_* : Path** :
         - HKEY_CURRENT_USER  : PathXX
         - HKEY_LOCAL_MACHINE : PathXX
         - HKEY_CURRENT_USER  : Path
         - HKEY_LOCAL_MACHINE : Path
       PathXX is Path32 in 32-bit code
       PathXX is Path64 in 64-bit code


EXPORT_CODECS
-------------
  if (EXTERNAL_CODECS) is defined, then the code exports internal
  codecs of client from CCodecs object to external plugins.
  7-Zip doesn't use that feature. 7-Zip uses the scheme:
    - client application without internal plugins.
    - 7z.dll module contains all (or almost all) plugins.
      7z.dll can use codecs from another plugins, if required.
*/


#include "StdAfx.h"

#include "../../../../C/7zVersion.h"

#include "../../../Common/MyCom.h"
#include "../../../Common/StringToInt.h"
#include "../../../Common/StringConvert.h"

#include "../../../Windows/PropVariant.h"

#include "LoadCodecs.h"

using namespace NWindows;

#include "../../ICoder.h"
#include "../../Common/RegisterArc.h"

static const unsigned kNumArcsMax = 64;
static unsigned g_NumArcs = 0;
static const CArcInfo *g_Arcs[kNumArcsMax];

void RegisterArc(const CArcInfo *arcInfo) throw()
{
  if (g_NumArcs < kNumArcsMax)
  {
    g_Arcs[g_NumArcs] = arcInfo;
    g_NumArcs++;
  }
}

static void SplitString(const UString &srcString, UStringVector &destStrings)
{
  destStrings.Clear();
  UString s;
  unsigned len = srcString.Len();
  if (len == 0)
    return;
  for (unsigned i = 0; i < len; i++)
  {
    wchar_t c = srcString[i];
    if (c == L' ')
    {
      if (!s.IsEmpty())
      {
        destStrings.Add(s);
        s.Empty();
      }
    }
    else
      s += c;
  }
  if (!s.IsEmpty())
    destStrings.Add(s);
}

int CArcInfoEx::FindExtension(const UString &ext) const
{
  FOR_VECTOR (i, Exts)
    if (ext.IsEqualTo_NoCase(Exts[i].Ext))
      return i;
  return -1;
}

void CArcInfoEx::AddExts(const UString &ext, const UString &addExt)
{
  UStringVector exts, addExts;
  SplitString(ext, exts);
  SplitString(addExt, addExts);
  FOR_VECTOR (i, exts)
  {
    CArcExtInfo extInfo;
    extInfo.Ext = exts[i];
    if (i < addExts.Size())
    {
      extInfo.AddExt = addExts[i];
      if (extInfo.AddExt == L"*")
        extInfo.AddExt.Empty();
    }
    Exts.Add(extInfo);
  }
}

static bool ParseSignatures(const Byte *data, unsigned size, CObjectVector<CByteBuffer> &signatures)
{
  signatures.Clear();
  while (size > 0)
  {
    unsigned len = *data++;
    size--;
    if (len > size)
      return false;
    signatures.AddNew().CopyFrom(data, len);
    data += len;
    size -= len;
  }
  return true;
}

HRESULT CCodecs::Load()
{
  Formats.Clear();

  for (UInt32 i = 0; i < g_NumArcs; i++)
  {
    const CArcInfo &arc = *g_Arcs[i];
    CArcInfoEx item;

    item.Name.SetFromAscii(arc.Name);
    item.CreateInArchive = arc.CreateInArchive;
    item.IsArcFunc = arc.IsArc;
    item.Flags = arc.Flags;

    {
      UString e, ae;
      if (arc.Ext)
        e.SetFromAscii(arc.Ext);
      if (arc.AddExt)
        ae.SetFromAscii(arc.AddExt);
      item.AddExts(e, ae);
    }

    item.CreateOutArchive = arc.CreateOutArchive;
    item.UpdateEnabled = (arc.CreateOutArchive != NULL);
    item.SignatureOffset = arc.SignatureOffset;
    item.NewInterface = true;

    if (arc.IsMultiSignature())
      ParseSignatures(arc.Signature, arc.SignatureSize, item.Signatures);
    else
      item.Signatures.AddNew().CopyFrom(arc.Signature, arc.SignatureSize);

    Formats.Add(item);
  }
  return S_OK;
}

int CCodecs::FindFormatForArchiveName(const UString &arcPath) const
{
  int dotPos = arcPath.ReverseFind_Dot();
  if (dotPos <= arcPath.ReverseFind_PathSepar())
    return -1;
  const UString ext = arcPath.Ptr(dotPos + 1);
  if (ext.IsEmpty())
    return -1;
  if (ext.IsEqualTo_Ascii_NoCase("exe"))
    return -1;
  FOR_VECTOR (i, Formats)
  {
    const CArcInfoEx &arc = Formats[i];
    if (arc.FindExtension(ext) >= 0)
      return i;
  }
  return -1;
}

int CCodecs::FindFormatForExtension(const UString &ext) const
{
  if (ext.IsEmpty())
    return -1;
  FOR_VECTOR (i, Formats)
    if (Formats[i].FindExtension(ext) >= 0)
      return i;
  return -1;
}

int CCodecs::FindFormatForArchiveType(const UString &arcType) const
{
  FOR_VECTOR (i, Formats)
    if (Formats[i].Name.IsEqualTo_NoCase(arcType))
      return i;
  return -1;
}

bool CCodecs::FindFormatForArchiveType(const UString &arcType, CIntVector &formatIndices) const
{
  formatIndices.Clear();
  for (unsigned pos = 0; pos < arcType.Len();)
  {
    int pos2 = arcType.Find(L'.', pos);
    if (pos2 < 0)
      pos2 = arcType.Len();
    const UString name = arcType.Mid(pos, pos2 - pos);
    if (name.IsEmpty())
      return false;
    int index = FindFormatForArchiveType(name);
    if (index < 0 && name != L"*")
    {
      formatIndices.Clear();
      return false;
    }
    formatIndices.Add(index);
    pos = pos2 + 1;
  }
  return true;
}
