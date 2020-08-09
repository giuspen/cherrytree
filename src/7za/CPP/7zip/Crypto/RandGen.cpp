// RandGen.cpp

#include "StdAfx.h"

#include "../../Windows/Synchronization.h"

#include "RandGen.h"

#include <unistd.h>
#define USE_POSIX_TIME
#define USE_POSIX_TIME2

#ifdef USE_POSIX_TIME
#include <time.h>
#ifdef USE_POSIX_TIME2
#include <sys/time.h>
#endif
#endif

// This is not very good random number generator.
// Please use it only for salt.
// First generated data block depends from timer and processID.
// Other generated data blocks depend from previous state
// Maybe it's possible to restore original timer value from generated value.

#define HASH_UPD(x) Sha256_Update(&hash, (const Byte *)&x, sizeof(x));

void CRandomGenerator::Init()
{
  CSha256 hash;
  Sha256_Init(&hash);

  pid_t pid = getpid();
  HASH_UPD(pid);
  auto ppid = pthread_self();
  HASH_UPD(ppid);

  for (unsigned i = 0; i <
    #ifdef _DEBUG
    2;
    #else
    1000;
    #endif
    i++)
  {
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

    DWORD tickCount = ::GetTickCount();
    HASH_UPD(tickCount);
    
    for (unsigned j = 0; j < 100; j++)
    {
      Sha256_Final(&hash, _buff);
      Sha256_Init(&hash);
      Sha256_Update(&hash, _buff, SHA256_DIGEST_SIZE);
    }
  }
  Sha256_Final(&hash, _buff);
  _needInit = false;
}

static NWindows::NSynchronization::CCriticalSection g_CriticalSection;
#define MT_LOCK NWindows::NSynchronization::CCriticalSectionLock lock(g_CriticalSection);

void CRandomGenerator::Generate(Byte *data, unsigned size)
{
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
    for (unsigned i = 0; i < SHA256_DIGEST_SIZE && size != 0; i++, size--)
      *data++ = buff[i];
  }
}

CRandomGenerator g_RandomGenerator;
