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
	$(CC) $(CFLAGS) tests/me_test.c $(OUT) -pthread -o mytest
	$(CC) $(CFLAGS) tests/abort.c $(OUT) -pthread -o abort
	$(CC) $(CFLAGS) tests/basic1.c $(OUT) -pthread -o basic1
	$(CC) $(CFLAGS) tests/basic2.c $(OUT) -pthread -o basic2
	$(CC) $(CFLAGS) tests/multi.c $(OUT) -pthread -o multi
	$(CC) $(CFLAGS) tests/multi-abort.c $(OUT) -pthread -o multiabort
	$(CC) $(CFLAGS) tests/truncate.c $(OUT) -pthread -o truncate

#server :
#	$(CC) $(CFLAGS) server.c $(OUT) -pthread -o server

#matrix:
#	$(CC) $(CFLAGS) src/gt_matrix.c $(OUT) -o bin/matrix

#all : gt_include.h gt_kthread.c gt_kthread.h gt_uthread.c gt_uthread.h gt_pq.c gt_pq.h gt_signal.h gt_signal.c gt_spinlock.h gt_spinlock.c gt_matrix.c
#	@echo Building...
#	@gcc -o matrix gt_matrix.c gt_kthread.c gt_pq.c gt_signal.c gt_spinlock.c gt_uthread.c
#	@echo Done!
#	@echo Now run './matrix'


clean :
	@rm *.o bin/*.a mytest abort basic1 basic2 multi multiabort truncate
	@echo Cleaned!
