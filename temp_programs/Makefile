CFLAGS=-I. -Wall -Wextra -Wpedantic -g $(flags)

all: temp_linkedlist temp_arraymap temp_assign

temp_linkedlist: temp_linkedlist.o ../linkedlist.o

temp_arraymap: temp_arraymap.o ../arraymap.o ../arraylist.o

temp_assign: temp_assign.o ../arraymap.o ../arraylist.o ../context.o ../linkedlist.o ../variable.o ../clause.o

clean:
	rm -rf *.o
	rm -rf ../*.o
	rm -rf temp_linkedlist
