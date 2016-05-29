#ifndef SMT_USER_DEFS_H
#define SMT_USER_DEFS_H

#include <stddef.h>

/*
 * this interface should be implemented by user in order to
 * allocate buffers for SMT
 */
void* smt_get_buffer(size_t size);

/*
 * this interface should be implemented by user in order to free buffer that
 * is allocated by using smt_get_buffer
 */
void smt_free_buffer(void* bufPtr);

#endif /* SMT_USER_DEFS_H */
