# Makefile for Gomoku

CC = gcc
CFLAGS = -Wall -O3
TARGET = gomoku
SRC = gomoku.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) file.txt

run: all
	./$(TARGET)

.PHONY: all clean run