#include "../rvm.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("just give me a directory man. How is it that hard?");
	}
	else {
		rvm_init(argv[1]);
	}
}
