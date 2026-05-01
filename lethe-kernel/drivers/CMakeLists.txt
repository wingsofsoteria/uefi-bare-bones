ADD_LIBRARY(drivers STATIC)
TARGET_SOURCES(drivers PRIVATE config.c keyboard.c shell.c trace.c)