CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -std=c11 -pedantic -O2
TARGET  := taskcli
SRC     := main.c task.c
OBJ     := $(SRC:.c=.o)

.PHONY: all debug run clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c task.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build com sanitizers para caçar vazamentos e acessos inválidos
debug: CFLAGS = -Wall -Wextra -std=c11 -pedantic -g -O0 -fsanitize=address,undefined
debug: clean $(TARGET)

run: $(TARGET)
	./$(TARGET) list

clean:
	rm -f $(OBJ) $(TARGET) *.tmp
