#include "config.h"
#include "cpu/task.h"
#include "graphics/tty.h"
#include "keyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

static char* shell_cmd;
static int shell_size;
static int shell_cur;

static const char* command_list[COMMAND_COUNT] = {"test", "exit", "rem"};

static void test(void* _unused);
static void exit(void* _unused);
static void rem(void* comment);
static const task_function commands[COMMAND_COUNT] = {test, exit, rem};

void checkpoint(char* name)
{
  printf("CHECKPOINT %s [yN]", name);

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

void rem(void* comment)
{
  if (comment == NULL)
  {
    printf("ERROR!\n");
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
    shell_cmd   = realloc(shell_cmd, shell_size);
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
  shell_cmd  = malloc(shell_size * sizeof(char));
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
#ifdef KERNEL_DEBUG
      printf(
        "SHELL COMMAND: %s, count %d, shell_cur: %d, args: %x, args_triggered? %s\n",
        command_list[index], count, shell_cur, args,
        count < shell_cur ? "true" : "false");
#endif
      commands[index](args);
      goto fini;
    }
  }

fini:
  shell_cur = 0;
  shell_cmd = memset(shell_cmd, 0, shell_size);
  putchar('>');
}
