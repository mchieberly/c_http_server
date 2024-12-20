# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror

# Targets
TARGETS = client server

all: $(TARGETS)

client: client.o
	$(CC) $(CFLAGS) -o client client.o

server: server.o
	$(CC) $(CFLAGS) -o server server.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) *.o

