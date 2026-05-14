makefile
CC=gcc
CFLAGS=-Iinclude

all:
	$(CC) process_manager.c thread_manager.c ipc_module.c main_sim.c -o eduos $(CFLAGS)

clean:
	rm -f eduos
