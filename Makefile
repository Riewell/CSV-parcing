CC=gcc

all: csv_parcing.o files_functions.o create_tdat.o create_result.o help.o
	$(CC) -o csv_parcing csv_parcing.o files_functions.o create_tdat.o create_result.o help.o

csv_parcing.o: csv_parcing.c
	$(CC) -c -Wall -g -std=c99 -o csv_parcing.o csv_parcing.c

files_functions.o: files_functions.c
	$(CC) -c -Wall -g -std=c99 -o files_functions.o files_functions.c

create_tdat.o: create_tdat.c
	$(CC) -c -Wall -g -std=c99 -o create_tdat.o create_tdat.c

create_result.o: create_result.c
	$(CC) -c -Wall -g -std=c99 -o create_result.o create_result.c

help.o: help.c
	$(CC) -c -Wall -g -std=c99 -o help.o help.c

clean:
	rm -rf *.o

