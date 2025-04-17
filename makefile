CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=c11

OUTPUT = echo

SOURCE_FILES = echo.c

all: $(OUTPUT)

$(OUTPUT): $(SOURCE_FILES)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCE_FILES) -lm

clean:
	rm -f $(OUTPUT)