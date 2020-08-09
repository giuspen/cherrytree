// DirItem.h

#ifndef __DIR_ITEM_H
#define __DIR_ITEM_H

#include "../../../Common/MyString.h"

#include "../../../Windows/FileFind.h"

#include "../../Common/UniqBlocks.h"

#include "../../Archive/IArchive.h"

struct CDirItemsStat
{
  UInt64 NumDirs;
  UInt64 NumFiles;
  UInt64 NumAltStreams;
  UInt64 FilesSize;
  UInt64 AltStreamsSize;
  
  UInt64 NumErrors;
  // UInt64 GetTotalItems() const { return NumDirs + NumFiles + NumAltStreams; }
  
  UInt64 GetTotalBytes() const { return FilesSize + AltStreamsSize; }
  
  CDirItemsStat():
      NumDirs(0),
      NumFiles(0),
      NumAltStreams(0),
      FilesSize(0),
      AltStreamsSize(0),
      NumErrors(0)
    {}
};

#define INTERFACE_IDirItemsCallback(x) \
  virtual HRESULT ScanError(const FString &path, DWORD systemError) x; \
  virtual HRESULT ScanProgress(const CDirItemsStat &st, const FString &path, bool isDir) x; \

struct IDirItemsCallback
{
  INTERFACE_IDirItemsCallback(=0)
};

struct CDirItem
{
  UInt64 Size;
  FILETIME CTime;
  FILETIME ATime;
  FILETIME MTime;
  UString Name;

  UInt32 Attrib;
  int PhyParent;
  int LogParent;
  int SecureIndex;

  bool IsAltStream;
  
  CDirItem(): PhyParent(-1), LogParent(-1), SecureIndex(-1), IsAltStream(false) {}
  bool IsDir() const { return (Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0 ; }
};

class CDirItems
{
  UStringVector Prefixes;
  CIntVector PhyParents;
  CIntVector LogParents;

  UString GetPrefixesPath(const CIntVector &parents, int index, const UString &name) const;

  HRESULT EnumerateDir(int phyParent, int logParent, const FString &phyPrefix);

public:
  CObjectVector<CDirItem> Items;

  bool SymLinks;

  bool ScanAltStreams;
  
  CDirItemsStat Stat;

  #ifndef UNDER_CE
  HRESULT SetLinkInfo(CDirItem &dirItem, const NWindows::NFile::NFind::CFileInfo &fi,
      const FString &phyPrefix);
  #endif

  IDirItemsCallback *Callback;

  CDirItems();

  void AddDirFileInfo(int phyParent, int logParent, int secureIndex,
      const NWindows::NFile::NFind::CFileInfo &fi);

  HRESULT AddError(const FString &path, DWORD errorCode);
  HRESULT AddError(const FString &path);

  HRESULT ScanProgress(const FString &path);

  // unsigned GetNumFolders() const { return Prefixes.Size(); }
  FString GetPhyPath(unsigned index) const;
  UString GetLogPath(unsigned index) const;

  unsigned AddPrefix(int phyParent, int logParent, const UString &prefix);
  void DeleteLastPrefix();
  
  HRESULT EnumerateItems2(
    const FString &phyPrefix,
    const UString &logPrefix,
    const FStringVector &filePaths,
    FStringVector *requestedPaths);

  void ReserveDown();
};

struct CArcItem
{
  UInt64 Size;
  FILETIME MTime;
  UString Name;
  bool IsDir;
  bool IsAltStream;
  bool SizeDefined;
  bool MTimeDefined;
  bool Censored;
  UInt32 IndexInServer;
  int TimeType;
  
  CArcItem(): IsDir(false), IsAltStream(false), SizeDefined(false), MTimeDefined(false), Censored(false), TimeType(-1) {}
};

#endif
