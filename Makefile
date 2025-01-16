CC = gcc                # Kompilator
CFLAGS = -Wall -g       # Flagi kompilatora

# Nazwy plików wykonywalnych
EXECUTABLES = main cashier firefighter customer

all: $(EXECUTABLES)

main: main.c utils.c utils.h
	$(CC) $(CFLAGS) -o main main.c utils.c

cashier: cashier.c utils.c utils.h
	$(CC) $(CFLAGS) -o cashier cashier.c utils.c

firefighter: firefighter.c utils.c utils.h
	$(CC) $(CFLAGS) -o firefighter firefighter.c utils.c

customer: customer.c utils.c utils.h
	$(CC) $(CFLAGS) -o customer customer.c utils.c

clean:
	rm -f $(EXECUTABLES)
                                       
