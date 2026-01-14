TARGET = gitsync
SOURCES = src/github_scanner.c
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

test: $(TARGET)
	./$(TARGET)

.PHONY: clean test