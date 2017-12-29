// ArchiveOpenCallback.h

#ifndef __ARCHIVE_OPEN_CALLBACK_H
#define __ARCHIVE_OPEN_CALLBACK_H

#include "../../../Common/MyCom.h"

#include "../../../Windows/FileFind.h"

#include "../../IPassword.h"
#include "../../Archive/IArchive.h"

#define INTERFACE_IOpenCallbackUI_Crypto(x) \
  virtual HRESULT Open_CryptoGetTextPassword(BSTR *password) x;

#define INTERFACE_IOpenCallbackUI(x) \
  virtual HRESULT Open_CheckBreak() x; \
  virtual HRESULT Open_SetTotal(const UInt64 *files, const UInt64 *bytes) x; \
  virtual HRESULT Open_SetCompleted(const UInt64 *files, const UInt64 *bytes) x; \
  virtual HRESULT Open_Finished() x; \
  INTERFACE_IOpenCallbackUI_Crypto(x)

struct IOpenCallbackUI
{
  INTERFACE_IOpenCallbackUI(=0)
};

class COpenCallbackImp :
  public IArchiveOpenCallback,
  public IArchiveOpenVolumeCallback,
  public IArchiveOpenSetSubArchiveName,
  public ICryptoGetTextPassword,
  public CMyUnknownImp
{
public:
  MY_QUERYINTERFACE_BEGIN2(IArchiveOpenVolumeCallback)
  MY_QUERYINTERFACE_ENTRY(IArchiveOpenSetSubArchiveName)
  MY_QUERYINTERFACE_ENTRY(ICryptoGetTextPassword)
  MY_QUERYINTERFACE_END
  MY_ADDREF_RELEASE

  INTERFACE_IArchiveOpenCallback(;)
  INTERFACE_IArchiveOpenVolumeCallback(;)

  STDMETHOD(CryptoGetTextPassword)(BSTR *password);

  STDMETHOD(SetSubArchiveName(const wchar_t *name))
  {
    _subArchiveMode = true;
    _subArchiveName = name;
    return S_OK;
  }

private:
  FString _folderPrefix;
  NWindows::NFile::NFind::CFileInfo _fileInfo;
  bool _subArchiveMode;
  UString _subArchiveName;

public:
  UStringVector FileNames;
  CBoolVector FileNames_WasUsed;
  CRecordVector<UInt64> FileSizes;
  
  bool PasswordWasAsked;

  IOpenCallbackUI *Callback;
  CMyComPtr<IArchiveOpenCallback> ReOpenCallback;

  COpenCallbackImp(): Callback(NULL), _subArchiveMode(false) {}
  
  void Init(const FString &folderPrefix, const FString &fileName)
  {
    _folderPrefix = folderPrefix;
    if (!_fileInfo.Find(_folderPrefix + fileName,true))
      throw 20121118;
    FileNames.Clear();
    FileNames_WasUsed.Clear();
    FileSizes.Clear();
    _subArchiveMode = false;
    PasswordWasAsked = false;
  }

  bool SetSecondFileInfo(CFSTR newName)
  {
    return _fileInfo.Find(newName) && !_fileInfo.IsDir();
  }
};

#endif
