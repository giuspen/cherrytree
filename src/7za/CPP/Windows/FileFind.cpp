// Windows/FileFind.cpp

#include "StdAfx.h"

#include "FileFind.h"
#include "FileIO.h"

#include "../Common/StringConvert.h"


#ifndef _UNICODE
extern bool g_IsNT;
#endif

#include <unistd.h>
#include <errno.h>

#define NEED_NAME_WINDOWS_TO_UNIX
#include "myPrivate.h"

// #define TRACEN(u) u;
#define TRACEN(u)  /* */

void my_windows_split_path(const AString &p_path, AString &dir , AString &base) {
  int pos = p_path.ReverseFind('/');
  if (pos == -1) {
    // no separator
    dir  = ".";
    if (p_path.IsEmpty())
      base = ".";
    else
      base = p_path;
  } else if ((pos+1) < p_path.Len()) {
    // true separator
    base = p_path.Ptr(pos+1);
    while ((pos >= 1) && (p_path[pos-1] == '/'))
      pos--;
    if (pos == 0)
      dir = "/";
    else
      dir = p_path.Left(pos);
  } else {
    // separator at the end of the path
    // pos = p_path.find_last_not_of("/");
    pos = -1;
    int ind = 0;
    while (p_path[ind]) {
      if (p_path[ind] != '/')
        pos = ind;
      ind++;
    }
    if (pos == -1) {
      base = "/";
      dir = "/";
    } else {
      my_windows_split_path(p_path.Left(pos+1),dir,base);
    }
  }
}

static void my_windows_split_path(const UString &p_path, UString &dir , UString &base) {
  int pos = p_path.ReverseFind(L'/');
  if (pos == -1) {
    // no separator
    dir  = L".";
    if (p_path.IsEmpty())
      base = L".";
    else
      base = p_path;
  } else if ((pos+1) < p_path.Len()) {
    // true separator
    base = p_path.Ptr(pos+1);
    while ((pos >= 1) && (p_path[pos-1] == L'/'))
      pos--;
    if (pos == 0)
      dir = L"/";
    else
      dir = p_path.Left(pos);
  } else {
    // separator at the end of the path
    // pos = p_path.find_last_not_of("/");
    pos = -1;
    int ind = 0;
    while (p_path[ind]) {
      if (p_path[ind] != L'/')
        pos = ind;
      ind++;
    }
    if (pos == -1) {
      base = L"/";
      dir = L"/";
    } else {
      my_windows_split_path(p_path.Left(pos+1),dir,base);
    }
  }
}

static int filter_pattern(const char *string , const char *pattern , int flags_nocase) {
  if ((string == 0) || (*string==0)) {
    if (pattern == 0)
      return 1;
    while (*pattern=='*')
      ++pattern;
    return (!*pattern);
  }

  switch (*pattern) {
  case '*':
    if (!filter_pattern(string+1,pattern,flags_nocase))
      return filter_pattern(string,pattern+1,flags_nocase);
    return 1;
  case 0:
    if (*string==0)
      return 1;
    break;
  case '?':
    return filter_pattern(string+1,pattern+1,flags_nocase);
  default:
    if (   ((flags_nocase) && (tolower(*pattern)==tolower(*string)))
           || (*pattern == *string)
       ) {
      return filter_pattern(string+1,pattern+1,flags_nocase);
    }
    break;
  }
  return 0;
}

namespace NWindows {
namespace NFile {

#ifdef SUPPORT_DEVICE_FILE
bool IsDeviceName(CFSTR n);
#endif

#if defined(WIN_LONG_PATH)
bool GetLongPath(CFSTR fileName, UString &res);
#endif

namespace NFind {

bool CFileInfo::IsDots() const throw()
{
  if (!IsDir() || Name.IsEmpty())
    return false;
  if (Name[0] != FTEXT('.'))
    return false;
  return Name.Len() == 1 || (Name.Len() == 2 && Name[1] == FTEXT('.'));
}

#define WIN_FD_TO_MY_FI(fi, fd) \
  fi.Attrib = fd.dwFileAttributes; \
  fi.CTime = fd.ftCreationTime; \
  fi.ATime = fd.ftLastAccessTime; \
  fi.MTime = fd.ftLastWriteTime; \
  fi.Size = (((UInt64)fd.nFileSizeHigh) << 32) + fd.nFileSizeLow; \
  fi.IsDevice = false;

  /*
  #ifdef UNDER_CE
  fi.ObjectID = fd.dwOID;
  #else
  fi.ReparseTag = fd.dwReserved0;
  #endif
  */

#ifndef _UNICODE

static inline UINT GetCurrentCodePage() { return ::AreFileApisANSI() ? CP_ACP : CP_OEMCP; }

static void ConvertWIN32_FIND_DATA_To_FileInfo(const WIN32_FIND_DATA &fd, CFileInfo &fi)
{
  WIN_FD_TO_MY_FI(fi, fd);
  fi.Name = fas2fs(fd.cFileName);
}
#endif
  
////////////////////////////////
// CFindFile

bool CFindFile::Close()
{
  if (_dirp != 0) {
    g_dir_close(_dirp);
    _dirp = 0;
  }
  return true;
}


static bool originalFilename(const UString & src, AString & res)
{
  // Try to recover the original filename
  res = "";
  int i=0;
  while (src[i])
  {
    if (src[i] >= 256) {
      return false;
    } else {
      res += char(src[i]);
    }
    i++;
  }
  return true;
}

// Warning this function cannot update "fileInfo.Name"
static int fillin_CFileInfo(CFileInfo &fileInfo,const char *filename,bool ignoreLink) {
  struct stat stat_info;

  int ret;
#ifdef ENV_HAVE_LSTAT
  if ( (global_use_lstat) && (ignoreLink == false)) {
    ret = g_lstat(filename,&stat_info);
  } else
#endif
  {
     ret = g_stat(filename,&stat_info);
  }

  // printf("fillin_CFileInfo(%s,%d)=%d  mode=%o\n",filename,(int)ignoreLink,ret,(unsigned)stat_info.st_mode);


  if (ret != 0) return ret;

  /* FIXME : FILE_ATTRIBUTE_HIDDEN ? */
  if (S_ISDIR(stat_info.st_mode)) {
    fileInfo.Attrib = FILE_ATTRIBUTE_DIRECTORY;
  } else {
    fileInfo.Attrib = FILE_ATTRIBUTE_ARCHIVE;
  }

  if (!(stat_info.st_mode & S_IWUSR))
    fileInfo.Attrib |= FILE_ATTRIBUTE_READONLY;

  fileInfo.Attrib |= FILE_ATTRIBUTE_UNIX_EXTENSION + ((stat_info.st_mode & 0xFFFF) << 16);

  RtlSecondsSince1970ToFileTime( stat_info.st_ctime, &fileInfo.CTime );
  RtlSecondsSince1970ToFileTime( stat_info.st_mtime, &fileInfo.MTime );
  RtlSecondsSince1970ToFileTime( stat_info.st_atime, &fileInfo.ATime );

  fileInfo.IsDevice = false;

  if (S_ISDIR(stat_info.st_mode)) {
    fileInfo.Size = 0;
  } else { // file or symbolic link
    fileInfo.Size = stat_info.st_size; // for a symbolic link, size = size of filename
  }
  return 0;
}

static int fillin_CFileInfo(CFileInfo &fi,const char *dir,const char *name,bool ignoreLink) {
  char filename[MAX_PATHNAME_LEN];
  size_t dir_len = strlen(dir);
  size_t name_len = strlen(name);
  size_t total = dir_len + 1 + name_len + 1; // 1 = strlen("/"); + le zero character
  if (total >= MAX_PATHNAME_LEN) throw "fillin_CFileInfo - internal error - MAX_PATHNAME_LEN";
  memcpy(filename,dir,dir_len);
  if (dir_len >= 1)
  {
    if (filename[dir_len-1] == CHAR_PATH_SEPARATOR)
    { // delete the '/'
        dir_len--;
    }
  }
  filename[dir_len] = CHAR_PATH_SEPARATOR;
  memcpy(filename+(dir_len+1),name,name_len+1); // copy also final '\0'

#ifdef _UNICODE
  fi.Name = GetUnicodeString(name, CP_ACP);
#else
  fi.Name = name;
#endif

  int ret = fillin_CFileInfo(fi,filename,ignoreLink);
  if (ret != 0) {
    AString err_msg = "stat error for ";
        err_msg += filename;
        err_msg += " (";
        err_msg += strerror(errno);
        err_msg += ")";
        throw err_msg;
  }
  return ret;
}

bool CFindFile::FindFirst(CFSTR cfWildcard, CFileInfo &fi, bool ignoreLink)
{
  if (!Close())
    return false;

  AString Awildcard = UnicodeStringToMultiByte(cfWildcard, CP_ACP);
  const char * wildcard = (const char *)Awildcard;


  if ((!wildcard) || (wildcard[0]==0)) {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return false;
  }
 
  my_windows_split_path(nameWindowToUnix(wildcard),_directory,_pattern);
  
  TRACEN((printf("CFindFile::FindFirst : %s (dirname=%s,pattern=%s)\n",wildcard,(const char *)_directory,(const char *)_pattern)))

  _dirp = g_dir_open((const char *)_directory, 0, NULL);
  TRACEN((printf("CFindFile::FindFirst : opendir=%p\n",_dirp)))

  if ((_dirp == 0) && (global_use_utf16_conversion)) {
    // Try to recover the original filename
    UString ustr = MultiByteToUnicodeString(_directory, 0);
    AString resultString;
    bool is_good = originalFilename(ustr, resultString);
    if (is_good) {
      _dirp = g_dir_open((const char *)resultString, 0, NULL);
      _directory = resultString;
    }
  }

  if (_dirp == 0) return false;

  const gchar* d_name;
  while ((d_name = g_dir_read_name(_dirp)) != NULL) {
    if (filter_pattern(d_name,(const char *)_pattern,0) == 1) {
      int retf = fillin_CFileInfo(fi,(const char *)_directory,d_name,ignoreLink);
      if (retf)
      {
         TRACEN((printf("CFindFile::FindFirst : closedir-1(dirp=%p)\n",_dirp)))
         g_dir_close(_dirp);
         _dirp = 0;
         SetLastError( ERROR_NO_MORE_FILES );
         return false;
      }
      TRACEN((printf("CFindFile::FindFirst -%s- true\n",dp->d_name)))
      return true;
    }
  }

  TRACEN((printf("CFindFile::FindFirst : closedir-2(dirp=%p)\n",_dirp)))
  g_dir_close(_dirp);
  _dirp = 0;
  SetLastError( ERROR_NO_MORE_FILES );
  return false;

}

bool CFindFile::FindNext(CFileInfo &fi)
{
  if (_dirp == 0)
  {
    SetLastError( ERROR_INVALID_HANDLE );
    return false;
  }

  const gchar* d_name;
  while ((d_name = g_dir_read_name(_dirp)) != NULL) {
      if (filter_pattern(d_name,(const char *)_pattern,0) == 1) {
        int retf = fillin_CFileInfo(fi,(const char *)_directory,d_name,false);
        if (retf)
        {
           TRACEN((printf("FindNextFileA -%s- ret_handle=FALSE (errno=%d)\n",d_name,errno)))
           return false;

        }
        TRACEN((printf("FindNextFileA -%s- true\n",d_name)))
        return true;
      }
    }
  TRACEN((printf("FindNextFileA ret_handle=FALSE (ERROR_NO_MORE_FILES)\n")))
  SetLastError( ERROR_NO_MORE_FILES );
  return false;
}

#define MY_CLEAR_FILETIME(ft) ft.dwLowDateTime = ft.dwHighDateTime = 0;

void CFileInfoBase::ClearBase() throw()
{
  Size = 0;
  MY_CLEAR_FILETIME(CTime);
  MY_CLEAR_FILETIME(ATime);
  MY_CLEAR_FILETIME(MTime);
  Attrib = 0;
  IsAltStream = false;
  IsDevice = false;
}
  
bool CFileInfo::Find(CFSTR wildcard, bool ignoreLink)
{
  #ifdef SUPPORT_DEVICE_FILE
  if (IsDeviceName(wildcard))
  {
    Clear();
    IsDevice = true;
    NIO::CInFile inFile;
    if (!inFile.Open(wildcard))
      return false;
    Name = wildcard + 4;
    if (inFile.LengthDefined)
      Size = inFile.Length;
    return true;
  }
  #endif
  CFindFile finder;
  if (finder.FindFirst(wildcard, *this,ignoreLink))
    return true;
  return false;
}

bool DoesFileExist(CFSTR name)
{
  CFileInfo fi;
  return fi.Find(name) && !fi.IsDir();
}

bool DoesDirExist(CFSTR name)
{
  CFileInfo fi;
  return fi.Find(name) && fi.IsDir();
}
bool DoesFileOrDirExist(CFSTR name)
{
  CFileInfo fi;
  return fi.Find(name);
}

bool CEnumerator::NextAny(CFileInfo &fi)
{
  if (_findFile.IsHandleAllocated())
    return _findFile.FindNext(fi);
  else
    return _findFile.FindFirst(_wildcard, fi);
}

bool CEnumerator::Next(CFileInfo &fi)
{
  for (;;)
  {
    if (!NextAny(fi))
      return false;
    if (!fi.IsDots())
      return true;
  }
}

bool CEnumerator::Next(CFileInfo &fi, bool &found)
{
  if (Next(fi))
  {
    found = true;
    return true;
  }
  found = false;
  return (::GetLastError() == ERROR_NO_MORE_FILES);
}

} // namespace NFind
} // namespace NFile
} // namespace NWindows
