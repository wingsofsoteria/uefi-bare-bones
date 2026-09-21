ADD_LIBRARY(lethe-utils STATIC)
TARGET_SOURCES(lethe-utils PRIVATE hash_32a.c hash_64a.c hashmap.c linked_list.c queue.c)
INCLUDE_DIRECTORIES(include)
