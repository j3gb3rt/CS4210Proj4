#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "rvm.h"



//Mappings
rvm_t rvm_init(const char *directory) {
	int error;

	//create list with segment transaction mappings
	error = mkdir(directory, S_IRUSR | S_IWUSR);
	if (error) {
		printf("You derped bro: %s\n", strerror(errno));
	}
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);

void rvm_unmap(rvm_t rvm, void *segbase);

void rvm_destroy(rvm_t rvm, const char *segname);

	
//Transactions
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){
//check through segment/trans mappings then write to them
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){
//make sure segment is mapped to transaction
//save existing segment values in memory in case of abort
}

void rvm_commit_trans(trans_t tid){
//write committed new values to log
//erase saved old-values from memory
}

void rvm_abort_trans(trans_t tid){
//return segment values to the old-values saved in memory
//erase saved old-values from memory
}


//Logging
void rvm_truncate_log(rvm_t rvm);

void rvm_verbose(int enable_flag);
