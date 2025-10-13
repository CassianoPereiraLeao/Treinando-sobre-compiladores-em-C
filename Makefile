CC = gcc
CFLAGS = -Wall -ILexer
SRC = main.c Lexer/lexer.c
OUT = uebeti

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)