// Windows/Thread.h

#ifndef __WINDOWS_THREAD_H
#define __WINDOWS_THREAD_H

#include "Defs.h"

extern "C"
{
#include "../../C/Threads.h"
}

namespace NWindows {

class CThread
{
  ::CThread thread;
public:
  CThread() { Thread_Construct(&thread); }
  ~CThread() { Close(); }
  bool IsCreated() { return Thread_WasCreated(&thread) != 0; }
  WRes Close()  { return Thread_Close(&thread); }
  WRes Create(THREAD_FUNC_RET_TYPE (*startAddress)(void *), LPVOID parameter)
    { return Thread_Create(&thread, startAddress, parameter); }
  WRes Wait() { return Thread_Wait(&thread); }
};

}

#endif
