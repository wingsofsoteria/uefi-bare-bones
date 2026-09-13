#include "shell.h"

#include "config.h"
#include "keyboard.h"
#include "log.h"
#include "memory/alloc.h"
#include "terminal/tty.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* shell_cmd;
static int   shell_size;
static int   shell_cur;
static int   enabled = 0;

/*static const char* command_list[COMMAND_COUNT] = {"test", "exit", "rem",
                                                  "jump"};

static void                test(void* _unused);
static void                exit(void* _unused);
static void                rem(void* comment);
static void                jump(void* function);
static const task_function commands[COMMAND_COUNT] = {test, exit, rem, jump};
*/
void checkpoint(char* name)
{
  klog("CHECKPOINT %s [yN]", name);

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

extern uint64_t resolve_function_address(char*);
extern void     call_function(void*);

void kexit() { kernel_config.kexit = 1; }

static void jump(void* function)
{
  uint64_t rip = resolve_function_address((char*)(function + 1));
  klog("RIP: %lx %s\n", rip, (char*)function);
  if (rip == 0)
    {
      printf("Could not locate function %s\n", (char*)function);
      return;
    }
  call_function((void*)rip);
}

static void rem(void* comment)
{
  if (comment == NULL)
    {
      klog("Rem command needs an argument\n");
      return;
    }
  printf("REM %s\n", (char*)comment);
}

void test() { printf("test command! :3\n"); }

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
  enabled    = 1;
  shell_cur  = 0;
  shell_size = 80;
  shell_cmd  = kmalloc(shell_size * sizeof(char));
  putchar('>');
}

extern void dump_function_names();

static int is_digit16(char c)
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
         (c >= 'a' && c <= 'f');
}

static uint64_t naive_atoi16(char* str, int len)
{
  char*    p     = str;
  uint64_t value = 0;
  for (int i = 0; i < len && *p; i++)
    {
      if (!is_digit16(*p)) { break; }
      if (*p <= '9') { value = (value * 16) + (*p - '0'); }
      if (*p >= 'A')
        {
          value = (value * 16) + ((*p - 'A') & (~('a' ^ 'A'))) + 10;
        }
      p++;
    }

  return value;
}

int shell_prompt_hex(char* msg)
{
  puts(msg);
  char input[4];
  int  len = 0;
  for (int i = 0; i < 4; i++)
    {
      char ch = getch();
      if (ch == '\n')
        {
          len = i + 1;
          break;
        }
      input[i] = ch;
    }

  uint64_t value = naive_atoi16(input, 4);
  return value;
}

void execute_command()
{
  if (!enabled)
    {
      shell_cur = 0;
      shell_cmd = memset(shell_cmd, 0, shell_size);
      return;
    }
  putchar('\n');
  if (shell_cur == 0) { goto fini; }
  shell_cmd[shell_cur++] = 0;
  klog("\'%s\'\n", shell_cmd);
  uint64_t rip = resolve_function_address(shell_cmd);
  /*int      index = 0;
  for (; index < COMMAND_COUNT; index++) {
    int count = strlen(command_list[index]);
    if (strncmp(shell_cmd, command_list[index], count) == 0) {
      char* args = NULL;
      if (count < shell_cur) {
        args = (void*)(shell_cmd + count);
      }
      commands[index](args);
      goto fini;
    }
  }*/
  if (rip != 0) { call_function((void*)rip); }
  else
    {
      dump_function_names();
    }
fini:
  shell_cur = 0;
  shell_cmd = memset(shell_cmd, 0, shell_size);
  putchar('>');
}
