#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "rvm.h"

#define MAX_TRANSACTIONS 1000


seg_list_t *seg_list;
int *transactions;
char **rvm_list;

//initializes seg_list
seg_list_t *create_seg_list(){
	seg_list_t *list = malloc(sizeof(seg_list_t));
	return list;
}
//adds segment to list
segment_t *add_segment(seg_list_t *list,void *segbase){
	segment_t *node = malloc(sizeof(segment_t));
	node->segbase = segbase;
	node->trans_t = -1;
	
	node->next = list->head;
	list->head = node;
	
	return node;
}

void remove_segment(seg_list *list, char *segname){
	segment_t *curr = list->head;
	if(curr == NULL){
		return;
	}else{
		if(curr->segname == segname){
			list->head = curr->next;
			free(curr);
		}else{
			segment_t *prev = curr;
			curr = curr->next;
			while(prev->next != NULL){
				if(curr->segname == segname){
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

segment_t *find_segment(seg_list_t *list, void *segbase){
	segment_t *curr = list->head;
	if(curr == NULL)
		return NULL;
	}else{
		if(curr->segbase == segbase){
			return curr;
		}
	}
	while(curr->next != NULL){
		curr = curr->next;
		if(curr->segbase == segbase){
			return curr;
		}
	}
	return NULL;
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

//slow, can improve if theres time
trans_t start_transaction(int *trans){
	int i;
	for(i = 0; i < MAX_TRANSACTIONS; i++){
		if(trans[i] == 0){
			trans[i] = 1;
			return (trans_t) i;
		}
	}
}

void end_transaction(int *trans, segment_t *segment){
	trans[segment->transaction] = 0;
	segment->transaction = -1;
}

void write_to_log(segment_t *segment){
	//TODO write to log	

	//free regions
	region_t region = segment->regions;
	while(region != NULL){
		segment->regions = region->next;
		free(region->regbase);
		free(region);
		region = segment->regions;
	}
	
	end_transaction(transactions, segment);
}

void undo_changes(segment_t *segment){
	region_t region = segment->regions;
	while(region != NULL){
		//apply saved old-values to segment in a FILO order
		segment->segbase = memcpy(segment->segbase, region->regbase, region->size);
		segment->regions = region->next;
		free(region->regbase);
		free(region);
		region = segment->regions;
	}

	end_transaction(transactions, segment);
}


//Mappings
rvm_t rvm_init(const char *directory) {
	int error;

	//TODO transactions and segment lists can't be global values as they are now
	//they should values of a rvm key
	seg_list = create_seg_list();
	transactions = calloc(MAX_TRANSACTIONS * sizeof(int));

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
		//segment exists but has been unmapped
		if(segment->segbase == NULL){
			//TODO apply log
			segment->segbase = malloc(size_to_create);
			segment->size = size_to_create;
		//segment needs to be elongated
		}else if(segment->size < size_to_create){
			segment->segbase = realloc(segment->segbase, size_to_create);
			segment->size = size_to_create;
		//tried to map existing or smaller segment
		}else{
			//TODO return error?
		}
	//segment does not exist
	}else{
		void *seg_base = malloc(size_to_create);
		segment = add_segment(seg_list,seg_base);
		segment->size = size_to_create;
		segment->segname = segname;
	}
	return segment->segbase;
}

void rvm_unmap(rvm_t rvm, void *segbase){
	//unmalloc segbase
	//if they can unmap during a transaction, this needs more work
	segment_t segment = find_segment(seg_list, segbase);
	free(segbase);
	segbase = NULL;
}

void rvm_destroy(rvm_t rvm, const char *segname){
	//TODO delete file
	remove_node(seg_list, segnmae);
}

	
//Transactions
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){
//check through segment/trans mappings then write to them
	segment_t segments[numsegs];
	int i;
	for(i = 0; i <numsegs; i++){
		segments[i] = find_segment(seg_list, *(segbase + i));
		if(segments[i]->transaction != -1){
			//if segments are already being modified, return error
			return (trans_t) -1;
		}
	}
	trans_t transaction= start_transaction(transactions);	//generate a trans number
	for(i = 0; i <numsegs; i++){
		segments[i]->trans = transaction; 
	}
	return transaction;	
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){
//make sure segment is mapped to transaction
//save existing segment values in memory in case of abort
//kept it simple for now, can check for redundancy to make more efficient
//pushes new region onto stack
	segment_t *segment = find_segment(seg_list, segbase);
	if(segment->transaction == tid){
		region_t *region = malloc(sizeof(region_t));
		region->regbase = malloc(size);
		region->next = segment->regions;
		region->size = size;
		segment->regions = region;
		//check to make sure region is in segment
		if(segment->size >= offset + size){
			region->regbase = memcpy(region->regbase, segbase + offset, size);
		}else{
			printf("error, tried to save region not in segment\n");
		}
	}
}

void rvm_commit_trans(trans_t tid){
//iterate through segments, write to log and end transaction on appropriate segments
//erase saved old-values from memory
	segment_t *curr = seg_list->head;
	if(curr != NULL){
		if(curr->transaction == tid){
			write_to_log(curr);// end transaction in write_to_log
		}
		while(curr->next != NULL){
			curr = curr->next;
			if(curr->transaction == tid){
				write_to_log(curr);
			}	
		}	
	}
}

void rvm_abort_trans(trans_t tid){
//return segment values to the old-values saved in memory
//erase saved old-values from memory
	segment_t *curr = seg_list->head;
	if(curr != NULL){
		if(curr->transaction == tid){
			undo_changes(curr);
		}
		while(curr->next != NULL){
			curr = curr->next;
			if(curr->transaction == tid){
				undo_changes(curr);
			}
		}
	}
}


//Logging
void rvm_truncate_log(rvm_t rvm);

void rvm_verbose(int enable_flag);
