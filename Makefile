CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = $(shell pkg-config --libs libcurl libcrypto libssl)
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean docker-build docker-run
