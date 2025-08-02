#include "uefi/uefi.h"
#include "../kernel/include/types.h"
#include <stdint.h>
#include <stdbool.h>
#include "loader.h"
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

efi_status_t get_rsdp(kernel_bootinfo_t* bootinfo)
{
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
      return EFI_NOT_FOUND;
    }
    else
    {
      fallback = true;
    }
  }
  acpi_rsdp_structure_t* rsdp_structure = (acpi_rsdp_structure_t*)acpi_table;
  printf("RSDP_STRUCTURE: %x, %d, %x, %d, %x\n", rsdp_structure->signature, rsdp_structure->checksum, rsdp_structure->oemid, rsdp_structure->revision, rsdp_structure->rsdt_address);
  if (!strcmp(rsdp_structure->signature, "RSD PTR ") || !rsdp_checksum(rsdp_structure))
  {
    return EFI_NOT_FOUND;
  }
  if (rsdp_structure->revision > 0)
  {
    bootinfo->xsdt_address = rsdp_structure->xsdt_address;
    bootinfo->rsdt_address = 0;
  }
  else
  {
    bootinfo->xsdt_address = 0;
    bootinfo->rsdt_address = rsdp_structure->rsdt_address;
  }
  return EFI_SUCCESS;
}

efi_status_t get_initfs(kernel_bootinfo_t* bootinfo)
{
  int initfs_size = 1048576;

  FILE* initfs = fopen("\\initfs", "r");
  if (initfs == NULL)
  {
    return EFI_NOT_FOUND;
  }
  void* initfs_buffer;
  BS->AllocatePool(EfiLoaderData, initfs_size, &initfs_buffer);
  if (initfs_buffer == NULL)
  {
    return EFI_OUT_OF_RESOURCES;
  }
  fread(initfs_buffer, initfs_size, 1, initfs);
  fclose(initfs);
  map_pages(initfs_buffer + KERNEL_START, initfs_buffer, initfs_size / EFI_PAGE_SIZE, 0b10);
  bootinfo->initfs_size = initfs_size;
  bootinfo->initfs      = initfs_buffer + KERNEL_START;
  return EFI_SUCCESS;
}

efi_status_t get_gop(kernel_bootinfo_t* bootinfo)
{
  printf("getting EFI gop\n");
  efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  efi_gop_t* gop;
  efi_status_t status = BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
  if (status != EFI_SUCCESS)
  {
    return status;
  }
  map_pages((void*)gop->Mode->FrameBufferBase + KERNEL_START, (void*)gop->Mode->FrameBufferBase, gop->Mode->FrameBufferSize / EFI_PAGE_SIZE, 0b10);
  bootinfo->base                  = gop->Mode->FrameBufferBase + KERNEL_START;
  bootinfo->pitch                 = gop->Mode->Information->PixelsPerScanLine * 4;
  bootinfo->horizontal_resolution = gop->Mode->Information->HorizontalResolution;
  bootinfo->vertical_resolution   = gop->Mode->Information->VerticalResolution;
  bootinfo->size                  = gop->Mode->FrameBufferSize;
  return EFI_SUCCESS;
}

kernel_bootinfo_t* get_bootinfo()
{
  void* p_addr;
  BS->AllocatePool(EfiLoaderData, sizeof(kernel_bootinfo_t), (void**)&p_addr);
  kernel_bootinfo_t* bootinfo = p_addr + KERNEL_START;
  memset(bootinfo, 0, sizeof(kernel_bootinfo_t));
  get_rsdp(bootinfo);

  get_gop(bootinfo);

  strncpy(bootinfo->magic, "OS649", 5);

  get_initfs(bootinfo);
  printf("%x", ((kernel_bootinfo_t*)p_addr)->base);
  BS->AllocatePool(EfiLoaderData, sizeof(mmap_t), (void**)&bootinfo->mmap);
  mmap_t dummy        = quick_memory_map();
  uint64_t page_count = 0;
  for (int i = 0; i < (dummy.size / dummy.desc_size); i++)
  {
    efi_memory_descriptor_t* desc  = (efi_memory_descriptor_t*)((uint8_t*)dummy.addr + (i * dummy.desc_size));
    page_count                    += desc->NumberOfPages;
  }
  BS->AllocatePool(EfiLoaderData, (page_count / 8) * sizeof(uint8_t), (void**)&bootinfo->bitmap);

  mmap_t mmap = quick_memory_map();
  mmap.addr   = (void*)((uint64_t)mmap.addr + KERNEL_START);
  for (int i = 0; i < (mmap.size / mmap.desc_size); i++)
  {
    efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint8_t*)mmap.addr + (i * mmap.desc_size));
    desc->VirtualStart            = KERNEL_START + desc->PhysicalStart;
  }
  memcpy(bootinfo->mmap, &mmap, sizeof(mmap_t));
  return bootinfo;
}
