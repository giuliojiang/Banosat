CFLAGS=-I. -Wall -Wextra -Wpedantic -g $(flags)

SAT_OBJECTS = ../linkedlist.o \
	../arraymap.o \
	../arraylist.o \
	../context.o \
	../variable.o \
	../clause.o \
	../assignment_level.o

all: temp_linkedlist \
	temp_arraymap \
	temp_assign \
	temp_unassign

temp_linkedlist: temp_linkedlist.o $(SAT_OBJECTS)

temp_arraymap: temp_arraymap.o $(SAT_OBJECTS)

temp_assign: temp_assign.o $(SAT_OBJECTS)

temp_unassign: temp_unassign.o $(SAT_OBJECTS)

clean:
	rm -rf *.o
	rm -rf ../*.o
	rm -rf temp_linkedlist
