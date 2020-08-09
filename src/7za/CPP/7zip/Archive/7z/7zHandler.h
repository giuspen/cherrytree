// 7z/Handler.h

#ifndef __7Z_HANDLER_H
#define __7Z_HANDLER_H

#include "../../ICoder.h"
#include "../IArchive.h"

#include "../../Common/CreateCoder.h"

#include "../Common/HandlerOut.h"

#include "7zCompressionMode.h"
#include "7zIn.h"

namespace NArchive {
namespace N7z {

#ifndef __7Z_SET_PROPERTIES
#define __7Z_SET_PROPERTIES
#endif

class COutHandler: public CMultiMethodProps
{
  HRESULT SetSolidFromString(const UString &s);
  HRESULT SetSolidFromPROPVARIANT(const PROPVARIANT &value);
public:
  bool _removeSfxBlock;

  UInt64 _numSolidFiles;
  UInt64 _numSolidBytes;
  bool _numSolidBytesDefined;
  bool _solidExtension;
  bool _useTypeSorting;

  bool _compressHeaders;
  bool _encryptHeadersSpecified;
  bool _encryptHeaders;
  // bool _useParents; 9.26

  CBoolPair Write_CTime;
  CBoolPair Write_ATime;
  CBoolPair Write_MTime;

  bool _useMultiThreadMixer;

  void InitSolidFiles() { _numSolidFiles = (UInt64)(Int64)(-1); }
  void InitSolidSize()  { _numSolidBytes = (UInt64)(Int64)(-1); }
  void InitSolid()
  {
    InitSolidFiles();
    InitSolidSize();
    _solidExtension = false;
    _numSolidBytesDefined = false;
  }

  void InitProps();

  COutHandler() { InitProps(); }

  HRESULT SetProperty(const wchar_t *name, const PROPVARIANT &value);
};

class CHandler :
  public IInArchive,
  public IArchiveGetRawProps,
  #ifdef __7Z_SET_PROPERTIES
  public ISetProperties,
  #endif
  public IOutArchive,
  PUBLIC_ISetCompressCodecsInfo
  public CMyUnknownImp,
  public COutHandler
{
public:
  MY_QUERYINTERFACE_BEGIN2(IInArchive)
  MY_QUERYINTERFACE_ENTRY(IArchiveGetRawProps)
  #ifdef __7Z_SET_PROPERTIES
  MY_QUERYINTERFACE_ENTRY(ISetProperties)
  #endif
  MY_QUERYINTERFACE_ENTRY(IOutArchive)
  QUERY_ENTRY_ISetCompressCodecsInfo
  MY_QUERYINTERFACE_END
  MY_ADDREF_RELEASE

  INTERFACE_IInArchive(;)
  INTERFACE_IArchiveGetRawProps(;)

  #ifdef __7Z_SET_PROPERTIES
  STDMETHOD(SetProperties)(const wchar_t * const *names, const PROPVARIANT *values, UInt32 numProps);
  #endif

  INTERFACE_IOutArchive(;)

  DECL_ISetCompressCodecsInfo

  CHandler();

private:
  CMyComPtr<IInStream> _inStream;
  NArchive::N7z::CDbEx _db;

  bool _isEncrypted;
  bool _passwordIsDefined;
  UString _password;

  CRecordVector<CBond2> _bonds;

  HRESULT PropsMethod_To_FullMethod(CMethodFull &dest, const COneMethodInfo &m);
  HRESULT SetHeaderMethod(CCompressionMethodMode &headerMethod);
  HRESULT SetMainMethod(CCompressionMethodMode &method, UInt32 numThreads);

  bool IsFolderEncrypted(CNum folderIndex) const;

  CRecordVector<UInt64> _fileInfoPopIDs;
  void FillPopIDs();
  void AddMethodName(AString &s, UInt64 id);
  HRESULT SetMethodToProp(CNum folderIndex, PROPVARIANT *prop) const;

  DECL_EXTERNAL_CODECS_VARS
};

}}

#endif
