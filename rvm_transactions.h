include "rvm.h"

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);

void rvm_commit_trans(trans_t tid);

void rvm_abort_trans(trans_t tid);
