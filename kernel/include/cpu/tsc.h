#pragma once
#include <stdint.h>
uint64_t calibrate_tsc_slow();
void     set_tsc_deadline(uint64_t);
