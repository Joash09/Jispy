##
# Lisp Project
#
# @file
# @version 0.1

CC=cc
FLAGS=
LDFLAGS=-leditline

SOURCES=prompt.c
OBJS=$(SOURCES:.c=.o)
TARGET=main

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:
	@rm $(TARGET) $(OBJS)

# end
