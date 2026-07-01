CC = gcc
CFLAGS = -Wall -ansi -pedantic -I include -g

SRC = src/main.c src/sub_op.c src/sub_proc.c src/sub_macro.c src/sub_utils.c

TARGET = assembler

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) *.ent *.ext *.ob *.am
