CC = gcc
CFLAGS = -Wall

TARGET = polyreg
SRC = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)