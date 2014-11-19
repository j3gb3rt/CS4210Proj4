include "rvm_mapping.h"
include "rvm_transactions.h"
include "rvm_loggin.h"

typedef struct rvm {
	char * backing_file
} rvm_t;

typedef unsigned int trans_t;

//Mappings
extern rvm_t rvm_init(const char *directory);

extern void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);

extern void rvm_unmap(rvm_t rvm, void *segbase);

extern void rvm_destroy(rvm_t rvm, const char *segname);


//Transactions
extern trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);

extern void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);

extern void rvm_commit_trans(trans_t tid);

extern void rvm_abort_trans(trans_t tid);


//Logging
extern void rvm_truncate_log(rvm_t rvm);

extern void rvm_verbose(int enable_flag);
