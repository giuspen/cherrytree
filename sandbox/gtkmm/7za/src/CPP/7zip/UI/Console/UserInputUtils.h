// UserInputUtils.h

#ifndef __USER_INPUT_UTILS_H
#define __USER_INPUT_UTILS_H

#include "../../../Common/StdOutStream.h"

namespace NUserAnswerMode {

enum EEnum
{
  kYes,
  kNo,
  kYesAll,
  kNoAll,
  kAutoRenameAll,
  kQuit
};
}

#ifndef _LIB_FOR_CHERRYTREE
NUserAnswerMode::EEnum ScanUserYesNoAllQuit(CStdOutStream *outStream);
UString GetPassword(CStdOutStream *outStream,bool verify = false);
#endif // _LIB_FOR_CHERRYTREE

#endif
