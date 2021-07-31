# Compiler
CC = gcc

# Compiler flags:
# -g debug
# -Wall warnings
# -O3 level 3 optimization
CFLAGS = -ggdb -Wall -O3  -fsanitize-recover=all -fno-omit-frame-pointer

# target executable
TARGET = sicasm

# Included header directories 
INCLUDE = -Ilib -ISIC

# All object files needed for target.
OBJS =  main.o Functions.o Hash_Table.o Queue.o  assembler_components.o initialize.o pass1.o pass2.o
OBJSPATH = $(addprefix obj/, $(OBJS))

default: $(TARGET)

all: $(TARGET)

$(TARGET): obj $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) -O3 $(OBJSPATH) -o $(TARGET)

obj: obj
	mkdir obj

main.o: main.c lib/Functions.h lib/Hash_Table.h lib/Queue.h SIC/assembler_components.h SIC/initialize.h SIC/pass1.h SIC/pass2.h
	$(CC) $(CFLAGS) $(INCLUDE) -c main.c -o obj/main.o

pass2.o: SIC/pass2.c SIC/pass2.h lib/Functions.h lib/Hash_Table.h lib/Queue.h SIC/assembler_components.h
	$(CC) $(CFLAGS) $(INCLUDE) -c SIC/pass2.c -o obj/pass2.o

pass1.o: SIC/pass1.c SIC/pass1.h lib/Functions.h lib/Hash_Table.h lib/Queue.h SIC/assembler_components.h
	$(CC) $(CFLAGS) $(INCLUDE) -c SIC/pass1.c -o obj/pass1.o

initialize.o: SIC/initialize.c SIC/initialize.h lib/Hash_Table.h SIC/assembler_components.h
	$(CC) $(CFLAGS) $(INCLUDE) -c SIC/initialize.c -o obj/initialize.o

assembler_components.o: SIC/assembler_components.c SIC/assembler_components.h
	$(CC) $(CFLAGS) $(INCLUDE) -c SIC/assembler_components.c -o obj/assembler_components.o

Functions.o: lib/Functions.c lib/Functions.h
	$(CC) $(CFLAGS) $(INCLUDE) -c lib/Functions.c -o obj/Functions.o

Hash_Table.o: lib/Hash_Table.c lib/Hash_Table.h
	$(CC) $(CFLAGS) $(INCLUDE) -c lib/Hash_Table.c -o obj/Hash_Table.o

Queue.o: lib/Queue.c lib/Queue.h
	$(CC) $(CFLAGS) $(INCLUDE) -c lib/Queue.c -o obj/Queue.o

clean:
	rm sicasm
	rm -r obj