#ifndef __KERNEL_KEYBOARD_H__
#define __KERNEL_KEYBOARD_H__

void kb_handle_key();
int is_key_pressed(char);
int naive_key_released(char);

#endif
