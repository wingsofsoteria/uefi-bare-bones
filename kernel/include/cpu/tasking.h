#ifndef __KERNEL_CPU_TASKING_H__
#define __KERNEL_CPU_TASKING_H__

void create_task(void(*)(void), int);
void switch_task(int);
void init_tasking();
void Task1();
void Task2();

#endif
