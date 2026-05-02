#pragma once
typedef __builtin_va_list va_list;
#define va_arg(v, t)   __builtin_va_arg(v, t)
#define va_start(f, v) __builtin_va_start(f, v)
#define va_end(v)      __builtin_va_end(v)
