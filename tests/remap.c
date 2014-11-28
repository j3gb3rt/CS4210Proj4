/* remap.c - test that mapping, unmapping, then mapping works
 * correctly */

#include "../rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_STRING "hello, world"

int main(int argc, char **argv)
{
     rvm_t rvm;
     char *seg;
     void *segs[1];
     trans_t trans;
     
     rvm = rvm_init("rvm_segments");
	 
	 rvm_destroy(rvm, "remapseg");
     
     seg = (char *) rvm_map(rvm, "remapseg", 10000);
	 segs[0] = seg;

     /* write some data and commit it */
     trans = rvm_begin_trans(rvm, 1, segs);
     rvm_about_to_modify(trans, seg, 0, 100);
     sprintf(seg, TEST_STRING);
     
     rvm_commit_trans(trans);

	 /* unmap the segment */
	 rvm_unmap(rvm, "remapseg");

	 /* map the segment back */
	 seg = (char *) rvm_map(rvm, "remapseg", 10000);

     /* test that the data was restored */

     if(strcmp(seg, TEST_STRING1)) {
	  printf("ERROR: hello world not found(%s)\n",
		 seg);
	  exit(2);
     }
     
     rvm_unmap(rvm, seg);
     printf("OK\n");
     exit(0);
}

