# Makefile dla projektu bez plików obiektowych

CC = gcc                # Kompilator
CFLAGS = -Wall -g       # Flagi kompilatora

# Nazwy plików wykonywalnych
EXECUTABLES = main manager firefighter customer

all: $(EXECUTABLES)

main: main.c utils.c utils.h
	$(CC) $(CFLAGS) -o main main.c utils.c

manager: manager.c utils.c utils.h
	$(CC) $(CFLAGS) -o manager manager.c utils.c

firefighter: firefighter.c utils.c utils.h
	$(CC) $(CFLAGS) -o firefighter firefighter.c utils.c

customer: customer.c utils.c utils.h
	$(CC) $(CFLAGS) -o customer customer.c utils.c

clean:
	rm -f $(EXECUTABLES)
                                       
