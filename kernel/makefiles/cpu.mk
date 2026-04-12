CPU_SRC := $(wildcard cpu/*.c cpu/*.s)
CPU_OBJS := $(patsubst %,%.o,$(CPU_SRC))
CPU = cpu.a
OBJS += $(CPU)
CLEAN_OBJS += $(CPU) $(CPU_OBJS)
$(CPU): $(CPU_OBJS)
	$(AR) rc $@ $^
