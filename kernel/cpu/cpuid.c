#include "cpuid.h"
#include <stdint.h>
#include <stdio.h>
#include "config.h"
void check_cpuid()
{
  uint32_t eax    = 0;
  uint32_t ebx    = 0;
  uint32_t ecx    = 0;
  uint32_t edx    = 0;
  uint32_t unused = 0;
  int supported   = __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  if (!supported)
  {
    printf("Could not get CPU information\n");
  }
  if (!(edx & bit_APIC))
  {
    printf("APIC interrupts unsupported\n");
  }
  if (!(edx & bit_MSR))
  {
    printf("MSRs unsupported\n");
  }

  if (!(ecx & bit_TSCDeadline))
  {
    printf("TSC deadline mode unsupported\n");
  }
  supported = __get_cpuid(0x80000007, &unused, &unused, &unused, &edx);
  if ((supported && !(edx & (1 << 8))) || !supported)
  {
    printf("Invariant TSC unsupported\n");
  }

  supported = __get_cpuid(0x15, &eax, &ebx, &ecx, &edx);
  if (supported)
  {
    printf("CPUID 0x15: eax %d, ebx %d, ecx %d, edx %d\n", eax, ebx, ecx, edx);
  }
  supported = __get_cpuid(0x16, &eax, &ebx, &ecx, &edx);
  if (supported)
  {
    printf("CPUID 0x16: eax %d, ebx %d, ecx %d, edx %d\n", eax, ebx, ecx, edx);
  }
}
