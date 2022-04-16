##
# Lisp Project
#
# @file
# @version 0.1

CC=cc
FLAGS=-Wall
LDFLAGS=-leditline -lm

SOURCES=prompt.c mpc.c
OBJS=$(SOURCES:.c=.o)
TARGET=main

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:
	@rm $(TARGET) $(OBJS)

# end
