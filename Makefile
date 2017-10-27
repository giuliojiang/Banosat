CFLAGS=-I. -Wall -Wextra -Wpedantic $(flags) -g -DDEBUG

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
