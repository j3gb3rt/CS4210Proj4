#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rvm.h"

#define MAX_TRANSACTIONS 1000


rvm_t *transactions;
rvm_list_t *rvm_list;
rvm_t rvm_id = 0;

//initializes seg_list
seg_list_t *create_seg_list(){
	seg_list_t *list;
	list = (seg_list_t *) malloc(sizeof(seg_list_t));
	return list;
}
//adds segment to list
segment_t *add_segment(seg_list_t *list,void *segbase){
	segment_t *node;
	node = (segment_t *) malloc(sizeof(segment_t));
	node->segbase = segbase;
	node->transaction = -1;
	
	node->next = list->head;
	list->head = node;
	
	return node;
}

void remove_segment(seg_list_t *list, const char *segname){
	segment_t *curr = list->head;
	if(curr == NULL){
		return;
	}else{
		if(strcmp(curr->segname, segname) == 0){
			list->head = curr->next;
			free(curr);
		}else{
			segment_t *prev = curr;
			curr = curr->next;
			while(prev->next != NULL){
				if(strcmp(curr->segname, segname) ==0) {
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

segment_t *find_segment_by_ptr(seg_list_t *list, void *segbase){
	segment_t *curr = list->head;
	if(curr == NULL) {
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

segment_t *find_segment_by_name(seg_list_t *list, const char *segname){
	segment_t *curr = list->head;
	if(curr == NULL){
		return NULL;
	}else{
		if(strcmp(curr->segname, segname) == 0) {
			return curr;
		}
	}
	while(curr->next != NULL){
		curr = curr->next;
		if(strcmp(curr->segname, segname) == 0) {
			return curr;
		}
	}
	return NULL;
}

rvm_list_t *find_rvm(rvm_t rvm_id){	
	rvm_list_t *curr = rvm_list;
	if(curr == NULL){
		//this should never happen if they've called the init function
		return NULL;
	}else{
		if(curr->rvm_id == rvm_id){
			return curr;
		}
	}
	while(curr->next != NULL){
		curr = curr->next;
		if(curr->rvm_id == rvm_id){
			return curr;
		}
	}
	return NULL;
}

//slow, can improve if theres time
trans_t start_transaction(rvm_t rvm){
	int i;
	for(i = 0; i < MAX_TRANSACTIONS; i++){
		if(transactions[i] == 0){
			transactions[i] = rvm;
			return (trans_t) i;
		}
	}
}

void end_transaction(segment_t *segment){
	transactions[segment->transaction] = 0;
	segment->transaction = -1;
}

void write_to_log(segment_t *segment){
	char *seglogname;
	FILE *segment_log;
	
	seglogname = malloc(strlen(segment->rvm_dir) + strlen(segment->segname) + 6);
	
	strcpy(seglogname, segment->rvm_dir);
	strcat(seglogname, "/");
	strcat(seglogname, segment->segname);
	strcat(seglogname, ".log");
	segment_log = fopen(seglogname, "a");
	fwrite(&segment->size, sizeof(size_t), 1, segment_log);
	fwrite(segment->segbase, segment->size, 1, segment_log);
	fclose(segment_log);
	
	//free regions
	region_t *region = segment->regions;
	while(region != (region_t *) NULL){
		segment->regions = region->next;
		free(region->regbase);
		free(region);
		region = segment->regions;
	}
	end_transaction(segment);
	free(seglogname);
}

void load_and_update(segment_t *segment, char * segment_path, char * segment_log_path) {
	FILE *segment_backer;
	FILE *segment_log;
	size_t old_size;
	size_t log_entry_size;
		
	segment->segbase = malloc(segment->size);
	segment_backer = fopen(segment_path, "r+");
	fread(&old_size, sizeof(size_t), 1, segment_backer);
	printf("Backing File Segment size: %u\n", (unsigned int) old_size);
	fread(segment->segbase, old_size, 1, segment_backer);
	printf("Segment is : %s\n", (char *) segment->segbase);

	segment_log = fopen(segment_log_path, "r");
	while (fread(&log_entry_size, sizeof(size_t), 1, segment_log) == sizeof(size_t)) {
		fread(segment->segbase, log_entry_size, 1, segment_log);
		printf("Segment is : %s\n", (char *) segment->segbase);
		//If we move to region changes, things will actually happen here
	};
	
	//rewind(segment_backer);
	//fwrite(&segment->size, sizeof(int), 1, segment_backer);
	fclose(segment_backer);
	fclose(segment_log);
}

void undo_changes(segment_t *segment){
	region_t *region = segment->regions;
	while(region != NULL){
		//apply saved old-values to segment in a FILO order
		//this line seems broken what is reb base in comparison to segbase
		segment->segbase = memcpy(segment->segbase, region->regbase, region->size);
		segment->regions = region->next;
		free(region->regbase);
		free(region);
		region = segment->regions;
	}

	end_transaction(segment);
}


//Mappings

//must do init function before starting a transaction
rvm_t rvm_init(const char *directory) {
	int error;
	segment_t *segment;

	//if transactions hasn't been allocated, do it
	if(transactions == NULL){
		transactions = calloc(MAX_TRANSACTION, sizeof(int));
	}

	rvm_list_t *rvm_node;
	rvm_list_t *curr;
	//add new rvm_node
	if(rvm_list == NULL){
		rvm_node = malloc(sizeof(rvm_list_t));
		rvm_list = rvm_node;
	}else{
		curr = rvm_list;
		while(curr->next != NULL){
			curr = curr->next;
		}
		rvm_node = malloc(sizeof(rvm_list_t));
		curr->next = rvm_node;
	}

	rvm_node->seg_list = create_seg_list();
	rvm_node->rvm_id = (rvm_t) rvm_id++;
		
	error = mkdir(directory, S_IRWXU);
	if (error == 0 || errno == EEXIST) {
		rvm_node->rvm_dir = directory;
	} else {
		printf("You derped bro: %s\n", strerror(errno));
		return (rvm_t) -1;
	}

	DIR *dir;
	struct dirent *ent;
	size_t file_name_length;
	int is_log_file;

	if ((dir = opendir(directory)) != NULL) {
 		while ((ent = readdir(dir)) != NULL) {
    		if (!(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)) {
				//check cor entries ending in .log
				is_log_file = 0;				

				file_name_length = strlen(ent->d_name);		
				if (file_name_length > 5) {
					printf("File ends with %s\n", &ent->d_name[file_name_length - 4]);
					is_log_file = !strcmp(&ent->d_name[file_name_length - 4], ".log");
				}
				
				if (!is_log_file) {
					printf("Adding a segment for: %s\n", ent->d_name);
					segment = add_segment(rvm_node->seg_list, NULL);
					segment->size = 0;
					segment->segname = ent->d_name;
					segment->rvm_dir = rvm_node->rvm_dir;
				}
			}
		}
  	}
  	closedir (dir);

	return rvm_node->rvm_id;
}

//check through seg_list. if exists with same size error
//else extend memory of segment.
//should memory be blank, or copied from some file?
void *rvm_map(rvm_t rvm, const char *segname, int size_to_create){
	segment_t *segment;
	rvm_list_t *rvm_node;
	char *segment_path;
	char *segment_log_path;
	
	rvm_node = find_rvm(rvm);
	segment = find_segment_by_name(rvm_node->seg_list, segname);
	printf("segment found at %p\n", segment);

	segment_path = malloc(strlen(rvm_node->rvm_dir) + 2 + strlen(segname));
	segment_log_path = malloc(strlen(rvm_node->rvm_dir) + 6 + strlen(segname));
	printf("You made it past the mallocs with the following pointers\n");
	printf("segment_path: %p\n", segment_path);
	printf("segment_log_path: %p\n", segment_log_path);

	strcpy(segment_path, rvm_node->rvm_dir);
	strcat(segment_path, "/");
	strcat(segment_path, segname);
	printf("path to file: %s\n", segment_path);

	strcpy(segment_log_path, segment_path);
	strcat(segment_log_path, ".log");
	printf("path to log: %s\n", segment_log_path);

	if(segment != NULL){
		//segment exists but has been unmapped &
		//size_to_create is larger or eqaul to current size
		if(segment->segbase == NULL && segment->size <= size_to_create){		
			segment->size = size_to_create;
			load_and_update(segment, segment_path, segment_log_path);

		//segment needs to be elongated
		}else if(segment->size < size_to_create){
			segment->segbase = realloc(segment->segbase, size_to_create);
			segment->size = size_to_create;
		
		//tried to map existing or smaller segment
		}else{
			free(segment_path);
			free(segment_log_path);
			return (void *) -1;
		}
	//segment does not exist
	}else{
		//check exist
		FILE *new_segment_backer;
		//int size;

		//if (new_segment_backer = fopen(segment_path, "r")){
		//	printf("Found an existing file. Loading it in!\n");
		//	printf("File pointer: %p\n", new_segment_backer);

		//	//void *seg_base = malloc(size_to_create);
		//	segment = add_segment(rvm_node->seg_list, NULL);  //seg_base);
		//	segment->size = size_to_create;
		//	segment->segname = segname;
		//	segment->rvm_dir = rvm_node->rvm_dir;

		//	fread(&size, sizeof(int), 1, new_segment_backer);
		//	fclose(new_segment_backer);
		//	//add return to fail if commits make segbase larger than size to create
		//	load_and_update(segment, segment_path, segment_log_path);
		//}else{

		printf("making a new file\n");
		//THIS SHOULD CORRECTLY INITIALIZE BACKING STORE AND LOG
		FILE *new_segment_log;

		printf("backing path: %s\n", segment_path);
		new_segment_backer = fopen(segment_path, "w+");
		size_t header = (size_t) size_to_create;
		if(new_segment_backer == NULL){
			printf("open is broken: %s\n", strerror(errno));
		}
		fwrite(&header, sizeof(size_t), 1, new_segment_backer);
		fclose(new_segment_backer);

		new_segment_log = fopen(segment_log_path, "w+");
		//fseek(new_segment_log, sizeof(fpos_t), SEEK_SET);
		//fgetpos(new_segment_log, &log_head);
		//rewind(new_segment_log);
		//fwrite(&log_head, sizeof(fpos_t), 1, new_segment_log); 
		fclose(new_segment_log);

		void *seg_base = malloc(size_to_create);
		segment = add_segment(rvm_node->seg_list,seg_base);
		segment->size = size_to_create;
		segment->segname = segname;
		segment->rvm_dir = rvm_node->rvm_dir;
		//}
	}
	free(segment_path);
	free(segment_log_path);
	return segment->segbase;
}

void rvm_unmap(rvm_t rvm, void *segbase){
	//unmalloc segbase
	//if they can unmap during a transaction, this needs more work
	rvm_list_t *rvm_node = find_rvm(rvm);
	segment_t *segment = find_segment_by_ptr(rvm_node->seg_list, segbase);
	free(segbase);
	segbase = NULL;
}

void rvm_destroy(rvm_t rvm, const char *segname){
	remove(segname);
	rvm_list_t *rvm_node = find_rvm(rvm);
	remove_segment(rvm_node->seg_list, segname);
}

	
//Transactions
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){
//check through segment/trans mappings then write to them
	rvm_list_t *rvm_node = find_rvm(rvm);
	segment_t *segments[numsegs];
	int i;
	for(i = 0; i <numsegs; i++){
		segments[i] = find_segment_by_ptr(rvm_node->seg_list, *(segbases + i));
		if(segments[i]->transaction != -1){
			//if segments are already being modified, return error
			return (trans_t) -1;
		}
	}
	trans_t transaction= start_transaction(rvm);	//generate a trans number
	for(i = 0; i <numsegs; i++){
		segments[i]->transaction = transaction; 
	}
	return transaction;	
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){
//make sure segment is mapped to transaction
//save existing segment values in memory in case of abort
//kept it simple for now, can check for redundancy to make more efficient
//pushes new region onto stack
	rvm_t rvm = transactions[tid];
	rvm_list_t *rvm_node = find_rvm(rvm);
	segment_t *segment = find_segment_by_ptr(rvm_node->seg_list, segbase);
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
//iterate through segments, write to log and end transaction on appropriate segment
//erase saved old-values from memory
	rvm_t rvm = transactions[tid];
	rvm_list_t *rvm_node = find_rvm(rvm);
	segment_t *curr = rvm_node->seg_list->head;
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
	rvm_t rvm = transactions[tid];
	rvm_list_t *rvm_node = find_rvm(rvm);
	segment_t *curr = rvm_node->seg_list->head;
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
void rvm_truncate_log(rvm_t rvm) {
	segment_t *curr;
	rvm_list_t *rvm_node;
	char *segment_path;
	char *segment_log_path;
	FILE *segment_backer;
	FILE *segment_log;
	int log_read_size;
	void *changes;

	
	rvm_node = find_rvm(rvm);
	curr = rvm_node->seg_list->head;
	while (curr->next != NULL) {
		segment_path = malloc(strlen(curr->rvm_dir) + 2 + strlen(curr->segname));
		segment_log_path = malloc(strlen(curr->rvm_dir) + 6 + strlen(curr->segname));

		strcpy(segment_path, curr->rvm_dir);
		strcat(segment_path, "/");
		strcat(segment_path, curr->segname);

		strcpy(segment_log_path, segment_path);
		strcat(segment_log_path, ".log");

		segment_log = fopen(segment_log_path, "r");
		
		while(fread(&log_read_size, sizeof(size_t), 1, segment_log) == sizeof(size_t)) {
			changes = malloc(log_read_size);
			fread(changes, log_read_size, 1, segment_log);
			segment_backer = fopen(segment_path, "w+");
			fwrite(&log_read_size, sizeof(size_t), 1, segment_backer);
			fwrite(changes, log_read_size, 1, segment_backer);
			fclose(segment_backer);
			free(changes);
		}
		
		fclose(segment_log);
		segment_log = fopen(segment_log_path, "w+");
		fclose(segment_log);
		free(segment_path);
		free(segment_log_path);
	}
}

void rvm_verbose(int enable_flag){}
