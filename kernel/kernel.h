#ifndef __SOTERIA_KERNEL_KERNEL_H__
#define __SOTERIA_KERNEL_KERNEL_H__
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long int uint64_t;
int qemu_print(char*);
void test_pixels(uint64_t, uint32_t, int, int);
#endif
