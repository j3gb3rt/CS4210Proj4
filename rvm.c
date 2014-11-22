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
	segment_t *node = malloc(sizeof(segments_t));
	node->segbase = segbase;
	node->trans_t = -1;
	

	//maybe redo to just push on top for efficiency
	if(list->head == NULL){
		list->head = node;
	}else{
		segment_t *curr = head;
		while(curr->next != NULL){
			curr = curr->next;
		}
		curr->next = node;
	}
	return node;
}

void remove_segment(seg_list *list, void *segbase){
	segment_t *curr = list->head;
	if(curr == NULL){
		return;
	}else{
		if(curr->segbase == segbase){
			list->head = curr->next;
			free(curr);
		}else{
			segment_t *prev = curr;
			curr = curr->next;
			while(prev->next != NULL){
				if(curr->segbase == segbase){
					prev->next = curr->next;
					free(curr);
					curr = prev->next;
					return;
				}else{	
					prev = curr;
					curr = curr->next;
				}
			}
		}
	}
}

segment_t *find_segment(seg_list_t *list, char *segname){
	segment_t *curr = list->head;
	if(curr == NULL)
		return NULL;
	}else{
		if(curr->segname == segname){
			return curr;
		}
	}
	while(curr->next != NULL){
		curr = curr->next;
		if(curr->segname == segname){
			return curr;
		}
	}
	return NULL;
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

//check through seg_list. if exists with same size error
//else extend memory of segment.
//should memory be blank, or copied from some file?
void *rvm_map(rvm_t rvm, const char *segname, int size_to_create){
	segment_t *segment;
	segment = find_segment(seg_list, segname);
	if(segment != NULL){
		if(segment->size < size_to_create){
			segment->segbase = realloc(segbase, size_to_create);
			segment->size = size_to_create;
		}else{
			//return error?
		}
	}else{
		void *seg_base = malloc(size_to_create);
		segment = add_segment(seg_list,seg_base);
		segment->size = size_to_create;
		segment->segname = segname;
	}

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
	segment_t segments[numsegs];
	int i;
	for(i = 0; i <numsegs; i++){
		segments[i] = find_segment(*(segbase + i));
		if(segments[i]->transaction != -1){
			return (trans_t) -1;
		}
	}
	for(i = 0; i <numsegs; i++){
		segments[i]->trans = 1; //generate a trans number
	}
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
