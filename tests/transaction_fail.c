
/* transaction_fail.c - test that begin_transaction will fail if multiple transactions are applied to a segment */

#include "../rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000


/* proc1 writes some data, commits it, then exits */
void proc1() 
{
     rvm_t rvm;
     trans_t trans, trans2;
     char* segs[0];
     
     rvm = rvm_init("rvm_segments");
	 rvm_verbose(1);
     rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);

     
     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     
     trans2 = rvm_begin_trans(rvm, 1, (void **) segs);
     if(trans2 == -1){
	printf("begin transaction correctly fails to allow multiple transactions to modify a segment\n");
     }
}

int main(int argc, char **argv){
	proc1();
}

