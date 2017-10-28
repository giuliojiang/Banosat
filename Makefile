# Testing flags
CFLAGS=-I. -Wall -Wextra -Wpedantic $(flags) -g -DDEBUG
# Production flags
# CFLAGS=-I. -Wall -Wextra -Wpedantic $(flags)

DEPS = $(wildcard *.h)
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

%.o : %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

sat: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f *.o sat

.PHONY: tests
tests: sat
	bash ./tests.sh
