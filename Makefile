# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0` -lpthread

# Targets
TARGETS = server client

all: $(TARGETS)

server: server.o
	$(CC) $(CFLAGS) -o server server.o $(LDFLAGS)

client: client.o
	$(CC) $(CFLAGS) -o client client.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) *.o

