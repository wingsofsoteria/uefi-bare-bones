DRIVERS_SRC := $(wildcard drivers/*.c)
DRIVERS_OBJS := $(patsubst %,%.o,$(DRIVERS_SRC))
DRIVERS = drivers.a
OBJS += $(DRIVERS)
CLEAN_OBJS += $(DRIVERS) $(DRIVERS_OBJS)
$(DRIVERS): $(DRIVERS_OBJS)
	$(AR) rc $@ $^
