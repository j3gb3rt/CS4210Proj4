#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "rvm.h"

seg_list_t *seg_list;

//initializes seg_list
seg_list_t *create_seg_list(){
	seg_list_t *list = malloc(sizeof(seg_list_t));
	return list;
}
//adds segment to list
segment_t *add_segment(seg_list_t *list,void *segbase){
	segment_t node = malloc(sizeof(segments_t));
	node->segbase = segbase;
	node->trans_t = -1;
	
	if(list->head == NULL){
		list->head = node;
	}else{
		segment_t *curr = head;
		while(curr->next != NULL){
			curr = curr->next;
		}
		curr->next = node;
	}
}

//Mappings
rvm_t rvm_init(const char *directory) {
	int error;
	seg_list = create_seg_list();

	error = mkdir(directory, S_IRUSR | S_IWUSR);
	if (error) {
		printf("You derped bro: %s\n", strerror(errno));
	}
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create){
	//check through seg_list. if exists with same size error
	//else extend memory of segment.
	//make empty file here of same size here?
}

void rvm_unmap(rvm_t rvm, void *segbase){
	//unmalloc segbase
}

void rvm_destroy(rvm_t rvm, const char *segname){
	//delete file
	//not sure whether we should remove from seglist here or at unmap
}

	
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
