// clang-format Language: C
#ifndef __SOTERIA_KERNEL_KERNEL_H__
#define __SOTERIA_KERNEL_KERNEL_H__
#include <stdint.h>
int qemu_print(char*);
void qemu_printn(unsigned long long int value, int base);
// void setup_paging(void*, int);
// int ident_map(void*, unsigned int);
// uint64_t get_map(void*);

// static inline void __native_flush_tlb_single(unsigned long addr);
#endif
