#include "uefi/uefi.h"
#include "bootinfo.h"
#include "elf.h"
#include <stdint.h>
#include <stdbool.h>

bool is_image_valid(Elf64_Ehdr* hdr)
{
  if (hdr->e_ident[EI_MAG0] != 0x7F)
    return false;
  if (hdr->e_ident[EI_MAG1] != 0x45)
    return false;
  if (hdr->e_ident[EI_MAG2] != 0x4C)
    return false;
  if (hdr->e_ident[EI_MAG3] != 0x46)
    return false;
  return true;
}

void* load(char* buf, unsigned int size)
{
  Elf64_Ehdr* hdr = (Elf64_Ehdr*)buf;
  printf("%s, %d, %x, %x, %d, %d\n", hdr->e_ident, hdr->e_type, hdr->e_entry, hdr->e_phoff, hdr->e_phentsize, hdr->e_phnum);
  if (!is_image_valid(hdr))
  {
    return NULL;
  }
  Elf64_Phdr* phdr = (Elf64_Phdr*)(buf + hdr->e_phoff);
  uint32_t align   = 4096;
  uint64_t addr;
  uint64_t begin = 18446744073709551615U;
  for (uint16_t i = 0; i < hdr->e_phnum; i++)
  {
    Elf64_Phdr current = phdr[i];
    if (current.p_type != PT_LOAD)
    {
      continue;
    }
    if (current.p_align > align)
    {
      align = current.p_align;
    }
    uint64_t phdr_begin = current.p_vaddr & ~(align - 1);
    if (begin > phdr_begin)
    {
      begin = phdr_begin;
    }
  }

  BS->AllocatePages(AllocateAnyPages, EfiLoaderData, size / EFI_PAGE_SIZE, (void*)&addr);
  printf("Address %x\n", addr);
  for (int i = 0; i < hdr->e_phnum; i++)
  {
    Elf64_Phdr current = phdr[i];
    printf("%x, %x, %x, %x, %x, %d, %x\n", current.p_align, current.p_filesz, current.p_memsz, current.p_offset, current.p_paddr, current.p_type, current.p_vaddr);
    if (current.p_type != PT_LOAD)
    {
      continue;
    }
    uint64_t p_addr = (uint64_t)addr + current.p_vaddr - begin;
    void* res       = memmove((void*)p_addr, buf + current.p_offset, current.p_filesz);
    printf("memmove result %x, p_addr %x\n", res, p_addr);
  }
  void* entry_addr = (void*)addr + hdr->e_entry - begin;
  printf("%x\n", entry_addr);
  if ((uint64_t)entry_addr > (uint64_t)addr + size)
  {
    printf("Failed to get correct address\n");
    return NULL;
  }
  return entry_addr;
}

void cls()
{
  efi_guid_t con_guid = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;
  simple_text_output_interface_t* con;
  BS->LocateProtocol(&con_guid, NULL, (void**)&con);
  con->ClearScreen(con);
}

int guid_equal(efi_guid_t a, efi_guid_t b)
{
  return a.Data1 == b.Data1 &&
    a.Data2 == b.Data2 &&
    a.Data3 == b.Data3 &&
    a.Data4[0] == b.Data4[0] &&
    a.Data4[1] == b.Data4[1] &&
    a.Data4[2] == b.Data4[2] &&
    a.Data4[3] == b.Data4[3] &&
    a.Data4[4] == b.Data4[4] &&
    a.Data4[5] == b.Data4[5] &&
    a.Data4[6] == b.Data4[6] &&
    a.Data4[7] == b.Data4[7];
}

void* get_configuration_table(efi_guid_t guid)
{
  for (int i = 0; i < ST->NumberOfTableEntries; i++)
  {
    efi_configuration_table_t table = ST->ConfigurationTable[i];
    efi_guid_t table_guid           = table.VendorGuid;
    if (guid_equal(table_guid, guid))
    {
      printf("found entry\n");
      return table.VendorTable;
    }
  }
  return NULL;
}

bool rsdp_checksum(acpi_rsdp_structure_t* rsdp)
{
  uint8_t* bytes = (uint8_t*)rsdp;
  uint8_t sum    = 0;
  int checksum_bytes;
  if (rsdp->revision > 0)
  {
    printf("Revision was: %d, using XSDT checksum\n", rsdp->revision);
    checksum_bytes = 36;
  }
  else
  {
    checksum_bytes = 20;
  }
  for (int i = 0; i < checksum_bytes; i++)
  {
    sum += bytes[i];
  }
  return sum == 0;
}

int main()
{
  efi_status_t status;
  __attribute__((sysv_abi)) int (*ptr)(kernel_bootinfo_t*);
  int kernel_size = 1048576 * 5;
  int initfs_size = 12288;

  FILE* initfs = fopen("\\initfs", "r");
  if (initfs == NULL)
  {
    return 1;
  }
  FILE* elf = fopen("\\kernel", "r");
  if (elf == NULL)
  {
    return 1;
  }

  char* elf_buffer;
  BS->AllocatePool(EfiLoaderData, kernel_size, (void**)&elf_buffer);
  if (elf_buffer == NULL)
  {
    return 1;
  }
  void* initfs_buffer;
  BS->AllocatePool(EfiLoaderData, initfs_size, &initfs_buffer);
  if (initfs_buffer == NULL)
  {
    return 1;
  }
  fread(initfs_buffer, initfs_size, 1, initfs);
  fread(elf_buffer, kernel_size, 1, elf);
  fclose(elf);
  fclose(initfs);

  ptr = load(elf_buffer, kernel_size);
  free(elf_buffer);
  sleep(3);
  cls();
  if (ptr == NULL)
  {
    printf("Got Null Entry Point\n");
  }
  printf("getting rsdp\n");
  efi_guid_t acpi_guid     = ACPI_20_TABLE_GUID;
  efi_guid_t fallback_guid = ACPI_TABLE_GUID;
  void* acpi_table         = get_configuration_table(acpi_guid);
  bool fallback            = false;
  if (acpi_table == NULL)
  {
    acpi_table = get_configuration_table(fallback_guid);
    if (acpi_table == NULL)
    {
      // TODO ACTUALLY HANDLE UNSUPPORTED SYSTEMS
      printf("ACPI Unsupported\n");
      return 1;
    }
    else
    {
      fallback = true;
    }
  }

  printf("getting EFI gop\n");
  efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  efi_gop_t* gop;
  status = BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
  if (status != EFI_SUCCESS)
  {
    return 1;
  }

  acpi_rsdp_structure_t* rsdp_structure = (acpi_rsdp_structure_t*)acpi_table;
  printf("RSDP_STRUCTURE: %x, %d, %x, %d, %x\n", rsdp_structure->signature, rsdp_structure->checksum, rsdp_structure->oemid, rsdp_structure->revision, rsdp_structure->rsdt_address);
  if (!strcmp(rsdp_structure->signature, "RSD PTR ") || !rsdp_checksum(rsdp_structure))
  {
    printf("Invalid RSDP\n");
    for (;;);
  }

  printf("filling out boot info\n");
  kernel_bootinfo_t bootinfo;
  memset(&bootinfo, 0, sizeof(bootinfo));
  strncpy(bootinfo.magic, "OS649", 5);
  bootinfo.base                  = gop->Mode->FrameBufferBase;
  bootinfo.pitch                 = gop->Mode->Information->PixelsPerScanLine * 4;
  bootinfo.horizontal_resolution = gop->Mode->Information->HorizontalResolution;
  bootinfo.vertical_resolution   = gop->Mode->Information->VerticalResolution;
  bootinfo.size                  = gop->Mode->FrameBufferSize;
  bootinfo.initfs                = initfs_buffer;
  bootinfo.initfs_size           = initfs_size;
  if (rsdp_structure->revision > 0)
  {
    bootinfo.xsdt_address = rsdp_structure->xsdt_address;
    bootinfo.rsdt_address = 0;
  }
  else
  {
    bootinfo.xsdt_address = 0;
    bootinfo.rsdt_address = rsdp_structure->rsdt_address;
  }
  printf("exiting boot services\n");
  status             = 0;
  uint64_t mmap_size = 0, desc_size = 0, cnt = 3, map_key = 0;
  uint32_t desc_version;
  efi_memory_descriptor_t* memory_map = NULL;
  while (1)
  {
    status = BS->GetMemoryMap(&mmap_size, memory_map, &map_key, &desc_size, &desc_version);
    if (status == EFI_SUCCESS)
      break;
    status = BS->AllocatePool(EfiLoaderData, mmap_size, (void**)&memory_map);
  }
  status = BS->ExitBootServices(IM, map_key);

  if (status != EFI_SUCCESS || mmap_size == 0)
  {
    for (;;);
  }

  for (int i = 0; i < (mmap_size / desc_size); i++)
  {
    efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint8_t*)memory_map + (i * desc_size));
    desc->VirtualStart            = desc->PhysicalStart;
  }

  RT->SetVirtualAddressMap(mmap_size, desc_size, desc_version, memory_map);

  bootinfo.mmap      = (loader_memory_descriptor_t*)memory_map;
  bootinfo.mmap_size = mmap_size;
  bootinfo.desc_size = desc_size;

  ptr(&bootinfo);
}
