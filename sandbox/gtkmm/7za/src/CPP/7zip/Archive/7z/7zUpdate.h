// 7zUpdate.h

#ifndef __7Z_UPDATE_H
#define __7Z_UPDATE_H

#include "../IArchive.h"

#include "7zCompressionMode.h"
#include "7zIn.h"
#include "7zOut.h"

namespace NArchive {
namespace N7z {

struct CUpdateItem
{
  int IndexInArchive;
  int IndexInClient;
  
  UInt64 CTime;
  UInt64 ATime;
  UInt64 MTime;

  UInt64 Size;
  UString Name;

  // that code is not used in 9.26

  UInt32 Attrib;
  
  bool NewData;
  bool NewProps;

  bool IsAnti;
  bool IsDir;

  bool AttribDefined;
  bool CTimeDefined;
  bool ATimeDefined;
  bool MTimeDefined;

  bool HasStream() const { return !IsDir && !IsAnti && Size != 0; }

  CUpdateItem():
      IsAnti(false),
      IsDir(false),
      AttribDefined(false),
      CTimeDefined(false),
      ATimeDefined(false),
      MTimeDefined(false)
      {}
  void SetDirStatusFromAttrib() { IsDir = ((Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0); }

};

struct CUpdateOptions
{
  const CCompressionMethodMode *Method;
  const CCompressionMethodMode *HeaderMethod;
  bool UseFilters; // use additional filters for some files
  bool MaxFilter;  // use BCJ2 filter instead of BCJ
  int AnalysisLevel;

  CHeaderOptions HeaderOptions;

  UInt64 NumSolidFiles;
  UInt64 NumSolidBytes;
  bool SolidExtension;
  
  bool UseTypeSorting;
  
  bool RemoveSfxBlock;
  bool MultiThreadMixer;

  CUpdateOptions():
      Method(NULL),
      HeaderMethod(NULL),
      UseFilters(false),
      MaxFilter(false),
      AnalysisLevel(-1),
      NumSolidFiles((UInt64)(Int64)(-1)),
      NumSolidBytes((UInt64)(Int64)(-1)),
      SolidExtension(false),
      UseTypeSorting(true),
      RemoveSfxBlock(false),
      MultiThreadMixer(true)
    {}
};

HRESULT Update(
    DECL_EXTERNAL_CODECS_LOC_VARS
    IInStream *inStream,
    const CDbEx *db,
    const CObjectVector<CUpdateItem> &updateItems,
    COutArchive &archive,
    CArchiveDatabaseOut &newDatabase,
    ISequentialOutStream *seqOutStream,
    IArchiveUpdateCallback *updateCallback,
    const CUpdateOptions &options,
    ICryptoGetTextPassword *getDecoderPassword
    );
}}

#endif
