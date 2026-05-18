# ═══════════════════════════════════════════════
# EduOS Makefile
# Student: Laurent Namacha | Reg: 25311351030
# ═══════════════════════════════════════════════

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pthread
TARGET  = eduos
SOURCES = main_sim.c process_manager.c thread_manager.c ipc_module.c

# ─── Default: build everything ───
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) -lrt

# ─── Race condition demo (NO mutex) ───
race:
	$(CC) $(CFLAGS) -DNO_MUTEX -o eduos_race $(SOURCES) -lrt
	@echo "Running race condition demo 5 times..."
	@for i in 1 2 3 4 5; do \
		echo "Run $$i:"; \
		./eduos_race; \
	done

# ─── Fixed demo (WITH mutex) ───
fixed:
	$(CC) $(CFLAGS) -o eduos_fixed $(SOURCES) -lrt
	@echo "Running fixed mutex demo 5 times..."
	@for i in 1 2 3 4 5; do \
		echo "Run $$i:"; \
		./eduos_fixed; \
	done

# ─── Memory check with valgrind ───
memcheck:
	valgrind --leak-check=full \
	         --show-leak-kinds=all \
	         --track-origins=yes \
	         --verbose \
	         ./$(TARGET) 2>&1 | tee valgrind_output.txt
	@echo "Valgrind output saved to valgrind_output.txt"

# ─── Clean build artifacts ───
clean:
	rm -f $(TARGET) eduos_race eduos_fixed *.o \
	      valgrind_output.txt pcb_snapshot.json
	@echo "Cleaned all build artifacts"

.PHONY: all race fixed memcheck clean
