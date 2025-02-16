// RandGen.cpp

#include "StdAfx.h"

// Used for logging with "std::cout"
// #include <iostream>

#include "RandGen.h"

#ifndef USE_STATIC_SYSTEM_RAND

#ifndef _7ZIP_ST
#include "../../Windows/Synchronization.h"
#endif

#ifdef _WIN32

#ifdef _WIN64
#define USE_STATIC_RtlGenRandom
#endif

#ifdef USE_STATIC_RtlGenRandom

// #include <ntsecapi.h>

EXTERN_C_BEGIN
#ifndef RtlGenRandom
  #define RtlGenRandom SystemFunction036
  BOOLEAN WINAPI RtlGenRandom(PVOID RandomBuffer, ULONG RandomBufferLength);
#endif
EXTERN_C_END

#else
EXTERN_C_BEGIN
typedef BOOLEAN (WINAPI * Func_RtlGenRandom)(PVOID RandomBuffer, ULONG RandomBufferLength);
EXTERN_C_END
#endif


#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define USE_POSIX_TIME
#define USE_POSIX_TIME2
#endif

#ifdef USE_POSIX_TIME
#include <time.h>
#ifdef USE_POSIX_TIME2
#include <sys/time.h>
#endif
#endif

// The seed and first generated data block depend from processID,
// theadID, timer and system random generator, if available.
// Other generated data blocks depend from previous state

#define HASH_UPD(x) Sha256_Update(&hash, (const Byte *)&x, sizeof(x));

void CRandomGenerator::Init()
{
	
  // ### DEBUG --- BEGIN ---
  // std::cout << "### 7Zip_CRandomGenerator::Init ...\n";
  // ### DEBUG ---  END  ---

  CSha256 hash;
  Sha256_Init(&hash);

  unsigned numIterations = 1000;

  {
  #ifndef UNDER_CE
  const unsigned kNumIterations_Small = 100;
  const unsigned kBufSize = 32;
  Byte buf[kBufSize];
  #endif

  #ifdef _WIN32

  // DWORD w = ::GetCurrentProcessId();
  DWORD w = getpid();
  HASH_UPD(w);
  // w = ::GetCurrentThreadId();
  w = pthread_self();
  HASH_UPD(w);

  #ifdef UNDER_CE
  /*
  if (CeGenRandom(kBufSize, buf))
  {
    numIterations = kNumIterations_Small;
    Sha256_Update(&hash, buf, kBufSize);
  }
  */
  #elif defined(USE_STATIC_RtlGenRandom)
  if (RtlGenRandom(buf, kBufSize))
  {
    numIterations = kNumIterations_Small;
    Sha256_Update(&hash, buf, kBufSize);
    // ### DEBUG --- BEGIN ---
    // std::cout << "### 7Zip_CRandomGenerator::Init STATIC_RtlGenRandom: data received and hashed ...\n";
    // ### DEBUG ---  END  ---
  }
  #else
  {
    HMODULE hModule = ::LoadLibrary(TEXT("Advapi32.dll"));
    if (hModule)
    {
      // ### DEBUG --- BEGIN ---
      // std::cout << "### 7Zip_CRandomGenerator::Init RtlGenRandom: initializing ...\n";
      // ### DEBUG ---  END  ---

      // SystemFunction036() is real name of RtlGenRandom() function
      Func_RtlGenRandom my_RtlGenRandom = (Func_RtlGenRandom)GetProcAddress(hModule, "SystemFunction036");
      if (my_RtlGenRandom)
      {
       // ### DEBUG --- BEGIN ---
       // std::cout << "### 7Zip_CRandomGenerator::Init RtlGenRandom: initialized ...\n";
       // ### DEBUG ---  END  ---
        if (my_RtlGenRandom(buf, kBufSize))
        {
          numIterations = kNumIterations_Small;
          Sha256_Update(&hash, buf, kBufSize);
          // ### DEBUG --- BEGIN ---
          // std::cout << "### 7Zip_CRandomGenerator::Init RtlGenRandom: data received and hashed ...\n";
          // ### DEBUG ---  END  ---
        }
      }
      ::FreeLibrary(hModule);
    }
  }
  #endif

  #else
  
  pid_t pid = getpid();
  HASH_UPD(pid);
  auto ppid = pthread_self();
  HASH_UPD(ppid);

  {
    int f = open("/dev/urandom", O_RDONLY);
    unsigned numBytes = kBufSize;
    if (f >= 0)
    {
      // ### DEBUG --- BEGIN ---
      // std::cout << "### 7Zip_CRandomGenerator::Init /dev/urandom: initialized ...\n";
      // ### DEBUG ---  END  ---
      do
      {
        ssize_t n = read(f, buf, numBytes);
        if (n <= 0)
          break;
        Sha256_Update(&hash, buf, (size_t)n);
        numBytes -= (unsigned)n;
        // ### DEBUG --- BEGIN ---
        // std::cout << "### 7Zip_CRandomGenerator::Init /dev/urandom: data received and hashed ...\n";
        // ### DEBUG ---  END  ---
      }
      while (numBytes);
      close(f);
      if (numBytes == 0)
        numIterations = kNumIterations_Small;
    }
    // ### DEBUG --- BEGIN ---
    // std::cout << "### 7Zip_CRandomGenerator::Init WARNING NO DATA FROM /dev/urandom !\n";
    // ### DEBUG ---  END  ---
  }
  /*
  {
    int n = getrandom(buf, kBufSize, 0);
    if (n > 0)
    {
      Sha256_Update(&hash, buf, n);
      if (n == kBufSize)
        numIterations = kNumIterations_Small;
    }
  }
  */

  #endif
  }

  #ifdef _DEBUG
  numIterations = 2;
  #endif

  do
  {
    // #ifdef _WIN32
    // LARGE_INTEGER v;
    // if (::QueryPerformanceCounter(&v))
    //   HASH_UPD(v.QuadPart);
    // #endif

    #ifdef USE_POSIX_TIME
    #ifdef USE_POSIX_TIME2
    timeval v;
    if (gettimeofday(&v, 0) == 0)
    {
      HASH_UPD(v.tv_sec);
      HASH_UPD(v.tv_usec);
    }
    #endif
    time_t v2 = time(NULL);
    HASH_UPD(v2);
    #endif

    #ifdef _WIN32
    DWORD tickCount = ::GetTickCount();
    HASH_UPD(tickCount);
    #endif
    
    for (unsigned j = 0; j < 100; j++)
    {
      Sha256_Final(&hash, _buff);
      Sha256_Init(&hash);
      Sha256_Update(&hash, _buff, SHA256_DIGEST_SIZE);
    }
  }
  while (--numIterations);

  Sha256_Final(&hash, _buff);
  _needInit = false;
}

#ifndef _7ZIP_ST
  static NWindows::NSynchronization::CCriticalSection g_CriticalSection;
  #define MT_LOCK NWindows::NSynchronization::CCriticalSectionLock lock(g_CriticalSection);
#else
  #define MT_LOCK
#endif

void CRandomGenerator::Generate(Byte *data, unsigned size)
{

  // ### DEBUG --- BEGIN ---
  // std::cout << "### 7Zip_CRandomGenerator::Generate ...\n";
  // ### DEBUG ---  END  ---

  MT_LOCK

  if (_needInit)
    Init();
  while (size != 0)
  {
    CSha256 hash;
    
    Sha256_Init(&hash);
    Sha256_Update(&hash, _buff, SHA256_DIGEST_SIZE);
    Sha256_Final(&hash, _buff);
    
    Sha256_Init(&hash);
    UInt32 salt = 0xF672ABD1;
    HASH_UPD(salt);
    Sha256_Update(&hash, _buff, SHA256_DIGEST_SIZE);
    Byte buff[SHA256_DIGEST_SIZE];
    Sha256_Final(&hash, buff);
    // ### DEBUG --- BEGIN ---
    // std::cout << "### 7Zip_CRandomGenerator::Generate finalizing hashed data ...\n";
    // ### DEBUG ---  END  ---
    for (unsigned i = 0; i < SHA256_DIGEST_SIZE && size != 0; i++, size--)
      *data++ = buff[i];
  }
}

CRandomGenerator g_RandomGenerator;

#endif
