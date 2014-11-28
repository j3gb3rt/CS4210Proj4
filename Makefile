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
	$(CC) $(CFLAGS) tests/abort.c $(OUT) -o abort
	$(CC) $(CFLAGS) tests/basic.c $(OUT) -o basic
	$(CC) $(CFLAGS) tests/multi.c $(OUT) -o multi
	$(CC) $(CFLAGS) tests/multi-abort.c $(OUT) -o multiabort
	$(CC) $(CFLAGS) tests/truncate.c $(OUT) -o truncate
	$(CC) $(CFLAGS) tests/destroy.c $(OUT) -o destroy
	$(CC) $(CFLAGS) tests/remap.c $(OUT) -o remap
	$(CC) $(CFLAGS) tests/transaction_fail.c $(OUT) -o transaction_fail

clean :
	@rm *.o bin/*.a abort basic multi multiabort truncate destroy remap transaction_fail
	@echo Cleaned!
