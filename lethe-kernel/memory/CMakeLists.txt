ADD_LIBRARY(memory STATIC)
TARGET_SOURCES(memory PRIVATE alloc.c buddy_allocator.c initial_frame_allocator.c liballoc.c list_allocator.c paging.c)