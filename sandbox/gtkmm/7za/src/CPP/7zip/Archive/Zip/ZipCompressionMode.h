// CompressionMode.h

#ifndef __ZIP_COMPRESSION_MODE_H
#define __ZIP_COMPRESSION_MODE_H

#include "../../../Common/MyString.h"

#include "../../../Windows/System.h"

#include "../Common/HandlerOut.h"

namespace NArchive {
namespace NZip {

struct CBaseProps
{
  CMethodProps MethodInfo;
  Int32 Level;

  UInt32 NumThreads;
  bool NumThreadsWasChanged;
  bool IsAesMode;
  Byte AesKeyMode;

  void Init()
  {
    MethodInfo.Clear();
    Level = -1;
    NumThreads = NWindows::NSystem::GetNumberOfProcessors();;
    NumThreadsWasChanged = false;
    IsAesMode = false;
    AesKeyMode = 3;
  }
};

struct CCompressionMethodMode: public CBaseProps
{
  CRecordVector<Byte> MethodSequence;
  bool PasswordIsDefined;
  AString Password;

  UInt64 _dataSizeReduce;
  bool _dataSizeReduceDefined;
  
  bool IsRealAesMode() const { return PasswordIsDefined && IsAesMode; }

  CCompressionMethodMode(): PasswordIsDefined(false)
  {
    _dataSizeReduceDefined = false;
    _dataSizeReduce = 0;
  }
};

}}

#endif
