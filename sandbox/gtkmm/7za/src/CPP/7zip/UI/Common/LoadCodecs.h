// LoadCodecs.h

#ifndef __LOAD_CODECS_H
#define __LOAD_CODECS_H

/*
Client application uses LoadCodecs.* to load plugins to
CCodecs object, that contains 3 lists of plugins:
  1) Formats - internal and external archive handlers
  2) Codecs  - external codecs
  3) Hashers - external hashers

EXTERNAL_CODECS
---------------

  if EXTERNAL_CODECS is defined, then the code tries to load external
  plugins from DLL files (shared libraries).

  There are two types of executables in 7-Zip:

  1) Executable that uses external plugins must be compiled
     with EXTERNAL_CODECS defined:
       - 7z.exe, 7zG.exe, 7zFM.exe

     Note: EXTERNAL_CODECS is used also in CPP/7zip/Common/CreateCoder.h
           that code is used in plugin module (7z.dll).

  2) Standalone modules are compiled without EXTERNAL_CODECS:
    - SFX modules: 7z.sfx, 7zCon.sfx
    - standalone versions of console 7-Zip: 7za.exe, 7zr.exe

  if EXTERNAL_CODECS is defined, CCodecs class implements interfaces:
    - ICompressCodecsInfo : for Codecs
    - IHashers            : for Hashers

  The client application can send CCodecs object to each plugin module.
  And plugin module can use ICompressCodecsInfo or IHashers interface to access
  another plugins.

  There are 2 ways to send (ICompressCodecsInfo * compressCodecsInfo) to plugin
    1) for old versions:
        a) request ISetCompressCodecsInfo from created archive handler.
        b) call ISetCompressCodecsInfo::SetCompressCodecsInfo(compressCodecsInfo)
    2) for new versions:
        a) request "SetCodecs" function from DLL file
        b) call SetCodecs(compressCodecsInfo) function from DLL file
*/

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
      IsArcFunc(NULL)
      , CreateOutArchive(NULL)
      , UpdateEnabled(false)
      , NewInterface(false)
      , SignatureOffset(0)
  {}
};

class CCodecs:
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
  {
    // OutputDebugStringA("~CCodecs");
  }

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
