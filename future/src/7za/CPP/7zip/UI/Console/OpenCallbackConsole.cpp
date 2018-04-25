// OpenCallbackConsole.cpp

#include "StdAfx.h"

#include "OpenCallbackConsole.h"

#include "ConsoleClose.h"
#include "UserInputUtils.h"

static HRESULT CheckBreak2()
{
  return NConsoleClose::TestBreakSignal() ? E_ABORT : S_OK;
}

HRESULT COpenCallbackConsole::Open_CheckBreak()
{
  return CheckBreak2();
}

HRESULT COpenCallbackConsole::Open_SetTotal(const UInt64 *files, const UInt64 *bytes)
{
  if (!MultiArcMode && NeedPercents())
  {
    if (files)
    {
      _totalFilesDefined = true;
      _percent.Total = *files;
    }
    else
      _totalFilesDefined = false;

    if (bytes)
    {
      _totalBytesDefined = true;
      if (!files)
        _percent.Total = *bytes;
    }
    else
      _totalBytesDefined = false;
  }

  return CheckBreak2();
}

HRESULT COpenCallbackConsole::Open_SetCompleted(const UInt64 *files, const UInt64 *bytes)
{
  if (!MultiArcMode && NeedPercents())
  {
    if (files)
    {
      _percent.Files = *files;
      if (_totalFilesDefined)
        _percent.Completed = *files;
    }

    if (bytes)
    {
      if (!_totalFilesDefined)
        _percent.Completed = *bytes;
    }
    _percent.Print();
  }

  return CheckBreak2();
}

HRESULT COpenCallbackConsole::Open_Finished()
{
  ClosePercents();
  return S_OK;
}

HRESULT COpenCallbackConsole::Open_CryptoGetTextPassword(BSTR *password)
{
  *password = NULL;
  RINOK(CheckBreak2());

  if (!PasswordIsDefined)
  {
    ClosePercents();
#ifndef _LIB_FOR_CHERRYTREE
    Password = GetPassword(_so);
#else
    printf("!! not supported\n");
    exit(1);
#endif // _LIB_FOR_CHERRYTREE
    PasswordIsDefined = true;
  }
  return StringToBstr(Password, password);
}
