#include "config.h"
#include "cpu/task.h"
#include "memory/alloc.h"
#include "terminal/tty.h"
#include "keyboard.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

static char* shell_cmd;
static int shell_size;
static int shell_cur;

static const char* command_list[COMMAND_COUNT] = {
  "test", "exit", "rem", "jump"};

static void test(void* _unused);
static void exit(void* _unused);
static void rem(void* comment);
static void jump(void* function);
static const task_function commands[COMMAND_COUNT] = {test, exit, rem, jump};

void checkpoint(char* name)
{
  kernel_log_debug("CHECKPOINT %s [yN]", name);

  while (1)
  {
    if (is_key_pressed('y'))
    {
      tty_delc();
      putchar('\n');
      break;
    }
  }
}

// NOLINTNEXTLINE(*-unused-parameters)
void exit(void* _unused)
{
  kernel_config.kexit = 1;
}

extern uint64_t resolve_function_address(char*);
extern void call_function(void*);

void jump(void* function)
{
  uint64_t rip = resolve_function_address((char*)(function + 1));
  kernel_log_debug("RIP: %lx %s", rip, (char*)function);
  if (rip == 0)
  {
    printf("Could not locate function %s\n", (char*)function);
    return;
  }
  call_function((void*)rip);
}

void rem(void* comment)
{
  if (comment == NULL)
  {
    kernel_log_error("Rem command needs an argument\n");
    return;
  }
  printf("REM %s\n", (char*)comment);
}

// NOLINTNEXTLINE(*-unused-parameters)
void test(void* _unused)
{
  printf("test command! :3\n");
}

void push_char(char ch)
{
  if (shell_cur >= shell_size)
  {
    shell_size += 80;
    shell_cmd   = krealloc(shell_cmd, shell_size);
  }
  shell_cmd[shell_cur++] = ch;
  putchar(ch);
}

void del_char()
{
  shell_cmd[--shell_cur] = 0;
  tty_delc();
}

void init_shell()
{
  shell_cur  = 0;
  shell_size = 80;
  shell_cmd  = kmalloc(shell_size * sizeof(char));
  putchar('>');
}

void execute_command()
{
  putchar('\n');
  if (shell_cur == 0)
  {
    goto fini;
  }

  int index = 0;
  for (; index < COMMAND_COUNT; index++)
  {
    int count = strlen(command_list[index]);
    if (strncmp(shell_cmd, command_list[index], count) == 0)
    {
      char* args = NULL;
      if (count < shell_cur)
      {
        args = (void*)(shell_cmd + count);
      }
      kernel_log_debug(
        "SHELL COMMAND: %s, count %d, shell_cur: %d, args: %s, args_triggered? %s\n",
        command_list[index], count, shell_cur, args,
        count < shell_cur ? "true" : "false");
      commands[index](args);
      goto fini;
    }
  }

fini:
  shell_cur = 0;
  shell_cmd = memset(shell_cmd, 0, shell_size);
  putchar('>');
}
