MEMORY_SRC := $(wildcard memory/*.c)
MEMORY_SRC := $(filter-out memory/list_allocator.c, $(MEMORY_SRC))
MEMORY_OBJS := $(patsubst %,%.o,$(MEMORY_SRC))
MEMORY = memory.a
OBJS += $(MEMORY)
CLEAN_OBJS += $(MEMORY_OBJS) $(MEMORY) 
$(MEMORY): $(MEMORY_OBJS)
	$(AR) rc $@ $^
