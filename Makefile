CC = gcc
CFLAGS = -Wall -Wextra -O2 $(shell pkg-config --cflags libcurl openssl)
LIBS = $(shell pkg-config --libs libcurl openssl)
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean docker-build docker-run
