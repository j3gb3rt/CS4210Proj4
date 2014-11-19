include "rvm.h"

rvm_t rvm_init(const char *directory);

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);

void rvm_unmap(rvm_t rvm, void *segbase);

void rvm_destroy(rvm_t rvm, const char *segname);
