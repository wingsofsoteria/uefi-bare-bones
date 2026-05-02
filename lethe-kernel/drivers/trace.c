#include "elf.h"
#include "log.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct StackFrame
{
  struct StackFrame* rbp;
  uint64_t           rip;
};

extern void* kernel_file_address;

static Elf64_Ehdr* kernel_header = NULL;
static Elf64_Sym*  symtab        = NULL;
static int         sym_entries   = 0;
static char*       strtab        = NULL;
static char*       shstrtab      = NULL;
// NOLINTNEXTLINE
uint64_t resolve_function_address(char* name)
{
  if (kernel_header == NULL)
    {
      kernel_header = kernel_file_address;
      Elf64_Shdr* kernel_shstrtab =
        kernel_file_address + kernel_header->e_shoff +
        (kernel_header->e_shstrndx * kernel_header->e_shentsize);
      shstrtab = kernel_file_address + kernel_shstrtab->sh_offset;

      Elf64_Shdr* sections = kernel_file_address + kernel_header->e_shoff;
      for (int i = 0; i < kernel_header->e_shnum; ++i)
        {
          Elf64_Shdr section = sections[i];
          if (section.sh_type == 2)
            {
              symtab      = kernel_file_address + section.sh_offset;
              sym_entries = section.sh_size / sizeof(Elf64_Sym);
            }
          if (
            section.sh_type == 3 &&
            strncmp(shstrtab + section.sh_name, ".strtab", 7) == 0
          )
            {
              strtab = kernel_file_address + section.sh_offset;
            }
        }
    }
  if (strtab == NULL || symtab == NULL)
    {
      kernel_log_debug("Could not find symbol table");
      return 0;
    }
  for (int i = 0; i < sym_entries; i++)
    {
      Elf64_Sym symbol = symtab[i];
      if ((symbol.st_info & 0xF) != 2) { continue; }
      if (strlen(strtab + symbol.st_name) != strlen(name)) { continue; }
      if (
        strncmp(
          strtab + symbol.st_name,
          name,
          strlen(strtab + symbol.st_name)
        ) == 0
      )
        {
          return symbol.st_value;
        }
    }
  return 0;
}

void dump_function_names()
{
  if (kernel_header == NULL)
    {
      kernel_header = kernel_file_address;
      Elf64_Shdr* kernel_shstrtab =
        kernel_file_address + kernel_header->e_shoff +
        (kernel_header->e_shstrndx * kernel_header->e_shentsize);
      shstrtab = kernel_file_address + kernel_shstrtab->sh_offset;

      Elf64_Shdr* sections = kernel_file_address + kernel_header->e_shoff;
      for (int i = 0; i < kernel_header->e_shnum; ++i)
        {
          Elf64_Shdr section = sections[i];
          if (section.sh_type == 2)
            {
              symtab      = kernel_file_address + section.sh_offset;
              sym_entries = section.sh_size / sizeof(Elf64_Sym);
            }
          if (
            section.sh_type == 3 &&
            strncmp(shstrtab + section.sh_name, ".strtab", 7) == 0
          )
            {
              strtab = kernel_file_address + section.sh_offset;
            }
        }
    }
  if (strtab == NULL || symtab == NULL) { return; }
  for (int i = 0; i < sym_entries; i++)
    {
      Elf64_Sym symbol = symtab[i];
      if ((symbol.st_info & 0xF) != 2) { continue; }
      printf("%s\n", strtab + symbol.st_name);
    }
}
// NOLINTNEXTLINE
char* resolve_function_name(uint64_t rip)
{
  if (kernel_header == NULL)
    {
      kernel_header = kernel_file_address;
      Elf64_Shdr* kernel_shstrtab =
        kernel_file_address + kernel_header->e_shoff +
        (kernel_header->e_shstrndx * kernel_header->e_shentsize);
      shstrtab = kernel_file_address + kernel_shstrtab->sh_offset;

      Elf64_Shdr* sections = kernel_file_address + kernel_header->e_shoff;
      for (int i = 0; i < kernel_header->e_shnum; ++i)
        {
          Elf64_Shdr section = sections[i];
          if (section.sh_type == 2)
            {
              symtab      = kernel_file_address + section.sh_offset;
              sym_entries = section.sh_size / sizeof(Elf64_Sym);
            }
          if (
            section.sh_type == 3 &&
            strncmp(shstrtab + section.sh_name, ".strtab", 7) == 0
          )
            {
              strtab = kernel_file_address + section.sh_offset;
            }
        }
    }

  if (strtab == NULL) { return "???"; }
  if (symtab == NULL) { return "???"; }
  Elf64_Sym best = symtab[0];
  for (int i = 0; i < sym_entries; i++)
    {
      Elf64_Sym symbol = symtab[i];
      if ((symbol.st_info & 0xF) != 2) { continue; }
      if (symbol.st_value > best.st_value && symbol.st_value <= rip)
        {
          best = symbol;
        }
    }
  return strtab + best.st_name;
}

void walk_stack()
{
  struct StackFrame* frame;
  asm("mov %%rbp, %0"
    : "=r"(frame));
  printf("\n==TRACE==\n");
  while (frame != NULL)
    {
      char* function = resolve_function_name(frame->rip);
      printf("[%p] %s\n", frame->rip, function);
      frame = frame->rbp;
    }
  printf("\n==TRACE==\n");
}
