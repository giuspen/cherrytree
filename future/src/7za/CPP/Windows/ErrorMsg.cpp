// Windows/ErrorMsg.h

#include "StdAfx.h"

#include "Windows/ErrorMsg.h"
#include "Common/StringConvert.h"

namespace NWindows {
namespace NError {

UString MyFormatMessage(DWORD errorCode)
{
  const char * txt = 0;
  AString msg;

  switch((HRESULT)errorCode) {
    case ERROR_NO_MORE_FILES   : txt = "No more files"; break ;
    case E_NOTIMPL             : txt = "E_NOTIMPL"; break ;
    case E_NOINTERFACE         : txt = "E_NOINTERFACE"; break ;
    case E_ABORT               : txt = "E_ABORT"; break ;
    case E_FAIL                : txt = "E_FAIL"; break ;
    case STG_E_INVALIDFUNCTION : txt = "STG_E_INVALIDFUNCTION"; break ;
    case E_OUTOFMEMORY         : txt = "E_OUTOFMEMORY"; break ;
    case E_INVALIDARG          : txt = "E_INVALIDARG"; break ;
    case ERROR_DIRECTORY          : txt = "Error Directory"; break ;
    default:
      txt = strerror(errorCode);
  }
  if (txt) {
    msg = txt;
  } else {
    char msgBuf[256];
    snprintf(msgBuf,sizeof(msgBuf),"error #%x",(unsigned)errorCode);
    msgBuf[sizeof(msgBuf)-1] = 0;
    msg = msgBuf;
  }
  return MultiByteToUnicodeString(msg);
}

} // NError
} // NWindows
