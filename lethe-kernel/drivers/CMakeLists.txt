ADD_LIBRARY(drivers STATIC)
TARGET_SOURCES(drivers PRIVATE utils.c config.c keyboard.c shell.c trace.c)
