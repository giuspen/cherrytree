// Main.cpp

#include "StdAfx.h"

#include "../../../Common/MyWindows.h"

#include "myPrivate.h"

#include "../../../../C/CpuArch.h"

#if defined( _7ZIP_LARGE_PAGES)
#include "../../../../C/Alloc.h"
#endif

#include "../../../Common/MyInitGuid.h"

#include "../../../Common/CommandLineParser.h"
#include "../../../Common/IntToString.h"
#include "../../../Common/MyException.h"
#include "../../../Common/StringConvert.h"
#include "../../../Common/StringToInt.h"
#include "../../../Common/UTFConvert.h"

#include "../../../Windows/ErrorMsg.h"

#include "../../../Windows/TimeUtils.h"

#include "../Common/ArchiveCommandLine.h"
#include "../Common/ExitCode.h"
#include "../Common/Extract.h"

#include "../../Common/RegisterCodec.h"

#include "BenchCon.h"
#include "ConsoleClose.h"
#include "ExtractCallbackConsole.h"
#include "List.h"
#include "OpenCallbackConsole.h"
#include "UpdateCallbackConsole.h"

#include "HashCon.h"

#ifdef PROG_VARIANT_R
#include "../../../../C/7zVersion.h"
#else
#include "../../MyVersion.h"
#endif

using namespace NWindows;
using namespace NFile;
using namespace NCommandLineParser;

extern CStdOutStream *g_StdStream;
extern CStdOutStream *g_ErrStream;

extern unsigned g_NumCodecs;
extern const CCodecInfo *g_Codecs[];

extern unsigned g_NumHashers;
extern const CHasherInfo *g_Hashers[];

static const char *kCopyrightString = "\n7-Zip"
#ifdef PROG_VARIANT_R
" (r)"
#else
" (a)"
#endif

#ifdef MY_CPU_64BIT
" [64]"
#elif defined MY_CPU_32BIT
" [32]"
#endif

" " MY_VERSION_COPYRIGHT_DATE "\n";

static const char *kHelpString =
    "Usage: 7z"
#ifdef PROG_VARIANT_R
    "r"
#else
    "a"
#endif
    " <command> [<switches>...] <archive_name> [<file_names>...]\n"
    "       [<@listfiles...>]\n"
    "\n"
    "<Commands>\n"
    "  a : Add files to archive\n"
    "  b : Benchmark\n"
    "  d : Delete files from archive\n"
    "  e : Extract files from archive (without using directory names)\n"
    "  h : Calculate hash values for files\n"
    "  i : Show information about supported formats\n"
    "  l : List contents of archive\n"
    "  rn : Rename files in archive\n"
    "  t : Test integrity of archive\n"
    "  u : Update files to archive\n"
    "  x : eXtract files with full paths\n"
    "\n"
    "<Switches>\n"
    "  -- : Stop switches parsing\n"
    "  -ai[r[-|0]]{@listfile|!wildcard} : Include archives\n"
    "  -ax[r[-|0]]{@listfile|!wildcard} : eXclude archives\n"
    "  -ao{a|s|t|u} : set Overwrite mode\n"
    "  -an : disable archive_name field\n"
    "  -bb[0-3] : set output log level\n"
    "  -bd : disable progress indicator\n"
    "  -bs{o|e|p}{0|1|2} : set output stream for output/error/progress line\n"
    "  -bt : show execution time statistics\n"
    "  -i[r[-|0]]{@listfile|!wildcard} : Include filenames\n"
    "  -m{Parameters} : set compression Method\n"
    "    -mmt[N] : set number of CPU threads\n"
    "  -o{Directory} : set Output directory\n"
    "  -p{Password} : set Password\n"
    "  -r[-|0] : Recurse subdirectories\n"
    "  -sa{a|e|s} : set Archive name mode\n"
    "  -scc{UTF-8|WIN|DOS} : set charset for for console input/output\n"
    "  -scs{UTF-8|UTF-16LE|UTF-16BE|WIN|DOS|{id}} : set charset for list files\n"
    "  -scrc[CRC32|CRC64|SHA1|SHA256|*] : set hash function for x, e, h commands\n"
    "  -sdel : delete files after compression\n"
    "  -seml[.] : send archive by email\n"
    "  -sfx[{name}] : Create SFX archive\n"
    "  -si[{name}] : read data from stdin\n"
    "  -slp : set Large Pages mode\n"
    "  -slt : show technical information for l (List) command\n"
    "  -snh : store hard links as links\n"
    "  -snl : store symbolic links as links\n"
    "  -sni : store NT security information\n"
    "  -sns[-] : store NTFS alternate streams\n"
    "  -so : write data to stdout\n"
    "  -spd : disable wildcard matching for file names\n"
    "  -spe : eliminate duplication of root folder for extract command\n"
    "  -spf : use fully qualified file paths\n"
    "  -ssc[-] : set sensitive case mode\n"
    "  -ssw : compress shared files\n"
    "  -stl : set archive timestamp from the most recently modified file\n"
    "  -stm{HexMask} : set CPU thread affinity mask (hexadecimal number)\n"
    "  -stx{Type} : exclude archive type\n"
    "  -t{Type} : Set type of archive\n"
    "  -u[-][p#][q#][r#][x#][y#][z#][!newArchiveName] : Update options\n"
    "  -v{Size}[b|k|m|g] : Create volumes\n"
    "  -w[{path}] : assign Work directory. Empty path means a temporary directory\n"
    "  -x[r[-|0]]{@listfile|!wildcard} : eXclude filenames\n"
    "  -y : assume Yes on all queries\n";

// ---------------------------
// exception messages

static const char *kEverythingIsOk = "Everything is Ok";
static const char *kUserErrorMessage = "Incorrect command line";
static const char *kNoFormats = "7-Zip cannot find the code that works with archives.";
static const char *kUnsupportedArcTypeMessage = "Unsupported archive type";

static CFSTR kDefaultSfxModule = FTEXT("7zCon.sfx");

static void ShowMessageAndThrowException(LPCSTR message, NExitCode::EEnum code)
{
  if (g_ErrStream)
    *g_ErrStream << endl << "ERROR: " << message << endl;
  throw code;
}

static void ShowCopyrightAndHelp(CStdOutStream *so, bool needHelp)
{
  if (!so)
    return;
  *so << kCopyrightString;

  showP7zipInfo(so);

  if (needHelp)
    *so << kHelpString;
}

static void PrintStringRight(CStdOutStream &so, const AString &s, unsigned size)
{
  unsigned len = s.Len();
  for (unsigned i = len; i < size; i++)
    so << ' ';
  so << s;
}

static void PrintUInt32(CStdOutStream &so, UInt32 val, unsigned size)
{
  char s[16];
  ConvertUInt32ToString(val, s);
  PrintStringRight(so, s, size);
}

static void PrintLibIndex(CStdOutStream &so, int libIndex)
{
  if (libIndex >= 0)
    PrintUInt32(so, libIndex, 2);
  else
    so << "  ";
  so << ' ';
}

static void PrintString(CStdOutStream &so, const UString &s, unsigned size)
{
  unsigned len = s.Len();
  so << s;
  for (unsigned i = len; i < size; i++)
    so << ' ';
}

static inline char GetHex(unsigned val)
{
  return (char)((val < 10) ? ('0' + val) : ('A' + (val - 10)));
}

static void PrintWarningsPaths(const CErrorPathCodes &pc, CStdOutStream &so)
{
  FOR_VECTOR(i, pc.Paths)
  {
    so << pc.Paths[i] << " : ";
    so << NError::MyFormatMessage(pc.Codes[i]) << endl;
  }
  so << "----------------" << endl;
}

static int WarningsCheck(HRESULT result, const CCallbackConsoleBase &callback,
    const CUpdateErrorInfo &errorInfo,
    CStdOutStream *so,
    CStdOutStream *se,
    bool showHeaders)
{
  int exitCode = NExitCode::kSuccess;

  if (callback.ScanErrors.Paths.Size() != 0)
  {
    if (se)
    {
      *se << endl;
      *se << "Scan WARNINGS for files and folders:" << endl << endl;
      PrintWarningsPaths(callback.ScanErrors, *se);
      *se << "Scan WARNINGS: " << callback.ScanErrors.Paths.Size();
      *se << endl;
    }
    exitCode = NExitCode::kWarning;
  }

  if (result != S_OK || errorInfo.ThereIsError())
  {
    if (se)
    {
      UString message;
      if (!errorInfo.Message.IsEmpty())
      {
        message.AddAscii(errorInfo.Message);
        message.Add_LF();
      }
      {
        FOR_VECTOR(i, errorInfo.FileNames)
        {
          message += fs2us(errorInfo.FileNames[i]);
          message.Add_LF();
        }
      }
      if (errorInfo.SystemError != 0)
      {
        message += NError::MyFormatMessage(errorInfo.SystemError);
        message.Add_LF();
      }
      if (!message.IsEmpty())
        *se << L"\nError:\n" << message;
    }

    return NExitCode::kFatalError;
  }

  unsigned numErrors = callback.FailedFiles.Paths.Size();
  if (numErrors == 0)
  {
    if (showHeaders)
      if (callback.ScanErrors.Paths.Size() == 0)
        if (so)
        {
          if (se)
            se->Flush();
          *so << kEverythingIsOk << endl;
        }
  }
  else
  {
    if (se)
    {
      *se << endl;
      *se << "WARNINGS for files:" << endl << endl;
      PrintWarningsPaths(callback.FailedFiles, *se);
      *se << "WARNING: Cannot open " << numErrors << " file";
      if (numErrors > 1)
        *se << 's';
      *se << endl;
    }
    exitCode = NExitCode::kWarning;
  }

  return exitCode;
}

static void ThrowException_if_Error(HRESULT res)
{
  if (res != S_OK)
    throw CSystemException(res);
}

static void PrintNum(UInt64 val, unsigned numDigits, char c = ' ')
{
  char temp[64];
  char *p = temp + 32;
  ConvertUInt64ToString(val, p);
  unsigned len = MyStringLen(p);
  for (; len < numDigits; len++)
    *--p = c;
  *g_StdStream << p;
}

static void PrintTime(const char *s, UInt64 val, UInt64 total)
{
  *g_StdStream << endl << s << " Time =";
  const UInt32 kFreq = 10000000;
  UInt64 sec = val / kFreq;
  PrintNum(sec, 6);
  *g_StdStream << '.';
  UInt32 ms = (UInt32)(val - (sec * kFreq)) / (kFreq / 1000);
  PrintNum(ms, 3, '0');

  while (val > ((UInt64)1 << 56))
  {
    val >>= 1;
    total >>= 1;
  }

  UInt64 percent = 0;
  if (total != 0)
    percent = val * 100 / total;
  *g_StdStream << " =";
  PrintNum(percent, 5);
  *g_StdStream << '%';
}

static inline UInt64 GetTime64(const FILETIME &t) { return ((UInt64)t.dwHighDateTime << 32) | t.dwLowDateTime; }

static void PrintHexId(CStdOutStream &so, UInt64 id)
{
  char s[32];
  ConvertUInt64ToHex(id, s);
  PrintStringRight(so, s, 8);
}

int Main2(int numArgs, char *args[])
{
  UStringVector commandStrings;

  mySplitCommandLine(numArgs,args,commandStrings);

  if (commandStrings.Size() == 1)
  {
    ShowCopyrightAndHelp(g_StdStream, true);
    return 0;
  }

  commandStrings.Delete(0);

  CArcCmdLineOptions options;

  CArcCmdLineParser parser;

  parser.Parse1(commandStrings, options);


  if (options.Number_for_Out != k_OutStream_stdout)
    g_StdStream = (options.Number_for_Out == k_OutStream_stderr ? &g_StdErr : NULL);

  if (options.Number_for_Errors != k_OutStream_stderr)
    g_ErrStream = (options.Number_for_Errors == k_OutStream_stdout ? &g_StdOut : NULL);

  CStdOutStream *percentsStream = NULL;
  if (options.Number_for_Percents != k_OutStream_disabled)
    percentsStream = (options.Number_for_Percents == k_OutStream_stderr) ? &g_StdErr : &g_StdOut;;

  if (options.HelpMode)
  {
    ShowCopyrightAndHelp(g_StdStream, true);
    return 0;
  }

  #ifdef _7ZIP_LARGE_PAGES
  if (options.LargePages)
  {
    SetLargePageSize();
  }
  #endif

  if (options.EnableHeaders)
    ShowCopyrightAndHelp(g_StdStream, false);

  parser.Parse2(options);

  unsigned percentsNameLevel = 1;
  if (options.LogLevel == 0 || options.Number_for_Percents != options.Number_for_Out)
    percentsNameLevel = 2;

  unsigned consoleWidth = 80; // FIXME

  CREATE_CODECS_OBJECT

  codecs->CaseSensitiveChange = options.CaseSensitiveChange;
  codecs->CaseSensitive = options.CaseSensitive;
  ThrowException_if_Error(codecs->Load());

  bool isExtractGroupCommand = options.Command.IsFromExtractGroup();

  if (codecs->Formats.Size() == 0 &&
        (isExtractGroupCommand
        || options.Command.CommandType == NCommandType::kList
        || options.Command.IsFromUpdateGroup()))
  {
    throw kNoFormats;
  }

  CObjectVector<COpenType> types;
  if (!ParseOpenTypes(*codecs, options.ArcType, types))
    throw kUnsupportedArcTypeMessage;

  CIntVector excludedFormats;
  FOR_VECTOR (k, options.ExcludedArcTypes)
  {
    CIntVector tempIndices;
    if (!codecs->FindFormatForArchiveType(options.ExcludedArcTypes[k], tempIndices)
        || tempIndices.Size() != 1)
      throw kUnsupportedArcTypeMessage;
    excludedFormats.AddToUniqueSorted(tempIndices[0]);
  }

  int retCode = NExitCode::kSuccess;
  HRESULT hresultMain = S_OK;

  if (options.Command.CommandType == NCommandType::kInfo)
  {
    CStdOutStream &so = (g_StdStream ? *g_StdStream : g_StdOut);
    unsigned i;

    so << endl << "Formats:" << endl;

    const char *kArcFlags = "KSNFMGOPBELH";
    const unsigned kNumArcFlags = (unsigned)strlen(kArcFlags);

    for (i = 0; i < codecs->Formats.Size(); i++)
    {
      const CArcInfoEx &arc = codecs->Formats[i];

      so << "  ";

      so << (char)(arc.UpdateEnabled ? 'C' : ' ');

      for (unsigned b = 0; b < kNumArcFlags; b++)
      {
        so << (char)
          ((arc.Flags & ((UInt32)1 << b)) != 0 ? kArcFlags[b] : ' ');
      }

      so << ' ';
      PrintString(so, arc.Name, 8);
      so << ' ';
      UString s;

      FOR_VECTOR (t, arc.Exts)
      {
        if (t != 0)
          s.Add_Space();
        const CArcExtInfo &ext = arc.Exts[t];
        s += ext.Ext;
        if (!ext.AddExt.IsEmpty())
        {
          s += L" (";
          s += ext.AddExt;
          s += L')';
        }
      }

      PrintString(so, s, 13);
      so << ' ';

      if (arc.SignatureOffset != 0)
        so << "offset=" << arc.SignatureOffset << ' ';

      FOR_VECTOR(si, arc.Signatures)
      {
        if (si != 0)
          so << "  ||  ";

        const CByteBuffer &sig = arc.Signatures[si];

        for (size_t j = 0; j < sig.Size(); j++)
        {
          if (j != 0)
            so << ' ';
          Byte b = sig[j];
          if (b > 0x20 && b < 0x80)
          {
            so << (char)b;
          }
          else
          {
            so << GetHex((b >> 4) & 0xF);
            so << GetHex(b & 0xF);
          }
        }
      }
      so << endl;
    }

    so << endl << "Codecs:" << endl; //  << "Lib          ID Name" << endl;

    for (i = 0; i < g_NumCodecs; i++)
    {
      const CCodecInfo &cod = *g_Codecs[i];

      PrintLibIndex(so, -1);

      if (cod.NumStreams == 1)
        so << ' ';
      else
        so << cod.NumStreams;

      so << (char)(cod.CreateEncoder ? 'E' : ' ');
      so << (char)(cod.CreateDecoder ? 'D' : ' ');

      so << ' ';
      PrintHexId(so, cod.Id);
      so << ' ' << cod.Name << endl;
    }

    so << endl << "Hashers:" << endl; //  << " L Size       ID Name" << endl;

    for (i = 0; i < g_NumHashers; i++)
    {
      const CHasherInfo &codec = *g_Hashers[i];
      PrintLibIndex(so, -1);
      PrintUInt32(so, codec.DigestSize, 4);
      so << ' ';
      PrintHexId(so, codec.Id);
      so << ' ' << codec.Name << endl;
    }
  }
  else if (options.Command.CommandType == NCommandType::kBenchmark)
  {
    CStdOutStream &so = (g_StdStream ? *g_StdStream : g_StdOut);
    hresultMain = BenchCon(EXTERNAL_CODECS_VARS_L
        options.Properties, options.NumIterations, (FILE *)so);
    if (hresultMain == S_FALSE)
    {
      if (g_ErrStream)
        *g_ErrStream << "\nDecoding ERROR\n";
      retCode = NExitCode::kFatalError;
      hresultMain = S_OK;
    }
  }
  else if (isExtractGroupCommand || options.Command.CommandType == NCommandType::kList)
  {
    UStringVector ArchivePathsSorted;
    UStringVector ArchivePathsFullSorted;

    if (options.StdInMode)
    {
      ArchivePathsSorted.Add(options.ArcName_for_StdInMode);
      ArchivePathsFullSorted.Add(options.ArcName_for_StdInMode);
    }
    else
    {
      CExtractScanConsole scan;

      scan.Init(options.EnableHeaders ? g_StdStream : NULL, g_ErrStream, percentsStream);
      scan.SetWindowWidth(consoleWidth);

      if (g_StdStream && options.EnableHeaders)
        *g_StdStream << "Scanning the drive for archives:" << endl;

      CDirItemsStat st;

      scan.StartScanning();

      hresultMain = EnumerateDirItemsAndSort(
          options.arcCensor,
          NWildcard::k_RelatPath,
          UString(), // addPathPrefix
          ArchivePathsSorted,
          ArchivePathsFullSorted,
          st,
          &scan);

      scan.CloseScanning();

      if (hresultMain == S_OK)
      {
        if (options.EnableHeaders)
          scan.PrintStat(st);
      }
    }

    if (hresultMain == S_OK)
    {
      if (isExtractGroupCommand)
      {
        CExtractCallbackConsole *ecs = new CExtractCallbackConsole;
        CMyComPtr<IFolderArchiveExtractCallback> extractCallback = ecs;

        ecs->PasswordIsDefined = options.PasswordEnabled;
        ecs->Password = options.Password;

        ecs->Init(g_StdStream, g_ErrStream, percentsStream);
        ecs->MultiArcMode = (ArchivePathsSorted.Size() > 1);

        ecs->LogLevel = options.LogLevel;
        ecs->PercentsNameLevel = percentsNameLevel;

        if (percentsStream)
          ecs->SetWindowWidth(consoleWidth);

        CExtractOptions eo;
        (CExtractOptionsBase &)eo = options.ExtractOptions;

        eo.StdInMode = options.StdInMode;
        eo.StdOutMode = options.StdOutMode;
        eo.YesToAll = options.YesToAll;
        eo.TestMode = options.Command.IsTestCommand();

        eo.Properties = options.Properties;

        UString errorMessage;
        CDecompressStat stat;
        CHashBundle hb;
        IHashCalc *hashCalc = NULL;

        if (!options.HashMethods.IsEmpty())
        {
          hashCalc = &hb;
          ThrowException_if_Error(hb.SetMethods(EXTERNAL_CODECS_VARS_L options.HashMethods));
          hb.Init();
        }

        hresultMain = Extract(
            codecs,
            types,
            excludedFormats,
            ArchivePathsSorted,
            ArchivePathsFullSorted,
            options.Censor.Pairs.Front().Head,
            eo, ecs, ecs, hashCalc, errorMessage, stat);

        ecs->ClosePercents();

        if (!errorMessage.IsEmpty())
        {
          if (g_ErrStream)
            *g_ErrStream << endl << "ERROR:" << endl << errorMessage << endl;
          if (hresultMain == S_OK)
            hresultMain = E_FAIL;
        }

        CStdOutStream *so = g_StdStream;

        bool isError = false;

        if (so)
        {
          *so << endl;

          if (ecs->NumTryArcs > 1)
          {
            *so << "Archives: " << ecs->NumTryArcs << endl;
            *so << "OK archives: " << ecs->NumOkArcs << endl;
          }
        }

        if (ecs->NumCantOpenArcs != 0)
        {
          isError = true;
          if (so)
            *so << "Can't open as archive: " << ecs->NumCantOpenArcs << endl;
        }

        if (ecs->NumArcsWithError != 0)
        {
          isError = true;
          if (so)
            *so << "Archives with Errors: " << ecs->NumArcsWithError << endl;
        }

        if (so)
        {
          if (ecs->NumArcsWithWarnings != 0)
            *so << "Archives with Warnings: " << ecs->NumArcsWithWarnings << endl;

          if (ecs->NumOpenArcWarnings != 0)
          {
            *so << endl;
            if (ecs->NumOpenArcWarnings != 0)
              *so << "Warnings: " << ecs->NumOpenArcWarnings << endl;
          }
        }

        if (ecs->NumOpenArcErrors != 0)
        {
          isError = true;
          if (so)
          {
            *so << endl;
            if (ecs->NumOpenArcErrors != 0)
              *so << "Open Errors: " << ecs->NumOpenArcErrors << endl;
          }
        }

        if (isError)
          retCode = ecs->NumCantOpenArcs > 0 ? NExitCode::kFatalCantOpenArcs : NExitCode::kFatalError;

        if (so)
        {
          if (ecs->NumArcsWithError != 0 || ecs->NumFileErrors != 0)
          {
            {
              *so << endl;
              if (ecs->NumFileErrors != 0)
                *so << "Sub items Errors: " << ecs->NumFileErrors << endl;
            }
          }
          else if (hresultMain == S_OK)
          {
            if (stat.NumFolders != 0)
              *so << "Folders: " << stat.NumFolders << endl;
            if (stat.NumFiles != 1 || stat.NumFolders != 0 || stat.NumAltStreams != 0)
              *so << "Files: " << stat.NumFiles << endl;
            if (stat.NumAltStreams != 0)
            {
              *so << "Alternate Streams: " << stat.NumAltStreams << endl;
              *so << "Alternate Streams Size: " << stat.AltStreams_UnpackSize << endl;
            }

            *so
              << "Size:       " << stat.UnpackSize << endl
              << "Compressed: " << stat.PackSize << endl;
            if (hashCalc)
            {
              *so << endl;
              PrintHashStat(*so, hb);
            }
          }
        }
      }
      else
      {
        UInt64 numErrors = 0;
        UInt64 numWarnings = 0;

        hresultMain = ListArchives(
            codecs,
            types,
            excludedFormats,
            options.StdInMode,
            ArchivePathsSorted,
            ArchivePathsFullSorted,
            options.ExtractOptions.NtOptions.AltStreams.Val,
            options.AltStreams.Val, // we don't want to show AltStreams by default
            options.Censor.Pairs.Front().Head,
            options.EnableHeaders,
            options.TechMode,
            options.PasswordEnabled,
            options.Password,
            &options.Properties,
            numErrors, numWarnings);

        if (options.EnableHeaders)
          if (numWarnings > 0)
            g_StdOut << endl << "Warnings: " << numWarnings << endl;

        if (numErrors > 0)
        {
          if (options.EnableHeaders)
            g_StdOut << endl << "Errors: " << numErrors << endl;
          retCode = NExitCode::kFatalError;
        }
      }
    }
  }
  else if (options.Command.IsFromUpdateGroup())
  {
    CUpdateOptions &uo = options.UpdateOptions;
    if (uo.SfxMode && uo.SfxModule.IsEmpty())
      uo.SfxModule = kDefaultSfxModule;

    COpenCallbackConsole openCallback;
    openCallback.Init(g_StdStream, g_ErrStream, percentsStream);

    bool passwordIsDefined =
        (options.PasswordEnabled && !options.Password.IsEmpty());
    openCallback.PasswordIsDefined = passwordIsDefined;
    openCallback.Password = options.Password;

    CUpdateCallbackConsole callback;
    callback.LogLevel = options.LogLevel;
    callback.PercentsNameLevel = percentsNameLevel;

    if (percentsStream)
      callback.SetWindowWidth(consoleWidth);

    callback.PasswordIsDefined = passwordIsDefined;
    callback.AskPassword = (options.PasswordEnabled && options.Password.IsEmpty());
    callback.Password = options.Password;

    callback.StdOutMode = uo.StdOutMode;
    callback.Init(
      g_StdStream, g_ErrStream, percentsStream);

    CUpdateErrorInfo errorInfo;

    hresultMain = UpdateArchive(codecs,
        types,
        options.ArchiveName,
        options.Censor,
        uo,
        errorInfo, &openCallback, &callback, true);

    callback.ClosePercents2();

    CStdOutStream *se = g_StdStream;
    if (!se)
      se = g_ErrStream;

    retCode = WarningsCheck(hresultMain, callback, errorInfo,
        g_StdStream, se,
        true // options.EnableHeaders
        );
#ifdef ENV_UNIX
    if (uo.SfxMode)
    {
        void myAddExeFlag(const UString &name);
        for(int i = 0; i < uo.Commands.Size(); i++)
        {
            CUpdateArchiveCommand &command = uo.Commands[i];
            if (!uo.StdOutMode)
            {
                myAddExeFlag(command.ArchivePath.GetFinalPath());
            }
        }
    }
#endif
  }
  else if (options.Command.CommandType == NCommandType::kHash)
  {
    const CHashOptions &uo = options.HashOptions;

    CHashCallbackConsole callback;
    if (percentsStream)
      callback.SetWindowWidth(consoleWidth);

    callback.Init(g_StdStream, g_ErrStream, percentsStream);
    callback.PrintHeaders = options.EnableHeaders;

    AString errorInfoString;
    hresultMain = HashCalc(EXTERNAL_CODECS_VARS_L
        options.Censor, uo,
        errorInfoString, &callback);
    CUpdateErrorInfo errorInfo;
    errorInfo.Message = errorInfoString;
    CStdOutStream *se = g_StdStream;
    if (!se)
      se = g_ErrStream;
    retCode = WarningsCheck(hresultMain, callback, errorInfo, g_StdStream, se, options.EnableHeaders);
  }
  else
    ShowMessageAndThrowException(kUserErrorMessage, NExitCode::kUserError);

  ThrowException_if_Error(hresultMain);

  return retCode;
}
