#ifndef __KERNEL_ACPI_PIC_H__
#define __KERNEL_ACPI_PIC_H__

#include <stdint.h>

#define PIC1         0x20 /* IO base address for master PIC */
#define PIC2         0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2 + 1)

#define ICW1_ICW4      0x01 /* Indicates that ICW4 will be present */
#define ICW1_SINGLE    0x02 /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL     0x08 /* Level triggered (edge) mode */
#define ICW1_INIT      0x10 /* Initialization - required! */

#define ICW4_8086       0x01 /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02 /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08 /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM       0x10 /* Special fully nested (not) */

#define CASCADE_IRQ 2

#define LAPIC_TIMER_INITIAL_COUNT_REGISTER 0x380
#define LAPIC_TIMER_CURRENT_COUNT_REGISTER 0x390
#define LAPIC_TIMER_DIVIDE_CONFIG_REGISTER 0x3E0
#define LAPIC_TIMER_REGISTER               0x320
#define LAPIC_INTERRUPT_MASK               0x10000
#define LAPIC_TIMER_MODE_PERIODIC          (1 << 17)
#define LAPIC_TIMER_IRQ                    32

typedef struct
{
  uint8_t pin;
  uint8_t vector;
  uint8_t delivery_mode : 3;
  uint8_t destination_mode : 1;
  uint8_t delivery_status : 1;
  uint8_t pin_polarity : 1;
  uint8_t remote_irr : 1;
  uint8_t trigger_mode : 1;
  uint8_t mask : 1;
  uint64_t reserved : 39;
  uint8_t destination;
} __attribute__((packed)) ioapic_redtbl_t;

typedef struct
{
  uint8_t type;
  uint8_t length;
  uint8_t processor_uid;
  uint8_t apic_id;
  uint32_t flags;
} __attribute__((packed)) acpi_local_apic_structure_t;

typedef struct
{
  uint8_t type;
  uint8_t length;
  uint8_t io_apic_id;
  uint8_t reserved;
  uint32_t io_apic_addr;
  uint32_t global_system_interrupt_base;
} __attribute__((packed)) acpi_io_apic_structure_t;

void apic_enable_timer();
void apic_sleep(uint64_t microseconds);

#endif
