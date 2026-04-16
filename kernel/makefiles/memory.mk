MEMORY_SRC := $(wildcard memory/*.c)
MEMORY_OBJS := $(patsubst %,%.o,$(MEMORY_SRC))
MEMORY = memory.a
OBJS += $(MEMORY)
CLEAN_OBJS += $(MEMORY_OBJS) $(MEMORY) 
$(MEMORY): $(MEMORY_OBJS)
	$(AR) rc $@ $^
