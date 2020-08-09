/* CpuArch.c -- CPU specific code
2016-02-25: Igor Pavlov : Public domain */

#include "Precomp.h"

#include "CpuArch.h"

#ifdef _7ZIP_ASM
#ifdef MY_CPU_X86_OR_AMD64

#if defined(__GNUC__)
#define USE_ASM
#endif

#if defined(USE_ASM) && !defined(MY_CPU_AMD64)
static UInt32 CheckFlag(UInt32 flag)
{
  __asm__ __volatile__ (
    "pushf\n\t"
    "pop  %%EAX\n\t"
    "movl %%EAX,%%EDX\n\t"
    "xorl %0,%%EAX\n\t"
    "push %%EAX\n\t"
    "popf\n\t"
    "pushf\n\t"
    "pop  %%EAX\n\t"
    "xorl %%EDX,%%EAX\n\t"
    "push %%EDX\n\t"
    "popf\n\t"
    "andl %%EAX, %0\n\t":
    "=c" (flag) : "c" (flag) :
    "%eax", "%edx");
  return flag;
}
#define CHECK_CPUID_IS_SUPPORTED if (CheckFlag(1 << 18) == 0 || CheckFlag(1 << 21) == 0) return False;
#else
#define CHECK_CPUID_IS_SUPPORTED
#endif

void MyCPUID(UInt32 function, UInt32 *a, UInt32 *b, UInt32 *c, UInt32 *d)
{
  #ifdef USE_ASM

  __asm__ __volatile__ (
  #if defined(MY_CPU_AMD64) && defined(__PIC__)
    "mov %%rbx, %%rdi;"
    "cpuid;"
    "xchg %%rbx, %%rdi;"
    : "=a" (*a) ,
      "=D" (*b) ,
  #elif defined(MY_CPU_X86) && defined(__PIC__)
    "mov %%ebx, %%edi;"
    "cpuid;"
    "xchgl %%ebx, %%edi;"
    : "=a" (*a) ,
      "=D" (*b) ,
  #else
    "cpuid"
    : "=a" (*a) ,
      "=b" (*b) ,
  #endif
      "=c" (*c) ,
      "=d" (*d)
    : "0" (function)) ;
  
  #else

  int CPUInfo[4];
  __cpuid(CPUInfo, function);
  *a = CPUInfo[0];
  *b = CPUInfo[1];
  *c = CPUInfo[2];
  *d = CPUInfo[3];

  #endif
}

Bool x86cpuid_CheckAndRead(Cx86cpuid *p)
{
  CHECK_CPUID_IS_SUPPORTED
  MyCPUID(0, &p->maxFunc, &p->vendor[0], &p->vendor[2], &p->vendor[1]);
  MyCPUID(1, &p->ver, &p->b, &p->c, &p->d);
  return True;
}

static const UInt32 kVendors[][3] =
{
  { 0x756E6547, 0x49656E69, 0x6C65746E},
  { 0x68747541, 0x69746E65, 0x444D4163},
  { 0x746E6543, 0x48727561, 0x736C7561}
};

int x86cpuid_GetFirm(const Cx86cpuid *p)
{
  unsigned i;
  for (i = 0; i < sizeof(kVendors) / sizeof(kVendors[i]); i++)
  {
    const UInt32 *v = kVendors[i];
    if (v[0] == p->vendor[0] &&
        v[1] == p->vendor[1] &&
        v[2] == p->vendor[2])
      return (int)i;
  }
  return -1;
}

Bool CPU_Is_InOrder()
{
  Cx86cpuid p;
  int firm;
  UInt32 family, model;
  if (!x86cpuid_CheckAndRead(&p))
    return True;

  family = x86cpuid_GetFamily(p.ver);
  model = x86cpuid_GetModel(p.ver);
  
  firm = x86cpuid_GetFirm(&p);

  switch (firm)
  {
    case CPU_FIRM_INTEL: return (family < 6 || (family == 6 && (
        /* In-Order Atom CPU */
           model == 0x1C  /* 45 nm, N4xx, D4xx, N5xx, D5xx, 230, 330 */
        || model == 0x26  /* 45 nm, Z6xx */
        || model == 0x27  /* 32 nm, Z2460 */
        || model == 0x35  /* 32 nm, Z2760 */
        || model == 0x36  /* 32 nm, N2xxx, D2xxx */
        )));
    case CPU_FIRM_AMD: return (family < 5 || (family == 5 && (model < 6 || model == 0xA)));
    case CPU_FIRM_VIA: return (family < 6 || (family == 6 && model < 0xF));
  }
  return True;
}

#define CHECK_SYS_SSE_SUPPORT

Bool CPU_Is_Aes_Supported()
{
  Cx86cpuid p;
  CHECK_SYS_SSE_SUPPORT
  if (!x86cpuid_CheckAndRead(&p))
    return False;
  return (p.c >> 25) & 1;
}

#endif

#else

Bool CPU_Is_InOrder()
{
    return True;
}

#endif // ifdef _7ZIP_ASM

