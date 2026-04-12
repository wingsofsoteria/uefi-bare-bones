#pragma once
#include <stdint.h>
void pit_init();
void pit_sleep(uint64_t);
uint16_t pit_count();
#define PIT_FREQ             1193182
#define PIT_HZ               1000
#define PIT_MODE_COMMAND_REG 0x43
#define PIT_DATA0            0x40
