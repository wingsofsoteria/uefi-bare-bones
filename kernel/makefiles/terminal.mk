TERMINAL_SRC := $(wildcard terminal/*.c)
TERMINAL_OBJS := $(patsubst %,%.o,$(TERMINAL_SRC))
TERMINAL = terminal.a
OBJS += $(TERMINAL)
CLEAN_OBJS += $(TERMINAL) $(TERMINAL_OBJS)
$(TERMINAL): $(TERMINAL_OBJS)
	$(AR) rc $@ $^
