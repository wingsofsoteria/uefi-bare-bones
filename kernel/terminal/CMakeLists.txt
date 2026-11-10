ADD_LIBRARY(terminal STATIC)
TARGET_SOURCES(terminal PRIVATE log.c pixel.c tty.c)