#define MAX_TRANSACTION 10000

typedef unsigned int rvm_t;
typedef unsigned int trans_t;

typedef struct region {
	void *regbase;
	size_t size;		
	struct region *next;
} region_t;

typedef struct segment {
	void *segbase;
	const char *segname;
	trans_t transaction;
	size_t size;
	struct segment *next;
	region_t *regions;
} segment_t;

typedef struct seg_list {
	segment_t *head;
} seg_list_t;

typedef struct rvm_list {
	rvm_t rvm_id;
	seg_list_t *seg_list;
	struct rvm_list *next;
} rvm_list_t;

//Mappings
rvm_t rvm_init(const char *directory);

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);

void rvm_unmap(rvm_t rvm, void *segbase);

void rvm_destroy(rvm_t rvm, const char *segname);

	
//Transactions
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);

void rvm_commit_trans(trans_t tid);

void rvm_abort_trans(trans_t tid);


//Logging
void rvm_truncate_log(rvm_t rvm);

void rvm_verbose(int enable_flag);
