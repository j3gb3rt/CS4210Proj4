#include "../rvm.h"
#include <stdio.h>

int main(int argc, char **argv)
{
     rvm_t rvm;
     
     rvm = rvm_init("rvm_segments");
     rvm_destroy(rvm, "testseg");
}
