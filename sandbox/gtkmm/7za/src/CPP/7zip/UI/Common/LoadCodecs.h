// LoadCodecs.h

#ifndef __LOAD_CODECS_H
#define __LOAD_CODECS_H

#include "../../../Common/MyBuffer.h"
#include "../../../Common/MyCom.h"
#include "../../../Common/MyString.h"
#include "../../../Common/ComTry.h"

#include "../../ICoder.h"

#include "../../Archive/IArchive.h"

struct CArcExtInfo
{
  UString Ext;
  UString AddExt;

  CArcExtInfo() {}
  CArcExtInfo(const UString &ext): Ext(ext) {}
  CArcExtInfo(const UString &ext, const UString &addExt): Ext(ext), AddExt(addExt) {}
};

struct CArcInfoEx
{
  UInt32 Flags;

  Func_CreateInArchive CreateInArchive;
  Func_IsArc IsArcFunc;

  UString Name;
  CObjectVector<CArcExtInfo> Exts;

  Func_CreateOutArchive CreateOutArchive;
  bool UpdateEnabled;
  bool NewInterface;
  UInt32 SignatureOffset;
  CObjectVector<CByteBuffer> Signatures;

  bool Flags_KeepName() const { return (Flags & NArcInfoFlags::kKeepName) != 0; }
  bool Flags_FindSignature() const { return (Flags & NArcInfoFlags::kFindSignature) != 0; }

  bool Flags_AltStreams() const { return (Flags & NArcInfoFlags::kAltStreams) != 0; }
  bool Flags_NtSecure() const { return (Flags & NArcInfoFlags::kNtSecure) != 0; }
  bool Flags_SymLinks() const { return (Flags & NArcInfoFlags::kSymLinks) != 0; }
  bool Flags_HardLinks() const { return (Flags & NArcInfoFlags::kHardLinks) != 0; }

  bool Flags_UseGlobalOffset() const { return (Flags & NArcInfoFlags::kUseGlobalOffset) != 0; }
  bool Flags_StartOpen() const { return (Flags & NArcInfoFlags::kStartOpen) != 0; }
  bool Flags_BackwardOpen() const { return (Flags & NArcInfoFlags::kBackwardOpen) != 0; }
  bool Flags_PreArc() const { return (Flags & NArcInfoFlags::kPreArc) != 0; }
  bool Flags_PureStartOpen() const { return (Flags & NArcInfoFlags::kPureStartOpen) != 0; }

  UString GetMainExt() const
  {
    if (Exts.IsEmpty())
      return UString();
    return Exts[0].Ext;
  }

  int FindExtension(const UString &ext) const;

  void AddExts(const UString &ext, const UString &addExt);

  bool IsSplit() const { return StringsAreEqualNoCase_Ascii(Name, "Split"); }

  CArcInfoEx():
      Flags(0),
      CreateInArchive(NULL),
      IsArcFunc(NULL),
      CreateOutArchive(NULL),
      UpdateEnabled(false),
      NewInterface(false),
      SignatureOffset(0)
  {}
};

class CCodecs :
  public IUnknown,
  public CMyUnknownImp
{
  CLASS_NO_COPY(CCodecs);
public:
  CObjectVector<CArcInfoEx> Formats;

  bool CaseSensitiveChange;
  bool CaseSensitive;

  CCodecs() :
      CaseSensitiveChange(false),
      CaseSensitive(false)
  {}

  ~CCodecs()
  {}

  const wchar_t *GetFormatNamePtr(int formatIndex) const
  {
    return formatIndex < 0 ? L"#" : (const wchar_t *)Formats[formatIndex].Name;
  }

  HRESULT Load();

  int FindFormatForArchiveName(const UString &arcPath) const;
  int FindFormatForExtension(const UString &ext) const;
  int FindFormatForArchiveType(const UString &arcType) const;
  bool FindFormatForArchiveType(const UString &arcType, CIntVector &formatIndices) const;

  MY_UNKNOWN_IMP

  HRESULT CreateInArchive(unsigned formatIndex, CMyComPtr<IInArchive> &archive) const
  {
    const CArcInfoEx &ai = Formats[formatIndex];
    {
      COM_TRY_BEGIN
      archive = ai.CreateInArchive();
      return S_OK;
      COM_TRY_END
    }
  }

  HRESULT CreateOutArchive(unsigned formatIndex, CMyComPtr<IOutArchive> &archive) const
  {
    const CArcInfoEx &ai = Formats[formatIndex];
    {
      COM_TRY_BEGIN
      archive = ai.CreateOutArchive();
      return S_OK;
      COM_TRY_END
    }
  }

  int FindOutFormatFromName(const UString &name) const
  {
    FOR_VECTOR (i, Formats)
    {
      const CArcInfoEx &arc = Formats[i];
      if (!arc.UpdateEnabled)
        continue;
      if (arc.Name.IsEqualTo_NoCase(name))
        return i;
    }
    return -1;
  }
};

#define CREATE_CODECS_OBJECT \
  CCodecs *codecs = new CCodecs; \
  CMyComPtr<IUnknown> __codecsRef = codecs;

#endif
