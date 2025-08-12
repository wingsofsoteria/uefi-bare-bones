// clang-format Language: C
#ifndef __KERNEL_KERNEL_H__
#define __KERNEL_KERNEL_H__
#include "../graphics/pixel.h"
#include "../graphics/tty.h"

#include "debug.h"
#include "types.h"
#include "../memory/cpio.h"
#include "../memory/alloc.h"
#include "../memory/pages.h"
#include "acpi.h"
void load_gdt();
void load_idt();
#endif
