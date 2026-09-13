ADD_LIBRARY(acpi STATIC)
TARGET_SOURCES(acpi PRIVATE acpi.c hpet.c mcfg.c ioapic.c lapic.c madt.c timer.c)
