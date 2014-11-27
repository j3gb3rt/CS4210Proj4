CC = gcc
CFLAGS = -g
LDFLAGS = 
LIBS = .
SRC = rvm.c
OBJ = $(SRC:.c=.o)

OUT = bin/librvm.a

.PHONY: all
.DEFAULT: all

all : $(OUT) tests

.c.o: 
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	@mkdir -p bin
	ar rcs $(OUT) $(OBJ)

tests: $(OUT)
	$(CC) $(CFLAGS) tests/me_test.c $(OUT) -o mytest
	$(CC) $(CFLAGS) tests/abort.c $(OUT) -o abort
	$(CC) $(CFLAGS) tests/basic.c $(OUT) -o basic
	$(CC) $(CFLAGS) tests/multi.c $(OUT) -o multi
	$(CC) $(CFLAGS) tests/multi-abort.c $(OUT) -o multiabort
	$(CC) $(CFLAGS) tests/truncate.c $(OUT) -o truncate

clean :
	@rm *.o bin/*.a mytest abort basic multi multiabort truncate
	@echo Cleaned!
