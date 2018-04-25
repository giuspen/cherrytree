// CommandLineParser.cpp

#include "StdAfx.h"

#include "CommandLineParser.h"

static bool IsString1PrefixedByString2_NoCase(const wchar_t *u, const char *a)
{
  for (;;)
  {
    char c = *a;
    if (c == 0)
      return true;
    if ((unsigned char)MyCharLower_Ascii(c) != MyCharLower_Ascii(*u))
      return false;
    a++;
    u++;
  }
}

namespace NCommandLineParser {

static const char *kStopSwitchParsing = "--";

static bool inline IsItSwitchChar(wchar_t c)
{
  return (c == '-');
}

CParser::CParser(unsigned numSwitches):
  _numSwitches(numSwitches),
  _switches(0)
{
  _switches = new CSwitchResult[numSwitches];
}

CParser::~CParser()
{
  delete []_switches;
}


// if (s) contains switch then function updates switch structures
// out: true, if (s) is a switch
bool CParser::ParseString(const UString &s, const CSwitchForm *switchForms)
{
  if (s.IsEmpty() || !IsItSwitchChar(s[0]))
    return false;

  unsigned pos = 1;
  unsigned switchIndex = 0;
  int maxLen = -1;
  
  for (unsigned i = 0; i < _numSwitches; i++)
  {
    const char *key = switchForms[i].Key;
    unsigned switchLen = MyStringLen(key);
    if ((int)switchLen <= maxLen || pos + switchLen > s.Len())
      continue;
    if (IsString1PrefixedByString2_NoCase((const wchar_t *)s + pos, key))
    {
      switchIndex = i;
      maxLen = switchLen;
    }
  }

  if (maxLen < 0)
  {
    ErrorMessage = "Unknown switch:";
    return false;
  }

  pos += maxLen;
  
  CSwitchResult &sw = _switches[switchIndex];
  const CSwitchForm &form = switchForms[switchIndex];
  
  if (!form.Multi && sw.ThereIs)
  {
    ErrorMessage = "Multiple instances for switch:";
    return false;
  }

  sw.ThereIs = true;

  int rem = s.Len() - pos;
  if (rem < form.MinLen)
  {
    ErrorMessage = "Too short switch:";
    return false;
  }
  
  sw.WithMinus = false;
  sw.PostCharIndex = -1;
  
  switch (form.Type)
  {
    case NSwitchType::kMinus:
      if (rem == 1)
      {
        sw.WithMinus = (s[pos] == '-');
        if (sw.WithMinus)
          return true;
        ErrorMessage = "Incorrect switch postfix:";
        return false;
      }
      break;
      
    case NSwitchType::kChar:
      if (rem == 1)
      {
        wchar_t c = s[pos];
        if (c <= 0x7F)
        {
          sw.PostCharIndex = FindCharPosInString(form.PostCharSet, (char)c);
          if (sw.PostCharIndex >= 0)
            return true;
        }
        ErrorMessage = "Incorrect switch postfix:";
        return false;
      }
      break;
      
    case NSwitchType::kString:
      sw.PostStrings.Add((const wchar_t *)s + pos);
      return true;
  }

  if (pos != s.Len())
  {
    ErrorMessage = "Too long switch:";
    return false;
  }
  return true;
}

bool CParser::ParseStrings(const CSwitchForm *switchForms, const UStringVector &commandStrings)
{
  ErrorLine.Empty();
  bool stopSwitch = false;
  FOR_VECTOR (i, commandStrings)
  {
    const UString &s = commandStrings[i];
    if (!stopSwitch)
    {
      if (s.IsEqualTo(kStopSwitchParsing))
      {
        stopSwitch = true;
        continue;
      }
      if (!s.IsEmpty() && IsItSwitchChar(s[0]))
      {
        if (ParseString(s, switchForms))
          continue;
        ErrorLine = s;
        return false;
      }
    }
    NonSwitchStrings.Add(s);
  }
  return true;
}

}
