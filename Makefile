CC = gcc
CFLAGS = -Wall

TARGET = polyreg
SRC = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ -lm

clean:
	rm -f $(TARGET)