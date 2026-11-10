#pragma once
#define COMMAND_COUNT 4
int  shell_prompt_hex(char*);
void push_char(char);
void del_char();
void execute_command();
void init_shell();
void checkpoint(char*);
