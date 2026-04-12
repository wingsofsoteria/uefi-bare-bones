ACPI_SRC := $(wildcard acpi/*.c)
ACPI_OBJS := $(patsubst %,%.o,$(ACPI_SRC))
ACPI = acpi.a
OBJS += $(ACPI)
CLEAN_OBJS += $(ACPI) $(ACPI_OBJS)
$(ACPI): $(ACPI_OBJS)
	$(AR) rc $@ $^
