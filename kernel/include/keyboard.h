#ifndef __KERNEL_KEYBOARD_H__
#define __KERNEL_KEYBOARD_H__

void kb_handle_key();
void init_kb_status();
int is_key_pressed(char);
int naive_key_released(char);

#endif
