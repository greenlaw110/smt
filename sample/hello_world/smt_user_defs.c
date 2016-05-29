#include "smt_user_defs.h"
#include <stdlib.h>
#include <stddef.h>

void* smt_get_buffer(size_t size) {
    void* x = malloc(size);
    return x;
}
