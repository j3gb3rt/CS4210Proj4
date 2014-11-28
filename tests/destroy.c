
/* destroy.c - test that destroying a segment deletes the segment, even after remapping*/

#include "../rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_STRING1 ""
#define OFFSET2 1000


int main(int argc, char **argv)
{
     rvm_t rvm;
     char *seg;
     void *segs[1];
     trans_t trans;
     
     rvm = rvm_init("rvm_segments");
     
     rvm_destroy(rvm, "testseg");
     
     seg = (char *) rvm_map(rvm, "testseg", 10000);
	 segs[0] = seg;

     /* write some data and commit it */
     trans = rvm_begin_trans(rvm, 1, segs);
     rvm_about_to_modify(trans, seg, 0, 100);
     sprintf(seg, "hello, world");
     
     rvm_commit_trans(trans);
     rvm_unmap(rvm,seg);

     rvm_destroy(rvm, "testseg");
     seg = (char *) rvm_map(rvm, "testseg", 10000);


     if(strcmp(seg, TEST_STRING1)) {
	  printf("ERROR: seg is incorrect (%s)\n", seg);
	  exit(2);
     }
     

     rvm_unmap(rvm, seg);
     printf("OK\n");
     exit(0);
}
